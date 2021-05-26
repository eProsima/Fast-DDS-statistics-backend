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



// void change_key(DatabaseDump &object, const std::string& old_key, const std::string& new_key)
// {
//     // get iterator to old key; TODO: error handling if key is not present
//     DatabaseDump::iterator it = object.find(old_key);

//     // create null value for new key and swap value from old key
//     std::swap(object[new_key], it.value());

//     // delete value at old key (cheap, because the value is null after swap)
//     object.erase(it);
// }

// void check_change_key(
//         DatabaseDump& dump,
//         DatabaseDump& container,
//         std::string const& key,
//         std::string const& newKey,
//         bool const& throwException)
// {
//     Database db;
//     change_key(container, key, newKey);

//     if (throwException)
//     {
//         ASSERT_THROW(db.load_database(dump), CorruptedFile);
//     }
//     else
//     {
//         ASSERT_NO_THROW(db.load_database(dump));
//     }
// }


// void check_entity_change_key(
//     DatabaseDump &dump,
//     std::string const &entityTag)
// {
//     std::string key = dump[entityTag].begin().key();
//     std::vector<std::string> badkeys = {"1234", "34.12", "true", "false", "qwerty"};

//     for (auto badkey : badkeys)
//     {
//         DatabaseDump dumpCopy = dump;
//         check_change_key(dumpCopy, dumpCopy[entityTag], key,badkey,true);
//     }
// }

// void check_data_change_key(
//     DatabaseDump &dump,
//     std::string const &entityTag,
//     std::string const &dataTag)
// {

//     std::string key = dump[entityTag][dump[entityTag].begin().key()][DATA_VALUE_DATA_TAG][dataTag].begin().key();
//     std::vector<std::string> badkeys = {"true", "false", "qwerty"};
//     std::vector<std::string> goodkeys = {"1234","34.12"};

//     for (auto badkey : badkeys)
//     {
//         DatabaseDump dumpCopy = dump;
//         check_change_key(dumpCopy, dumpCopy[entityTag][dump[entityTag].begin().key()][DATA_VALUE_DATA_TAG][dataTag],
//                          key, badkey,true);
//     }

//     for (auto goodkey : goodkeys)
//     {
//         DatabaseDump dumpCopy = dump;
//         check_change_key(dumpCopy, dumpCopy[entityTag][dump[entityTag].begin().key()][DATA_VALUE_DATA_TAG][dataTag],
//                          key, goodkey,false);
//     }
// }

// TEST(database_load_and_dump_tests, load_and_dump_modified_keys)
// {
//     // Read JSON
//     DatabaseDump dump = load_file(COMPLEX_DUMP_FILE);

//     // Entities
//     // check_entity_change_key(dump,HOST_CONTAINER_TAG);
//     // check_entity_change_key(dump,USER_CONTAINER_TAG);
//     // check_entity_change_key(dump,PROCESS_CONTAINER_TAG);
//     // check_entity_change_key(dump,DOMAIN_CONTAINER_TAG);
//     // check_entity_change_key(dump,TOPIC_CONTAINER_TAG);
//     // check_entity_change_key(dump,PARTICIPANT_CONTAINER_TAG);
//     // check_entity_change_key(dump,DATAWRITER_CONTAINER_TAG);
//     // check_entity_change_key(dump,DATAREADER_CONTAINER_TAG);
//     // check_entity_change_key(dump,LOCATOR_CONTAINER_TAG);

//     // Datas with index
//     // check_data_change_key(dump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_DISCOVERY_TIME_TAG);
//     // check_data_change_key(dump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_RTPS_PACKETS_SENT_TAG);
//     // check_data_change_key(dump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_RTPS_BYTES_SENT_TAG);
//     // check_data_change_key(dump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_RTPS_PACKETS_LOST_TAG);
//     // check_data_change_key(dump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_RTPS_BYTES_LOST_TAG);
//     // check_data_change_key(dump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_RTPS_PACKETS_SENT_LAST_REPORTED_TAG);
//     // check_data_change_key(dump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_RTPS_BYTES_SENT_LAST_REPORTED_TAG);
//     // check_data_change_key(dump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_RTPS_PACKETS_LOST_LAST_REPORTED_TAG);
//     // check_data_change_key(dump,PARTICIPANT_CONTAINER_TAG,DATA_KIND_RTPS_BYTES_LOST_LAST_REPORTED_TAG);

