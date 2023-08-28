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

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <StatisticsBackend.hpp>
#include <StatisticsBackendData.hpp>
#include <DatabaseUtils.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

TEST(backend_dump_tests, reset)
{
    // Load the dump to the database
    StatisticsBackend::load_database(SIMPLE_DUMP_FILE);
    ASSERT_FALSE(StatisticsBackend::get_entities(EntityKind::DOMAIN_ENTITY).empty());

    // Resetting at this point should work and clear the database
    StatisticsBackend::reset();
    ASSERT_TRUE(StatisticsBackend::get_entities(EntityKind::DOMAIN_ENTITY).empty());

    // Load the dump to the database again
    StatisticsBackend::load_database(SIMPLE_DUMP_FILE);
    ASSERT_FALSE(StatisticsBackend::get_entities(EntityKind::DOMAIN_ENTITY).empty());

    // Start a monitor and try resetting
    auto monitor = StatisticsBackend::init_monitor(100);
    EXPECT_THROW(StatisticsBackend::reset(), PreconditionNotMet);
    ASSERT_FALSE(StatisticsBackend::get_entities(EntityKind::DOMAIN_ENTITY).empty());

    // Stop the monitor and try again
    StatisticsBackend::stop_monitor(monitor);
    EXPECT_NO_THROW(StatisticsBackend::reset());
    ASSERT_TRUE(StatisticsBackend::get_entities(EntityKind::DOMAIN_ENTITY).empty());
}

TEST(backend_dump_tests, database_dump_load)
{
    constexpr const char* TEST_DUMP_FILE = "test_dump.json";
    constexpr const char* NON_EXISTENT_FILE = "/this_directory_does_not_exist/test_dump.json";

    // The dump as loaded by the JSON library
    DatabaseDump expected;
    load_file(SIMPLE_DUMP_FILE, expected);

    // Load the dump to the database and check
    // The correct behavior of Database::dump_database() is already checked on its unit tests
    // and we can consider it works properly
    StatisticsBackend::load_database(SIMPLE_DUMP_FILE);
    DatabaseDump dump = details::StatisticsBackendData::get_instance()->database_->dump_database();
    EXPECT_EQ(expected, dump);

    // Trying to load the dump again must fail
    EXPECT_THROW(StatisticsBackend::load_database(SIMPLE_DUMP_FILE),
            PreconditionNotMet);

    // Reset the backend and load the dump again
    StatisticsBackend::reset();
    StatisticsBackend::load_database(SIMPLE_DUMP_FILE);
    dump = details::StatisticsBackendData::get_instance()->database_->dump_database();
    EXPECT_EQ(expected, dump);

    // Check if the output file exists (we do not want to overwrite anything)
    std::ifstream file(TEST_DUMP_FILE);
    if (file.good())
    {
        FAIL() << "File " << TEST_DUMP_FILE << " exists in the testing directory";
        return;
    }

    // Dump the load
    StatisticsBackend::dump_database(TEST_DUMP_FILE, false);

    // Load it by the JSON library and check equality
    load_file(TEST_DUMP_FILE, dump);
    EXPECT_EQ(expected, dump);

    // Remove the dump file
    std::remove(TEST_DUMP_FILE);

    // Try dumping on a non-existent directory
    ASSERT_THROW(StatisticsBackend::dump_database(NON_EXISTENT_FILE, false),
            BadParameter);

    // Reset the backend and try loading a non existent file
    StatisticsBackend::reset();
    ASSERT_THROW(StatisticsBackend::load_database(NON_EXISTENT_FILE),
            BadParameter);
}

