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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cmath>

#include <exception/Exception.hpp>
#include <Monitor.hpp>
#include <StatisticsBackend.hpp>
#include <StatisticsBackendData.hpp>
#include <database/database.hpp>
#include <types/types.hpp>

using ::testing::_;
using ::testing::Invoke;
using ::testing::AnyNumber;
using ::testing::Throw;

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;



class get_data_tests_base
{

public:

    get_data_tests_base()
    {
        StatisticsBackend::load_database("resources/database_dump.json");
    }

    const std::vector<DataKind> data_kind_values_ = {
            DataKind::FASTDDS_LATENCY,
            DataKind::NETWORK_LATENCY,
            DataKind::PUBLICATION_THROUGHPUT,
            DataKind::SUBSCRIPTION_THROUGHPUT,
            DataKind::RTPS_PACKETS_SENT,
            DataKind::RTPS_BYTES_SENT,
            DataKind::RTPS_PACKETS_LOST,
            DataKind::RTPS_BYTES_LOST,
            DataKind::RESENT_DATA,
            DataKind::HEARTBEAT_COUNT,
            DataKind::ACKNACK_COUNT,
            DataKind::NACKFRAG_COUNT,
            DataKind::GAP_COUNT,
            DataKind::DATA_COUNT,
            DataKind::PDP_PACKETS,
            DataKind::EDP_PACKETS,
            DataKind::DISCOVERY_TIME,
            DataKind::SAMPLE_DATAS};

    const std::vector<StatisticKind> statistic_kind_values_ = {
            StatisticKind::COUNT,
            StatisticKind::MAX,
            StatisticKind::MIN,
            StatisticKind::MEDIAN,
            StatisticKind::STANDARD_DEVIATION,
            StatisticKind::SUM,
            StatisticKind::NONE};
};



#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_SUITE_P(x, y, z)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_CASE_P(x, y, z)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

class get_data_no_data_tests
    : public get_data_tests_base
    , public ::testing::TestWithParam<std::tuple<DataKind, EntityId, EntityId>>
{
    // Tests with entities that have no data for the given data kind
};

GTEST_INSTANTIATE_TEST_MACRO(
    get_data_no_data_tests,
    get_data_no_data_tests,
    ::testing::Values(
        std::make_tuple(DataKind::FASTDDS_LATENCY, EntityId(25), EntityId(26)),
        std::make_tuple(DataKind::NETWORK_LATENCY, EntityId(18), EntityId(18)),
        std::make_tuple(DataKind::PUBLICATION_THROUGHPUT, EntityId(25), EntityId::invalid()),
        std::make_tuple(DataKind::SUBSCRIPTION_THROUGHPUT, EntityId(26), EntityId::invalid()),
        std::make_tuple(DataKind::RTPS_PACKETS_SENT, EntityId(24), EntityId(18)),
        std::make_tuple(DataKind::RTPS_BYTES_SENT, EntityId(24), EntityId(18)),
        std::make_tuple(DataKind::RTPS_PACKETS_LOST, EntityId(24), EntityId(18)),
        std::make_tuple(DataKind::RTPS_BYTES_LOST, EntityId(24), EntityId(18)),
        std::make_tuple(DataKind::RESENT_DATA, EntityId(25), EntityId::invalid()),
        std::make_tuple(DataKind::HEARTBEAT_COUNT, EntityId(25), EntityId::invalid()),
        std::make_tuple(DataKind::ACKNACK_COUNT, EntityId(26), EntityId::invalid()),
        std::make_tuple(DataKind::NACKFRAG_COUNT, EntityId(26), EntityId::invalid()),
        std::make_tuple(DataKind::GAP_COUNT, EntityId(25), EntityId::invalid()),
        std::make_tuple(DataKind::DATA_COUNT, EntityId(25), EntityId::invalid()),
        std::make_tuple(DataKind::PDP_PACKETS, EntityId(24), EntityId::invalid()),
        std::make_tuple(DataKind::EDP_PACKETS, EntityId(24), EntityId::invalid()),
        std::make_tuple(DataKind::DISCOVERY_TIME, EntityId(24), EntityId(24)),
        std::make_tuple(DataKind::DISCOVERY_TIME, EntityId(24), EntityId(25)),
        std::make_tuple(DataKind::DISCOVERY_TIME, EntityId(24), EntityId(26)),
        std::make_tuple(DataKind::SAMPLE_DATAS, EntityId(25), EntityId::invalid())
    ));

