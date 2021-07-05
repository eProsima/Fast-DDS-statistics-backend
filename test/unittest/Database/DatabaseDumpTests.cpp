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
#include <fstream>
#include <memory>
#include <string>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/JSONTags.h>
#include <fastdds_statistics_backend/types/types.hpp>

#include <database/database.hpp>
#include <database/entities.hpp>
#include <database/samples.hpp>
#include <DatabaseUtils.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

constexpr const char* GUID_DEFAULT = "01.0f.00.00.00.00.00.00.00.00.00.00|0.0.0.0";
constexpr const char* PID_DEFAULT = "36000";
constexpr const char* DATA_TYPE_DEFAULT = "data_type";
#define QOS_DEFAULT Qos(R"({"qos":"empty"})"_json)

constexpr const bool STATUS_DEFAULT = true;
constexpr const uint64_t COUNT_DEFAULT = 2;
constexpr const double DATA_DEFAULT = 1.1;
constexpr const uint64_t SEQUENCE_NUMBER_DEFAULT = 3;
constexpr const int16_t MAGNITUDE_DEFAULT = 0;

#define LOCATOR_DEFAULT_NAME(x) "locator_" + std::to_string(x)
#define HOST_DEFAULT_NAME(x) "host_" + std::to_string(x)
#define USER_DEFAULT_NAME(x) "user_" + std::to_string(x)
#define PROCESS_DEFAULT_NAME(x) "process_" + std::to_string(x)
#define DOMAIN_DEFAULT_NAME(x) "12" + std::to_string(x)
#define ALIAS_DEFAULT_NAME(x) "domain_" + std::to_string(x)
#define TOPIC_DEFAULT_NAME(x) "topic_" + std::to_string(x)
#define PARTICIPANT_DEFAULT_NAME(x) "participant_" + std::to_string(x)
#define DATAWRITER_DEFAULT_NAME(x) "datawriter_" + std::to_string(x)
#define DATAREADER_DEFAULT_NAME(x) "datareader_" + std::to_string(x)

#define LOCATOR_DEFAULT_ID(x) EntityId(x * 9 + 0)
#define HOST_DEFAULT_ID(x) EntityId(x * 9 + 1)
#define USER_DEFAULT_ID(x) EntityId(x * 9 + 2)
#define PROCESS_DEFAULT_ID(x) EntityId(x * 9 + 3)
#define DOMAIN_DEFAULT_ID(x) EntityId(x * 9 + 4)
#define TOPIC_DEFAULT_ID(x) EntityId(x * 9 + 5)
#define PARTICIPANT_DEFAULT_ID(x) EntityId(x * 9 + 6)
#define DATAWRITER_DEFAULT_ID(x) EntityId(x * 9 + 7)
#define DATAREADER_DEFAULT_ID(x) EntityId(x * 9 + 8)

// at least pass microseconds tenths to avoid windows system_clock resolution issue
#define TIME_DEFAULT(x) nanoseconds_to_systemclock(100 * (x))
#define GUID_DEFAULT(x) "01.0f.00.00.00.00.00.00.00.00.00.0" + std::to_string(x) + "|0.0.0.0"

void initialize_empty_entities(
        Database& db,
        int index,
        bool link_process_participant)
{
    std::shared_ptr<Host> host = std::make_shared<Host>(std::string(HOST_DEFAULT_NAME(index)));
    std::shared_ptr<User> user = std::make_shared<User>(std::string(USER_DEFAULT_NAME(index)), host);
    std::shared_ptr<Process> process = std::make_shared<Process>(std::string(PROCESS_DEFAULT_NAME(
                        index)), PID_DEFAULT, user);
    std::shared_ptr<Domain> domain = std::make_shared<Domain>(std::string(DOMAIN_DEFAULT_NAME(index)));
    domain->alias = ALIAS_DEFAULT_NAME(index);
    std::shared_ptr<Topic> topic = std::make_shared<Topic>(std::string(TOPIC_DEFAULT_NAME(
                        index)), DATA_TYPE_DEFAULT, domain);
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(std::string(
                        PARTICIPANT_DEFAULT_NAME(index)), QOS_DEFAULT, GUID_DEFAULT(index), nullptr, domain);
    std::shared_ptr<DataWriter> dw = std::make_shared<DataWriter>(std::string(
                        DATAWRITER_DEFAULT_NAME(index)), QOS_DEFAULT, GUID_DEFAULT(index), participant, topic);
    std::shared_ptr<DataReader> dr = std::make_shared<DataReader>(std::string(
                        DATAREADER_DEFAULT_NAME(index)), QOS_DEFAULT, GUID_DEFAULT(index), participant, topic);
    std::shared_ptr<Locator> locator = std::make_shared<Locator>(std::string(LOCATOR_DEFAULT_NAME(index)));

    locator->id = db.generate_entity_id();

    dw->locators[locator->id] = locator;
    dr->locators[locator->id] = locator;

    ASSERT_NE(db.insert(host), EntityId::invalid());
    ASSERT_NE(db.insert(user), EntityId::invalid());
    ASSERT_NE(db.insert(process), EntityId::invalid());
    ASSERT_NE(db.insert(domain), EntityId::invalid());
    ASSERT_NE(db.insert(topic), EntityId::invalid());
    ASSERT_NE(db.insert(participant), EntityId::invalid());
    ASSERT_NE(db.insert(dw), EntityId::invalid());
    ASSERT_NE(db.insert(dr), EntityId::invalid());

    if (link_process_participant)
    {
        db.link_participant_with_process(participant->id, process->id);
    }
    locator->data_writers[dw->id] = dw;
    locator->data_readers[dr->id] = dr;
}

