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

#include <DatabaseUtils.hpp>

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
 * Parameteres to the tests are:
 *  - std::get<0>(GetParam()) The EntityKind we are looking for
 *  - std::get<1>(GetParam()) The unique identifier of the origin Entity, as given by the fixture/testing
 *  - std::get<2>(GetParam()) A list containing the unique identifiers of the entities expected in the result
 */
class database_get_entities_tests : public ::testing::TestWithParam<std::tuple<EntityKind, size_t, std::vector<size_t>>>
{
public:

    typedef uint32_t TestId;

    void SetUp()
    {
        entities = PopulateDatabase::populate_database(db);
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

TEST_P(database_get_entities_tests, get_entity_ids)
{
    EntityKind kind = std::get<0>(GetParam());
    EntityId origin = entities[std::get<1>(GetParam())]->id;
    std::vector<EntityId> expected;
    for (auto it : std::get<2>(GetParam()))
    {
        expected.push_back(entities[it]->id);
    }

    EXPECT_THROW(db.get_entity_ids(kind, db.generate_entity_id()), BadParameter);
    EXPECT_THROW(db.get_entity_ids(EntityKind::INVALID, origin), BadParameter);


    auto result = db.get_entity_ids(kind, origin);
    ASSERT_EQ(expected.size(), result.size());
    std::sort(expected.begin(), expected.end());
    std::sort(result.begin(), result.end());
    for (size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(expected[i], result[i]);
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
        // ALL - HOST
        std::make_tuple(EntityKind::HOST, 0, std::vector<size_t>{1, 2}),
        // ALL - USER
        std::make_tuple(EntityKind::USER, 0, std::vector<size_t>{3, 4}),
        // ALL - PROCESS
        std::make_tuple(EntityKind::PROCESS, 0, std::vector<size_t>{5, 6}),
        // ALL - DOMAIN
        std::make_tuple(EntityKind::DOMAIN, 0, std::vector<size_t>{7, 8}),
        // ALL - PARTICIPANT
        std::make_tuple(EntityKind::PARTICIPANT, 0, std::vector<size_t>{9, 10}),
        // ALL - TOPIC
        std::make_tuple(EntityKind::TOPIC, 0, std::vector<size_t>{11, 12}),
        // ALL - DATAREADER
        std::make_tuple(EntityKind::DATAREADER, 0, std::vector<size_t>{13, 14}),
        // ALL - DATAWRITER
        std::make_tuple(EntityKind::DATAWRITER, 0, std::vector<size_t>{15, 16}),
        // ALL - LOCATOR
        std::make_tuple(EntityKind::LOCATOR, 0, std::vector<size_t>{17, 18, 19, 20}),
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
        // HOST - DOMAIN
        std::make_tuple(EntityKind::DOMAIN, 2, std::vector<size_t> { 8 }),
        // HOST - DOMAIN: none
        std::make_tuple(EntityKind::DOMAIN, 1, std::vector<size_t> { }),
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
        // USER - DOMAIN
        std::make_tuple(EntityKind::DOMAIN, 4, std::vector<size_t> { 8 }),
        // USER - DOMAIN: none
        std::make_tuple(EntityKind::DOMAIN, 3, std::vector<size_t> { }),
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
        // PROCESS - DOMAIN
        std::make_tuple(EntityKind::DOMAIN, 6, std::vector<size_t> { 8 }),
        // PROCESS - DOMAIN: none
        std::make_tuple(EntityKind::DOMAIN, 5, std::vector<size_t> { }),
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

        // DOMAIN - HOST
        std::make_tuple(EntityKind::HOST, 8, std::vector<size_t> { 2 }),
        // DOMAIN - HOST: none
        std::make_tuple(EntityKind::HOST, 7, std::vector<size_t> { }),
        // DOMAIN - USER
        std::make_tuple(EntityKind::USER, 8, std::vector<size_t> { 4 }),
        // DOMAIN - USER: none
        std::make_tuple(EntityKind::USER, 7, std::vector<size_t> { }),
        // DOMAIN - PROCESS
        std::make_tuple(EntityKind::PROCESS, 8, std::vector<size_t> { 6 }),
        // DOMAIN - PROCESS: none
        std::make_tuple(EntityKind::PROCESS, 7, std::vector<size_t> { }),
        // DOMAIN - DOMAIN
        std::make_tuple(EntityKind::DOMAIN, 8, std::vector<size_t> { 8 }),
        std::make_tuple(EntityKind::DOMAIN, 7, std::vector<size_t> { 7 }),
        // DOMAIN - PARTICIPANT
        std::make_tuple(EntityKind::PARTICIPANT, 8, std::vector<size_t> { 9, 10 }),
        // DOMAIN - PARTICIPANT: none
        std::make_tuple(EntityKind::PARTICIPANT, 7, std::vector<size_t> { }),
        // DOMAIN - TOPIC
        std::make_tuple(EntityKind::TOPIC, 8, std::vector<size_t> { 11, 12 }),
        // DOMAIN - TOPIC: none
        std::make_tuple(EntityKind::TOPIC, 7, std::vector<size_t> { }),
        // DOMAIN - DATAWRITER
        std::make_tuple(EntityKind::DATAWRITER, 8, std::vector<size_t> { 15, 16 }),
        // DOMAIN - DATAWRITER: none
        std::make_tuple(EntityKind::DATAWRITER, 7, std::vector<size_t> { }),
        // DOMAIN - DATAREADER
        std::make_tuple(EntityKind::DATAREADER, 8, std::vector<size_t> { 13, 14 }),
        // DOMAIN - DATAREADER: none
        std::make_tuple(EntityKind::DATAREADER, 7, std::vector<size_t> { }),
        // DOMAIN - LOCATOR
        std::make_tuple(EntityKind::LOCATOR, 8, std::vector<size_t> { 17, 18, 19, 20 }),
        // DOMAIN - LOCATOR: none
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
        // PARTICIPANT - DOMAIN
        std::make_tuple(EntityKind::DOMAIN, 10, std::vector<size_t> { 8 }),
        std::make_tuple(EntityKind::DOMAIN, 9, std::vector<size_t> { 8 }),
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
        // TOPIC - DOMAIN
        std::make_tuple(EntityKind::DOMAIN, 12, std::vector<size_t> { 8 }),
        std::make_tuple(EntityKind::DOMAIN, 11, std::vector<size_t> { 8 }),
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
        // DATAREADER - DOMAIN
        std::make_tuple(EntityKind::DOMAIN, 14, std::vector<size_t> { 8 }),
        std::make_tuple(EntityKind::DOMAIN, 13, std::vector<size_t> { 8 }),
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
        // DATAWRITER - DOMAIN
        std::make_tuple(EntityKind::DOMAIN, 16, std::vector<size_t> { 8 }),
        std::make_tuple(EntityKind::DOMAIN, 15, std::vector<size_t> { 8 }),
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
        // LOCATOR - DOMAIN
        std::make_tuple(EntityKind::DOMAIN, 17, std::vector<size_t> { 8 }),
        std::make_tuple(EntityKind::DOMAIN, 18, std::vector<size_t> { 8 }),
        std::make_tuple(EntityKind::DOMAIN, 19, std::vector<size_t> { 8 }),
        std::make_tuple(EntityKind::DOMAIN, 20, std::vector<size_t> { 8 }),
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
