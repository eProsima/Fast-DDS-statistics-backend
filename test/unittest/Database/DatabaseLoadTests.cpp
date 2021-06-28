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

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <exception/Exception.hpp>
#include <types/JSONTags.h>
#include <types/types.hpp>

#include <database/database.hpp>
#include <DatabaseUtils.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

/**
 * Auxiliar function for the load_and_dump tests.
 * This function:
 * 1. Read a .json file, storing it in a dump.
 * 2. Load a database with the dump
 * 3. Make a dump of the loaded database
 * 4. Compare that the .json dump and the loaded dump is the same
 */
void load_and_dump(
        std::string filename)
{
    // Read JSON
    DatabaseDump dump;
    load_file(filename, dump);

    // Create database
    Database db;

    // Load dump in the database
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

// Test the load of a dump database without any entity
TEST(database_load_tests, load_and_dump_empty_database)
{
    load_and_dump(EMPTY_DUMP_FILE);
}

// Test the load of a dump database with one entity of each kind
TEST(database_load_tests, load_and_dump_empty_entities_database)
{
    load_and_dump(EMPTY_ENTITIES_DUMP_FILE);
}

// Test the load of a dump database with one entity of each kind and one data of each kind
TEST(database_load_tests, load_and_dump_simple_database)
{
    load_and_dump(SIMPLE_DUMP_FILE);
}

// Test the load of a dump database with three entities of each kind and three datas of each kind
TEST(database_load_tests, load_and_dump_complex_database)
{
    load_and_dump(COMPLEX_DUMP_FILE);
}

// Test the load of an old dump with several samples_data for the same sequence number
TEST(database_load_tests, load_and_dump_old_complex_database)
{
    // Read JSON
    DatabaseDump dump_old;
    load_file(OLD_COMPLEX_DUMP_FILE, dump_old);
    DatabaseDump dump;
    load_file(COMPLEX_DUMP_FILE, dump);

    // Create database
    Database db;

    // Load dump in the database
    db.load_database(dump_old);

    // Dump loaded database
    DatabaseDump loadedDump = db.dump_database();

    // Compare two dumps
    ASSERT_EQ(dump_old[HOST_CONTAINER_TAG], loadedDump[HOST_CONTAINER_TAG]);
    ASSERT_EQ(dump_old[USER_CONTAINER_TAG], loadedDump[USER_CONTAINER_TAG]);
    ASSERT_EQ(dump_old[PROCESS_CONTAINER_TAG], loadedDump[PROCESS_CONTAINER_TAG]);
    ASSERT_EQ(dump_old[DOMAIN_CONTAINER_TAG], loadedDump[DOMAIN_CONTAINER_TAG]);
    ASSERT_EQ(dump_old[TOPIC_CONTAINER_TAG], loadedDump[TOPIC_CONTAINER_TAG]);
    ASSERT_EQ(dump_old[PARTICIPANT_CONTAINER_TAG], loadedDump[PARTICIPANT_CONTAINER_TAG]);
    ASSERT_EQ(dump_old[LOCATOR_CONTAINER_TAG], loadedDump[LOCATOR_CONTAINER_TAG]);
    ASSERT_EQ(dump_old[DATAREADER_CONTAINER_TAG], loadedDump[DATAREADER_CONTAINER_TAG]);
    // Sample_datas are related to the DataWriter
    ASSERT_EQ(dump[DATAWRITER_CONTAINER_TAG], loadedDump[DATAWRITER_CONTAINER_TAG]);

    // The old dump is not equal to the loadedDump because there are only one sample_data per sequence number
    ASSERT_EQ(dump, loadedDump);
}

// Test the load of a dump database with one entity of each kind
TEST(database_load_tests, load_twice)
{
    // Read JSON
    DatabaseDump dump;
    load_file(SIMPLE_DUMP_FILE, dump);

    // Create database
    Database db;

    // Load dump in the database
    db.load_database(dump);
    ASSERT_THROW(db.load_database(dump), PreconditionNotMet);
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
        DatabaseDump dump_copy = dump;
        check_no_key(dump_copy, dump_copy, key);
    }

    // No entity key
    {
        DatabaseDump dump_copy = dump;
        check_no_key(dump_copy, dump_copy[key], dump_copy[key].begin().key());
    }

    // No entity name key
    {
        DatabaseDump dump_copy = dump;
        check_no_key(dump_copy, dump_copy[key][dump_copy[key].begin().key()], NAME_INFO_TAG);
    }
}

void check_entity_no_key(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& keyTag)
{
    DatabaseDump dump_copy = dump;
    check_no_key(dump_copy, dump_copy[entityTag][dump_copy[entityTag].begin().key()], keyTag);
}

