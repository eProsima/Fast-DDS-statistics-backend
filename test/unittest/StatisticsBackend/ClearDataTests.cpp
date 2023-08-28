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

/*
 * Test calling StatisticsBackend::clear_statistics_data without a timestamp clear all data
 *
 * @todo check all other entities and data kinds
 */
TEST(clear_statistics_data_tests, clear_statistics_data_simple)
{
    // Load the dump to the database
    StatisticsBackend::load_database(SIMPLE_DUMP_FILE);

    // Check that there is at least a domain
    ASSERT_FALSE(StatisticsBackend::get_entities(EntityKind::DOMAIN_ENTITY).empty());

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

/*
 * Test calling StatisticsBackend::clear_statistics_data with a timestamp and check that old data is removed
 * and forward data is not.
 *
 * @todo check all other entities and data kinds
 */
TEST(clear_statistics_data_tests, clear_statistics_data_time)
{
    // Load the dump to the database
    StatisticsBackend::load_database(COMPLEX_DUMP_FILE);

    // Check that there is at least a domain
    ASSERT_FALSE(StatisticsBackend::get_entities(EntityKind::DOMAIN_ENTITY).empty());

    // Check that there is data loaded
    // Get Reader id 17
    auto reader_ids = StatisticsBackend::get_entities(EntityKind::DATAREADER, EntityId(17));
    ASSERT_GE(reader_ids.size(), 1u);
    auto reader_id = reader_ids[0];

    auto data_before_clear = StatisticsBackend::get_data(
        DataKind::SUBSCRIPTION_THROUGHPUT, std::vector<EntityId>({reader_id}), 0, StatisticKind::NONE);
    ASSERT_GT(data_before_clear.size(), 1u);
    auto amount_of_data_before_clear = data_before_clear.size();

    // Check that there is data before and after the time-point chosen
    auto cut_point = data_before_clear[1].first;
    unsigned int data_before = 0;
    unsigned int data_after = 0;
    for (const auto& data : data_before_clear)
    {
        if (data.first < cut_point)
        {
            ++data_before;
        }
        else
        {
            ++data_after;
        }
    }
    ASSERT_GT(data_before, 0u);
    ASSERT_GT(data_after, 0u);

    // Clearing data
    StatisticsBackend::clear_statistics_data(cut_point);

    // Check that DataReader still exist but do not have data before time-point
    auto reader_ids_after_reset = StatisticsBackend::get_entities(EntityKind::DATAREADER, 17);
    ASSERT_GE(reader_ids_after_reset.size(), 1u);
    ASSERT_EQ(reader_ids_after_reset[0], reader_id);

    auto data_after_clear = StatisticsBackend::get_data(
        DataKind::SUBSCRIPTION_THROUGHPUT, std::vector<EntityId>({reader_id}), 0, StatisticKind::NONE);
    ASSERT_GT(data_after_clear.size(), 0u);
    auto amount_of_data_after_clear = data_after_clear.size();
    ASSERT_GT(amount_of_data_before_clear, amount_of_data_after_clear);

    // Check each data so old data has disappeared
    unsigned int after_clear_data_before = 0;
    unsigned int after_clear_data_after = 0;
    for (const auto& data : data_after_clear)
    {
        if (data.first < cut_point)
        {
            ++after_clear_data_before;
        }
        else
        {
            ++after_clear_data_after;
        }
    }
    ASSERT_EQ(after_clear_data_before, 0u);
    ASSERT_GT(after_clear_data_after, 0u);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
