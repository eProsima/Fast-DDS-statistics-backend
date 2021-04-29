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

#include "database/database.hpp"

namespace eprosima {
namespace statistics_backend {

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
    static_cast<void>(entity_type);
    static_cast<void>(entity_id);
    return std::vector<EntityId>();
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
    static_cast<void>(entity_id);
    return EntityKind::HOST;
}

Info StatisticsBackend::get_info(
        EntityId entity_id)
{
    static_cast<void>(entity_id);
    return Info();
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

} // namespace statistics_backend
} // namespace eprosima
