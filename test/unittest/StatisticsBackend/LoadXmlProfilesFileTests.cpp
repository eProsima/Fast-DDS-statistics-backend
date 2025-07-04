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

#include <list>
#include <string>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.hpp>
#include <fastdds/statistics/topic_names.hpp>

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/listener/CallbackMask.hpp>
#include <fastdds_statistics_backend/listener/DomainListener.hpp>
#include <fastdds_statistics_backend/StatisticsBackend.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/types.hpp>
#include <fastdds_statistics_backend/types/app_names.h>

#include <Monitor.hpp>
#include <StatisticsBackendData.hpp>
#include <topic_types/typesPubSubTypes.hpp>


using namespace eprosima::statistics_backend;
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::statistics;
using namespace eprosima::fastdds::rtps;


class load_xml_profiles_file_tests : public ::testing::Test
{

public:

    load_xml_profiles_file_tests()
    {
    }

    ~load_xml_profiles_file_tests()
    {
        StatisticsBackend::reset();
    }
};

TEST_F(load_xml_profiles_file_tests, load_xml_profile_no_file)
{
    EXPECT_THROW(StatisticsBackend::load_xml_profiles_file(
            "no_file.xml"), BadParameter);
}

TEST_F(load_xml_profiles_file_tests, load_xml_profiles_returns_expected_profiles)
{
    // Assume "profile.xml" exists and contains two profiles: "backend_participant" and "participant_domain_3"
    std::vector<std::string> profiles = StatisticsBackend::load_xml_profiles_file("profile.xml");

    ASSERT_EQ(profiles.size(), 2u);
    EXPECT_THAT(profiles, ::testing::ElementsAre("backend_participant", "participant_domain_3"));
}

TEST_F(load_xml_profiles_file_tests, load_xml_profiles_returns_expected_profiles_standalone)
{
    // Assume "profile.xml" exists and contains two profiles: "backend_participant" and "participant_domain_3"
    std::vector<std::string> profiles = StatisticsBackend::load_xml_profiles_file("profile_standalone.xml");

    ASSERT_EQ(profiles.size(), 2u);
    EXPECT_THAT(profiles, ::testing::ElementsAre("backend_participant", "participant_domain_3"));
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
