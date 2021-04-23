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

#include "gtest/gtest.h"

#include <fastdds-statistics-backend/exception/Exception.hpp>
#include <fastdds-statistics-backend/types/EntityId.hpp>
#include <fastdds-statistics-backend/types/JSONTags.h>
#include <fastdds-statistics-backend/types/types.hpp>

#include <database/database.hpp>
#include <database/entities.hpp>
#include <database/samples.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

constexpr const char* DESCRIPTION_TAG = "description";

constexpr const char* EMPTY_DUMP_FILE = "resources/empty_dump.json";
constexpr const char* EMPTY_ENTITIES_DUMP_FILE = "resources/empty_entities_dump.json";
constexpr const char* SIMPLE_DUMP_FILE = "resources/simple_dump.json";

constexpr const char* GUID_DEFAULT = "01.0f.00.00.00.00.00.00.00.00.00.00|00.00.00.00";
constexpr const char* PID_DEFAULT = "36000";
constexpr const char* DATA_TYPE_DEFAULT = "data_type";
#define QOS_DEFAULT Qos(R"({"qos":"empty"})"_json)

constexpr const bool STATUS_DEFAULT = true;
constexpr const uint64_t COUNT_DEFAULT = 2;
constexpr const double DATA_DEFAULT = 1.1;
constexpr const uint64_t SEQUENCE_NUMBER_DEFAULT = 3;
constexpr const int16_t MAGNITUDE_DEFAULT = 0;
constexpr const std::chrono::steady_clock::time_point TIME_DEFAULT =
        std::chrono::steady_clock::time_point(std::chrono::steady_clock::duration(1600000000000000001L));

#define DOMAIN_DEFAULT_ID EntityId(4)
#define PARTICIPANT_DEFAULT_ID EntityId(6)
#define DATAWRITER_DEFAULT_ID EntityId(7)
#define DATAREADER_DEFAULT_ID EntityId(8)
#define LOCATOR_DEFAULT_ID EntityId(0)

void initialize_empty_entities(
        Database& db);
void initialize_simple_entities(
        Database& db);
void initialize_participant_data(
        Database& db);
void initialize_datawriter_data(
        Database& db);
void initialize_datareader_data(
        Database& db);
void initialize_locator_data(
        Database& db);
void initialize_data(
        Database& db);
void initialize_data(
        Database& db,
        int sample_number);

DatabaseDump load_file(
        std::string filename)
{
    // Check if the file exists
    std::ifstream file(filename);
    if (!file.good())
    {
        throw BadParameter("File " + filename + " does not exist");
    }

    // Get the json file
    DatabaseDump dump;
    file >> dump;

    // Erase the description tag if existing
    if (dump.contains(DESCRIPTION_TAG))
    {
        dump.erase(DESCRIPTION_TAG);
    }

    return dump;
}

void initialize_empty_entities(
        Database& db)
{
    std::shared_ptr<Host> host = std::make_shared<Host>(std::string(HOST_ENTITY));
    std::shared_ptr<User> user = std::make_shared<User>(std::string(USER_ENTITY), host);
    std::shared_ptr<Process> process = std::make_shared<Process>(std::string(PROCESS_ENTITY), PID_DEFAULT, user);
    std::shared_ptr<Domain> domain = std::make_shared<Domain>(std::string(DOMAIN_ENTITY));
    std::shared_ptr<Topic> topic = std::make_shared<Topic>(std::string(TOPIC_ENTITY), DATA_TYPE_DEFAULT, domain);
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(std::string(
                        PARTICIPANT_ENTITY), QOS_DEFAULT, GUID_DEFAULT, nullptr, domain);
    std::shared_ptr<DataWriter> dw = std::make_shared<DataWriter>(std::string(
                        DATAWRITER_ENTITY), QOS_DEFAULT, GUID_DEFAULT, participant, topic);
    std::shared_ptr<DataReader> dr = std::make_shared<DataReader>(std::string(
                        DATAREADER_ENTITY), QOS_DEFAULT, GUID_DEFAULT, participant, topic);
    std::shared_ptr<Locator> locator = std::make_shared<Locator>(std::string(LOCATOR_ENTITY));

    // This ID is set manually as there si no insert for Locator in database
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

    db.link_participant_with_process(participant->id, process->id);

    locator->data_writers[dw->id] = dw;
    locator->data_readers[dr->id] = dr;
}

void initialize_simple_entities(
        Database& db)
{
    // Initialize entities
    initialize_empty_entities(db);
    // Insert data
    initialize_data(db);
}

void initialize_participant_data(
        Database& db)
{
    // discovered_entity
    {
        DiscoveryTimeSample sample;
        sample.remote_entity = PARTICIPANT_DEFAULT_ID;
        sample.time = TIME_DEFAULT;
        sample.discovered = STATUS_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID, PARTICIPANT_DEFAULT_ID, sample);
    }

    // pdp_packets
    {
        PdpCountSample sample;
        sample.src_ts = TIME_DEFAULT;
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID, PARTICIPANT_DEFAULT_ID, sample);
    }

    // edp_packets
    {
        EdpCountSample sample;
        sample.src_ts = TIME_DEFAULT;
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID, PARTICIPANT_DEFAULT_ID, sample);
    }
}

