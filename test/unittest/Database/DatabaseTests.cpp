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

#include <chrono>
#include <memory>
#include <string>

#include "gtest/gtest.h"

#include <fastdds-statistics-backend/exception/Exception.hpp>
#include <fastdds-statistics-backend/types/EntityId.hpp>

#include <database/database.hpp>
#include <database/entities.hpp>
#include <database/samples.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

class DataBaseTest : public Database
{
public:

    const std::map<EntityId, std::shared_ptr<Host>>& hosts()
    {
        return hosts_;
    }

    const std::map<EntityId, std::shared_ptr<User>>& users()
    {
        return users_;
    }

    const std::map<EntityId, std::shared_ptr<Process>>& processes()
    {
        return processes_;
    }

    const std::map<EntityId, std::shared_ptr<Domain>>& domains()
    {
        return domains_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<Topic>>>& topics()
    {
        return topics_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>>& participants()
    {
        return participants_;
    }

    const std::map<EntityId, std::shared_ptr<Locator>>& locators()
    {
        return locators_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<Locator>>>& locators_by_participant()
    {
        return locators_by_participant_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>>& participants_by_locator()
    {
        return participants_by_locator_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<Domain>>>& domains_by_process()
    {
        return domains_by_process_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<Process>>>& processes_by_domain()
    {
        return processes_by_domain_;
    }

    template<typename T>
    std::map<EntityId, std::map<EntityId, std::shared_ptr<T>>>& get_dds_endpoints()
    {
        return dds_endpoints<T>();
    }

    Qos test_qos = {
        {"available_builtin_endpoints", 3135},
        {"lease_duration", {
             {"nanoseconds", 0},
             {"seconds", 3}
         }},
        {"properties", {
             {
                 {"name", "PARTICIPANT_TYPE"},
                 {"value", "CLIENT"}
             },
             {
                 {"name", "DS_VERSION"},
                 {"value", "2.0"}
             }
         }},
        {"user_data", "656e636c6176653d2f3b00"},
        {"vendor_id", "010f"}
    };
};

template<typename T>
void insert_ddsendpoint_valid()
{
    /* Insert a host, user, process, domain, topic, and participant */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    auto domain_id = db.insert(domain);
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "01.02.03.04", process, domain);
    auto participant_id = db.insert(participant);
    auto topic = std::make_shared<Topic>("test_topic_name", "test_topic_type", domain);
    db.insert(topic);

    /* Insert a DDSEndpoint */
    std::string endpoint_name = "test_endpoint";
    std::string endpoint_guid = "test_guid";
    auto endpoint = std::make_shared<T>(
        endpoint_name, db.test_qos, endpoint_guid, participant, topic);
    // Create a locator for the endpoint
    auto locator = std::make_shared<Locator>("test_locator");
    locator->id = db.generate_entity_id();
    endpoint->locators[locator->id] = locator;
    auto endpoint_id = db.insert(endpoint);

    /* Check that the endpoint is correctly inserted in participant */
    ASSERT_EQ(participant->ddsendpoints<T>().size(), 1);
    ASSERT_EQ(participant->ddsendpoints<T>()[endpoint_id].get(), endpoint.get());

    /* Check that the endpoint is correctly inserted in topic */
    ASSERT_EQ(topic->ddsendpoints<T>().size(), 1);
    ASSERT_EQ(topic->ddsendpoints<T>()[endpoint_id].get(), endpoint.get());

    /* Check x_by_y_ collections and locators_ */
    auto locators = db.locators();
    auto locators_by_participant = db.locators_by_participant();
    auto participants_by_locator = db.participants_by_locator();
    ASSERT_EQ(locators_by_participant[participant_id].size(), endpoint->locators.size());
    ASSERT_EQ(participants_by_locator[locator->id].size(), endpoint->locators.size());
    for (auto locator_it : endpoint->locators)
    {
        // Check that the endpoint's locators are correctly inserted in locators_
        ASSERT_NE(
            locators.find(locator_it.first),
            locators.end());
        // Check that the endpoint's locators are correctly inserted in locators_by_participant_
        ASSERT_NE(
            locators_by_participant[participant_id].find(locator_it.first),
            locators_by_participant[participant_id].end());
        // Check that the endpoint's participant is correctly inserted in participants_by_locator_
        ASSERT_NE(
            participants_by_locator[locator_it.first].find(participant_id),
            participants_by_locator[locator_it.first].end());
    }

    /* Check that the ddsendpoint is inserted correctly inserted in the endpoints_<T> collection */
    auto endpoints = db.get_dds_endpoints<T>();
    ASSERT_EQ(endpoints.size(), 1);
    ASSERT_EQ(endpoints[domain_id].size(), 1);
    ASSERT_NE(endpoints[domain_id].find(endpoint_id), endpoints[domain_id].end());
    ASSERT_EQ(endpoint_name, endpoints[domain_id][endpoint_id]->name);
    ASSERT_EQ(db.test_qos, endpoints[domain_id][endpoint_id]->qos);
    ASSERT_EQ(endpoint_guid, endpoints[domain_id][endpoint_id]->guid);
}

template<typename T>
void insert_ddsendpoint_two_valid()
{
    /* Insert a host, user, process, domain, topic, and participant */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    auto domain_id = db.insert(domain);
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "01.02.03.04", process, domain);
    auto participant_id = db.insert(participant);
    auto topic = std::make_shared<Topic>("test_topic_name", "test_topic_type", domain);
    db.insert(topic);

    /* Insert two DDSEndpoint */
    std::string endpoint_name = "test_endpoint";
    std::string endpoint_guid = "test_guid";
    auto endpoint = std::make_shared<T>(
        endpoint_name, db.test_qos, endpoint_guid, participant, topic);
    auto locator = std::make_shared<Locator>("test_locator");
    locator->id = db.generate_entity_id();
    endpoint->locators[locator->id] = locator;
    auto endpoint_id = db.insert(endpoint);

    std::string endpoint_name_2 = "test_endpoint_2";
    std::string endpoint_guid_2 = "test_guid_2";
    auto endpoint_2 = std::make_shared<T>(
        endpoint_name_2, db.test_qos, endpoint_guid_2, participant, topic);
    auto locator_2 = std::make_shared<Locator>("test_locator_2");
    locator_2->id = db.generate_entity_id();
    endpoint_2->locators[locator_2->id] = locator_2;
    auto endpoint_id_2 = db.insert(endpoint_2);

    /* Check that the endpoints are correctly inserted in participant */
    ASSERT_EQ(participant->ddsendpoints<T>().size(), 2);
    ASSERT_EQ(participant->ddsendpoints<T>()[endpoint_id].get(), endpoint.get());
    ASSERT_EQ(participant->ddsendpoints<T>()[endpoint_id_2].get(), endpoint_2.get());

    /* Check that the endpoints are correctly inserted in topic */
    ASSERT_EQ(topic->ddsendpoints<T>().size(), 2);
    ASSERT_EQ(topic->ddsendpoints<T>()[endpoint_id].get(), endpoint.get());
    ASSERT_EQ(topic->ddsendpoints<T>()[endpoint_id_2].get(), endpoint_2.get());

    /* Check x_by_y_ collections and locators_ */
    auto locators = db.locators();
    auto locators_by_participant = db.locators_by_participant();
    auto participants_by_locator = db.participants_by_locator();
    ASSERT_EQ(locators_by_participant[participant_id].size(), endpoint->locators.size() * 2);
    ASSERT_EQ(participants_by_locator[locator->id].size(), endpoint->locators.size());
    ASSERT_EQ(participants_by_locator[locator_2->id].size(), endpoint_2->locators.size());
    for (auto locator_it : endpoint->locators)
    {
        // Check that the endpoint's locators are correctly inserted in locators_
        ASSERT_NE(
            locators.find(locator_it.first),
            locators.end());
        // Check that the endpoint's locators are correctly inserted in locators_by_participant_
        ASSERT_NE(
            locators_by_participant[participant_id].find(locator_it.first),
            locators_by_participant[participant_id].end());
        // Check that the endpoint's participant is correctly inserted in participants_by_locator_
        ASSERT_NE(
            participants_by_locator[locator_it.first].find(participant_id),
            participants_by_locator[locator_it.first].end());
    }

    for (auto locator_it : endpoint_2->locators)
    {
        // Check that the endpoint's locators are correctly inserted in locators_
        ASSERT_NE(
            locators.find(locator_it.first),
            locators.end());
        // Check that the endpoint's locators are correctly inserted in locators_by_participant_
        ASSERT_NE(
            locators_by_participant[participant_id].find(locator_it.first),
            locators_by_participant[participant_id].end());
        // Check that the endpoint's participant is correctly inserted in participants_by_locator_
        ASSERT_NE(
            participants_by_locator[locator_it.first].find(participant_id),
            participants_by_locator[locator_it.first].end());
    }

    /* Check that the ddsendpoint is inserted correctly inserted in the endpoints_<T> collection */
    auto endpoints = db.get_dds_endpoints<T>();
    ASSERT_EQ(endpoints.size(), 1);
    ASSERT_EQ(endpoints[domain_id].size(), 2);
    ASSERT_NE(endpoints[domain_id].find(endpoint_id), endpoints[domain_id].end());
    ASSERT_NE(endpoints[domain_id].find(endpoint_id_2), endpoints[domain_id].end());
    ASSERT_EQ(endpoint_name, endpoints[domain_id][endpoint_id]->name);
    ASSERT_EQ(endpoint_name_2, endpoints[domain_id][endpoint_id_2]->name);
    ASSERT_EQ(db.test_qos, endpoints[domain_id][endpoint_id]->qos);
    ASSERT_EQ(db.test_qos, endpoints[domain_id][endpoint_id_2]->qos);
    ASSERT_EQ(endpoint_guid, endpoints[domain_id][endpoint_id]->guid);
    ASSERT_EQ(endpoint_guid_2, endpoints[domain_id][endpoint_id_2]->guid);
}

template<typename T>
void insert_ddsendpoint_duplicated()
{
    /* Insert a host, user, process, domain, topic, and participant */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "01.02.03.04", process, domain);
    db.insert(participant);
    auto topic = std::make_shared<Topic>("test_topic_name", "test_topic_type", domain);
    db.insert(topic);

    /* Insert a DDSEndpoint twice */
    auto endpoint = std::make_shared<T>(
        "test_endpoint", db.test_qos, "test_guid", participant, topic);
    auto locator = std::make_shared<Locator>("test_locator");
    locator->id = db.generate_entity_id();
    endpoint->locators[locator->id] = locator;
    db.insert(endpoint);
    ASSERT_THROW(db.insert(endpoint), BadParameter);
}

template<typename T>
void insert_ddsendpoint_wrong_participant()
{
    /* Insert a host, user, process, domain, topic, and participant */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "01.02.03.04", process, domain);
    db.insert(participant);
    auto topic = std::make_shared<Topic>("test_topic_name", "test_topic_type", domain);
    db.insert(topic);

    /* Insert a DDSEndpoint with a non-inserted participant */
    auto participant_2 = std::make_shared<DomainParticipant>(
        "test_participant_2", db.test_qos, "01.02.03.04.05", process, domain);
    auto endpoint = std::make_shared<T>(
        "test_endpoint", db.test_qos, "test_guid", participant_2, topic);
    auto locator = std::make_shared<Locator>("test_locator");
    locator->id = db.generate_entity_id();
    endpoint->locators[locator->id] = locator;
    ASSERT_THROW(db.insert(endpoint), BadParameter);
}

template<typename T>
void insert_ddsendpoint_wrong_topic()
{
    /* Insert a host, user, process, domain, topic, and participant */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "01.02.03.04", process, domain);
    db.insert(participant);
    auto topic = std::make_shared<Topic>("test_topic_name", "test_topic_type", domain);
    db.insert(topic);

    /* Insert a DDSEndpoint with a non-inserted topic */
    auto topic_2 = std::make_shared<Topic>("test_topic_name_2", "test_topic_type_2", domain);
    auto endpoint = std::make_shared<T>(
        "test_endpoint", db.test_qos, "test_guid", participant, topic_2);
    auto locator = std::make_shared<Locator>("test_locator");
    locator->id = db.generate_entity_id();
    endpoint->locators[locator->id] = locator;
    ASSERT_THROW(db.insert(endpoint), BadParameter);
}

template<typename T>
void insert_ddsendpoint_empty_name()
{
    /* Insert a host, user, process, domain, topic, and participant */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "01.02.03.04", process, domain);
    db.insert(participant);
    auto topic = std::make_shared<Topic>("test_topic_name", "test_topic_type", domain);
    db.insert(topic);