void initialize_participant_data(
        Database& db,
        int index,
        int time)
{
    // discovered_entity
    {
        DiscoveryTimeSample sample;
        sample.remote_entity = PARTICIPANT_DEFAULT_ID(index);
        sample.time = TIME_DEFAULT(time);
        sample.src_ts = TIME_DEFAULT(time + 1);
        sample.discovered = STATUS_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID(index), PARTICIPANT_DEFAULT_ID(index), sample);
    }

    // pdp_packets
    {
        PdpCountSample sample;
        sample.src_ts = TIME_DEFAULT(time);
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID(index), PARTICIPANT_DEFAULT_ID(index), sample);
    }

    // edp_packets
    {
        EdpCountSample sample;
        sample.src_ts = TIME_DEFAULT(time);
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID(index), PARTICIPANT_DEFAULT_ID(index), sample);
    }

    // rtps_packets_sent
    {
        RtpsPacketsSentSample sample;
        sample.src_ts = TIME_DEFAULT(time);
        sample.remote_locator = LOCATOR_DEFAULT_ID(index);
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID(index), PARTICIPANT_DEFAULT_ID(index), sample);
    }

    // rtps_bytes_sent
    {
        RtpsBytesSentSample sample;
        sample.src_ts = TIME_DEFAULT(time);
        sample.remote_locator = LOCATOR_DEFAULT_ID(index);
        sample.count = COUNT_DEFAULT;
        sample.magnitude_order = MAGNITUDE_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID(index), PARTICIPANT_DEFAULT_ID(index), sample);
    }

    // rtps_packets_lost
    {
        RtpsPacketsLostSample sample;
        sample.src_ts = TIME_DEFAULT(time);
        sample.remote_locator = LOCATOR_DEFAULT_ID(index);
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID(index), PARTICIPANT_DEFAULT_ID(index), sample);
    }

    // rtps_bytes_lost
    {
        RtpsBytesLostSample sample;
        sample.src_ts = TIME_DEFAULT(time);
        sample.remote_locator = LOCATOR_DEFAULT_ID(index);
        sample.count = COUNT_DEFAULT;
        sample.magnitude_order = MAGNITUDE_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID(index), PARTICIPANT_DEFAULT_ID(index), sample);
    }
}

void initialize_datawriter_data(
        Database& db,
        int index,
        int time)
{
    // publication_throughput
    {
        PublicationThroughputSample sample;
        sample.src_ts = TIME_DEFAULT(time);
        sample.data = DATA_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID(index), DATAWRITER_DEFAULT_ID(index), sample);
    }

    // resent_datas
    {
        ResentDataSample sample;
        sample.src_ts = TIME_DEFAULT(time);
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID(index), DATAWRITER_DEFAULT_ID(index), sample);
    }

    // heartbeat_count
    {
        HeartbeatCountSample sample;
        sample.src_ts = TIME_DEFAULT(time);
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID(index), DATAWRITER_DEFAULT_ID(index), sample);
    }

    // gap_count
    {
        GapCountSample sample;
        sample.src_ts = TIME_DEFAULT(time);
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID(index), DATAWRITER_DEFAULT_ID(index), sample);
    }

    // data_count
    {
        DataCountSample sample;
        sample.src_ts = TIME_DEFAULT(time);
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID(index), DATAWRITER_DEFAULT_ID(index), sample);
    }

    // sample_datas
    {
        SampleDatasCountSample sample;
        sample.src_ts = TIME_DEFAULT(time);
        sample.count = COUNT_DEFAULT;
        sample.sequence_number = SEQUENCE_NUMBER_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID(index), DATAWRITER_DEFAULT_ID(index), sample);
    }

    // history2history_latency
    {
        HistoryLatencySample sample;
        sample.src_ts = TIME_DEFAULT(time);
        sample.data = DATA_DEFAULT;
        sample.reader = DATAREADER_DEFAULT_ID(index);
        db.insert(DOMAIN_DEFAULT_ID(index), DATAWRITER_DEFAULT_ID(index), sample);
    }

}

