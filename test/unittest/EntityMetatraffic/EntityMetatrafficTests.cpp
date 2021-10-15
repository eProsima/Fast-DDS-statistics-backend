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

#define NON_METATRAFFIC_TOPIC_NAMES {"topic_0", "rt/chatter", "Square"}
#define METATRAFFIC_TOPIC_NAMES {"ros_discovery_info", "rt/rosout", "_fastdds_statistics_network_latency"}

// Test is_metatraffic_topic Entity method for non-metatraffic topics
TEST(entitymetatraffic, non_metatraffic_topic)
{
    for (auto topic_name : NON_METATRAFFIC_TOPIC_NAMES)
    {
        ASSERT_FALSE(Entity::is_metatraffic_topic(topic_name));
    }
}

// Test is_metatraffic_topic Entity method for metatraffic topics
TEST(entitymetatraffic, metatraffic_topic)
{
    for (auto topic_name : METATRAFFIC_TOPIC_NAMES)
    {
        ASSERT_TRUE(Entity::is_metatraffic_topic(topic_name));
    }
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