TEST_P(get_data_no_data_tests, no_data)
{
    DataKind data_type = std::get<0>(GetParam());
    EntityId entity1 = std::get<1>(GetParam());
    EntityId entity2 = std::get<2>(GetParam());
    StatisticKind statistic = StatisticKind::COUNT;

    std::vector<StatisticsData> result;

    if (entity2.is_valid())
    {
        result = StatisticsBackend::get_data(
            data_type,
            std::vector<EntityId>(1, entity1),
            std::vector<EntityId>(1, entity2),
            0,
            Timestamp(),
            std::chrono::system_clock::now(),
            statistic);

        ASSERT_TRUE(result.empty());

        result = StatisticsBackend::get_data(
            data_type,
            std::vector<EntityId>(1, entity1),
            std::vector<EntityId>(1, entity2),
            10,
            Timestamp(),
            Timestamp() + std::chrono::seconds(10),
            statistic);

        ASSERT_EQ(10, result.size());
    }
    else
    {
        result = StatisticsBackend::get_data(
            data_type,
            std::vector<EntityId>(1, entity1),
            0,
            Timestamp(),
            std::chrono::system_clock::now(),
            statistic);

        ASSERT_TRUE(result.empty());

        result = StatisticsBackend::get_data(
            data_type,
            std::vector<EntityId>(1, entity1),
            10,
            Timestamp(),
            Timestamp() + std::chrono::seconds(10),
            statistic);

        ASSERT_EQ(10, result.size());
    }

    for (size_t i = 0; i < result.size(); ++i)
    {
        ASSERT_TRUE(std::isnan(result[i].second));
        ASSERT_EQ(Timestamp() + std::chrono::seconds(i), result[i].first);
    }
}

class get_data_with_data_tests
    : public get_data_tests_base
    , public ::testing::TestWithParam<std::tuple<DataKind, EntityId, EntityId>>
{
    // Tests with entities that have some data for the given data kind
};


GTEST_INSTANTIATE_TEST_MACRO(
    get_data_with_data_tests,
    get_data_with_data_tests,
    ::testing::Values(
        std::make_tuple(DataKind::FASTDDS_LATENCY, EntityId(16), EntityId(17)),
        std::make_tuple(DataKind::NETWORK_LATENCY, EntityId(9), EntityId(9)),
        std::make_tuple(DataKind::PUBLICATION_THROUGHPUT, EntityId(16), EntityId::invalid()),
        std::make_tuple(DataKind::SUBSCRIPTION_THROUGHPUT, EntityId(17), EntityId::invalid()),
        std::make_tuple(DataKind::RTPS_PACKETS_SENT, EntityId(15), EntityId(9)),
        std::make_tuple(DataKind::RTPS_BYTES_SENT, EntityId(15), EntityId(9)),
        std::make_tuple(DataKind::RTPS_PACKETS_LOST, EntityId(15), EntityId(9)),
        std::make_tuple(DataKind::RTPS_BYTES_LOST, EntityId(15), EntityId(9)),
        std::make_tuple(DataKind::RESENT_DATA, EntityId(16), EntityId::invalid()),
        std::make_tuple(DataKind::HEARTBEAT_COUNT, EntityId(16), EntityId::invalid()),
        std::make_tuple(DataKind::ACKNACK_COUNT, EntityId(17), EntityId::invalid()),
        std::make_tuple(DataKind::NACKFRAG_COUNT, EntityId(17), EntityId::invalid()),
        std::make_tuple(DataKind::GAP_COUNT, EntityId(16), EntityId::invalid()),
        std::make_tuple(DataKind::DATA_COUNT, EntityId(16), EntityId::invalid()),
        std::make_tuple(DataKind::PDP_PACKETS, EntityId(15), EntityId::invalid()),
        std::make_tuple(DataKind::EDP_PACKETS, EntityId(15), EntityId::invalid()),
        std::make_tuple(DataKind::DISCOVERY_TIME, EntityId(15), EntityId(15)),
        std::make_tuple(DataKind::DISCOVERY_TIME, EntityId(15), EntityId(16)),
        std::make_tuple(DataKind::DISCOVERY_TIME, EntityId(15), EntityId(17)),
        std::make_tuple(DataKind::SAMPLE_DATAS, EntityId(16), EntityId::invalid())
    ));