    /* Insert a DDSEndpoint with a non-inserted topic */
    auto endpoint = std::make_shared<T>(
        "", db.test_qos, "test_guid", participant, topic);
    auto locator = std::make_shared<Locator>("test_locator");
    locator->id = db.generate_entity_id();
    endpoint->locators[locator->id] = locator;
    ASSERT_THROW(db.insert(endpoint), BadParameter);
}

template<typename T>
void insert_ddsendpoint_empty_qos()
{
    /* Insert a host, user, process, domain, topic, and participant */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "01.02.03.04", process, domain);
    db.insert(participant);
    auto topic = std::make_shared<Topic>("test_topic_name", "test_topic_type", domain);
    db.insert(topic);

    /* Insert a DDSEndpoint with a non-inserted topic */
    auto endpoint = std::make_shared<T>(
        "test_endpoint", Qos(), "test_guid", participant, topic);
    auto locator = std::make_shared<Locator>("test_locator");
    locator->id = db.generate_entity_id();
    endpoint->locators[locator->id] = locator;
    ASSERT_THROW(db.insert(endpoint), BadParameter);
}

template<typename T>
void insert_ddsendpoint_empty_guid()
{
    /* Insert a host, user, process, domain, topic, and participant */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "01.02.03.04", process, domain);
    db.insert(participant);
    auto topic = std::make_shared<Topic>("test_topic_name", "test_topic_type", domain);
    db.insert(topic);

    /* Insert a DDSEndpoint with a non-inserted topic */
    auto endpoint = std::make_shared<T>(
        "test_endpoint", db.test_qos, "", participant, topic);
    auto locator = std::make_shared<Locator>("test_locator");
    locator->id = db.generate_entity_id();
    endpoint->locators[locator->id] = locator;
    ASSERT_THROW(db.insert(endpoint), BadParameter);
}

template<typename T>
void insert_ddsendpoint_empty_locators()
{
    /* Insert a host, user, process, domain, topic, and participant */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "01.02.03.04", process, domain);
    db.insert(participant);
    auto topic = std::make_shared<Topic>("test_topic_name", "test_topic_type", domain);
    db.insert(topic);

    /* Insert a DDSEndpoint with a non-inserted topic */
    auto endpoint = std::make_shared<T>(
        "test_endpoint", db.test_qos, "test_guid", participant, topic);
    ASSERT_THROW(db.insert(endpoint), BadParameter);
}

template<typename T>
void insert_ddsendpoint_two_same_domain_same_guid()
{
    /* Insert a host, user, process, domain, topic, and participant */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "01.02.03.04", process, domain);
    db.insert(participant);
    auto topic = std::make_shared<Topic>("test_topic_name", "test_topic_type", domain);
    db.insert(topic);

    /* Insert two DDSEndpoints with same GUID */
    std::string endpoint_guid = "test_guid";
    auto endpoint = std::make_shared<T>(
        "test_endpoint", db.test_qos, endpoint_guid, participant, topic);
    auto locator = std::make_shared<Locator>("test_locator");
    locator->id = db.generate_entity_id();
    endpoint->locators[locator->id] = locator;
    db.insert(endpoint);

    auto endpoint_2 = std::make_shared<T>(
        "test_endpoint_2", db.test_qos, endpoint_guid, participant, topic);
    endpoint_2->locators[locator->id] = locator;
    ASSERT_THROW(db.insert(endpoint_2), BadParameter);
}

template<typename T>
void insert_ddsendpoint_two_diff_domain_same_guid()
{
    /* Insert a host, user, process, domain, topic, and participant */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);
    auto domain_2 = std::make_shared<Domain>("test_domain_2");
    db.insert(domain_2);
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "01.02.03.04", process, domain);
    db.insert(participant);
    auto participant_2 = std::make_shared<DomainParticipant>(
        "test_participant_2", db.test_qos, "01.02.03.04.05", process, domain_2);
    db.insert(participant_2);
    auto topic = std::make_shared<Topic>("test_topic_name", "test_topic_type", domain);
    auto topic_2 = std::make_shared<Topic>("test_topic_name_2", "test_topic_type_2", domain_2);
    db.insert(topic);
    db.insert(topic_2);

    /* Insert two DDSEndpoints with same GUID */
    std::string endpoint_guid = "test_guid";
    auto endpoint = std::make_shared<T>(
        "test_endpoint", db.test_qos, endpoint_guid, participant, topic);
    auto locator = std::make_shared<Locator>("test_locator");
    locator->id = db.generate_entity_id();
    endpoint->locators[locator->id] = locator;
    db.insert(endpoint);

    auto endpoint_2 = std::make_shared<T>(
        "test_endpoint_2", db.test_qos, endpoint_guid, participant_2, topic_2);
    endpoint_2->locators[locator->id] = locator;
    ASSERT_THROW(db.insert(endpoint_2), BadParameter);
}

