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
 * @file utils.cpp
 */

#include <random>
#include <sstream>

#include <fastdds-statistics-backend/types/types.hpp>

#include "utils.hpp"

namespace eprosima {
namespace statistics_backend {

std::vector<EntityId> get_ids(
    const std::map<EntityId, const Entity*> map)
{
    return keys<EntityId, const Entity*>(map);
}

std::vector<EntityId> get_entities_related(
    const std::map<EntityId, const Entity*> map,
    const EntityKind entity_type)
{
    std::vector<EntityId> result;
    for (auto entity : map)
    {
        auto entities = entity.second->get_entities(entity_type);
        result.insert(result.end(), entities.begin(), entities.end());
    }
    return result;
}

std::string entityId_to_string(EntityId id)
{
    std::ostringstream oss;
    oss << id;
    return oss.str();
}

std::vector<EntityId> get_entities_kind(
    const std::map<EntityId, const Entity*> map,
    const EntityKind entity_type)
{
    std::vector<EntityId> result;
    for (auto entity : map)
    {
        if (entity.second->kind() == entity_type)
        {
            result.push_back(entity.first);
        }
    }
    return result;
}

Info random_qos(EntityId seed)
{
    // Seed rand number
    srand(seed.value());

    Info qos;

    // Add durability
    qos["durability"] = Info();
    int aux_ran = rand() % 4;

    if (aux_ran == 0)
    {
        qos["durability"]["kind"] = "VOLATILE_DURABILITY_QOS";
    }
    else if (aux_ran == 1)
    {
        qos["durability"]["kind"] = "TRANSIENT_LOCAL_DURABILITY_QOS";
    }
    else if (aux_ran == 2)
    {
        qos["durability"]["kind"] = "TRANSIENT_DURABILITY_QOS";
    }
    else
    {
        qos["durability"]["kind"] = "PERSISTENT_DURABILITY_QOS";
    }

    // Add Reliability
    qos["reliability"] = Info();
    int aux_ran = rand() % 2;

    if (aux_ran == 0)
    {
        qos["reliability"]["kind"] = "BEST_EFFORT_RELIABILITY_QOS";
    }
    else if (aux_ran == 1)
    {
        qos["reliability"]["kind"] = "RELIABLE_RELIABILITY_QOS";
        qos["reliability"]["max_blocking_time"] = Info();
        qos["reliability"]["max_blocking_time"]["nanoseconds"] = rand() % 1000000000;
        qos["reliability"]["max_blocking_time"]["seconds"] = rand() % 100;
    }

    return qos;

    // Info json_obj = R"({
    //     "data_sharing":
    //     {
    //         "domain_ids":
    //         [
    //             0
    //         ],
    //         "kind": "AUTO",
    //         "max_domains": 1,
    //         "shm_directory": "/dev/shm"
    //     },
    //     "deadline":
    //     {
    //         "period":
    //         {
    //             "nanoseconds": 50,
    //             "seconds": 10
    //         }
    //     },
    //     "destination_order":
    //     {
    //         "kind": "BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS"
    //     },
    //     "disable_positive_acks":
    //     {
    //         "duration":
    //         {
    //             "nanoseconds": 100,
    //             "seconds": 0
    //         },
    //         "enabled": true
    //     },
    //     "durability":
    //     {
    //         "kind": "VOLATILE_DURABILITY_QOS"
    //     },
    //     "durability_service":
    //     {
    //         "history_depth": "1",
    //         "history_kind": "KEEP_LAST_HISTORY_QOS",
    //         "max_instances": "30",
    //         "max_samples": "3000",
    //         "max_samples_per_instance": "100",
    //         "service_cleanup_delay":
    //         {
    //             "nanoseconds": 0,
    //             "seconds": 5
    //         }
    //     },
    //     "group_data": "9d46781410ff",
    //     "latency_budget":
    //     {
    //         "duration":
    //         {
    //             "nanoseconds": 50,
    //             "seconds": 10
    //         }
    //     },
    //     "lifespan":
    //     {
    //         "duration":
    //         {
    //             "nanoseconds": 0,
    //             "seconds": 10000
    //         }
    //     },
    //     "liveliness":
    //     {
    //         "announcement_period":
    //         {
    //             "nanoseconds": 0,
    //             "seconds": 3
    //         },
    //         "lease_duration":
    //         {
    //             "nanoseconds": 0,
    //             "seconds": 10
    //         },
    //         "kind": "AUTOMATIC_LIVELINESS_QOS"
    //     },
    //     "ownership":
    //     {
    //         "kind": "SHARED_OWNERSHIP_QOS"
    //     },
    //     "partition":
    //     [
    //         "partition_1",
    //         "partition_2"
    //     ],
    //     "presentation":
    //     {
    //         "access_scope": "INSTANCE_PRESENTATION_QOS",
    //         "coherent_access": false,
    //         "ordered_access": false
    //     },
    //     "reliability":
    //     {
    //         "kind": "RELIABLE_RELIABILITY_QOS",
    //         "max_blocking_time":
    //         {
    //             "nanoseconds": 0,
    //             "seconds": 3
    //         }
    //     },
    //     "representation":
    //     [
    //     ],
    //     "time_based_filter":
    //     {
    //         "minimum_separation":
    //         {
    //             "seconds": 12,
    //             "nanoseconds": 0
    //         }
    //     },
    //     "topic_data": "5b33419a",
    //     "type_consistency":
    //     {
    //         "force_type_validation": false,
    //         "ignore_member_names": false,
    //         "ignore_sequence_bounds": true,
    //         "ignore_string_bounds": true,
    //         "kind": "DISALLOW_TYPE_COERCION",
    //         "prevent_type_widening": false
    //     },
    //     "user_data": "ff00"
    // })"_json;

    // json_obj["id"] = entity_id;

    // return json_obj;
}

} // namespace statistics_backend
} // namespace eprosima
