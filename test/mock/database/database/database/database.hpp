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

#include "gtest/gtest.h"
#include <gmock/gmock.h>

#include "fastdds/rtps/common/Guid.h"
#include "fastdds/rtps/common/Locator.h"

#include "database/entities.hpp"

#include <fastdds-statistics-backend/types/EntityId.hpp>

namespace eprosima {
namespace statistics_backend {
namespace database {

class Database
{
public:

    MOCK_METHOD1(insert, void(
            std::shared_ptr<Entity> entity));

    MOCK_METHOD2(insert, void(
            const EntityId& entity_id,
            const StatisticsSample& sample));

    MOCK_METHOD1(erase, void(
            EntityId& domain_id));

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

    MOCK_CONST_METHOD2(get_entities, const std::vector<std::shared_ptr<const Entity>>(
            EntityKind entity_kind,
            const eprosima::fastrtps::rtps::GUID_t& guid));

    MOCK_CONST_METHOD2(get_entities, const std::vector<std::shared_ptr<const Entity>>(
            EntityKind entity_kind,
            const std::string& name));

    template<typename T>
    const std::vector<EntityId> get_entities_with_attribute(
            EntityKind /*entity_kind*/,
            const T& /*attribute*/)
    {
        // Now construct the fake output
        EntityId id (1);
        std::vector<EntityId> result;
        result.push_back(id);
        return result;
    }

};

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_DATABASE_HPP_