TEST_P(get_data_with_data_tests, invalid_entity_kind)
{
    DataKind data_type = std::get<0>(GetParam());
    EntityId entity1 = std::get<1>(GetParam());
    EntityId entity2 = std::get<2>(GetParam());
    StatisticKind statistic = StatisticKind::COUNT;

    if (entity2.is_valid())
    {
        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, EntityId::invalid()),
                    std::vector<EntityId>(1, entity2),
                    0,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);

        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, EntityId::invalid()),
                    std::vector<EntityId>(1, entity2),
                    10,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);

        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, entity1),
                    std::vector<EntityId>(1, EntityId::invalid()),
                    0,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);

        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, entity1),
                    std::vector<EntityId>(1, EntityId::invalid()),
                    10,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);
    }
    else
    {
        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, EntityId::invalid()),
                    0,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);

        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, EntityId::invalid()),
                    10,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);
    }
}

TEST_P(get_data_with_data_tests, nonexistent_entity_id)
{
    DataKind data_type = std::get<0>(GetParam());
    EntityId entity1 = std::get<1>(GetParam());
    EntityId entity2 = std::get<2>(GetParam());
    StatisticKind statistic = StatisticKind::COUNT;

    if (entity2.is_valid())
    {
        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, EntityId(200)),
                    std::vector<EntityId>(1, entity2),
                    0,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);

        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, EntityId(200)),
                    std::vector<EntityId>(1, entity2),
                    10,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);

        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, entity1),
                    std::vector<EntityId>(1, EntityId(200)),
                    0,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);

        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, entity1),
                    std::vector<EntityId>(1, EntityId(200)),
                    10,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);
    }
    else
    {
        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, EntityId(200)),
                    0,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);

        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, EntityId(200)),
                    10,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);
    }
}

TEST_P(get_data_with_data_tests, invalid_data_kind)
{
    DataKind data_type = std::get<0>(GetParam());
    EntityId entity1 = std::get<1>(GetParam());
    EntityId entity2 = std::get<2>(GetParam());
    StatisticKind statistic = StatisticKind::COUNT;

    if (entity2.is_valid())
    {
        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, entity1),
                    std::vector<EntityId>(1, entity2),
                    0,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);

        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, entity1),
                    std::vector<EntityId>(1, entity2),
                    10,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);
    }
    else
    {
        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, entity1),
                    0,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);

        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, entity1),
                    10,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);
    }
}

class get_data_unsupported_entities_tests
    : public get_data_tests_base
    , public ::testing::TestWithParam<std::tuple<DataKind, EntityId, EntityId>>
{
    // Tests with entities that are not supported for the given data kind
    // (but otherwise exist in the database)
};

