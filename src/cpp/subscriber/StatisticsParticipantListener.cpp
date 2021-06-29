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

#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/rtps/common/EntityId_t.hpp>

#include "database/database_queue.hpp"
#include "QosSerializer.hpp"


namespace eprosima {
namespace statistics_backend {
namespace subscriber {

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

template<typename T>
std::string to_string(
        T data)
{
    std::stringstream ss;
    ss << data;
    return ss.str();
}

/**
 * Checks whether an entity id corresponds to a builtin statistics writer.
 * @param [in] entity_id The entity id to check.
 * @return true when the entity id corresponds to a builtin statistics writer.
 */
inline bool is_statistics_builtin(
        const fastrtps::rtps::EntityId_t& entity_id)
{
    return 0x60 == (0xE0 & entity_id.value[3]);
}

template<typename T>
void StatisticsParticipantListener::process_endpoint_discovery(
        T&& info)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Get the domain from the database
    // This may throw if the domain does not exist
    // The database MUST contain the domain, or something went wrong upstream
    std::shared_ptr<database::Domain> domain = std::const_pointer_cast<database::Domain>(
        std::static_pointer_cast<const database::Domain>(database_->get_entity(domain_id_)));

    // Get the participant from the database
    GUID_t endpoint_guid = info.info.guid();
    GUID_t participant_guid(endpoint_guid.guidPrefix, c_EntityId_RTPSParticipant);
    std::pair<EntityId, EntityId> participant_id;
    try
    {
        participant_id = database_->get_entity_by_guid(EntityKind::PARTICIPANT, to_string(participant_guid));
    }
    catch (const Exception&)
    {
        logError(STATISTICS_BACKEND, "endpoint " << to_string(endpoint_guid) + " discovered on Participant " + to_string(
                    participant_guid)
                + " but there is no such Participant in the database");
        return;
    }
    std::shared_ptr<database::DomainParticipant> participant =
            std::const_pointer_cast<database::DomainParticipant>(
        std::static_pointer_cast<const database::DomainParticipant>(database_->get_entity(
            participant_id.second)));

    assert(participant_id.first == domain_id_);

    // Check whether the topic is already in the database
    std::shared_ptr<database::Topic> topic;
    auto topic_ids = database_->get_entities_by_name(EntityKind::TOPIC, info.info.topicName().to_string());

    // Check if any of these topics is in the current domain AND shares the data type
    for (const auto& topic_id : topic_ids)
    {
        if (topic_id.first == domain_id_)
        {
            topic = std::const_pointer_cast<database::Topic>(
                std::static_pointer_cast<const database::Topic>(database_->get_entity(topic_id.second)));

            if (topic->data_type == info.info.typeName().to_string())
            {
                //Found the correct topic
                break;
            }
            else
            {
                // The data type is not the same, so it must be another topic
                topic.reset();
            }
        }
    }

    // If no such topic exists, create a new one
    if (!topic)
    {
        topic = std::make_shared<database::Topic>(
            info.info.topicName().to_string(),
            info.info.typeName().to_string(),
            domain);

        database::EntityDiscoveryInfo entity_discovery_info;
        entity_discovery_info.domain_id = domain_id_;
        entity_discovery_info.entity = topic;
        entity_queue_->push(timestamp, entity_discovery_info);
    }

    // Create the endpoint
    auto endpoint = create_endpoint(endpoint_guid, info, participant, topic);

    /* Start processing the locator info */

    // Take all the locators already defined for this participant.
    // This will be used only if there is no physical info.
    // Even if the host info is not available, the participant can only have one locator
    // with a given address/port combination, so we don't want to add duplicates
    auto participant_locators = database_->get_entities(EntityKind::LOCATOR, participant_id.second);