void check_data_no_key(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& keyTag)
{
    DatabaseDump dump_copy = dump;
    check_no_key(dump_copy, dump_copy[entityTag][dump_copy[entityTag].begin().key()][DATA_VALUE_DATA_TAG], keyTag);
}

void check_data_value_no_id_key(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& keyTag)
{
    DatabaseDump dump_copy = dump;
    check_no_key(dump_copy, dump_copy[entityTag][dump_copy[entityTag].begin().key()][DATA_VALUE_DATA_TAG][keyTag],
            dump_copy[entityTag][dump_copy[entityTag].begin().key()][DATA_VALUE_DATA_TAG][keyTag].begin().key(), false);
}

void check_data_value_index_no_key(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& keyTag,
        std::string const& data_param)
{
    DatabaseDump dump_copy = dump;
    check_no_key(dump_copy,
            dump_copy[entityTag][dump_copy[entityTag].begin().key()][DATA_VALUE_DATA_TAG][keyTag]
                    .begin().value().begin().value(),
            data_param);
}

void check_data_value_no_key(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& keyTag,
        std::string const& data_param)
{
    DatabaseDump dump_copy = dump;
    check_no_key(dump_copy,
            dump_copy[entityTag][dump_copy[entityTag].begin().key()][DATA_VALUE_DATA_TAG][keyTag]
                    .begin().value(),
            data_param);
}

void check_data_value_index_last_no_key(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& keyTag,
        std::string const& data_param)
{
    DatabaseDump dump_copy = dump;
    check_no_key(dump_copy,
            dump_copy[entityTag][dump_copy[entityTag].begin().key()][DATA_VALUE_DATA_TAG][keyTag]
                    .begin().value(),
            data_param);
}

void check_data_value_last_no_key(
        DatabaseDump& dump,
        std::string const& entityTag,
        std::string const& keyTag,
        std::string const& data_param)
{
    DatabaseDump dump_copy = dump;
    check_no_key(dump_copy,
            dump_copy[entityTag][dump_copy[entityTag].begin().key()][DATA_VALUE_DATA_TAG][keyTag],
            data_param);
}

