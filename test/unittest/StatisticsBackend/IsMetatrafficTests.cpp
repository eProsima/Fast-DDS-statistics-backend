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

#include <database/entities.hpp>
#include <StatisticsBackend.hpp>
#include <StatisticsBackendData.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

constexpr const char* PID_DEFAULT = "36000";
constexpr const char* DATA_TYPE_DEFAULT = "data_type";
#define QOS_DEFAULT Qos(R"({"qos":"empty"})"_json)


#define LOCATOR_DEFAULT_NAME(x) "locator_" + std::to_string(x)
#define HOST_DEFAULT_NAME(x) "host_" + std::to_string(x)
#define USER_DEFAULT_NAME(x) "user_" + std::to_string(x)
#define PROCESS_DEFAULT_NAME(x) "process_" + std::to_string(x)
#define DOMAIN_DEFAULT_NAME(x) "domain_name_" + std::to_string(x)
#define ALIAS_DEFAULT_NAME(x) "domain_alias_" + std::to_string(x)
#define NON_METATRAFFIC_TOPIC_NAMES {"topic_0", "rt/chatter", "Square"}
#define METATRAFFIC_TOPIC_NAMES {"ros_discovery_info", "rt/rosout", "_fastdds_statistics_network_latency"}
#define PARTICIPANT_DEFAULT_NAME(x) "participant_" + std::to_string(x)
#define DATAWRITER_DEFAULT_NAME(x) "datawriter_" + std::to_string(x)
#define DATAREADER_DEFAULT_NAME(x) "datareader_" + std::to_string(x)

#define GUID_DEFAULT(x) "01.0f.00.00.00.00.00.00.00.00.00.0" + std::to_string(x) + "|0.0.0.0"

// Test metatraffic member of non-metatraffic topics
TEST(is_metatraffic_tests, non_metatraffic_topic)
{
    // reset database
    details::StatisticsBackendData::get_instance()->database_.reset(new Database);

    std::shared_ptr<Domain> domain = std::make_shared<Domain>(std::string(DOMAIN_DEFAULT_NAME(0)), 0);
    domain->alias = ALIAS_DEFAULT_NAME(0);
    details::StatisticsBackendData::get_instance()->database_->insert(domain);

    for (auto topic_name : NON_METATRAFFIC_TOPIC_NAMES)
    {
        // non-metatraffic topic
        std::shared_ptr<Topic> topic = std::make_shared<Topic>(topic_name, DATA_TYPE_DEFAULT, domain);
        details::StatisticsBackendData::get_instance()->database_->insert(topic);
        ASSERT_EQ(StatisticsBackend::is_metatraffic(topic->id), topic->metatraffic);
        ASSERT_FALSE(StatisticsBackend::is_metatraffic(topic->id));
    }
}

// Test metatraffic member of metatraffic topics
TEST(is_metatraffic_tests, metatraffic_topic)
{
    // reset database
    details::StatisticsBackendData::get_instance()->database_.reset(new Database);

    std::shared_ptr<Domain> domain = std::make_shared<Domain>(std::string(DOMAIN_DEFAULT_NAME(0)), 0);
    domain->alias = ALIAS_DEFAULT_NAME(0);
    details::StatisticsBackendData::get_instance()->database_->insert(domain);

    for (auto topic_name : METATRAFFIC_TOPIC_NAMES)
    {
        // metatraffic topic
        std::shared_ptr<Topic> topic = std::make_shared<Topic>(topic_name, DATA_TYPE_DEFAULT, domain);
        details::StatisticsBackendData::get_instance()->database_->insert(topic);
        ASSERT_EQ(StatisticsBackend::is_metatraffic(topic->id), topic->metatraffic);
        ASSERT_TRUE(StatisticsBackend::is_metatraffic(topic->id));
    }
}

// Test metatraffic member of non-metatraffic endpoints
TEST(is_metatraffic_tests, non_metatraffic_endpoint)
{
    // reset database
    details::StatisticsBackendData::get_instance()->database_.reset(new Database);

    std::shared_ptr<Domain> domain = std::make_shared<Domain>(std::string(DOMAIN_DEFAULT_NAME(0)), 0);
    domain->alias = ALIAS_DEFAULT_NAME(0);
    details::StatisticsBackendData::get_instance()->database_->insert(domain);

    int idx = 0;
    for (auto topic_name : NON_METATRAFFIC_TOPIC_NAMES)
    {
        // non-metatraffic topic
        std::shared_ptr<Topic> topic = std::make_shared<Topic>(topic_name, DATA_TYPE_DEFAULT, domain);
        details::StatisticsBackendData::get_instance()->database_->insert(topic);
        std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(std::string(
                            PARTICIPANT_DEFAULT_NAME(idx)), QOS_DEFAULT, GUID_DEFAULT(idx), nullptr, domain);
        details::StatisticsBackendData::get_instance()->database_->insert(participant);
        std::shared_ptr<Locator> locator = std::make_shared<Locator>(std::string(LOCATOR_DEFAULT_NAME(idx)));
        details::StatisticsBackendData::get_instance()->database_->insert(locator);
        std::shared_ptr<DataWriter> dw = std::make_shared<DataWriter>(std::string(
                            DATAWRITER_DEFAULT_NAME(idx)), QOS_DEFAULT, GUID_DEFAULT(idx), participant, topic);
        dw->locators[locator->id] = locator;
        details::StatisticsBackendData::get_instance()->database_->insert(dw);
        ASSERT_EQ(StatisticsBackend::is_metatraffic(dw->id), dw->metatraffic);
        ASSERT_FALSE(StatisticsBackend::is_metatraffic(dw->id));
        idx++;
    }
}

