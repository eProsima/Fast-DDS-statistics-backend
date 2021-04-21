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

#include <chrono>

#include "gtest/gtest.h"

#include <database/samples.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

TEST(database, statistics_sample_clear)
{
    StatisticsSample sample(DataKind::NACKFRAG_COUNT);
    sample.src_ts = std::chrono::steady_clock::now();
    sample.clear();
    ASSERT_EQ(sample.kind, DataKind::INVALID);
    ASSERT_EQ(sample.src_ts, std::chrono::steady_clock::time_point());
}

TEST(database, entitydata_sample_clear)
{
    EntityDataSample sample(DataKind::NACKFRAG_COUNT);
    sample.src_ts = std::chrono::steady_clock::now();
    sample.data = 12.0;
    sample.clear();
    ASSERT_EQ(sample.kind, DataKind::INVALID);
    ASSERT_EQ(sample.src_ts, std::chrono::steady_clock::time_point());
    ASSERT_EQ(sample.data, 0);
}

TEST(database, entitycount_sample_clear)
{
    EntityCountSample sample(DataKind::NACKFRAG_COUNT);
    sample.src_ts = std::chrono::steady_clock::now();
    sample.count = 12;
    sample.clear();
    ASSERT_EQ(sample.kind, DataKind::INVALID);
    ASSERT_EQ(sample.src_ts, std::chrono::steady_clock::time_point());
    ASSERT_EQ(sample.count, 0);
}

TEST(database, bytecount_sample_clear)
{
    ByteCountSample sample(DataKind::NACKFRAG_COUNT);
    sample.src_ts = std::chrono::steady_clock::now();
    sample.count = 12;
    sample.magnitude_order = 2;
    sample.clear();
    ASSERT_EQ(sample.kind, DataKind::INVALID);
    ASSERT_EQ(sample.src_ts, std::chrono::steady_clock::time_point());
    ASSERT_EQ(sample.count, 0);
    ASSERT_EQ(sample.magnitude_order, 0);
}

