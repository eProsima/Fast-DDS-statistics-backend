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

/*
enum dump_type{_NULL,_OBJECT,_ARRAY,_STRING,_INT};

std::map<dump_type,DatabaseDump> test_values =
{
    std::pair<dump_type,DatabaseDump>(_NULL,DatabaseDump()),
    std::pair<dump_type,DatabaseDump>(_OBJECT,DatabaseDump::object()),
    std::pair<dump_type,DatabaseDump>(_ARRAY,DatabaseDump::array()),
    std::pair<dump_type,DatabaseDump>(_STRING,"wrong"),
    std::pair<dump_type,DatabaseDump>(_INT,"1")
};

// Check the load of a container with all types of values
void check_values(DatabaseDump & dump, DatabaseDump & container, std::map<dump_type, bool>& is_wrong)
{
    for (auto test_value : test_values)
    {
        Database db;
        DatabaseDump aux = container;
        container = test_value.second;

        if (is_wrong[test_value.first])
        {
            ASSERT_THROW(db.load_database(dump), CorruptedFile);
        }
        else
        {
            ASSERT_NO_THROW(db.load_database(dump));
        }

        container = aux;
    }
}

void check_wrong(DatabaseDump & dump, DatabaseDump & container)
{
    std::map<dump_type, bool> is_wrong =
        {
            std::pair<dump_type, DatabaseDump>(_NULL, true),
            std::pair<dump_type, DatabaseDump>(_OBJECT, true),
            std::pair<dump_type, DatabaseDump>(_ARRAY, true),
            std::pair<dump_type, DatabaseDump>(_STRING, true),
            std::pair<dump_type, DatabaseDump>(_INT, true)};

    check_values(dump, container, is_wrong);
}
*/

// Check that the load of the 'dump 'without 'key' in 'container', throws an exception
void check_no_key(DatabaseDump & dump, DatabaseDump & container, std::string const& key)
{
    Database db;
    container.erase(key);
    ASSERT_THROW(db.load_database(dump), CorruptedFile);
}

void check_entity_generic_keys(DatabaseDump & dump, std::string const& key)
{
    // No entities key
    {
        DatabaseDump dumpCopy = dump;
        check_no_key(dumpCopy,dumpCopy,key);
    }

    // No entity key
    {
        DatabaseDump dumpCopy = dump;
        check_no_key(dumpCopy,dumpCopy[key],dumpCopy[key].begin().key());
    }

    // No entity name key
    {
        DatabaseDump dumpCopy = dump;
        check_no_key(dumpCopy,dumpCopy[key][dumpCopy[key].begin().key()],NAME_INFO_TAG);
    }
}

void check_entity_no_key(DatabaseDump & dump, std::string const& entityTag, std::string const& keyTag)
{
    DatabaseDump dumpCopy = dump;
    check_no_key(dumpCopy, dumpCopy[entityTag][dumpCopy[entityTag].begin().key()],keyTag);
}



void check_data_no_key(DatabaseDump & dump, std::string const& entityTag, std::string const& keyTag)
{
    DatabaseDump dumpCopy = dump;
    check_no_key(dumpCopy, dumpCopy[entityTag][dumpCopy[entityTag].begin().key()][DATA_VALUE_DATA_TAG],keyTag);
}

