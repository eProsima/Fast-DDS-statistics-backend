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
#include <string>
#include <vector>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <exception/Exception.hpp>
#include <types/JSONTags.h>
#include <types/types.hpp>

#include <database/database.hpp>
#include <DatabaseUtils.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

void check_erased_database(
        const DataBaseTest& db,
        const EntityId& domain_id,
        const std::vector<std::shared_ptr<const Entity>>& participants,
        const std::vector<std::shared_ptr<const Entity>>& readers,
        const std::vector<std::shared_ptr<const Entity>>& writers)
{
    // Check that the map elements have been correctly erased
    EXPECT_EQ(db.domains().find(domain_id), db.domains().end());
    EXPECT_EQ(db.topics().find(domain_id), db.topics().end());
    EXPECT_EQ(db.participants().find(domain_id), db.participants().end());
    EXPECT_EQ(db.datawriters().find(domain_id), db.datawriters().end());
    EXPECT_EQ(db.datareaders().find(domain_id), db.datareaders().end());
    // Any reference to the erased endpoints has been deleted from the locators map
    for (auto locator : db.locators())
    {
        for (auto reader : readers)
        {
            EXPECT_EQ(locator.second->data_readers.find(reader->id), locator.second->data_readers.end());
        }
        for (auto writer : writers)
        {
            EXPECT_EQ(locator.second->data_writers.find(writer->id), locator.second->data_writers.end());
        }
    }

    // Any reference to the erased domain has been deleted
    auto domains_by_process = db.domains_by_process();
    for (auto process : domains_by_process)
    {
        EXPECT_EQ(process.second.find(domain_id), process.second.end());
    }

    auto processes_by_domain = db.processes_by_domain();
    EXPECT_EQ(processes_by_domain.find(domain_id), processes_by_domain.end());

    // Any reference to the erased participant has been deleted
    for (auto participant : participants)
    {
        EntityId participant_id = participant->id;
        for (auto process : db.processes())
        {
            EXPECT_EQ(process.second->participants.find(participant_id), process.second->participants.end());
        }
        for (auto locator : db.participants_by_locator())
        {
            EXPECT_EQ(locator.second.find(participant_id), locator.second.end());
        }

        auto locators_by_participant = db.locators_by_participant();
        EXPECT_EQ(locators_by_participant.find(participant_id), locators_by_participant.end());
    }
}

void erase_and_check(
        const std::string& initial_filename,
        const std::string& final_filename,
        const std::string& domain)
{
    // Read JSON files
    DatabaseDump initial_dump;
    DatabaseDump final_dump;
    load_file(initial_filename, initial_dump);
    load_file(final_filename, final_dump);

    // Create database
    DataBaseTest db;

    // Load initial dump in the database
    db.load_database(initial_dump);

    // Call erase monitor removing domain_1
    std::vector<std::pair<EntityId, EntityId>> domains = db.get_entities_by_name(EntityKind::DOMAIN_ENTITY, domain);
    EntityId domain_id = domains.begin()->first;
    // Save entities associated to the erased domain to check that the cross maps are correctly erased.
    std::vector<std::shared_ptr<const Entity>> participants = db.get_entities(EntityKind::PARTICIPANT, domain_id);
    std::vector<std::shared_ptr<const Entity>> readers = db.get_entities(EntityKind::DATAREADER, domain_id);
    std::vector<std::shared_ptr<const Entity>> writers = db.get_entities(EntityKind::DATAWRITER, domain_id);
    db.erase(domain_id);

    check_erased_database(db, domain_id, participants, readers, writers);

    // Calling again to erase an already erased domain throws an exception
    ASSERT_THROW(db.erase(domains.begin()->first), BadParameter);

    // Dump erased database
    DatabaseDump erased_dump = db.dump_database();

    // Compare erased and final dumps
    EXPECT_EQ(final_dump, erased_dump);
}

