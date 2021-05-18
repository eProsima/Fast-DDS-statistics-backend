// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file StatisticsBackend.cpp
 */

#include <fastdds-statistics-backend/StatisticsBackend.hpp>
#include <fastdds-statistics-backend/types/JSONTags.h>

#include "database/database.hpp"
#include "Monitor.hpp"

namespace eprosima {
namespace statistics_backend {

using namespace eprosima::statistics_backend::database;

Database* StatisticsBackend::database_ = new database::Database();
std::map<EntityId, Monitor*> StatisticsBackend::monitors_;

PhysicalListener* StatisticsBackend::physical_listener_;
CallbackMask StatisticsBackend::physical_callback_mask_;
DomainListener::Status StatisticsBackend::host_status_;
DomainListener::Status StatisticsBackend::user_status_;
DomainListener::Status StatisticsBackend::process_status_;
DomainListener::Status StatisticsBackend::locator_status_;


void StatisticsBackend::set_physical_listener(
        PhysicalListener* listener,
        CallbackMask callback_mask,
        DataKindMask data_mask)
{
    static_cast<void>(listener);
    static_cast<void>(callback_mask);
    static_cast<void>(data_mask);
}

EntityId StatisticsBackend::init_monitor(
        DomainId domain,
        DomainListener* domain_listener,
        CallbackMask callback_mask,
        DataKindMask data_mask)
{
    static_cast<void>(domain);
    static_cast<void>(domain_listener);
    static_cast<void>(callback_mask);
    static_cast<void>(data_mask);
    return EntityId();
}

EntityId StatisticsBackend::init_monitor(
        std::string discovery_server_locators,
        DomainListener* domain_listener,
        CallbackMask callback_mask,
        DataKindMask data_mask)
{
    static_cast<void>(discovery_server_locators);
    static_cast<void>(domain_listener);
    static_cast<void>(callback_mask);
    static_cast<void>(data_mask);
    return EntityId();
}

void StatisticsBackend::restart_monitor(
        EntityId monitor_id)
{
    static_cast<void>(monitor_id);
}

void StatisticsBackend::stop_monitor(
        EntityId monitor_id)
{
    static_cast<void>(monitor_id);
}

void StatisticsBackend::clear_monitor(
        EntityId monitor_id)
{
    static_cast<void>(monitor_id);
}

void StatisticsBackend::set_domain_listener(
        EntityId monitor_id,
        DomainListener* listener,
        CallbackMask callback_mask,
        DataKindMask data_mask)
{
    static_cast<void>(monitor_id);
    static_cast<void>(listener);
    static_cast<void>(callback_mask);
    static_cast<void>(data_mask);
}

std::vector<EntityId> StatisticsBackend::get_entities(
        EntityKind entity_type,
        EntityId entity_id)
{
    return database_->get_entity_ids(entity_type, entity_id);
}

bool StatisticsBackend::is_active(
        EntityId entity_id)
{
    static_cast<void>(entity_id);
    return false;
}

EntityKind StatisticsBackend::get_type(
        EntityId entity_id)
{
    return database_->get_entity_kind(entity_id);
}

Info StatisticsBackend::get_info(
        EntityId entity_id)
{
    Info info = Info::object();

    std::shared_ptr<const Entity> entity = database_->get_entity(entity_id);

    info[ID_INFO_TAG] = entity_id.value();
    info[KIND_INFO_TAG] = entity_kind_str[(int)entity->kind];
    info[NAME_INFO_TAG] = entity->name;

    switch (entity->kind)
    {
        case EntityKind::PROCESS:
        {
            std::shared_ptr<const Process> process =
                    std::dynamic_pointer_cast<const Process>(entity);
            info[PID_INFO_TAG] = process->pid;
            break;
        }
        case EntityKind::TOPIC:
        {
            std::shared_ptr<const Topic> topic =
                    std::dynamic_pointer_cast<const Topic>(entity);
            info[DATA_TYPE_INFO_TAG] = topic->data_type;
            break;
        }
        case EntityKind::PARTICIPANT:
        case EntityKind::DATAWRITER:
        case EntityKind::DATAREADER:
        {
            std::shared_ptr<const DDSEntity> dds_entity =
                    std::dynamic_pointer_cast<const DDSEntity>(entity);
            info[GUID_INFO_TAG] = dds_entity->guid;
            info[QOS_INFO_TAG] = dds_entity->qos;
            break;
        }
        default:
        {
            break;
        }
    }

    return info;
}

std::vector<StatisticsData> StatisticsBackend::get_data(
        DataKind data_type,
        const std::vector<EntityId> entity_ids_source,
        const std::vector<EntityId> entity_ids_target,
        uint16_t bins,
        Timestamp t_from,
        Timestamp t_to,
        StatisticKind statistic)
{
    static_cast<void>(data_type);
    static_cast<void>(entity_ids_source);
    static_cast<void>(entity_ids_target);
    static_cast<void>(bins);
    static_cast<void>(t_from);
    static_cast<void>(t_to);
    static_cast<void>(statistic);
    return std::vector<StatisticsData>();
}

std::vector<StatisticsData> StatisticsBackend::get_data(
        DataKind data_type,
        const std::vector<EntityId> entity_ids,
        uint16_t bins,
        Timestamp t_from,
        Timestamp t_to,
        StatisticKind statistic)
{
    static_cast<void>(data_type);
    static_cast<void>(entity_ids);
    static_cast<void>(bins);
    static_cast<void>(t_from);
    static_cast<void>(t_to);
    static_cast<void>(statistic);
    return std::vector<StatisticsData>();
}

std::vector<StatisticsData> StatisticsBackend::get_data(
        DataKind data_type,
        const std::vector<EntityId> entity_ids_source,
        const std::vector<EntityId> entity_ids_target,
        uint16_t bins,
        StatisticKind statistic)
{
    return get_data(
        data_type,
        entity_ids_source,
        entity_ids_target,
        bins,
        Timestamp(),
        std::chrono::system_clock::now(),
        statistic);
}

std::vector<StatisticsData> StatisticsBackend::get_data(
        DataKind data_type,
        const std::vector<EntityId> entity_ids,
        uint16_t bins,
        StatisticKind statistic)
{
    return get_data(
        data_type,
        entity_ids,
        bins,
        Timestamp(),
        std::chrono::system_clock::now(),
        statistic);
}

Graph StatisticsBackend::get_graph()
{
    return Graph();
}

std::vector<std::pair<EntityKind, EntityKind>> StatisticsBackend::get_data_supported_entity_kinds(
        DataKind data_kind)
{
    static std::map<DataKind, std::vector<std::pair<EntityKind, EntityKind>>> data_to_entity_map =
    {
        {DataKind::INVALID, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::INVALID, EntityKind::INVALID)})},

