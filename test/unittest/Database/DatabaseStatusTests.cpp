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
    ASSERT_DEATH(db.change_entity_status(host->id, false), "");
    db.change_entity_status_test(host->id, false);

    ASSERT_FALSE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    db.change_entity_status_test(host->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    auto host1 = std::make_shared<Host>("host1");
    db.insert(host1);

    db.change_entity_status_test(host->id, false);

    ASSERT_FALSE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(host1->active);

    db.change_entity_status_test(host->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(host1->active);
}

TEST_F(database_status_tests, user)
{
    ASSERT_DEATH(db.change_entity_status(user->id, false), "");
    db.change_entity_status_test(user->id, false);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    db.change_entity_status_test(user->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    auto user1 = std::make_shared<User>("user1", host);
    db.insert(user1);

    db.change_entity_status_test(user->id, false);

    ASSERT_TRUE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(user1->active);

    db.change_entity_status_test(user->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(user1->active);
}

TEST_F(database_status_tests, process)
{
    ASSERT_DEATH(db.change_entity_status(process->id, false), "");
    db.change_entity_status_test(process->id, false);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_FALSE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    db.change_entity_status_test(process->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    auto process1 = std::make_shared<Process>("process1", "123", user);
    db.insert(process1);

    db.change_entity_status_test(process->id, false);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_FALSE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(process1->active);

    db.change_entity_status_test(process->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(process1->active);
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

    auto domain1 = std::make_shared<Domain>("domain1");
    db.insert(domain1);

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
    ASSERT_TRUE(domain1->active);

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
    ASSERT_TRUE(domain1->active);
}

TEST_F(database_status_tests, topic)
{
    ASSERT_DEATH(db.change_entity_status(topic->id, false), "");
    db.change_entity_status_test(topic->id, false);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    db.change_entity_status_test(topic->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    auto topic1 = std::make_shared<Topic>("topic1", "type", domain);
    db.insert(topic1);

    db.change_entity_status_test(topic->id, false);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(topic1->active);

    db.change_entity_status_test(topic->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(topic1->active);
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

    auto participant1 = std::make_shared<DomainParticipant>("participant1", "qos",
                    "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1", nullptr,
                    domain);
    db.insert(participant1);
    db.link_participant_with_process(participant1->id, process->id);

    db.change_entity_status(participant->id, false);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_FALSE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(participant1->active);

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
    ASSERT_TRUE(participant1->active);
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

    auto datawriter1 = std::make_shared<DataWriter>("datawriter1", "qos", "21.22.23.24", participant, topic);
    auto writer_locator1 = std::make_shared<Locator>("writer_locator1");
    writer_locator1->id = db.generate_entity_id();
    datawriter1->locators[writer_locator1->id] = writer_locator1;
    db.insert(datawriter1);

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
    ASSERT_TRUE(datawriter1->active);
    ASSERT_TRUE(writer_locator1->active);

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
    ASSERT_TRUE(datawriter1->active);
    ASSERT_TRUE(writer_locator1->active);
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

    auto datareader1 = std::make_shared<DataReader>("datareader1", "qos", "11.12.13.14", participant, topic);
    auto reader_locator1 = std::make_shared<Locator>("reader_locator1");
    reader_locator1->id = db.generate_entity_id();
    datareader1->locators[reader_locator1->id] = reader_locator1;
    db.insert(datareader1);

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
    ASSERT_TRUE(datareader1->active);
    ASSERT_TRUE(reader_locator1->active);

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
    ASSERT_TRUE(datareader1->active);
    ASSERT_TRUE(reader_locator1->active);
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

    auto datawriter1 = std::make_shared<DataWriter>("datawriter1", "qos", "21.22.23.24", participant, topic);
    auto writer_locator1 = std::make_shared<Locator>("writer_locator1");
    writer_locator1->id = db.generate_entity_id();
    datawriter1->locators[writer_locator1->id] = writer_locator1;
    db.insert(datawriter1);

    auto datareader1 = std::make_shared<DataReader>("datareader1", "qos", "11.12.13.14", participant, topic);
    auto reader_locator1 = std::make_shared<Locator>("reader_locator1");
    reader_locator1->id = db.generate_entity_id();
    datareader1->locators[reader_locator1->id] = reader_locator1;
    db.insert(datareader1);

    db.change_entity_status(datawriter->id, false);
    db.change_entity_status(datareader->id, false);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(datareader1->active);
    ASSERT_TRUE(reader_locator1->active);
    ASSERT_TRUE(datawriter1->active);
    ASSERT_TRUE(writer_locator1->active);

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
    ASSERT_TRUE(datareader1->active);
    ASSERT_TRUE(reader_locator1->active);
    ASSERT_TRUE(datawriter1->active);
    ASSERT_TRUE(writer_locator1->active);
}

TEST_F(database_status_tests, dds_entities)
{
    db.change_entity_status(participant->id, false);
    db.change_entity_status(datareader->id, false);
    db.change_entity_status(datawriter->id, false);

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
    db.change_entity_status(datareader->id, true);
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

    auto participant1 = std::make_shared<DomainParticipant>("participant1", "qos",
                    "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1", nullptr,
                    domain);
    db.insert(participant1);
    db.link_participant_with_process(participant1->id, process->id);

    auto datawriter1 = std::make_shared<DataWriter>("datawriter1", "qos", "21.22.23.24", participant, topic);
    auto writer_locator1 = std::make_shared<Locator>("writer_locator1");
    writer_locator1->id = db.generate_entity_id();
    datawriter1->locators[writer_locator1->id] = writer_locator1;
    db.insert(datawriter1);

    auto datareader1 = std::make_shared<DataReader>("datareader1", "qos", "11.12.13.14", participant, topic);
    auto reader_locator1 = std::make_shared<Locator>("reader_locator1");
    reader_locator1->id = db.generate_entity_id();
    datareader1->locators[reader_locator1->id] = reader_locator1;
    db.insert(datareader1);

    db.change_entity_status(participant->id, false);
    db.change_entity_status(datareader->id, false);
    db.change_entity_status(datawriter->id, false);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_FALSE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(participant1->active);
    ASSERT_TRUE(datareader1->active);
    ASSERT_TRUE(reader_locator1->active);
    ASSERT_TRUE(datawriter1->active);
    ASSERT_TRUE(writer_locator1->active);

    db.change_entity_status(participant->id, false);
    db.change_entity_status(datareader->id, true);
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
    ASSERT_TRUE(participant1->active);
    ASSERT_TRUE(datareader1->active);
    ASSERT_TRUE(reader_locator1->active);
    ASSERT_TRUE(datawriter1->active);
    ASSERT_TRUE(writer_locator1->active);
}

TEST_F(database_status_tests, locator)
{
    ASSERT_DEATH(db.change_entity_status(locator->id, false), "");
    db.change_entity_status_test(locator->id, false);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    db.change_entity_status_test(locator->id, true);

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
