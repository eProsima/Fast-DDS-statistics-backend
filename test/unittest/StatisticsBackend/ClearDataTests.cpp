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

#include <fstream>
#include <vector>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <StatisticsBackend.hpp>
#include <DatabaseUtils.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

TEST(clear_statistics_data_tests, clear_statistics_data_simple)
{
    // Load the dump to the database
    StatisticsBackend::load_database(SIMPLE_DUMP_FILE);

    // Check that there is at least a domain
    ASSERT_FALSE(StatisticsBackend::get_entities(EntityKind::DOMAIN).empty());

    // Check that there is data loaded
    // Get Reader id
    auto reader_ids = StatisticsBackend::get_entities(EntityKind::DATAREADER);
    ASSERT_EQ(reader_ids.size(), 1u);
    auto reader_id = reader_ids[0];
    ASSERT_FALSE(
        StatisticsBackend::get_data(
            DataKind::SUBSCRIPTION_THROUGHPUT, std::vector<EntityId>({reader_id}), 0, StatisticKind::NONE).empty()
    );

    // Clearing data
    StatisticsBackend::clear_statistics_data();

    // Check that DataReader still exist but do not have data anymore
    auto reader_ids_after_reset = StatisticsBackend::get_entities(EntityKind::DATAREADER);
    ASSERT_EQ(reader_ids_after_reset.size(), 1u);
    ASSERT_EQ(reader_ids_after_reset[0], reader_id);
    ASSERT_TRUE(
        StatisticsBackend::get_data(
            DataKind::SUBSCRIPTION_THROUGHPUT, std::vector<EntityId>({reader_id}), 0, StatisticKind::NONE).empty()
    );
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
