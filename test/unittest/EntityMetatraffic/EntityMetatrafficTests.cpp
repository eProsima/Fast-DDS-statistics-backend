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

using namespace eprosima::statistics_backend::database;

constexpr const char* PID_DEFAULT = "36000";
constexpr const char* DATA_TYPE_DEFAULT = "data_type";
#define QOS_DEFAULT Qos(R"({"qos":"empty"})"_json)


#define LOCATOR_DEFAULT_NAME(x) "locator_" + std::to_string(x)
#define HOST_DEFAULT_NAME(x) "host_" + std::to_string(x)
#define USER_DEFAULT_NAME(x) "user_" + std::to_string(x)
#define PROCESS_DEFAULT_NAME(x) "process_" + std::to_string(x)
#define DOMAIN_DEFAULT_NAME(x) "12" + std::to_string(x)
#define ALIAS_DEFAULT_NAME(x) "domain_" + std::to_string(x)
#define TOPIC_DEFAULT_NAME(x) "topic_" + std::to_string(x)
#define PARTICIPANT_DEFAULT_NAME(x) "participant_" + std::to_string(x)
#define DATAWRITER_DEFAULT_NAME(x) "datawriter_" + std::to_string(x)
#define DATAREADER_DEFAULT_NAME(x) "datareader_" + std::to_string(x)

#define GUID_DEFAULT(x) "01.0f.00.00.00.00.00.00.00.00.00.0" + std::to_string(x) + "|0.0.0.0"

// Test metatraffic member of non-metatraffic topic
TEST(entitymetatraffic, non_metatraffic_topic)
{
    std::shared_ptr<Domain> domain = std::make_shared<Domain>(std::string(DOMAIN_DEFAULT_NAME(0)));
    domain->alias = ALIAS_DEFAULT_NAME(0);
    // non-metatraffic topic
    std::shared_ptr<Topic> topic = std::make_shared<Topic>(std::string(TOPIC_DEFAULT_NAME(
                        0)), DATA_TYPE_DEFAULT, domain);
    ASSERT_EQ(topic->is_metatraffic_topic(topic->name), topic->metatraffic);
    ASSERT_FALSE(topic->metatraffic);
}

// Test metatraffic member of metatraffic topic
TEST(entitymetatraffic, metatraffic_topic)
{
    std::shared_ptr<Domain> domain = std::make_shared<Domain>(std::string(DOMAIN_DEFAULT_NAME(0)));
    domain->alias = ALIAS_DEFAULT_NAME(0);
    // fastdds statistics metatraffic topic
    std::shared_ptr<Topic> topic = std::make_shared<Topic>("_fastdds_statistics_network_latency",
                    DATA_TYPE_DEFAULT, domain);
    ASSERT_EQ(topic->is_metatraffic_topic(topic->name), topic->metatraffic);
    ASSERT_TRUE(topic->metatraffic);
}

// Test metatraffic member of non-metatraffic endpoint
TEST(entitymetatraffic, non_metatraffic_endpoint)
{
    std::shared_ptr<Domain> domain = std::make_shared<Domain>(std::string(DOMAIN_DEFAULT_NAME(0)));
    domain->alias = ALIAS_DEFAULT_NAME(0);
    // non-metatraffic topic
    std::shared_ptr<Topic> topic = std::make_shared<Topic>(std::string(TOPIC_DEFAULT_NAME(
                        0)), DATA_TYPE_DEFAULT, domain);
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(std::string(
                        PARTICIPANT_DEFAULT_NAME(0)), QOS_DEFAULT, GUID_DEFAULT(0), nullptr, domain);
    std::shared_ptr<DataWriter> dw = std::make_shared<DataWriter>(std::string(
                        DATAWRITER_DEFAULT_NAME(0)), QOS_DEFAULT, GUID_DEFAULT(0), participant, topic);
    ASSERT_FALSE(dw->metatraffic);
}

// Test metatraffic member of metatraffic endpoint
TEST(entitymetatraffic, metatraffic_endpoint)
{
    std::shared_ptr<Domain> domain = std::make_shared<Domain>(std::string(DOMAIN_DEFAULT_NAME(0)));
    domain->alias = ALIAS_DEFAULT_NAME(0);
    // fastdds statistics metatraffic topic
    std::shared_ptr<Topic> topic = std::make_shared<Topic>("_fastdds_statistics_network_latency",
                    DATA_TYPE_DEFAULT, domain);
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(std::string(
                        PARTICIPANT_DEFAULT_NAME(0)), QOS_DEFAULT, GUID_DEFAULT(0), nullptr, domain);
    std::shared_ptr<DataWriter> dw = std::make_shared<DataWriter>(std::string(
                        DATAWRITER_DEFAULT_NAME(0)), QOS_DEFAULT, GUID_DEFAULT(0), participant, topic);
    ASSERT_TRUE(dw->metatraffic);
}

// Test metatraffic member of other entity kinds
TEST(entitymetatraffic, other_entity_kinds)
{
    std::shared_ptr<Host> host = std::make_shared<Host>(std::string(HOST_DEFAULT_NAME(0)));
    ASSERT_FALSE(host->metatraffic);
    std::shared_ptr<User> user = std::make_shared<User>(std::string(USER_DEFAULT_NAME(0)), host);
    ASSERT_FALSE(user->metatraffic);
    std::shared_ptr<Process> process = std::make_shared<Process>(std::string(PROCESS_DEFAULT_NAME(
                        0)), PID_DEFAULT, user);
    ASSERT_FALSE(process->metatraffic);
    std::shared_ptr<Domain> domain = std::make_shared<Domain>(std::string(DOMAIN_DEFAULT_NAME(0)));
    domain->alias = ALIAS_DEFAULT_NAME(0);
    ASSERT_FALSE(domain->metatraffic);
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(std::string(
                        PARTICIPANT_DEFAULT_NAME(0)), QOS_DEFAULT, GUID_DEFAULT(0), nullptr, domain);
    ASSERT_FALSE(participant->metatraffic);
    std::shared_ptr<Locator> locator = std::make_shared<Locator>(std::string(LOCATOR_DEFAULT_NAME(0)));
    ASSERT_FALSE(locator->metatraffic);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