void initialize_datawriter_data(
        Database& db)
{
    // publication_throughput
    {
        PublicationThroughputSample sample;
        sample.src_ts = TIME_DEFAULT;
        sample.data = DATA_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID, DATAWRITER_DEFAULT_ID, sample);
    }

    // resent_datas
    {
        ResentDataSample sample;
        sample.src_ts = TIME_DEFAULT;
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID, DATAWRITER_DEFAULT_ID, sample);
    }

    // heartbeat_count
    {
        HeartbeatCountSample sample;
        sample.src_ts = TIME_DEFAULT;
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID, DATAWRITER_DEFAULT_ID, sample);
    }

    // gap_count
    {
        GapCountSample sample;
        sample.src_ts = TIME_DEFAULT;
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID, DATAWRITER_DEFAULT_ID, sample);
    }

    // data_count
    {
        DataCountSample sample;
        sample.src_ts = TIME_DEFAULT;
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID, DATAWRITER_DEFAULT_ID, sample);
    }

    // sample_datas
    {
        SampleDatasCountSample sample;
        sample.src_ts = TIME_DEFAULT;
        sample.count = COUNT_DEFAULT;
        sample.sequence_number = SEQUENCE_NUMBER_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID, DATAWRITER_DEFAULT_ID, sample);
    }

    // history2history_latency
    {
        HistoryLatencySample sample;
        sample.src_ts = TIME_DEFAULT;
        sample.data = DATA_DEFAULT;
        sample.reader = DATAREADER_DEFAULT_ID;
        db.insert(DOMAIN_DEFAULT_ID, DATAWRITER_DEFAULT_ID, sample);
    }

    // rtps_packets_sent
    {
        RtpsPacketsSentSample sample;
        sample.src_ts = TIME_DEFAULT;
        sample.remote_locator = LOCATOR_DEFAULT_ID;
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID, DATAWRITER_DEFAULT_ID, sample);
    }

    // rtps_bytes_sent
    {
        RtpsBytesSentSample sample;
        sample.src_ts = TIME_DEFAULT;
        sample.remote_locator = LOCATOR_DEFAULT_ID;
        sample.count = COUNT_DEFAULT;
        sample.magnitude_order = MAGNITUDE_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID, DATAWRITER_DEFAULT_ID, sample);
    }

    // rtps_packets_lost
    {
        RtpsPacketsLostSample sample;
        sample.src_ts = TIME_DEFAULT;
        sample.remote_locator = LOCATOR_DEFAULT_ID;
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID, DATAWRITER_DEFAULT_ID, sample);
    }

    // rtps_bytes_lost
    {
        RtpsBytesLostSample sample;
        sample.src_ts = TIME_DEFAULT;
        sample.remote_locator = LOCATOR_DEFAULT_ID;
        sample.count = COUNT_DEFAULT;
        sample.magnitude_order = MAGNITUDE_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID, DATAWRITER_DEFAULT_ID, sample);
    }
}

void initialize_datareader_data(
        Database& db)
{
    // subscription_throughput
    {
        SubscriptionThroughputSample sample;
        sample.src_ts = TIME_DEFAULT;
        sample.data = DATA_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID, DATAREADER_DEFAULT_ID, sample);
    }

    // acknack_count
    {
        AcknackCountSample sample;
        sample.src_ts = TIME_DEFAULT;
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID, DATAREADER_DEFAULT_ID, sample);
    }

    // nackfrag_count
    {
        NackfragCountSample sample;
        sample.src_ts = TIME_DEFAULT;
        sample.count = COUNT_DEFAULT;
        db.insert(DOMAIN_DEFAULT_ID, DATAREADER_DEFAULT_ID, sample);
    }
}

void initialize_locator_data(
        Database& db)
{
    // network_latency_per_locator
    {
        NetworkLatencySample sample;
        sample.src_ts = TIME_DEFAULT;
        sample.data = DATA_DEFAULT;
        sample.remote_locator = LOCATOR_DEFAULT_ID;
        db.insert(DOMAIN_DEFAULT_ID, LOCATOR_DEFAULT_ID, sample);
    }
}

void initialize_data(
        Database& db)
{
    initialize_participant_data(db);
    initialize_datawriter_data(db);
    initialize_datareader_data(db);
    initialize_locator_data(db);
}

void initialize_data(
        Database& db,
        int sample_number)
{
    for (int i = 0; i < sample_number; i++)
    {
        initialize_data(db);
    }
}

class database : public ::testing::Test
{
public:

    void SetUp()
    {
    }

};

TEST(database, dump_empty_database)
{
    Database db;
    ASSERT_EQ(db.dump_database(), load_file(EMPTY_DUMP_FILE));
}

TEST(database, dump_empty_entities_database)
{
    Database db;
    initialize_empty_entities(db);
    ASSERT_EQ(db.dump_database(), load_file(EMPTY_ENTITIES_DUMP_FILE));
}

TEST(database, dump_simple_database)
{
    Database db;
    initialize_simple_entities(db);
    std::cout << "finish initializing" << std::endl;
    ASSERT_EQ(db.dump_database(), load_file(SIMPLE_DUMP_FILE));
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
