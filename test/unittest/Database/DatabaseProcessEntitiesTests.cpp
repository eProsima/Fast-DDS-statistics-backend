// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <chrono>
#include <memory>
#include <string>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <fastdds/rtps/common/RemoteLocators.hpp>
#include <fastdds/rtps/common/Locator.hpp>

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/JSONTags.h>

#include <database/database.hpp>
#include <database/entities.hpp>

#include <DatabaseUtils.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::fastdds::rtps;

class database_process_entities_tests : public ::testing::Test
{
public:

    void SetUp()
    {
        host_name = "test_host";
        user_name = "test_user";
        process_name = "test_process";
        process_pid = process_name;
        domain_name = "test_domain";

        participant_name = "test_participant";
        participant_guid = "01.0f.03.04.05.06.07.08.09.10.11.12";

        topic_name = "test_topic";
        topic_alias = topic_name;
        topic_type = "test_topic_type";

        writer_name = "test_writer";
        writer_alias = writer_name;
        writer_guid = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.3";

        reader_name = "test_reader";
        reader_alias = reader_name;
        reader_guid = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.4";

        unicast_locator_str = "UDPv4:[127.0.0.1]:1024";
        multicast_locator_str = "UDPv4:[239.1.1.1]:1024";

        entity_qos["_"] = "";
    }

    DataBaseTest db;
    std::string host_name;
    std::string user_name;
    std::string process_name;
    std::string process_pid;
    std::string domain_name;
    std::string participant_name;
    std::string participant_guid;
    std::string topic_name;
    std::string topic_alias;
    std::string topic_type;
    std::string writer_locator_name;
    std::string writer_name;
    std::string writer_alias;
    std::string writer_guid;
    std::string reader_locator_name;
    std::string reader_name;
    std::string reader_alias;
    std::string reader_guid;
    std::string unicast_locator_str;
    std::string multicast_locator_str;
    Qos entity_qos;
};

TEST_F(database_process_entities_tests, insert_new_participant)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Create a DomainParticipant*/
    EntityId participant_id = db.insert_new_participant(
        participant_name,
        entity_qos,
        participant_guid,
        domain_id,
        StatusLevel::OK_STATUS,
        AppId::UNKNOWN,
        "");

    /* Check that the participant is inserted correctly */
    std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>> participants = db.participants();
    ASSERT_EQ(participants.size(), 1u);
    ASSERT_NE(participants[domain_id].find(participant_id), participants[domain_id].end());
    ASSERT_EQ(participant_name, participants[domain_id][participant_id]->name);
    ASSERT_EQ(EntityKind::PARTICIPANT, participants[domain_id][participant_id]->kind);
    ASSERT_EQ(participant_name, participants[domain_id][participant_id]->alias);
    ASSERT_EQ(StatusLevel::OK_STATUS, participants[domain_id][participant_id]->status);

    ASSERT_EQ(entity_qos, participants[domain_id][participant_id]->qos);
    ASSERT_EQ(participant_guid, participants[domain_id][participant_id]->guid);

    ASSERT_EQ(nullptr, participants[domain_id][participant_id]->process);
    ASSERT_EQ(AppId::UNKNOWN, participants[domain_id][participant_id]->app_id);
    ASSERT_EQ("", participants[domain_id][participant_id]->app_metadata);
    ASSERT_EQ(domain->participants[participant_id].get(), participants[domain_id][participant_id].get());
    ASSERT_EQ(DdsVendor::FASTDDS, participants[domain_id][participant_id]->dds_vendor);
}

TEST_F(database_process_entities_tests, insert_new_participant_already_exists)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Insert a DomainParticipant*/
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(
        participant_name,
        entity_qos,
        participant_guid,
        std::shared_ptr<Process>(),
        domain,
        StatusLevel::OK_STATUS,
        AppId::UNKNOWN,
        "");
    db.insert(participant);

    /* Check that the participant insertion throws*/
    EXPECT_THROW(db.insert_new_participant(
                participant_name,
                entity_qos,
                participant_guid,
                domain_id,
                StatusLevel::OK_STATUS,
                AppId::UNKNOWN,
                ""), BadParameter);
}

TEST_F(database_process_entities_tests, insert_new_participant_no_domain)
{
    /* Check that the participant insertion throws*/
    EXPECT_THROW(db.insert_new_participant(
                participant_name,
                entity_qos,
                participant_guid,
                EntityId(),
                StatusLevel::OK_STATUS,
                AppId::UNKNOWN,
                ""), BadParameter);
}