// Test metatraffic member of metatraffic endpoints
TEST(is_metatraffic_tests, metatraffic_endpoint)
{
    // reset database
    details::StatisticsBackendData::get_instance()->database_.reset(new Database);

    std::shared_ptr<Domain> domain = std::make_shared<Domain>(std::string(DOMAIN_DEFAULT_NAME(0)), 0);
    domain->alias = ALIAS_DEFAULT_NAME(0);
    details::StatisticsBackendData::get_instance()->database_->insert(domain);

    int idx = 0;
    for (auto topic_name : METATRAFFIC_TOPIC_NAMES)
    {
        // metatraffic topic
        std::shared_ptr<Topic> topic = std::make_shared<Topic>(topic_name, DATA_TYPE_DEFAULT, domain);
        details::StatisticsBackendData::get_instance()->database_->insert(topic);
        std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(std::string(
                            PARTICIPANT_DEFAULT_NAME(idx)), QOS_DEFAULT, GUID_DEFAULT(idx), nullptr, domain);
        details::StatisticsBackendData::get_instance()->database_->insert(participant);
        std::shared_ptr<Locator> locator = std::make_shared<Locator>(std::string(LOCATOR_DEFAULT_NAME(idx)));
        details::StatisticsBackendData::get_instance()->database_->insert(locator);
        std::shared_ptr<DataWriter> dw = std::make_shared<DataWriter>(std::string(
                            DATAWRITER_DEFAULT_NAME(idx)), QOS_DEFAULT, GUID_DEFAULT(idx), participant, topic);
        dw->locators[locator->id] = locator;
        details::StatisticsBackendData::get_instance()->database_->insert(dw);
        ASSERT_EQ(StatisticsBackend::is_metatraffic(dw->id), dw->metatraffic);
        ASSERT_TRUE(StatisticsBackend::is_metatraffic(dw->id));
        idx++;
    }
}

// Test metatraffic member of other entity kinds
TEST(is_metatraffic_tests, other_entity_kinds)
{
    // reset database
    details::StatisticsBackendData::get_instance()->database_.reset(new Database);

    std::shared_ptr<Host> host = std::make_shared<Host>(std::string(HOST_DEFAULT_NAME(0)));
    details::StatisticsBackendData::get_instance()->database_->insert(host);
    ASSERT_EQ(StatisticsBackend::is_metatraffic(host->id), host->metatraffic);
    ASSERT_FALSE(StatisticsBackend::is_metatraffic(host->id));
    std::shared_ptr<User> user = std::make_shared<User>(std::string(USER_DEFAULT_NAME(0)), host);
    details::StatisticsBackendData::get_instance()->database_->insert(user);
    ASSERT_EQ(StatisticsBackend::is_metatraffic(user->id), user->metatraffic);
    ASSERT_FALSE(StatisticsBackend::is_metatraffic(user->id));
    std::shared_ptr<Process> process = std::make_shared<Process>(std::string(PROCESS_DEFAULT_NAME(
                        0)), PID_DEFAULT, user);
    details::StatisticsBackendData::get_instance()->database_->insert(process);
    ASSERT_EQ(StatisticsBackend::is_metatraffic(process->id), process->metatraffic);
    ASSERT_FALSE(StatisticsBackend::is_metatraffic(process->id));
    std::shared_ptr<Domain> domain = std::make_shared<Domain>(std::string(DOMAIN_DEFAULT_NAME(0)), 0);
    domain->alias = ALIAS_DEFAULT_NAME(0);
    details::StatisticsBackendData::get_instance()->database_->insert(domain);
    ASSERT_EQ(StatisticsBackend::is_metatraffic(domain->id), domain->metatraffic);
    ASSERT_FALSE(StatisticsBackend::is_metatraffic(domain->id));
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(std::string(
                        PARTICIPANT_DEFAULT_NAME(0)), QOS_DEFAULT, GUID_DEFAULT(0), nullptr, domain);
    details::StatisticsBackendData::get_instance()->database_->insert(participant);
    ASSERT_EQ(StatisticsBackend::is_metatraffic(participant->id), participant->metatraffic);
    ASSERT_FALSE(StatisticsBackend::is_metatraffic(participant->id));
    std::shared_ptr<Locator> locator = std::make_shared<Locator>(std::string(LOCATOR_DEFAULT_NAME(0)));
    details::StatisticsBackendData::get_instance()->database_->insert(locator);
    ASSERT_EQ(StatisticsBackend::is_metatraffic(locator->id), locator->metatraffic);
    ASSERT_FALSE(StatisticsBackend::is_metatraffic(locator->id));
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
