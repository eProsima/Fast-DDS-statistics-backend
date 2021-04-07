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

#include <fastdds-statistics-backend/types/EntityId.hpp>

#include "gtest/gtest.h"

#include <iostream>


using namespace eprosima::statistics_backend;

TEST(entityid, all)
{
    EXPECT_EQ(0, EntityId::all().value());
}

TEST(entityid, invalid)
{
    EXPECT_EQ(-1, EntityId::invalid().value());
}

TEST(entityid, invalidate)
{
    EntityId entity_id(1);
    EXPECT_TRUE(entity_id.is_valid());
    entity_id.invalidate();
    EXPECT_FALSE(entity_id.is_valid());
}

TEST(entityid, valid)
{
    EXPECT_FALSE(EntityId().is_valid());
    EXPECT_FALSE(EntityId::invalid().is_valid());
    EXPECT_TRUE(EntityId(0).is_valid());
    EXPECT_TRUE(EntityId(1).is_valid());
    EXPECT_FALSE(EntityId(-5).is_valid());
}

TEST(entityid, value)
{
    int64_t value = 12;
    EXPECT_EQ(value, EntityId(value).value());
}

TEST(entityid, operator_ostream)
{
    int64_t value = 12;
    EntityId entity_id(value);
    std::stringstream out;
    out << entity_id;
    std::stringstream check;
    check << value;
    EXPECT_EQ(check.str(), out.str());
}

TEST(entityid, operator_less)
{
    EntityId entity_id_1(1);
    EntityId entity_id_2(2);
    EXPECT_TRUE(entity_id_1 < entity_id_2);
    EXPECT_FALSE(entity_id_2 < entity_id_1);
}

TEST(entityid, operator_less_equal)
{
    EntityId entity_id_1(1);
    EntityId entity_id_2(2);
    EntityId entity_id_3(2);
    EXPECT_TRUE(entity_id_1 <= entity_id_2);
    EXPECT_TRUE(entity_id_2 <= entity_id_3);
    EXPECT_FALSE(entity_id_2 <= entity_id_1);
}

TEST(entityid, operator_greater)
{
    EntityId entity_id_1(1);
    EntityId entity_id_2(2);
    EXPECT_TRUE(entity_id_2 > entity_id_1);
    EXPECT_FALSE(entity_id_1 > entity_id_2);
}

TEST(entityid, operator_greater_equal)
{
    EntityId entity_id_1(1);
    EntityId entity_id_2(2);
    EntityId entity_id_3(2);
    EXPECT_TRUE(entity_id_2 >= entity_id_1);
    EXPECT_TRUE(entity_id_3 >= entity_id_2);
    EXPECT_FALSE(entity_id_1 >= entity_id_2);
}

TEST(entityid, operator_equal)
{
    EntityId entity_id_1(1);
    EntityId entity_id_2(2);
    EntityId entity_id_3(2);
    EXPECT_FALSE(entity_id_1 == entity_id_2);
    EXPECT_TRUE(entity_id_2 == entity_id_3);
}

TEST(entityid, operator_different)
{
    EntityId entity_id_1(1);
    EntityId entity_id_2(2);
    EntityId entity_id_3(2);
    EXPECT_FALSE(entity_id_2 != entity_id_3);
    EXPECT_TRUE(entity_id_1 != entity_id_2);
}


int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