class database_tests : public ::testing::Test
{
public:

    void SetUp()
    {
        host.reset(new Host("test_host"));
        host_id = db.insert(host);
        user.reset(new User("test_user", host));
        user_id = db.insert(user);
        process.reset(new Process("test_process", "12345", user));
        process_id = db.insert(process);
        domain.reset(new Domain("test_domain"));
        domain_id = db.insert(domain);
        participant.reset(new DomainParticipant("test_participant", db.test_qos, "01.02.03.04", nullptr, domain));
        participant_id = db.insert(participant);
        topic.reset(new Topic("test_topic_name", "test_topic_type", domain));
        topic_id = db.insert(topic);
        writer_locator.reset(new Locator("writer_locator"));
        writer_locator->id = db.generate_entity_id();
        writer.reset(new DataWriter("test_writer", db.test_qos, "writer_guid", participant, topic));
        writer->locators[writer_locator->id] = writer_locator;
        writer_id = db.insert(writer);
        reader_locator.reset(new Locator("reader_locator"));
        reader_locator->id = db.generate_entity_id();
        reader.reset(new DataReader("test_reader", db.test_qos, "reader_guid", participant, topic));
        reader->locators[reader_locator->id] = reader_locator;
        reader_id = db.insert(reader);
    }

    DataBaseTest db;
    std::shared_ptr<Host> host;
    EntityId host_id;
    std::shared_ptr<User> user;
    EntityId user_id;
    std::shared_ptr<Process> process;
    EntityId process_id;
    std::shared_ptr<Domain> domain;
    EntityId domain_id;
    std::shared_ptr<DomainParticipant> participant;
    EntityId participant_id;
    std::shared_ptr<Topic> topic;
    EntityId topic_id;
    std::shared_ptr<Locator> writer_locator;
    std::shared_ptr<DataWriter> writer;
    EntityId writer_id;
    std::shared_ptr<Locator> reader_locator;
    std::shared_ptr<DataReader> reader;
    EntityId reader_id;
};

TEST_F(database_tests, insert_host)
{
    /* Insert a host */
    DataBaseTest db;
    std::string host_name = "test_host";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);
    EntityId host_id = db.insert(host);

    /* Check that the host is inserted correctly */
    std::map<EntityId, std::shared_ptr<Host>> hosts = db.hosts();
    ASSERT_EQ(hosts.size(), 1);
    ASSERT_NE(hosts.find(host_id), hosts.end());
    ASSERT_EQ(host_name, hosts[host_id]->name);
}

TEST_F(database_tests, insert_host_two)
{
    /* Insert two hosts */
    DataBaseTest db;
    std::string host_name = "test_host";
    std::string host_name_2 = "test_host_2";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);
    std::shared_ptr<Host> host_2 = std::make_shared<Host>(host_name_2);
    EntityId host_id = db.insert(host);
    EntityId host_id_2 = db.insert(host_2);

    /* Check that the hosts are inserted correctly */
    std::map<EntityId, std::shared_ptr<Host>> hosts = db.hosts();
    ASSERT_EQ(hosts.size(), 2);
    ASSERT_NE(hosts.find(host_id), hosts.end());
    ASSERT_NE(hosts.find(host_id_2), hosts.end());
    ASSERT_EQ(host_name, hosts[host_id]->name);
    ASSERT_EQ(host_name_2, hosts[host_id_2]->name);
}

TEST_F(database_tests, insert_host_two_same_name)
{
    /* Insert two hosts */
    DataBaseTest db;
    std::string host_name = "test_host";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);
    std::shared_ptr<Host> host_2 = std::make_shared<Host>(host_name);
    db.insert(host);
    ASSERT_THROW(db.insert(host_2), BadParameter);
}

TEST_F(database_tests, insert_host_duplicated)
{
    /* Insert a host twice */
    DataBaseTest db;
    std::shared_ptr<Host> host = std::make_shared<Host>("test_host");
    db.insert(host);
    ASSERT_THROW(db.insert(host), BadParameter);
}

TEST_F(database_tests, insert_host_empty_name)
{
    /* Insert a host with empty name */
    DataBaseTest db;
    auto host = std::make_shared<Host>("");
    ASSERT_THROW(db.insert(host), BadParameter);
}

TEST_F(database_tests, insert_user_valid)
{
    /* Insert a host */
    DataBaseTest db;
    std::shared_ptr<Host> host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    std::string user_name = "test_user";
    std::shared_ptr<User> user = std::make_shared<User>(user_name, host);
    EntityId user_id = db.insert(user);

    /* Check that the user is correctly inserted in host */
    ASSERT_EQ(host->users.size(), 1);
    ASSERT_EQ(host->users[user_id].get(), user.get());

    /* Check that the user is correctly inserted users_ */
    std::map<EntityId, std::shared_ptr<User>> users = db.users();
    ASSERT_EQ(users.size(), 1);
    ASSERT_NE(users.find(user_id), users.end());
    ASSERT_EQ(user_name, users[user_id]->name);
}

TEST_F(database_tests, insert_user_two_valid)
{
    /* Insert a host */
    DataBaseTest db;
    std::shared_ptr<Host> host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert two users */
    std::string user_name = "test_user";
    std::string user_name_2 = "test_user_2";
    std::shared_ptr<User> user = std::make_shared<User>(user_name, host);
    std::shared_ptr<User> user_2 = std::make_shared<User>(user_name_2, host);
    EntityId user_id = db.insert(user);
    EntityId user_id_2 = db.insert(user_2);

    /* Check that the users are correctly inserted in host */
    ASSERT_EQ(host->users.size(), 2);
    ASSERT_EQ(host->users[user_id].get(), user.get());
    ASSERT_EQ(host->users[user_id_2].get(), user_2.get());

    /* Check that the users are correctly inserted users_ */
    std::map<EntityId, std::shared_ptr<User>> users = db.users();
    ASSERT_EQ(users.size(), 2);
    ASSERT_NE(users.find(user_id), users.end());
    ASSERT_NE(users.find(user_id_2), users.end());
    ASSERT_EQ(user_name, users[user_id]->name);
    ASSERT_EQ(user_name_2, users[user_id_2]->name);
}

TEST_F(database_tests, insert_user_duplicated)
{
    /* Insert a host */
    DataBaseTest db;
    std::shared_ptr<Host> host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user twice */
    std::shared_ptr<User> user = std::make_shared<User>("test_user", host);
    db.insert(user);
    ASSERT_THROW(db.insert(user), BadParameter);
}

TEST_F(database_tests, insert_user_wrong_host)
{
    /* Insert a host */
    DataBaseTest db;
    std::shared_ptr<Host> host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert user with a non-inserted host */
    std::string user_name = "test_user";
    std::shared_ptr<Host> host_2 = std::make_shared<Host>("non-inserter_host");
    std::shared_ptr<User> user = std::make_shared<User>(user_name, host_2);
    ASSERT_THROW(db.insert(user), BadParameter);
}

TEST_F(database_tests, insert_user_empty_name)
{
    /* Insert a host */
    DataBaseTest db;
    std::shared_ptr<Host> host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user with empty name */
    std::shared_ptr<User> user = std::make_shared<User>("", host);
    ASSERT_THROW(db.insert(user), BadParameter);
}

TEST_F(database_tests, insert_user_duplicated_name)
{
    /* Insert a host */
    DataBaseTest db;
    std::shared_ptr<Host> host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    std::string user_name = "test_user";
    std::shared_ptr<User> user = std::make_shared<User>(user_name, host);
    db.insert(user);

    /* Insert a user with duplicated name */
    std::shared_ptr<User> user_2 = std::make_shared<User>(user_name, host);
    ASSERT_THROW(db.insert(user_2), BadParameter);
}

