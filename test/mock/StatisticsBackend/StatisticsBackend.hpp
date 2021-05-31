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
 * @file StatisticsBackend.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATISTICSBACKEND_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATISTICSBACKEND_HPP_

#include <fastdds-statistics-backend/fastdds_statistics_backend_dll.h>
#include <fastdds-statistics-backend/listener/DomainListener.hpp>
#include <fastdds-statistics-backend/listener/PhysicalListener.hpp>
#include <fastdds-statistics-backend/listener/CallbackMask.hpp>
#include <fastdds-statistics-backend/types/types.hpp>
#include <fastdds-statistics-backend/types/EntityId.hpp>


#include <chrono>
#include <string>

namespace eprosima {
namespace statistics_backend {
namespace database {

class DatabaseDataQueue;
class DatabaseEntityQueue;

} // namespace database


class StatisticsBackend
{
    friend class database::DatabaseDataQueue;
    friend class database::DatabaseEntityQueue;

public:

    // Exposing the singleton instance so the tests can have expectations on mocked methods
    static StatisticsBackend* get_instance()
    {
        static StatisticsBackend instance;
        return &instance;
    }

    enum DiscoveryStatus
    {
        DISCOVERY,      ///< The entity was discovered
        UNDISCOVERY,    ///< The entity was undiscovered
        UPDATE          ///< The entity was updated
    };

    MOCK_METHOD4(mocked_on_domain_entity_discovery, void(
                EntityId domain_id,
                EntityId entity_id,
                EntityKind entity_kind,
                DiscoveryStatus discovery_status));

    MOCK_METHOD3(mocked_on_physical_entity_discovery, void(
                EntityId participant_id,
                EntityId entity_id,
                EntityKind entity_kind));

    MOCK_METHOD3(mocked_on_data_available, void(
                EntityId domain_id,
                EntityId entity_id,
                DataKind data_kind));

protected:

    StatisticsBackend()
    {
    }

    static void on_domain_entity_discovery(
            EntityId domain_id,
            EntityId entity_id,
            EntityKind entity_kind,
            DiscoveryStatus discovery_status)
    {
        StatisticsBackend::get_instance()->mocked_on_domain_entity_discovery(
            domain_id, entity_id, entity_kind, discovery_status);
    }

    static void on_physical_entity_discovery(
            EntityId participant_id,
            EntityId entity_id,
            EntityKind entity_kind)
    {
        StatisticsBackend::get_instance()->mocked_on_physical_entity_discovery(
            participant_id, entity_id, entity_kind);
    }

    static void on_data_available(
            EntityId domain_id,
            EntityId entity_id,
            DataKind data_kind)
    {
        StatisticsBackend::get_instance()->mocked_on_data_available(
            domain_id, entity_id, data_kind);
    }

};

} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATISTICSBACKEND_HPP_