void initialize_datareader_data(
        Database& db,
        int index,
        int time)
{
    // subscription_throughput
    {
        SubscriptionThroughputSample sample;
        sample.src_ts = TIME_DEFAULT(time);
        sample.data = DATA_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID(index), DATAREADER_DEFAULT_ID(index), sample);
    }

    // acknack_count
    {
        AcknackCountSample sample;
        sample.src_ts = TIME_DEFAULT(time);
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID(index), DATAREADER_DEFAULT_ID(index), sample);
    }

    // nackfrag_count
    {
        NackfragCountSample sample;
        sample.src_ts = TIME_DEFAULT(time);
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID(index), DATAREADER_DEFAULT_ID(index), sample);
    }
}

void initialize_locator_data(
        Database& db,
        int index,
        int time)
{
    // network_latency_per_locator
    {
        NetworkLatencySample sample;
        sample.src_ts = TIME_DEFAULT(time);
        sample.data = DATA_DEFAULT;
        sample.remote_locator = LOCATOR_DEFAULT_ID(index);
        db.insert(DOMAIN_DEFAULT_ID(index), LOCATOR_DEFAULT_ID(index), sample);
    }
}

void initialize_database(
        Database& db,
        int n_entity,
        int n_data,
        bool link_process_participant = true)
{
    for (int i = 0; i < n_entity; ++i)
    {
        initialize_empty_entities(db, i, link_process_participant);
        for (int j = 0; j < n_data; ++j)
        {
            initialize_participant_data(db, i, j);
            initialize_datawriter_data(db, i, j);
            initialize_datareader_data(db, i, j);
            initialize_locator_data(db, i, j);
        }
    }
}

class database : public ::testing::Test
{
public:

    void SetUp()
    {
    }

};

// Test the dump of a database without any entity
TEST(database, dump_empty_database)
{
    Database db;
    DatabaseDump dump;
    load_file(EMPTY_DUMP_FILE, dump);
    ASSERT_EQ(db.dump_database(), dump);
}

// Test the dump of a database with one entity of each kind
TEST(database, dump_empty_entities_database)
{
    Database db;
    initialize_database(db, 1, 0);
    DatabaseDump dump;
    load_file(EMPTY_ENTITIES_DUMP_FILE, dump);
    ASSERT_EQ(db.dump_database(), dump);
}

// Test the dump of a database with one entity of each kind and one data of each kind
TEST(database, dump_simple_database)
{
    Database db;
    initialize_database(db, 1, 1);
    DatabaseDump dump;
    load_file(SIMPLE_DUMP_FILE, dump);
    ASSERT_EQ(db.dump_database(), dump);
}

// Test the dump of a database with no links between process and participant
TEST(database, dump_no_process_participant_link)
{
    Database db;
    initialize_database(db, 1, 1, false);
    DatabaseDump dump;
    load_file(NO_PROCESS_PARTICIPANT_LINK_DUMP_FILE, dump);
    ASSERT_EQ(db.dump_database(), dump);
}

// Test the dump of a database with three entities of each kind and three datas of each kind
TEST(database, dump_complex_database)
{
    Database db;
    initialize_database(db, 3, 3);
    DatabaseDump dump;
    load_file(COMPLEX_DUMP_FILE, dump);
    ASSERT_EQ(db.dump_database(), dump);
}