// Test the load of a corrupted database with erased keys
TEST(database_load_tests, load_erased_keys)
{
    // Read JSON
    DatabaseDump dump;
    load_file(SIMPLE_DUMP_FILE, dump);

    constexpr const char* char_message = "test";
    std::string string_message = "test";
    ASSERT_ANY_THROW(throw CorruptedFile(char_message));
    ASSERT_ANY_THROW(throw CorruptedFile(string_message));

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

void check_and_restore(
        DatabaseDump& dump,
        DatabaseDump& container,
        std::vector<DatabaseDump>const& wrongValues,
        std::vector<DatabaseDump>const& correctValues)
{
    // Wrong
    for (auto wrongValue: wrongValues)
    {
        DatabaseDump aux = DatabaseDump(container);
        container = wrongValue;
        Database db;
        ASSERT_ANY_THROW(db.load_database(dump));
        container = aux;
    }

    for (auto correctValue: correctValues)
    {
        DatabaseDump aux = DatabaseDump(container);
        container = correctValue;
        Database db;
        ASSERT_NO_THROW(db.load_database(dump));
        container = aux;
    }
}

void check_is_string(
        DatabaseDump& dump,
        DatabaseDump& container)
{
    std::vector<DatabaseDump> wrongValues = {DatabaseDump::object({{"one", 1}, {"two", 2} }), DatabaseDump::array(
                                                 {1, 2, 3, 4}), true, 1234, 34.12, -9876, -76.98};
    std::vector<DatabaseDump> correctValues = {"qwerty", "1234", "34.12", "-9876", "-76.98"};
    check_and_restore(dump, container, wrongValues, correctValues);
}

void check_is_id(
        DatabaseDump& dump,
        DatabaseDump& container)
{
    std::vector<DatabaseDump> wrongValues = {DatabaseDump::object({{"one", 1}, {"two", 2}}), DatabaseDump::array({1, 2,
                                                                                                                  3,
                                                                                                                  4}),
                                             true, 1234, 34.12, -9876, -76.98, "qwerty", "1234", "34.12", "-9876",
                                             "-76.98"};

    std::vector<DatabaseDump> correctValues = {};
    check_and_restore(dump, container, wrongValues, correctValues);
}

void check_is_object(
        DatabaseDump& dump,
        DatabaseDump& container)
{
    std::vector<DatabaseDump> wrongValues = {};
    std::vector<DatabaseDump> correctValues = {DatabaseDump::object({{"one", 1}, {"two", 2}}), DatabaseDump::array({1,
                                                                                                                    2,
                                                                                                                    3,
                                                                                                                    4}),
                                               true, 1234, 34.12, -9876, -76.98, "qwerty", "1234", "34.12", "-9876",
                                               "-76.98"};

    check_and_restore(dump, container, wrongValues, correctValues);
}

void check_is_string_int(
        DatabaseDump& dump,
        DatabaseDump& container)
{
    std::vector<DatabaseDump> wrongValues = {DatabaseDump::object({{"one", 1}, {"two", 2}}), DatabaseDump::array({1, 2,
                                                                                                                  3,
                                                                                                                  4}),
                                             true, 1234, 34.12, -9876, -76.98, "qwerty", "34.12", "-76.98"};

    std::vector<DatabaseDump> correctValues = {"1234", "-9876"};
    check_and_restore(dump, container, wrongValues, correctValues);
}

void check_is_string_uint(
        DatabaseDump& dump,
        DatabaseDump& container)
{
    std::vector<DatabaseDump> wrongValues = {DatabaseDump::object({{"one", 1}, {"two", 2}}), DatabaseDump::array({1, 2,
                                                                                                                  3,
                                                                                                                  4}),
                                             true, 1234, 34.12, -9876, -76.98, "qwerty", "34.12", "-9876", "-76.98"};

    std::vector<DatabaseDump> correctValues = {"1234"};
    check_and_restore(dump, container, wrongValues, correctValues);
}

void check_is_bool(
        DatabaseDump& dump,
        DatabaseDump& container)
{
    std::vector<DatabaseDump> wrongValues = {DatabaseDump::object({{"one", 1}, {"two", 2}}), DatabaseDump::array({1, 2,
                                                                                                                  3,
                                                                                                                  4}),
                                             1234, 34.12, -9876, -76.98, "qwerty", "1234", "34.12", "-9876", "-76.98"};

    std::vector<DatabaseDump> correctValues = {true};
    check_and_restore(dump, container, wrongValues, correctValues);
}

void check_is_uint(
        DatabaseDump& dump,
        DatabaseDump& container)
{
    std::vector<DatabaseDump> wrongValues = {DatabaseDump::object({{"one", 1}, {"two", 2}}), DatabaseDump::array({1, 2,
                                                                                                                  3,
                                                                                                                  4}),
                                             true, 34.12, -9876, -76.98, "qwerty", "1234", "34.12", "-9876", "-76.98"};

    std::vector<DatabaseDump> correctValues = {1234};
    check_and_restore(dump, container, wrongValues, correctValues);
}

void check_is_int(
        DatabaseDump& dump,
        DatabaseDump& container)
{
    std::vector<DatabaseDump> wrongValues = {DatabaseDump::object({{"one", 1}, {"two", 2}}), DatabaseDump::array({1, 2,
                                                                                                                  3,
                                                                                                                  4}),
                                             true, 34.12, -76.98, "qwerty", "1234", "34.12", "-9876", "-76.98"};

    std::vector<DatabaseDump> correctValues = {1234, -9876};
    check_and_restore(dump, container, wrongValues, correctValues);
}

void check_is_double(
        DatabaseDump& dump,
        DatabaseDump& container)
{
    std::vector<DatabaseDump> wrongValues = {DatabaseDump::object({{"one", 1}, {"two", 2}}), DatabaseDump::array({1, 2,
                                                                                                                  3,
                                                                                                                  4}),
                                             true, "qwerty", "1234", "34.12", "-9876", "-76.98"};

    std::vector<DatabaseDump> correctValues = {1234, 34.12, -9876, -76.98};
    check_and_restore(dump, container, wrongValues, correctValues);
}

// Test the load of a database with wrong value types
TEST(database_load_tests, load_wrong_values)
{
    // Read JSON
    DatabaseDump dump;
    load_file(SIMPLE_DUMP_FILE, dump);

    // object, string, float, int, array, bool

    // Host
    check_is_id(dump, dump[HOST_CONTAINER_TAG]);
    check_is_id(dump, dump[HOST_CONTAINER_TAG].begin().value());
    check_is_string(dump, dump[HOST_CONTAINER_TAG].begin().value()[NAME_INFO_TAG]);
    check_is_string(dump, dump[HOST_CONTAINER_TAG].begin().value()[ALIAS_INFO_TAG]);
    check_is_id(dump, dump[HOST_CONTAINER_TAG].begin().value()[USER_CONTAINER_TAG]);

    // User
    check_is_id(dump, dump[USER_CONTAINER_TAG]);
    check_is_id(dump, dump[USER_CONTAINER_TAG].begin().value());
    check_is_string(dump, dump[USER_CONTAINER_TAG].begin().value()[NAME_INFO_TAG]);
    check_is_string(dump, dump[USER_CONTAINER_TAG].begin().value()[ALIAS_INFO_TAG]);
    check_is_id(dump, dump[USER_CONTAINER_TAG].begin().value()[HOST_ENTITY_TAG]);
    check_is_id(dump, dump[USER_CONTAINER_TAG].begin().value()[PROCESS_CONTAINER_TAG]);

    // Process
    check_is_id(dump, dump[PROCESS_CONTAINER_TAG]);
    check_is_id(dump, dump[PROCESS_CONTAINER_TAG].begin().value());
    check_is_string(dump, dump[PROCESS_CONTAINER_TAG].begin().value()[NAME_INFO_TAG]);
    check_is_string(dump, dump[PROCESS_CONTAINER_TAG].begin().value()[ALIAS_INFO_TAG]);
    check_is_string(dump, dump[PROCESS_CONTAINER_TAG].begin().value()[PID_INFO_TAG]);
    check_is_id(dump, dump[PROCESS_CONTAINER_TAG].begin().value()[USER_ENTITY_TAG]);
    check_is_id(dump, dump[PROCESS_CONTAINER_TAG].begin().value()[PARTICIPANT_CONTAINER_TAG]);

    // Domain
    check_is_id(dump, dump[DOMAIN_CONTAINER_TAG]);
    check_is_id(dump, dump[DOMAIN_CONTAINER_TAG].begin().value());
    check_is_string(dump, dump[DOMAIN_CONTAINER_TAG].begin().value()[NAME_INFO_TAG]);
    check_is_string(dump, dump[DOMAIN_CONTAINER_TAG].begin().value()[ALIAS_INFO_TAG]);
    check_is_id(dump, dump[DOMAIN_CONTAINER_TAG].begin().value()[PARTICIPANT_CONTAINER_TAG]);
    check_is_id(dump, dump[DOMAIN_CONTAINER_TAG].begin().value()[TOPIC_CONTAINER_TAG]);

    // Topic
    check_is_id(dump, dump[TOPIC_CONTAINER_TAG]);
    check_is_id(dump, dump[TOPIC_CONTAINER_TAG].begin().value());
    check_is_string(dump, dump[TOPIC_CONTAINER_TAG].begin().value()[NAME_INFO_TAG]);
    check_is_string(dump, dump[TOPIC_CONTAINER_TAG].begin().value()[ALIAS_INFO_TAG]);
    check_is_string(dump, dump[TOPIC_CONTAINER_TAG].begin().value()[DATA_TYPE_INFO_TAG]);
    check_is_id(dump, dump[TOPIC_CONTAINER_TAG].begin().value()[DOMAIN_ENTITY_TAG]);
    check_is_id(dump, dump[TOPIC_CONTAINER_TAG].begin().value()[DATAWRITER_CONTAINER_TAG]);
    check_is_id(dump, dump[TOPIC_CONTAINER_TAG].begin().value()[DATAREADER_CONTAINER_TAG]);

    // Participant
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG]);
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value());
    check_is_string(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[NAME_INFO_TAG]);
    check_is_string(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[ALIAS_INFO_TAG]);
    check_is_string(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[GUID_INFO_TAG]);
    check_is_object(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[QOS_INFO_TAG]);
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[PROCESS_ENTITY_TAG]);
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DOMAIN_ENTITY_TAG]);
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATAWRITER_CONTAINER_TAG]);
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATAREADER_CONTAINER_TAG]);
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]);

    // Locator
    check_is_id(dump, dump[LOCATOR_CONTAINER_TAG]);
    check_is_id(dump, dump[LOCATOR_CONTAINER_TAG].begin().value());
    check_is_string(dump, dump[LOCATOR_CONTAINER_TAG].begin().value()[NAME_INFO_TAG]);
    check_is_string(dump, dump[LOCATOR_CONTAINER_TAG].begin().value()[ALIAS_INFO_TAG]);
    check_is_id(dump, dump[LOCATOR_CONTAINER_TAG].begin().value()[DATAWRITER_CONTAINER_TAG]);
    check_is_id(dump, dump[LOCATOR_CONTAINER_TAG].begin().value()[DATAREADER_CONTAINER_TAG]);
    check_is_id(dump, dump[LOCATOR_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]);

    // Datawriter
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG]);
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value());
    check_is_string(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[NAME_INFO_TAG]);
    check_is_string(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[ALIAS_INFO_TAG]);
    check_is_string(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[GUID_INFO_TAG]);
    check_is_object(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[QOS_INFO_TAG]);
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[PARTICIPANT_ENTITY_TAG]);
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[TOPIC_ENTITY_TAG]);
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[LOCATOR_CONTAINER_TAG]);
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]);

    // Datareader
    check_is_id(dump, dump[DATAREADER_CONTAINER_TAG]);
    check_is_id(dump, dump[DATAREADER_CONTAINER_TAG].begin().value());
    check_is_string(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[NAME_INFO_TAG]);
    check_is_string(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[ALIAS_INFO_TAG]);
    check_is_string(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[GUID_INFO_TAG]);
    check_is_object(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[QOS_INFO_TAG]);
    check_is_id(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[PARTICIPANT_ENTITY_TAG]);
    check_is_id(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[TOPIC_ENTITY_TAG]);
    check_is_id(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[LOCATOR_CONTAINER_TAG]);
    check_is_id(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]);

    /* _________________DATA_____________________ */

    // --------------------- Participants ---------------------

    // discovery_time
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_DISCOVERY_TIME_TAG]);
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_DISCOVERY_TIME_TAG].begin().value());
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_DISCOVERY_TIME_TAG].begin().value().begin().value());
    check_is_string_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_DISCOVERY_TIME_TAG].begin().value().begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_string_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_DISCOVERY_TIME_TAG].begin().value().begin().value()[DATA_VALUE_TIME_TAG]);
    check_is_string_int(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_DISCOVERY_TIME_TAG].begin().value().begin().value()[DATA_VALUE_REMOTE_ENTITY_TAG]);
    check_is_bool(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_DISCOVERY_TIME_TAG].begin().value().begin().value()[DATA_VALUE_DISCOVERED_TAG]);

    // pdp_packets
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_PDP_PACKETS_TAG]);
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_PDP_PACKETS_TAG].begin().value());
    check_is_string_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_PDP_PACKETS_TAG].begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_PDP_PACKETS_TAG].begin().value()[DATA_VALUE_COUNT_TAG]);

    // edp_packets
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_EDP_PACKETS_TAG]);
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_EDP_PACKETS_TAG].begin().value());
    check_is_string_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_EDP_PACKETS_TAG].begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_EDP_PACKETS_TAG].begin().value()[DATA_VALUE_COUNT_TAG]);

    // rtps_packets_sent
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_SENT_TAG]);
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_SENT_TAG].begin().value());
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_SENT_TAG].begin().value().begin().value());
    check_is_string_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_SENT_TAG].begin().value().begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_SENT_TAG].begin().value().begin().value()[DATA_VALUE_COUNT_TAG]);

    // rtps_bytes_sent
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_SENT_TAG]);
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_SENT_TAG].begin().value());
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_SENT_TAG].begin().value().begin().value());
    check_is_string_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_SENT_TAG].begin().value().begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_int(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_SENT_TAG].begin().value().begin().value()[DATA_VALUE_MAGNITUDE_TAG]);
    check_is_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_SENT_TAG].begin().value().begin().value()[DATA_VALUE_COUNT_TAG]);

    // rtps_packets_lost
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_LOST_TAG]);
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_LOST_TAG].begin().value());
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_LOST_TAG].begin().value().begin().value());
    check_is_string_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_LOST_TAG].begin().value().begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_LOST_TAG].begin().value().begin().value()[DATA_VALUE_COUNT_TAG]);

    // rtps_bytes_lost
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_LOST_TAG]);
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_LOST_TAG].begin().value());
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_LOST_TAG].begin().value().begin().value());
    check_is_string_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_LOST_TAG].begin().value().begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_int(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_LOST_TAG].begin().value().begin().value()[DATA_VALUE_MAGNITUDE_TAG]);
    check_is_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_LOST_TAG].begin().value().begin().value()[DATA_VALUE_COUNT_TAG]);

    // last_reported_edp_packets
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_EDP_PACKETS_LAST_REPORTED_TAG]);
    check_is_string_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_EDP_PACKETS_LAST_REPORTED_TAG][DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_EDP_PACKETS_LAST_REPORTED_TAG][DATA_VALUE_COUNT_TAG]);

    // last_reported_pdp_packets
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_PDP_PACKETS_LAST_REPORTED_TAG]);
    check_is_string_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_PDP_PACKETS_LAST_REPORTED_TAG][DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_PDP_PACKETS_LAST_REPORTED_TAG][DATA_VALUE_COUNT_TAG]);

    // last_reported_rtps_bytes_lost
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_LOST_LAST_REPORTED_TAG]);
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_LOST_LAST_REPORTED_TAG].begin().value());
    check_is_string_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_LOST_LAST_REPORTED_TAG].begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_int(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_LOST_LAST_REPORTED_TAG].begin().value()[DATA_VALUE_MAGNITUDE_TAG]);
    check_is_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_LOST_LAST_REPORTED_TAG].begin().value()[DATA_VALUE_COUNT_TAG]);

    // last_reported_rtps_bytes_sent
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_SENT_LAST_REPORTED_TAG]);
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_SENT_LAST_REPORTED_TAG].begin().value());
    check_is_string_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_SENT_LAST_REPORTED_TAG].begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_int(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_SENT_LAST_REPORTED_TAG].begin().value()[DATA_VALUE_MAGNITUDE_TAG]);
    check_is_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_BYTES_SENT_LAST_REPORTED_TAG].begin().value()[DATA_VALUE_COUNT_TAG]);

    // last_reported_rtps_packets_lost
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_LOST_LAST_REPORTED_TAG]);
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_LOST_LAST_REPORTED_TAG].begin().value());
    check_is_string_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_LOST_LAST_REPORTED_TAG].begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_LOST_LAST_REPORTED_TAG].begin().value()[DATA_VALUE_COUNT_TAG]);

    // last_reported_rtps_packets_sent
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_SENT_LAST_REPORTED_TAG]);
    check_is_id(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_SENT_LAST_REPORTED_TAG].begin().value());
    check_is_string_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_SENT_LAST_REPORTED_TAG].begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[PARTICIPANT_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RTPS_PACKETS_SENT_LAST_REPORTED_TAG].begin().value()[DATA_VALUE_COUNT_TAG]);


    // --------------------- Datawriters ---------------------

    // publication_throughput
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_PUBLICATION_THROUGHPUT_TAG]);
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_PUBLICATION_THROUGHPUT_TAG].begin().value());
    check_is_string_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_PUBLICATION_THROUGHPUT_TAG].begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_double(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_PUBLICATION_THROUGHPUT_TAG].begin().value()[DATA_VALUE_DATA_TAG]);

    // resent_datas
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RESENT_DATA_TAG]);
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RESENT_DATA_TAG].begin().value());
    check_is_string_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RESENT_DATA_TAG].begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RESENT_DATA_TAG].begin().value()[DATA_VALUE_COUNT_TAG]);

    // heartbeat_count
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_HEARTBEAT_COUNT_TAG]);
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_HEARTBEAT_COUNT_TAG].begin().value());
    check_is_string_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_HEARTBEAT_COUNT_TAG].begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_HEARTBEAT_COUNT_TAG].begin().value()[DATA_VALUE_COUNT_TAG]);

    // gap_count
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_GAP_COUNT_TAG]);
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_GAP_COUNT_TAG].begin().value());
    check_is_string_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_GAP_COUNT_TAG].begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_GAP_COUNT_TAG].begin().value()[DATA_VALUE_COUNT_TAG]);

    // data_count
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_DATA_COUNT_TAG]);
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_DATA_COUNT_TAG].begin().value());
    check_is_string_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_DATA_COUNT_TAG].begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_DATA_COUNT_TAG].begin().value()[DATA_VALUE_COUNT_TAG]);

    // samples_datas
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_SAMPLE_DATAS_TAG]);
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_SAMPLE_DATAS_TAG].begin().value());
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_SAMPLE_DATAS_TAG].begin().value().begin().value());
    check_is_string_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_SAMPLE_DATAS_TAG].begin().value().begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_SAMPLE_DATAS_TAG].begin().value().begin().value()[DATA_VALUE_COUNT_TAG]);

    // history2history_latency
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_FASTDDS_LATENCY_TAG]);
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_FASTDDS_LATENCY_TAG].begin().value());
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_FASTDDS_LATENCY_TAG].begin().value().begin().value());
    check_is_string_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_FASTDDS_LATENCY_TAG].begin().value().begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_double(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_FASTDDS_LATENCY_TAG].begin().value().begin().value()[DATA_VALUE_DATA_TAG]);

    // last_reported_data_count
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_DATA_COUNT_LAST_REPORTED_TAG]);
    check_is_string_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_DATA_COUNT_LAST_REPORTED_TAG][DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_VALUE_DATA_TAG]
            [DATA_KIND_DATA_COUNT_LAST_REPORTED_TAG][DATA_VALUE_COUNT_TAG]);

    // last_reported_gap_count
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_GAP_COUNT_LAST_REPORTED_TAG]);
    check_is_string_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_GAP_COUNT_LAST_REPORTED_TAG][DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_GAP_COUNT_LAST_REPORTED_TAG][DATA_VALUE_COUNT_TAG]);

    // last_reported_heartbeat_count
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_HEARTBEAT_COUNT_LAST_REPORTED_TAG]);
    check_is_string_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_HEARTBEAT_COUNT_LAST_REPORTED_TAG][DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_HEARTBEAT_COUNT_LAST_REPORTED_TAG][DATA_VALUE_COUNT_TAG]);

    // last_reported_resent_datas
    check_is_id(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RESENT_DATA_LAST_REPORTED_TAG]);
    check_is_string_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RESENT_DATA_LAST_REPORTED_TAG][DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[DATAWRITER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_RESENT_DATA_LAST_REPORTED_TAG][DATA_VALUE_COUNT_TAG]);

    // --------------------- Datareaders ---------------------

    // subscription_throughput
    check_is_id(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_SUBSCRIPTION_THROUGHPUT_TAG]);
    check_is_id(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_SUBSCRIPTION_THROUGHPUT_TAG].begin().value());
    check_is_string_uint(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_SUBSCRIPTION_THROUGHPUT_TAG].begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_double(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_SUBSCRIPTION_THROUGHPUT_TAG].begin().value()[DATA_VALUE_DATA_TAG]);

    // acknack_count
    check_is_id(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_ACKNACK_COUNT_TAG]);
    check_is_id(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_ACKNACK_COUNT_TAG].begin().value());
    check_is_string_uint(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_ACKNACK_COUNT_TAG].begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_ACKNACK_COUNT_TAG].begin().value()[DATA_VALUE_COUNT_TAG]);

    // nackfrag_count
    check_is_id(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_NACKFRAG_COUNT_TAG]);
    check_is_id(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_NACKFRAG_COUNT_TAG].begin().value());
    check_is_string_uint(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_NACKFRAG_COUNT_TAG].begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_NACKFRAG_COUNT_TAG].begin().value()[DATA_VALUE_COUNT_TAG]);

    // last_reported_acknack_count
    check_is_id(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_ACKNACK_COUNT_LAST_REPORTED_TAG]);
    check_is_string_uint(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_ACKNACK_COUNT_LAST_REPORTED_TAG][DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_ACKNACK_COUNT_LAST_REPORTED_TAG][DATA_VALUE_COUNT_TAG]);

    // last_reported_nackfrag_count
    check_is_id(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_NACKFRAG_COUNT_LAST_REPORTED_TAG]);
    check_is_string_uint(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_NACKFRAG_COUNT_LAST_REPORTED_TAG][DATA_VALUE_SRC_TIME_TAG]);
    check_is_uint(dump, dump[DATAREADER_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_NACKFRAG_COUNT_LAST_REPORTED_TAG][DATA_VALUE_COUNT_TAG]);

    // --------------------- Locators ---------------------

    // network_latency_per_locator
    check_is_id(dump, dump[LOCATOR_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_NETWORK_LATENCY_TAG]);
    check_is_id(dump, dump[LOCATOR_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_NETWORK_LATENCY_TAG].begin().value());
    check_is_id(dump, dump[LOCATOR_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_NETWORK_LATENCY_TAG].begin().value().begin().value());
    check_is_string_uint(dump, dump[LOCATOR_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_NETWORK_LATENCY_TAG].begin().value().begin().value()[DATA_VALUE_SRC_TIME_TAG]);
    check_is_double(dump, dump[LOCATOR_CONTAINER_TAG].begin().value()[DATA_CONTAINER_TAG]
            [DATA_KIND_NETWORK_LATENCY_TAG].begin().value().begin().value()[DATA_VALUE_DATA_TAG]);

}

