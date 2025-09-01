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
#include <gmock/gmock.h>

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/JSONTags.h>
#include <fastdds_statistics_backend/types/types.hpp>

#include <StatisticsBackendData.hpp>  // Mock

#include <database/database.hpp>
#include <database/entities.hpp>
#include <database/samples.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Throw;
using ::testing::AnyNumber;
using ::testing::StrictMock;

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
#define DOMAIN_DEFAULT_NAME(x) "domain_name_" + std::to_string(x)
#define ALIAS_DEFAULT_NAME(x) "domain_alias_" + std::to_string(x)
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
#define PART_GUID_DEFAULT(x) "01.0f.00.00.00.00.00.00.00.00.00.0" + std::to_string(x) + "|0.0.1.0"
#define GUID_DEFAULT(x) "01.0f.00.00.00.00.00.00.00.00.00.0" + std::to_string(x) + "|0.0.0.0"

namespace test {

void initialize_empty_entities(
        Database& db,
        int index,
        bool link_process_participant)
{
    std::shared_ptr<Host> host = std::make_shared<Host>(std::string(HOST_DEFAULT_NAME(index)));
    std::shared_ptr<User> user = std::make_shared<User>(std::string(USER_DEFAULT_NAME(index)), host);
    std::shared_ptr<Process> process = std::make_shared<Process>(std::string(PROCESS_DEFAULT_NAME(
                        index)), PID_DEFAULT, user);
    std::shared_ptr<Domain> domain = std::make_shared<Domain>(std::string(DOMAIN_DEFAULT_NAME(index)), 0);
    domain->alias = ALIAS_DEFAULT_NAME(index);
    std::shared_ptr<Topic> topic = std::make_shared<Topic>(std::string(TOPIC_DEFAULT_NAME(
                        index)), DATA_TYPE_DEFAULT, domain);
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(std::string(
                        PARTICIPANT_DEFAULT_NAME(index)), QOS_DEFAULT, PART_GUID_DEFAULT(index), nullptr, domain);
    std::shared_ptr<DataWriter> dw = std::make_shared<DataWriter>(std::string(
                        DATAWRITER_DEFAULT_NAME(index)), QOS_DEFAULT, GUID_DEFAULT(index), participant, topic);
    std::shared_ptr<DataReader> dr = std::make_shared<DataReader>(std::string(
                        DATAREADER_DEFAULT_NAME(index)), QOS_DEFAULT, GUID_DEFAULT(index), participant, topic);
    std::shared_ptr<Locator> locator = std::make_shared<Locator>(std::string(LOCATOR_DEFAULT_NAME(index)));

    locator->id = db.insert(locator);
    ASSERT_NE(locator->id, EntityId::invalid());
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

} // namespace test

/**
 * @brief Test that clearing inactive entities from database works
 *
 * STEPS:
 * - initialize database
 * - DataWriter
 *   - check that has access to DataWriter
 *   - set DataWriter as inactive
 *   - clear inactive entities
 *   - check that DataWriter no longer exist
 *   - check that other entities exist and are alive
 * - All Entities
 *   - check that has access to every entity (Host & Participant)
 *   - set Participant as inactive
 *   - clear inactive entities
 *   - check that all entities have been cleared (except Domain and Locators)
 */
TEST(database, clear_inactive_entities_database_simple)
{
    // Expect that discovery callbacks are called with any arguments any number ot times
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_physical_entity_discovery(
            _, _, _)).Times(AnyNumber());
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(
            _, _, _, _)).Times(AnyNumber());

    // initialize database
    Database db;
    test::initialize_empty_entities(db, 0, true);
    auto domains = db.get_entities_by_name(EntityKind::DOMAIN, DOMAIN_DEFAULT_NAME(0));
    db.init_domain_view_graph(DOMAIN_DEFAULT_NAME(0), 0, domains[0].second);
    db.regenerate_domain_graph(domains[0].second);

    // DATAWRITER
    {
        // check that has access to DataWriter
        auto datawriters = db.get_entity_ids(EntityKind::DATAWRITER, EntityId::all());
        ASSERT_EQ(datawriters.size(), 1u);
        auto datawriter_id = datawriters[0];
        ASSERT_NE(datawriter_id, EntityId::invalid());
        ASSERT_TRUE(db.get_entity(datawriter_id)->active);

        // set DataWriter as inactive
        db.change_entity_status(datawriter_id, false);
        ASSERT_FALSE(db.get_entity(datawriter_id)->active);

        // clear inactive entities
        db.clear_inactive_entities();

        // check that DataWriter no longer exist
        ASSERT_EQ(db.get_entity_ids(EntityKind::DATAWRITER, EntityId::all()).size(), 0u);
        ASSERT_FALSE(db.is_entity_present(datawriter_id));

        // check that other entities exist and are alive
        std::vector<EntityKind> entities_kind_to_check = {
            EntityKind::PARTICIPANT,
            EntityKind::DATAREADER,
            EntityKind::HOST,
            EntityKind::USER,
            EntityKind::PROCESS,
            EntityKind::DOMAIN,
            EntityKind::TOPIC
        };
        for (const auto& kind : entities_kind_to_check)
        {
            auto ids = db.get_entity_ids(kind, EntityId::all());
            ASSERT_EQ(ids.size(), 1u);
            ASSERT_TRUE(db.get_entity(ids[0])->active);
        }
    }

    // HOST && PARTICIPANT
    {
        // check that has access to every entity
        // Host
        auto hosts = db.get_entity_ids(EntityKind::HOST, EntityId::all());
        ASSERT_EQ(hosts.size(), 1u);
        auto host_id = hosts[0];
        ASSERT_NE(host_id, EntityId::invalid());
        ASSERT_TRUE(db.get_entity(host_id)->active);

        // Participant
        auto participants = db.get_entity_ids(EntityKind::PARTICIPANT, EntityId::all());
        ASSERT_EQ(participants.size(), 1u);
        auto participant_id = participants[0];
        ASSERT_NE(participant_id, EntityId::invalid());
        ASSERT_TRUE(db.get_entity(participant_id)->active);

        // set Participant as inactive
        db.change_entity_status(participant_id, false);
        ASSERT_FALSE(db.get_entity(participant_id)->active);
        ASSERT_FALSE(db.get_entity(host_id)->active);

        // clear inactive entities
        db.clear_inactive_entities();

        // check that all entities have been cleared (except for locators)
        ASSERT_FALSE(db.is_entity_present(participant_id));
        ASSERT_FALSE(db.is_entity_present(host_id));

        std::vector<EntityKind> entities_kind_to_check = {
            EntityKind::PARTICIPANT,
            EntityKind::DATAWRITER,
            EntityKind::DATAREADER,
            EntityKind::HOST,
            EntityKind::USER,
            EntityKind::PROCESS,
            EntityKind::TOPIC
        };
        for (const auto& kind : entities_kind_to_check)
        {
            ASSERT_EQ(db.get_entity_ids(kind, EntityId::all()).size(), 0u)
                << static_cast<int>(kind);
        }

        // 2 entities remain: Domain and 1 locator
        ASSERT_EQ(db.get_entity_ids(EntityKind::DOMAIN, EntityId::all()).size(), 1u);
        ASSERT_EQ(db.get_entity_ids(EntityKind::LOCATOR, EntityId::all()).size(), 1u);
    }
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
