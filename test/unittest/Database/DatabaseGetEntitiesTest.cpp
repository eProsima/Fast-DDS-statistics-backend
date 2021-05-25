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

#include <algorithm>
#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "gtest/gtest.h"

#include <fastdds-statistics-backend/exception/Exception.hpp>
#include <fastdds-statistics-backend/types/EntityId.hpp>

#include <database/database.hpp>
#include <database/entities.hpp>
#include <database/samples.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

/**
 * @brief Fixture for the get_entities method tests
 *
 * \c get_entities retrieves all the entities of a given kind that are reachable from a given entity.
 * The casuistry for this functionality is rather complex,
 * and the tests need a populated database with several entity combinations in order to be able to
 * test all this casuistry.
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
 *   "host2": {
 *     "ID": 2,
 *     "users": ["user1", "user2"]
 *   },
 *   "user1": {
 *     "ID": 3,
 *     "processes": []
 *   },
 *   "user2": {
 *     "ID": 4,
 *     "processes": ["process1", "process2"]
 *   },
 *   "process1": {
 *     "ID": 5,
 *     "participants": []
 *   },
 *   "process2": {
 *     "ID": 6,
 *     "participants": ["participant1", "participant2"]
 *   },
 *   "domain1": {
 *     "ID": 7,
 *     "participants": [],
 *     "topics": []
 *   },
 *   "domain2": {
 *     "ID": 8,
 *     "participants": ["participant1", "participant2"],
 *     "topics": ["topic1", "topic2"]
 *   },
 *   "participant1": {
 *     "ID": 9,
 *     "datareaders": [],
 *     "datawriters": []
 *   },
 *   "participant2": {
 *     "ID": 10,
 *     "datareaders": ["datareader1", "datareader2"],
 *     "datawriters": ["datawriter1", "datawriter2"]
 *   },
 *   "topic1": {
 *     "ID": 11,
 *     "datareaders": [],
 *     "datawriters": []
 *   },
 *   "topic2": {
 *     "ID": 12,
 *     "datareaders": ["datareader1", "datareader2"],
 *     "datawriters": ["datawriter1", "datawriter2"]
 *   },
 *   "datareader1": {
 *     "ID": 13,
 *     "locators": ["reader_locator1"]
 *   }
 *   "datareader2": {
 *     "ID": 14,
 *     "locators": ["reader_locator1", "reader_locator2"]
 *   }
 *   "datawriter1": {
 *     "ID": 15,
 *     "locators": ["writer_locator1"]
 *   }
 *   "datawriter2": {
 *     "ID": 16,
 *     "locators": ["writer_locator1", "writer_locator2"]
 *   }
 *   "reader_locator1": {
 *     "ID": 17
 *   }
 *   "reader_locator2": {
 *     "ID": 18
 *   }
 *   "writer_locator1": {
 *     "ID": 19
 *   }
 *   "writer_locator2": {
 *     "ID": 20
 *   }
 * }
 * \endcode
 *
 *
 * Parameteres to the tests are:
 *  - std::get<0>(GetParam()) The EntityKind we are looking for
 *  - std::get<1>(GetParam()) The unique identifier of the origin Entity, as given by the fixture/testing
 *  - std::get<2>(GetParam()) A list containing the unique identifiers of the entities expected in the result
 */
class database_get_entities_tests : public ::testing::TestWithParam<std::tuple<EntityKind, size_t, std::vector<size_t>>>
{
public:

    typedef size_t TestId;

    void SetUp()
    {
        populate_database();
    }

