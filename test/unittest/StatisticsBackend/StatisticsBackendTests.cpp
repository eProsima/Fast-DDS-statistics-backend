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
#include <list>
#include <string>
#include <vector>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <StatisticsBackend.hpp>
#include <StatisticsBackendData.hpp>
#include <types/JSONTags.h>
#include <types/types.hpp>
#include <database/database.hpp>
#include <DatabaseUtils.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

/**
 * @brief Fixture for the statistics_backend class tests
 *
 * \c get_type Returns the entity kind of a given id.
 *
 * Parameteres to the tests are:
 *  - std::get<0>(GetParam()) The EntityKind we are looking for
 *  - std::get<1>(GetParam()) The unique identifier of the origin Entity, as given by the fixture/testing
 *  - std::get<2>(GetParam()) A list containing the unique identifiers of the entities expected in the result
 */
class statistics_backend_tests : public ::testing::TestWithParam<std::tuple<EntityKind, size_t, std::vector<size_t>>>
{
public:

    using TestId = PopulateDatabase::TestId;

    void SetUp()
    {
        db = new DataBaseTest;
        entities = PopulateDatabase::populate_database(*db);
    }

    DataBaseTest* db;
    std::map<TestId, std::shared_ptr<const Entity>> entities;
};

class StatisticsBackendTest : public StatisticsBackend
{
public:

    static void set_database(
            Database* db)
    {
        details::StatisticsBackendData::get_instance()->database_.reset(db);
    }

};

// Check the is_active StatisticsBackend method
TEST_F(statistics_backend_tests, is_active)
{
    StatisticsBackend::is_active(0);
    ASSERT_TRUE(true);
}

void check_dds_entity(
        std::shared_ptr<const DDSEntity> const& entity,
        Info const& info)
{
    ASSERT_EQ(entity->guid, info[GUID_INFO_TAG]);
    ASSERT_EQ(entity->qos, info[QOS_INFO_TAG]);
}

// Check the get_info StatisticsBackend method
TEST_F(statistics_backend_tests, get_info)
{
    StatisticsBackendTest::set_database(db);

    ASSERT_ANY_THROW(StatisticsBackendTest::get_info(entities[0]->id));

    // Erase invalid entity
    entities.erase(0);

    for (auto pair : entities)
    {
        std::shared_ptr<const Entity> entity = pair.second;
        Info info = StatisticsBackendTest::get_info(entity->id);

        // Check generic info
        // Once the info is checked, it is erased so the final check is confirm that the info is empty (there is no
        // more information than the expected)
        EXPECT_EQ(entity->id, EntityId(info[ID_INFO_TAG]));
        info.erase(ID_INFO_TAG);
        EXPECT_EQ(entity_kind_str[(int)entity->kind], info[KIND_INFO_TAG]);
        info.erase(KIND_INFO_TAG);
        EXPECT_EQ(entity->name, info[NAME_INFO_TAG]);
        info.erase(NAME_INFO_TAG);
        EXPECT_EQ(entity->alias, info[ALIAS_INFO_TAG]);
        info.erase(ALIAS_INFO_TAG);

        // Check specific info
        switch (entity->kind)
        {
            case EntityKind::PROCESS:
            {
                std::shared_ptr<const Process> process =
                        std::dynamic_pointer_cast<const Process>(entity);
                EXPECT_EQ(process->pid, info[PID_INFO_TAG]);
                info.erase(PID_INFO_TAG);
                break;
            }
            case EntityKind::TOPIC:
            {
                std::shared_ptr<const Topic> topic =
                        std::dynamic_pointer_cast<const Topic>(entity);
                EXPECT_EQ(topic->data_type, info[DATA_TYPE_INFO_TAG]);
                info.erase(DATA_TYPE_INFO_TAG);
                break;
            }
            case EntityKind::PARTICIPANT:
            {
                std::shared_ptr<const DomainParticipant> participant =
                        std::dynamic_pointer_cast<const DomainParticipant>(entity);
                EXPECT_EQ(participant->guid, info[GUID_INFO_TAG]);
                info.erase(GUID_INFO_TAG);
                EXPECT_EQ(participant->qos, info[QOS_INFO_TAG]);
                info.erase(QOS_INFO_TAG);

                // Obtain the locators list associated to the participant's endpoints
                std::vector<std::string> locators;
                for (auto reader : participant->data_readers)
                {
                    for (auto locator : reader.second.get()->locators)
                    {
                        locators.push_back(locator.second.get()->name);
                    }
                }
                for (auto writer : participant->data_writers)
                {
                    for (auto locator : writer.second.get()->locators)
                    {
                        locators.push_back(locator.second.get()->name);
                    }
                }
                // Remove duplicates
                auto last = std::unique(locators.begin(), locators.end(), [](
                                    const std::string& first,
                                    const std::string& second)
                                {
                                    return first.compare(second) == 0;
                                });
                locators.erase(last, locators.end());

                // Check that every locator is included in the Info object
                for (auto locator_name : locators)
                {
                    auto locator_it = std::find(info[LOCATOR_CONTAINER_TAG].begin(),
                                    info[LOCATOR_CONTAINER_TAG].end(), locator_name);
                    ASSERT_NE(locator_it, info[LOCATOR_CONTAINER_TAG].end());
                    info[LOCATOR_CONTAINER_TAG].erase(locator_it);
                }
                EXPECT_TRUE(info[LOCATOR_CONTAINER_TAG].empty());
                info.erase(LOCATOR_CONTAINER_TAG);
                break;
            }
            case EntityKind::DATAWRITER:
            case EntityKind::DATAREADER:
            {
                std::shared_ptr<const DDSEntity> dds_entity =
                        std::dynamic_pointer_cast<const DDSEntity>(entity);
                EXPECT_EQ(dds_entity->guid, info[GUID_INFO_TAG]);
                info.erase(GUID_INFO_TAG);
                EXPECT_EQ(dds_entity->qos, info[QOS_INFO_TAG]);
                info.erase(QOS_INFO_TAG);
                break;
            }
            default:
            {
                break;
            }
        }
        EXPECT_TRUE(info.empty());
    }
}

