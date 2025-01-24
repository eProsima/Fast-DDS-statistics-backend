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

#include <iostream>
#include <functional>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <database/database.hpp>
#include <database/database_queue.hpp>

#include <fastdds_statistics_backend/topic_types/monitorservice_types.hpp>
#include <fastdds_statistics_backend/topic_types/types.hpp>

#include <TestUtils.hpp>

using namespace eprosima::fastdds::statistics;
using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;
using namespace eprosima::fastdds::rtps;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Throw;
using ::testing::AnyNumber;


class database_domain_view_graph_tests : public ::testing::Test
{

public:

    Database database;
    DatabaseEntityQueue entity_queue;

    database_domain_view_graph_tests()
        : entity_queue(&database)
    {
    }

    void SetUp()
    {
        std::string domain_name = "33";
        domain = std::make_shared<database::Domain>(domain_name);

        domain_id = database.insert(domain);
        database.init_domain_view_graph(domain_name, domain_id);

        topic = std::make_shared<database::Topic>(
            "HelloWorld",
            "HelloWorld",
            domain);

        locator_1 = std::make_shared<database::Locator>("UDPv4:[127.0.0.1]:15662");
        locator_2 = std::make_shared<database::Locator>("UDPv4:[127.0.0.1]:15663");

        entity_qos["_"] = "";

        host = std::make_shared<database::Host>("eprosima-host");

        user = std::make_shared<database::User>("eprosima", host);

        process_1 = std::make_shared<database::Process>("1234", "1234", user);

        participant_1 = std::make_shared<database::DomainParticipant>(
            "participant_1",
            entity_qos,
            "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1",
            std::shared_ptr<database::Process>(),
            domain,
            StatusLevel::OK_STATUS,
            AppId::UNKNOWN,
            "");

        datawriter_1 = std::make_shared<database::DataWriter>(
            "publisher",
            entity_qos,
            "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c2",
            participant_1,
            topic,
            StatusLevel::OK_STATUS,
            AppId::UNKNOWN,
            "");

        process_2 = std::make_shared<database::Process>("1235", "1235", user);

        participant_2 = std::make_shared<database::DomainParticipant>(
            "participant_2",
            entity_qos,
            "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c3",
            std::shared_ptr<database::Process>(),
            domain,
            StatusLevel::OK_STATUS,
            AppId::UNKNOWN,
            "");

        datareader_2 = std::make_shared<database::DataReader>(
            "subscriber",
            entity_qos,
            "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c4",
            participant_2,
            topic,
            StatusLevel::OK_STATUS,
            AppId::UNKNOWN,
            "");


    }

    EntityId domain_id;
    std::shared_ptr<eprosima::statistics_backend::database::Domain> domain;

    Qos entity_qos;

    EntityId host_id;
    std::shared_ptr<eprosima::statistics_backend::database::Host> host;

    EntityId user_id;
    std::shared_ptr<eprosima::statistics_backend::database::User> user;

    EntityId topic_id;
    std::shared_ptr<eprosima::statistics_backend::database::Topic> topic;

    EntityId process_id_1;
    std::shared_ptr<eprosima::statistics_backend::database::Process> process_1;

    EntityId participant_id_1;
    std::shared_ptr<eprosima::statistics_backend::database::DomainParticipant> participant_1;

    EntityId datawriter_id_1;
    std::shared_ptr<eprosima::statistics_backend::database::DataWriter> datawriter_1;

    EntityId process_id_2;
    std::shared_ptr<eprosima::statistics_backend::database::Process> process_2;

    EntityId participant_id_2;
    std::shared_ptr<eprosima::statistics_backend::database::DomainParticipant> participant_2;

    EntityId datareader_id_2;
    std::shared_ptr<eprosima::statistics_backend::database::DataReader> datareader_2;

    EntityId locator_id_1;
    std::shared_ptr<database::Locator> locator_1;

    EntityId locator_id_2;
    std::shared_ptr<database::Locator> locator_2;

};

