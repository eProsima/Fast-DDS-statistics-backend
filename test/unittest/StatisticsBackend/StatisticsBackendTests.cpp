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

#include <list>

#include <StatisticsBackend.hpp>
#include <types/types.hpp>

#include "gtest/gtest.h"

using namespace eprosima::statistics_backend;

// Check the get_data_supported_entity_kinds StatisticsBackend method
TEST(statistics_backend_tests, get_data_supported_entity_kinds)
{
    std::map<DataKind, std::pair<EntityKind, EntityKind>> data_to_entity_map =
    {
        {DataKind::INVALID, std::pair<EntityKind, EntityKind>(
             EntityKind::INVALID, EntityKind::INVALID)},

        {DataKind::FASTDDS_LATENCY, std::pair<EntityKind, EntityKind>(
             EntityKind::DATAWRITER, EntityKind::DATAREADER)},

        {DataKind::NETWORK_LATENCY, std::pair<EntityKind, EntityKind>(
             EntityKind::LOCATOR, EntityKind::LOCATOR)},

        {DataKind::PUBLICATION_THROUGHPUT, std::pair<EntityKind, EntityKind>(
             EntityKind::DATAWRITER, EntityKind::INVALID)},

        {DataKind::SUBSCRIPTION_THROUGHPUT, std::pair<EntityKind, EntityKind>(
             EntityKind::DATAREADER, EntityKind::INVALID)},

        {DataKind::RTPS_PACKETS_SENT, std::pair<EntityKind, EntityKind>(
             EntityKind::DATAWRITER, EntityKind::LOCATOR)},

        {DataKind::RTPS_BYTES_SENT, std::pair<EntityKind, EntityKind>(
             EntityKind::DATAWRITER, EntityKind::LOCATOR)},

        {DataKind::RTPS_PACKETS_LOST, std::pair<EntityKind, EntityKind>(
             EntityKind::DATAWRITER, EntityKind::LOCATOR)},

        {DataKind::RTPS_BYTES_LOST, std::pair<EntityKind, EntityKind>(
             EntityKind::DATAWRITER, EntityKind::LOCATOR)},

        {DataKind::RESENT_DATA, std::pair<EntityKind, EntityKind>(
             EntityKind::DATAWRITER, EntityKind::INVALID)},

        {DataKind::HEARTBEAT_COUNT, std::pair<EntityKind, EntityKind>(
             EntityKind::DATAWRITER, EntityKind::INVALID)},

        {DataKind::ACKNACK_COUNT, std::pair<EntityKind, EntityKind>(
             EntityKind::DATAREADER, EntityKind::INVALID)},

        {DataKind::NACKFRAG_COUNT, std::pair<EntityKind, EntityKind>(
             EntityKind::DATAREADER, EntityKind::INVALID)},

        {DataKind::GAP_COUNT, std::pair<EntityKind, EntityKind>(
             EntityKind::DATAWRITER, EntityKind::INVALID)},

        {DataKind::DATA_COUNT, std::pair<EntityKind, EntityKind>(
             EntityKind::DATAWRITER, EntityKind::INVALID)},

        {DataKind::PDP_PACKETS, std::pair<EntityKind, EntityKind>(
             EntityKind::PARTICIPANT, EntityKind::INVALID)},

        {DataKind::EDP_PACKETS, std::pair<EntityKind, EntityKind>(
             EntityKind::PARTICIPANT, EntityKind::INVALID)},

        {DataKind::SAMPLE_DATAS, std::pair<EntityKind, EntityKind>(
             EntityKind::DATAWRITER, EntityKind::INVALID)}
    };

    // Check every DataKind with only one relation
    for (auto element : data_to_entity_map)
    {
        auto res = StatisticsBackend::get_data_supported_entity_kinds(element.first);
        ASSERT_EQ(res.size(), 1);

        ASSERT_EQ(res[0].first, element.second.first);
        ASSERT_EQ(res[0].second, element.second.second);
    }

    // Check DISCOVERY_TIME
    {
        auto discovery_time_types = std::list<std::pair<EntityKind, EntityKind>> ({
            std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::PARTICIPANT),
            std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::DATAWRITER),
            std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::DATAREADER)
        });

        auto res = StatisticsBackend::get_data_supported_entity_kinds(DataKind::DISCOVERY_TIME);
        ASSERT_EQ(res.size(), 3);

        for (auto types_allowed : res)
        {
            ASSERT_NE(
                std::find(discovery_time_types.begin(),
                discovery_time_types.end(),
                types_allowed),
                discovery_time_types.end());
        }
    }
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