void initialize_empty_entities_unlinked(
        Database& db,
        int index)
{
    std::shared_ptr<Host> host = std::make_shared<Host>(std::string(HOST_DEFAULT_NAME(index)));
    std::shared_ptr<User> user = std::make_shared<User>(std::string(USER_DEFAULT_NAME(index)), host);
    std::shared_ptr<Process> process = std::make_shared<Process>(std::string(PROCESS_DEFAULT_NAME(
                        index)), PID_DEFAULT, user);
    std::shared_ptr<Domain> domain = std::make_shared<Domain>(std::string(DOMAIN_DEFAULT_NAME(index)));
    domain->alias = ALIAS_DEFAULT_NAME(index);
    std::shared_ptr<Topic> topic = std::make_shared<Topic>(std::string(TOPIC_DEFAULT_NAME(
                        index)), DATA_TYPE_DEFAULT, domain);
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(std::string(
                        PARTICIPANT_DEFAULT_NAME(index)), QOS_DEFAULT, GUID_DEFAULT(index), nullptr, domain);
    std::shared_ptr<DataWriter> dw = std::make_shared<DataWriter>(std::string(
                        DATAWRITER_DEFAULT_NAME(index)), QOS_DEFAULT, GUID_DEFAULT(index), participant, topic);
    std::shared_ptr<DataReader> dr = std::make_shared<DataReader>(std::string(
                        DATAREADER_DEFAULT_NAME(index)), QOS_DEFAULT, GUID_DEFAULT(index), participant, topic);
    std::shared_ptr<Locator> locator = std::make_shared<Locator>(std::string(LOCATOR_DEFAULT_NAME(index)));

    locator->id = db.generate_entity_id();

    dw->locators[locator->id] = locator;
    dr->locators[locator->id] = locator;

    ASSERT_NE(db.insert(host), EntityId::invalid());
    ASSERT_NE(db.insert(user), EntityId::invalid());
    ASSERT_NE(db.insert(process), EntityId::invalid());
    ASSERT_NE(db.insert(domain), EntityId::invalid());
    ASSERT_NE(db.insert(topic), EntityId::invalid());
    ASSERT_NE(db.insert(participant), EntityId::invalid());
    ASSERT_NE(db.insert(dw), EntityId::invalid());
    ASSERT_NE(db.insert(dr), EntityId::invalid());

    // Locator unlinked
    std::shared_ptr<Locator> locator_2 = std::make_shared<Locator>(std::string(LOCATOR_DEFAULT_NAME(index + 1)));
    ASSERT_NE(db.insert(locator_2), EntityId::invalid());
}

// Test the dump of a database with one entity of each kind with unlinked entities
TEST(database, dump_unlinked_database)
{
    DataBaseTest db;
    initialize_empty_entities_unlinked(db, 0);

    DatabaseDump dump;
    load_file(EMPTY_ENTITIES_DUMP_FILE, dump);
    dump[PARTICIPANT_CONTAINER_TAG].begin().value()[PROCESS_ENTITY_TAG] = "-1";
    dump[PROCESS_CONTAINER_TAG].begin().value()[PARTICIPANT_CONTAINER_TAG] = DatabaseDump::array();

    std::string locator2_id = std::to_string(db.next_id() - 1);

    dump[LOCATOR_CONTAINER_TAG][locator2_id] = dump[LOCATOR_CONTAINER_TAG].begin().value();
    dump[LOCATOR_CONTAINER_TAG][locator2_id][NAME_INFO_TAG] = std::string(LOCATOR_DEFAULT_NAME(1));
    dump[LOCATOR_CONTAINER_TAG][locator2_id][ALIAS_INFO_TAG] = std::string(LOCATOR_DEFAULT_NAME(1));

    // Locator unlinked
    dump[LOCATOR_CONTAINER_TAG][locator2_id][DATAWRITER_CONTAINER_TAG] = DatabaseDump::array();
    dump[LOCATOR_CONTAINER_TAG][locator2_id][DATAREADER_CONTAINER_TAG] = DatabaseDump::array();

    ASSERT_EQ(db.dump_database(), dump);
}

