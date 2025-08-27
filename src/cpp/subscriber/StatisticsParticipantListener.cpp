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
#include <utility>

#include "subscriber/StatisticsParticipantListener.hpp"
#include "subscriber/ProxyDiscoveryInfo.hpp"

#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/utils.hpp>
#include <fastdds/rtps/common/EntityId_t.hpp>
#include <fastdds/rtps/writer/WriterDiscoveryStatus.hpp>

#include "database/database_queue.hpp"
#include "subscriber/QosSerializer.hpp"


namespace eprosima {
namespace statistics_backend {
namespace subscriber {

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

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
        const EntityId_t& entity_id)
{
    return 0x60 == (0xE0 & entity_id.value[3]);
}

StatisticsParticipantListener::StatisticsParticipantListener(
        EntityId domain_id,
        database::Database* database,
        database::DatabaseEntityQueue* entity_queue,
        database::DatabaseDataQueue<eprosima::fastdds::statistics::Data>* data_queue,
        database::DatabaseDataQueue<database::ExtendedMonitorServiceStatusData>* monitor_service_data_queue)
noexcept
    : DomainParticipantListener()
    , domain_id_(domain_id)
    , database_(database)
    , entity_queue_(entity_queue)
    , data_queue_(data_queue)
    , monitor_service_status_data_queue_(monitor_service_data_queue)
{
}

void StatisticsParticipantListener::on_participant_discovery(
        DomainParticipant* /*participant*/,
        ParticipantDiscoveryStatus reason,
        const ParticipantBuiltinTopicData& info,
        bool& /*should_be_ignored*/)
{
    // First stop the data queues until the new entity is created
    data_queue_->stop_consumer();
    monitor_service_status_data_queue_->stop_consumer();

    std::chrono::system_clock::time_point timestamp = now();

    EntityDiscoveryInfo discovery_info = get_discovery_info(domain_id_, info, reason, DiscoverySource::DISCOVERY);
    entity_queue_->push(timestamp, discovery_info);

    // Create metatraffic entities
    if (details::StatisticsBackendData::DiscoveryStatus::UPDATE != discovery_info.discovery_status)
    {
        // Meaningful prefix for metatraffic entities
        const std::string metatraffic_prefix = "___EPROSIMA___METATRAFFIC___DOMAIN_" +
                std::to_string(domain_id_.value()) +
                "___";
        const std::string metatraffic_alias = "_metatraffic_";
        // Create metatraffic endpoint and locator on the metatraffic topic.
        {
            // The endpoint QoS cannot be empty. We can use this to give a description to the user.
            database::Qos meta_traffic_qos = {
                {"description", "This is a virtual placeholder endpoint with no real counterpart"}};
            // Push it to the queue
            database::EntityDiscoveryInfo datawriter_discovery_info(EntityKind::DATAWRITER);

            for (auto dds_locator : info.metatraffic_locators.unicast)
            {
                datawriter_discovery_info.locators.add_unicast_locator(dds_locator);
            }
            for (auto dds_locator : info.metatraffic_locators.multicast)
            {
                datawriter_discovery_info.locators.add_multicast_locator(dds_locator);
            }
            for (auto dds_locator : info.default_locators.unicast)
            {
                datawriter_discovery_info.locators.add_unicast_locator(dds_locator);
            }
            for (auto dds_locator : info.default_locators.multicast)
            {
                datawriter_discovery_info.locators.add_multicast_locator(dds_locator);
            }

            datawriter_discovery_info.domain_id = domain_id_;
            datawriter_discovery_info.topic_name = metatraffic_prefix + "TOPIC";
            datawriter_discovery_info.type_name = metatraffic_prefix + "TYPE";
            datawriter_discovery_info.guid = info.guid;
            datawriter_discovery_info.qos = meta_traffic_qos;
            datawriter_discovery_info.alias = metatraffic_alias;
            datawriter_discovery_info.is_virtual_metatraffic = true;
            datawriter_discovery_info.entity_status = StatusLevel::OK_STATUS;
            datawriter_discovery_info.discovery_status = discovery_info.discovery_status;
            datawriter_discovery_info.discovery_source = DiscoverySource::DISCOVERY;
            entity_queue_->push(timestamp, datawriter_discovery_info);
        }
    }

    // Wait until the entity queues is processed and restart the data queue
    entity_queue_->flush();
    data_queue_->start_consumer();
    monitor_service_status_data_queue_->start_consumer();
}

void StatisticsParticipantListener::on_data_reader_discovery(
        DomainParticipant* participant,
        ReaderDiscoveryStatus reason,
        const SubscriptionBuiltinTopicData& info,
        bool& /*should_be_ignored*/)
{
    // Filter out our own statistics readers
    if (participant->guid().guidPrefix == info.guid.guidPrefix)
    {
        return;
    }

    std::chrono::system_clock::time_point timestamp = now();
    // Build the discovery info for the queue
    EntityDiscoveryInfo discovery_info = get_discovery_info(domain_id_, info, reason, DiscoverySource::DISCOVERY);
    entity_queue_->push(timestamp, discovery_info);

    // Wait until the entity queue is processed and restart the data queues
    entity_queue_->flush();
    data_queue_->start_consumer();
    monitor_service_status_data_queue_->start_consumer();
}

void StatisticsParticipantListener::on_data_writer_discovery(
        DomainParticipant* participant,
        WriterDiscoveryStatus reason,
        const PublicationBuiltinTopicData& info,
        bool& /*should_be_ignored*/)
{
    // Contrary to what it's done in on_subscriber_discovery, here we do not filter our own datawritters, as
    // deactivation of fastdds statistics module is enforced for the statistics backend, and hence none is ever created
    static_cast<void>(participant);

    // First stop the data queues until the new entity is created
    data_queue_->stop_consumer();
    monitor_service_status_data_queue_->stop_consumer();

    std::chrono::system_clock::time_point timestamp = now();
    // Build the discovery info for the queue
    EntityDiscoveryInfo discovery_info = get_discovery_info(domain_id_, info, reason, DiscoverySource::DISCOVERY);
    entity_queue_->push(timestamp, discovery_info);

    // Wait until the entity queue is processed and restart the data queues
    entity_queue_->flush();
    data_queue_->start_consumer();
    monitor_service_status_data_queue_->start_consumer();
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