    // Routine to process one locator from the locator list of the endpoint
    auto process_locators = [&](const Locator_t& dds_locator)
            {
                std::shared_ptr<database::Locator> locator =
                        std::make_shared<database::Locator>(to_string(dds_locator));

                // we need to create only one copy of the locator for this participant
                using namespace std::placeholders;
                auto found = std::find_if(participant_locators.begin(), participant_locators.end(),
                                std::bind([](std::shared_ptr<database::Entity> new_locator,
                                std::shared_ptr<const database::Entity> existing)
                                {
                                    return new_locator->name == existing->name;
                                }, locator, _1));
                if (found != participant_locators.end())
                {
                    // The locator exists. Add the existing one.
                    auto existing = std::const_pointer_cast<database::Locator>(
                        std::static_pointer_cast<const database::Locator>(*found));
                    endpoint->locators[existing->id] = existing;
                }
                else
                {
                    // The locator is not in the database. Add the new one.
                    locator->id = database_->generate_entity_id();
                    endpoint->locators[locator->id] = locator;
                    participant_locators.push_back(locator);
                }
            };

    for (const auto& dds_locator : info.info.remote_locators().unicast)
    {
        process_locators(dds_locator);
    }
    for (const auto& dds_locator : info.info.remote_locators().multicast)
    {
        process_locators(dds_locator);
    }