TEST_F(database_tests, insert_process_valid)
{
    /* Insert a host */
    DataBaseTest db;
    std::shared_ptr<Host> host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    std::shared_ptr<User> user = std::make_shared<User>("test_user", host);
    db.insert(user);

    /* Insert a process */
    std::string process_name = "test_process";
    std::string process_pid = "test_pid";
    std::shared_ptr<Process> process = std::make_shared<Process>(process_name, process_pid, user);
    EntityId process_id = db.insert(process);

    /* Check that the process is correctly inserted in user */
    ASSERT_EQ(user->processes.size(), 1);
    ASSERT_EQ(user->processes[process_id].get(), process.get());

    /* Check that the process is correctly inserted processes_s */
    auto processes = db.processes();
    ASSERT_EQ(processes.size(), 1);
    ASSERT_NE(processes.find(process_id), processes.end());
    ASSERT_EQ(process_name, processes[process_id]->name);
    ASSERT_EQ(process_pid, processes[process_id]->pid);
}

TEST_F(database_tests, insert_process_two_valid)
{
    /* Insert a host */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);

    /* Insert two processes */
    std::string process_name = "test_process";
    std::string process_name_2 = "test_process_2";
    std::string process_pid = "test_pid";
    std::string process_pid_2 = "test_pid_2";
    auto process = std::make_shared<Process>(process_name, process_pid, user);
    auto process_2 = std::make_shared<Process>(process_name_2, process_pid_2, user);
    EntityId process_id = db.insert(process);
    EntityId process_id_2 = db.insert(process_2);

    /* Check that the processes are correctly inserted in user */
    ASSERT_EQ(user->processes.size(), 2);
    ASSERT_EQ(user->processes[process_id].get(), process.get());
    ASSERT_EQ(user->processes[process_id_2].get(), process_2.get());

    /* Check that the processes are correctly inserted processes_ */
    auto processes = db.processes();
    ASSERT_EQ(processes.size(), 2);
    ASSERT_NE(processes.find(process_id), processes.end());
    ASSERT_NE(processes.find(process_id_2), processes.end());
    ASSERT_EQ(process_name, processes[process_id]->name);
    ASSERT_EQ(process_name_2, processes[process_id_2]->name);
}

TEST_F(database_tests, insert_process_duplicated)
{
    /* Insert a host */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);

    /* Insert a process twice */
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    ASSERT_THROW(db.insert(process), BadParameter);
}

TEST_F(database_tests, insert_process_wrong_user)
{
    /* Insert a host */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);

    /* Insert process with a non-inserted user */
    auto user_2 = std::make_shared<User>("non-inserter_user", host);
    auto process = std::make_shared<Process>("test_process", "test_pid", user_2);
    ASSERT_THROW(db.insert(process), BadParameter);
}

TEST_F(database_tests, insert_process_empty_name)
{
    /* Insert a host */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);

    /* Insert a process with empty pid */
    auto process = std::make_shared<Process>("", "test_pid", user);
    ASSERT_THROW(db.insert(process), BadParameter);
}

TEST_F(database_tests, insert_process_empty_pid)
{
    /* Insert a host */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);

    /* Insert a process with empty pid */
    auto process = std::make_shared<Process>("test_process", "", user);
    ASSERT_THROW(db.insert(process), BadParameter);
}

TEST_F(database_tests, insert_process_two_same_user_diff_pid)
{
    /* Insert a host */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);

    /* Insert a process */
    std::string process_pid = "test_pid";
    std::string process_pid_2 = "test_pid_2";
    auto process = std::make_shared<Process>("test_process", process_pid, user);
    auto process_2 = std::make_shared<Process>("test_process", process_pid_2, user);
    auto process_id = db.insert(process);
    auto process_id_2 = db.insert(process_2);

    /* Check that the processes are correctly inserted in user */
    ASSERT_EQ(user->processes.size(), 2);
    ASSERT_EQ(user->processes[process_id].get(), process.get());
    ASSERT_EQ(user->processes[process_id_2].get(), process_2.get());

    /* Check that the processes are correctly inserted processes_ */
    auto processes = db.processes();
    ASSERT_EQ(processes.size(), 2);
    ASSERT_NE(processes.find(process_id), processes.end());
    ASSERT_NE(processes.find(process_id_2), processes.end());
    ASSERT_EQ(process_pid, processes[process_id]->pid);
    ASSERT_EQ(process_pid_2, processes[process_id_2]->pid);
}

TEST_F(database_tests, insert_process_two_same_user_same_pid)
{
    /* Insert a host */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert a user */
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);

    /* Insert a process */
    std::string process_pid = "test_pid";
    auto process = std::make_shared<Process>("test_process", process_pid, user);
    db.insert(process);

    /* Insert a process in the same user with a duplicated pid */
    auto process_2 = std::make_shared<Process>("test_process", process_pid, user);
    ASSERT_THROW(db.insert(process_2), BadParameter);
}

TEST_F(database_tests, insert_process_two_diff_user_same_pid)
{
    /* Insert a host */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);

    /* Insert two users */
    auto user = std::make_shared<User>("test_user", host);
    auto user_2 = std::make_shared<User>("test_user_2", host);
    db.insert(user);
    db.insert(user_2);

    /* Insert a process */
    std::string process_pid = "test_pid";
    auto process = std::make_shared<Process>("test_process", process_pid, user);
    db.insert(process);

    /* Insert a process in the same host with a duplicated pid for a different user */
    auto process_2 = std::make_shared<Process>("test_process", process_pid, user_2);
    ASSERT_THROW(db.insert(process_2), BadParameter);
}

TEST_F(database_tests, insert_process_two_diff_host_same_pid)
{
    /* Insert two host */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    auto host_2 = std::make_shared<Host>("test_host_2");
    db.insert(host);
    db.insert(host_2);

    /* Insert two users */
    auto user = std::make_shared<User>("test_user", host);
    auto user_2 = std::make_shared<User>("test_user_2", host_2);
    db.insert(user);
    db.insert(user_2);

    /* Insert two process with same PID in different hosts */
    std::string process_pid = "test_pid";
    auto process = std::make_shared<Process>("test_process", process_pid, user);
    auto process_id = db.insert(process);
    auto process_2 = std::make_shared<Process>("test_process", process_pid, user_2);
    auto process_id_2 = db.insert(process_2);

    /* Check that the processes are correctly inserted in user */
    ASSERT_EQ(user->processes.size(), 1);
    ASSERT_EQ(user_2->processes.size(), 1);
    ASSERT_EQ(user->processes[process_id].get(), process.get());
    ASSERT_EQ(user_2->processes[process_id_2].get(), process_2.get());

    /* Check that the processes are correctly inserted processes_ */
    auto processes = db.processes();
    ASSERT_EQ(processes.size(), 2);
    ASSERT_NE(processes.find(process_id), processes.end());
    ASSERT_NE(processes.find(process_id_2), processes.end());
    ASSERT_EQ(process_pid, processes[process_id]->pid);
    ASSERT_EQ(process_pid, processes[process_id_2]->pid);
}

TEST_F(database_tests, insert_domain_valid)
{
    /* Insert a domain */
    DataBaseTest db;
    std::string domain_name = "test_domain";
    auto domain = std::make_shared<Domain>(domain_name);
    EntityId domain_id = db.insert(domain);

    /* Check that the domain is inserted correctly */
    auto domains = db.domains();
    ASSERT_EQ(domains.size(), 1);
    ASSERT_NE(domains.find(domain_id), domains.end());
    ASSERT_EQ(domain_name, domains[domain_id]->name);
}

TEST_F(database_tests, insert_domain_two_valid)
{
    /* Insert two domains */
    DataBaseTest db;
    std::string domain_name = "test_domain";
    std::string domain_name_2 = "test_domain_2";
    auto domain = std::make_shared<Domain>(domain_name);
    auto domain_2 = std::make_shared<Domain>(domain_name_2);
    EntityId domain_id = db.insert(domain);
    EntityId domain_id_2 = db.insert(domain_2);

    /* Check that the domains are inserted correctly */
    auto domains = db.domains();
    ASSERT_EQ(domains.size(), 2);
    ASSERT_NE(domains.find(domain_id), domains.end());
    ASSERT_NE(domains.find(domain_id_2), domains.end());
    ASSERT_EQ(domain_name, domains[domain_id]->name);
    ASSERT_EQ(domain_name_2, domains[domain_id_2]->name);
}

TEST_F(database_tests, insert_domain_duplicated)
{
    /* Insert a domain twice */
    DataBaseTest db;
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);
    ASSERT_THROW(db.insert(domain), BadParameter);
}

