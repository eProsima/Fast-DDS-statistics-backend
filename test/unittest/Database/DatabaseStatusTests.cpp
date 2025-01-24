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
        DatabaseDump dump;
        load_file(EMPTY_ENTITIES_DUMP_FILE, dump);
        db.load_database(dump);
        host = db.hosts().begin()->second;
        user = db.users().begin()->second;
        process = db.processes().begin()->second;
        domain = db.domains().begin()->second;
        topic = db.topics().begin()->second.begin()->second;
        participant = db.participants().begin()->second.begin()->second;
        datawriter = db.get_dds_endpoints<DataWriter>().begin()->second.begin()->second;
        datareader = db.get_dds_endpoints<DataReader>().begin()->second.begin()->second;
        locator = db.locators().begin()->second;

        // Simulate that the backend is motorizing the domain
        // NOTE: This is dangerous, please do not do it again
        std::unique_ptr<details::Monitor> monitor = std::make_unique<details::Monitor>();
        monitor->id = domain->id;
        details::StatisticsBackendData::get_instance()->monitors_by_entity_[domain->id] = std::move(monitor);

        // Simulate the discover of the entitiy
        db.change_entity_status_test(host->id, true, domain->id);
        db.change_entity_status_test(user->id, true, domain->id);
        db.change_entity_status_test(process->id, true, domain->id);
        topic->active = false;
        db.change_entity_status_test(topic->id, true, domain->id);

        // Simulate the discover of the entities
        details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
            host->id,
            host->kind, details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
        details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
            user->id,
            user->kind, details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
        details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
            process->id,
            process->kind, details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
        details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(domain->id,
                topic->id,
                topic->kind,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
        details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(domain->id,
                participant->id,
                participant->kind,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
        details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(domain->id,
                datawriter->id,
                datawriter->kind,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
        details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(domain->id,
                datareader->id,
                datareader->kind,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
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
#ifndef NDEBUG
    ASSERT_DEATH(db.change_entity_status(host->id, false), "");
#endif // ifndef NDEBUG

    // Deactivate entity will only deactivate itself
    db.change_entity_status_test(host->id, false, domain->id);

    ASSERT_FALSE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    // Activate entity will only activate itself
    db.change_entity_status_test(host->id, true, domain->id);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    // The new entity will be inactive
    auto host1 = std::make_shared<Host>("host1");
    db.insert(host1);
    ASSERT_FALSE(host1->active);

    // Simulate the discover of the entity
    db.change_entity_status_test(host1->id, true, domain->id);
    ASSERT_TRUE(host1->active);

    // Deactivate entity will only deactivate itself
    db.change_entity_status_test(host->id, false, domain->id);

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

    // Activate entity will only activate itself
    db.change_entity_status_test(host->id, true, domain->id);

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

    // Deactivate both entity will only deactivate itselfs
    db.change_entity_status_test(host->id, false, domain->id);
    db.change_entity_status_test(host1->id, false, domain->id);

    ASSERT_FALSE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_FALSE(host1->active);

    // Activate one entity will only activate itself
    db.change_entity_status_test(host->id, true, domain->id);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_FALSE(host1->active);

    // Activate other entity will only activate itself
    db.change_entity_status_test(host1->id, true, domain->id);

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
#ifndef NDEBUG
    ASSERT_DEATH(db.change_entity_status(user->id, false), "");
#endif // ifndef NDEBUG

    // Deactivate entity will only deactivate itself
    db.change_entity_status_test(user->id, false, domain->id);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    // Activate entity will only activate itself
    db.change_entity_status_test(user->id, true, domain->id);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    // The new entity will be inactive
    auto user1 = std::make_shared<User>("user1", host);
    db.insert(user1);
    ASSERT_FALSE(user1->active);

    // Simulate the discover of the entity
    db.change_entity_status_test(user1->id, true, domain->id);
    ASSERT_TRUE(user1->active);

    // Deactivate entity will only deactivate itself
    db.change_entity_status_test(user->id, false, domain->id);

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

    // Activate entity will only activate itself
    db.change_entity_status_test(user->id, true, domain->id);

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

    // Deactivate both entities will deactivate subentities
    db.change_entity_status_test(user->id, false, domain->id);
    db.change_entity_status_test(user1->id, false, domain->id);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_FALSE(user1->active);

    // Activate one entity will activate subentities
    db.change_entity_status_test(user->id, true, domain->id);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_FALSE(user1->active);

    // Activate other entity will only activate itself
    db.change_entity_status_test(user1->id, true, domain->id);

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
#ifndef NDEBUG
    ASSERT_DEATH(db.change_entity_status(process->id, false), "");
#endif // ifndef NDEBUG

    // Deactivate entity will only deactivate itself
    db.change_entity_status_test(process->id, false, domain->id);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_FALSE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    // Activate entity will only activate itself
    db.change_entity_status_test(process->id, true, domain->id);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    // The new entity will be inactive
    auto process1 = std::make_shared<Process>("process1", "123", user);
    db.insert(process1);
    ASSERT_FALSE(process1->active);

    // Simulate the discover of the entity
    db.change_entity_status_test(process1->id, true, domain->id);
    ASSERT_TRUE(process1->active);

    // Deactivate entity will only deactivate itself
    db.change_entity_status_test(process->id, false, domain->id);

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

    // Activate entity will only activate itself
    db.change_entity_status_test(process->id, true, domain->id);

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

    // Deactivate both entities will deactivate subentities
    db.change_entity_status_test(process->id, false, domain->id);
    db.change_entity_status_test(process1->id, false, domain->id);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_FALSE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_FALSE(process1->active);

    // Activate one entity will activate subentities
    db.change_entity_status_test(process->id, true, domain->id);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_FALSE(process1->active);

    // Activate other entity will only activate itself
    db.change_entity_status_test(process1->id, true, domain->id);

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
    // Deactivate entity will only deactivate itself
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

    // Activate entity will only activate itself
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

    // The new entity will be active
    auto domain1 = std::make_shared<Domain>("domain1");
    db.insert(domain1);
    ASSERT_TRUE(domain1->active);

    // Deactivate entity will only deactivate itself
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

    // Activate entity will only activate itself
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

    // Deactivate both entity will only deactivate itselfs
    db.change_entity_status_test(domain->id, false, domain->id);
    db.change_entity_status_test(domain1->id, false, domain->id);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_FALSE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_FALSE(domain1->active);

    // Activate one entity will only activate itself
    db.change_entity_status_test(domain->id, true, domain->id);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_FALSE(domain1->active);

    // Activate other entity will only activate itself
    db.change_entity_status_test(domain1->id, true, domain->id);

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
#ifndef NDEBUG
    ASSERT_DEATH(db.change_entity_status(topic->id, false), "");
#endif // ifndef NDEBUG

    // Deactivate entity will only deactivate itself
    db.change_entity_status_test(topic->id, false, domain->id);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    // Activate entity will only activate itself
    db.change_entity_status_test(topic->id, true, domain->id);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    // The new entity will be active
    auto topic1 = std::make_shared<Topic>("topic1", "type", domain);
    db.insert(topic1);
    ASSERT_TRUE(topic1->active);

    // Deactivate entity will only deactivate itself
    db.change_entity_status_test(topic->id, false, domain->id);

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

    // Activate entity will only activate itself
    db.change_entity_status_test(topic->id, true, domain->id);

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

    // Deactivate both entity will only deactivate itselfs
    db.change_entity_status_test(topic->id, false, domain->id);
    db.change_entity_status_test(topic1->id, false, domain->id);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_FALSE(topic1->active);

    // Activate one entity will only activate itself
    db.change_entity_status_test(topic->id, true, domain->id);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_FALSE(topic1->active);

    // Activate other entity will only activate itself
    db.change_entity_status_test(topic1->id, true, domain->id);

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
    // Deactivate entity will deactivate subentities
    db.change_entity_status(participant->id, false);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_FALSE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_FALSE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);

    // Activate entity will activate subentities
    db.change_entity_status(participant->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);

    // The new entity will be active
    auto participant1 = std::make_shared<DomainParticipant>("participant1", "qos",
                    "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1", nullptr,
                    domain);
    db.insert(participant1);
    ASSERT_TRUE(participant1->active);

    // Deactivate new entity will only deactivate itself
    db.change_entity_status(participant1->id, false);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_FALSE(participant1->active);

    // Activate new entity will activate itself
    db.change_entity_status(participant1->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(participant1->active);

    // Deactivate entity will deactivate subentities
    // This is because the new entity is not already linked with subentities
    db.change_entity_status(participant->id, false);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_FALSE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_FALSE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(participant1->active);

    // Activate entity will activate subentities
    db.change_entity_status(participant->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(participant1->active);

    db.link_participant_with_process(participant1->id, process->id);

    // Link of the new entity done
    // Deactivate entity will not deactivate subentities
    db.change_entity_status(participant->id, false);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_FALSE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(participant1->active);

    // Activate entity will only activate itself
    db.change_entity_status(participant->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(participant1->active);

    // Deactivate both entities, will deactivate subentities
    db.change_entity_status(participant->id, false);
    db.change_entity_status(participant1->id, false);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_FALSE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_FALSE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_FALSE(participant1->active);

    // Activate one entity, will activate subentities
    db.change_entity_status(participant->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_FALSE(participant1->active);

    // Activate other entity, will only activate itself
    db.change_entity_status(participant1->id, true);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(participant1->active);
}

TEST_F(database_status_tests, link_active_participant_with_inactive_process)
{
    // Deactivate entity will deactivate subentities
    db.change_entity_status(participant->id, false);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_FALSE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_FALSE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);

    // The new entity will be active
    auto participant1 = std::make_shared<DomainParticipant>("participant1", "qos",
                    "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1", nullptr,
                    domain);
    db.insert(participant1);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_FALSE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_FALSE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(participant1->active);

    // Link participant will activate process and it subentities
    db.link_participant_with_process(participant1->id, process->id);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_FALSE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_TRUE(participant1->active);
}

TEST_F(database_status_tests, link_inactive_participant_with_active_process)
{
    // The new entity will be inactive
    auto participant1 = std::make_shared<DomainParticipant>("participant1", "qos",
                    "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1", nullptr,
                    domain);
    db.insert(participant1);
    db.change_entity_status(participant1->id, false);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_FALSE(participant1->active);

    // Link participant will not modify status
    db.link_participant_with_process(participant1->id, process->id);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_FALSE(participant1->active);
}

TEST_F(database_status_tests, link_inactive_participant_with_inactive_process)
{
    // Deactivate entity will deactivate subentities
    db.change_entity_status(participant->id, false);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_FALSE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_FALSE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);

    // The new entity will be inactive
    auto participant1 = std::make_shared<DomainParticipant>("participant1", "qos",
                    "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1", nullptr,
                    domain);
    db.insert(participant1);
    db.change_entity_status(participant1->id, false);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_FALSE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_FALSE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_FALSE(participant1->active);

    // Link participant will not modify status
    db.link_participant_with_process(participant1->id, process->id);

    ASSERT_FALSE(host->active);
    ASSERT_FALSE(user->active);
    ASSERT_FALSE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_FALSE(topic->active);
    ASSERT_FALSE(participant->active);
    ASSERT_FALSE(datawriter->active);
    ASSERT_FALSE(datareader->active);
    ASSERT_TRUE(locator->active);
    ASSERT_FALSE(participant1->active);
}

TEST_F(database_status_tests, endpoints)
{
    // 1. datawriter->active = false
    // 2. datareader->active = false
    // 3. datawriter->active = true
    // 4. datareader->active = true
    {
        // Deactivate one endpoint will only deactivate itself
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

        // Deactivate both endpoints will deactivate topic
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

        // Activate one endpoint will activate topic
        db.change_entity_status(datawriter->id, true);

        ASSERT_TRUE(host->active);
        ASSERT_TRUE(user->active);
        ASSERT_TRUE(process->active);
        ASSERT_TRUE(domain->active);
        ASSERT_TRUE(topic->active);
        ASSERT_TRUE(participant->active);
        ASSERT_TRUE(datawriter->active);
        ASSERT_FALSE(datareader->active);
        ASSERT_TRUE(locator->active);

        // Activate other endpoint will onlny activate itself
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

    // Same that above but on inversal order

    // 1. datareader->active = false
    // 2. datawriter->active = false
    // 3. datareader->active = true
    // 4. datawriter->active = true
    {
        // Deactivate one endpoint will only deactivate itself
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

        // Deactivate both endpoints will deactivate topic
        db.change_entity_status(datawriter->id, false);

        ASSERT_TRUE(host->active);
        ASSERT_TRUE(user->active);
        ASSERT_TRUE(process->active);
        ASSERT_TRUE(domain->active);
        ASSERT_FALSE(topic->active);
        ASSERT_TRUE(participant->active);
        ASSERT_FALSE(datawriter->active);
        ASSERT_FALSE(datareader->active);
        ASSERT_TRUE(locator->active);

        // Activate one endpoint will activate topic
        db.change_entity_status(datareader->id, true);

        ASSERT_TRUE(host->active);
        ASSERT_TRUE(user->active);
        ASSERT_TRUE(process->active);
        ASSERT_TRUE(domain->active);
        ASSERT_TRUE(topic->active);
        ASSERT_TRUE(participant->active);
        ASSERT_FALSE(datawriter->active);
        ASSERT_TRUE(datareader->active);
        ASSERT_TRUE(locator->active);

        // Activate other endpoint will onlny activate itself
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

    // The new entities will be active
    auto datawriter1 = std::make_shared<DataWriter>("datawriter1", "qos", "21.22.23.24", participant, topic);
    auto writer_locator1 = std::make_shared<Locator>("writer_locator1");
    writer_locator1->id = db.insert(writer_locator1);
    datawriter1->locators[writer_locator1->id] = writer_locator1;
    db.insert(datawriter1);
    ASSERT_TRUE(datawriter1->active);
    ASSERT_TRUE(writer_locator1->active);

    auto datareader1 = std::make_shared<DataReader>("datareader1", "qos", "11.12.13.14", participant, topic);
    auto reader_locator1 = std::make_shared<Locator>("reader_locator1");
    reader_locator1->id = db.insert(reader_locator1);
    datareader1->locators[reader_locator1->id] = reader_locator1;
    db.insert(datareader1);
    ASSERT_TRUE(datareader1->active);
    ASSERT_TRUE(reader_locator1->active);

    // 1. datawriter->active = false
    // 2. datawriter1->active = false
    // 3. datareader->active = false
    // 4. datareader1->active = false
    // 5. datawriter->active = true
    // 6. datawriter1->active = true
    // 7. datareader->active = true
    // 8. datareader1->active = true
    {
        // Deactivate one endpoint will only deactivate itself
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
        ASSERT_TRUE(datareader1->active);
        ASSERT_TRUE(reader_locator1->active);
        ASSERT_TRUE(datawriter1->active);
        ASSERT_TRUE(writer_locator1->active);

        // Deactivate one endpoint will only deactivate itself
        db.change_entity_status(datawriter1->id, false);

        ASSERT_TRUE(host->active);
        ASSERT_TRUE(user->active);
        ASSERT_TRUE(process->active);
        ASSERT_TRUE(domain->active);
        ASSERT_TRUE(topic->active);
        ASSERT_TRUE(participant->active);
        ASSERT_FALSE(datawriter->active);
        ASSERT_TRUE(datareader->active);
        ASSERT_TRUE(locator->active);
        ASSERT_TRUE(datareader1->active);
        ASSERT_TRUE(reader_locator1->active);
        ASSERT_FALSE(datawriter1->active);
        ASSERT_TRUE(writer_locator1->active);

        // Deactivate one endpoint will only deactivate itself
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
        ASSERT_FALSE(datawriter1->active);
        ASSERT_TRUE(writer_locator1->active);

        // Deactivate all endpoints will deactivate topic
        db.change_entity_status(datareader1->id, false);

        ASSERT_TRUE(host->active);
        ASSERT_TRUE(user->active);
        ASSERT_TRUE(process->active);
        ASSERT_TRUE(domain->active);
        ASSERT_FALSE(topic->active);
        ASSERT_TRUE(participant->active);
        ASSERT_FALSE(datawriter->active);
        ASSERT_FALSE(datareader->active);
        ASSERT_TRUE(locator->active);
        ASSERT_FALSE(datareader1->active);
        ASSERT_TRUE(reader_locator1->active);
        ASSERT_FALSE(datawriter1->active);
        ASSERT_TRUE(writer_locator1->active);

        // Activate one endpoint will activate topic
        db.change_entity_status(datawriter->id, true);

        ASSERT_TRUE(host->active);
        ASSERT_TRUE(user->active);
        ASSERT_TRUE(process->active);
        ASSERT_TRUE(domain->active);
        ASSERT_TRUE(topic->active);
        ASSERT_TRUE(participant->active);
        ASSERT_TRUE(datawriter->active);
        ASSERT_FALSE(datareader->active);
        ASSERT_TRUE(locator->active);
        ASSERT_FALSE(datareader1->active);
        ASSERT_TRUE(reader_locator1->active);
        ASSERT_FALSE(datawriter1->active);
        ASSERT_TRUE(writer_locator1->active);

        // Activate one endpoint will only activate itself
        db.change_entity_status(datawriter1->id, true);

        ASSERT_TRUE(host->active);
        ASSERT_TRUE(user->active);
        ASSERT_TRUE(process->active);
        ASSERT_TRUE(domain->active);
        ASSERT_TRUE(topic->active);
        ASSERT_TRUE(participant->active);
        ASSERT_TRUE(datawriter->active);
        ASSERT_FALSE(datareader->active);
        ASSERT_TRUE(locator->active);
        ASSERT_FALSE(datareader1->active);
        ASSERT_TRUE(reader_locator1->active);
        ASSERT_TRUE(datawriter1->active);
        ASSERT_TRUE(writer_locator1->active);

        // Activate one endpoint will only activate itself
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
        ASSERT_FALSE(datareader1->active);
        ASSERT_TRUE(reader_locator1->active);
        ASSERT_TRUE(datawriter1->active);
        ASSERT_TRUE(writer_locator1->active);

        // Activate one endpoint will only activate itself
        db.change_entity_status(datareader1->id, true);

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

    // Same that above but on inversal order

    // 1. datareader->active = false
    // 2. datareader1->active = false
    // 3. datawriter->active = false
    // 4. datawriter1->active = false
    // 5. datareader->active = true
    // 6. datareader1->active = true
    // 7. datawriter->active = true
    // 8. datawriter1->active = true
    {
        // Deactivate one endpoint will only deactivate itself
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
        ASSERT_TRUE(datawriter1->active);
        ASSERT_TRUE(writer_locator1->active);

        // Deactivate one endpoint will only deactivate itself
        db.change_entity_status(datareader1->id, false);

        ASSERT_TRUE(host->active);
        ASSERT_TRUE(user->active);
        ASSERT_TRUE(process->active);
        ASSERT_TRUE(domain->active);
        ASSERT_TRUE(topic->active);
        ASSERT_TRUE(participant->active);
        ASSERT_TRUE(datawriter->active);
        ASSERT_FALSE(datareader->active);
        ASSERT_TRUE(locator->active);
        ASSERT_FALSE(datareader1->active);
        ASSERT_TRUE(reader_locator1->active);
        ASSERT_TRUE(datawriter1->active);
        ASSERT_TRUE(writer_locator1->active);

        // Deactivate one endpoint will only deactivate itself
        db.change_entity_status(datawriter->id, false);

        ASSERT_TRUE(host->active);
        ASSERT_TRUE(user->active);
        ASSERT_TRUE(process->active);
        ASSERT_TRUE(domain->active);
        ASSERT_TRUE(topic->active);
        ASSERT_TRUE(participant->active);
        ASSERT_FALSE(datawriter->active);
        ASSERT_FALSE(datareader->active);
        ASSERT_TRUE(locator->active);
        ASSERT_FALSE(datareader1->active);
        ASSERT_TRUE(reader_locator1->active);
        ASSERT_TRUE(datawriter1->active);
        ASSERT_TRUE(writer_locator1->active);

        // Deactivate all endpoints will deactivate topic
        db.change_entity_status(datawriter1->id, false);

        ASSERT_TRUE(host->active);
        ASSERT_TRUE(user->active);
        ASSERT_TRUE(process->active);
        ASSERT_TRUE(domain->active);
        ASSERT_FALSE(topic->active);
        ASSERT_TRUE(participant->active);
        ASSERT_FALSE(datawriter->active);
        ASSERT_FALSE(datareader->active);
        ASSERT_TRUE(locator->active);
        ASSERT_FALSE(datareader1->active);
        ASSERT_TRUE(reader_locator1->active);
        ASSERT_FALSE(datawriter1->active);
        ASSERT_TRUE(writer_locator1->active);

        // Activate one endpoint will activate topic
        db.change_entity_status(datareader->id, true);

        ASSERT_TRUE(host->active);
        ASSERT_TRUE(user->active);
        ASSERT_TRUE(process->active);
        ASSERT_TRUE(domain->active);
        ASSERT_TRUE(topic->active);
        ASSERT_TRUE(participant->active);
        ASSERT_FALSE(datawriter->active);
        ASSERT_TRUE(datareader->active);
        ASSERT_TRUE(locator->active);
        ASSERT_FALSE(datareader1->active);
        ASSERT_TRUE(reader_locator1->active);
        ASSERT_FALSE(datawriter1->active);
        ASSERT_TRUE(writer_locator1->active);

        // Activate one endpoint will only activate itself
        db.change_entity_status(datareader1->id, true);

        ASSERT_TRUE(host->active);
        ASSERT_TRUE(user->active);
        ASSERT_TRUE(process->active);
        ASSERT_TRUE(domain->active);
        ASSERT_TRUE(topic->active);
        ASSERT_TRUE(participant->active);
        ASSERT_FALSE(datawriter->active);
        ASSERT_TRUE(datareader->active);
        ASSERT_TRUE(locator->active);
        ASSERT_TRUE(datareader1->active);
        ASSERT_TRUE(reader_locator1->active);
        ASSERT_FALSE(datawriter1->active);
        ASSERT_TRUE(writer_locator1->active);

        // Activate one endpoint will only activate itself
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
        ASSERT_TRUE(datareader1->active);
        ASSERT_TRUE(reader_locator1->active);
        ASSERT_FALSE(datawriter1->active);
        ASSERT_TRUE(writer_locator1->active);

        // Activate one endpoint will only activate itself
        db.change_entity_status(datawriter1->id, true);

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
}

TEST_F(database_status_tests, locator)
{
#ifndef NDEBUG
    ASSERT_DEATH(db.change_entity_status(locator->id, false), "");
#endif // ifndef NDEBUG

    db.change_entity_status_test(locator->id, false, domain->id);

    ASSERT_TRUE(host->active);
    ASSERT_TRUE(user->active);
    ASSERT_TRUE(process->active);
    ASSERT_TRUE(domain->active);
    ASSERT_TRUE(topic->active);
    ASSERT_TRUE(participant->active);
    ASSERT_TRUE(datawriter->active);
    ASSERT_TRUE(datareader->active);
    ASSERT_TRUE(locator->active);

    db.change_entity_status_test(locator->id, true, domain->id);

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