    // Push the endpoint
    database::EntityDiscoveryInfo entity_discovery_info;
    entity_discovery_info.domain_id = domain_id_;
    entity_discovery_info.entity = endpoint;
    entity_queue_->push(timestamp, entity_discovery_info);
}

template<>
std::shared_ptr<database::DDSEndpoint> StatisticsParticipantListener::create_endpoint(
        const GUID_t& guid,
        const fastrtps::rtps::WriterDiscoveryInfo& info,
        std::shared_ptr<database::DomainParticipant> participant,
        std::shared_ptr<database::Topic> topic)
{
    std::stringstream name;
    name << "DataWriter_" << info.info.topicName().to_string() << "_" << info.info.guid().entityId;

    return std::make_shared<database::DataWriter>(
        name.str(),
        writer_info_to_backend_qos(info),
        to_string(guid),
        participant,
        topic);
}

template<>
std::shared_ptr<database::DDSEndpoint> StatisticsParticipantListener::create_endpoint(
        const GUID_t& guid,
        const fastrtps::rtps::ReaderDiscoveryInfo& info,

        std::shared_ptr<database::DomainParticipant> participant,
        std::shared_ptr<database::Topic> topic)
{
    std::stringstream name;
    name << "DataReader_" << info.info.topicName().to_string() << "_" << info.info.guid().entityId;

    return std::make_shared<database::DataReader>(
        name.str(),
        reader_info_to_backend_qos(info),
        to_string(guid),
        participant,
        topic);
}

StatisticsParticipantListener::StatisticsParticipantListener(
        EntityId domain_id,
        database::Database* database,
        database::DatabaseEntityQueue* entity_queue,
        database::DatabaseDataQueue* data_queue) noexcept
    : DomainParticipantListener()
    , domain_id_(domain_id)
    , database_(database)
    , entity_queue_(entity_queue)
    , data_queue_(data_queue)
{
}

void StatisticsParticipantListener::on_participant_discovery(
        DomainParticipant* /*participant*/,
        ParticipantDiscoveryInfo&& info)
{
    // First stop the data queue until the new entity is created
    data_queue_->stop_consumer();

    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
    switch (info.status)
    {
        case ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT:
        {
            // If the participant is already in the database, change the status
            try
            {
                EntityId participant_id =
                        database_->get_entity_by_guid(EntityKind::PARTICIPANT,
                                to_string(info.info.m_guid)).second;
                database_->change_entity_status(participant_id, true);
            }
            catch (BadParameter&)
            {
                // Get the domain from the database
                // This may throw if the domain does not exist
                // The database MUST contain the domain, or something went wrong upstream
                std::shared_ptr<database::Domain> domain = std::const_pointer_cast<database::Domain>(
                    std::static_pointer_cast<const database::Domain>(database_->get_entity(domain_id_)));

                // Create the participant and push it to the queue
                GUID_t participant_guid = info.info.m_guid;
                auto participant = std::make_shared<database::DomainParticipant>(
                    info.info.m_participantName.to_string(),
                    participant_info_to_backend_qos(info),
                    to_string(participant_guid),
                    std::shared_ptr<database::Process>(),
                    domain);

                database::EntityDiscoveryInfo entity_discovery_info;
                entity_discovery_info.domain_id = domain_id_;
                entity_discovery_info.entity = participant;
                entity_queue_->push(timestamp, entity_discovery_info);
            }
            break;
        }

        case ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT:
            // TODO [ILG] : Process these messages and save the updated QoS
            break;

        case ParticipantDiscoveryInfo::REMOVED_PARTICIPANT:
        case ParticipantDiscoveryInfo::DROPPED_PARTICIPANT:
        {
            // TODO [ILG] : Process these messages

            // Save the status of the entity
            EntityId participant_id =  database_->get_entity_by_guid(EntityKind::PARTICIPANT, to_string(
                                info.info.m_guid)).second;
            database_->change_entity_status(participant_id, false);

            break;
        }
    }

    // Wait until the entity queue is processed and restart the data queue
    entity_queue_->flush();
    data_queue_->start_consumer();
}

void StatisticsParticipantListener::on_subscriber_discovery(
        DomainParticipant* participant,
        ReaderDiscoveryInfo&& info)
{
    // First stop the data queue until the new entity is created
    data_queue_->stop_consumer();

    // Filter out our own statistics readers
    if (participant->guid().guidPrefix == info.info.guid().guidPrefix)
    {
        return;
    }

    switch (info.status)
    {
        case ReaderDiscoveryInfo::DISCOVERED_READER:
        {
            // If the datareader is already in the database, change the status
            try
            {
                EntityId datareader_id =
                        database_->get_entity_by_guid(EntityKind::DATAREADER,
                                to_string(info.info.guid())).second;
                database_->change_entity_status(datareader_id, true);
            }
            catch (BadParameter&)
            {
                process_endpoint_discovery(info);
            }
            break;
        }
        case ReaderDiscoveryInfo::CHANGED_QOS_READER:
        {
            // TODO [ILG] : Process these messages and save the updated QoS and/or Locators
            break;
        }
        case ReaderDiscoveryInfo::REMOVED_READER:
        {
            // TODO [ILG] : Process these messages

            // Save the status of the entity
            EntityId reader_id =
                    database_->get_entity_by_guid(EntityKind::DATAREADER,
                            to_string(info.info.guid())).second;
            database_->change_entity_status(reader_id, false);

            break;
        }
    }

    // Wait until the entity queue is processed and restart the data queue
    entity_queue_->flush();
    data_queue_->start_consumer();
}

void StatisticsParticipantListener::on_publisher_discovery(
        DomainParticipant* participant,
        WriterDiscoveryInfo&& info)
{
    // First stop the data queue until the new entity is created
    data_queue_->stop_consumer();

    static_cast<void>(participant);

    // Filter out other statistics writers
    if (is_statistics_builtin(info.info.guid().entityId))
    {
        return;
    }

    switch (info.status)
    {
        case WriterDiscoveryInfo::DISCOVERED_WRITER:
        {
            // If the datawriter is already in the database, change the status
            try
            {
                EntityId datawriter_id =
                        database_->get_entity_by_guid(EntityKind::DATAWRITER,
                                to_string(info.info.guid())).second;
                database_->change_entity_status(datawriter_id, true);
            }
            catch (BadParameter&)
            {
                process_endpoint_discovery(info);
            }
            break;
        }
        case WriterDiscoveryInfo::CHANGED_QOS_WRITER:
        {
            // TODO [ILG] : Process these messages and save the updated QoS and/or Locators
            break;
        }
        case WriterDiscoveryInfo::REMOVED_WRITER:
        {
            // TODO [ILG] : Process these messages

            // Save the status of the entity
            EntityId writer_id =
                    database_->get_entity_by_guid(EntityKind::DATAWRITER,
                            to_string(info.info.guid())).second;
            database_->change_entity_status(writer_id, false);

            break;
        }
    }

    // Wait until the entity queue is processed and restart the data queue
    entity_queue_->flush();
    data_queue_->start_consumer();
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
