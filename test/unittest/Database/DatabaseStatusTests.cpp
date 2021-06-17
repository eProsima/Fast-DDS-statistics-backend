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

#include <gtest/gtest.h>

#include <database/database.hpp>
#include <DatabaseUtils.hpp>

/**
 * @brief Fixture for the database_status_tests
 */
class database_status_tests : public ::testing::Test
{
public:

    void SetUp()
    {
        db.load_database(load_file(EMPTY_ENTITIES_DUMP_FILE));
        host = db.hosts().begin()->second;
        user = db.users().begin()->second;
        process = db.processes().begin()->second;
        domain = db.domains().begin()->second;
        topic = db.topics().begin()->second.begin()->second;
        participant = db.participants().begin()->second.begin()->second;
        datawriter = db.get_dds_endpoints<DataWriter>().begin()->second.begin()->second;
        datareader = db.get_dds_endpoints<DataReader>().begin()->second.begin()->second;
        locator = db.locators().begin()->second;
    }

    std::shared_ptr<Host> host;
    std::shared_ptr<User> user;
    std::shared_ptr<Process> process;
    std::shared_ptr<Domain> domain;
    std::shared_ptr<Topic> topic;
    std::shared_ptr<DomainParticipant> participant;
    std::shared_ptr<DataWriter> datawriter;
    std::shared_ptr<DataReader> datareader;
    std::shared_ptr<Locator> locator;

    DataBaseTest db;
};

TEST_F(database_status_tests, initial_status)
{
    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
}

TEST_F(database_status_tests, host)
{
    db.change_entity_status(host->id, false);

    ASSERT_FALSE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    db.change_entity_status(host->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
}

TEST_F(database_status_tests, user)
{
    db.change_entity_status(user->id, false);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    db.change_entity_status(user->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
}

TEST_F(database_status_tests, process)
{
    db.change_entity_status(process->id, false);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_FALSE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    db.change_entity_status(process->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
}

TEST_F(database_status_tests, domain)
{
    db.change_entity_status(domain->id, false);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_FALSE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    db.change_entity_status(domain->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
}

TEST_F(database_status_tests, topic)
{
    db.change_entity_status(topic->id, false);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    db.change_entity_status(topic->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
}

TEST_F(database_status_tests, participant)
{
    db.change_entity_status(participant->id, false);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_FALSE(process->active);
    ASSERT_FALSE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_FALSE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    db.change_entity_status(participant->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
}

TEST_F(database_status_tests, datawriter)
{
    db.change_entity_status(datawriter->id, false);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    db.change_entity_status(datawriter->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
}

TEST_F(database_status_tests, datareader)
{
    db.change_entity_status(datareader->id, false);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);

    db.change_entity_status(datareader->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
}

TEST_F(database_status_tests, endpoints)
{
    db.change_entity_status(datawriter->id, false);
    db.change_entity_status(datareader->id, false);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);

    db.change_entity_status(datawriter->id, true);
    db.change_entity_status(datareader->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
}

TEST_F(database_status_tests, dds_entities)
{
    db.change_entity_status(participant->id, false);
    db.change_entity_status(datawriter->id, false);
    db.change_entity_status(datareader->id, false);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_FALSE(process->active);
    ASSERT_FALSE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_FALSE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);

    db.change_entity_status(participant->id, true);
    db.change_entity_status(datawriter->id, true);
    db.change_entity_status(datareader->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
}

TEST_F(database_status_tests, locator)
{
    db.change_entity_status(locator->id, false);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    db.change_entity_status(locator->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
}

int main(

        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
