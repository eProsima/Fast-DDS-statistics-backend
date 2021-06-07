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

#include <gtest/gtest.h>

#include <StatisticsBackend.hpp>
#include <types/types.hpp>

#include <database/database.hpp>
#include <DatabaseUtils.hpp>
#include <fastdds-statistics-backend/types/JSONTags.h>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

/**
 * @brief Fixture for the statistics_backend class tests
 *
 * \c get_type Returns the entity kind of a given id.
 *
 */
class statistics_backend_tests : public ::testing::Test
{
public:

    typedef uint32_t TestId;

    void SetUp()
    {
        entities = PopulateDatabase::populate_database(db);
    }

    DataBaseTest db;
    std::map<TestId, std::shared_ptr<const Entity>> entities;
};

class StatisticsBackendTest : public StatisticsBackend
{
public:

    static void set_database(
            Database* db)
    {
        database_ = db;
    }

    static const char* get_entity_kind_str(
            EntityKind kind)
    {
        return entity_kind_str[(int)kind];
    }

};

void check_dds_entity(
        std::shared_ptr<const DDSEntity> const& entity)
{
    Info info = StatisticsBackendTest::get_info(entity->id);
    ASSERT_EQ(entity->guid, info[GUID_INFO_TAG]);
    ASSERT_EQ(entity->qos, info[QOS_INFO_TAG]);
}

// Check the get_type StatisticsBackend method
TEST_F(statistics_backend_tests, get_info)
{
    StatisticsBackendTest::set_database(&db);

    // Check generic info
    for (auto pair : entities)
    {
        std::shared_ptr<const Entity> entity = pair.second;
        Info info = StatisticsBackendTest::get_info(entity->id);

        ASSERT_EQ(entity->id, EntityId(info[ID_INFO_TAG]));
        ASSERT_EQ(StatisticsBackendTest::get_entity_kind_str(entity->kind), info[KIND_INFO_TAG]);
        ASSERT_EQ(entity->name, info[NAME_INFO_TAG]);
    }

    // Process
    for (auto pair : db.processes())
    {
        std::shared_ptr<const Process> entity = pair.second;
        Info info = StatisticsBackendTest::get_info(entity->id);
        ASSERT_EQ(entity->pid, info[PID_INFO_TAG]);
    }

    // Topic
    for (auto domainPair : db.topics())
    {
        for (auto pair : domainPair.second)
        {
            std::shared_ptr<const Topic> entity = pair.second;
            Info info = StatisticsBackendTest::get_info(entity->id);
            ASSERT_EQ(entity->data_type, info[DATA_TYPE_INFO_TAG]);
        }
    }

    // Participant
    for (auto domainPair : db.participants())
    {
        for (auto pair : domainPair.second)
        {
            check_dds_entity(pair.second);
        }
    }

    // Datawriter
    for (auto domainPair : db.get_dds_endpoints<DataWriter>())
    {
        for (auto pair : domainPair.second)
        {
            check_dds_entity(pair.second);
        }
    }

    // Datareader
    for (auto domainPair : db.get_dds_endpoints<DataReader>())
    {
        for (auto pair : domainPair.second)
        {
            check_dds_entity(pair.second);
        }
    }
}

// Check the get_type StatisticsBackend method
TEST_F(statistics_backend_tests, get_type)
{
    StatisticsBackendTest::set_database(&db);

    for (auto pair : db.hosts())
    {
        auto entity = pair.second;
        ASSERT_EQ(StatisticsBackendTest::get_type(entity->id), entity->kind);
    }

    for (auto pair : db.users())
    {
        auto entity = pair.second;
        ASSERT_EQ(StatisticsBackendTest::get_type(entity->id), entity->kind);
    }

    for (auto pair : db.processes())
    {
        auto entity = pair.second;
        ASSERT_EQ(StatisticsBackendTest::get_type(entity->id), entity->kind);
    }

    for (auto pair : db.domains())
    {
        auto entity = pair.second;
        ASSERT_EQ(StatisticsBackendTest::get_type(entity->id), entity->kind);
    }

    for (auto domainPair : db.participants())
    {
        auto domainEntities = domainPair.second;
        for (auto pair : domainEntities)
        {
            auto entity = pair.second;
            ASSERT_EQ(StatisticsBackendTest::get_type(entity->id), entity->kind);
        }
    }

    for (auto domainPair : db.topics())
    {
        auto domainEntities = domainPair.second;
        for (auto pair : domainEntities)
        {
            auto entity = pair.second;
            ASSERT_EQ(StatisticsBackendTest::get_type(entity->id), entity->kind);
        }
    }

    for (auto domainPair : db.get_dds_endpoints<DataReader>())
    {
        auto domainEntities = domainPair.second;
        for (auto pair : domainEntities)
        {
            auto entity = pair.second;
            ASSERT_EQ(StatisticsBackendTest::get_type(entity->id), entity->kind);
        }
    }

    for (auto domainPair : db.get_dds_endpoints<DataWriter>())
    {
        auto domainEntities = domainPair.second;
        for (auto pair : domainEntities)
        {
            auto entity = pair.second;
            ASSERT_EQ(StatisticsBackendTest::get_type(entity->id), entity->kind);
        }
    }

    for (auto pair : db.locators())
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
