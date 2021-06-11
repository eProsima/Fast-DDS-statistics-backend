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
        entity_discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
        entity_queue_->push(timestamp, entity_discovery_info);
    }

    // Create the endpoint
    auto endpoint = create_endpoint(endpoint_guid, info, participant, topic);

    /* Start processing the locator info */

    // Routine to process one locator from the locator list of the endpoint
    auto process_locators = [&](const Locator_t& dds_locator)
            {
                std::shared_ptr<database::Locator> locator;

                // Look for an existing locator
                // There can only be one
                auto locator_ids = database_->get_entities_by_name(EntityKind::LOCATOR, to_string(dds_locator));
                assert(locator_ids.empty() || locator_ids.size() == 1);

                if (!locator_ids.empty())
                {
                    // The locator exists.
                    locator = std::const_pointer_cast<database::Locator>(
                        std::static_pointer_cast<const database::Locator>(database_->get_entity(locator_ids.front().
                                second)));
                }
                else
                {
                    // The locator is not in the database. Add the new one.
                    locator = std::make_shared<database::Locator>(to_string(dds_locator));
                    locator->id = database_->generate_entity_id();
                }

                endpoint->locators[locator->id] = locator;
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
    entity_discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
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

// Search for an address different from localhost in the locator list
bool search_address_in_locators(
        const eprosima::fastrtps::ResourceLimitedVector<Locator_t>& locators,
        std::string& address)
{
    for (auto locator : locators)
    {
        // if the address is not localhost
        if (!IPLocator::isLocal(locator))
        {
            // Convert the locator to an address with IP format
            address =  IPLocator::ip_to_string(locator);
            return true;
        }
    }
    return false;
}

// Return a IP obtained from participant locators
std::string get_address(
        const ParticipantProxyData& info)
{
    // The IP is obtained from the announced locators
    // Search for a locator with an IP different from localhost
    std::string address;

    // 1. default_locators.unicast
    if (search_address_in_locators(info.default_locators.unicast, address))
    {
        return address;
    }

    // 2. metatraffic_locators.unicast
    if (search_address_in_locators(info.metatraffic_locators.unicast, address))
    {
        return address;
    }

    // 3. default_locators.multicast
    if (search_address_in_locators(info.default_locators.multicast, address))
    {
        return address;
    }

    // 4. metatraffic_locators.multicast
    if (search_address_in_locators(info.metatraffic_locators.multicast, address))
    {
        return address;
    }

    // The only option is for localhost to be the only valid IP
    return "localhost";
}

// Return the participant_id
std::string get_participant_id(
        const GUID_t& guid)
{
    // The participant_id can be obtained from the last 4 octets in the GUID prefix
    std::stringstream buffer;
    buffer << std::hex << std::setfill('0');
    for (int i = 0; i < 3; i++)
    {
        buffer << std::setw(2) << static_cast<unsigned>(guid.guidPrefix.value[i + 8]);
        buffer << ".";
    }
    buffer << std::setw(2) << static_cast<unsigned>(guid.guidPrefix.value[3 + 8]);

    return buffer.str();
}

void StatisticsParticipantListener::on_participant_discovery(
        DomainParticipant* /*participant*/,
        ParticipantDiscoveryInfo&& info)
{
    // First stop the data queue until the new entity is created
    data_queue_->stop_consumer();

    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // The participant is already in the database
    try
    {
        EntityId participant_id = database_->get_entity_by_guid(EntityKind::PARTICIPANT, to_string(info.info.m_guid))
                        .second;

        // Build discovery info
        database::EntityDiscoveryInfo entity_discovery_info;
        entity_discovery_info.domain_id = domain_id_;
        entity_discovery_info.entity = std::const_pointer_cast<database::DomainParticipant>(
            std::static_pointer_cast<const database::DomainParticipant>(database_->get_entity(participant_id)));

        switch (info.status)
        {
            case ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT:
            {
                entity_discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
                break;
            }
            case ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT:
                // TODO [ILG] : Process these messages and save the updated QoS
                entity_discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UPDATE;
                break;

            case ParticipantDiscoveryInfo::REMOVED_PARTICIPANT:
            case ParticipantDiscoveryInfo::DROPPED_PARTICIPANT:
            {
                entity_discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY;
                break;
            }
        }

        entity_queue_->push(timestamp, entity_discovery_info);
    }
    // The participant is not in the database
    catch (BadParameter&)
    {
        if (info.status == ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT)
        {
            // Get the domain from the database
            // This may throw if the domain does not exist
            // The database MUST contain the domain, or something went wrong upstream
            std::shared_ptr<database::Domain> domain = std::const_pointer_cast<database::Domain>(
                std::static_pointer_cast<const database::Domain>(database_->get_entity(domain_id_)));

            std::string name = info.info.m_participantName.to_string();

            // If the user does not provide a specific name for the participant, give it a descriptive name
            if (name.empty())
            {
                // The name will be constructed as IP:participant_id
                name = get_address(info.info) + ":" + get_participant_id(info.info.m_guid);
            }

            // Create the participant and push it to the queue
            GUID_t participant_guid = info.info.m_guid;
            auto participant = std::make_shared<database::DomainParticipant>(
                name,
                participant_info_to_backend_qos(info),
                to_string(participant_guid),
                std::shared_ptr<database::Process>(),
                domain);

            // Build discovery info
            database::EntityDiscoveryInfo participant_discovery_info;
            participant_discovery_info.domain_id = domain_id_;
            participant_discovery_info.entity = participant;
            participant_discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
            entity_queue_->push(timestamp, participant_discovery_info);

            // Create metatraffic entities
            {
                // Meaningful prefix for metatraffic entities
                const std::string metatraffic_prefix = "___EPROSIMA___METATRAFFIC___DOMAIN_" +
                                                       std::to_string(domain_id_.value()) + "___";

                std::shared_ptr<database::Topic> metatraffic_topic;

                // Check if is the first participant discovered
                if (database_->get_entities(EntityKind::PARTICIPANT, domain_id_).size() == 0)
                {
                    // Create the metatraffic topic
                    metatraffic_topic = std::make_shared<database::Topic>(
                        metatraffic_prefix + "TOPIC",
                        metatraffic_prefix + "TYPE",
                        domain);

                    // Push it to the queue
                    database::EntityDiscoveryInfo topic_discovery_info;
                    topic_discovery_info.domain_id = domain_id_;
                    topic_discovery_info.entity = metatraffic_topic;
                    entity_queue_->push(timestamp, topic_discovery_info);
                }
                else
                {
                    // Obtain reference to metatraffic topic
                    EntityId metatraffic_topic_id =
                        database_->get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")[0].second;
                    metatraffic_topic = std::const_pointer_cast<database::Topic>(
                        std::static_pointer_cast<const database::Topic>(database_->get_entity(metatraffic_topic_id)));
                }

                // Create metatraffic reader and writer on the metatraffic topic.

                // Datawriter
                {
                    // Create the metatraffic datawriter
                    auto datawriter = std::make_shared<database::DataWriter>(
                        metatraffic_prefix + "DATAWRITER_" + to_string(participant_guid),
                        database::Qos({}),
                        to_string(participant_guid),
                        participant,
                        metatraffic_topic);

                    // Push it to the queue
                    database::EntityDiscoveryInfo datawriter_discovery_info;
                    datawriter_discovery_info.domain_id = domain_id_;
                    datawriter_discovery_info.entity = datawriter;
                    entity_queue_->push(timestamp, datawriter_discovery_info);
                }

                // Datareader
                {
                    // Create the metatraffic datawriter
                    auto datareader = std::make_shared<database::DataReader>(
                        metatraffic_prefix + "DATAREADER_" + to_string(participant_guid),
                        database::Qos({}),
                        to_string(participant_guid),
                        participant,
                        metatraffic_topic);

                    // Push it to the queue
                    database::EntityDiscoveryInfo datareader_discovery_info;
                    datareader_discovery_info.domain_id = domain_id_;
                    datareader_discovery_info.entity = datareader;
                    entity_queue_->push(timestamp, datareader_discovery_info);
                }
            }
        }
        else
        {
            // Start the data consumer again before reporting error
            data_queue_->start_consumer();
            throw BadParameter("Update or undiscover a participant which is not in the database");
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
    // Filter out our own statistics readers
    if (participant->guid().guidPrefix == info.info.guid().guidPrefix)
    {
        return;
    }

    // First stop the data queue until the new entity is created
    data_queue_->stop_consumer();

    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // The subscriber is already in the database
    try
    {
        EntityId datareader_id =
                database_->get_entity_by_guid(EntityKind::DATAREADER, to_string(info.info.guid())).second;

        // Build discovery info
        database::EntityDiscoveryInfo entity_discovery_info;
        entity_discovery_info.domain_id = domain_id_;
        entity_discovery_info.entity = std::const_pointer_cast<database::DataReader>(
            std::static_pointer_cast<const database::DataReader>(database_->get_entity(datareader_id)));

        switch (info.status)
        {
            case ReaderDiscoveryInfo::DISCOVERED_READER:
            {
                entity_discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
                break;
            }
            case ReaderDiscoveryInfo::CHANGED_QOS_READER:
                // TODO [ILG] : Process these messages and save the updated QoS and/or Locators
                entity_discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UPDATE;
                break;

            case ReaderDiscoveryInfo::REMOVED_READER:
            {
                entity_discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY;
                break;
            }
        }

        entity_queue_->push(timestamp, entity_discovery_info);
    }
    // The subscriber is not in the database
    catch (BadParameter&)
    {
        if (info.status == ReaderDiscoveryInfo::DISCOVERED_READER)
        {
            process_endpoint_discovery(info);
        }
        else
        {
            // Start the data consumer again before reporting error
            data_queue_->start_consumer();
            throw BadParameter("Update or undiscover a subscriber which is not in the database");
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
    static_cast<void>(participant);

    // Filter out other statistics writers
    if (is_statistics_builtin(info.info.guid().entityId))
    {
        return;
    }

    // First stop the data queue until the new entity is created
    data_queue_->stop_consumer();

    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // The publisher is already in the database
    try
    {
        EntityId datawriter_id =
                database_->get_entity_by_guid(EntityKind::DATAWRITER, to_string(info.info.guid())).second;

        // Build discovery info
        database::EntityDiscoveryInfo entity_discovery_info;
        entity_discovery_info.domain_id = domain_id_;
        entity_discovery_info.entity = std::const_pointer_cast<database::DataWriter>(
            std::static_pointer_cast<const database::DataWriter>(database_->get_entity(datawriter_id)));

        switch (info.status)
        {
            case WriterDiscoveryInfo::DISCOVERED_WRITER:
            {
                entity_discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
                break;
            }
            case WriterDiscoveryInfo::CHANGED_QOS_WRITER:
                // TODO [ILG] : Process these messages and save the updated QoS and/or Locators
                entity_discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UPDATE;
                break;

            case WriterDiscoveryInfo::REMOVED_WRITER:
            {
                entity_discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY;
                break;
            }
        }

        entity_queue_->push(timestamp, entity_discovery_info);
    }
    // The publisher is not in the database
    catch (BadParameter&)
    {
        if (info.status == WriterDiscoveryInfo::DISCOVERED_WRITER)
        {
            process_endpoint_discovery(info);
        }
        else
        {
            // Start the data consumer again before reporting error
            data_queue_->start_consumer();
            throw BadParameter("Update or undiscover a publichser which is not in the database");
        }
    }

    // Wait until the entity queue is processed and restart the data queue
    entity_queue_->flush();
    data_queue_->start_consumer();
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
