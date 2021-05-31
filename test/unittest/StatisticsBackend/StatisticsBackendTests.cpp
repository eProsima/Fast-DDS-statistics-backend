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
#include "database/database.hpp"

#include "gtest/gtest.h"

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

/**
 * @brief Fixture for the statistics_backend class tests
 *
 * \c get_type Returns the entity kind of a given id.
 * 
 * This fixture populates a database with several entities and several relations among them.
 * Since the tests need to know these relations, each entity is given a unique identifier within the fixture,
 * and the fixture keeps all these entities on a map,
 * for the test to be able to get them and check the result of the execution.
 * This identifier is independent of the EntityId given by the database,
 * and is totally under control of the fixture and the tests.
 * This is important to keep the tests stable even if the Database implementation to assign an EntityId changes.
 *
 * The following Json object describes the populated database objects and their given unique identifiers:
 *
 * \code
 * {
 *  "host1": {
 *     "ID": 1,
 *     "users": []
 *   },
 *   "host": {
 *     "ID": 2,
 *     "users": ["user1", "user"]
 *   },
 *   "user1": {
 *     "ID": 3,
 *     "processes": []
 *   },
 *   "user": {
 *     "ID": 4,
 *     "processes": ["process1", "process"]
 *   },
 *   "process1": {
 *     "ID": 5,
 *     "participants": []
 *   },
 *   "process": {
 *     "ID": 6,
 *     "participants": ["participant1", "participant"]
 *   },
 *   "domain1": {
 *     "ID": 7,
 *     "participants": [],
 *     "topics": []
 *   },
 *   "domain": {
 *     "ID": 8,
 *     "participants": ["participant1", "participant"],
 *     "topics": ["topic1", "topic"]
 *   },
 *   "participant1": {
 *     "ID": 9,
 *     "datareaders": [],
 *     "datawriters": []
 *   },
 *   "participant": {
 *     "ID": 10,
 *     "datareaders": ["datareader1", "datareader"],
 *     "datawriters": ["datawriter1", "datawriter"]
 *   },
 *   "topic1": {
 *     "ID": 11,
 *     "datareaders": [],
 *     "datawriters": []
 *   },
 *   "topic": {
 *     "ID": 12,
 *     "datareaders": ["datareader1", "datareader"],
 *     "datawriters": ["datawriter1", "datawriter"]
 *   },
 *   "datareader1": {
 *     "ID": 13,
 *     "locators": ["reader_locator1"]
 *   }
 *   "datareader": {
 *     "ID": 14,
 *     "locators": ["reader_locator1", "reader_locator"]
 *   }
 *   "datawriter1": {
 *     "ID": 15,
 *     "locators": ["writer_locator1"]
 *   }
 *   "datawriter": {
 *     "ID": 16,
 *     "locators": ["writer_locator1", "writer_locator"]
 *   }
 *   "reader_locator1": {
 *     "ID": 17
 *   }
 *   "reader_locator": {
 *     "ID": 18
 *   }
 *   "writer_locator1": {
 *     "ID": 19
 *   }
 *   "writer_locator": {
 *     "ID": 20
 *   }
 * }
 * \endcode
 */
class statistics_backend_tests : public ::testing::Test
{
public:
    void SetUp()
    {
        populate_database();
    }

    void populate_database()
    {
        host = std::make_shared<Host>("host");
        db.insert(host);

        user = std::make_shared<User>("user", host);
        db.insert(user);

        process = std::make_shared<Process>("process", "345", user);
        db.insert(process);

        domain = std::make_shared<Domain>("domain");
        db.insert(domain);

        participant = std::make_shared<DomainParticipant>("participant", "qos", "5.6.7.8", nullptr, domain);
        db.insert(participant);
        db.link_participant_with_process(participant->id, process->id);

        topic = std::make_shared<Topic>("topic", "type", domain);
        db.insert(topic);

        datareader = std::make_shared<DataReader>("datareader", "qos", "15.16.17.18", participant, topic);
        reader_locator = std::make_shared<Locator>("reader_locator");
        reader_locator->id = db.generate_entity_id();
        datareader->locators[reader_locator->id] = reader_locator;
        datareader->locators[reader_locator->id] = reader_locator;
        db.insert(datareader);

        datawriter = std::make_shared<DataWriter>("datawriter", "qos", "25.26.27.28", participant, topic);
        writer_locator = std::make_shared<Locator>("writer_locator");
        writer_locator->id = db.generate_entity_id();
        datawriter->locators[writer_locator->id] = writer_locator;
        datawriter->locators[writer_locator->id] = writer_locator;
        db.insert(datawriter);
    }

    Database db;
    std::shared_ptr<Host> host;
    std::shared_ptr<User> user;
    std::shared_ptr<Process> process;
    std::shared_ptr<Domain> domain;
    std::shared_ptr<DomainParticipant> participant;
    std::shared_ptr<Topic> topic;
    std::shared_ptr<DataReader> datareader;
    std::shared_ptr<Locator> reader_locator;
    std::shared_ptr<DataWriter> datawriter;
    std::shared_ptr<Locator> writer_locator;
};

class StatisticsBackendTest : public StatisticsBackend
{
public:

     static void set_database(Database * db)
     {
          database_ = db;
     }
};

// Check the get_type StatisticsBackend method
TEST_F(statistics_backend_tests, get_type)
{
	StatisticsBackendTest::set_database(&db);
	ASSERT_EQ(StatisticsBackendTest::get_type(host->id), host->kind);
	ASSERT_EQ(StatisticsBackendTest::get_type(user->id), user->kind);
	ASSERT_EQ(StatisticsBackendTest::get_type(process->id), process->kind);
	ASSERT_EQ(StatisticsBackendTest::get_type(domain->id), domain->kind);
	ASSERT_EQ(StatisticsBackendTest::get_type(participant->id), participant->kind);
	ASSERT_EQ(StatisticsBackendTest::get_type(topic->id), topic->kind);
	ASSERT_EQ(StatisticsBackendTest::get_type(datareader->id), datareader->kind);
	ASSERT_EQ(StatisticsBackendTest::get_type(reader_locator->id), reader_locator->kind);
	ASSERT_EQ(StatisticsBackendTest::get_type(datawriter->id), datawriter->kind);
	ASSERT_EQ(StatisticsBackendTest::get_type(writer_locator->id), writer_locator->kind);

	ASSERT_THROW(StatisticsBackendTest::get_type(EntityId::all()),BadParameter);
	ASSERT_THROW(StatisticsBackendTest::get_type(EntityId::invalid()),BadParameter);
	ASSERT_THROW(StatisticsBackendTest::get_type(EntityId(1234)),BadParameter);
}

// Check the get_data_supported_entity_kinds StatisticsBackend method
TEST_F(statistics_backend_tests, get_data_supported_entity_kinds)
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