// Check the get_entities StatisticsBackend method
TEST_P(statistics_backend_tests, get_entities)
{
    EntityKind kind = std::get<0>(GetParam());
    EntityId origin = entities[std::get<1>(GetParam())]->id;
    std::vector<EntityId> expected;
    for (auto it : std::get<2>(GetParam()))
    {
        expected.push_back(entities[it]->id);
    }

    StatisticsBackendTest::set_database(db);

    EXPECT_THROW(StatisticsBackendTest::get_entities(kind, db->generate_entity_id()), BadParameter);
    EXPECT_THROW(StatisticsBackendTest::get_entities(EntityKind::INVALID, origin), BadParameter);

    auto result = StatisticsBackendTest::get_entities(kind, origin);
    ASSERT_EQ(expected.size(), result.size());
    std::sort(expected.begin(), expected.end());
    std::sort(result.begin(), result.end());
    for (size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(expected[i], result[i]);
    }
}

// Check the get_type StatisticsBackend method
TEST_F(statistics_backend_tests, get_type)
{
    StatisticsBackendTest::set_database(db);

    for (auto pair : db->hosts())
    {
        auto entity = pair.second;
        ASSERT_EQ(StatisticsBackendTest::get_type(entity->id), entity->kind);
    }

    for (auto pair : db->users())
    {
        auto entity = pair.second;
        ASSERT_EQ(StatisticsBackendTest::get_type(entity->id), entity->kind);
    }

    for (auto pair : db->processes())
    {
        auto entity = pair.second;
        ASSERT_EQ(StatisticsBackendTest::get_type(entity->id), entity->kind);
    }

    for (auto pair : db->domains())
    {
        auto entity = pair.second;
        ASSERT_EQ(StatisticsBackendTest::get_type(entity->id), entity->kind);
    }

    for (auto domainPair : db->participants())
    {
        auto domainEntities = domainPair.second;
        for (auto pair : domainEntities)
        {
            auto entity = pair.second;
            ASSERT_EQ(StatisticsBackendTest::get_type(entity->id), entity->kind);
        }
    }

    for (auto domainPair : db->topics())
    {
        auto domainEntities = domainPair.second;
        for (auto pair : domainEntities)
        {
            auto entity = pair.second;
            ASSERT_EQ(StatisticsBackendTest::get_type(entity->id), entity->kind);
        }
    }

    for (auto domainPair : db->get_dds_endpoints<DataReader>())
    {
        auto domainEntities = domainPair.second;
        for (auto pair : domainEntities)
        {
            auto entity = pair.second;
            ASSERT_EQ(StatisticsBackendTest::get_type(entity->id), entity->kind);
        }
    }

    for (auto domainPair : db->get_dds_endpoints<DataWriter>())
    {
        auto domainEntities = domainPair.second;
        for (auto pair : domainEntities)
        {
            auto entity = pair.second;
            ASSERT_EQ(StatisticsBackendTest::get_type(entity->id), entity->kind);
        }
    }

    for (auto pair : db->locators())
    {
        auto entity = pair.second;
        ASSERT_EQ(StatisticsBackendTest::get_type(entity->id), entity->kind);
    }

    ASSERT_THROW(StatisticsBackendTest::get_type(EntityId::all()), BadParameter);
    ASSERT_THROW(StatisticsBackendTest::get_type(EntityId::invalid()), BadParameter);
    ASSERT_THROW(StatisticsBackendTest::get_type(EntityId(1234)), BadParameter);
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
             EntityKind::PARTICIPANT, EntityKind::LOCATOR)},

        {DataKind::RTPS_BYTES_SENT, std::pair<EntityKind, EntityKind>(
             EntityKind::PARTICIPANT, EntityKind::LOCATOR)},

        {DataKind::RTPS_PACKETS_LOST, std::pair<EntityKind, EntityKind>(
             EntityKind::PARTICIPANT, EntityKind::LOCATOR)},

        {DataKind::RTPS_BYTES_LOST, std::pair<EntityKind, EntityKind>(
             EntityKind::PARTICIPANT, EntityKind::LOCATOR)},

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

// Check the get_data_supported_entity_kinds StatisticsBackend method
TEST_F(statistics_backend_tests, set_alias)
{
    StatisticsBackendTest::set_database(db);

    for (auto entity : entities)
    {
        if (entity.second->kind == EntityKind::INVALID)
        {
            EXPECT_THROW(StatisticsBackend::set_alias(entity.second->id, "my_alias"), BadParameter);
        }
        else
        {
            ASSERT_NE("my_alias", entity.second->alias);
            StatisticsBackend::set_alias(entity.second->id, "my_alias");
            ASSERT_EQ("my_alias", entity.second->alias);
        }
    }
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_SUITE_P(x, y, z)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_CASE_P(x, y, z)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(
    statistics_backend_tests,
    statistics_backend_tests,
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