TEST_F(database_process_entities_tests, process_physical_entities)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Insert a DomainParticipant*/
    EntityId participant_id = db.insert_new_participant(
        participant_name,
        entity_qos,
        participant_guid,
        domain_id,
        StatusLevel::OK_STATUS,
        AppId::UNKNOWN,
        "");
    std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>> participants = db.participants();

    /* Insert a Host*/
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);
    EntityId host_id = db.insert(host);
    host->id = host_id;

    /* Insert a User*/
    std::shared_ptr<User> user = std::make_shared<User>(user_name, host);
    EntityId user_id = db.insert(user);
    user->id = user_id;

    /* Insert a Process*/
    std::shared_ptr<Process> process = std::make_shared<Process>(process_name, process_pid, user);
    EntityId process_id = db.insert(process);
    process->id = process_id;

    std::map<std::string, EntityId> physical_entities_ids;
    physical_entities_ids[HOST_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[USER_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId::invalid();

    bool should_link_process_participant = true;

    /* Process Physical Entities*/
    db.process_physical_entities(
        host_name,
        user_name,
        process_name,
        process_pid,
        should_link_process_participant,
        participant_id,
        physical_entities_ids);

    /* Check that the host was found correctly */
    ASSERT_EQ(host_id, physical_entities_ids[HOST_ENTITY_TAG]);

    /* Check that the user was found correctly */
    ASSERT_EQ(user_id, physical_entities_ids[USER_ENTITY_TAG]);

    /* Check that the user was found correctly */
    ASSERT_EQ(process_id, physical_entities_ids[PROCESS_ENTITY_TAG]);

    /* Check that the process has linked correctly */
    std::map<EntityId, std::shared_ptr<Process>> processes = db.processes();
    ASSERT_EQ(
        participants[domain_id][participant_id].get(),
        processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->participants[participant_id].get());

    ASSERT_EQ(
        processes[physical_entities_ids[PROCESS_ENTITY_TAG]].get(),
        participants[domain_id][participant_id]->process.get());
}

TEST_F(database_process_entities_tests, process_physical_entities_no_link)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Insert a DomainParticipant*/
    EntityId participant_id = db.insert_new_participant(
        participant_name,
        entity_qos,
        participant_guid,
        domain_id,
        StatusLevel::OK_STATUS,
        AppId::UNKNOWN,
        "");
    std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>> participants = db.participants();

    /* Insert a Host*/
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);
    EntityId host_id = db.insert(host);
    host->id = host_id;

    /* Insert a User*/
    std::shared_ptr<User> user = std::make_shared<User>(user_name, host);
    EntityId user_id = db.insert(user);
    user->id = user_id;

    /* Insert a Process*/
    std::shared_ptr<Process> process = std::make_shared<Process>(process_name, process_pid, user);
    EntityId process_id = db.insert(process);
    process->id = process_id;

    std::map<std::string, EntityId> physical_entities_ids;
    physical_entities_ids[HOST_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[USER_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId::invalid();

    //Force not to link
    bool should_link_process_participant = false;

    /* Process Physical Entities*/
    db.process_physical_entities(
        host_name,
        user_name,
        process_name,
        process_pid,
        should_link_process_participant,
        participant_id,
        physical_entities_ids);

    /* Check that the host was found correctly */
    ASSERT_EQ(host_id, physical_entities_ids[HOST_ENTITY_TAG]);

    /* Check that the user was found correctly */
    ASSERT_EQ(user_id, physical_entities_ids[USER_ENTITY_TAG]);

    /* Check that the user was found correctly */
    ASSERT_EQ(process_id, physical_entities_ids[PROCESS_ENTITY_TAG]);

    /* Check that the process hasn't linked */
    std::map<EntityId, std::shared_ptr<Process>> processes = db.processes();
    ASSERT_EQ(nullptr, processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->participants[participant_id].get());

    ASSERT_EQ(nullptr, participants[domain_id][participant_id]->process.get());
}

TEST_F(database_process_entities_tests, process_physical_entities_no_process)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Insert a DomainParticipant*/
    EntityId participant_id = db.insert_new_participant(
        participant_name,
        entity_qos,
        participant_guid,
        domain_id,
        StatusLevel::OK_STATUS,
        AppId::UNKNOWN,
        "");
    std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>> participants = db.participants();

    /* Insert a Host*/
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);
    EntityId host_id = db.insert(host);
    host->id = host_id;

    /* Insert a User*/
    std::shared_ptr<User> user = std::make_shared<User>(user_name, host);
    EntityId user_id = db.insert(user);
    user->id = user_id;

    std::map<std::string, EntityId> physical_entities_ids;
    physical_entities_ids[HOST_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[USER_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId::invalid();

    // The new process should link automatically
    bool should_link_process_participant = false;

    /* Process Physical Entities*/
    db.process_physical_entities(
        host_name,
        user_name,
        process_name,
        process_pid,
        should_link_process_participant,
        participant_id,
        physical_entities_ids);

    /* Check that the host was found correctly */
    ASSERT_EQ(host_id, physical_entities_ids[HOST_ENTITY_TAG]);

    /* Check that the user was found correctly */
    ASSERT_EQ(user_id, physical_entities_ids[USER_ENTITY_TAG]);

    /* Check that the process is inserted correctly */
    std::map<EntityId, std::shared_ptr<User>> users = db.users();
    std::map<EntityId, std::shared_ptr<Process>> processes = db.processes();
    ASSERT_EQ(processes.size(), 1u);
    ASSERT_NE(processes.find(physical_entities_ids[PROCESS_ENTITY_TAG]), processes.end());
    ASSERT_EQ(process_name, processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->name);
    ASSERT_EQ(EntityKind::PROCESS, processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->kind);
    ASSERT_EQ(process_name, processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->alias);
    ASSERT_EQ(StatusLevel::OK_STATUS, processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->status);

    ASSERT_EQ(process_pid, processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->pid);
    ASSERT_EQ(
        users[physical_entities_ids[USER_ENTITY_TAG]].get(),
        processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->user.get());

    /* Check that the process has linked correctly */
    ASSERT_EQ(
        participants[domain_id][participant_id].get(),
        processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->participants[participant_id].get());

    ASSERT_EQ(
        processes[physical_entities_ids[PROCESS_ENTITY_TAG]].get(),
        participants[domain_id][participant_id]->process.get());
}

TEST_F(database_process_entities_tests, process_physical_entities_no_process_no_user)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Insert a DomainParticipant*/
    EntityId participant_id = db.insert_new_participant(
        participant_name,
        entity_qos,
        participant_guid,
        domain_id,
        StatusLevel::OK_STATUS,
        AppId::UNKNOWN,
        "");
    std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>> participants = db.participants();

    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);
    EntityId host_id = db.insert(host);
    host->id = host_id;

    std::map<std::string, EntityId> physical_entities_ids;
    physical_entities_ids[HOST_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[USER_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId::invalid();

    // The new process should link automatically
    bool should_link_process_participant = false;

    /* Process Physical Entities*/
    db.process_physical_entities(
        host_name,
        user_name,
        process_name,
        process_pid,
        should_link_process_participant,
        participant_id,
        physical_entities_ids);

    /* Check that the host was found correctly */
    ASSERT_EQ(host_id, physical_entities_ids[HOST_ENTITY_TAG]);

    /* Check that the user is inserted correctly */
    std::map<EntityId, std::shared_ptr<Host>> hosts = db.hosts();
    std::map<EntityId, std::shared_ptr<User>> users = db.users();
    ASSERT_EQ(users.size(), 1u);
    ASSERT_NE(users.find(physical_entities_ids[USER_ENTITY_TAG]), users.end());
    ASSERT_EQ(user_name, users[physical_entities_ids[USER_ENTITY_TAG]]->name);
    ASSERT_EQ(EntityKind::USER, users[physical_entities_ids[USER_ENTITY_TAG]]->kind);
    ASSERT_EQ(user_name, users[physical_entities_ids[USER_ENTITY_TAG]]->alias);
    ASSERT_EQ(StatusLevel::OK_STATUS, users[physical_entities_ids[USER_ENTITY_TAG]]->status);

    ASSERT_EQ(
        hosts[physical_entities_ids[HOST_ENTITY_TAG]].get(),
        users[physical_entities_ids[USER_ENTITY_TAG]]->host.get());

    /* Check that the process is inserted correctly */
    std::map<EntityId, std::shared_ptr<Process>> processes = db.processes();
    ASSERT_EQ(processes.size(), 1u);
    ASSERT_NE(processes.find(physical_entities_ids[PROCESS_ENTITY_TAG]), processes.end());
    ASSERT_EQ(process_name, processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->name);
    ASSERT_EQ(EntityKind::PROCESS, processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->kind);
    ASSERT_EQ(process_name, processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->alias);
    ASSERT_EQ(StatusLevel::OK_STATUS, processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->status);

    ASSERT_EQ(process_pid, processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->pid);
    ASSERT_EQ(
        users[physical_entities_ids[USER_ENTITY_TAG]].get(),
        processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->user.get());

    /* Check that the process has linked correctly */
    ASSERT_EQ(
        participants[domain_id][participant_id].get(),
        processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->participants[participant_id].get());

    ASSERT_EQ(
        processes[physical_entities_ids[PROCESS_ENTITY_TAG]].get(),
        participants[domain_id][participant_id]->process.get());
}

TEST_F(database_process_entities_tests, process_physical_entities_no_process_no_user_no_host)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Insert a DomainParticipant*/
    EntityId participant_id = db.insert_new_participant(
        participant_name,
        entity_qos,
        participant_guid,
        domain_id,
        StatusLevel::OK_STATUS,
        AppId::UNKNOWN,
        "");
    std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>> participants = db.participants();

    std::map<std::string, EntityId> physical_entities_ids;
    physical_entities_ids[HOST_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[USER_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId::invalid();

    // The new process should link automatically
    bool should_link_process_participant = false;

    /* Process Physical Entities*/
    db.process_physical_entities(
        host_name,
        user_name,
        process_name,
        process_pid,
        should_link_process_participant,
        participant_id,
        physical_entities_ids);

    /* Check that the host is inserted correctly */
    std::map<EntityId, std::shared_ptr<Host>> hosts = db.hosts();
    ASSERT_EQ(hosts.size(), 1u);
    ASSERT_NE(hosts.find(physical_entities_ids[HOST_ENTITY_TAG]), hosts.end());
    ASSERT_EQ(host_name, hosts[physical_entities_ids[HOST_ENTITY_TAG]]->name);
    ASSERT_EQ(EntityKind::HOST, hosts[physical_entities_ids[HOST_ENTITY_TAG]]->kind);
    ASSERT_EQ(host_name, hosts[physical_entities_ids[HOST_ENTITY_TAG]]->alias);
    ASSERT_EQ(StatusLevel::OK_STATUS, hosts[physical_entities_ids[HOST_ENTITY_TAG]]->status);

    /* Check that the user is inserted correctly */
    std::map<EntityId, std::shared_ptr<User>> users = db.users();
    ASSERT_EQ(users.size(), 1u);
    ASSERT_NE(users.find(physical_entities_ids[USER_ENTITY_TAG]), users.end());
    ASSERT_EQ(user_name, users[physical_entities_ids[USER_ENTITY_TAG]]->name);
    ASSERT_EQ(EntityKind::USER, users[physical_entities_ids[USER_ENTITY_TAG]]->kind);
    ASSERT_EQ(user_name, users[physical_entities_ids[USER_ENTITY_TAG]]->alias);
    ASSERT_EQ(StatusLevel::OK_STATUS, users[physical_entities_ids[USER_ENTITY_TAG]]->status);

    ASSERT_EQ(
        hosts[physical_entities_ids[HOST_ENTITY_TAG]].get(),
        users[physical_entities_ids[USER_ENTITY_TAG]]->host.get());

    /* Check that the process is inserted correctly */
    std::map<EntityId, std::shared_ptr<Process>> processes = db.processes();
    ASSERT_EQ(processes.size(), 1u);
    ASSERT_NE(processes.find(physical_entities_ids[PROCESS_ENTITY_TAG]), processes.end());
    ASSERT_EQ(process_name, processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->name);
    ASSERT_EQ(EntityKind::PROCESS, processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->kind);
    ASSERT_EQ(process_name, processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->alias);
    ASSERT_EQ(StatusLevel::OK_STATUS, processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->status);

    ASSERT_EQ(process_pid, processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->pid);
    ASSERT_EQ(
        users[physical_entities_ids[USER_ENTITY_TAG]].get(),
        processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->user.get());

    /* Check that the process has linked correctly */
    ASSERT_EQ(
        participants[domain_id][participant_id].get(),
        processes[physical_entities_ids[PROCESS_ENTITY_TAG]]->participants[participant_id].get());

    ASSERT_EQ(
        processes[physical_entities_ids[PROCESS_ENTITY_TAG]].get(),
        participants[domain_id][participant_id]->process.get());
}

TEST_F(database_process_entities_tests, process_physical_entities_process_throws)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Insert a DomainParticipant*/
    EntityId participant_id = db.insert_new_participant(
        participant_name,
        entity_qos,
        participant_guid,
        domain_id,
        StatusLevel::OK_STATUS,
        AppId::UNKNOWN,
        "");
    std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>> participants = db.participants();

    std::map<std::string, EntityId> physical_entities_ids;
    physical_entities_ids[HOST_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[USER_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId::invalid();

    bool should_link_process_participant = true;

    /* Process Physical Entities*/
    EXPECT_THROW(db.process_physical_entities(
                host_name,
                user_name,
                "",
                "",
                should_link_process_participant,
                participant_id,
                physical_entities_ids), BadParameter);

    /* Check that the host is inserted correctly */
    std::map<EntityId, std::shared_ptr<Host>> hosts = db.hosts();
    ASSERT_EQ(hosts.size(), 1u);
    ASSERT_NE(hosts.find(physical_entities_ids[HOST_ENTITY_TAG]), hosts.end());
    ASSERT_EQ(host_name, hosts[physical_entities_ids[HOST_ENTITY_TAG]]->name);
    ASSERT_EQ(EntityKind::HOST, hosts[physical_entities_ids[HOST_ENTITY_TAG]]->kind);
    ASSERT_EQ(host_name, hosts[physical_entities_ids[HOST_ENTITY_TAG]]->alias);
    ASSERT_EQ(StatusLevel::OK_STATUS, hosts[physical_entities_ids[HOST_ENTITY_TAG]]->status);

    /* Check that the user is inserted correctly */
    std::map<EntityId, std::shared_ptr<User>> users = db.users();
    ASSERT_EQ(users.size(), 1u);
    ASSERT_NE(users.find(physical_entities_ids[USER_ENTITY_TAG]), users.end());
    ASSERT_EQ(user_name, users[physical_entities_ids[USER_ENTITY_TAG]]->name);
    ASSERT_EQ(EntityKind::USER, users[physical_entities_ids[USER_ENTITY_TAG]]->kind);
    ASSERT_EQ(user_name, users[physical_entities_ids[USER_ENTITY_TAG]]->alias);
    ASSERT_EQ(StatusLevel::OK_STATUS, users[physical_entities_ids[USER_ENTITY_TAG]]->status);

    ASSERT_EQ(
        hosts[physical_entities_ids[HOST_ENTITY_TAG]].get(),
        users[physical_entities_ids[USER_ENTITY_TAG]]->host.get());

    /* Check that the process hasn't been inserted */
    std::map<EntityId, std::shared_ptr<Process>> processes = db.processes();
    ASSERT_EQ(processes.size(), 0u);
    ASSERT_EQ(physical_entities_ids[PROCESS_ENTITY_TAG], EntityId());

    /* Check that the process hasn't linked */
    ASSERT_EQ(participants[domain_id][participant_id]->process.get(), nullptr);
}

TEST_F(database_process_entities_tests, process_physical_entities_user_throws)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Insert a DomainParticipant*/
    EntityId participant_id = db.insert_new_participant(
        participant_name,
        entity_qos,
        participant_guid,
        domain_id,
        StatusLevel::OK_STATUS,
        AppId::UNKNOWN,
        "");
    std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>> participants = db.participants();

    std::map<std::string, EntityId> physical_entities_ids;
    physical_entities_ids[HOST_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[USER_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId::invalid();

    bool should_link_process_participant = true;

    /* Process Physical Entities*/
    EXPECT_THROW(db.process_physical_entities(
                host_name,
                "",
                "",
                "",
                should_link_process_participant,
                participant_id,
                physical_entities_ids), BadParameter);

    /* Check that the host is inserted correctly */
    std::map<EntityId, std::shared_ptr<Host>> hosts = db.hosts();
    ASSERT_EQ(hosts.size(), 1u);
    ASSERT_NE(hosts.find(physical_entities_ids[HOST_ENTITY_TAG]), hosts.end());
    ASSERT_EQ(host_name, hosts[physical_entities_ids[HOST_ENTITY_TAG]]->name);
    ASSERT_EQ(EntityKind::HOST, hosts[physical_entities_ids[HOST_ENTITY_TAG]]->kind);
    ASSERT_EQ(host_name, hosts[physical_entities_ids[HOST_ENTITY_TAG]]->alias);
    ASSERT_EQ(StatusLevel::OK_STATUS, hosts[physical_entities_ids[HOST_ENTITY_TAG]]->status);

    /* Check that the user hasn't been inserted */
    std::map<EntityId, std::shared_ptr<User>> users = db.users();
    ASSERT_EQ(users.size(), 0u);
    ASSERT_EQ(physical_entities_ids[USER_ENTITY_TAG], EntityId());

    /* Check that the process hasn't been inserted */
    std::map<EntityId, std::shared_ptr<Process>> processes = db.processes();
    ASSERT_EQ(processes.size(), 0u);
    ASSERT_EQ(physical_entities_ids[PROCESS_ENTITY_TAG], EntityId());

    /* Check that the process hasn't linked */
    ASSERT_EQ(participants[domain_id][participant_id]->process.get(), nullptr);
}

TEST_F(database_process_entities_tests, process_physical_entities_host_throws)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Insert a DomainParticipant*/
    EntityId participant_id = db.insert_new_participant(
        participant_name,
        entity_qos,
        participant_guid,
        domain_id,
        StatusLevel::OK_STATUS,
        AppId::UNKNOWN,
        "");
    std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>> participants = db.participants();

    std::map<std::string, EntityId> physical_entities_ids;
    physical_entities_ids[HOST_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[USER_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId::invalid();

    bool should_link_process_participant = true;

    /* Process Physical Entities*/
    EXPECT_THROW(db.process_physical_entities(
                "",
                "",
                "",
                "",
                should_link_process_participant,
                participant_id,
                physical_entities_ids), BadParameter);

    /* Check that the host hasn't been inserted */
    std::map<EntityId, std::shared_ptr<Host>> hosts = db.hosts();
    ASSERT_EQ(hosts.size(), 0u);
    ASSERT_EQ(physical_entities_ids[HOST_ENTITY_TAG], EntityId());

    /* Check that the user hasn't been inserted */
    std::map<EntityId, std::shared_ptr<User>> users = db.users();
    ASSERT_EQ(users.size(), 0u);
    ASSERT_EQ(physical_entities_ids[USER_ENTITY_TAG], EntityId());

    /* Check that the process hasn't been inserted */
    std::map<EntityId, std::shared_ptr<Process>> processes = db.processes();
    ASSERT_EQ(processes.size(), 0u);
    ASSERT_EQ(physical_entities_ids[PROCESS_ENTITY_TAG], EntityId());

    /* Check that the process hasn't linked */
    ASSERT_EQ(participants[domain_id][participant_id]->process.get(), nullptr);
}

TEST_F(database_process_entities_tests, is_topic_in_database)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);

    std::shared_ptr<Topic> topic = std::make_shared<Topic>(topic_name, topic_type, domain);
    EntityId topic_id = db.insert(topic);

    EXPECT_TRUE(db.is_topic_in_database(topic_type, topic_id));

    std::string other_topic_type = "other_test_topic";

    std::shared_ptr<Topic> other_topic = std::make_shared<Topic>(topic_name, other_topic_type, domain);
    EntityId other_topic_id = db.insert(other_topic);

    EXPECT_FALSE(db.is_topic_in_database(other_topic_type, topic_id));
    EXPECT_FALSE(db.is_topic_in_database(topic_type, other_topic_id));
    EXPECT_FALSE(db.is_topic_in_database(topic_type, EntityId()));
}

TEST_F(database_process_entities_tests, insert_new_topic)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Create a Topic*/
    EntityId topic_id = db.insert_new_topic(topic_name, topic_type, topic_alias, domain_id);

    /* Check that the topic is inserted correctly */
    std::map<EntityId, std::map<EntityId, std::shared_ptr<Topic>>> topics = db.topics();
    ASSERT_EQ(topics.size(), 1u);
    ASSERT_NE(topics[domain_id].find(topic_id), topics[domain_id].end());
    ASSERT_EQ(topic_name, topics[domain_id][topic_id]->name);
    ASSERT_EQ(EntityKind::TOPIC, topics[domain_id][topic_id]->kind);
    ASSERT_EQ(topic_alias, topics[domain_id][topic_id]->alias);
    ASSERT_EQ(StatusLevel::OK_STATUS, topics[domain_id][topic_id]->status);

    ASSERT_EQ(topic_type, topics[domain_id][topic_id]->data_type);
    ASSERT_EQ(domain->topics[topic_id].get(), topics[domain_id][topic_id].get());
}

TEST_F(database_process_entities_tests, insert_new_topic_already_exists)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Insert a DomainParticipant*/
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(
        participant_name,
        entity_qos,
        participant_guid,
        std::shared_ptr<Process>(),
        domain,
        StatusLevel::OK_STATUS,
        AppId::UNKNOWN,
        "");
    db.insert(participant);

    /* Insert a Topic*/
    std::shared_ptr<Topic> topic = std::make_shared<Topic>(
        topic_name,
        topic_type,
        domain);
    db.insert(topic);

    EXPECT_THROW(db.insert_new_topic(topic_name, topic_type, topic_alias, domain_id), BadParameter);
}

TEST_F(database_process_entities_tests, insert_new_topic_no_domain)
{
    EXPECT_THROW(db.insert_new_topic(topic_name, topic_type, topic_alias, EntityId()), BadParameter);
}

TEST_F(database_process_entities_tests, insert_new_endpoint_datawriter)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Insert a DomainParticipant*/
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(
        participant_name,
        entity_qos,
        participant_guid,
        std::shared_ptr<Process>(),
        domain,
        StatusLevel::OK_STATUS,
        AppId::UNKNOWN,
        "");
    EntityId participant_id = db.insert(participant);

    /* Insert a Topic*/
    std::shared_ptr<Topic> topic = std::make_shared<Topic>(
        topic_name,
        topic_type,
        domain);
    EntityId topic_id = db.insert(topic);

    /* Create a DataWriter*/
    Locator_t unicast_locator;
    std::stringstream(unicast_locator_str) >> unicast_locator;
    std::shared_ptr<database::Locator> ulocator = std::make_shared<database::Locator>(unicast_locator_str);
    EntityId ulocator_id = db.insert(ulocator);

    Locator_t multicast_locator;
    std::stringstream(multicast_locator_str) >> multicast_locator;
    std::shared_ptr<database::Locator> mlocator = std::make_shared<database::Locator>(multicast_locator_str);
    EntityId mlocator_id = db.insert(mlocator);
    RemoteLocatorList locators;
    locators.add_unicast_locator(unicast_locator);
    locators.add_multicast_locator(multicast_locator);

    bool is_virtual_metatraffic = true;

    EntityId endpoint_id = db.insert_new_endpoint(
        writer_guid,
        writer_name,
        writer_alias,
        entity_qos,
        is_virtual_metatraffic,
        locators,
        EntityKind::DATAWRITER,
        participant_id,
        topic_id,
        std::pair<AppId, std::string>(AppId::UNKNOWN, ""));

    /* Check that the endpoint is inserted correctly */
    std::map<EntityId, std::map<EntityId, std::shared_ptr<DataWriter>>> datawriters = db.datawriters();
    ASSERT_EQ(datawriters.size(), 1u);
    ASSERT_NE(datawriters[domain_id].find(endpoint_id), datawriters[domain_id].end());
    ASSERT_EQ(writer_name, datawriters[domain_id][endpoint_id]->name);
    ASSERT_EQ(EntityKind::DATAWRITER, datawriters[domain_id][endpoint_id]->kind);
    ASSERT_EQ(writer_alias, datawriters[domain_id][endpoint_id]->alias);
    ASSERT_EQ(StatusLevel::OK_STATUS, datawriters[domain_id][endpoint_id]->status);

    ASSERT_EQ(entity_qos, datawriters[domain_id][endpoint_id]->qos);
    ASSERT_EQ(writer_guid, datawriters[domain_id][endpoint_id]->guid);

    ASSERT_EQ(is_virtual_metatraffic, datawriters[domain_id][endpoint_id]->is_virtual_metatraffic);

    std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>> participants = db.participants();
    ASSERT_EQ(participants[domain_id][participant_id].get(), datawriters[domain_id][endpoint_id]->participant.get());
    ASSERT_EQ(participants[domain_id][participant_id]->data_writers[endpoint_id].get(),
            datawriters[domain_id][endpoint_id].get());

    std::map<EntityId, std::map<EntityId, std::shared_ptr<Topic>>> topics = db.topics();
    ASSERT_EQ(topics[domain_id][topic_id].get(), datawriters[domain_id][endpoint_id]->topic.get());
    ASSERT_EQ(topics[domain_id][topic_id]->data_writers[endpoint_id].get(), datawriters[domain_id][endpoint_id].get());

    std::map<EntityId, std::shared_ptr<database::Locator>> locators_db = db.locators();
    ASSERT_EQ(locators_db[ulocator_id].get(), datawriters[domain_id][endpoint_id]->locators[ulocator_id].get());
    ASSERT_EQ(locators_db[ulocator_id]->data_writers[endpoint_id].get(), datawriters[domain_id][endpoint_id].get());
    ASSERT_EQ(locators_db[mlocator_id].get(), datawriters[domain_id][endpoint_id]->locators[mlocator_id].get());
    ASSERT_EQ(locators_db[mlocator_id]->data_writers[endpoint_id].get(), datawriters[domain_id][endpoint_id].get());
}