// Test the dump of a database with a clear of the statistics data
TEST(backend_dump_tests, database_dump_and_clear)
{
    StatisticsBackend::load_database(SIMPLE_DUMP_FILE);

    auto participants = details::StatisticsBackendData::get_instance()->database_->get_entities(EntityKind::PARTICIPANT,
                    EntityId::all());
    auto datawriters = details::StatisticsBackendData::get_instance()->database_->get_entities(EntityKind::DATAWRITER,
                    EntityId::all());
    auto datareaders = details::StatisticsBackendData::get_instance()->database_->get_entities(EntityKind::DATAREADER,
                    EntityId::all());
    auto locators = details::StatisticsBackendData::get_instance()->database_->get_entities(EntityKind::LOCATOR,
                    EntityId::all());

    // Check database contains statistics data
    {
        // Participants
        for (const auto& it : participants)
        {
            auto participant = std::dynamic_pointer_cast<const DomainParticipant>(it);
            ASSERT_FALSE(participant->data.discovered_entity.empty());
            ASSERT_FALSE(participant->data.pdp_packets.empty());
            ASSERT_FALSE(participant->data.edp_packets.empty());
            ASSERT_FALSE(participant->data.rtps_packets_sent.empty());
            ASSERT_FALSE(participant->data.rtps_bytes_sent.empty());
            ASSERT_FALSE(participant->data.rtps_packets_lost.empty());
            ASSERT_FALSE(participant->data.rtps_bytes_lost.empty());
            ASSERT_FALSE(participant->data.network_latency_per_locator.empty());

            ASSERT_FALSE(participant->data.last_reported_rtps_packets_sent_count.empty());
            ASSERT_FALSE(participant->data.last_reported_rtps_bytes_sent_count.empty());
            ASSERT_FALSE(participant->data.last_reported_rtps_packets_lost_count.empty());
            ASSERT_FALSE(participant->data.last_reported_rtps_bytes_lost_count.empty());
            ASSERT_FALSE(participant->data.last_reported_pdp_packets.kind == DataKind::INVALID &&
                    participant->data.last_reported_pdp_packets.src_ts ==
                    std::chrono::system_clock::time_point() &&
                    participant->data.last_reported_edp_packets.count == 0);
            ASSERT_FALSE(participant->data.last_reported_edp_packets.kind == DataKind::INVALID &&
                    participant->data.last_reported_edp_packets.src_ts ==
                    std::chrono::system_clock::time_point() &&
                    participant->data.last_reported_edp_packets.count == 0);
        }
        // Datawriters
        for (const auto& it : datawriters)
        {
            auto datawriter = std::dynamic_pointer_cast<const DataWriter>(it);
            ASSERT_FALSE(datawriter->data.publication_throughput.empty());
            ASSERT_FALSE(datawriter->data.resent_datas.empty());
            ASSERT_FALSE(datawriter->data.heartbeat_count.empty());
            ASSERT_FALSE(datawriter->data.gap_count.empty());
            ASSERT_FALSE(datawriter->data.data_count.empty());
            ASSERT_FALSE(datawriter->data.sample_datas.empty());

            ASSERT_FALSE(datawriter->data.last_reported_resent_datas.kind == DataKind::INVALID &&
                    datawriter->data.last_reported_resent_datas.src_ts ==
                    std::chrono::system_clock::time_point() &&
                    datawriter->data.last_reported_resent_datas.count == 0);
            ASSERT_FALSE(datawriter->data.last_reported_heartbeat_count.kind == DataKind::INVALID &&
                    datawriter->data.last_reported_heartbeat_count.src_ts ==
                    std::chrono::system_clock::time_point() &&
                    datawriter->data.last_reported_heartbeat_count.count == 0);
            ASSERT_FALSE(datawriter->data.last_reported_gap_count.kind == DataKind::INVALID &&
                    datawriter->data.last_reported_gap_count.src_ts ==
                    std::chrono::system_clock::time_point() &&
                    datawriter->data.last_reported_gap_count.count == 0);
            ASSERT_FALSE(datawriter->data.last_reported_data_count.kind == DataKind::INVALID &&
                    datawriter->data.last_reported_data_count.src_ts ==
                    std::chrono::system_clock::time_point() &&
                    datawriter->data.last_reported_data_count.count == 0);
        }
        // Datareaders
        for (const auto& it : datareaders)
        {
            auto datareader = std::dynamic_pointer_cast<const DataReader>(it);
            ASSERT_FALSE(datareader->data.subscription_throughput.empty());
            ASSERT_FALSE(datareader->data.acknack_count.empty());
            ASSERT_FALSE(datareader->data.nackfrag_count.empty());

            ASSERT_FALSE(datareader->data.last_reported_acknack_count.kind == DataKind::INVALID &&
                    datareader->data.last_reported_acknack_count.src_ts ==
                    std::chrono::system_clock::time_point() &&
                    datareader->data.last_reported_acknack_count.count == 0);
            ASSERT_FALSE(datareader->data.last_reported_nackfrag_count.kind == DataKind::INVALID &&
                    datareader->data.last_reported_nackfrag_count.src_ts ==
                    std::chrono::system_clock::time_point() &&
                    datareader->data.last_reported_nackfrag_count.count == 0);
        }
    }

    // Check if the output file exists (we do not want to overwrite anything)
    constexpr const char* TEST_DUMP_AND_CLEAR_FILE = "test_dump_and_clear.json";
    std::ifstream file(TEST_DUMP_AND_CLEAR_FILE);
    if (file.good())
    {
        FAIL() << "File " << TEST_DUMP_AND_CLEAR_FILE << " exists in the testing directory";
        return;
    }

    // Dump the load
    StatisticsBackend::dump_database(TEST_DUMP_AND_CLEAR_FILE, true);

    // Check database does not contain statistics data
    {
        // Participants
        for (const auto& it : participants)
        {
            auto participant = std::dynamic_pointer_cast<const DomainParticipant>(it);
            ASSERT_TRUE(participant->data.discovered_entity.empty());
            ASSERT_TRUE(participant->data.pdp_packets.empty());
            ASSERT_TRUE(participant->data.edp_packets.empty());
            ASSERT_TRUE(participant->data.rtps_packets_sent.empty());
            ASSERT_TRUE(participant->data.rtps_bytes_sent.empty());
            ASSERT_TRUE(participant->data.rtps_packets_lost.empty());
            ASSERT_TRUE(participant->data.rtps_bytes_lost.empty());
            ASSERT_TRUE(participant->data.network_latency_per_locator.empty());

            ASSERT_FALSE(participant->data.last_reported_rtps_packets_sent_count.empty());
            ASSERT_FALSE(participant->data.last_reported_rtps_bytes_sent_count.empty());
            ASSERT_FALSE(participant->data.last_reported_rtps_packets_lost_count.empty());
            ASSERT_FALSE(participant->data.last_reported_rtps_bytes_lost_count.empty());
            ASSERT_FALSE(participant->data.last_reported_pdp_packets.kind == DataKind::INVALID &&
                    participant->data.last_reported_pdp_packets.src_ts ==
                    std::chrono::system_clock::time_point() &&
                    participant->data.last_reported_edp_packets.count == 0);
            ASSERT_FALSE(participant->data.last_reported_edp_packets.kind == DataKind::INVALID &&
                    participant->data.last_reported_edp_packets.src_ts ==
                    std::chrono::system_clock::time_point() &&
                    participant->data.last_reported_edp_packets.count == 0);
        }
        // Datawriters
        for (const auto& it : datawriters)
        {
            auto datawriter = std::dynamic_pointer_cast<const DataWriter>(it);
            ASSERT_TRUE(datawriter->data.publication_throughput.empty());
            ASSERT_TRUE(datawriter->data.resent_datas.empty());
            ASSERT_TRUE(datawriter->data.heartbeat_count.empty());
            ASSERT_TRUE(datawriter->data.gap_count.empty());
            ASSERT_TRUE(datawriter->data.data_count.empty());
            ASSERT_TRUE(datawriter->data.sample_datas.empty());

            ASSERT_FALSE(datawriter->data.last_reported_resent_datas.kind == DataKind::INVALID &&
                    datawriter->data.last_reported_resent_datas.src_ts ==
                    std::chrono::system_clock::time_point() &&
                    datawriter->data.last_reported_resent_datas.count == 0);
            ASSERT_FALSE(datawriter->data.last_reported_heartbeat_count.kind == DataKind::INVALID &&
                    datawriter->data.last_reported_heartbeat_count.src_ts ==
                    std::chrono::system_clock::time_point() &&
                    datawriter->data.last_reported_heartbeat_count.count == 0);
            ASSERT_FALSE(datawriter->data.last_reported_gap_count.kind == DataKind::INVALID &&
                    datawriter->data.last_reported_gap_count.src_ts ==
                    std::chrono::system_clock::time_point() &&
                    datawriter->data.last_reported_gap_count.count == 0);
            ASSERT_FALSE(datawriter->data.last_reported_data_count.kind == DataKind::INVALID &&
                    datawriter->data.last_reported_data_count.src_ts ==
                    std::chrono::system_clock::time_point() &&
                    datawriter->data.last_reported_data_count.count == 0);
        }
        // Datareaders
        for (const auto& it : datareaders)
        {
            auto datareader = std::dynamic_pointer_cast<const DataReader>(it);
            ASSERT_TRUE(datareader->data.subscription_throughput.empty());
            ASSERT_TRUE(datareader->data.acknack_count.empty());
            ASSERT_TRUE(datareader->data.nackfrag_count.empty());

            ASSERT_FALSE(datareader->data.last_reported_acknack_count.kind == DataKind::INVALID &&
                    datareader->data.last_reported_acknack_count.src_ts ==
                    std::chrono::system_clock::time_point() &&
                    datareader->data.last_reported_acknack_count.count == 0);
            ASSERT_FALSE(datareader->data.last_reported_nackfrag_count.kind == DataKind::INVALID &&
                    datareader->data.last_reported_nackfrag_count.src_ts ==
                    std::chrono::system_clock::time_point() &&
                    datareader->data.last_reported_nackfrag_count.count == 0);
        }
    }

    // Remove the dump file
    std::remove(TEST_DUMP_AND_CLEAR_FILE);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