TEST_F(database_tests, insert_domain_empty_name)
{
    /* Insert a domain with empty name */
    DataBaseTest db;
    auto domain = std::make_shared<Domain>("");
    ASSERT_THROW(db.insert(domain), BadParameter);
}

TEST_F(database_tests, insert_domain_same_name)
{
    /* Insert two domains with same name */
    DataBaseTest db;
    auto domain = std::make_shared<Domain>("test_domain");
    auto domain_2 = std::make_shared<Domain>("test_domain");
    db.insert(domain);
    ASSERT_THROW(db.insert(domain_2), BadParameter);
}

TEST_F(database_tests, insert_topic_valid)
{
    /* Insert a domain */
    DataBaseTest db;
    auto domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Insert a topic */
    std::string topic_name = "test_topic_name";
    std::string topic_type = "test_topic_type";
    auto topic = std::make_shared<Topic>(topic_name, topic_type, domain);
    EntityId topic_id = db.insert(topic);

    /* Check that the topic is correctly inserted in domain */
    ASSERT_EQ(domain->topics.size(), 1);
    ASSERT_EQ(domain->topics[topic_id].get(), topic.get());

    /* Check that the topic is inserted correctly inserted in topic_ */
    auto topics = db.topics();
    ASSERT_EQ(topics.size(), 1);
    ASSERT_EQ(topics[domain_id].size(), 1);
    ASSERT_NE(topics[domain_id].find(topic_id), topics[domain_id].end());
    ASSERT_EQ(topic_name, topics[domain_id][topic_id]->name);
    ASSERT_EQ(topic_type, topics[domain_id][topic_id]->data_type);
}

TEST_F(database_tests, insert_topic_two_valid)
{
    /* Insert a domain */
    DataBaseTest db;
    auto domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Insert two topics */
    std::string topic_name = "test_topic_name";
    std::string topic_type = "test_topic_type";
    auto topic = std::make_shared<Topic>(topic_name, topic_type, domain);
    EntityId topic_id = db.insert(topic);

    std::string topic_name_2 = "test_topic_name_2";
    std::string topic_type_2 = "test_topic_type_2";
    auto topic_2 = std::make_shared<Topic>(topic_name_2, topic_type_2, domain);
    EntityId topic_id_2 = db.insert(topic_2);

    /* Check that the topics are correctly inserted in domain */
    ASSERT_EQ(domain->topics.size(), 2);
    ASSERT_EQ(domain->topics[topic_id].get(), topic.get());
    ASSERT_EQ(domain->topics[topic_id_2].get(), topic_2.get());

    /* Check that the topics are correctly inserted in topic_ */
    auto topics = db.topics();
    ASSERT_EQ(topics.size(), 1);
    ASSERT_EQ(topics[domain_id].size(), 2);
    ASSERT_NE(topics[domain_id].find(topic_id), topics[domain_id].end());
    ASSERT_NE(topics[domain_id].find(topic_id_2), topics[domain_id].end());
    ASSERT_EQ(topic_name, topics[domain_id][topic_id]->name);
    ASSERT_EQ(topic_name_2, topics[domain_id][topic_id_2]->name);
    ASSERT_EQ(topic_type, topics[domain_id][topic_id]->data_type);
    ASSERT_EQ(topic_type_2, topics[domain_id][topic_id_2]->data_type);
}

TEST_F(database_tests, insert_topic_duplicated)
{
    /* Insert a domain */
    DataBaseTest db;
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);

    /* Insert a topic twice */
    auto topic = std::make_shared<Topic>("test_topic_name", "test_topic_type", domain);
    db.insert(topic);
    ASSERT_THROW(db.insert(topic), BadParameter);
}

TEST_F(database_tests, insert_topic_wrong_domain)
{
    /* Insert a domain */
    DataBaseTest db;
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);

    /* Insert a topic with a non-inserted domain */
    auto domain_2 = std::make_shared<Domain>("test_domain_2");
    auto topic = std::make_shared<Topic>("test_topic_name", "test_topic_type", domain_2);
    ASSERT_THROW(db.insert(topic), BadParameter);
}

TEST_F(database_tests, insert_topic_empty_name)
{
    /* Insert a domain */
    DataBaseTest db;
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);

    /* Insert a topic with empty name */
    auto topic = std::make_shared<Topic>("", "test_topic_type", domain);
    ASSERT_THROW(db.insert(topic), BadParameter);
}

TEST_F(database_tests, insert_topic_empty_datatype)
{
    /* Insert a domain */
    DataBaseTest db;
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);

    /* Insert a topic with empty data_type */
    auto topic = std::make_shared<Topic>("test_topic_name", "", domain);
    ASSERT_THROW(db.insert(topic), BadParameter);
}

TEST_F(database_tests, insert_topic_two_same_domain_same_name)
{
    /* Insert a domain */
    DataBaseTest db;
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);

    /* Insert a topic with a non-inserted domain */
    auto topic = std::make_shared<Topic>("test_topic_name", "test_topic_type", domain);
    auto topic_2 = std::make_shared<Topic>("test_topic_name", "test_topic_type_2", domain);
    db.insert(topic);
    ASSERT_THROW(db.insert(topic_2), BadParameter);
}

TEST_F(database_tests, insert_topic_two_same_domain_diff_name_same_type)
{
    /* Insert a domain */
    DataBaseTest db;
    auto domain = std::make_shared<Domain>("test_domain");
    EntityId domain_id = db.insert(domain);

    /* Insert two topics */
    std::string topic_name = "test_topic_name";
    std::string topic_type = "test_topic_type";
    auto topic = std::make_shared<Topic>(topic_name, topic_type, domain);
    EntityId topic_id = db.insert(topic);

    std::string topic_name_2 = "test_topic_name_2";
    auto topic_2 = std::make_shared<Topic>(topic_name_2, topic_type, domain);
    EntityId topic_id_2 = db.insert(topic_2);

    /* Check that the topics are correctly inserted in domain */
    ASSERT_EQ(domain->topics.size(), 2);
    ASSERT_EQ(domain->topics[topic_id].get(), topic.get());
    ASSERT_EQ(domain->topics[topic_id_2].get(), topic_2.get());

    /* Check that the topics are correctly inserted in topic_ */
    auto topics = db.topics();
    ASSERT_EQ(topics.size(), 1);
    ASSERT_EQ(topics[domain_id].size(), 2);
    ASSERT_NE(topics[domain_id].find(topic_id), topics[domain_id].end());
    ASSERT_NE(topics[domain_id].find(topic_id_2), topics[domain_id].end());
    ASSERT_EQ(topic_name, topics[domain_id][topic_id]->name);
    ASSERT_EQ(topic_name_2, topics[domain_id][topic_id_2]->name);
    ASSERT_EQ(topic_type, topics[domain_id][topic_id]->data_type);
    ASSERT_EQ(topic_type, topics[domain_id][topic_id_2]->data_type);
}

TEST_F(database_tests, insert_participant_valid)
{
    /* Insert a host, user, and process */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);

    /* Insert a domain */
    auto domain = std::make_shared<Domain>("test_domain");
    auto domain_id = db.insert(domain);

    /* Insert a DomainParticipant */
    std::string part_name = "test_participant";
    std::string part_guid = "01.02.03.04";
    auto participant = std::make_shared<DomainParticipant>(
        part_name, db.test_qos, part_guid, process, domain);
    auto participant_id = db.insert(participant);

    /* Check that the participant is correctly inserted in domain */
    ASSERT_EQ(domain->participants.size(), 1);
    ASSERT_EQ(domain->participants[participant_id].get(), participant.get());

    /* Check that the participant is inserted correctly inserted in participants_ */
    auto participants = db.participants();
    ASSERT_EQ(participants.size(), 1);
    ASSERT_EQ(participants[domain_id].size(), 1);
    ASSERT_NE(participants[domain_id].find(participant_id), participants[domain_id].end());
    ASSERT_EQ(part_name, participants[domain_id][participant_id]->name);
    ASSERT_EQ(db.test_qos, participants[domain_id][participant_id]->qos);
    ASSERT_EQ(part_guid, participants[domain_id][participant_id]->guid);
}