// Complete test with two participants (datawriter and datareader),
// testing add to graph and delete from graph funcionalities
TEST_F(database_domain_view_graph_tests, complete_with_two_participants)
{
    nlohmann::json json_graph;

    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_physical_entity_discovery(_, _, _))
            .Times(AnyNumber());
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_domain_entity_discovery(_, _, _, _))
            .Times(AnyNumber());

    // Add host, user, processes, participants, topic and endpoints. On each insertion add/delete elements.
    {
        // Insert host and user
        host_id = database.insert(host);
        user_id = database.insert(user);

        // Insert participant_1 from process_1
        process_id_1 = database.insert(process_1);
        participant_id_1 = database.insert(participant_1);
        database.link_participant_with_process(participant_id_1, process_id_1);

        // Add participant to graph
        database.update_participant_in_graph(domain_id, host_id, user_id, process_id_1, participant_id_1);

        // Insert topic
        topic_id = database.insert(topic);

        // Insert locator
        locator_id_1 = database.insert(locator_1);
        datawriter_1->locators[locator_id_1] = locator_1;

        // Insert datawriter_1 from participant_1
        datawriter_id_1 = database.insert(datawriter_1);

        // Add endpoint and topic to graph
        database.update_endpoint_in_graph(domain_id, participant_id_1, topic_id, datawriter_id_1);

        // Insert participant_2 from process_2
        process_id_2 = database.insert(process_2);
        participant_id_2 = database.insert(participant_2);
        database.link_participant_with_process(participant_id_2, process_id_2);
        database.update_participant_in_graph(domain_id, host_id, user_id, process_id_2, participant_id_2);

        // Insert locator
        locator_id_2 = database.insert(locator_2);
        datareader_2->locators[locator_id_2] = locator_2;

        //Insert datareader_2 from participant_2
        datareader_id_2 = database.insert(datareader_2);

        // Add endpoint and topic to graph
        database.update_endpoint_in_graph(domain_id, participant_id_2, topic_id, datareader_id_2);

        // Load reference graph
        load_file(DOMAIN_VIEW_GRAPH_TWO_PARTICIPANTS_DUMP_FILE, json_graph);

        ASSERT_EQ(json_graph, database.get_domain_view_graph(0));

    }
    // No Update
    {
        database.update_participant_in_graph(domain_id, host_id, user_id, process_id_1, participant_id_1);
        database.update_endpoint_in_graph(domain_id, participant_id_1, topic_id, datawriter_id_1);

        // Load reference graph
        load_file(DOMAIN_VIEW_GRAPH_TWO_PARTICIPANTS_DUMP_FILE, json_graph);

        ASSERT_EQ(json_graph, database.get_domain_view_graph(0));

    }
    // Update entities alias
    {
        std::shared_ptr<const database::Entity> participant_const_entity_modified =
                database.get_entity(participant_id_1);
        std::shared_ptr<database::Entity> participant_entity_modified = std::const_pointer_cast<database::Entity>(
            participant_const_entity_modified);
        participant_entity_modified->alias = "participant_1_modified";
        database.update_graph_on_updated_entity(domain_id, participant_id_1);

        std::shared_ptr<const database::Entity> host_const_entity_modified = database.get_entity(host_id);
        std::shared_ptr<database::Entity> host_entity_modified = std::const_pointer_cast<database::Entity>(
            host_const_entity_modified);
        host_entity_modified->alias = "eprosima-host_modified";
        database.update_graph_on_updated_entity(domain_id, host_id);

        std::shared_ptr<const database::Entity> user_const_entity_modified = database.get_entity(user_id);
        std::shared_ptr<database::Entity> user_entity_modified = std::const_pointer_cast<database::Entity>(
            user_const_entity_modified);
        user_entity_modified->alias = "eprosima_modified";
        database.update_graph_on_updated_entity(domain_id, user_id);

        std::shared_ptr<const database::Entity> process_const_entity_modified = database.get_entity(process_id_1);
        std::shared_ptr<database::Entity> process_entity_modified = std::const_pointer_cast<database::Entity>(
            process_const_entity_modified);
        process_entity_modified->alias = "1234_modified";
        database.update_graph_on_updated_entity(domain_id, process_id_1);

        std::shared_ptr<const database::Entity> topic_const_entity_modified = database.get_entity(topic_id);
        std::shared_ptr<database::Entity> topic_entity_modified = std::const_pointer_cast<database::Entity>(
            topic_const_entity_modified);
        topic_entity_modified->alias = "HelloWorld_modified";
        database.update_graph_on_updated_entity(domain_id, topic_id);

        std::shared_ptr<const database::Entity> datawriter_const_entity_modified = database.get_entity(datawriter_id_1);
        std::shared_ptr<database::Entity> datawriter_entity_modified = std::const_pointer_cast<database::Entity>(
            datawriter_const_entity_modified);
        datawriter_entity_modified->alias = "publisher_modified";
        database.update_graph_on_updated_entity(domain_id, datawriter_id_1);

        std::shared_ptr<const database::Entity> datareader_const_entity_modified = database.get_entity(datareader_id_2);
        std::shared_ptr<database::Entity> datareader_entity_modified = std::const_pointer_cast<database::Entity>(
            datareader_const_entity_modified);
        datareader_entity_modified->alias = "subscriber_modified";
        database.update_graph_on_updated_entity(domain_id, datareader_id_2);

        // Load reference graph
        load_file(DOMAIN_VIEW_GRAPH_UPDATED_ENTITIES_DUMP_FILE, json_graph);

        ASSERT_EQ(json_graph, database.get_domain_view_graph(0));

    }
    // Undiscovery one publisher
    {
        // Revert changes alias update changes
        std::shared_ptr<const database::Entity> participant_const_entity_modified =
                database.get_entity(participant_id_1);
        std::shared_ptr<database::Entity> participant_entity_modified = std::const_pointer_cast<database::Entity>(
            participant_const_entity_modified);
        participant_entity_modified->alias = "participant_1";
        database.update_graph_on_updated_entity(domain_id, participant_id_1);

        std::shared_ptr<const database::Entity> host_const_entity_modified = database.get_entity(host_id);
        std::shared_ptr<database::Entity> host_entity_modified = std::const_pointer_cast<database::Entity>(
            host_const_entity_modified);
        host_entity_modified->alias = "eprosima-host";
        database.update_graph_on_updated_entity(domain_id, host_id);

        std::shared_ptr<const database::Entity> user_const_entity_modified = database.get_entity(user_id);
        std::shared_ptr<database::Entity> user_entity_modified = std::const_pointer_cast<database::Entity>(
            user_const_entity_modified);
        user_entity_modified->alias = "eprosima";
        database.update_graph_on_updated_entity(domain_id, user_id);

        std::shared_ptr<const database::Entity> process_const_entity_modified = database.get_entity(process_id_1);
        std::shared_ptr<database::Entity> process_entity_modified = std::const_pointer_cast<database::Entity>(
            process_const_entity_modified);
        process_entity_modified->alias = "1234";
        database.update_graph_on_updated_entity(domain_id, process_id_1);

        std::shared_ptr<const database::Entity> topic_const_entity_modified = database.get_entity(topic_id);
        std::shared_ptr<database::Entity> topic_entity_modified = std::const_pointer_cast<database::Entity>(
            topic_const_entity_modified);
        topic_entity_modified->alias = "HelloWorld";
        database.update_graph_on_updated_entity(domain_id, topic_id);

        std::shared_ptr<const database::Entity> datawriter_const_entity_modified = database.get_entity(datawriter_id_1);
        std::shared_ptr<database::Entity> datawriter_entity_modified = std::const_pointer_cast<database::Entity>(
            datawriter_const_entity_modified);
        datawriter_entity_modified->alias = "publisher";
        database.update_graph_on_updated_entity(domain_id, datawriter_id_1);

        std::shared_ptr<const database::Entity> datareader_const_entity_modified = database.get_entity(datareader_id_2);
        std::shared_ptr<database::Entity> datareader_entity_modified = std::const_pointer_cast<database::Entity>(
            datareader_const_entity_modified);
        datareader_entity_modified->alias = "subscriber";
        database.update_graph_on_updated_entity(domain_id, datareader_id_2);

        // Change status
        database.change_entity_status(datawriter_id_1, false);
        database.update_endpoint_in_graph(domain_id, participant_id_1, topic_id, datawriter_id_1);

        // Load reference graph
        load_file(DOMAIN_VIEW_GRAPH_UNDISCOVER_ENDPOINT_DUMP_FILE, json_graph);

        ASSERT_EQ(json_graph, database.get_domain_view_graph(0));
    }
    // Undiscovery first participant
    {

        database.change_entity_status(participant_id_1, false);
        database.update_participant_in_graph(domain->id, host_id, user_id, process_id_1, participant_id_1);

        // Load reference graph
        load_file(DOMAIN_VIEW_GRAPH_UNDISCOVER_PARTICIPANT_DUMP_FILE, json_graph);

        ASSERT_EQ(json_graph, database.get_domain_view_graph(0));
    }
    // Undiscovery second participant
    {
        database.change_entity_status(participant_id_2, false);
        database.update_endpoint_in_graph(domain_id, participant_id_2, topic_id, datareader_id_2);
        database.update_participant_in_graph(domain->id, host_id, user_id, process_id_2, participant_id_2);

        // Load reference graph
        load_file(DOMAIN_VIEW_GRAPH_EMPTY_DOMAIN_DUMP_FILE, json_graph);

        ASSERT_EQ(json_graph, database.get_domain_view_graph(0));
    }
}

