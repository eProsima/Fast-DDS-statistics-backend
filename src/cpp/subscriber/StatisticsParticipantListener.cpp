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
#include "subscriber/QosSerializer.hpp"


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

void StatisticsParticipantListener::on_participant_discovery(
        DomainParticipant* /*participant*/,
        ParticipantDiscoveryInfo&& info)
{
    // First stop the data queue until the new entity is created
    data_queue_->stop_consumer();

    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Meaningful prefix for metatraffic entities
    const std::string metatraffic_prefix = "___EPROSIMA___METATRAFFIC___DOMAIN_" +
            std::to_string(domain_id_.value()) + "___";
    const std::string metatraffic_alias = "_metatraffic_";

    // Build the discovery info for the queue
    database::EntityDiscoveryInfo discovery_info(EntityKind::PARTICIPANT);
    discovery_info.domain_id = domain_id_;
    discovery_info.guid = info.info.m_guid;
    discovery_info.qos = subscriber::participant_proxy_data_to_backend_qos(info.info);

    discovery_info.address = get_address(info.info);
    discovery_info.participant_name = info.info.m_participantName.to_string();

    switch (info.status)
    {
        case ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT:
        {
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
            break;
        }
        case ParticipantDiscoveryInfo::CHANGED_QOS_PARTICIPANT:
        {
            // TODO [ILG] : Process these messages and save the updated QoS
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UPDATE;
            break;
        }
        case ParticipantDiscoveryInfo::REMOVED_PARTICIPANT:
        case ParticipantDiscoveryInfo::DROPPED_PARTICIPANT:
        {
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY;
            break;
        }
    }

    entity_queue_->push(timestamp, discovery_info);

    // Create metatraffic entities
    if (details::StatisticsBackendData::DiscoveryStatus::UPDATE != discovery_info.discovery_status)
    {
        // Create metatraffic endpoint and locator on the metatraffic topic.
        {
            // The endpoint QoS cannot be empty. We can use this to give a description to the user.
            database::Qos meta_traffic_qos = {
                {"description", "This is a virtual placeholder endpoint with no real counterpart"}};
            // Push it to the queue
            database::EntityDiscoveryInfo datawriter_discovery_info(EntityKind::DATAWRITER);

            for (auto dds_locator : info.info.metatraffic_locators.unicast)
            {
                datawriter_discovery_info.locators.add_unicast_locator(dds_locator);
            }
            for (auto dds_locator : info.info.metatraffic_locators.multicast)
            {
                datawriter_discovery_info.locators.add_multicast_locator(dds_locator);
            }
            for (auto dds_locator : info.info.default_locators.unicast)
            {
                datawriter_discovery_info.locators.add_unicast_locator(dds_locator);
            }
            for (auto dds_locator : info.info.default_locators.multicast)
            {
                datawriter_discovery_info.locators.add_multicast_locator(dds_locator);
            }

            datawriter_discovery_info.domain_id = domain_id_;
            datawriter_discovery_info.topic_name = metatraffic_prefix + "TOPIC";
            datawriter_discovery_info.type_name = metatraffic_prefix + "TYPE";
            datawriter_discovery_info.guid = info.info.m_guid;
            datawriter_discovery_info.qos = meta_traffic_qos;
            datawriter_discovery_info.alias = metatraffic_alias;
            datawriter_discovery_info.is_virtual_metatraffic = true;
            datawriter_discovery_info.discovery_status = discovery_info.discovery_status;
            entity_queue_->push(timestamp, datawriter_discovery_info);
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

    // Build the discovery info for the queue
    database::EntityDiscoveryInfo discovery_info(EntityKind::DATAREADER);
    discovery_info.domain_id = domain_id_;
    discovery_info.guid = info.info.guid();
    discovery_info.qos = subscriber::reader_proxy_data_to_backend_qos(info.info);

    discovery_info.topic_name = info.info.topicName().to_string();
    discovery_info.type_name = info.info.typeName().to_string();
    discovery_info.locators = info.info.remote_locators();

    switch (info.status)
    {
        case ReaderDiscoveryInfo::DISCOVERED_READER:
        {
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
            break;
        }
        case ReaderDiscoveryInfo::CHANGED_QOS_READER:
        {
            // TODO [ILG] : Process these messages and save the updated QoS
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UPDATE;
            break;
        }
        case ReaderDiscoveryInfo::REMOVED_READER:
        {
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY;
            break;
        }
    }

    entity_queue_->push(timestamp, discovery_info);

    // Wait until the entity queue is processed and restart the data queue
    entity_queue_->flush();
    data_queue_->start_consumer();
}

void StatisticsParticipantListener::on_publisher_discovery(
        DomainParticipant* participant,
        WriterDiscoveryInfo&& info)
{
    // Contrary to what it's done in on_subscriber_discovery, here we do not filter our own datawritters, as
    // deactivation of fastdds statistics module is enforced for the statistics backend, and hence none is ever created
    static_cast<void>(participant);

    // First stop the data queue until the new entity is created
    data_queue_->stop_consumer();

    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the discovery info for the queue
    database::EntityDiscoveryInfo discovery_info(EntityKind::DATAWRITER);
    discovery_info.domain_id = domain_id_;
    discovery_info.guid = info.info.guid();
    discovery_info.qos = subscriber::writer_proxy_data_to_backend_qos(info.info);

    discovery_info.topic_name = info.info.topicName().to_string();
    discovery_info.type_name = info.info.typeName().to_string();
    discovery_info.locators = info.info.remote_locators();

    switch (info.status)
    {
        case WriterDiscoveryInfo::DISCOVERED_WRITER:
        {
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
            break;
        }
        case WriterDiscoveryInfo::CHANGED_QOS_WRITER:
        {
            // TODO [ILG] : Process these messages and save the updated QoS
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UPDATE;
            break;
        }
        case WriterDiscoveryInfo::REMOVED_WRITER:
        {
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY;
            break;
        }
    }

    entity_queue_->push(timestamp, discovery_info);

    // Wait until the entity queue is processed and restart the data queue
    entity_queue_->flush();
    data_queue_->start_consumer();
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