TEST_F(database_tests, insert_participant_two_valid)
{
    /* Insert a host, user, and process */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);

    /* Insert a domain */
    auto domain = std::make_shared<Domain>("test_domain");
    auto domain_id = db.insert(domain);

    /* Insert two DomainParticipants */
    std::string part_name = "test_participant";
    std::string part_guid = "01.02.03.04";
    auto participant = std::make_shared<DomainParticipant>(
        part_name, db.test_qos, part_guid, process, domain);
    auto participant_id = db.insert(participant);

    std::string part_name_2 = "test_participant_2";
    std::string part_guid_2 = "01.02.03.04.05";
    auto participant_2 = std::make_shared<DomainParticipant>(
        part_name_2, db.test_qos, part_guid_2, process, domain);
    auto participant_id_2 = db.insert(participant_2);

    /* Check that the participants are correctly inserted in domain */
    ASSERT_EQ(domain->participants.size(), 2);
    ASSERT_EQ(domain->participants[participant_id].get(), participant.get());
    ASSERT_EQ(domain->participants[participant_id_2].get(), participant_2.get());

    /* Check that the participants are inserted correctly inserted in participants_ */
    auto participants = db.participants();
    ASSERT_EQ(participants.size(), 1);
    ASSERT_EQ(participants[domain_id].size(), 2);
    ASSERT_NE(participants[domain_id].find(participant_id), participants[domain_id].end());
    ASSERT_NE(participants[domain_id].find(participant_id_2), participants[domain_id].end());
    ASSERT_EQ(part_name, participants[domain_id][participant_id]->name);
    ASSERT_EQ(part_name_2, participants[domain_id][participant_id_2]->name);
    ASSERT_EQ(db.test_qos, participants[domain_id][participant_id]->qos);
    ASSERT_EQ(part_guid, participants[domain_id][participant_id]->guid);
    ASSERT_EQ(part_guid_2, participants[domain_id][participant_id_2]->guid);
}

TEST_F(database_tests, insert_participant_duplicated)
{
    /* Insert a host, user, process, and domain */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);

    /* Insert a DomainParticipant */
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "01.02.03.04", process, domain);
    db.insert(participant);
    ASSERT_THROW(db.insert(participant), BadParameter);
}

TEST_F(database_tests, insert_participant_wrong_domain)
{
    /* Insert a host, user, process, and domain */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);

    /* Insert a DomainParticipant in a non-inserted domain */
    auto domain_2 = std::make_shared<Domain>("test_domain_2");
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "01.02.03.04", process, domain_2);
    ASSERT_THROW(db.insert(participant), BadParameter);
}

TEST_F(database_tests, insert_participant_empty_name)
{
    /* Insert a host, user, process, and domain */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);

    /* Insert a DomainParticipant with an empty name */
    auto participant = std::make_shared<DomainParticipant>(
        "", db.test_qos, "01.02.03.04", process, domain);
    ASSERT_THROW(db.insert(participant), BadParameter);
}

TEST_F(database_tests, insert_participant_empty_qos)
{
    /* Insert a host, user, process, and domain */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);

    /* Insert a DomainParticipant with an empty name */
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", Qos(), "01.02.03.04", process, domain);
    ASSERT_THROW(db.insert(participant), BadParameter);
}

TEST_F(database_tests, insert_participant_empty_guid)
{
    /* Insert a host, user, process, and domain */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);

    /* Insert a DomainParticipant with an empty name */
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "", process, domain);
    ASSERT_THROW(db.insert(participant), BadParameter);
}

TEST_F(database_tests, insert_participant_two_same_domain_same_guid)
{
    /* Insert a host, user, process, and domain */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);

    /* Insert two DomainParticipants with same domain and guid */
    std::string part_guid = "01.02.03.04";
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, part_guid, process, domain);
    auto participant_2 = std::make_shared<DomainParticipant>(
        "test_participant_2", db.test_qos, part_guid, process, domain);
    db.insert(participant);
    ASSERT_THROW(db.insert(participant_2), BadParameter);
}

TEST_F(database_tests, insert_participant_two_diff_domain_same_guid)
{
    /* Insert a host, user, process, and domain */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);
    auto domain_2 = std::make_shared<Domain>("test_domain_2");
    db.insert(domain_2);

    /* Insert two DomainParticipants with different domain and same guid */
    std::string part_name = "test_participant";
    std::string part_name_2 = "test_participant_2";
    std::string part_guid = "01.02.03.04";
    auto participant = std::make_shared<DomainParticipant>(
        part_name, db.test_qos, part_guid, process, domain);
    auto participant_2 = std::make_shared<DomainParticipant>(
        part_name_2, db.test_qos, part_guid, process, domain_2);
    db.insert(participant);
    ASSERT_THROW(db.insert(participant_2), BadParameter);
}

TEST_F(database_tests, insert_ddsendpoint_valid)
{
    insert_ddsendpoint_valid<DataReader>();
    insert_ddsendpoint_valid<DataWriter>();
}

TEST_F(database_tests, insert_ddsendpoint_two_valid)
{
    insert_ddsendpoint_two_valid<DataReader>();
    insert_ddsendpoint_two_valid<DataWriter>();
}

TEST_F(database_tests, insert_ddsendpoint_duplicated)
{
    insert_ddsendpoint_duplicated<DataReader>();
    insert_ddsendpoint_duplicated<DataWriter>();
}

TEST_F(database_tests, insert_ddsendpoint_wrong_participant)
{
    insert_ddsendpoint_wrong_participant<DataReader>();
    insert_ddsendpoint_wrong_participant<DataWriter>();
}

TEST_F(database_tests, insert_ddsendpoint_wrong_topic)
{
    insert_ddsendpoint_wrong_topic<DataReader>();
    insert_ddsendpoint_wrong_topic<DataWriter>();
}

TEST_F(database_tests, insert_ddsendpoint_empty_name)
{
    insert_ddsendpoint_empty_name<DataReader>();
    insert_ddsendpoint_empty_name<DataWriter>();
}

TEST_F(database_tests, insert_ddsendpoint_empty_qos)
{
    insert_ddsendpoint_empty_qos<DataReader>();
    insert_ddsendpoint_empty_qos<DataWriter>();
}

TEST_F(database_tests, insert_ddsendpoint_empty_guid)
{
    insert_ddsendpoint_empty_guid<DataReader>();
    insert_ddsendpoint_empty_guid<DataWriter>();
}

TEST_F(database_tests, insert_ddsendpoint_empty_locators)
{
    insert_ddsendpoint_empty_locators<DataReader>();
    insert_ddsendpoint_empty_locators<DataWriter>();
}

TEST_F(database_tests, insert_ddsendpoint_two_same_domain_same_guid)
{
    insert_ddsendpoint_two_same_domain_same_guid<DataReader>();
    insert_ddsendpoint_two_same_domain_same_guid<DataWriter>();
}

TEST_F(database_tests, insert_ddsendpoint_two_diff_domain_same_guid)
{
    insert_ddsendpoint_two_diff_domain_same_guid<DataReader>();
    insert_ddsendpoint_two_diff_domain_same_guid<DataWriter>();
}

TEST_F(database_tests, insert_invalid)
{
    /* Insert an entity */
    DataBaseTest db;
    std::shared_ptr<Entity> entity = std::make_shared<Entity>();
    EntityId entity_id = db.insert(entity);
    ASSERT_EQ(entity_id, EntityId::invalid());
}

TEST_F(database_tests, link_participant_with_process_unlinked)
{
    /* Insert a host, user, process, domain, and participant */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    auto process_id = db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    auto domain_id = db.insert(domain);
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "01.02.03.04", nullptr, domain);
    auto participant_id = db.insert(participant);

    /* Link participant with process */
    ASSERT_NO_THROW(db.link_participant_with_process(participant_id, process_id));

    /* Check that the participant is correctly inserted in process */
    ASSERT_EQ(process->participants.size(), 1);
    ASSERT_EQ(process->participants[participant_id].get(), participant.get());

    /* Check that domain is inserted correctly in domains_by_process_ */
    auto domains_by_process = db.domains_by_process();
    ASSERT_EQ(domains_by_process.size(), 1);
    ASSERT_EQ(domains_by_process[process_id].size(), 1);
    ASSERT_EQ(domains_by_process[process_id][domain_id].get(), domain.get());

    /* Check that domain is inserted correctly in processes_by_domain_ */
    auto processes_by_domain = db.processes_by_domain();
    ASSERT_EQ(processes_by_domain.size(), 1);
    ASSERT_EQ(processes_by_domain[domain_id].size(), 1);
    ASSERT_EQ(processes_by_domain[domain_id][process_id].get(), process.get());
}

TEST_F(database_tests, link_participant_with_process_wrong_participant)
{
    /* Insert a host, user, process, domain, and participant */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    auto process_id = db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "01.02.03.04", nullptr, domain);
    db.insert(participant);

    /* Link another participant with process */
    ASSERT_THROW(db.link_participant_with_process(EntityId(12), process_id), BadParameter);
}