        {DataKind::FASTDDS_LATENCY, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::DATAREADER)})},

        {DataKind::NETWORK_LATENCY, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::LOCATOR, EntityKind::LOCATOR)})},

        {DataKind::PUBLICATION_THROUGHPUT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::INVALID)})},

        {DataKind::SUBSCRIPTION_THROUGHPUT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAREADER, EntityKind::INVALID)})},

        {DataKind::RTPS_PACKETS_SENT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::LOCATOR)})},

        {DataKind::RTPS_BYTES_SENT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::LOCATOR)})},

        {DataKind::RTPS_PACKETS_LOST, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::LOCATOR)})},

        {DataKind::RTPS_BYTES_LOST, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::LOCATOR)})},

        {DataKind::RESENT_DATA, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::INVALID)})},

        {DataKind::HEARTBEAT_COUNT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::INVALID)})},

        {DataKind::ACKNACK_COUNT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAREADER, EntityKind::INVALID)})},

        {DataKind::NACKFRAG_COUNT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAREADER, EntityKind::INVALID)})},

        {DataKind::GAP_COUNT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::INVALID)})},

        {DataKind::DATA_COUNT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::INVALID)})},

        {DataKind::PDP_PACKETS, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::INVALID)})},

        {DataKind::EDP_PACKETS, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::INVALID)})},

        {DataKind::DISCOVERY_TIME, std::vector<std::pair<EntityKind, EntityKind>>(
                    {
                        std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::PARTICIPANT),
                        std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::DATAWRITER),
                        std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::DATAREADER)})},

        {DataKind::SAMPLE_DATAS, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::INVALID)})}
    };

    return data_to_entity_map[data_kind];
}

void StatisticsBackend::on_data_available(
        EntityId domain_id,
        EntityId entity_id,
        DataKind data_kind)
{
    auto monitor = monitors_.find(domain_id);
    assert(monitor != monitors_.end());
    if (monitor->second->domain_listener == nullptr ||
            !monitor->second->domain_callback_mask.is_set(CallbackKind::ON_DATA_AVAILABLE))
    {
        // No user listener or mask deactivated
        return;
    }

    if (!monitor->second->data_mask.is_set(data_kind))
    {
        // Data mask deactivated
        return;
    }

    monitor->second->domain_listener->on_data_available(domain_id, entity_id, data_kind);
}