//     // check_data_change_key(dump,DATAWRITER_CONTAINER_TAG,DATA_KIND_SAMPLE_DATAS_TAG);
//     // check_data_change_key(dump,DATAWRITER_CONTAINER_TAG,DATA_KIND_FASTDDS_LATENCY_TAG);

//     // check_data_change_key(dump,LOCATOR_CONTAINER_TAG,DATA_KIND_NETWORK_LATENCY_TAG);
// }

void check_is_string(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& key)
{
    // Wrong
    std::vector<DatabaseDump> wrongValues = {true, 1234, 34.12, DatabaseDump::array({"9876"}), DatabaseDump::object(
                                                 {{"wasd", "zx"}})};
    for (auto wrongValue: wrongValues)
    {
        DatabaseDump dumpCopy = dump;
        dumpCopy[entityTag][dumpCopy[entityTag].begin().key()][key] = wrongValue;
        Database db;
        ASSERT_THROW(db.load_database(dumpCopy), DatabaseDump::exception);
    }

    // Correct
    std::vector<DatabaseDump> correctValues = {"qwerty", "true", "1234", "34.12", "[\"9876\"]", "{\"wasd\": \"zx\"}"};
    for (auto correctValue: correctValues)
    {
        DatabaseDump dumpCopy = dump;
        dumpCopy[entityTag][dumpCopy[entityTag].begin().key()][key] = correctValue;
        Database db;
        ASSERT_NO_THROW(db.load_database(dumpCopy));
    }
}

void check_is_int_id(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& key)
{
    // Wrong
    std::vector<DatabaseDump> wrongValues =
    {"qwerty", "true", "1234", "34.12", "[\"9876\"]", "{\"wasd\": \"zx\"}", true, 1234, 34.12,
     DatabaseDump::array({"9876"}), DatabaseDump::object({{"wasd", "zx"}})};

    for (auto wrongValue: wrongValues)
    {
        DatabaseDump dumpCopy = dump;
        dumpCopy[entityTag][dumpCopy[entityTag].begin().key()][key] = wrongValue;
        Database db;
        ASSERT_ANY_THROW(db.load_database(dumpCopy));
    }
}

void check_is_array_id(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& key)
{
    // Wrong
    std::vector<DatabaseDump> wrongValues =
    {"qwerty", "true", "1234", "34.12", "[\"9876\"]", "{\"wasd\": \"zx\"}", true, 1234, 34.12,
     DatabaseDump::array({"9876"}), DatabaseDump::object({{"wasd", "zx"}})};

    for (auto wrongValue: wrongValues)
    {
        DatabaseDump dumpCopy = dump;
        dumpCopy[entityTag][dumpCopy[entityTag].begin().key()][key] = wrongValue;
        Database db;
        ASSERT_ANY_THROW(db.load_database(dumpCopy));
    }
}

void check_is_object(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& key)
{
    // Wrong
    std::vector<DatabaseDump> wrongValues = {};
    for (auto wrongValue: wrongValues)
    {
        DatabaseDump dumpCopy = dump;
        dumpCopy[entityTag][dumpCopy[entityTag].begin().key()][key] = wrongValue;
        Database db;
        ASSERT_THROW(db.load_database(dumpCopy), DatabaseDump::exception);
    }

    // Correct
    std::vector<DatabaseDump> correctValues =
    {"qwerty", "true", "1234", "34.12", "[\"9876\"]", "{\"wasd\": \"zx\"}", true, 1234, 34.12,
             DatabaseDump::array({"9876"}), DatabaseDump::object({{"wasd", "zx"}})};
    for (auto correctValue: correctValues)
    {
        DatabaseDump dumpCopy = dump;
        dumpCopy[entityTag][dumpCopy[entityTag].begin().key()][key] = correctValue;
        Database db;
        ASSERT_NO_THROW(db.load_database(dumpCopy));
    }
}