TEST_F(database_process_entities_tests, insert_new_endpoint_datareader)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Insert a DomainParticipant*/
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(
        participant_name,
        entity_qos,
        participant_guid,
        std::shared_ptr<Process>(),
        domain,
        StatusLevel::OK_STATUS,
        AppId::UNKNOWN,
        "");
    EntityId participant_id = db.insert(participant);

    /* Insert a Topic*/
    std::shared_ptr<Topic> topic = std::make_shared<Topic>(
        topic_name,
        topic_type,
        domain);
    EntityId topic_id = db.insert(topic);

    /* Create a DataWriter*/
    Locator_t unicast_locator;
    std::stringstream(unicast_locator_str) >> unicast_locator;
    std::shared_ptr<database::Locator> ulocator = std::make_shared<database::Locator>(unicast_locator_str);
    EntityId ulocator_id = db.insert(ulocator);

    Locator_t multicast_locator;
    std::stringstream(multicast_locator_str) >> multicast_locator;
    std::shared_ptr<database::Locator> mlocator = std::make_shared<database::Locator>(multicast_locator_str);
    EntityId mlocator_id = db.insert(mlocator);

    RemoteLocatorList locators;
    locators.add_unicast_locator(unicast_locator);
    locators.add_multicast_locator(multicast_locator);

    bool is_virtual_metatraffic = true;

    EntityId endpoint_id = db.insert_new_endpoint(
        reader_guid,
        reader_name,
        reader_alias,
        entity_qos,
        is_virtual_metatraffic,
        locators,
        EntityKind::DATAREADER,
        participant_id,
        topic_id,
        std::pair<AppId, std::string>(AppId::UNKNOWN, ""));

    /* Check that the endpoint is inserted correctly */
    std::map<EntityId, std::map<EntityId, std::shared_ptr<DataReader>>> datareaders = db.datareaders();
    ASSERT_EQ(datareaders.size(), 1u);
    ASSERT_NE(datareaders[domain_id].find(endpoint_id), datareaders[domain_id].end());
    ASSERT_EQ(reader_name, datareaders[domain_id][endpoint_id]->name);
    ASSERT_EQ(EntityKind::DATAREADER, datareaders[domain_id][endpoint_id]->kind);
    ASSERT_EQ(reader_alias, datareaders[domain_id][endpoint_id]->alias);
    ASSERT_EQ(StatusLevel::OK_STATUS, datareaders[domain_id][endpoint_id]->status);

    ASSERT_EQ(entity_qos, datareaders[domain_id][endpoint_id]->qos);
    ASSERT_EQ(reader_guid, datareaders[domain_id][endpoint_id]->guid);

    ASSERT_EQ(is_virtual_metatraffic, datareaders[domain_id][endpoint_id]->is_virtual_metatraffic);

    std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>> participants = db.participants();
    ASSERT_EQ(participants[domain_id][participant_id].get(), datareaders[domain_id][endpoint_id]->participant.get());
    ASSERT_EQ(participants[domain_id][participant_id]->data_readers[endpoint_id].get(),
            datareaders[domain_id][endpoint_id].get());

    std::map<EntityId, std::map<EntityId, std::shared_ptr<Topic>>> topics = db.topics();
    ASSERT_EQ(topics[domain_id][topic_id].get(), datareaders[domain_id][endpoint_id]->topic.get());
    ASSERT_EQ(topics[domain_id][topic_id]->data_readers[endpoint_id].get(), datareaders[domain_id][endpoint_id].get());

    std::map<EntityId, std::shared_ptr<database::Locator>> locators_db = db.locators();
    ASSERT_EQ(locators_db[ulocator_id].get(), datareaders[domain_id][endpoint_id]->locators[ulocator_id].get());
    ASSERT_EQ(locators_db[ulocator_id]->data_readers[endpoint_id].get(), datareaders[domain_id][endpoint_id].get());
    ASSERT_EQ(locators_db[mlocator_id].get(), datareaders[domain_id][endpoint_id]->locators[mlocator_id].get());
    ASSERT_EQ(locators_db[mlocator_id]->data_readers[endpoint_id].get(), datareaders[domain_id][endpoint_id].get());
}