    void populate_database()
    {
        auto host1 = std::make_shared<Host>("host1");
        db.insert(host1);
        entities[1] = host1;

        auto host2 = std::make_shared<Host>("host2");
        db.insert(host2);
        entities[2] = host2;

        auto user1 = std::make_shared<User>("user1", host2);
        db.insert(user1);
        entities[3] = user1;

        auto user2 = std::make_shared<User>("user2", host2);
        db.insert(user2);
        entities[4] = user2;

        auto process1 = std::make_shared<Process>("process1", "123", user2);
        db.insert(process1);
        entities[5] = process1;

        auto process2 = std::make_shared<Process>("process2", "345", user2);
        db.insert(process2);
        entities[6] = process2;

        auto domain1 = std::make_shared<Domain>("domain1");
        db.insert(domain1);
        entities[7] = domain1;

        auto domain2 = std::make_shared<Domain>("domain2");
        db.insert(domain2);
        entities[8] = domain2;

        auto participant1 = std::make_shared<DomainParticipant>("participant1", "qos", "1.2.3.4", nullptr, domain2);
        db.insert(participant1);
        db.link_participant_with_process(participant1->id, process2->id);
        entities[9] = participant1;

        auto participant2 = std::make_shared<DomainParticipant>("participant2", "qos", "5.6.7.8", nullptr, domain2);
        db.insert(participant2);
        db.link_participant_with_process(participant2->id, process2->id);
        entities[10] = participant2;

        auto topic1 = std::make_shared<Topic>("topic1", "type", domain2);
        db.insert(topic1);
        entities[11] = topic1;

        auto topic2 = std::make_shared<Topic>("topic2", "type", domain2);
        db.insert(topic2);
        entities[12] = topic2;

        auto datareader1 = std::make_shared<DataReader>("datareader1", "qos", "11.12.13.14", participant2, topic2);
        auto reader_locator1 = std::make_shared<Locator>("reader_locator1");
        reader_locator1->id = db.generate_entity_id();
        datareader1->locators[reader_locator1->id] = reader_locator1;
        db.insert(datareader1);
        entities[13] = datareader1;
        entities[17] = reader_locator1;

        auto datareader2 = std::make_shared<DataReader>("datareader2", "qos", "15.16.17.18", participant2, topic2);
        auto reader_locator2 = std::make_shared<Locator>("reader_locator2");
        reader_locator2->id = db.generate_entity_id();
        datareader2->locators[reader_locator1->id] = reader_locator1;
        datareader2->locators[reader_locator2->id] = reader_locator2;
        db.insert(datareader2);
        entities[14] = datareader2;
        entities[18] = reader_locator2;

        auto datawriter1 = std::make_shared<DataWriter>("datawriter1", "qos", "21.22.23.24", participant2, topic2);
        auto writer_locator1 = std::make_shared<Locator>("writer_locator1");
        writer_locator1->id = db.generate_entity_id();
        datawriter1->locators[writer_locator1->id] = writer_locator1;
        db.insert(datawriter1);
        entities[15] = datawriter1;
        entities[19] = writer_locator1;

        auto datawriter2 = std::make_shared<DataWriter>("datawriter2", "qos", "25.26.27.28", participant2, topic2);
        auto writer_locator2 = std::make_shared<Locator>("writer_locator2");
        writer_locator2->id = db.generate_entity_id();
        datawriter2->locators[writer_locator1->id] = writer_locator1;
        datawriter2->locators[writer_locator2->id] = writer_locator2;
        db.insert(datawriter2);
        entities[16] = datawriter2;
        entities[20] = writer_locator2;
    }

    Database db;
    std::map<TestId, std::shared_ptr<const Entity>> entities;
};