TEST(database, statisticssample_operator_comparison)
{
    StatisticsSample sample_1;
    StatisticsSample sample_2;
    ASSERT_EQ(sample_1, sample_2);

    sample_2.src_ts = std::chrono::steady_clock::now();
    ASSERT_NE(sample_1, sample_2);

    sample_2 = StatisticsSample();
    sample_2.kind = DataKind::FASTDDS_LATENCY;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, entitydatasample_operator_comparison)
{
    EntityDataSample sample_1;
    EntityDataSample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.data = 2;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, entitycountsample_operator_comparison)
{
    EntityCountSample sample_1;
    EntityCountSample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.count = 2;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, bytecountsample_operator_comparison)
{
    ByteCountSample sample_1;
    ByteCountSample sample_2;
    ASSERT_EQ(sample_1, sample_2);

    sample_2.count = 2;
    ASSERT_NE(sample_1, sample_2);

    sample_2 = ByteCountSample();
    sample_2.magnitude_order = 3;
    sample_2.magnitude_order = 3;
    ASSERT_NE(sample_1, sample_2);

    sample_2 = ByteCountSample();
    sample_2.count = 2;
    sample_2.magnitude_order = 3;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, timepointsample_operator_comparison)
{
    TimepointSample sample_1;
    TimepointSample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.time = std::chrono::steady_clock::now();
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, entitytolocatorcountsample_operator_comparison)
{
    EntityToLocatorCountSample sample_1;
    EntityToLocatorCountSample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.remote_locator = EntityId(5);
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, bytetolocatorcountsample_operator_comparison)
{
    ByteToLocatorCountSample sample_1;
    ByteToLocatorCountSample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.remote_locator = EntityId(5);
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, historylatencysample_operator_comparison)
{
    HistoryLatencySample sample_1;
    HistoryLatencySample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.reader = EntityId(5);
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, networklatencysample_operator_comparison)
{
    NetworkLatencySample sample_1;
    NetworkLatencySample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.remote_locator = EntityId(5);
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, publicationthroughputsample_operator_comparison)
{
    PublicationThroughputSample sample_1;
    PublicationThroughputSample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.data = 2;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, subscriptionthroughputsample_operator_comparison)
{
    SubscriptionThroughputSample sample_1;
    SubscriptionThroughputSample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.data = 2;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, rtpspacketssentsample_operator_comparison)
{
    RtpsPacketsSentSample sample_1;
    RtpsPacketsSentSample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.count = 2;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, rtpsbytessentsample_operator_comparison)
{
    RtpsBytesSentSample sample_1;
    RtpsBytesSentSample sample_2;
    ASSERT_EQ(sample_1, sample_2);

    sample_2.count = 2;
    ASSERT_NE(sample_1, sample_2);

    sample_2 = RtpsBytesSentSample();
    sample_2.magnitude_order = 3;
    ASSERT_NE(sample_1, sample_2);

    sample_2 = RtpsBytesSentSample();
    sample_2.count = 2;
    sample_2.magnitude_order = 3;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, rtpspacketslostsample_operator_comparison)
{
    RtpsPacketsLostSample sample_1;
    RtpsPacketsLostSample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.count = 2;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, rtpsbyteslostsample_operator_comparison)
{
    RtpsBytesLostSample sample_1;
    RtpsBytesLostSample sample_2;
    ASSERT_EQ(sample_1, sample_2);

    sample_2.count = 2;
    ASSERT_NE(sample_1, sample_2);

    sample_2 = RtpsBytesLostSample();
    sample_2.magnitude_order = 3;
    ASSERT_NE(sample_1, sample_2);

    sample_2 = RtpsBytesLostSample();
    sample_2.count = 2;
    sample_2.magnitude_order = 3;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, resentdatasample_operator_comparison)
{
    ResentDataSample sample_1;
    ResentDataSample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.count = 2;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, heartbeatcountsample_operator_comparison)
{
    HeartbeatCountSample sample_1;
    HeartbeatCountSample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.count = 2;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, acknackcountsample_operator_comparison)
{
    AcknackCountSample sample_1;
    AcknackCountSample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.count = 2;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, nackfragcountsample_operator_comparison)
{
    NackfragCountSample sample_1;
    NackfragCountSample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.count = 2;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, gapcountsample_operator_comparison)
{
    GapCountSample sample_1;
    GapCountSample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.count = 2;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, datacountsample_operator_comparison)
{
    DataCountSample sample_1;
    DataCountSample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.count = 2;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, pdpcountsample_operator_comparison)
{
    PdpCountSample sample_1;
    PdpCountSample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.count = 2;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, edpcountsample_operator_comparison)
{
    EdpCountSample sample_1;
    EdpCountSample sample_2;
    ASSERT_EQ(sample_1, sample_2);
    sample_2.count = 2;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, discoverytimesample_operator_comparison)
{
    DiscoveryTimeSample sample_1;
    DiscoveryTimeSample sample_2;
    ASSERT_EQ(sample_1, sample_2);

    sample_2.remote_entity = EntityId(5);
    ASSERT_NE(sample_1, sample_2);

    sample_2 = DiscoveryTimeSample();
    sample_2.discovered = true;
    ASSERT_NE(sample_1, sample_2);

    sample_2 = DiscoveryTimeSample();
    sample_2.remote_entity = EntityId(5);
    sample_2.discovered = true;
    ASSERT_NE(sample_1, sample_2);
}

TEST(database, sampledatascountsample_operator_comparison)
{
    SampleDatasCountSample sample_1;
    SampleDatasCountSample sample_2;
    ASSERT_EQ(sample_1, sample_2);

    sample_2.count = 2;
    ASSERT_NE(sample_1, sample_2);

    sample_2 = SampleDatasCountSample();
    sample_2.sequence_number = 3;
    ASSERT_NE(sample_1, sample_2);

    sample_2 = SampleDatasCountSample();
    sample_2.count = 2;
    sample_2.sequence_number = 3;
    ASSERT_NE(sample_1, sample_2);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