GTEST_INSTANTIATE_TEST_MACRO(
    get_data_unsupported_entities_tests,
    get_data_unsupported_entities_tests,
    ::testing::Values(
        std::make_tuple(DataKind::FASTDDS_LATENCY, EntityId(17), EntityId(16)),
        std::make_tuple(DataKind::FASTDDS_LATENCY, EntityId(16), EntityId(16)),
        std::make_tuple(DataKind::FASTDDS_LATENCY, EntityId(17), EntityId(17)),
        std::make_tuple(DataKind::FASTDDS_LATENCY, EntityId(15), EntityId(16)),
        std::make_tuple(DataKind::FASTDDS_LATENCY, EntityId(17), EntityId(9)),

        std::make_tuple(DataKind::NETWORK_LATENCY, EntityId(16), EntityId(17)),
        std::make_tuple(DataKind::NETWORK_LATENCY, EntityId(17), EntityId(9)),
        std::make_tuple(DataKind::NETWORK_LATENCY, EntityId(16), EntityId(9)),
        std::make_tuple(DataKind::NETWORK_LATENCY, EntityId(9), EntityId(16)),
        std::make_tuple(DataKind::NETWORK_LATENCY, EntityId(9), EntityId(17)),

        std::make_tuple(DataKind::PUBLICATION_THROUGHPUT, EntityId(17), EntityId::invalid()),
        std::make_tuple(DataKind::PUBLICATION_THROUGHPUT, EntityId(15), EntityId::invalid()),
        std::make_tuple(DataKind::PUBLICATION_THROUGHPUT, EntityId(9), EntityId::invalid()),

        std::make_tuple(DataKind::SUBSCRIPTION_THROUGHPUT, EntityId(16), EntityId::invalid()),
        std::make_tuple(DataKind::SUBSCRIPTION_THROUGHPUT, EntityId(15), EntityId::invalid()),
        std::make_tuple(DataKind::SUBSCRIPTION_THROUGHPUT, EntityId(9), EntityId::invalid()),

        std::make_tuple(DataKind::RTPS_PACKETS_SENT, EntityId(15), EntityId(17)),
        std::make_tuple(DataKind::RTPS_PACKETS_SENT, EntityId(16), EntityId(17)),
        std::make_tuple(DataKind::RTPS_PACKETS_SENT, EntityId(16), EntityId(9)),

        std::make_tuple(DataKind::RTPS_BYTES_SENT, EntityId(15), EntityId(17)),
        std::make_tuple(DataKind::RTPS_BYTES_SENT, EntityId(16), EntityId(17)),
        std::make_tuple(DataKind::RTPS_BYTES_SENT, EntityId(16), EntityId(9)),

        std::make_tuple(DataKind::RTPS_PACKETS_LOST, EntityId(15), EntityId(17)),
        std::make_tuple(DataKind::RTPS_PACKETS_LOST, EntityId(16), EntityId(17)),
        std::make_tuple(DataKind::RTPS_PACKETS_LOST, EntityId(16), EntityId(9)),

        std::make_tuple(DataKind::RTPS_BYTES_LOST, EntityId(15), EntityId(17)),
        std::make_tuple(DataKind::RTPS_BYTES_LOST, EntityId(16), EntityId(17)),
        std::make_tuple(DataKind::RTPS_BYTES_LOST, EntityId(16), EntityId(9)),

        std::make_tuple(DataKind::RESENT_DATA, EntityId(17), EntityId::invalid()),
        std::make_tuple(DataKind::RESENT_DATA, EntityId(15), EntityId::invalid()),
        std::make_tuple(DataKind::RESENT_DATA, EntityId(9), EntityId::invalid()),

        std::make_tuple(DataKind::HEARTBEAT_COUNT, EntityId(17), EntityId::invalid()),
        std::make_tuple(DataKind::HEARTBEAT_COUNT, EntityId(15), EntityId::invalid()),
        std::make_tuple(DataKind::HEARTBEAT_COUNT, EntityId(9), EntityId::invalid()),

        std::make_tuple(DataKind::ACKNACK_COUNT, EntityId(16), EntityId::invalid()),
        std::make_tuple(DataKind::ACKNACK_COUNT, EntityId(15), EntityId::invalid()),
        std::make_tuple(DataKind::ACKNACK_COUNT, EntityId(9), EntityId::invalid()),

        std::make_tuple(DataKind::NACKFRAG_COUNT, EntityId(16), EntityId::invalid()),
        std::make_tuple(DataKind::NACKFRAG_COUNT, EntityId(15), EntityId::invalid()),
        std::make_tuple(DataKind::NACKFRAG_COUNT, EntityId(9), EntityId::invalid()),

        std::make_tuple(DataKind::GAP_COUNT, EntityId(17), EntityId::invalid()),
        std::make_tuple(DataKind::GAP_COUNT, EntityId(15), EntityId::invalid()),
        std::make_tuple(DataKind::GAP_COUNT, EntityId(9), EntityId::invalid()),

        std::make_tuple(DataKind::DATA_COUNT, EntityId(17), EntityId::invalid()),
        std::make_tuple(DataKind::DATA_COUNT, EntityId(15), EntityId::invalid()),
        std::make_tuple(DataKind::DATA_COUNT, EntityId(9), EntityId::invalid()),

        std::make_tuple(DataKind::PDP_PACKETS, EntityId(16), EntityId::invalid()),
        std::make_tuple(DataKind::PDP_PACKETS, EntityId(17), EntityId::invalid()),
        std::make_tuple(DataKind::PDP_PACKETS, EntityId(9), EntityId::invalid()),

        std::make_tuple(DataKind::EDP_PACKETS, EntityId(16), EntityId::invalid()),
        std::make_tuple(DataKind::EDP_PACKETS, EntityId(17), EntityId::invalid()),
        std::make_tuple(DataKind::EDP_PACKETS, EntityId(9), EntityId::invalid()),

        std::make_tuple(DataKind::DISCOVERY_TIME, EntityId(16), EntityId(15)),
        std::make_tuple(DataKind::DISCOVERY_TIME, EntityId(17), EntityId(16)),
        std::make_tuple(DataKind::DISCOVERY_TIME, EntityId(15), EntityId(14)),

        std::make_tuple(DataKind::SAMPLE_DATAS, EntityId(17), EntityId::invalid()),
        std::make_tuple(DataKind::SAMPLE_DATAS, EntityId(15), EntityId::invalid()),
        std::make_tuple(DataKind::SAMPLE_DATAS, EntityId(9), EntityId::invalid())
    ));