// Test host database insert failure and endpoint undiscovery
TEST_F(database_domain_view_graph_tests, host_insert_failure)
{
    nlohmann::json json_graph;

    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_physical_entity_discovery(_, _, _))
            .Times(AnyNumber());
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_domain_entity_discovery(_, _, _, _))
            .Times(AnyNumber());

    // Add participant, topic and endpoints. On each insertion add/delete elements.
    {
        // Insert participant_1 from process_1
        participant_id_1 = database.insert(participant_1);

        // Add participant to graph
        database.update_participant_in_graph(domain_id, EntityId(), EntityId(), EntityId(), participant_id_1);

        // Insert topic
        topic_id = database.insert(topic);

        // Insert locator
        locator_id_1 = database.insert(locator_1);
        datawriter_1->locators[locator_id_1] = locator_1;

        // Insert datawriter_1 from participant_1
        datawriter_id_1 = database.insert(datawriter_1);

        // Add endpoint and topic to graph
        database.update_endpoint_in_graph(domain_id, participant_id_1, topic_id, datawriter_id_1);

        // Load reference graph
        load_file(DOMAIN_VIEW_GRAPH_HOST_INSERT_FAILURE_DUMP_FILE, json_graph);

        ASSERT_EQ(json_graph, database.get_domain_view_graph(0));

    }
    {
        // Delete topic with no endpoint in the graph
        database.change_entity_status(datawriter_id_1, false);
        database.update_endpoint_in_graph(domain_id, participant_id_1, topic_id, datawriter_id_1);

        // Load reference graph
        load_file(DOMAIN_VIEW_GRAPH_EMPTY_DOMAIN_DUMP_FILE, json_graph);

        ASSERT_EQ(json_graph, database.get_domain_view_graph(0));
    }

}

