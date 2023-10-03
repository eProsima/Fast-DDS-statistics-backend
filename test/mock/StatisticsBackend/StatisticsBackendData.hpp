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
 * @file StatisticsBackendData.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATA_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATA_HPP_

#include <gtest_aux.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fastdds_statistics_backend/types/types.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>

#include <chrono>
#include <string>

namespace eprosima {
namespace statistics_backend {
namespace details {

class StatisticsBackendData
{

public:

    enum DiscoveryStatus
    {
        DISCOVERY,      ///< The entity was discovered
        UNDISCOVERY,    ///< The entity was undiscovered
        UPDATE          ///< The entity was updated
    };

    MOCK_METHOD4(on_domain_entity_discovery, void(
                EntityId domain_id,
                EntityId entity_id,
                EntityKind entity_kind,
                DiscoveryStatus discovery_status));

    MOCK_METHOD3(on_physical_entity_discovery, void(
                EntityId entity_id,
                EntityKind entity_kind,
                DiscoveryStatus discovery_status));

    MOCK_METHOD1(on_domain_graph_update, void(
                EntityId entity_id));

    MOCK_METHOD3(on_data_available, void(
                EntityId domain_id,
                EntityId entity_id,
                DataKind data_kind));

    static StatisticsBackendData* get_instance()
    {
        static StatisticsBackendData instance;
        return &instance;
    }

protected:

    StatisticsBackendData()
    {
    }

};

} // namespace details
} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATISTICSBACKEND_HPP_