TEST_P(database_get_entities_tests, get_entities)
{
    EntityKind kind = std::get<0>(GetParam());
    EntityId origin = entities[std::get<1>(GetParam())]->id;
    std::vector<std::shared_ptr<const Entity>> expected;
    for (auto it : std::get<2>(GetParam()))
    {
        expected.push_back(entities[it]);
    }

    EXPECT_THROW(db.get_entities(kind, db.generate_entity_id()), BadParameter);
    EXPECT_THROW(db.get_entities(EntityKind::INVALID, origin), BadParameter);


    auto result = db.get_entities(kind, origin);
    ASSERT_EQ(expected.size(), result.size());
    std::sort(expected.begin(), expected.end());
    std::sort(result.begin(), result.end());
    for (size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(expected[i].get(), result[i].get());
    }
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_SUITE_P(x, y, z)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_CASE_P(x, y, z)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(
    database_get_entities_tests,
    database_get_entities_tests,
    ::testing::Values(
        // HOST - HOST
        std::make_tuple(EntityKind::HOST, 2, std::vector<size_t> { 2 }),
        std::make_tuple(EntityKind::HOST, 1, std::vector<size_t> { 1 }),
        // HOST - USER
        std::make_tuple(EntityKind::USER, 2, std::vector<size_t> { 3, 4 }),
        // HOST - USER: none
        std::make_tuple(EntityKind::USER, 1, std::vector<size_t> { }),
        // HOST - PROCESS
        std::make_tuple(EntityKind::PROCESS, 2, std::vector<size_t> { 5, 6 }),
        // HOST - PROCESS: none
        std::make_tuple(EntityKind::PROCESS, 1, std::vector<size_t> { }),
        // HOST - DOMAIN_DDS
        std::make_tuple(EntityKind::DOMAIN_DDS, 2, std::vector<size_t> { 8 }),
        // HOST - DOMAIN_DDS: none
        std::make_tuple(EntityKind::DOMAIN_DDS, 1, std::vector<size_t> { }),
        // HOST - PARTICIPANT
        std::make_tuple(EntityKind::PARTICIPANT, 2, std::vector<size_t> { 9, 10 }),
        // HOST - PARTICIPANT: none
        std::make_tuple(EntityKind::PARTICIPANT, 1, std::vector<size_t> { }),
        // HOST - TOPIC
        std::make_tuple(EntityKind::TOPIC, 2, std::vector<size_t> { 12 }),
        // HOST - TOPIC: none
        std::make_tuple(EntityKind::TOPIC, 1, std::vector<size_t> { }),
        // HOST - DATAWRITER
        std::make_tuple(EntityKind::DATAWRITER, 2, std::vector<size_t> { 15, 16 }),
        // HOST - DATAWRITER: none
        std::make_tuple(EntityKind::DATAWRITER, 1, std::vector<size_t> { }),
        // HOST - DATAREADER
        std::make_tuple(EntityKind::DATAREADER, 2, std::vector<size_t> { 13, 14 }),
        // HOST - DATAREADER: none
        std::make_tuple(EntityKind::DATAREADER, 1, std::vector<size_t> { }),
        // HOST - LOCATOR
        std::make_tuple(EntityKind::LOCATOR, 2, std::vector<size_t> { 17, 18, 19, 20 }),
        // HOST - LOCATOR: none
        std::make_tuple(EntityKind::LOCATOR, 1, std::vector<size_t> { }),

        // USER - HOST
        std::make_tuple(EntityKind::HOST, 4, std::vector<size_t> { 2 }),
        std::make_tuple(EntityKind::HOST, 3, std::vector<size_t> { 2 }),
        // USER - USER
        std::make_tuple(EntityKind::USER, 4, std::vector<size_t> { 4 }),
        std::make_tuple(EntityKind::USER, 3, std::vector<size_t> { 3 }),
        // USER - PROCESS
        std::make_tuple(EntityKind::PROCESS, 4, std::vector<size_t> { 5, 6 }),
        // USER - PROCESS: none
        std::make_tuple(EntityKind::PROCESS, 3, std::vector<size_t> { }),
        // USER - DOMAIN_DDS
        std::make_tuple(EntityKind::DOMAIN_DDS, 4, std::vector<size_t> { 8 }),
        // USER - DOMAIN_DDS: none
        std::make_tuple(EntityKind::DOMAIN_DDS, 3, std::vector<size_t> { }),
        // USER - PARTICIPANT
        std::make_tuple(EntityKind::PARTICIPANT, 4, std::vector<size_t> { 9, 10 }),
        // USER - PARTICIPANT: none
        std::make_tuple(EntityKind::PARTICIPANT, 3, std::vector<size_t> { }),
        // USER - TOPIC
        std::make_tuple(EntityKind::TOPIC, 4, std::vector<size_t> { 12 }),
        // USER - TOPIC: none
        std::make_tuple(EntityKind::TOPIC, 3, std::vector<size_t> { }),
        // USER - DATAWRITER
        std::make_tuple(EntityKind::DATAWRITER, 4, std::vector<size_t> { 15, 16 }),
        // USER - DATAWRITER: none
        std::make_tuple(EntityKind::DATAWRITER, 3, std::vector<size_t> { }),
        // USER - DATAREADER
        std::make_tuple(EntityKind::DATAREADER, 4, std::vector<size_t> { 13, 14 }),
        // USER - DATAREADER: none
        std::make_tuple(EntityKind::DATAREADER, 3, std::vector<size_t> { }),
        // USER - LOCATOR
        std::make_tuple(EntityKind::LOCATOR, 4, std::vector<size_t> { 17, 18, 19, 20 }),
        // USER - LOCATOR: none
        std::make_tuple(EntityKind::LOCATOR, 3, std::vector<size_t> { }),

        // PROCESS - HOST
        std::make_tuple(EntityKind::HOST, 6, std::vector<size_t> { 2 }),
        std::make_tuple(EntityKind::HOST, 5, std::vector<size_t> { 2 }),
        // PROCESS - USER
        std::make_tuple(EntityKind::USER, 6, std::vector<size_t> { 4 }),
        std::make_tuple(EntityKind::USER, 5, std::vector<size_t> { 4 }),
        // PROCESS - PROCESS
        std::make_tuple(EntityKind::PROCESS, 6, std::vector<size_t> { 6 }),
        std::make_tuple(EntityKind::PROCESS, 5, std::vector<size_t> { 5 }),
        // PROCESS - DOMAIN_DDS
        std::make_tuple(EntityKind::DOMAIN_DDS, 6, std::vector<size_t> { 8 }),
        // PROCESS - DOMAIN_DDS: none
        std::make_tuple(EntityKind::DOMAIN_DDS, 5, std::vector<size_t> { }),
        // PROCESS - PARTICIPANT
        std::make_tuple(EntityKind::PARTICIPANT, 6, std::vector<size_t> { 9, 10 }),
        // PROCESS - PARTICIPANT: none
        std::make_tuple(EntityKind::PARTICIPANT, 5, std::vector<size_t> { }),
        // PROCESS - TOPIC
        std::make_tuple(EntityKind::TOPIC, 6, std::vector<size_t> { 12 }),
        // PROCESS - TOPIC: none
        std::make_tuple(EntityKind::TOPIC, 5, std::vector<size_t> { }),
        // PROCESS - DATAWRITER
        std::make_tuple(EntityKind::DATAWRITER, 6, std::vector<size_t> { 15, 16 }),
        // PROCESS - DATAWRITER: none
        std::make_tuple(EntityKind::DATAWRITER, 5, std::vector<size_t> { }),
        // PROCESS - DATAREADER
        std::make_tuple(EntityKind::DATAREADER, 6, std::vector<size_t> { 13, 14 }),
        // PROCESS - DATAREADER: none
        std::make_tuple(EntityKind::DATAREADER, 5, std::vector<size_t> { }),
        // PROCESS - LOCATOR
        std::make_tuple(EntityKind::LOCATOR, 6, std::vector<size_t> { 17, 18, 19, 20 }),
        // PROCESS - LOCATOR: none
        std::make_tuple(EntityKind::LOCATOR, 5, std::vector<size_t> { }),

        // DOMAIN_DDS - HOST
        std::make_tuple(EntityKind::HOST, 8, std::vector<size_t> { 2 }),
        // DOMAIN_DDS - HOST: none
        std::make_tuple(EntityKind::HOST, 7, std::vector<size_t> { }),
        // DOMAIN_DDS - USER
        std::make_tuple(EntityKind::USER, 8, std::vector<size_t> { 4 }),
        // DOMAIN_DDS - USER: none
        std::make_tuple(EntityKind::USER, 7, std::vector<size_t> { }),
        // DOMAIN_DDS - PROCESS
        std::make_tuple(EntityKind::PROCESS, 8, std::vector<size_t> { 6 }),
        // DOMAIN_DDS - PROCESS: none
        std::make_tuple(EntityKind::PROCESS, 7, std::vector<size_t> { }),
        // DOMAIN_DDS - DOMAIN_DDS
        std::make_tuple(EntityKind::DOMAIN_DDS, 8, std::vector<size_t> { 8 }),
        std::make_tuple(EntityKind::DOMAIN_DDS, 7, std::vector<size_t> { 7 }),
        // DOMAIN_DDS - PARTICIPANT
        std::make_tuple(EntityKind::PARTICIPANT, 8, std::vector<size_t> { 9, 10 }),
        // DOMAIN_DDS - PARTICIPANT: none
        std::make_tuple(EntityKind::PARTICIPANT, 7, std::vector<size_t> { }),
        // DOMAIN_DDS - TOPIC
        std::make_tuple(EntityKind::TOPIC, 8, std::vector<size_t> { 11, 12 }),
        // DOMAIN_DDS - TOPIC: none
        std::make_tuple(EntityKind::TOPIC, 7, std::vector<size_t> { }),
        // DOMAIN_DDS - DATAWRITER
        std::make_tuple(EntityKind::DATAWRITER, 8, std::vector<size_t> { 15, 16 }),
        // DOMAIN_DDS - DATAWRITER: none
        std::make_tuple(EntityKind::DATAWRITER, 7, std::vector<size_t> { }),
        // DOMAIN_DDS - DATAREADER
        std::make_tuple(EntityKind::DATAREADER, 8, std::vector<size_t> { 13, 14 }),
        // DOMAIN_DDS - DATAREADER: none
        std::make_tuple(EntityKind::DATAREADER, 7, std::vector<size_t> { }),
        // DOMAIN_DDS - LOCATOR
        std::make_tuple(EntityKind::LOCATOR, 8, std::vector<size_t> { 17, 18, 19, 20 }),
        // DOMAIN_DDS - LOCATOR: none
        std::make_tuple(EntityKind::LOCATOR, 7, std::vector<size_t> { }),

        // PARTICIPANT - HOST
        std::make_tuple(EntityKind::HOST, 10, std::vector<size_t> { 2 }),
        std::make_tuple(EntityKind::HOST, 9, std::vector<size_t> { 2 }),
        // PARTICIPANT - USER
        std::make_tuple(EntityKind::USER, 10, std::vector<size_t> { 4 }),
        std::make_tuple(EntityKind::USER, 9, std::vector<size_t> { 4 }),
        // PARTICIPANT - PROCESS
        std::make_tuple(EntityKind::PROCESS, 10, std::vector<size_t> { 6 }),
        std::make_tuple(EntityKind::PROCESS, 9, std::vector<size_t> { 6 }),
        // PARTICIPANT - DOMAIN_DDS
        std::make_tuple(EntityKind::DOMAIN_DDS, 10, std::vector<size_t> { 8 }),
        std::make_tuple(EntityKind::DOMAIN_DDS, 9, std::vector<size_t> { 8 }),
        // PARTICIPANT - PARTICIPANT
        std::make_tuple(EntityKind::PARTICIPANT, 10, std::vector<size_t> { 10 }),
        std::make_tuple(EntityKind::PARTICIPANT, 9, std::vector<size_t> { 9 }),
        // PARTICIPANT - TOPIC
        std::make_tuple(EntityKind::TOPIC, 10, std::vector<size_t> { 12 }),
        // PARTICIPANT - TOPIC: none
        std::make_tuple(EntityKind::TOPIC, 9, std::vector<size_t> { }),
        // PARTICIPANT - DATAWRITER
        std::make_tuple(EntityKind::DATAWRITER, 10, std::vector<size_t> { 15, 16 }),
        // PARTICIPANT - DATAWRITER: none
        std::make_tuple(EntityKind::DATAWRITER, 9, std::vector<size_t> { }),
        // PARTICIPANT - DATAREADER
        std::make_tuple(EntityKind::DATAREADER, 10, std::vector<size_t> { 13, 14 }),
        // PARTICIPANT - DATAREADER: none
        std::make_tuple(EntityKind::DATAREADER, 9, std::vector<size_t> { }),
        // PARTICIPANT - LOCATOR
        std::make_tuple(EntityKind::LOCATOR, 10, std::vector<size_t> { 17, 18, 19, 20 }),
        // PARTICIPANT - LOCATOR: none
        std::make_tuple(EntityKind::LOCATOR, 9, std::vector<size_t> { }),

        // TOPIC - HOST
        std::make_tuple(EntityKind::HOST, 12, std::vector<size_t> { 2 }),
        // TOPIC - HOST: none
        std::make_tuple(EntityKind::HOST, 11, std::vector<size_t> { }),
        // TOPIC - USER
        std::make_tuple(EntityKind::USER, 12, std::vector<size_t> { 4 }),
        // TOPIC - USER: none
        std::make_tuple(EntityKind::USER, 11, std::vector<size_t> { }),
        // TOPIC - PROCESS
        std::make_tuple(EntityKind::PROCESS, 12, std::vector<size_t> { 6 }),
        // TOPIC - PROCESS: none
        std::make_tuple(EntityKind::PROCESS, 11, std::vector<size_t> { }),
        // TOPIC - DOMAIN_DDS
        std::make_tuple(EntityKind::DOMAIN_DDS, 12, std::vector<size_t> { 8 }),
        std::make_tuple(EntityKind::DOMAIN_DDS, 11, std::vector<size_t> { 8 }),
        // TOPIC - PARTICIPANT
        std::make_tuple(EntityKind::PARTICIPANT, 12, std::vector<size_t> { 10 }),
        // TOPIC - PARTICIPANT: none
        std::make_tuple(EntityKind::PARTICIPANT, 11, std::vector<size_t> { }),
        // TOPIC - TOPIC
        std::make_tuple(EntityKind::TOPIC, 12, std::vector<size_t> { 12 }),
        std::make_tuple(EntityKind::TOPIC, 11, std::vector<size_t> { 11 }),
        // TOPIC - DATAWRITER
        std::make_tuple(EntityKind::DATAWRITER, 12, std::vector<size_t> { 15, 16 }),
        // TOPIC - DATAWRITER: none
        std::make_tuple(EntityKind::DATAWRITER, 11, std::vector<size_t> { }),
        // TOPIC - DATAREADER
        std::make_tuple(EntityKind::DATAREADER, 12, std::vector<size_t> { 13, 14 }),
        // TOPIC - DATAREADER: none
        std::make_tuple(EntityKind::DATAREADER, 11, std::vector<size_t> { }),
        // TOPIC - LOCATOR
        std::make_tuple(EntityKind::LOCATOR, 12, std::vector<size_t> { 17, 18, 19, 20 }),
        // TOPIC - LOCATOR: none
        std::make_tuple(EntityKind::LOCATOR, 11, std::vector<size_t> { }),

        // DATAREADER - HOST
        std::make_tuple(EntityKind::HOST, 14, std::vector<size_t> { 2 }),
        std::make_tuple(EntityKind::HOST, 13, std::vector<size_t> { 2 }),
        // DATAREADER - USER
        std::make_tuple(EntityKind::USER, 14, std::vector<size_t> { 4 }),
        std::make_tuple(EntityKind::USER, 13, std::vector<size_t> { 4 }),
        // DATAREADER - PROCESS
        std::make_tuple(EntityKind::PROCESS, 14, std::vector<size_t> { 6 }),
        std::make_tuple(EntityKind::PROCESS, 13, std::vector<size_t> { 6 }),
        // DATAREADER - DOMAIN_DDS
        std::make_tuple(EntityKind::DOMAIN_DDS, 14, std::vector<size_t> { 8 }),
        std::make_tuple(EntityKind::DOMAIN_DDS, 13, std::vector<size_t> { 8 }),
        // DATAREADER - PARTICIPANT
        std::make_tuple(EntityKind::PARTICIPANT, 14, std::vector<size_t> { 10 }),
        std::make_tuple(EntityKind::PARTICIPANT, 13, std::vector<size_t> { 10 }),
        // DATAREADER - TOPIC
        std::make_tuple(EntityKind::TOPIC, 14, std::vector<size_t> { 12 }),
        std::make_tuple(EntityKind::TOPIC, 13, std::vector<size_t> { 12 }),
        // DATAREADER - DATAWRITER
        std::make_tuple(EntityKind::DATAWRITER, 14, std::vector<size_t> { 15, 16 }),
        std::make_tuple(EntityKind::DATAWRITER, 13, std::vector<size_t> { 15, 16}),
        // DATAREADER - DATAREADER
        std::make_tuple(EntityKind::DATAREADER, 14, std::vector<size_t> { 14 }),
        std::make_tuple(EntityKind::DATAREADER, 13, std::vector<size_t> { 13 }),
        // DATAREADER - LOCATOR
        std::make_tuple(EntityKind::LOCATOR, 14, std::vector<size_t> { 17, 18 }),
        // DATAREADER - LOCATOR
        std::make_tuple(EntityKind::LOCATOR, 13, std::vector<size_t> { 17 }),

        // DATAWRITER - HOST
        std::make_tuple(EntityKind::HOST, 16, std::vector<size_t> { 2 }),
        std::make_tuple(EntityKind::HOST, 15, std::vector<size_t> { 2 }),
        // DATAWRITER - USER
        std::make_tuple(EntityKind::USER, 16, std::vector<size_t> { 4 }),
        std::make_tuple(EntityKind::USER, 15, std::vector<size_t> { 4 }),
        // DATAWRITER - PROCESS
        std::make_tuple(EntityKind::PROCESS, 16, std::vector<size_t> { 6 }),
        std::make_tuple(EntityKind::PROCESS, 15, std::vector<size_t> { 6 }),
        // DATAWRITER - DOMAIN_DDS
        std::make_tuple(EntityKind::DOMAIN_DDS, 16, std::vector<size_t> { 8 }),
        std::make_tuple(EntityKind::DOMAIN_DDS, 15, std::vector<size_t> { 8 }),
        // DATAWRITER - PARTICIPANT
        std::make_tuple(EntityKind::PARTICIPANT, 16, std::vector<size_t> { 10 }),
        std::make_tuple(EntityKind::PARTICIPANT, 15, std::vector<size_t> { 10 }),
        // DATAWRITER - TOPIC
        std::make_tuple(EntityKind::TOPIC, 16, std::vector<size_t> { 12 }),
        std::make_tuple(EntityKind::TOPIC, 15, std::vector<size_t> { 12 }),
        // DATAWRITER - DATAWRITER
        std::make_tuple(EntityKind::DATAWRITER, 16, std::vector<size_t> { 16 }),
        std::make_tuple(EntityKind::DATAWRITER, 15, std::vector<size_t> { 15}),
        // DATAWRITER - DATAREADER
        std::make_tuple(EntityKind::DATAREADER, 16, std::vector<size_t> { 13, 14 }),
        std::make_tuple(EntityKind::DATAREADER, 15, std::vector<size_t> { 13, 14 }),
        // DATAWRITER - LOCATOR
        std::make_tuple(EntityKind::LOCATOR, 16, std::vector<size_t> { 19, 20 }),
        // DATAWRITER - LOCATOR
        std::make_tuple(EntityKind::LOCATOR, 15, std::vector<size_t> { 19 }),

        // LOCATOR - HOST
        std::make_tuple(EntityKind::HOST, 17, std::vector<size_t> { 2 }),
        std::make_tuple(EntityKind::HOST, 18, std::vector<size_t> { 2 }),
        std::make_tuple(EntityKind::HOST, 19, std::vector<size_t> { 2 }),
        std::make_tuple(EntityKind::HOST, 20, std::vector<size_t> { 2 }),
        // LOCATOR - USER
        std::make_tuple(EntityKind::USER, 17, std::vector<size_t> { 4 }),
        std::make_tuple(EntityKind::USER, 18, std::vector<size_t> { 4 }),
        std::make_tuple(EntityKind::USER, 19, std::vector<size_t> { 4 }),
        std::make_tuple(EntityKind::USER, 20, std::vector<size_t> { 4 }),
        // LOCATOR - PROCESS
        std::make_tuple(EntityKind::PROCESS, 17, std::vector<size_t> { 6 }),
        std::make_tuple(EntityKind::PROCESS, 18, std::vector<size_t> { 6 }),
        std::make_tuple(EntityKind::PROCESS, 19, std::vector<size_t> { 6 }),
        std::make_tuple(EntityKind::PROCESS, 20, std::vector<size_t> { 6 }),
        // LOCATOR - DOMAIN_DDS
        std::make_tuple(EntityKind::DOMAIN_DDS, 17, std::vector<size_t> { 8 }),
        std::make_tuple(EntityKind::DOMAIN_DDS, 18, std::vector<size_t> { 8 }),
        std::make_tuple(EntityKind::DOMAIN_DDS, 19, std::vector<size_t> { 8 }),
        std::make_tuple(EntityKind::DOMAIN_DDS, 20, std::vector<size_t> { 8 }),
        // LOCATOR - PARTICIPANT
        std::make_tuple(EntityKind::PARTICIPANT, 17, std::vector<size_t> { 10 }),
        std::make_tuple(EntityKind::PARTICIPANT, 18, std::vector<size_t> { 10 }),
        std::make_tuple(EntityKind::PARTICIPANT, 19, std::vector<size_t> { 10 }),
        std::make_tuple(EntityKind::PARTICIPANT, 20, std::vector<size_t> { 10 }),
        // LOCATOR - TOPIC
        std::make_tuple(EntityKind::TOPIC, 17, std::vector<size_t> { 12 }),
        std::make_tuple(EntityKind::TOPIC, 18, std::vector<size_t> { 12 }),
        std::make_tuple(EntityKind::TOPIC, 19, std::vector<size_t> { 12 }),
        std::make_tuple(EntityKind::TOPIC, 20, std::vector<size_t> { 12 }),
        // LOCATOR - DATAWRITER
        std::make_tuple(EntityKind::DATAWRITER, 17, std::vector<size_t> { }),
        std::make_tuple(EntityKind::DATAWRITER, 18, std::vector<size_t> { }),
        std::make_tuple(EntityKind::DATAWRITER, 19, std::vector<size_t> { 15, 16 }),
        std::make_tuple(EntityKind::DATAWRITER, 20, std::vector<size_t> { 16 }),
        // LOCATOR - DATAREADER
        std::make_tuple(EntityKind::DATAREADER, 17, std::vector<size_t> { 13, 14 }),
        std::make_tuple(EntityKind::DATAREADER, 18, std::vector<size_t> { 14 }),
        std::make_tuple(EntityKind::DATAREADER, 19, std::vector<size_t> { }),
        std::make_tuple(EntityKind::DATAREADER, 20, std::vector<size_t> { }),
        // LOCATOR - LOCATOR
        std::make_tuple(EntityKind::LOCATOR, 17, std::vector<size_t> { 17 }),
        std::make_tuple(EntityKind::LOCATOR, 18, std::vector<size_t> { 18 }),
        std::make_tuple(EntityKind::LOCATOR, 19, std::vector<size_t> { 19 }),
        std::make_tuple(EntityKind::LOCATOR, 20, std::vector<size_t> { 20 })
        ));

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