void check_reference(
        DatabaseDump const&  dump,
        const char* entity_container_tag,
        const char* reference_container_tag,
        const char* entity_tag)
{
    // Entity -> Reference
    {
        // Entity references a Reference that does not exist
        {
            DatabaseDump dump_copy = dump;

            dump_copy[entity_container_tag].begin().value()[reference_container_tag] = DatabaseDump::array({"1111"});

            Database db;
            ASSERT_THROW(db.load_database(dump_copy), CorruptedFile);
        }

        // Entity references Reference which does not reference Entity
        {
            DatabaseDump dump_copy = dump;

            dump_copy[reference_container_tag].begin().value()[entity_tag] = "1111";

            Database db;
            ASSERT_THROW(db.load_database(dump_copy), CorruptedFile);
        }
    }

    // Reference -> Entity
    {
        DatabaseDump dump_copy = dump;

        // Reference references Entity which does not reference Reference
        dump_copy[entity_container_tag].begin().value()[reference_container_tag] = DatabaseDump::array();
        {
            Database db;
            ASSERT_THROW(db.load_database(dump_copy), CorruptedFile);
        }

        // Reference references a Entity that does not exist
        dump_copy[reference_container_tag].begin().value()[entity_tag] = "1111";
        {
            Database db;
            ASSERT_THROW(db.load_database(dump_copy), CorruptedFile);
        }
    }
}