TEST_P(get_data_unsupported_entities_tests, unsupported_entity_kind)
{
    DataKind data_type = DataKind::INVALID;
    EntityId entity1 = std::get<1>(GetParam());
    EntityId entity2 = std::get<2>(GetParam());
    StatisticKind statistic = StatisticKind::COUNT;

    if (entity2.is_valid())
    {
        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, entity1),
                    std::vector<EntityId>(1, entity2),
                    0,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);

        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, entity1),
                    std::vector<EntityId>(1, entity2),
                    10,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);
    }
    else
    {
        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, entity1),
                    0,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);

        EXPECT_THROW(
                StatisticsBackend::get_data(
                    data_type,
                    std::vector<EntityId>(1, entity1),
                    10,
                    Timestamp(),
                    std::chrono::system_clock::now(),
                    statistic),
                BadParameter);
    }
}
/*
#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_SUITE_P(x, y, z)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_CASE_P(x, y, z)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(
    get_data_bad_parameter_tests,
    get_data_bad_parameter_tests,
    ::testing::Combine(::testing::Values(DataKind::FASTDDS_LATENCY,
                    DataKind::NETWORK_LATENCY,
                    DataKind::PUBLICATION_THROUGHPUT,
                    DataKind::SUBSCRIPTION_THROUGHPUT,
                    DataKind::RTPS_PACKETS_SENT,
                    DataKind::RTPS_BYTES_SENT,
                    DataKind::RTPS_PACKETS_LOST,
                    DataKind::RTPS_BYTES_LOST,
                    DataKind::RESENT_DATA,
                    DataKind::HEARTBEAT_COUNT,
                    DataKind::ACKNACK_COUNT,
                    DataKind::NACKFRAG_COUNT,
                    DataKind::GAP_COUNT,
                    DataKind::DATA_COUNT,
                    DataKind::PDP_PACKETS,
                    DataKind::EDP_PACKETS,
                    DataKind::DISCOVERY_TIME,
                    DataKind::SAMPLE_DATAS),
            ::testing::Values(StatisticKind::COUNT,
                    StatisticKind::MAX,
                    StatisticKind::MIN,
                    StatisticKind::MEDIAN,
                    StatisticKind::STANDARD_DEVIATION,
                    StatisticKind::SUM,
                    StatisticKind::NONE)));
*/

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