// Test user database insert failure
TEST_F(database_domain_view_graph_tests, user_insert_failure)
{
    nlohmann::json json_graph;

    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_physical_entity_discovery(_, _, _))
            .Times(AnyNumber());
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_domain_entity_discovery(_, _, _, _))
            .Times(AnyNumber());

    // Add host, participants, topic and endpoints. On each insertion add/delete elements.

    // Host insert
    host_id = database.insert(host);

    // Insert participant_1 from process_1
    participant_id_1 = database.insert(participant_1);

    // Add participant to graph
    database.update_participant_in_graph(domain_id, host_id, EntityId(), EntityId(), participant_id_1);

    // Insert topic
    topic_id = database.insert(topic);

    // Insert locator
    locator_id_1 = database.insert(locator_1);
    datawriter_1->locators[locator_id_1] = locator_1;

    // Insert datawriter_1 from participant_1
    datawriter_id_1 = database.insert(datawriter_1);

    // Add endpoint and topic to graph
    database.update_endpoint_in_graph(domain_id, participant_id_1, topic_id, datawriter_id_1);

    // Load reference graph
    load_file(DOMAIN_VIEW_GRAPH_USER_INSERT_FAILURE_DUMP_FILE, json_graph);

    ASSERT_EQ(json_graph, database.get_domain_view_graph(0));
}

