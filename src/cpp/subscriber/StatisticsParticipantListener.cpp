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
#include "UserDataContext.hpp"

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
        database::DatabaseDataQueue<database::ExtendedMonitorServiceStatusData>* monitor_service_data_queue,
        UserDataContext* ctx,
        GuidPrefix_t spy_guid_prefix)
noexcept
    : DomainParticipantListener()
    , domain_id_(domain_id)
    , database_(database)
    , entity_queue_(entity_queue)
    , data_queue_(data_queue)
    , monitor_service_status_data_queue_(monitor_service_data_queue)
    , ctx_(ctx)
    , spy_guid_prefix_(spy_guid_prefix)
{
}

void StatisticsParticipantListener::on_participant_discovery(
        DomainParticipant* /*participant*/,
        ParticipantDiscoveryStatus reason,
        const ParticipantBuiltinTopicData& info,
        bool& should_be_ignored)
{
    // Check if this is our spy participant - if so, ignore it
    if (is_spy(info.guid))
    {
        should_be_ignored = true;
        EPROSIMA_LOG_INFO(STATISTICS_PARTICIPANT_LISTENER,
                "Ignoring spy participant: " << info.guid);
        return;
    }
    // First stop the data queues until the new entity is created
    data_queue_->stop_consumer();
    monitor_service_status_data_queue_->stop_consumer();

    std::chrono::system_clock::time_point timestamp = now();

    EntityDiscoveryInfo discovery_info = get_discovery_info(domain_id_, info, reason, DiscoverySource::DISCOVERY);
    entity_queue_->push(timestamp, discovery_info);

    // Create metatraffic entities
    if (details::StatisticsBackendData::DiscoveryStatus::UPDATE != discovery_info.discovery_status)
    {
        EntityDiscoveryInfo datawriter_discovery_info = get_metatraffic_discovery_info(domain_id_, info,
                        discovery_info.discovery_status,
                        DiscoverySource::DISCOVERY);
        entity_queue_->push(timestamp, datawriter_discovery_info);
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
        bool& should_be_ignored)
{
    // Filter out our own statistics readers
    if (participant->guid().guidPrefix == info.guid.guidPrefix)
    {
        return;
    }

    // Check if this reader belongs to the spy participant
    if (is_spy(info.guid))
    {
        should_be_ignored = true;
        EPROSIMA_LOG_INFO(STATISTICS_PARTICIPANT_LISTENER,
                "Ignoring spy reader: " << info.guid);
        return;
    }

    std::chrono::system_clock::time_point timestamp = now();

    // Update user data context with the discovery info
    update_user_data_context(reason, info);

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
        bool& should_be_ignored)
{
    // Filter out our own statistics writers
    if (participant->guid().guidPrefix == info.guid.guidPrefix)
    {
        return;
    }

    // Check if this writer belongs to the spy participant
    if (is_spy(info.guid))
    {
        should_be_ignored = true;
        EPROSIMA_LOG_INFO(STATISTICS_PARTICIPANT_LISTENER,
                "Ignoring spy writer: " << info.guid);
        return;
    }
    // Contrary to what it's done in on_subscriber_discovery, here we do not filter our own datawritters, as
    // deactivation of fastdds statistics module is enforced for the statistics backend, and hence none is ever created
    static_cast<void>(participant);

    // First stop the data queues until the new entity is created
    data_queue_->stop_consumer();
    monitor_service_status_data_queue_->stop_consumer();

    std::chrono::system_clock::time_point timestamp = now();

    // Update user data context with the discovery info
    update_user_data_context(reason, info);

    // Build the discovery info for the queue
    EntityDiscoveryInfo discovery_info = get_discovery_info(domain_id_, info, reason, DiscoverySource::DISCOVERY);
    entity_queue_->push(timestamp, discovery_info);

    // Wait until the entity queue is processed and restart the data queues
    entity_queue_->flush();
    data_queue_->start_consumer();
    monitor_service_status_data_queue_->start_consumer();
}

void StatisticsParticipantListener::update_user_data_context(
        WriterDiscoveryStatus reason,
        const PublicationBuiltinTopicData& info)
{
    if (WriterDiscoveryStatus::DISCOVERED_WRITER == reason && info.type_information.assigned() == true)
    {
        xtypes::TypeObject remote_type_object;
        if (RETCODE_OK != DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                    info.type_information.type_information.complete().typeid_with_size().type_id(),
                    remote_type_object))
        {
            EPROSIMA_LOG_ERROR(STATISTICS_PARTICIPANT_LISTENER,
                    "Error getting type object for type " << info.type_name);
            return;
        }

        // Build remotely discovered type
        DynamicType::_ref_type remote_type = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
            remote_type_object)->build();

        // Add the topic to the discovered topics if not already present
        ctx_->register_user_data_topic(info.topic_name.to_string(), remote_type);
    }
}

void StatisticsParticipantListener::update_user_data_context(
        ReaderDiscoveryStatus reason,
        const SubscriptionBuiltinTopicData& info)
{
    if (ReaderDiscoveryStatus::DISCOVERED_READER == reason && info.type_information.assigned() == true)
    {
        xtypes::TypeObject remote_type_object;
        if (RETCODE_OK != DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                    info.type_information.type_information.complete().typeid_with_size().type_id(),
                    remote_type_object))
        {
            EPROSIMA_LOG_ERROR(STATISTICS_PARTICIPANT_LISTENER,
                    "Error getting type object for type " << info.type_name);
            return;
        }

        // Build remotely discovered type
        DynamicType::_ref_type remote_type = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
            remote_type_object)->build();

        // Add the topic to the discovered topics if not already present
        ctx_->register_user_data_topic(info.topic_name.to_string(), remote_type);
    }
}

bool StatisticsParticipantListener::is_spy(
        const fastdds::rtps::GUID_t& guid)
{
    return guid.guidPrefix == spy_guid_prefix_;
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