TEST_F(database_process_entities_tests, insert_new_endpoint_already_exists)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);

    /* Insert a DomainParticipant*/
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(
        participant_name,
        entity_qos,
        participant_guid,
        std::shared_ptr<Process>(),
        domain,
        StatusLevel::OK_STATUS,
        AppId::UNKNOWN,
        "");
    EntityId participant_id = db.insert(participant);

    /* Insert a Topic*/
    std::shared_ptr<Topic> topic = std::make_shared<Topic>(
        topic_name,
        topic_type,
        domain);
    EntityId topic_id = db.insert(topic);

    /* Insert a DataReader*/
    Locator_t unicast_locator;
    std::stringstream(unicast_locator_str) >> unicast_locator;
    std::shared_ptr<database::Locator> ulocator = std::make_shared<database::Locator>(unicast_locator_str);
    EntityId ulocator_id = db.insert(ulocator);

    Locator_t multicast_locator;
    std::stringstream(multicast_locator_str) >> multicast_locator;
    std::shared_ptr<database::Locator> mlocator = std::make_shared<database::Locator>(multicast_locator_str);
    EntityId mlocator_id = db.insert(mlocator);

    RemoteLocatorList locators;
    locators.add_unicast_locator(unicast_locator);
    locators.add_multicast_locator(multicast_locator);

    bool is_virtual_metatraffic = true;
    std::shared_ptr<DataReader> datareader = std::make_shared<DataReader>(
        reader_name,
        entity_qos,
        reader_guid,
        participant,
        topic);
    datareader->locators[ulocator_id] = ulocator;
    datareader->locators[mlocator_id] = mlocator;
    db.insert(datareader);

    /* Check that the endpoint insertion throws*/
    EXPECT_THROW(db.insert_new_endpoint(
                reader_guid,
                reader_name,
                reader_alias,
                entity_qos,
                is_virtual_metatraffic,
                locators,
                EntityKind::DATAREADER,
                participant_id,
                topic_id,
                std::pair<AppId, std::string>(AppId::UNKNOWN, "")), BadParameter);
}

