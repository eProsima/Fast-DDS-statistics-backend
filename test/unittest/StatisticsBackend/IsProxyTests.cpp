// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/topic_types/monitorservice_types.hpp>

#include <database/database.hpp>
#include <database/entities.hpp>
#include <database/samples.hpp>

#include <DatabaseUtils.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

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

// Test proxy member of proxy participant
TEST(is_proxy_tests, is_proxy_entity)
{
    /* Insert a host, user, and process */
    details::StatisticsBackendData::get_instance()->database_.reset(new Database);
    auto host = std::make_shared<Host>("test_host");
    details::StatisticsBackendData::get_instance()->database_->insert(host);
    auto user = std::make_shared<User>("test_user", host);
    details::StatisticsBackendData::get_instance()->database_->insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    details::StatisticsBackendData::get_instance()->database_->insert(process);

    /* Insert a domain */
    auto domain = std::make_shared<Domain>("test_domain", 0);
    details::StatisticsBackendData::get_instance()->database_->insert(domain);

    /* Insert a DomainParticipant */
    std::string part_name = "test_participant";
    std::string part_guid = "01.02.03.04";
    auto participant = std::make_shared<DomainParticipant>(
        part_name, test_qos, part_guid, process, domain,
        StatusLevel::OK_STATUS, AppId::UNKNOWN, "", DiscoverySource::PROXY);
    auto participant_id = details::StatisticsBackendData::get_instance()->database_->insert(participant);

    // Check that the participant is proxy_
    ASSERT_TRUE(StatisticsBackend::is_proxy(participant_id));
}

// Test proxy member of non-proxy participant
TEST(is_proxy_tests, is_not_proxy_entity)
{
        /* Insert a host, user, and process */
    DataBaseTest db;
    auto host = std::make_shared<Host>("test_host");
    details::StatisticsBackendData::get_instance()->database_->insert(host);
    auto user = std::make_shared<User>("test_user", host);
    details::StatisticsBackendData::get_instance()->database_->insert(user);
    auto process = std::make_shared<Process>("test_process", "test_pid", user);
    details::StatisticsBackendData::get_instance()->database_->insert(process);

    /* Insert a domain */
    auto domain = std::make_shared<Domain>("test_domain", 0);
    details::StatisticsBackendData::get_instance()->database_->insert(domain);

    /* Insert a DomainParticipant */
    std::string part_name = "test_participant";
    std::string part_guid = "01.02.03.04";
    auto participant = std::make_shared<DomainParticipant>(
        part_name, test_qos, part_guid, process, domain,
        StatusLevel::OK_STATUS, AppId::UNKNOWN, "", DiscoverySource::DISCOVERY);
    auto participant_id = details::StatisticsBackendData::get_instance()->database_->insert(participant);

    // Check that the participant is proxy_
    ASSERT_FALSE(StatisticsBackend::is_proxy(participant_id));
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