// Test the load of a corrupted database
TEST(database_load_and_dump_tests, load_and_dump_erased_keys)
{
    // Read JSON
    DatabaseDump emptyEntitiesDump = load_file(COMPLEX_DUMP_FILE);

    // ------------ HOSTS ----------------

    check_entity_generic_keys(emptyEntitiesDump, HOST_CONTAINER_TAG);

    check_entity_no_key(emptyEntitiesDump,HOST_CONTAINER_TAG,USER_CONTAINER_TAG);
    
    // ------------ USERS ----------------

    check_entity_generic_keys(emptyEntitiesDump, USER_CONTAINER_TAG);

    check_entity_no_key(emptyEntitiesDump,USER_CONTAINER_TAG,HOST_ENTITY_TAG);
    check_entity_no_key(emptyEntitiesDump,USER_CONTAINER_TAG,PROCESS_CONTAINER_TAG);

    // ------------ PROCESSES ----------------

    check_entity_generic_keys(emptyEntitiesDump, PROCESS_CONTAINER_TAG);

    check_entity_no_key(emptyEntitiesDump,PROCESS_CONTAINER_TAG,PID_INFO_TAG);
    check_entity_no_key(emptyEntitiesDump,PROCESS_CONTAINER_TAG,USER_ENTITY_TAG);
    check_entity_no_key(emptyEntitiesDump,PROCESS_CONTAINER_TAG,PARTICIPANT_CONTAINER_TAG);

    // ------------ DOMAINS ----------------

    check_entity_generic_keys(emptyEntitiesDump, DOMAIN_CONTAINER_TAG);

    check_entity_no_key(emptyEntitiesDump,DOMAIN_CONTAINER_TAG,PARTICIPANT_CONTAINER_TAG);
    check_entity_no_key(emptyEntitiesDump,DOMAIN_CONTAINER_TAG,TOPIC_CONTAINER_TAG);

    // ------------ TOPICS ----------------

    check_entity_generic_keys(emptyEntitiesDump, TOPIC_CONTAINER_TAG);

    check_entity_no_key(emptyEntitiesDump,TOPIC_CONTAINER_TAG,DATA_TYPE_INFO_TAG);
    check_entity_no_key(emptyEntitiesDump,TOPIC_CONTAINER_TAG,DOMAIN_ENTITY_TAG);
    check_entity_no_key(emptyEntitiesDump,TOPIC_CONTAINER_TAG,DATAWRITER_CONTAINER_TAG);
    check_entity_no_key(emptyEntitiesDump,TOPIC_CONTAINER_TAG,DATAREADER_CONTAINER_TAG);

    // ------------ PARTICIPANTS ----------------

    check_entity_generic_keys(emptyEntitiesDump, PARTICIPANT_CONTAINER_TAG);

    check_entity_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,GUID_INFO_TAG);
    check_entity_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,QOS_INFO_TAG);
    check_entity_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,PROCESS_ENTITY_TAG);
    check_entity_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,DOMAIN_ENTITY_TAG);
    check_entity_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,DATAWRITER_CONTAINER_TAG);
    check_entity_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,DATAREADER_CONTAINER_TAG);
    check_entity_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,DATA_VALUE_DATA_TAG);

    // ------------ LOCATORS ----------------

    check_entity_generic_keys(emptyEntitiesDump, LOCATOR_CONTAINER_TAG);

    check_entity_no_key(emptyEntitiesDump,LOCATOR_CONTAINER_TAG,DATAWRITER_CONTAINER_TAG);
    check_entity_no_key(emptyEntitiesDump,LOCATOR_CONTAINER_TAG,DATAREADER_CONTAINER_TAG);
    check_entity_no_key(emptyEntitiesDump,LOCATOR_CONTAINER_TAG,DATA_VALUE_DATA_TAG);

    // ------------ DATAWRITERS ----------------

    check_entity_generic_keys(emptyEntitiesDump, DATAWRITER_CONTAINER_TAG);

    check_entity_no_key(emptyEntitiesDump,DATAWRITER_CONTAINER_TAG,GUID_INFO_TAG);
    check_entity_no_key(emptyEntitiesDump,DATAWRITER_CONTAINER_TAG,QOS_INFO_TAG);
    check_entity_no_key(emptyEntitiesDump,DATAWRITER_CONTAINER_TAG,PARTICIPANT_ENTITY_TAG);
    check_entity_no_key(emptyEntitiesDump,DATAWRITER_CONTAINER_TAG,TOPIC_ENTITY_TAG);
    check_entity_no_key(emptyEntitiesDump,DATAWRITER_CONTAINER_TAG,LOCATOR_CONTAINER_TAG);
    check_entity_no_key(emptyEntitiesDump,DATAWRITER_CONTAINER_TAG,DATA_VALUE_DATA_TAG);

    // ------------ DATAREADERS ----------------

    check_entity_generic_keys(emptyEntitiesDump, DATAREADER_CONTAINER_TAG);

    check_entity_no_key(emptyEntitiesDump,DATAREADER_CONTAINER_TAG,GUID_INFO_TAG);
    check_entity_no_key(emptyEntitiesDump,DATAREADER_CONTAINER_TAG,QOS_INFO_TAG);
    check_entity_no_key(emptyEntitiesDump,DATAREADER_CONTAINER_TAG,PARTICIPANT_ENTITY_TAG);
    check_entity_no_key(emptyEntitiesDump,DATAREADER_CONTAINER_TAG,TOPIC_ENTITY_TAG);
    check_entity_no_key(emptyEntitiesDump,DATAREADER_CONTAINER_TAG,LOCATOR_CONTAINER_TAG);
    check_entity_no_key(emptyEntitiesDump,DATAREADER_CONTAINER_TAG,DATA_VALUE_DATA_TAG);

    // ------------ PARTICIPANT DATA ----------------

    check_data_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_DISCOVERY_TIME_TAG);
    check_data_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_PDP_PACKETS_TAG);
    check_data_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_EDP_PACKETS_TAG);
    check_data_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_RTPS_PACKETS_SENT_TAG);
    check_data_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_RTPS_BYTES_SENT_TAG);
    check_data_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_RTPS_PACKETS_LOST_TAG);
    check_data_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_RTPS_BYTES_LOST_TAG);
    check_data_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_RTPS_BYTES_LOST_LAST_REPORTED_TAG);
    check_data_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_RTPS_BYTES_SENT_LAST_REPORTED_TAG);
    check_data_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_RTPS_PACKETS_LOST_LAST_REPORTED_TAG);
    check_data_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_RTPS_PACKETS_SENT_LAST_REPORTED_TAG);
    check_data_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_EDP_PACKETS_LAST_REPORTED_TAG);
    check_data_no_key(emptyEntitiesDump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_PDP_PACKETS_LAST_REPORTED_TAG);

    // ------------ DATAWRITER DATA ----------------

    check_data_no_key(emptyEntitiesDump,DATAREADER_CONTAINER_TAG,DATA_KIND_SUBSCRIPTION_THROUGHPUT_TAG);
    check_data_no_key(emptyEntitiesDump,DATAREADER_CONTAINER_TAG,DATA_KIND_ACKNACK_COUNT_TAG);
    check_data_no_key(emptyEntitiesDump,DATAREADER_CONTAINER_TAG,DATA_KIND_NACKFRAG_COUNT_TAG);
    check_data_no_key(emptyEntitiesDump,DATAREADER_CONTAINER_TAG,DATA_KIND_ACKNACK_COUNT_LAST_REPORTED_TAG);
    check_data_no_key(emptyEntitiesDump,DATAREADER_CONTAINER_TAG,DATA_KIND_NACKFRAG_COUNT_LAST_REPORTED_TAG);
    
    // ------------ LOCATOR DATA ----------------

    check_data_no_key(emptyEntitiesDump,LOCATOR_CONTAINER_TAG,DATA_KIND_NETWORK_LATENCY_TAG);
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