TEST_F(database_process_entities_tests, insert_new_endpoint_no_topic)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);

    /* Insert a DomainParticipant*/
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(
        participant_name,
        entity_qos,
        participant_guid,
        std::shared_ptr<Process>(),
        domain,
        StatusLevel::OK_STATUS,
        AppId::UNKNOWN,
        "");
    EntityId participant_id = db.insert(participant);

    /* Insert a DataReader*/
    Locator_t unicast_locator;
    std::stringstream(unicast_locator_str) >> unicast_locator;
    std::shared_ptr<database::Locator> ulocator = std::make_shared<database::Locator>(unicast_locator_str);
    db.insert(ulocator);

    Locator_t multicast_locator;
    std::stringstream(multicast_locator_str) >> multicast_locator;
    std::shared_ptr<database::Locator> mlocator = std::make_shared<database::Locator>(multicast_locator_str);
    db.insert(mlocator);

    RemoteLocatorList locators;
    locators.add_unicast_locator(unicast_locator);
    locators.add_multicast_locator(multicast_locator);

    bool is_virtual_metatraffic = true;

    /* Check that the endpoint insertion throws*/
    EXPECT_THROW(db.insert_new_endpoint(
                reader_guid,
                reader_name,
                reader_alias,
                entity_qos,
                is_virtual_metatraffic,
                locators,
                EntityKind::DATAREADER,
                participant_id,
                EntityId(),
                std::pair<AppId, std::string>(AppId::UNKNOWN, "")), BadParameter);
}

