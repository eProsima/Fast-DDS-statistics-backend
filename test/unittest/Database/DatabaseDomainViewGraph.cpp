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
#include <topic_types/types.h>
#include <TestUtils.hpp>

using namespace eprosima::fastdds::statistics;
using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;
using namespace eprosima::fastrtps::rtps;

using StatisticsData = eprosima::fastdds::statistics::Data;

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
        domain_value = 33;
        domain_name = std::to_string(domain_value);
        domain = std::make_shared<database::Domain>(domain_name);
        database.insert(domain);
    }

    EntityId insert_participant(
        const std::string& participant_name,
        const std::string& participant_guid_str,
        const details::fragile_ptr<Domain>& domain)
    {
        Qos participant_qos;
        participant_qos["_"] = "";

        auto participant = std::make_shared<database::DomainParticipant>(
                participant_name,
                participant_qos,
                participant_guid_str,
                std::shared_ptr<database::Process>(),
                domain,
                EntityStatus::OK,
                AppId::UNKNOWN,
                "");

        EntityId participant_id = database.insert(participant);

        return participant_id;
    }

    uint32_t domain_value;
    std::string domain_name;

    std::shared_ptr<eprosima::statistics_backend::database::Domain> domain;

};

TEST_F(database_domain_view_graph_tests, two_participants)
{

    EntityId host_id;
    std::string host_name = "eprosima-host";
    std::shared_ptr<eprosima::statistics_backend::database::Host> host = std::make_shared<database::Host>(host_name);

    EntityId user_id;
    std::string user_name = "eprosima";
    std::shared_ptr<eprosima::statistics_backend::database::User> user = std::make_shared<database::User>(user_name, host);

    EntityId process_id_1;
    std::string process_name_1 = "1234";
    std::shared_ptr<eprosima::statistics_backend::database::Process> process_1 = std::make_shared<database::Process>(process_name_1, process_name_1, user);

    EntityId participant_id_1;
    std::string participant_name_1 = "participant_1";
    std::string participant_guid_str_1 = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";

    EntityId process_id_2;
    std::string process_name_2 = "1235";
    std::shared_ptr<eprosima::statistics_backend::database::Process> process_2 = std::make_shared<database::Process>(process_name_2, process_name_2, user);

    EntityId participant_id_2;
    std::string participant_name_2 = "participant_2";
    std::string participant_guid_str_2 = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c2";

    nlohmann::json json_graph;

    // Add host, user, processes and participants
    {
        // Insert host and user
        host_id = database.insert(host);
        user_id = database.insert(user);   

        // Insert participant_1 from process_1
        process_id_1 = database.insert(process_1);       
        participant_id_1 = insert_participant(participant_name_1, participant_guid_str_1, domain);
        database.link_participant_with_process(participant_id_1, process_id_1); 
        database.add_participant_to_graph(domain->id, host_id, user_id, process_id_1, participant_id_1);

        // Insert participant_2 from process_2
        process_id_2 = database.insert(process_2);
        participant_id_2 = insert_participant(participant_name_2, participant_guid_str_2, domain);
        database.link_participant_with_process(participant_id_2, process_id_2);
        database.add_participant_to_graph(domain->id, host_id, user_id, process_id_2, participant_id_2);
        
        // Load reference graph
        load_file(DOMAIN_VIEW_GRAPH_PARTICIPANTS_DUMP_FILE, json_graph);

        ASSERT_EQ(json_graph, database.get_domain_view_graph(0));

    }

    {
        load_file(DOMAIN_VIEW_GRAPH_EMPTY_DOMAIN_DUMP_FILE, json_graph);
        database.change_entity_status(participant_id_1, false);
        database.delete_participant_from_graph(domain->id, host_id, user_id, process_id_1, participant_id_1);
        database.change_entity_status(participant_id_2, false);
        database.delete_participant_from_graph(domain->id, host_id, user_id, process_id_2, participant_id_2);

        ASSERT_EQ(json_graph, database.get_domain_view_graph(0));
    }
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