void check_multiple_reference(
        DatabaseDump const&  dump,
        const char* entity_container_tag,
        const char* reference_container_tag)
{
    // Entity -> Reference
    {
        // Entity references a Reference that does not exist
        {
            DatabaseDump dump_copy = dump;

            dump_copy[entity_container_tag].begin().value()[reference_container_tag] = DatabaseDump::array({"1111"});

            Database db;
            ASSERT_THROW(db.load_database(dump_copy), CorruptedFile);
        }

        // Entity references Reference which does not reference Entity
        {
            DatabaseDump dump_copy = dump;

            dump_copy[reference_container_tag].begin().value()[entity_container_tag] = DatabaseDump::array({"1111"});

            Database db;
            ASSERT_THROW(db.load_database(dump_copy), CorruptedFile);
        }
    }

    // Reference -> Entity
    {
        DatabaseDump dump_copy = dump;

        // Reference references Entity which does not reference Reference
        dump_copy[entity_container_tag].begin().value()[reference_container_tag] = DatabaseDump::array();
        {
            Database db;
            ASSERT_THROW(db.load_database(dump_copy), CorruptedFile);
        }

        // Reference references a Entity that does not exist
        dump_copy[reference_container_tag].begin().value()[entity_container_tag] = DatabaseDump::array({"1111"});
        {
            Database db;
            ASSERT_THROW(db.load_database(dump_copy), CorruptedFile);
        }
    }
}