// Test the dump of a database with a clear of the statistics data
TEST(database, dump_and_clear_database)
{
    DataBaseTest db;
    initialize_database(db, 1, 1);

    // Check database contains statistics data
    {
        // Participants
        for (const auto& super_it : db.participants())
        {
            // For each entity of this kind in the domain
            for (const auto& it : super_it.second)
            {
                ASSERT_FALSE(it.second->data.discovered_entity.empty());
                ASSERT_FALSE(it.second->data.pdp_packets.empty());
                ASSERT_FALSE(it.second->data.edp_packets.empty());
                ASSERT_FALSE(it.second->data.rtps_packets_sent.empty());
                ASSERT_FALSE(it.second->data.rtps_bytes_sent.empty());
                ASSERT_FALSE(it.second->data.rtps_packets_lost.empty());
                ASSERT_FALSE(it.second->data.rtps_bytes_lost.empty());
            }
        }
        // Datawriters
        for (const auto& super_it : db.datawriters())
        {
            // For each entity of this kind in the domain
            for (const auto& it : super_it.second)
            {
                ASSERT_FALSE(it.second->data.publication_throughput.empty());
                ASSERT_FALSE(it.second->data.resent_datas.empty());
                ASSERT_FALSE(it.second->data.heartbeat_count.empty());
                ASSERT_FALSE(it.second->data.gap_count.empty());
                ASSERT_FALSE(it.second->data.data_count.empty());
                ASSERT_FALSE(it.second->data.sample_datas.empty());
                ASSERT_FALSE(it.second->data.history2history_latency.empty());
            }
        }
        // Datareaders
        for (const auto& super_it : db.datareaders())
        {
            // For each entity of this kind in the domain
            for (const auto& it : super_it.second)
            {
                ASSERT_FALSE(it.second->data.subscription_throughput.empty());
                ASSERT_FALSE(it.second->data.acknack_count.empty());
                ASSERT_FALSE(it.second->data.nackfrag_count.empty());
            }
        }
        // Locators
        for (const auto& it : db.locators())
        {
            ASSERT_FALSE(it.second->data.network_latency_per_locator.empty());
        }
    }

    DatabaseDump dump = db.dump_database(true);

    // Check database does not contain statistics data
    {
        // Participants
        for (const auto& super_it : db.participants())
        {
            // For each entity of this kind in the domain
            for (const auto& it : super_it.second)
            {
                ASSERT_TRUE(it.second->data.discovered_entity.empty());
                ASSERT_TRUE(it.second->data.pdp_packets.empty());
                ASSERT_TRUE(it.second->data.edp_packets.empty());
                ASSERT_TRUE(it.second->data.rtps_packets_sent.empty());
                ASSERT_TRUE(it.second->data.rtps_bytes_sent.empty());
                ASSERT_TRUE(it.second->data.rtps_packets_lost.empty());
                ASSERT_TRUE(it.second->data.rtps_bytes_lost.empty());
            }
        }
        // Datawriters
        for (const auto& super_it : db.datawriters())
        {
            // For each entity of this kind in the domain
            for (const auto& it : super_it.second)
            {
                ASSERT_TRUE(it.second->data.publication_throughput.empty());
                ASSERT_TRUE(it.second->data.resent_datas.empty());
                ASSERT_TRUE(it.second->data.heartbeat_count.empty());
                ASSERT_TRUE(it.second->data.gap_count.empty());
                ASSERT_TRUE(it.second->data.data_count.empty());
                ASSERT_TRUE(it.second->data.sample_datas.empty());
                ASSERT_TRUE(it.second->data.history2history_latency.empty());
            }
        }
        // Datareaders
        for (const auto& super_it : db.datareaders())
        {
            // For each entity of this kind in the domain
            for (const auto& it : super_it.second)
            {
                ASSERT_TRUE(it.second->data.subscription_throughput.empty());
                ASSERT_TRUE(it.second->data.acknack_count.empty());
                ASSERT_TRUE(it.second->data.nackfrag_count.empty());
            }
        }
        // Locators
        for (const auto& it : db.locators())
        {
            ASSERT_TRUE(it.second->data.network_latency_per_locator.empty());
        }
    }
}

// Test the database method id_to_string()
TEST(database, id_to_string)
{
    DataBaseTest db;
    ASSERT_EQ(db.get_id_to_string(EntityId(0)), "0");
    ASSERT_EQ(db.get_id_to_string(EntityId(5)), "5");
    ASSERT_EQ(db.get_id_to_string(EntityId(-5)), "-5");
    ASSERT_NE(db.get_id_to_string(EntityId(0)), "5");
    ASSERT_NE(db.get_id_to_string(EntityId(-5)), "5");
    ASSERT_NE(db.get_id_to_string(EntityId(0)), "abc");
}

// Test the database method time_to_string()
TEST(database, time_to_string)
{
    DataBaseTest db;
    ASSERT_EQ(db.get_time_to_string(nanoseconds_to_systemclock(100)), "100");
    ASSERT_NE(db.get_time_to_string(nanoseconds_to_systemclock(100)), "500");
    ASSERT_EQ(db.get_time_to_string(nanoseconds_to_systemclock(-500)), "-500");
    ASSERT_NE(db.get_time_to_string(nanoseconds_to_systemclock(100)), "ABC");
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