TEST_F(database_tests, link_participant_with_process_wrong_process)
{
    /* Insert a host, user, process, domain, and participant */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "01.02.03.04", nullptr, domain);
    auto participant_id = db.insert(participant);

    /* Link participant with another process */
    ASSERT_THROW(db.link_participant_with_process(participant_id, EntityId(12)), BadParameter);
}

TEST_F(database_tests, link_participant_with_process_linked_participant)
{
    /* Insert a host, user, process, domain, and participant */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    db.insert(host);
    auto user = std::make_shared<User>("test_user", host);
    db.insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    auto process_id = db.insert(process);
    auto domain = std::make_shared<Domain>("test_domain");
    db.insert(domain);
    auto participant = std::make_shared<DomainParticipant>(
        "test_participant", db.test_qos, "01.02.03.04", nullptr, domain);
    auto participant_id = db.insert(participant);

    /* Link participant with process twice */
    ASSERT_NO_THROW(db.link_participant_with_process(participant_id, process_id));
    ASSERT_THROW(db.link_participant_with_process(participant_id, process_id), BadParameter);
}

TEST_F(database_tests, insert_sample_history_latency)
{
    HistoryLatencySample sample;
    sample.reader = reader_id;
    sample.data = 12;
    sample.src_ts = std::chrono::steady_clock::now();
    ASSERT_NO_THROW(db.insert(domain_id, writer_id, sample));

    ASSERT_EQ(writer->data.history2history_latency[reader_id].size(), 1);
    ASSERT_EQ(writer->data.history2history_latency[reader_id][0], static_cast<EntityDataSample>(sample));
}

TEST_F(database_tests, insert_sample_history_latency_wrong_entity)
{
    HistoryLatencySample sample;
    sample.reader = db.generate_entity_id();
    sample.data = 12;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_network_latency)
{
    NetworkLatencySample sample;
    sample.remote_locator = reader_locator->id;
    sample.data = 12;
    ASSERT_NO_THROW(db.insert(domain_id, writer_locator->id, sample));

    ASSERT_EQ(writer_locator->data.network_latency_per_locator[reader_locator->id].size(), 1);
    ASSERT_EQ(writer_locator->data.network_latency_per_locator[reader_locator->id][0],
            static_cast<EntityDataSample>(sample));
}

TEST_F(database_tests, insert_sample_network_latency_wrong_entity)
{
    NetworkLatencySample sample;
    sample.remote_locator = db.generate_entity_id();
    sample.data = 12;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_publication_throughput)
{
    PublicationThroughputSample sample;
    sample.data = 12;
    ASSERT_NO_THROW(db.insert(domain_id, writer_id, sample));

    ASSERT_EQ(writer->data.publication_throughput.size(), 1);
    ASSERT_EQ(writer->data.publication_throughput[0], static_cast<EntityDataSample>(sample));
}

TEST_F(database_tests, insert_sample_publication_throughput_wrong_entity)
{
    PublicationThroughputSample sample;
    sample.data = 12;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_subscription_throughput)
{
    SubscriptionThroughputSample sample;
    sample.data = 12;
    ASSERT_NO_THROW(db.insert(domain_id, reader_id, sample));

    ASSERT_EQ(reader->data.subscription_throughput.size(), 1);
    ASSERT_EQ(reader->data.subscription_throughput[0], static_cast<EntityDataSample>(sample));
}

TEST_F(database_tests, insert_sample_subscription_throughput_wrong_entity)
{
    SubscriptionThroughputSample sample;
    sample.data = 12;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_rtps_packets_sent)
{
    RtpsPacketsSentSample sample;
    sample.remote_locator = writer_locator->id;
    sample.count = 12;
    ASSERT_NO_THROW(db.insert(domain_id, writer_id, sample));

    ASSERT_EQ(writer->data.rtps_packets_sent.size(), 1);
    ASSERT_EQ(writer->data.rtps_packets_sent[writer_locator->id][0], static_cast<EntityCountSample>(sample));
}

TEST_F(database_tests, insert_sample_rtps_packets_sent_wrong_entity)
{
    RtpsPacketsSentSample sample;
    sample.remote_locator = db.generate_entity_id();
    sample.count = 12;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_rtps_bytes_sent)
{
    RtpsBytesSentSample sample;
    sample.remote_locator = writer_locator->id;
    sample.count = 12;
    sample.magnitude_order = 2;
    ASSERT_NO_THROW(db.insert(domain_id, writer_id, sample));

    ASSERT_EQ(writer->data.rtps_bytes_sent.size(), 1);
    ASSERT_EQ(writer->data.rtps_bytes_sent[writer_locator->id][0], static_cast<ByteCountSample>(sample));
}

TEST_F(database_tests, insert_sample_rtps_bytes_sent_wrong_entity)
{
    RtpsBytesSentSample sample;
    sample.remote_locator = db.generate_entity_id();
    sample.count = 12;
    sample.magnitude_order = 2;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_rtps_packets_lost)
{
    RtpsPacketsLostSample sample;
    sample.remote_locator = writer_locator->id;
    sample.count = 12;
    ASSERT_NO_THROW(db.insert(domain_id, writer_id, sample));

    ASSERT_EQ(writer->data.rtps_packets_lost.size(), 1);
    ASSERT_EQ(writer->data.rtps_packets_lost[writer_locator->id][0], static_cast<EntityCountSample>(sample));
}

TEST_F(database_tests, insert_sample_rtps_packets_lost_wrong_entity)
{
    RtpsPacketsLostSample sample;
    sample.remote_locator = db.generate_entity_id();
    sample.count = 12;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_rtps_bytes_lost)
{
    RtpsBytesLostSample sample;
    sample.remote_locator = writer_locator->id;
    sample.count = 12;
    sample.magnitude_order = 2;
    ASSERT_NO_THROW(db.insert(domain_id, writer_id, sample));

    ASSERT_EQ(writer->data.rtps_bytes_lost.size(), 1);
    ASSERT_EQ(writer->data.rtps_bytes_lost[writer_locator->id][0], static_cast<ByteCountSample>(sample));
}

TEST_F(database_tests, insert_sample_rtps_bytes_lost_wrong_entity)
{
    RtpsBytesLostSample sample;
    sample.remote_locator = db.generate_entity_id();
    sample.count = 12;
    sample.magnitude_order = 2;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_resent_data)
{
    ResentDataSample sample;
    sample.count = 12;
    ASSERT_NO_THROW(db.insert(domain_id, writer_id, sample));

    ASSERT_EQ(writer->data.resent_datas.size(), 1);
    ASSERT_EQ(writer->data.resent_datas[0], static_cast<EntityCountSample>(sample));
}

TEST_F(database_tests, insert_sample_resent_data_wrong_entity)
{
    ResentDataSample sample;
    sample.count = 12;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_heartbeat_count)
{
    HeartbeatCountSample sample;
    sample.count = 12;
    ASSERT_NO_THROW(db.insert(domain_id, writer_id, sample));

    ASSERT_EQ(writer->data.heartbeat_count.size(), 1);
    ASSERT_EQ(writer->data.heartbeat_count[0], static_cast<EntityCountSample>(sample));
}

TEST_F(database_tests, insert_sample_heartbeat_count_wrong_entity)
{
    HeartbeatCountSample sample;
    sample.count = 12;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_acknack_count)
{
    AcknackCountSample sample;
    sample.count = 12;
    ASSERT_NO_THROW(db.insert(domain_id, reader_id, sample));

    ASSERT_EQ(reader->data.acknack_count.size(), 1);
    ASSERT_EQ(reader->data.acknack_count[0], static_cast<EntityCountSample>(sample));
}

TEST_F(database_tests, insert_sample_acknack_count_wrong_entity)
{
    AcknackCountSample sample;
    sample.count = 12;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_nackfrag_count)
{
    NackfragCountSample sample;
    sample.count = 12;
    ASSERT_NO_THROW(db.insert(domain_id, reader_id, sample));

    ASSERT_EQ(reader->data.nackfrag_count.size(), 1);
    ASSERT_EQ(reader->data.nackfrag_count[0], static_cast<EntityCountSample>(sample));
}

TEST_F(database_tests, insert_sample_nackfrag_count_wrong_entity)
{
    NackfragCountSample sample;
    sample.count = 12;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_gap_count)
{
    GapCountSample sample;
    sample.count = 12;
    ASSERT_NO_THROW(db.insert(domain_id, writer_id, sample));

    ASSERT_EQ(writer->data.gap_count.size(), 1);
    ASSERT_EQ(writer->data.gap_count[0], static_cast<EntityCountSample>(sample));
}

