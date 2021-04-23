/* Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file StatisticsParticipantListener.cpp
 */

#include "subscriber/StatisticsParticipantListener.hpp"

#include "fastdds/dds/domain/DomainParticipant.hpp"
#include "fastdds/dds/domain/DomainParticipantListener.hpp"
#include "fastdds/dds/core/status/StatusMask.hpp"

#include "database/database_queue.hpp"
#include "QosSerializer.hpp"


namespace eprosima {
namespace statistics_backend {
namespace subscriber {

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

const StatusMask StatisticsParticipantListener::StatisticsParticipantMask =
    StatusMask::none();

StatisticsParticipantListener::StatisticsParticipantListener(
        database::Database* database,
        database::DatabaseEntityQueue* entity_queue,
        database::DatabaseDataQueue* data_queue) noexcept
    : DomainParticipantListener()
    , database_(database)
    , entity_queue_(entity_queue)
    , data_queue_(data_queue)
{
}

void StatisticsParticipantListener::on_participant_discovery(
        DomainParticipant* participant,
        ParticipantDiscoveryInfo&& info)
{
    (void)participant, (void)info;
}

void StatisticsParticipantListener::on_subscriber_discovery(
        DomainParticipant* participant,
        ReaderDiscoveryInfo&& info)
{
    // First stop the data queue until the new entity is created
    data_queue_->stop_consumer();

    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
    switch (info.status)
    {
        case ReaderDiscoveryInfo::DISCOVERED_READER:
            {
                // Get the domain from the database
                auto domain_ids = database_->get_entities_by_name(EntityKind::DOMAIN, std::to_string(participant->get_domain_id()));
                if (domain_ids.empty())
                {
                    throw Error("DataReader discovered on Domain " + std::to_string(participant->get_domain_id())
                            + " but there is no such Domain in the database");
                }

                std::shared_ptr<database::Domain> domain =
                        std::const_pointer_cast<database::Domain>(
                            std::static_pointer_cast<const database::Domain>(database_->get_entity(domain_ids.front().second)));

                // Check whether the reader is already on the database
                GUID_t reader_guid = info.info.guid();
                auto reader_ids = database_->get_entities_by_guid(EntityKind::DATAREADER, to_string(reader_guid));
                if (!reader_ids.empty())
                {
                    throw Error("DataReader discovered " + to_string(reader_guid)
                            + " but there is already a DataReader with the same GUID in the database");
                }

                // Get the participant from the database
                GUID_t participant_guid(reader_guid.guidPrefix, EntityId_t());
                auto participant_ids = database_->get_entities_by_guid(EntityKind::PARTICIPANT, to_string(participant_guid));
                if (participant_ids.empty())
                {
                    throw Error("DataReader discovered on Participant " + to_string(participant_guid)
                            + " but there is no such Participant in the database");
                }
                std::shared_ptr<database::DomainParticipant> participant = 
                        std::const_pointer_cast<database::DomainParticipant>(
                            std::static_pointer_cast<const database::DomainParticipant>(database_->get_entity(participant_ids.front().second)));

                assert(participant_ids.front().first == domain_ids.front().second);

                // Check whether the topic is already in the database
                std::shared_ptr<database::Topic> topic;
                auto topic_ids = database_->get_entities_by_name(EntityKind::TOPIC, info.info.topicName().to_string());
                if (topic_ids.empty())
                {
                    // Create the Topic and push it to the queue
                    topic = std::make_shared<database::Topic>(
                            info.info.topicName().to_string(),
                            info.info.typeName().to_string(),
                            domain);

                    entity_queue_->push(timestamp, topic);
                }
                else
                {
                    topic = std::const_pointer_cast<database::Topic>(
                            std::static_pointer_cast<const database::Topic>(database_->get_entity(topic_ids.front().second)));
                    assert(topic_ids.front().first == domain_ids.front().second);
                }

                // Check whether the locators are already in the database
                for (auto dds_locator : info.info.remote_locators().unicast)
                {
                    std::shared_ptr<database::Locator> locator;
                    auto locator_ids = database_->get_entities_by_name(EntityKind::LOCATOR, to_string(dds_locator));
                    if (locator_ids.empty())
                    {
                        // Create the Locator and push it to the queue
                        locator = std::make_shared<database::Locator>(to_string(dds_locator));
                        entity_queue_->push(timestamp, locator);
                    }
                }
                for (auto dds_locator : info.info.remote_locators().multicast)
                {
                    std::shared_ptr<database::Locator> locator;
                    auto locator_ids = database_->get_entities_by_name(EntityKind::LOCATOR, to_string(dds_locator));
                    if (locator_ids.empty())
                    {
                        // Create the Locator and push it to the queue
                        locator = std::make_shared<database::Locator>(to_string(dds_locator));
                        entity_queue_->push(timestamp, locator);
                    }
                }

                // Create the reader and push it to the queue
                auto datareader = std::make_shared<database::DataReader>(
                        to_string(reader_guid),
                        reader_qos_to_backend_qos(info.info.m_qos),
                        to_string(reader_guid),
                        participant,
                        topic);

                entity_queue_->push(timestamp, datareader);
            }
            break;

        case ReaderDiscoveryInfo::CHANGED_QOS_READER:
            // Update QoS on stored entity
            break;

        case ReaderDiscoveryInfo::REMOVED_READER:
            // Do nothing
            break;
    }

    // Wait until the entity queue is processed and restart the data queue
    entity_queue_->flush();
    data_queue_->start_consumer();
    (void)participant, (void)info;
}

void StatisticsParticipantListener::on_publisher_discovery(
        eprosima::fastdds::dds::DomainParticipant* participant,
        fastrtps::rtps::WriterDiscoveryInfo&& info)
{
    // First stop the data queue until the new entity is created
    data_queue_->stop_consumer();

    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
    switch (info.status)
    {
        case WriterDiscoveryInfo::DISCOVERED_WRITER:
            {
                // Get the domain from the database
                auto domain_ids = database_->get_entities_by_name(EntityKind::DOMAIN, std::to_string(participant->get_domain_id()));
                if (domain_ids.empty())
                {
                    throw Error("DataWriter discovered on Domain " + std::to_string(participant->get_domain_id())
                            + " but there is no such Domain in the database");
                }

                std::shared_ptr<database::Domain> domain =
                        std::const_pointer_cast<database::Domain>(
                            std::static_pointer_cast<const database::Domain>(database_->get_entity(domain_ids.front().second)));

                // Check whether the writer is already on the database
                GUID_t writer_guid = info.info.guid();
                auto writer_ids = database_->get_entities_by_guid(EntityKind::DATAWRITER, to_string(writer_guid));
                if (!writer_ids.empty())
                {
                    throw Error("DataWriter discovered " + to_string(writer_guid)
                            + " but there is already a DataWriter with the same GUID in the database");
                }

                // Get the participant from the database
                GUID_t participant_guid(writer_guid.guidPrefix, EntityId_t());
                auto participant_ids = database_->get_entities_by_guid(EntityKind::PARTICIPANT, to_string(participant_guid));
                if (participant_ids.empty())
                {
                    throw Error("DataWriter discovered on Participant " + to_string(participant_guid)
                            + " but there is no such Participant in the database");
                }
                std::shared_ptr<database::DomainParticipant> participant = 
                        std::const_pointer_cast<database::DomainParticipant>(
                            std::static_pointer_cast<const database::DomainParticipant>(database_->get_entity(participant_ids.front().second)));

                assert(participant_ids.front().first == domain_ids.front().second);

                // Check whether the topic is already in the database
                std::shared_ptr<database::Topic> topic;
                auto topic_ids = database_->get_entities_by_name(EntityKind::TOPIC, info.info.topicName().to_string());
                if (topic_ids.empty())
                {
                    // Create the Topic and push it to the queue
                    topic = std::make_shared<database::Topic>(
                            info.info.topicName().to_string(),
                            info.info.typeName().to_string(),
                            domain);

                    entity_queue_->push(timestamp, topic);
                }
                else
                {
                    topic = std::const_pointer_cast<database::Topic>(
                            std::static_pointer_cast<const database::Topic>(database_->get_entity(topic_ids.front().second)));
                    assert(topic_ids.front().first == domain_ids.front().second);
                }

                // Check whether the locators are already in the database
                for (auto dds_locator : info.info.remote_locators().unicast)
                {
                    std::shared_ptr<database::Locator> locator;
                    auto locator_ids = database_->get_entities_by_name(EntityKind::LOCATOR, to_string(dds_locator));
                    if (locator_ids.empty())
                    {
                        // Create the Locator and push it to the queue
                        locator = std::make_shared<database::Locator>(to_string(dds_locator));
                        entity_queue_->push(timestamp, locator);
                    }
                }
                for (auto dds_locator : info.info.remote_locators().multicast)
                {
                    std::shared_ptr<database::Locator> locator;
                    auto locator_ids = database_->get_entities_by_name(EntityKind::LOCATOR, to_string(dds_locator));
                    if (locator_ids.empty())
                    {
                        // Create the Locator and push it to the queue
                        locator = std::make_shared<database::Locator>(to_string(dds_locator));
                        entity_queue_->push(timestamp, locator);
                    }
                }

                // Create the writer and push it to the queue
                auto datawriter = std::make_shared<database::DataWriter>(
                        to_string(writer_guid),
                        writer_qos_to_backend_qos(info.info.m_qos),
                        to_string(writer_guid),
                        participant,
                        topic);

                entity_queue_->push(timestamp, datawriter);
            }
            break;

        case WriterDiscoveryInfo::CHANGED_QOS_WRITER:
            // Update QoS on stored entity
            break;

        case WriterDiscoveryInfo::REMOVED_WRITER:
            // Do nothing
            break;
    }

    // Wait until the entity queue is processed and restart the data queue
    entity_queue_->flush();
    data_queue_->start_consumer();
    (void)participant, (void)info;
}


} //namespace database
} //namespace statistics_backend
} //namespace eprosima
