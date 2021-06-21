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

#include <gtest/gtest.h>

#include <StatisticsBackend.hpp>
#include <StatisticsBackendData.hpp>


using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

void load_file(
        std::string filename,
        DatabaseDump& dump)
{
    constexpr const char* DESCRIPTION_TAG = "description";

    // Check if the file exists
    std::ifstream file(filename);
    if (!file.good())
    {
        FAIL() << "File " + filename + " does not exist";
    }

    // Get the json
    file >> dump;

    // Erase the description tag if existing
    if (dump.contains(DESCRIPTION_TAG))
    {
        dump.erase(DESCRIPTION_TAG);
    }
}

TEST(backend_dump_tests, reset)
{
    constexpr const char* DUMP_FILE = "../Resources/simple_dump.json";

    // Load the dump to the database
    StatisticsBackend::load_database(DUMP_FILE);
    ASSERT_FALSE(StatisticsBackend::get_entities(EntityKind::DOMAIN).empty());

    // Resetting at this point should work and clear the database
    StatisticsBackend::reset();
    ASSERT_TRUE(StatisticsBackend::get_entities(EntityKind::DOMAIN).empty());

    // Load the dump to the database again
    StatisticsBackend::load_database(DUMP_FILE);
    ASSERT_FALSE(StatisticsBackend::get_entities(EntityKind::DOMAIN).empty());

    // Start a monitor and try resetting
    auto monitor = StatisticsBackend::init_monitor(100);
    EXPECT_THROW(StatisticsBackend::reset(), PreconditionNotMet);
    ASSERT_FALSE(StatisticsBackend::get_entities(EntityKind::DOMAIN).empty());

    // Stop the monitor and try again
    StatisticsBackend::stop_monitor(monitor);
    EXPECT_NO_THROW(StatisticsBackend::reset());
    ASSERT_TRUE(StatisticsBackend::get_entities(EntityKind::DOMAIN).empty());
}

TEST(backend_dump_tests, database_dump_load)
{
    constexpr const char* DUMP_FILE = "../Resources/simple_dump.json";
    constexpr const char* TEST_DUMP_FILE = "test_dump.json";
    constexpr const char* NON_EXISTENT_FILE = "/this_directory_does_not_exist/test_dump.json";

    // The dump as loaded by the JSON library
    DatabaseDump expected;
    load_file(DUMP_FILE, expected);

    // Load the dump to the database and check
    // The correct behavior of Database::dump_database() is already checked on its unit tests
    // and we can consider it works properly
    StatisticsBackend::load_database(DUMP_FILE);
    DatabaseDump dump = details::StatisticsBackendData::get_instance()->database_->dump_database();
    EXPECT_EQ(expected, dump);

    // Trying to load the dump again must fail
    EXPECT_THROW(StatisticsBackend::load_database(DUMP_FILE),
            PreconditionNotMet);

    // Reset the backend and load the dump again
    StatisticsBackend::reset();
    StatisticsBackend::load_database(DUMP_FILE);
    dump = details::StatisticsBackendData::get_instance()->database_->dump_database();
    EXPECT_EQ(expected, dump);

    // Check if the output file exists (we do not want to overwrite anything)
    std::ifstream file(TEST_DUMP_FILE);
    if (file.good())
    {
        FAIL() << "File " << TEST_DUMP_FILE << " exists in the testing directory";
        return;
    }

    // Dump the load
    StatisticsBackend::dump_database(TEST_DUMP_FILE);

    // Load it by the JSON library and check equality
    load_file(TEST_DUMP_FILE, dump);
    EXPECT_EQ(expected, dump);

    // Remove the dump file
    std::remove(TEST_DUMP_FILE);

    // Try dumping on a non-existent directory
    ASSERT_THROW(StatisticsBackend::dump_database(NON_EXISTENT_FILE),
            BadParameter);

    // Reset the backend and try loading a non existent file
    StatisticsBackend::reset();
    ASSERT_THROW(StatisticsBackend::load_database(NON_EXISTENT_FILE),
            BadParameter);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
