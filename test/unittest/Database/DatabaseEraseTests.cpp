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

constexpr const char* DESCRIPTION_TAG = "description";

constexpr const char* INITIAL_DUMP_FILE = "resources/complex_dump.json";
constexpr const char* FINAL_DUMP_FILE = "resources/complex_dump_erased_domain_1.json";

#include <fstream>
#include <string>
#include <vector>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <exception/Exception.hpp>
#include <types/JSONTags.h>
#include <types/types.hpp>

#include <database/database.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

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

/**
 * This test checks the erase feature.
 * It loads the complex_dump.json and erases the domain_1.
 * The resulting dump should be equal to the one loaded from complex_dump_erased_domain_1.json.
 */ 
TEST(database_erase_tests, erase_domain)
{
    // Read JSON files
    DatabaseDump initial_dump = load_file(INITIAL_DUMP_FILE);
    DatabaseDump final_dump = load_file(FINAL_DUMP_FILE);

    // Create database
    Database db;

    // Load initial dump in the database
    db.load_database(initial_dump);

    // Call erase monitor removing domain_1
    std::vector<std::pair<EntityId, EntityId>> domains = db.get_entities_by_name(EntityKind::DOMAIN, "domain_1");
    db.erase(domains.begin()->first);

    // Calling again to erase an already erased domain throws an exception
    ASSERT_THROW(db.erase(domains.begin()->first), BadParameter);

    // Dump erased database
    DatabaseDump erased_dump = db.dump_database();

    // Compare erased and final dumps
    EXPECT_EQ(final_dump[HOST_CONTAINER_TAG], erased_dump[HOST_CONTAINER_TAG]);
    EXPECT_EQ(final_dump[USER_CONTAINER_TAG], erased_dump[USER_CONTAINER_TAG]);
    EXPECT_EQ(final_dump[PROCESS_CONTAINER_TAG], erased_dump[PROCESS_CONTAINER_TAG]);
    EXPECT_EQ(final_dump[DOMAIN_CONTAINER_TAG], erased_dump[DOMAIN_CONTAINER_TAG]);
    EXPECT_EQ(final_dump[TOPIC_CONTAINER_TAG], erased_dump[TOPIC_CONTAINER_TAG]);
    EXPECT_EQ(final_dump[PARTICIPANT_CONTAINER_TAG], erased_dump[PARTICIPANT_CONTAINER_TAG]);
    EXPECT_EQ(final_dump[LOCATOR_CONTAINER_TAG], erased_dump[LOCATOR_CONTAINER_TAG]);
    EXPECT_EQ(final_dump[DATAWRITER_CONTAINER_TAG], erased_dump[DATAWRITER_CONTAINER_TAG]);
    EXPECT_EQ(final_dump[DATAREADER_CONTAINER_TAG], erased_dump[DATAREADER_CONTAINER_TAG]);

    EXPECT_EQ(final_dump, erased_dump);

}

// This test checks that calling erase with an EntityId that does not correspond with EntityKind::DOMAIN, kills the
// thread with an assertion
TEST(database_erase_tests, erase_wrong_kind)
{
#ifndef NDEBUG
    // Read JSON files
    DatabaseDump initial_dump = load_file(INITIAL_DUMP_FILE);

    // Create database
    Database db;

    // Load initial dump in the database
    db.load_database(initial_dump);

    // Call erase monitor with EntityKind::HOST
    std::vector<std::pair<EntityId, EntityId>> hosts = db.get_entities_by_name(EntityKind::HOST, "host_0");
    ASSERT_DEATH(db.erase(hosts.begin()->second), "");
    // Call erase monitor with EntityKind::USER
    std::vector<std::pair<EntityId, EntityId>> users = db.get_entities_by_name(EntityKind::USER, "user_0");
    ASSERT_DEATH(db.erase(users.begin()->second), "");
    // Call erase monitor with EntityKind::PROCESS
    std::vector<std::pair<EntityId, EntityId>> processes = db.get_entities_by_name(EntityKind::PROCESS, "process_0");
    ASSERT_DEATH(db.erase(processes.begin()->second), "");
    // Call erase monitor with EntityKind::TOPIC
    std::vector<std::pair<EntityId, EntityId>> topics = db.get_entities_by_name(EntityKind::TOPIC, "topic_0");
    ASSERT_DEATH(db.erase(topics.begin()->second), "");
    // Call erase monitor with EntityKind::PARTICIPANT
    std::vector<std::pair<EntityId, EntityId>> participants = db.get_entities_by_name(EntityKind::PARTICIPANT,
        "participant_0");
    ASSERT_DEATH(db.erase(participants.begin()->second), "");
    // Call erase monitor with EntityKind::DATAWRITER
    std::vector<std::pair<EntityId, EntityId>> writers = db.get_entities_by_name(EntityKind::DATAWRITER,
        "datawriter_0");
    ASSERT_DEATH(db.erase(writers.begin()->second), "");
    // Call erase monitor with EntityKind::DATAREADER
    std::vector<std::pair<EntityId, EntityId>> readers = db.get_entities_by_name(EntityKind::DATAREADER,
        "datareader_0");
    ASSERT_DEATH(db.erase(readers.begin()->second), "");
    // Call erase monitor with EntityKind::LOCATOR
    std::vector<std::pair<EntityId, EntityId>> locators = db.get_entities_by_name(EntityKind::LOCATOR, "locator_0");
    ASSERT_DEATH(db.erase(locators.begin()->second), "");
#endif // NDEBUG
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
