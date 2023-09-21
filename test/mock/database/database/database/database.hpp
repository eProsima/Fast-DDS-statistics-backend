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

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_DATABASE_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_DATABASE_HPP_

#include <gtest_aux.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "fastdds/rtps/common/Guid.h"
#include "fastdds/rtps/common/Locator.h"

#include "database/entities.hpp"

#include <fastdds_statistics_backend/types/EntityId.hpp>

namespace eprosima {
namespace statistics_backend {

/**
 * std::chrono::duration uses sfinae to prevent information losses on construction. Some OS system_clock implemenations
 * are not accurate enough to handle a nanosecond resolution. For example on windows the system_clock resolution is 100
 * nanoseconds. Truncation must be enforced in those cases.
 * As reference on a linux distro over an i5 processor the actual system_clock tested resolution is in average 30 ns
 * with a standard deviation of 40 ns. The worse case scenario is about 70 ns ~ 100 ns used by windows.
 *
 * @param nanosecs from posix epoch 1970-01-01
 * @return argument date on local system_clock time point
 */
inline std::chrono::system_clock::time_point nanoseconds_to_systemclock(
        uint64_t nanosecs)
{
#if defined(_WIN32)
    auto span_since_epoch = std::chrono::duration<uint64_t, std::ratio<1, 10000000>>(nanosecs / 100);
#else
    auto span_since_epoch = std::chrono::nanoseconds(nanosecs);
#endif // if defined(_WIN32)
    return std::chrono::system_clock::time_point(span_since_epoch);
}

namespace database {

class Database
{
public:

    MOCK_METHOD1(insert, EntityId(
                std::shared_ptr<Entity> entity));

    MOCK_METHOD3(insert, void(
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample));

    MOCK_METHOD4(insert, void(
                const EntityId& domain_id,
                const EntityId& entity_id,
                const EntityKind& entity_kind,
                const MonitorServiceSample& sample));

    MOCK_METHOD1(erase, void(
                EntityId & domain_id));

    MOCK_METHOD5(select, std::vector<const StatisticsSample*>(
                DataKind data_type,
                EntityId entity_id_source,
                EntityId entity_id_target,
                Timestamp t_from,
                Timestamp t_to));

    MOCK_METHOD4(select, std::vector<const StatisticsSample*>(
                DataKind data_type,
                EntityId entity_id,
                Timestamp t_from,
                Timestamp t_to));

    MOCK_CONST_METHOD1(get_entity, const std::shared_ptr<const Entity>(
                const EntityId& entity_id));

    MOCK_CONST_METHOD2(get_entities, const std::vector<std::shared_ptr<const Entity>>(
                EntityKind entity_kind,
                const EntityId& entity_id));

    MOCK_CONST_METHOD2(get_entity_ids, std::vector<EntityId>(
                EntityKind entity_kind,
                const EntityId& entity_id));

    MOCK_CONST_METHOD2(get_entity_by_guid, std::pair<EntityId, EntityId>(
                EntityKind entity_kind,
                const std::string& guid));
    
    MOCK_CONST_METHOD1(get_entity_kind_by_guid, EntityKind(
                const eprosima::fastdds::statistics::detail::GUID_s& guid));

    MOCK_CONST_METHOD2(get_entities_by_name, std::vector<std::pair<EntityId, EntityId>>(
                EntityKind entity_kind,
                const std::string& name));

    MOCK_METHOD2(change_entity_status, void(
                const EntityId& entity_id,
                bool active));

    // Rely this method to the mock of get_entity
    EntityKind get_entity_kind(
            EntityId entity_id) const
    {
        return get_entity(entity_id).get()->kind;
    }

    MOCK_CONST_METHOD2(link_participant_with_process, void(
                const EntityId& participant_id,
                const EntityId& process_id));

    EntityId generate_entity_id() noexcept
    {
        return EntityId(next_id_++);
    }

    void set_next_entity_id(
            int64_t id) noexcept
    {
        next_id_ = id;
    }

    void load_database(
            const DatabaseDump& /*dump*/)
    {
    }

    MOCK_METHOD2(init_domain_view_graph, void(
                const std::string& domain_name,
                const EntityId& domain_entity_id));

    MOCK_METHOD5(update_participant_in_graph, bool(
                const EntityId& domain_entity_id,
                const EntityId& host_entity_id,
                const EntityId& user_entity_id,
                const EntityId& process_entity_id,
                const EntityId& participant_entity_id));

    MOCK_METHOD4(update_endpoint_in_graph, bool(
                const EntityId& domain_entity_id,
                const EntityId& participant_entity_id,
                const EntityId& topic_entity_id,
                const EntityId& endpoint_entity_id));

    MOCK_METHOD1(regenerate_domain_graph, void(
                const EntityId& domain_entity_id));

    MOCK_CONST_METHOD1(get_domain_view_graph, Graph(
                const EntityId& domain_id));

    MOCK_CONST_METHOD2(update_graph_on_updated_entity, void(
                const EntityId& domain_id,
                const EntityId& entity_id));

    MOCK_CONST_METHOD2(update_entity_status, bool(
                const EntityId& entity_id,
                const EntityKind& entity_kind));

    int64_t next_id_{0};
};

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_DATABASE_HPP_