// Test process database insert failure
TEST_F(database_domain_view_graph_tests, process_insert_failure)
{
    nlohmann::json json_graph;

    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_physical_entity_discovery(_, _, _))
            .Times(AnyNumber());
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_domain_entity_discovery(_, _, _, _))
            .Times(AnyNumber());

    // Add host, user, processes, participants and endpoints. On each insertion add/delete elements.

    // Host insert
    host_id = database.insert(host);
    user_id = database.insert(user);

    // Insert participant_1 from process_1
    participant_id_1 = database.insert(participant_1);

    // Add participant to graph
    database.update_participant_in_graph(domain_id, host_id, user_id, EntityId(), participant_id_1);

    // Insert topic
    topic_id = database.insert(topic);

    // Insert locator
    locator_id_1 = database.insert(locator_1);
    datawriter_1->locators[locator_id_1] = locator_1;

    // Insert datawriter_1 from participant_1
    datawriter_id_1 = database.insert(datawriter_1);

    // Add endpoint and topic to graph
    database.update_endpoint_in_graph(domain_id, participant_id_1, topic_id, datawriter_id_1);

    // Load reference graph
    load_file(DOMAIN_VIEW_GRAPH_PROCESS_INSERT_FAILURE_DUMP_FILE, json_graph);

    ASSERT_EQ(json_graph, database.get_domain_view_graph(0));
}

// Test regenerate graph functionality after some undiscoveries
TEST_F(database_domain_view_graph_tests, regenerate_graph_functionality)
{
    nlohmann::json json_graph;

    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_physical_entity_discovery(_, _, _))
            .Times(AnyNumber());
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_domain_entity_discovery(_, _, _, _))
            .Times(AnyNumber());

    // Add host, user, processes, participants, topic and endpoints. On each insertion add/delete elements.

    // Insert host and user
    host_id = database.insert(host);
    user_id = database.insert(user);

    // Insert participant_1 from process_1
    process_id_1 = database.insert(process_1);
    participant_id_1 = database.insert(participant_1);
    database.link_participant_with_process(participant_id_1, process_id_1);

    // Add participant to graph
    database.update_participant_in_graph(domain_id, host_id, user_id, process_id_1, participant_id_1);

    // Insert topic
    topic_id = database.insert(topic);

    // Insert locator
    locator_id_1 = database.insert(locator_1);
    datawriter_1->locators[locator_id_1] = locator_1;

    // Insert datawriter_1 from participant_1
    datawriter_id_1 = database.insert(datawriter_1);

    // Add endpoint and topic to graph
    database.update_endpoint_in_graph(domain_id, participant_id_1, topic_id, datawriter_id_1);

    // Insert participant_2 from process_2
    process_id_2 = database.insert(process_2);
    participant_id_2 = database.insert(participant_2);
    database.link_participant_with_process(participant_id_2, process_id_2);
    database.update_participant_in_graph(domain_id, host_id, user_id, process_id_2, participant_id_2);

    // Insert locator
    locator_id_2 = database.insert(locator_2);
    datareader_2->locators[locator_id_2] = locator_2;

    //Insert datareader_2 from participant_2
    datareader_id_2 = database.insert(datareader_2);

    // Add endpoint and topic to graph
    database.update_endpoint_in_graph(domain_id, participant_id_2, topic_id, datareader_id_2);

    // Undiscovery datawriter publisher
    load_file(DOMAIN_VIEW_GRAPH_UNDISCOVER_ENDPOINT_DUMP_FILE, json_graph);
    database.change_entity_status(datawriter_id_1, false);
    database.regenerate_domain_graph(domain_id);

    ASSERT_EQ(json_graph, database.get_domain_view_graph(0));

    // Undiscovery first participant
    load_file(DOMAIN_VIEW_GRAPH_UNDISCOVER_PARTICIPANT_DUMP_FILE, json_graph);
    database.change_entity_status(participant_id_1, false);
    database.regenerate_domain_graph(domain_id);

    ASSERT_EQ(json_graph, database.get_domain_view_graph(0));

    // Undiscovery second participant
    load_file(DOMAIN_VIEW_GRAPH_EMPTY_DOMAIN_DUMP_FILE, json_graph);
    database.change_entity_status(participant_id_2, false);
    database.regenerate_domain_graph(domain_id);
    ASSERT_EQ(json_graph, database.get_domain_view_graph(0));
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