TEST(database_load_and_dump_tests, load_and_dump_wrong_values)
{
    // Read JSON
    DatabaseDump dump = load_file(SIMPLE_DUMP_FILE);

    // object, string, float, int, array, bool

    // Host
    check_is_string(dump, HOST_CONTAINER_TAG, NAME_INFO_TAG);
    check_is_array_id(dump, HOST_CONTAINER_TAG, USER_CONTAINER_TAG);

    // User
    check_is_string(dump, USER_CONTAINER_TAG, NAME_INFO_TAG);
    check_is_int_id(dump, USER_CONTAINER_TAG, HOST_ENTITY_TAG);
    check_is_array_id(dump, USER_CONTAINER_TAG, PROCESS_CONTAINER_TAG);

    // Process
    check_is_string(dump, PROCESS_CONTAINER_TAG, NAME_INFO_TAG);
    check_is_string(dump, PROCESS_CONTAINER_TAG, PID_INFO_TAG);
    check_is_int_id(dump, PROCESS_CONTAINER_TAG, USER_ENTITY_TAG);
    check_is_array_id(dump, PROCESS_CONTAINER_TAG, PARTICIPANT_CONTAINER_TAG);

    // Domain
    check_is_string(dump, DOMAIN_CONTAINER_TAG, NAME_INFO_TAG);
    check_is_array_id(dump, DOMAIN_CONTAINER_TAG, PARTICIPANT_CONTAINER_TAG);
    check_is_array_id(dump, DOMAIN_CONTAINER_TAG, TOPIC_CONTAINER_TAG);

    // Topic
    check_is_string(dump, TOPIC_CONTAINER_TAG, NAME_INFO_TAG);
    check_is_string(dump, TOPIC_CONTAINER_TAG, DATA_TYPE_INFO_TAG);
    check_is_int_id(dump, TOPIC_CONTAINER_TAG, DOMAIN_ENTITY_TAG);
    check_is_array_id(dump, TOPIC_CONTAINER_TAG, DATAWRITER_CONTAINER_TAG);
    check_is_array_id(dump, TOPIC_CONTAINER_TAG, DATAREADER_CONTAINER_TAG);

    // Participant
    check_is_string(dump, PARTICIPANT_CONTAINER_TAG, NAME_INFO_TAG);
    check_is_string(dump, PARTICIPANT_CONTAINER_TAG, GUID_INFO_TAG);
    check_is_object(dump, PARTICIPANT_CONTAINER_TAG, QOS_INFO_TAG);
    check_is_int_id(dump, PARTICIPANT_CONTAINER_TAG, PROCESS_ENTITY_TAG);
    check_is_int_id(dump, PARTICIPANT_CONTAINER_TAG, DOMAIN_ENTITY_TAG);
    check_is_array_id(dump, PARTICIPANT_CONTAINER_TAG, DATAWRITER_CONTAINER_TAG);
    check_is_array_id(dump, PARTICIPANT_CONTAINER_TAG, DATAREADER_CONTAINER_TAG);

    // Locator
    check_is_string(dump, LOCATOR_CONTAINER_TAG, NAME_INFO_TAG);
    check_is_array_id(dump, LOCATOR_CONTAINER_TAG, DATAWRITER_CONTAINER_TAG);
    check_is_array_id(dump, LOCATOR_CONTAINER_TAG, DATAREADER_CONTAINER_TAG);

    // Datawriter
    check_is_string(dump, DATAWRITER_CONTAINER_TAG, NAME_INFO_TAG);
    check_is_string(dump, DATAWRITER_CONTAINER_TAG, GUID_INFO_TAG);
    check_is_object(dump, DATAWRITER_CONTAINER_TAG, QOS_INFO_TAG);
    check_is_int_id(dump, DATAWRITER_CONTAINER_TAG, PARTICIPANT_ENTITY_TAG);
    check_is_int_id(dump, DATAWRITER_CONTAINER_TAG, TOPIC_ENTITY_TAG);
    check_is_array_id(dump, DATAWRITER_CONTAINER_TAG, LOCATOR_CONTAINER_TAG);

    // Datareader
    check_is_string(dump, DATAREADER_CONTAINER_TAG, NAME_INFO_TAG);
    check_is_string(dump, DATAREADER_CONTAINER_TAG, GUID_INFO_TAG);
    check_is_object(dump, DATAREADER_CONTAINER_TAG, QOS_INFO_TAG);
    check_is_int_id(dump, DATAREADER_CONTAINER_TAG, PARTICIPANT_ENTITY_TAG);
    check_is_int_id(dump, DATAREADER_CONTAINER_TAG, TOPIC_ENTITY_TAG);
    check_is_array_id(dump, DATAREADER_CONTAINER_TAG, LOCATOR_CONTAINER_TAG);

}

