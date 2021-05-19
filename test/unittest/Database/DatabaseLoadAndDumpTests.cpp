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

#include "gtest/gtest.h"

#include <fastdds-statistics-backend/exception/Exception.hpp>
#include <fastdds-statistics-backend/types/JSONTags.h>
#include <fastdds-statistics-backend/types/types.hpp>

#include <database/database.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

constexpr const char* DESCRIPTION_TAG = "description";

constexpr const char* EMPTY_DUMP_FILE = "resources/empty_dump.json";
constexpr const char* EMPTY_ENTITIES_DUMP_FILE = "resources/empty_entities_dump.json";
constexpr const char* SIMPLE_DUMP_FILE = "resources/simple_dump.json";
constexpr const char* COMPLEX_DUMP_FILE = "resources/complex_dump.json";


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

// Test the load of a dump database without any entity
TEST(database_load_and_dump_tests, load_and_dump_empty_database)
{
    // Read JSON
    DatabaseDump dump = load_file(EMPTY_DUMP_FILE);

    // Create database
    Database db;

    // Load jump in database
    db.load_database(dump);

    // Dump loaded database
    DatabaseDump loadedDump = db.dump_database();

    // Compare two dumps
    ASSERT_EQ(dump, loadedDump);
}

// Test the load of a dump database with one entity of each kind
TEST(database_load_and_dump_tests, load_and_dump_empty_entities_database)
{
    // Read JSON
    DatabaseDump dump = load_file(EMPTY_ENTITIES_DUMP_FILE);

    // Create database
    Database db;

    // Load jump in database
    db.load_database(dump);

    // Dump loaded database
    DatabaseDump loadedDump = db.dump_database();

    // Compare two dumps
    ASSERT_EQ(dump[HOST_CONTAINER_TAG], loadedDump[HOST_CONTAINER_TAG]);
    ASSERT_EQ(dump[USER_CONTAINER_TAG], loadedDump[USER_CONTAINER_TAG]);
    ASSERT_EQ(dump[PROCESS_CONTAINER_TAG], loadedDump[PROCESS_CONTAINER_TAG]);
    ASSERT_EQ(dump[DOMAIN_CONTAINER_TAG], loadedDump[DOMAIN_CONTAINER_TAG]);
    ASSERT_EQ(dump[TOPIC_CONTAINER_TAG], loadedDump[TOPIC_CONTAINER_TAG]);
    ASSERT_EQ(dump[PARTICIPANT_CONTAINER_TAG], loadedDump[PARTICIPANT_CONTAINER_TAG]);
    ASSERT_EQ(dump[LOCATOR_CONTAINER_TAG], loadedDump[LOCATOR_CONTAINER_TAG]);
    ASSERT_EQ(dump[DATAWRITER_CONTAINER_TAG], loadedDump[DATAWRITER_CONTAINER_TAG]);
    ASSERT_EQ(dump[DATAREADER_CONTAINER_TAG], loadedDump[DATAREADER_CONTAINER_TAG]);

    ASSERT_EQ(dump, loadedDump);
}

// Test the load of a dump database with one entity of each kind and one data of each kind
TEST(database_load_and_dump_tests, load_and_dump_simple_database)
{
    // Read JSON
    DatabaseDump dump = load_file(SIMPLE_DUMP_FILE);

    // Create database
    Database db;

    // Load jump in database
    db.load_database(dump);

    // Dump loaded database
    DatabaseDump loadedDump = db.dump_database();

    // Compare two dumps
    ASSERT_EQ(dump[HOST_CONTAINER_TAG], loadedDump[HOST_CONTAINER_TAG]);
    ASSERT_EQ(dump[USER_CONTAINER_TAG], loadedDump[USER_CONTAINER_TAG]);
    ASSERT_EQ(dump[PROCESS_CONTAINER_TAG], loadedDump[PROCESS_CONTAINER_TAG]);
    ASSERT_EQ(dump[DOMAIN_CONTAINER_TAG], loadedDump[DOMAIN_CONTAINER_TAG]);
    ASSERT_EQ(dump[TOPIC_CONTAINER_TAG], loadedDump[TOPIC_CONTAINER_TAG]);
    ASSERT_EQ(dump[PARTICIPANT_CONTAINER_TAG], loadedDump[PARTICIPANT_CONTAINER_TAG]);
    ASSERT_EQ(dump[LOCATOR_CONTAINER_TAG], loadedDump[LOCATOR_CONTAINER_TAG]);
    ASSERT_EQ(dump[DATAWRITER_CONTAINER_TAG], loadedDump[DATAWRITER_CONTAINER_TAG]);
    ASSERT_EQ(dump[DATAREADER_CONTAINER_TAG], loadedDump[DATAREADER_CONTAINER_TAG]);

    ASSERT_EQ(dump, loadedDump);
}

// Test the load of a dump database with three entities of each kind and three datas of each kind
TEST(database_load_and_dump_tests, load_and_dump_complex_database)
{
    // Read JSON
    DatabaseDump dump = load_file(COMPLEX_DUMP_FILE);

    // Create database
    Database db;

    // Load jump in database
    db.load_database(dump);

    // Dump loaded database
    DatabaseDump loadedDump = db.dump_database();

    // Compare two dumps
    ASSERT_EQ(dump[HOST_CONTAINER_TAG], loadedDump[HOST_CONTAINER_TAG]);
    ASSERT_EQ(dump[USER_CONTAINER_TAG], loadedDump[USER_CONTAINER_TAG]);
    ASSERT_EQ(dump[PROCESS_CONTAINER_TAG], loadedDump[PROCESS_CONTAINER_TAG]);
    ASSERT_EQ(dump[DOMAIN_CONTAINER_TAG], loadedDump[DOMAIN_CONTAINER_TAG]);
    ASSERT_EQ(dump[TOPIC_CONTAINER_TAG], loadedDump[TOPIC_CONTAINER_TAG]);
    ASSERT_EQ(dump[PARTICIPANT_CONTAINER_TAG], loadedDump[PARTICIPANT_CONTAINER_TAG]);
    ASSERT_EQ(dump[LOCATOR_CONTAINER_TAG], loadedDump[LOCATOR_CONTAINER_TAG]);
    ASSERT_EQ(dump[DATAWRITER_CONTAINER_TAG], loadedDump[DATAWRITER_CONTAINER_TAG]);
    ASSERT_EQ(dump[DATAREADER_CONTAINER_TAG], loadedDump[DATAREADER_CONTAINER_TAG]);

    ASSERT_EQ(dump, loadedDump);
}


int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}