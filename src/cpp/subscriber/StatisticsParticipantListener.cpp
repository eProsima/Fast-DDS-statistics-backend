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

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/log/Log.hpp>

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
        const std::string& endpoint_name)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Get the domain from the database
    auto domain_ids =
            database_->get_entities_by_name(EntityKind::DOMAIN,
                    std::to_string(statistics_participant->get_domain_id()));
    if (domain_ids.empty())
    {
        logError(STATISTICS_BACKEND, endpoint_name + " discovered on Domain " + std::to_string(statistics_participant->get_domain_id())
                      + " but there is no such Domain in the database");
        return;
    }

    std::shared_ptr<database::Domain> domain =
            std::const_pointer_cast<database::Domain>(
        std::static_pointer_cast<const database::Domain>(database_->get_entity(domain_ids.front().second)));

    // Get the participant from the database
    GUID_t endpoint_guid = info.info.guid();
    GUID_t participant_guid(endpoint_guid.guidPrefix, EntityId_t());
    auto participant_ids = database_->get_entities_by_guid(EntityKind::PARTICIPANT, to_string(participant_guid));
    if (participant_ids.empty())
    {
        logError(STATISTICS_BACKEND, endpoint_name + " discovered on Participant " + to_string(participant_guid)
                      + " but there is no such Participant in the database");
        return;
    }
    std::shared_ptr<database::DomainParticipant> participant =
            std::const_pointer_cast<database::DomainParticipant>(
        std::static_pointer_cast<const database::DomainParticipant>(database_->get_entity(
            participant_ids.front().second)));

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

    // Create the endpoint
    auto endpoint = create_endpoint(endpoint_guid, info, participant, topic);

    /* Start processing the locator info */

    // Get the host for the participant
    // May be empty, if the physical info didn't arrive.
    auto hosts = database_->get_entities(EntityKind::HOST, participant_ids.front().second);

    // Take all the locators already defined for this participant.
    // This will be used only if there is no physical info.
    // Even if the host info is not available, the participant can only have one locator
    // with a given address/port combination, so we don't want to add duplicates
    auto participant_locators = database_->get_entities(EntityKind::LOCATOR, participant_ids.front().second);

    // Routine to process one locator from the locator list of the endpoint
    auto process_locators = [&](const Locator_t& dds_locator)
        {
            std::shared_ptr<database::Locator> locator = std::make_shared<database::Locator>(to_string(dds_locator));

            // See if the participant has the physical info attached
            if (!hosts.empty())
            {
                assert(hosts.size() == 1);

                //There is physical info. Add the host info to the locator
                locator->name += std::string("@") + hosts.front()->name;

                // Look for the locator
                auto locator_ids = database_->get_entities_by_name(EntityKind::LOCATOR, locator->name);
                if (locator_ids.empty())
                {
                    // The locator is not in the database. Add the new one.
                    locator->id = database_->generate_entity_id();
                    endpoint->locators[locator->id] = locator;
                }
                else
                {
                    // The locator exists. Add the existing one.
                    auto existing = std::const_pointer_cast<database::Locator>(
                            std::static_pointer_cast<const database::Locator>(database_->get_entity(locator_ids.front().second)));
                    endpoint->locators[existing->id] = existing;
                }
            }
            else
            {
                // If there is no physical information,
                // we need to create only one copy of the locator for this participant
                using namespace std::placeholders;
                auto found = std::find_if(participant_locators.begin(), participant_locators.end(),
                    std::bind([](std::shared_ptr<database::Entity> new_locator, std::shared_ptr<const database::Entity> existing)
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
            }
        };

    for (auto dds_locator : info.info.remote_locators().unicast)
    {
        process_locators(dds_locator);
    }
    for (auto dds_locator : info.info.remote_locators().multicast)
    {
        process_locators(dds_locator);
    }

    // Push the endpoint
    entity_queue_->push(timestamp, endpoint);
}

template<>
std::shared_ptr<database::DDSEndpoint> StatisticsParticipantListener::create_endpoint(
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
std::shared_ptr<database::DDSEndpoint> StatisticsParticipantListener::create_endpoint(
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

    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
    switch (info.status)
    {
        case ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT:
        {
            // Get the domain from the database
            auto domain_ids =
                    database_->get_entities_by_name(EntityKind::DOMAIN,
                            std::to_string(participant->get_domain_id()));
            if (domain_ids.empty())
            {
                logError(STATISTICS_BACKEND, "Participant discovered on Domain " + std::to_string(participant->get_domain_id())
                              + " but there is no such Domain in the database");
                return;
            }

            std::shared_ptr<database::Domain> domain =
                    std::const_pointer_cast<database::Domain>(
                std::static_pointer_cast<const database::Domain>(database_->get_entity(domain_ids.front().second)));

            // Create the participant and push it to the queue
            GUID_t participant_guid = info.info.m_guid;
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
            // TODO [ILG] : Process these messages and save the updated QoS
            break;

        case ParticipantDiscoveryInfo::REMOVED_PARTICIPANT:
        case ParticipantDiscoveryInfo::DROPPED_PARTICIPANT:
            // TODO [ILG] : Process these messages and save the status of the entity
            break;
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

    switch (info.status)
    {
        case ReaderDiscoveryInfo::DISCOVERED_READER:
        {
            std::stringstream name;
            name << info.info.guid();
            process_endpoint_discovery(participant, info, name.str());
            break;
        }
        case ReaderDiscoveryInfo::CHANGED_QOS_READER:
        {
            // TODO [ILG] : Process these messages and save the updated QoS and/or Locators
            break;
        }
        case ReaderDiscoveryInfo::REMOVED_READER:
        {
            // TODO [ILG] : Process these messages and save the status of the entity
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

    switch (info.status)
    {
        case WriterDiscoveryInfo::DISCOVERED_WRITER:
        {
            std::stringstream name;
            name << info.info.guid();
            process_endpoint_discovery(participant, info, name.str());
            break;
        }
        case WriterDiscoveryInfo::CHANGED_QOS_WRITER:
        {
            // TODO [ILG] : Process these messages and save the updated QoS and/or Locators
            break;
        }
        case WriterDiscoveryInfo::REMOVED_WRITER:
        {
            // TODO [ILG] : Process these messages and save the status of the entity
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