// Check that the load of the 'dump 'without 'key' in 'container', throws an exception
void check_no_key(
        DatabaseDump& dump,
        DatabaseDump& container,
        std::string const& key,
        bool error = true)
{
    Database db;
    container.erase(key);
    if (error)
    {
        ASSERT_ANY_THROW(db.load_database(dump));
    }
    else
    {
        ASSERT_NO_THROW(db.load_database(dump));
    }
}

void check_entity_no_generic_keys(
        DatabaseDump& dump,
        std::string const& key)
{
    // No entities key
    {
        DatabaseDump dumpCopy = dump;
        check_no_key(dumpCopy, dumpCopy, key);
    }

    // No entity key
    {
        DatabaseDump dumpCopy = dump;
        check_no_key(dumpCopy, dumpCopy[key], dumpCopy[key].begin().key());
    }

    // No entity name key
    {
        DatabaseDump dumpCopy = dump;
        check_no_key(dumpCopy, dumpCopy[key][dumpCopy[key].begin().key()], NAME_INFO_TAG);
    }
}

void check_entity_no_key(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& keyTag)
{
    DatabaseDump dumpCopy = dump;
    check_no_key(dumpCopy, dumpCopy[entityTag][dumpCopy[entityTag].begin().key()], keyTag);
}

void check_data_no_key(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& keyTag)
{
    DatabaseDump dumpCopy = dump;
    check_no_key(dumpCopy, dumpCopy[entityTag][dumpCopy[entityTag].begin().key()][DATA_VALUE_DATA_TAG], keyTag);
}

void check_data_value_no_id_key(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& keyTag)
{
    DatabaseDump dumpCopy = dump;
    check_no_key(dumpCopy, dumpCopy[entityTag][dumpCopy[entityTag].begin().key()][DATA_VALUE_DATA_TAG][keyTag],
            dumpCopy[entityTag][dumpCopy[entityTag].begin().key()][DATA_VALUE_DATA_TAG][keyTag].begin().key(), false);
}

void check_data_value_index_no_key(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& keyTag,
        std::string const& data_param)
{
    DatabaseDump dumpCopy = dump;
    check_no_key(dumpCopy,
            dumpCopy[entityTag][dumpCopy[entityTag].begin().key()][DATA_VALUE_DATA_TAG][keyTag]
                    .begin().value().begin().value(),
            data_param);
}

void check_data_value_no_key(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& keyTag,
        std::string const& data_param)
{
    DatabaseDump dumpCopy = dump;
    check_no_key(dumpCopy,
            dumpCopy[entityTag][dumpCopy[entityTag].begin().key()][DATA_VALUE_DATA_TAG][keyTag]
                    .begin().value(),
            data_param);
}

void check_data_value_index_last_no_key(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& keyTag,
        std::string const& data_param)
{
    DatabaseDump dumpCopy = dump;
    check_no_key(dumpCopy,
            dumpCopy[entityTag][dumpCopy[entityTag].begin().key()][DATA_VALUE_DATA_TAG][keyTag]
                    .begin().value(),
            data_param);
}

void check_data_value_last_no_key(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& keyTag,
        std::string const& data_param)
{
    DatabaseDump dumpCopy = dump;
    check_no_key(dumpCopy,
            dumpCopy[entityTag][dumpCopy[entityTag].begin().key()][DATA_VALUE_DATA_TAG][keyTag],
            data_param);
}