TEST_F(database_process_entities_tests, insert_new_endpoint_no_participant)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);

    /* Insert a Topic*/
    std::shared_ptr<Topic> topic = std::make_shared<Topic>(
        topic_name,
        topic_type,
        domain);
    EntityId topic_id = db.insert(topic);

    /* Insert a DataReader*/
    Locator_t unicast_locator;
    std::stringstream(unicast_locator_str) >> unicast_locator;
    std::shared_ptr<database::Locator> ulocator = std::make_shared<database::Locator>(unicast_locator_str);
    db.insert(ulocator);

    Locator_t multicast_locator;
    std::stringstream(multicast_locator_str) >> multicast_locator;
    std::shared_ptr<database::Locator> mlocator = std::make_shared<database::Locator>(multicast_locator_str);
    db.insert(mlocator);

    RemoteLocatorList locators;
    locators.add_unicast_locator(unicast_locator);
    locators.add_multicast_locator(multicast_locator);

    bool is_virtual_metatraffic = true;

    /* Check that the endpoint insertion throws*/
    EXPECT_THROW(db.insert_new_endpoint(
                reader_guid,
                reader_name,
                reader_alias,
                entity_qos,
                is_virtual_metatraffic,
                locators,
                EntityKind::DATAREADER,
                EntityId(),
                topic_id,
                std::pair<AppId, std::string>(AppId::UNKNOWN, "")), BadParameter);
}

