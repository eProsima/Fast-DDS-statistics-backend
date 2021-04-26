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

template<typename T>
std::string to_string(
        T data)
{
    std::stringstream ss;
    ss << data;
    return ss.str();
}


template<typename T>
void StatisticsParticipantListener::process_endpoint_discovery(
        eprosima::fastdds::dds::DomainParticipant* statistics_participant,
        T&& info,
        EntityKind endpoint_kind,
        const std::string& endpoint_name)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    // Get the domain from the database
    auto domain_ids = database_->get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant->get_domain_id()));
    if (domain_ids.empty())
    {
        throw Error(endpoint_name + " discovered on Domain " + std::to_string(statistics_participant->get_domain_id())
                + " but there is no such Domain in the database");
    }

    std::shared_ptr<database::Domain> domain =
            std::const_pointer_cast<database::Domain>(
                std::static_pointer_cast<const database::Domain>(database_->get_entity(domain_ids.front().second)));

    // Check whether the endpoint is already on the database
    GUID_t endpoint_guid = info.info.guid();
    auto endpoint_ids = database_->get_entities_by_guid(endpoint_kind, to_string(endpoint_guid));
    if (!endpoint_ids.empty())
    {
        throw Error(endpoint_name + " discovered " + to_string(endpoint_guid)
                + " but there is already a " + endpoint_name + " with the same GUID in the database");
    }

    // Get the participant from the database
    GUID_t participant_guid(endpoint_guid.guidPrefix, EntityId_t());
    auto participant_ids = database_->get_entities_by_guid(EntityKind::PARTICIPANT, to_string(participant_guid));
    if (participant_ids.empty())
    {
        throw Error(endpoint_name + " discovered on Participant " + to_string(participant_guid)
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

    // Create the endpoint and push it to the queue
    auto endpoint = create_endpoint(endpoint_guid, info, participant, topic);
    entity_queue_->push(timestamp, endpoint);
}

template<>
std::shared_ptr<database::Entity> StatisticsParticipantListener::create_endpoint(
        const GUID_t& guid,
        const fastrtps::rtps::WriterDiscoveryInfo& info,
        std::shared_ptr<database::DomainParticipant> participant,
        std::shared_ptr<database::Topic> topic)
{
    return std::make_shared<database::DataWriter>(
        to_string(guid),
        writer_info_to_backend_qos(info),
        to_string(guid),
        participant,
        topic);
}

template<>
std::shared_ptr<database::Entity> StatisticsParticipantListener::create_endpoint(
        const GUID_t& guid,
                const fastrtps::rtps::ReaderDiscoveryInfo& info,

        std::shared_ptr<database::DomainParticipant> participant,
        std::shared_ptr<database::Topic> topic)
{
    return std::make_shared<database::DataReader>(
        to_string(guid),
        reader_info_to_backend_qos(info),
        to_string(guid),
        participant,
        topic);
}

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
    // First stop the data queue until the new entity is created
    data_queue_->stop_consumer();

    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
    switch (info.status)
    {
        case ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT:
            {
                // Get the domain from the database
                auto domain_ids = database_->get_entities_by_name(EntityKind::DOMAIN, std::to_string(participant->get_domain_id()));
                if (domain_ids.empty())
                {
                    throw Error("Participant discovered on Domain " + std::to_string(participant->get_domain_id())
                            + " but there is no such Domain in the database");
                }

                std::shared_ptr<database::Domain> domain =
                        std::const_pointer_cast<database::Domain>(
                            std::static_pointer_cast<const database::Domain>(database_->get_entity(domain_ids.front().second)));

                // Check whether the participant is already on the database
                GUID_t participant_guid = info.info.m_guid;
                auto participant_ids = database_->get_entities_by_guid(EntityKind::PARTICIPANT, to_string(participant_guid));
                if (!participant_ids.empty())
                {
                    throw Error("Participant discovered " + to_string(participant_guid)
                            + " but there is already a Participant with the same GUID in the database");
                }

                // Create the participant and push it to the queue
                auto participant = std::make_shared<database::DomainParticipant>(
                        info.info.m_participantName.to_string(),
                        participant_info_to_backend_qos(info),
                        to_string(participant_guid),
                        std::shared_ptr<database::Process>(),
                        domain);

                entity_queue_->push(timestamp, participant);
            }
            break;

        case ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT:
            // Update QoS on stored entity
            break;

        case ParticipantDiscoveryInfo::REMOVED_PARTICIPANT:
        case ParticipantDiscoveryInfo::DROPPED_PARTICIPANT:
            // Do nothing
            break;
    }

    // Wait until the entity queue is processed and restart the data queue
    entity_queue_->flush();
    data_queue_->start_consumer();
    (void)participant, (void)info;
}

void StatisticsParticipantListener::on_subscriber_discovery(
        DomainParticipant* participant,
        ReaderDiscoveryInfo&& info)
{
    // First stop the data queue until the new entity is created
    data_queue_->stop_consumer();

    switch (info.status)
    {
        case ReaderDiscoveryInfo::DISCOVERED_READER:
            process_endpoint_discovery(participant, info, EntityKind::DATAREADER, "DataReader");
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
        DomainParticipant* participant,
        WriterDiscoveryInfo&& info)
{
    // First stop the data queue until the new entity is created
    data_queue_->stop_consumer();

    switch (info.status)
    {
        case WriterDiscoveryInfo::DISCOVERED_WRITER:
            process_endpoint_discovery(participant, info, EntityKind::DATAWRITER, "DataWriter");
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