// Test the load of a corrupted database with erased keys
TEST(database_load_and_dump_tests, load_and_dump_erased_keys)
{
    // Read JSON
    DatabaseDump dump = load_file(SIMPLE_DUMP_FILE);

    // ------------ HOSTS ----------------

    check_entity_no_generic_keys(dump, HOST_CONTAINER_TAG);

    check_entity_no_key(dump, HOST_CONTAINER_TAG, USER_CONTAINER_TAG);

    // ------------ USERS ----------------

    check_entity_no_generic_keys(dump, USER_CONTAINER_TAG);

    check_entity_no_key(dump, USER_CONTAINER_TAG, HOST_ENTITY_TAG);
    check_entity_no_key(dump, USER_CONTAINER_TAG, PROCESS_CONTAINER_TAG);

    // ------------ PROCESSES ----------------

    check_entity_no_generic_keys(dump, PROCESS_CONTAINER_TAG);

    check_entity_no_key(dump, PROCESS_CONTAINER_TAG, PID_INFO_TAG);
    check_entity_no_key(dump, PROCESS_CONTAINER_TAG, USER_ENTITY_TAG);
    check_entity_no_key(dump, PROCESS_CONTAINER_TAG, PARTICIPANT_CONTAINER_TAG);

    // ------------ DOMAINS ----------------

    check_entity_no_generic_keys(dump, DOMAIN_CONTAINER_TAG);

    check_entity_no_key(dump, DOMAIN_CONTAINER_TAG, PARTICIPANT_CONTAINER_TAG);
    check_entity_no_key(dump, DOMAIN_CONTAINER_TAG, TOPIC_CONTAINER_TAG);

    // ------------ TOPICS ----------------

    check_entity_no_generic_keys(dump, TOPIC_CONTAINER_TAG);

    check_entity_no_key(dump, TOPIC_CONTAINER_TAG, DATA_TYPE_INFO_TAG);
    check_entity_no_key(dump, TOPIC_CONTAINER_TAG, DOMAIN_ENTITY_TAG);
    check_entity_no_key(dump, TOPIC_CONTAINER_TAG, DATAWRITER_CONTAINER_TAG);
    check_entity_no_key(dump, TOPIC_CONTAINER_TAG, DATAREADER_CONTAINER_TAG);

    // ------------ PARTICIPANTS ----------------

    check_entity_no_generic_keys(dump, PARTICIPANT_CONTAINER_TAG);

    check_entity_no_key(dump, PARTICIPANT_CONTAINER_TAG, GUID_INFO_TAG);
    check_entity_no_key(dump, PARTICIPANT_CONTAINER_TAG, QOS_INFO_TAG);
    check_entity_no_key(dump, PARTICIPANT_CONTAINER_TAG, PROCESS_ENTITY_TAG);
    check_entity_no_key(dump, PARTICIPANT_CONTAINER_TAG, DOMAIN_ENTITY_TAG);
    check_entity_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATAWRITER_CONTAINER_TAG);
    check_entity_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATAREADER_CONTAINER_TAG);
    check_entity_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_VALUE_DATA_TAG);

    // ------------ LOCATORS ----------------

    check_entity_no_generic_keys(dump, LOCATOR_CONTAINER_TAG);

    check_entity_no_key(dump, LOCATOR_CONTAINER_TAG, DATAWRITER_CONTAINER_TAG);
    check_entity_no_key(dump, LOCATOR_CONTAINER_TAG, DATAREADER_CONTAINER_TAG);
    check_entity_no_key(dump, LOCATOR_CONTAINER_TAG, DATA_VALUE_DATA_TAG);

    // ------------ DATAWRITERS ----------------

    check_entity_no_generic_keys(dump, DATAWRITER_CONTAINER_TAG);

    check_entity_no_key(dump, DATAWRITER_CONTAINER_TAG, GUID_INFO_TAG);
    check_entity_no_key(dump, DATAWRITER_CONTAINER_TAG, QOS_INFO_TAG);
    check_entity_no_key(dump, DATAWRITER_CONTAINER_TAG, PARTICIPANT_ENTITY_TAG);
    check_entity_no_key(dump, DATAWRITER_CONTAINER_TAG, TOPIC_ENTITY_TAG);
    check_entity_no_key(dump, DATAWRITER_CONTAINER_TAG, LOCATOR_CONTAINER_TAG);
    check_entity_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_VALUE_DATA_TAG);

    // ------------ DATAREADERS ----------------

    check_entity_no_generic_keys(dump, DATAREADER_CONTAINER_TAG);

    check_entity_no_key(dump, DATAREADER_CONTAINER_TAG, GUID_INFO_TAG);
    check_entity_no_key(dump, DATAREADER_CONTAINER_TAG, QOS_INFO_TAG);
    check_entity_no_key(dump, DATAREADER_CONTAINER_TAG, PARTICIPANT_ENTITY_TAG);
    check_entity_no_key(dump, DATAREADER_CONTAINER_TAG, TOPIC_ENTITY_TAG);
    check_entity_no_key(dump, DATAREADER_CONTAINER_TAG, LOCATOR_CONTAINER_TAG);
    check_entity_no_key(dump, DATAREADER_CONTAINER_TAG, DATA_VALUE_DATA_TAG);

    // ------------ PARTICIPANT DATA ----------------

    check_data_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_DISCOVERY_TIME_TAG);
    check_data_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_PDP_PACKETS_TAG);
    check_data_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_EDP_PACKETS_TAG);
    check_data_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_PACKETS_SENT_TAG);
    check_data_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_SENT_TAG);
    check_data_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_PACKETS_LOST_TAG);
    check_data_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_LOST_TAG);
    check_data_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_LOST_LAST_REPORTED_TAG);
    check_data_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_SENT_LAST_REPORTED_TAG);
    check_data_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_PACKETS_LOST_LAST_REPORTED_TAG);
    check_data_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_PACKETS_SENT_LAST_REPORTED_TAG);
    check_data_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_EDP_PACKETS_LAST_REPORTED_TAG);
    check_data_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_PDP_PACKETS_LAST_REPORTED_TAG);

    check_data_value_no_id_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_DISCOVERY_TIME_TAG);
    check_data_value_no_id_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_PACKETS_SENT_TAG);
    check_data_value_no_id_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_SENT_TAG);
    check_data_value_no_id_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_PACKETS_LOST_TAG);
    check_data_value_no_id_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_LOST_TAG);
    check_data_value_no_id_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_LOST_LAST_REPORTED_TAG);
    check_data_value_no_id_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_SENT_LAST_REPORTED_TAG);
    check_data_value_no_id_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_PACKETS_LOST_LAST_REPORTED_TAG);
    check_data_value_no_id_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_PACKETS_SENT_LAST_REPORTED_TAG);

    check_data_value_index_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_DISCOVERY_TIME_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_index_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_DISCOVERY_TIME_TAG, DATA_VALUE_TIME_TAG);
    check_data_value_index_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_DISCOVERY_TIME_TAG,
            DATA_VALUE_REMOTE_ENTITY_TAG);
    check_data_value_index_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_DISCOVERY_TIME_TAG,
            DATA_VALUE_DISCOVERED_TAG);
    check_data_value_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_PDP_PACKETS_TAG, DATA_VALUE_SRC_TIME_TAG);
    check_data_value_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_PDP_PACKETS_TAG, DATA_VALUE_COUNT_TAG);
    check_data_value_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_EDP_PACKETS_TAG, DATA_VALUE_SRC_TIME_TAG);
    check_data_value_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_EDP_PACKETS_TAG, DATA_VALUE_COUNT_TAG);
    check_data_value_index_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_PACKETS_SENT_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_index_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_PACKETS_SENT_TAG,
            DATA_VALUE_COUNT_TAG);
    check_data_value_index_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_SENT_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_index_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_SENT_TAG, DATA_VALUE_COUNT_TAG);
    check_data_value_index_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_SENT_TAG,
            DATA_VALUE_MAGNITUDE_TAG);
    check_data_value_index_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_PACKETS_LOST_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_index_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_PACKETS_LOST_TAG,
            DATA_VALUE_COUNT_TAG);
    check_data_value_index_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_LOST_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_index_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_LOST_TAG, DATA_VALUE_COUNT_TAG);
    check_data_value_index_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_LOST_TAG,
            DATA_VALUE_MAGNITUDE_TAG);
    check_data_value_last_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_PDP_PACKETS_LAST_REPORTED_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_last_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_PDP_PACKETS_LAST_REPORTED_TAG,
            DATA_VALUE_COUNT_TAG);
    check_data_value_last_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_EDP_PACKETS_LAST_REPORTED_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_last_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_EDP_PACKETS_LAST_REPORTED_TAG,
            DATA_VALUE_COUNT_TAG);
    check_data_value_index_last_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_PACKETS_SENT_LAST_REPORTED_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_index_last_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_PACKETS_SENT_LAST_REPORTED_TAG,
            DATA_VALUE_COUNT_TAG);
    check_data_value_index_last_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_SENT_LAST_REPORTED_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_index_last_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_SENT_LAST_REPORTED_TAG,
            DATA_VALUE_COUNT_TAG);
    check_data_value_index_last_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_SENT_LAST_REPORTED_TAG,
            DATA_VALUE_MAGNITUDE_TAG);
    check_data_value_index_last_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_PACKETS_LOST_LAST_REPORTED_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_index_last_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_PACKETS_LOST_LAST_REPORTED_TAG,
            DATA_VALUE_COUNT_TAG);
    check_data_value_index_last_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_LOST_LAST_REPORTED_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_index_last_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_LOST_LAST_REPORTED_TAG,
            DATA_VALUE_COUNT_TAG);
    check_data_value_index_last_no_key(dump, PARTICIPANT_CONTAINER_TAG, DATA_KIND_RTPS_BYTES_LOST_LAST_REPORTED_TAG,
            DATA_VALUE_MAGNITUDE_TAG);

    // ------------ DATAWRITER DATA ----------------

    check_data_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_PUBLICATION_THROUGHPUT_TAG);
    check_data_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_RESENT_DATA_TAG);
    check_data_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_HEARTBEAT_COUNT_TAG);
    check_data_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_GAP_COUNT_TAG);
    check_data_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_DATA_COUNT_TAG);
    check_data_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_SAMPLE_DATAS_TAG);
    check_data_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_FASTDDS_LATENCY_TAG);
    check_data_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_DATA_COUNT_LAST_REPORTED_TAG);
    check_data_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_GAP_COUNT_LAST_REPORTED_TAG);
    check_data_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_HEARTBEAT_COUNT_LAST_REPORTED_TAG);
    check_data_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_RESENT_DATA_LAST_REPORTED_TAG);

    check_data_value_no_id_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_SAMPLE_DATAS_TAG);
    check_data_value_no_id_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_FASTDDS_LATENCY_TAG);

    check_data_value_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_PUBLICATION_THROUGHPUT_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_PUBLICATION_THROUGHPUT_TAG, DATA_VALUE_DATA_TAG);
    check_data_value_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_RESENT_DATA_TAG, DATA_VALUE_SRC_TIME_TAG);
    check_data_value_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_RESENT_DATA_TAG, DATA_VALUE_COUNT_TAG);
    check_data_value_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_HEARTBEAT_COUNT_TAG, DATA_VALUE_SRC_TIME_TAG);
    check_data_value_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_HEARTBEAT_COUNT_TAG, DATA_VALUE_COUNT_TAG);
    check_data_value_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_GAP_COUNT_TAG, DATA_VALUE_SRC_TIME_TAG);
    check_data_value_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_GAP_COUNT_TAG, DATA_VALUE_COUNT_TAG);
    check_data_value_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_DATA_COUNT_TAG, DATA_VALUE_SRC_TIME_TAG);
    check_data_value_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_DATA_COUNT_TAG, DATA_VALUE_COUNT_TAG);
    check_data_value_index_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_SAMPLE_DATAS_TAG, DATA_VALUE_SRC_TIME_TAG);
    check_data_value_index_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_SAMPLE_DATAS_TAG, DATA_VALUE_COUNT_TAG);
    check_data_value_index_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_FASTDDS_LATENCY_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_index_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_FASTDDS_LATENCY_TAG, DATA_VALUE_DATA_TAG);
    check_data_value_last_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_DATA_COUNT_LAST_REPORTED_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_last_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_DATA_COUNT_LAST_REPORTED_TAG,
            DATA_VALUE_COUNT_TAG);
    check_data_value_last_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_GAP_COUNT_LAST_REPORTED_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_last_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_GAP_COUNT_LAST_REPORTED_TAG,
            DATA_VALUE_COUNT_TAG);
    check_data_value_last_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_HEARTBEAT_COUNT_LAST_REPORTED_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_last_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_HEARTBEAT_COUNT_LAST_REPORTED_TAG,
            DATA_VALUE_COUNT_TAG);
    check_data_value_last_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_RESENT_DATA_LAST_REPORTED_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_last_no_key(dump, DATAWRITER_CONTAINER_TAG, DATA_KIND_RESENT_DATA_LAST_REPORTED_TAG,
            DATA_VALUE_COUNT_TAG);

    // ------------ DATAREADER DATA ----------------

    check_data_no_key(dump, DATAREADER_CONTAINER_TAG, DATA_KIND_SUBSCRIPTION_THROUGHPUT_TAG);
    check_data_no_key(dump, DATAREADER_CONTAINER_TAG, DATA_KIND_ACKNACK_COUNT_TAG);
    check_data_no_key(dump, DATAREADER_CONTAINER_TAG, DATA_KIND_NACKFRAG_COUNT_TAG);
    check_data_no_key(dump, DATAREADER_CONTAINER_TAG, DATA_KIND_ACKNACK_COUNT_LAST_REPORTED_TAG);
    check_data_no_key(dump, DATAREADER_CONTAINER_TAG, DATA_KIND_NACKFRAG_COUNT_LAST_REPORTED_TAG);

    check_data_value_no_key(dump, DATAREADER_CONTAINER_TAG, DATA_KIND_SUBSCRIPTION_THROUGHPUT_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_no_key(dump, DATAREADER_CONTAINER_TAG, DATA_KIND_SUBSCRIPTION_THROUGHPUT_TAG, DATA_VALUE_DATA_TAG);
    check_data_value_no_key(dump, DATAREADER_CONTAINER_TAG, DATA_KIND_ACKNACK_COUNT_TAG, DATA_VALUE_SRC_TIME_TAG);
    check_data_value_no_key(dump, DATAREADER_CONTAINER_TAG, DATA_KIND_ACKNACK_COUNT_TAG, DATA_VALUE_COUNT_TAG);
    check_data_value_no_key(dump, DATAREADER_CONTAINER_TAG, DATA_KIND_NACKFRAG_COUNT_TAG, DATA_VALUE_SRC_TIME_TAG);
    check_data_value_no_key(dump, DATAREADER_CONTAINER_TAG, DATA_KIND_NACKFRAG_COUNT_TAG, DATA_VALUE_COUNT_TAG);

    check_data_value_last_no_key(dump, DATAREADER_CONTAINER_TAG, DATA_KIND_ACKNACK_COUNT_LAST_REPORTED_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_last_no_key(dump, DATAREADER_CONTAINER_TAG, DATA_KIND_ACKNACK_COUNT_LAST_REPORTED_TAG,
            DATA_VALUE_COUNT_TAG);

    check_data_value_last_no_key(dump, DATAREADER_CONTAINER_TAG, DATA_KIND_NACKFRAG_COUNT_LAST_REPORTED_TAG,
            DATA_VALUE_SRC_TIME_TAG);
    check_data_value_last_no_key(dump, DATAREADER_CONTAINER_TAG, DATA_KIND_NACKFRAG_COUNT_LAST_REPORTED_TAG,
            DATA_VALUE_COUNT_TAG);

    // ------------ LOCATOR DATA ----------------

    check_data_no_key(dump, LOCATOR_CONTAINER_TAG, DATA_KIND_NETWORK_LATENCY_TAG);

    check_data_value_no_id_key(dump, LOCATOR_CONTAINER_TAG, DATA_KIND_NETWORK_LATENCY_TAG);
    check_data_value_index_no_key(dump, LOCATOR_CONTAINER_TAG, DATA_KIND_NETWORK_LATENCY_TAG, DATA_VALUE_SRC_TIME_TAG);
    check_data_value_index_no_key(dump, LOCATOR_CONTAINER_TAG, DATA_KIND_NETWORK_LATENCY_TAG, DATA_VALUE_DATA_TAG);
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