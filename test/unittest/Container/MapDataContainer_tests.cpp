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

#include <string>
#include <chrono>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/types/utils.hpp>
#include <types/MapDataContainer.hpp>
#include <database/data.hpp>

using namespace eprosima::statistics_backend;


// Empty class to parametrized tests
template<class T>
struct MapDataContainer_tests : public ::testing::Test
{};
// Needed gtest macro
TYPED_TEST_SUITE_P(MapDataContainer_tests);

// Auxiliary methods for test
namespace test {

//! Create a new value
template <typename T>
T arbitrary_value(
        unsigned int index = 0);

template <>
int arbitrary_value<int>(
        unsigned int index /* = 0 */)
{
    return index;
}

template <>
std::string arbitrary_value<std::string>(
        unsigned int index /* = 0 */)
{
    return std::to_string(index);
}

template <>
EntityId arbitrary_value<EntityId>(
        unsigned int index /* = 0 */)
{
    return EntityId(index);
}

constexpr const unsigned int N_DATA_TEST = 10;
constexpr const unsigned int N_KEY_TEST = 5;

using Data = database::EntityDataSample;

Timestamp arbitrary_timestamp(
        unsigned int index = 0)
{
    return the_initial_time() + std::chrono::seconds(index);
}

Data arbitrary_data(
        unsigned int index = 0)
{
    Data d;
    d.kind = DataKind::INVALID;
    d.src_ts = arbitrary_timestamp(index);
    d.data = index;
    return d;
}

} // namespace test

/**
 * Test creating, adding, finding and destroying a MapDataContainer
 */
TYPED_TEST_P(MapDataContainer_tests, trivial)
{
    // Create map
    details::MapDataContainer<TypeParam, test::Data> map;

    // Insert a new vector
    auto value = test::arbitrary_value<TypeParam>();
    map[value];

    // Look for the vector
    ASSERT_NE(map.find(value), map.end());
    ASSERT_EQ(map.find(test::arbitrary_value<TypeParam>(1)), map.end());

    // Insert a new value in vector
    auto data = test::arbitrary_data(0);
    map[value].push_back(data);
    ASSERT_EQ(map[value].size(), 1u);
    ASSERT_EQ(map[value][0], data);
}

/**
 * Test clear method
 *
 * STEPS:
 * - Create map
 * - Fill map with empty vectors
 * - Fill data inside vectors
 * - Add an element in map that has newer data
 * - Clear only old data
 * - Check new data is still in vectors
 * - Clear a great time
 * - Check only aux element is in map
 * - Clear to the end of time
 * - Check map is empty
 */
TYPED_TEST_P(MapDataContainer_tests, clear)
{
    // Create map
    details::MapDataContainer<TypeParam, test::Data> map;

    // Fill map with empty vectors
    for (unsigned int i = 0; i < test::N_KEY_TEST; ++i)
    {
        map[test::arbitrary_value<TypeParam>(i)];
    }
    ASSERT_EQ(map.size(), test::N_KEY_TEST);

    // Fill data inside vectors
    for (unsigned int i = 0; i < test::N_KEY_TEST; ++i)
    {
        for (unsigned int j = 0; j < test::N_DATA_TEST; ++j)
        {
            map[test::arbitrary_value<TypeParam>(i)].push_back(
                test::arbitrary_data(j));
        }
        ASSERT_EQ(map[test::arbitrary_value<TypeParam>(i)].size(), test::N_DATA_TEST);
    }

    // Add an element in map that has more data
    auto aux_key = test::arbitrary_value<TypeParam>(test::N_KEY_TEST * 2);
    map[aux_key].push_back(test::arbitrary_data(test::N_DATA_TEST*3));

    // Clear only old data
    auto cut_point = test::arbitrary_timestamp(test::N_DATA_TEST / 2);
    map.clear(cut_point);

    // Check new data is still in vectors
    for (unsigned int i = 0; i < test::N_KEY_TEST; ++i)
    {
        ASSERT_FALSE(map[test::arbitrary_value<TypeParam>(i)].empty());
        for (const auto& value : map[test::arbitrary_value<TypeParam>(i)])
        {
            ASSERT_GE(value.src_ts, cut_point);
        }
    }

    // Clear a great time
    auto future_cut_point = test::arbitrary_timestamp(test::N_DATA_TEST * 2);
    map.clear(future_cut_point);

    // Check no data is in the vectors
    ASSERT_EQ(map.size(), 1u);

    // Clear to the end of time
    map.clear(the_end_of_time());

    // Check map is empty
    ASSERT_TRUE(map.empty());
}

REGISTER_TYPED_TEST_SUITE_P(
    MapDataContainer_tests,
    trivial,
    clear
    );

// Set types used in parametrization
typedef ::testing::Types<
            int,
            std::string,
            EntityId
            > CaseTypes;

// Generate each test case for each type case
INSTANTIATE_TYPED_TEST_SUITE_P(
    parametrized_MapDataContainer_tests,
    MapDataContainer_tests,
    CaseTypes);

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