void StatisticsBackend::on_domain_entity_discovery(
        EntityId domain_id,
        EntityId entity_id,
        EntityKind entity_kind)
{
    auto monitor = monitors_.find(domain_id);
    assert(monitor != monitors_.end());
    if (monitor->second->domain_listener == nullptr)
    {
        // No user listener
        return;
    }

    switch(entity_kind)
    {
        case EntityKind::PARTICIPANT:
        {
            if (!monitor->second->domain_callback_mask.is_set(CallbackKind::ON_PARTICIPANT_DISCOVERY))
            {
                // mask deactivated
                return;
            }

            // Update the status before calling the
            monitor->second->participant_status_.on_instance_discovered();
            monitor->second->domain_listener->on_participant_discovery(domain_id, entity_id, monitor->second->participant_status_);
            monitor->second->participant_status_.on_status_read();
            break;
        }
        case EntityKind::TOPIC:
        {
            if (!monitor->second->domain_callback_mask.is_set(CallbackKind::ON_TOPIC_DISCOVERY))
            {
                // mask deactivated
                return;
            }

            monitor->second->topic_status_.on_instance_discovered();
            monitor->second->domain_listener->on_topic_discovery(domain_id, entity_id, monitor->second->topic_status_);
            monitor->second->topic_status_.on_status_read();
            break;
        }
        case EntityKind::DATAWRITER:
        {
            if (!monitor->second->domain_callback_mask.is_set(CallbackKind::ON_DATAWRITER_DISCOVERY))
            {
                // mask deactivated
                return;
            }

            monitor->second->datawriter_status_.on_instance_discovered();
            monitor->second->domain_listener->on_datawriter_discovery(domain_id, entity_id, monitor->second->datawriter_status_);
            monitor->second->datawriter_status_.on_status_read();
            break;
        }
        case EntityKind::DATAREADER:
        {
            if (!monitor->second->domain_callback_mask.is_set(CallbackKind::ON_DATAREADER_DISCOVERY))
            {
                // mask deactivated
                return;
            }

            monitor->second->datareader_status_.on_instance_discovered();
            monitor->second->domain_listener->on_datareader_discovery(domain_id, entity_id, monitor->second->datareader_status_);
            monitor->second->datareader_status_.on_status_read();
            break;
        }
        default:
        {
            throw Error("wrong entity_kind");
        }
    }
}

void StatisticsBackend::on_physical_entity_discovery(
        EntityId participant_id,
        EntityId entity_id,
        EntityKind entity_kind)
{
    if (physical_listener_ == nullptr)
    {
        // No user listener
        return;
    }

    switch(entity_kind)
    {
        case EntityKind::HOST:
        {
            if (!physical_callback_mask_.is_set(CallbackKind::ON_HOST_DISCOVERY))
            {
                // mask deactivated
                return;
            }

            host_status_.on_instance_discovered();
            physical_listener_->on_host_discovery(participant_id, entity_id, host_status_);
            host_status_.on_status_read();
            break;
        }
        case EntityKind::USER:
        {
            if (!physical_callback_mask_.is_set(CallbackKind::ON_USER_DISCOVERY))
            {
                // mask deactivated
                return;
            }

            user_status_.on_instance_discovered();
            physical_listener_->on_user_discovery(participant_id, entity_id, user_status_);
            user_status_.on_status_read();
            break;
        }
        case EntityKind::PROCESS:
        {
            if (!physical_callback_mask_.is_set(CallbackKind::ON_PROCESS_DISCOVERY))
            {
                // mask deactivated
                return;
            }

            process_status_.on_instance_discovered();
            physical_listener_->on_process_discovery(participant_id, entity_id, process_status_);
            process_status_.on_status_read();
            break;
        }
        case EntityKind::LOCATOR:
        {
            if (!physical_callback_mask_.is_set(CallbackKind::ON_LOCATOR_DISCOVERY))
            {
                // mask deactivated
                return;
            }

            locator_status_.on_instance_discovered();
            physical_listener_->on_locator_discovery(participant_id, entity_id, locator_status_);
            locator_status_.on_status_read();
            break;
        }
        default:
        {
            throw Error("wrong entity_kind");
        }
    }
}

} // namespace statistics_backend
} // namespace eprosima
