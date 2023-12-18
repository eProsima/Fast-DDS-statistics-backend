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

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <fastdds_statistics_backend/types/EntityId.hpp>

#include <database/data.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

TEST(database, domainparticipant_data_clear)
{
    /* Add dummy data to DomainParticipantStatisticsData */
    DomainParticipantStatisticsData data;
    EntityCountSample count_sample;
    count_sample.count = 12;
    ByteCountSample byte_sample;
    byte_sample.count = 13;
    byte_sample.magnitude_order = 2;
    DiscoveryTimeSample time_sample;
    time_sample.discovered = true;
    time_sample.time = std::chrono::system_clock::now();
    EntityDataSample data_sample;
    data_sample.data = 11.0;

    // RTPSData
    data.rtps_packets_sent[EntityId(2)].push_back(count_sample);
    data.last_reported_rtps_packets_sent_count[EntityId(2)] = count_sample;
    data.rtps_bytes_sent[EntityId(3)].push_back(byte_sample);
    data.last_reported_rtps_bytes_sent_count[EntityId(3)] = byte_sample;
    data.rtps_packets_lost[EntityId(4)].push_back(count_sample);
    data.last_reported_rtps_packets_lost_count[EntityId(4)] = count_sample;
    data.rtps_bytes_lost[EntityId(5)].push_back(byte_sample);
    data.last_reported_rtps_bytes_lost_count[EntityId(5)] = byte_sample;

    // DomainParticipantStatisticsData
    data.discovered_entity[EntityId(1)].push_back(time_sample);
    data.pdp_packets.push_back(count_sample);
    data.last_reported_pdp_packets = count_sample;
    data.edp_packets.push_back(count_sample);
    data.last_reported_edp_packets = count_sample;
    data.network_latency_per_locator[EntityId(6)].push_back(data_sample);

    /* Check that data in cleared */
    data.clear();
    ASSERT_TRUE(data.rtps_packets_sent.empty());
    ASSERT_EQ(data.last_reported_rtps_packets_sent_count[EntityId(2)].count, 0ul);
    ASSERT_TRUE(data.rtps_bytes_sent.empty());
    ASSERT_EQ(data.last_reported_rtps_bytes_sent_count[EntityId(3)].count, 0ul);
    ASSERT_TRUE(data.rtps_packets_lost.empty());
    ASSERT_EQ(data.last_reported_rtps_packets_lost_count[EntityId(4)].count, 0ul);
    ASSERT_TRUE(data.rtps_bytes_lost.empty());
    ASSERT_EQ(data.last_reported_rtps_bytes_lost_count[EntityId(5)].count, 0ul);
    ASSERT_TRUE(data.discovered_entity.empty());
    ASSERT_TRUE(data.pdp_packets.empty());
    ASSERT_EQ(data.last_reported_pdp_packets.count, 0ul);
    ASSERT_TRUE(data.edp_packets.empty());
    ASSERT_EQ(data.last_reported_edp_packets.count, 0ul);
    ASSERT_TRUE(data.network_latency_per_locator.empty());
}

TEST(database, datareader_data_clear)
{
    /* Add dummy data to DataReaderStatisticsData */
    DataReaderStatisticsData data;
    EntityDataSample data_sample;
    data_sample.data = 11.0;
    EntityCountSample count_sample;
    count_sample.count = 12;

    // DDSEndpointData
    data.subscription_throughput.push_back(data_sample);
    data.acknack_count.push_back(count_sample);
    data.last_reported_acknack_count = count_sample;
    data.nackfrag_count.push_back(count_sample);
    data.last_reported_nackfrag_count = count_sample;

    /* Check that data in cleared */
    data.clear();
    ASSERT_TRUE(data.subscription_throughput.empty());
    ASSERT_TRUE(data.acknack_count.empty());
    ASSERT_EQ(data.last_reported_acknack_count.count, 0ul);
    ASSERT_TRUE(data.nackfrag_count.empty());
    ASSERT_EQ(data.last_reported_nackfrag_count.count, 0ul);
}

TEST(database, datawriter_data_clear)
{
    /* Add dummy data to DataWriterStatisticsData */
    DataWriterStatisticsData data;
    EntityDataSample data_sample;
    data_sample.data = 11.0;
    EntityCountSample count_sample;
    count_sample.count = 12;

    // DataWriterStatisticsData
    data.publication_throughput.push_back(data_sample);
    data.resent_datas.push_back(count_sample);
    data.last_reported_resent_datas = count_sample;
    data.heartbeat_count.push_back(count_sample);
    data.last_reported_heartbeat_count = count_sample;
    data.gap_count.push_back(count_sample);
    data.last_reported_gap_count = count_sample;
    data.data_count.push_back(count_sample);
    data.last_reported_data_count = count_sample;
    data.sample_datas[1].push_back(count_sample);
    data.history2history_latency[EntityId(1)].push_back(data_sample);

    /* Check that data in cleared */
    data.clear();
    ASSERT_TRUE(data.history2history_latency.empty());
    ASSERT_TRUE(data.publication_throughput.empty());
    ASSERT_TRUE(data.resent_datas.empty());
    ASSERT_EQ(data.last_reported_resent_datas.count, 0ul);
    ASSERT_TRUE(data.heartbeat_count.empty());
    ASSERT_EQ(data.last_reported_heartbeat_count.count, 0ul);
    ASSERT_TRUE(data.gap_count.empty());
    ASSERT_EQ(data.last_reported_gap_count.count, 0ul);
    ASSERT_TRUE(data.data_count.empty());
    ASSERT_EQ(data.last_reported_data_count.count, 0ul);
    ASSERT_TRUE(data.sample_datas.empty());
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