/**
 * This test checks the erase feature.
 * It loads the complex_dump.json and erases the domain_1.
 * The resulting dump should be equal to the one loaded from complex_dump_erased_domain_1.json.
 */
TEST(database_erase_tests, erase_domain_1)
{
    erase_and_check(COMPLEX_DUMP_FILE, COMPLEX_ERASED_DOMAIN_1_DUMP_FILE, "121");
}

/**
 * This test checks the erase feature, when a process has participants from several domains.
 * It loads the alternative_complex_dump.json and erases the domain_2.
 * The resulting dump should be equal to the one loaded from alternative_complex_dump_erased_domain_2.json.
 */
TEST(database_erase_tests, erase_domain_2)
{
    erase_and_check(ALTERNATIVE_COMPLEX_DUMP_FILE, ALTERNATIVE_COMPLEX_ERASED_DOMAIN_2_DUMP_FILE, "122");
}

// This test checks that erasing a database where the participant is not yet linked to the process works as expected
TEST(database_erase_tests, erase_domain_unlinked_participant_process)
{
    erase_and_check(NO_PROCESS_PARTICIPANT_LINK_DUMP_FILE, NO_PROCESS_PARTICIPANT_LINK_ERASED_DOMAIN_DUMP_FILE,
            "120");
}

// This test checks that calling erase with an EntityId that does not correspond with EntityKind::DOMAIN_ENTITY, kills the
// thread with an assertion
TEST(database_erase_tests, erase_wrong_kind)
{
    // Read JSON files
    DatabaseDump initial_dump;
    load_file(COMPLEX_DUMP_FILE, initial_dump);

    // Create database
    Database db;

    // Load initial dump in the database
    db.load_database(initial_dump);

    // Call erase monitor with EntityKind::HOST
    std::vector<std::pair<EntityId, EntityId>> hosts = db.get_entities_by_name(EntityKind::HOST, "host_0");
    EXPECT_THROW(db.erase(hosts.begin()->second), BadParameter);
    // Call erase monitor with EntityKind::USER
    std::vector<std::pair<EntityId, EntityId>> users = db.get_entities_by_name(EntityKind::USER, "user_0");
    EXPECT_THROW(db.erase(users.begin()->second), BadParameter);
    // Call erase monitor with EntityKind::PROCESS
    std::vector<std::pair<EntityId, EntityId>> processes = db.get_entities_by_name(EntityKind::PROCESS, "process_0");
    EXPECT_THROW(db.erase(processes.begin()->second), BadParameter);
    // Call erase monitor with EntityKind::TOPIC
    std::vector<std::pair<EntityId, EntityId>> topics = db.get_entities_by_name(EntityKind::TOPIC, "topic_0");
    EXPECT_THROW(db.erase(topics.begin()->second), BadParameter);
    // Call erase monitor with EntityKind::PARTICIPANT
    std::vector<std::pair<EntityId, EntityId>> participants = db.get_entities_by_name(EntityKind::PARTICIPANT,
                    "participant_0");
    EXPECT_THROW(db.erase(participants.begin()->second), BadParameter);
    // Call erase monitor with EntityKind::DATAWRITER
    std::vector<std::pair<EntityId, EntityId>> writers = db.get_entities_by_name(EntityKind::DATAWRITER,
                    "datawriter_0");
    EXPECT_THROW(db.erase(writers.begin()->second), BadParameter);
    // Call erase monitor with EntityKind::DATAREADER
    std::vector<std::pair<EntityId, EntityId>> readers = db.get_entities_by_name(EntityKind::DATAREADER,
                    "datareader_0");
    EXPECT_THROW(db.erase(readers.begin()->second), BadParameter);
    // Call erase monitor with EntityKind::LOCATOR
    std::vector<std::pair<EntityId, EntityId>> locators = db.get_entities_by_name(EntityKind::LOCATOR, "locator_0");
    EXPECT_THROW(db.erase(locators.begin()->second), BadParameter);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