TEST_F(database_tests, insert_sample_gap_count_wrong_entity)
{
    GapCountSample sample;
    sample.count = 12;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_data_count)
{
    DataCountSample sample;
    sample.count = 12;
    ASSERT_NO_THROW(db.insert(domain_id, writer_id, sample));

    ASSERT_EQ(writer->data.data_count.size(), 1);
    ASSERT_EQ(writer->data.data_count[0], static_cast<EntityCountSample>(sample));
}

TEST_F(database_tests, insert_sample_data_count_wrong_entity)
{
    DataCountSample sample;
    sample.count = 12;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_pdp_packets)
{
    PdpCountSample sample;
    sample.count = 12;
    ASSERT_NO_THROW(db.insert(domain_id, participant_id, sample));

    ASSERT_EQ(participant->data.pdp_packets.size(), 1);
    ASSERT_EQ(participant->data.pdp_packets[0], static_cast<EntityCountSample>(sample));
}

TEST_F(database_tests, insert_sample_pdp_packets_wrong_entity)
{
    PdpCountSample sample;
    sample.count = 12;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_edp_packets)
{
    EdpCountSample sample;
    sample.count = 12;
    ASSERT_NO_THROW(db.insert(domain_id, participant_id, sample));

    ASSERT_EQ(participant->data.edp_packets.size(), 1);
    ASSERT_EQ(participant->data.edp_packets[0], static_cast<EntityCountSample>(sample));
}

TEST_F(database_tests, insert_sample_edp_packets_wrong_entity)
{
    EdpCountSample sample;
    sample.count = 12;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_discovery_time)
{
    DiscoveryTimeSample sample;
    sample.remote_entity = writer_id;
    sample.time = std::chrono::steady_clock::now();
    sample.discovered = true;
    ASSERT_NO_THROW(db.insert(domain_id, participant_id, sample));

    ASSERT_EQ(participant->data.discovered_entity[writer_id].size(), 1);
    ASSERT_EQ(participant->data.discovered_entity[writer_id][0].first, sample.time);
    ASSERT_EQ(participant->data.discovered_entity[writer_id][0].second, sample.discovered);
}

TEST_F(database_tests, insert_sample_discovery_time_wrong_entity)
{
    DiscoveryTimeSample sample;
    sample.remote_entity = db.generate_entity_id();
    sample.time = std::chrono::steady_clock::now();
    sample.discovered = true;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_sample_datas)
{
    SampleDatasCountSample sample;
    sample.sequence_number = 2;
    sample.count = 12;
    ASSERT_NO_THROW(db.insert(domain_id, writer_id, sample));

    ASSERT_EQ(writer->data.sample_datas.size(), 1);
    ASSERT_EQ(writer->data.sample_datas[sample.sequence_number], sample.count);
}

TEST_F(database_tests, insert_sample_sample_datas_wrong_entity)
{
    SampleDatasCountSample sample;
    sample.sequence_number = 2;
    sample.count = 12;
    ASSERT_THROW(db.insert(domain_id, db.generate_entity_id(), sample), BadParameter);
}

TEST_F(database_tests, insert_sample_invalid)
{
    StatisticsSample sample;
    ASSERT_THROW(db.insert(domain_id, writer_id, sample), BadParameter);
}

TEST_F(database_tests, insert_sample_valid_wrong_domain)
{
    HistoryLatencySample history_lantency_sample;
    ASSERT_THROW(db.insert(db.generate_entity_id(), writer_id, history_lantency_sample), BadParameter);

    NetworkLatencySample network_lantency_sample;
    ASSERT_NO_THROW(db.insert(db.generate_entity_id(), writer_locator->id, network_lantency_sample));

    PublicationThroughputSample pub_throughput_sample;
    ASSERT_THROW(db.insert(db.generate_entity_id(), writer_id, pub_throughput_sample), BadParameter);

    SubscriptionThroughputSample sub_throughput_sample;
    ASSERT_THROW(db.insert(db.generate_entity_id(), reader_id, sub_throughput_sample), BadParameter);

    RtpsPacketsSentSample rtps_packets_sent_sample;
    ASSERT_THROW(db.insert(db.generate_entity_id(), writer_id, rtps_packets_sent_sample), BadParameter);

    RtpsBytesSentSample rtps_bytes_sent_sample;
    ASSERT_THROW(db.insert(db.generate_entity_id(), writer_id, rtps_bytes_sent_sample), BadParameter);

    RtpsBytesLostSample rtps_packets_lost_sample;
    ASSERT_THROW(db.insert(db.generate_entity_id(), writer_id, rtps_packets_lost_sample), BadParameter);

    RtpsBytesLostSample rtps_bytes_lost_sample;
    ASSERT_THROW(db.insert(db.generate_entity_id(), writer_id, rtps_bytes_lost_sample), BadParameter);

    ResentDataSample resent_data_sample;
    ASSERT_THROW(db.insert(db.generate_entity_id(), writer_id, resent_data_sample), BadParameter);

    HeartbeatCountSample heartbeat_count_sample;
    ASSERT_THROW(db.insert(db.generate_entity_id(), writer_id, heartbeat_count_sample), BadParameter);

    AcknackCountSample acknack_count_sample;
    ASSERT_THROW(db.insert(db.generate_entity_id(), reader_id, acknack_count_sample), BadParameter);

    NackfragCountSample nackfrag_count_sample;
    ASSERT_THROW(db.insert(db.generate_entity_id(), reader_id, nackfrag_count_sample), BadParameter);

    GapCountSample gap_count_sample;
    ASSERT_THROW(db.insert(db.generate_entity_id(), writer_id, gap_count_sample), BadParameter);

    DataCountSample data_count_sample;
    ASSERT_THROW(db.insert(db.generate_entity_id(), writer_id, data_count_sample), BadParameter);

    PdpCountSample pdp_count_sample;
    ASSERT_THROW(db.insert(db.generate_entity_id(), participant_id, pdp_count_sample), BadParameter);

    EdpCountSample edp_count_sample;
    ASSERT_THROW(db.insert(db.generate_entity_id(), participant_id, edp_count_sample), BadParameter);

    DiscoveryTimeSample discovery_time_sample;
    ASSERT_THROW(db.insert(db.generate_entity_id(), participant_id, discovery_time_sample), BadParameter);

    SampleDatasCountSample sample;
    ASSERT_THROW(db.insert(db.generate_entity_id(), writer_id, sample), BadParameter);
}

TEST_F(database_tests, get_entity_host)
{
    auto local_host = db.get_entity(host_id);
    ASSERT_EQ(local_host.get(), host.get());
}

TEST_F(database_tests, get_entity_process)
{
    auto local_process = db.get_entity(process_id);
    ASSERT_EQ(local_process.get(), process.get());
}

TEST_F(database_tests, get_entity_user)
{
    auto local_user = db.get_entity(user_id);
    ASSERT_EQ(local_user.get(), user.get());
}

TEST_F(database_tests, get_entity_domain)
{
    auto local_domain = db.get_entity(domain_id);
    ASSERT_EQ(local_domain.get(), domain.get());
}

TEST_F(database_tests, get_entity_topic)
{
    auto local_topic = db.get_entity(topic_id);
    ASSERT_EQ(local_topic.get(), topic.get());
}

TEST_F(database_tests, get_entity_participant)
{
    auto local_participant = db.get_entity(participant_id);
    ASSERT_EQ(local_participant.get(), participant.get());
}

TEST_F(database_tests, get_entity_datareader)
{
    auto local_reader = db.get_entity(reader_id);
    ASSERT_EQ(local_reader.get(), reader.get());
}

TEST_F(database_tests, get_entity_datawriter)
{
    auto local_writer = db.get_entity(writer_id);
    ASSERT_EQ(local_writer.get(), writer.get());
}

TEST_F(database_tests, get_entity_locator)
{
    auto local_reader_locator = db.get_entity(reader_locator->id);
    ASSERT_EQ(local_reader_locator.get(), reader_locator.get());
    auto local_writer_locator = db.get_entity(writer_locator->id);
    ASSERT_EQ(local_writer_locator.get(), writer_locator.get());
}

TEST_F(database_tests, get_entity_no_existing)
{
    ASSERT_THROW(db.get_entity(EntityId()), BadParameter);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