// Test the load of a database with wrong references
TEST(database_load_tests, load_wrong_references)
{
    // Read JSON
    DatabaseDump dump;
    load_file(SIMPLE_DUMP_FILE, dump);

    // HOST <---> USER
    check_reference(dump, HOST_CONTAINER_TAG, USER_CONTAINER_TAG, HOST_ENTITY_TAG);
    // USER <---> PROCESS
    check_reference(dump, USER_CONTAINER_TAG, PROCESS_CONTAINER_TAG, USER_ENTITY_TAG);
    // PROCESS <---> PARTICIPANT
    check_reference(dump, PROCESS_CONTAINER_TAG, PARTICIPANT_CONTAINER_TAG, PROCESS_ENTITY_TAG);
    // DOMAIN <---> PARTICIPANT
    check_reference(dump, DOMAIN_CONTAINER_TAG, PARTICIPANT_CONTAINER_TAG, DOMAIN_ENTITY_TAG);
    // DOMAIN <---> TOPIC
    check_reference(dump, DOMAIN_CONTAINER_TAG, TOPIC_CONTAINER_TAG, DOMAIN_ENTITY_TAG);
    // TOPIC <---> DATAWRITER
    check_reference(dump, TOPIC_CONTAINER_TAG, DATAWRITER_CONTAINER_TAG, TOPIC_ENTITY_TAG);
    // TOPIC <---> DATAREADDER
    check_reference(dump, TOPIC_CONTAINER_TAG, DATAREADER_CONTAINER_TAG, TOPIC_ENTITY_TAG);
    // PARTICIPANT <---> DATAWRITER
    check_reference(dump, PARTICIPANT_CONTAINER_TAG, DATAWRITER_CONTAINER_TAG, PARTICIPANT_ENTITY_TAG);
    // PARTICIPANT <---> DATAREADDER
    check_reference(dump, PARTICIPANT_CONTAINER_TAG, DATAREADER_CONTAINER_TAG, PARTICIPANT_ENTITY_TAG);
    // LOCATOR <---> DATAWRITER
    check_multiple_reference(dump, LOCATOR_CONTAINER_TAG, DATAWRITER_CONTAINER_TAG);
    // LOCATOR <---> DATAREADDER
    check_multiple_reference(dump, LOCATOR_CONTAINER_TAG, DATAREADER_CONTAINER_TAG);
    // DATAWRITER <---> LOCATOR
    check_multiple_reference(dump, DATAWRITER_CONTAINER_TAG, LOCATOR_CONTAINER_TAG);
    // DATAREADDER <---> LOCATOR
    check_multiple_reference(dump, DATAREADER_CONTAINER_TAG, LOCATOR_CONTAINER_TAG);
}

// Test the database method string_to_int()
TEST(database_load_tests, string_to_int)
{
    DataBaseTest db;
    ASSERT_THROW(db.get_string_to_int("abc"), CorruptedFile);
    ASSERT_THROW(db.get_string_to_int("43.21"), CorruptedFile);
    ASSERT_THROW(db.get_string_to_int("-43.21"), CorruptedFile);
    ASSERT_NO_THROW(db.get_string_to_int("1234"));
    ASSERT_NO_THROW(db.get_string_to_int("-1234"));
}

// Test the database method string_to_uint()
TEST(database_load_tests, string_to_uint)
{
    DataBaseTest db;
    ASSERT_THROW(db.get_string_to_uint("abc"), CorruptedFile);
    ASSERT_THROW(db.get_string_to_uint("43.21"), CorruptedFile);
    ASSERT_THROW(db.get_string_to_uint("-43.21"), CorruptedFile);
    ASSERT_NO_THROW(db.get_string_to_uint("1234"));
    ASSERT_THROW(db.get_string_to_uint("-1234"), CorruptedFile);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