TEST_F(database_process_entities_tests, insert_new_endpoint_no_locators)
{
    /* Insert a domain */
    std::shared_ptr<Domain> domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Insert a DomainParticipant*/
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(
        participant_name,
        entity_qos,
        participant_guid,
        std::shared_ptr<Process>(),
        domain,
        StatusLevel::OK_STATUS,
        AppId::UNKNOWN,
        "");
    EntityId participant_id = db.insert(participant);

    /* Insert a Topic*/
    std::shared_ptr<Topic> topic = std::make_shared<Topic>(
        topic_name,
        topic_type,
        domain);
    EntityId topic_id = db.insert(topic);

    /* Insert a DataReader*/
    Locator_t unicast_locator;
    std::stringstream(unicast_locator_str) >> unicast_locator;

    Locator_t multicast_locator;
    std::stringstream(multicast_locator_str) >> multicast_locator;

    RemoteLocatorList locators;
    locators.add_unicast_locator(unicast_locator);
    locators.add_multicast_locator(multicast_locator);

    bool is_virtual_metatraffic = true;

    EntityId endpoint_id = db.insert_new_endpoint(
        reader_guid,
        reader_name,
        reader_alias,
        entity_qos,
        is_virtual_metatraffic,
        locators,
        EntityKind::DATAREADER,
        participant_id,
        topic_id,
        std::pair<AppId, std::string>(AppId::UNKNOWN, ""));

    /* Check that the endpoint is inserted correctly */
    std::map<EntityId, std::map<EntityId, std::shared_ptr<DataReader>>> datareaders = db.datareaders();
    ASSERT_EQ(datareaders.size(), 1u);
    ASSERT_NE(datareaders[domain_id].find(endpoint_id), datareaders[domain_id].end());
    ASSERT_EQ(reader_name, datareaders[domain_id][endpoint_id]->name);
    ASSERT_EQ(EntityKind::DATAREADER, datareaders[domain_id][endpoint_id]->kind);
    ASSERT_EQ(reader_alias, datareaders[domain_id][endpoint_id]->alias);
    ASSERT_EQ(StatusLevel::OK_STATUS, datareaders[domain_id][endpoint_id]->status);

    ASSERT_EQ(entity_qos, datareaders[domain_id][endpoint_id]->qos);
    ASSERT_EQ(reader_guid, datareaders[domain_id][endpoint_id]->guid);

    ASSERT_EQ(is_virtual_metatraffic, datareaders[domain_id][endpoint_id]->is_virtual_metatraffic);

    std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>> participants = db.participants();
    ASSERT_EQ(participants[domain_id][participant_id].get(), datareaders[domain_id][endpoint_id]->participant.get());
    ASSERT_EQ(participants[domain_id][participant_id]->data_readers[endpoint_id].get(),
            datareaders[domain_id][endpoint_id].get());

    std::map<EntityId, std::map<EntityId, std::shared_ptr<Topic>>> topics = db.topics();
    ASSERT_EQ(topics[domain_id][topic_id].get(), datareaders[domain_id][endpoint_id]->topic.get());
    ASSERT_EQ(topics[domain_id][topic_id]->data_readers[endpoint_id].get(), datareaders[domain_id][endpoint_id].get());

    std::map<EntityId, std::shared_ptr<database::Locator>> locators_db = db.locators();
    ASSERT_EQ(2u, datareaders[domain_id][endpoint_id]->locators.size());
    ASSERT_EQ(2u, locators_db.size());
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
