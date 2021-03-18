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

namespace eprosima {
namespace statistics_backend {

void StatisticsBackend::set_physical_listener(
        PhysicalListener* listener,
        CallbackMask callback_mask)
{
    static_cast<void>(listener);
    static_cast<void>(callback_mask);
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
    return "";
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
    return "";
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
        EntityId entity_id,
        EntityKind entity_type)
{
    static_cast<void>(entity_id);
    static_cast<void>(entity_type);
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

Qos StatisticsBackend::get_qos(
            EntityId entity_id)
{
    static_cast<void>(entity_id);
    return Qos();
}

std::string StatisticsBackend::get_name(
            EntityId entity_id)
{
    static_cast<void>(entity_id);
    return "";
}

std::vector<StatisticsData> StatisticsBackend::get_data(
            DataKind data_type,
            EntityId entity_id_source,
            EntityId entity_id_target,
            uint16_t bins,
            Timestamp t_from,
            Timestamp t_to,
            StatisticKind statistic)
{
    static_cast<void>(data_type);
    static_cast<void>(entity_id_source);
    static_cast<void>(entity_id_target);
    static_cast<void>(bins);
    static_cast<void>(t_from);
    static_cast<void>(t_to);
    static_cast<void>(statistic);
    return std::vector<StatisticsData>();
}

std::vector<StatisticsData> StatisticsBackend::get_data(
            DataKind data_type,
            EntityId entity_id,
            uint16_t bins,
            Timestamp t_from,
            Timestamp t_to,
            StatisticKind statistic)
{
    static_cast<void>(data_type);
    static_cast<void>(entity_id);
    static_cast<void>(bins);
    static_cast<void>(t_from);
    static_cast<void>(t_to);
    static_cast<void>(statistic);
    return std::vector<StatisticsData>();
}

Graph StatisticsBackend::get_graph()
{
    return Graph();
}

} // namespace statistics_backend
} // namespace eprosima
