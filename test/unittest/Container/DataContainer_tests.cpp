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
#include <types/DataContainer.hpp>
#include <database/data.hpp>

using namespace eprosima::statistics_backend;


// Empty class to parametrized tests
template<class T>
struct DataContainer_tests : public ::testing::Test
{};
// Needed gtest macro
TYPED_TEST_SUITE_P(DataContainer_tests);

// Auxiliary methods for test
namespace test {

Timestamp arbitrary_timestamp(unsigned int index = 0)
{
    return the_initial_time() + std::chrono::seconds(index);
}

//! Create a new value
template <typename T>
T arbitrary_value(unsigned int index = 0);

template <>
database::EntityCountSample arbitrary_value<database::EntityCountSample>(unsigned int index /* = 0 */)
{
    database::EntityCountSample d;
    d.kind = DataKind::INVALID;
    d.src_ts = arbitrary_timestamp(index);
    d.count = index;
    return d;
}

template <>
database::EntityDataSample arbitrary_value<database::EntityDataSample>(unsigned int index /* = 0 */)
{
    database::EntityDataSample d;
    d.kind = DataKind::INVALID;
    d.src_ts = arbitrary_timestamp(index);
    d.data = index;
    return d;
}

template <>
database::ByteCountSample arbitrary_value<database::ByteCountSample>(unsigned int index /* = 0 */)
{
    database::ByteCountSample d;
    d.kind = DataKind::INVALID;
    d.src_ts = arbitrary_timestamp(index);
    d.count = index;
    d.magnitude_order = 0;
    return d;
}

//! Class to make public find_by_timestamp_
template<typename T>
class DataContainerTest : public details::DataContainer<T>
{
public:
    using details::DataContainer<T>::find_by_timestamp_;
};

} // namespace test

/**
 * Test creating, adding and destroying a DataContainer
 */
TYPED_TEST_P(DataContainer_tests, trivial)
{
    // Create
    details::DataContainer<TypeParam> container;

    // Add value
    auto value = test::arbitrary_value<TypeParam>();
    container.push_back(value);
    ASSERT_EQ(container.size(), 1u);
}

/**
 * Test DataContainer::clear method
 *
 * CASES:
 * - empty
 * - clear with time
 * - clear bigger time
 * - clear all
 * - clear without arguments
 */
TYPED_TEST_P(DataContainer_tests, clear)
{
    // empty
    {
        details::DataContainer<TypeParam> container;
        container.clear();
    }


    // clear with time
    {
        details::DataContainer<TypeParam> container;
        container.push_back(test::arbitrary_value<TypeParam>(1));
        container.push_back(test::arbitrary_value<TypeParam>(3));
        container.push_back(test::arbitrary_value<TypeParam>(5));
        container.push_back(test::arbitrary_value<TypeParam>(7));
        ASSERT_EQ(container.size(), 4u);

        container.clear(test::arbitrary_timestamp(0));
        ASSERT_EQ(container.size(), 4u);

        container.clear(test::arbitrary_timestamp(2));
        ASSERT_EQ(container.size(), 3u);

        container.clear(test::arbitrary_timestamp(6));
        ASSERT_EQ(container.size(), 1u);

        container.clear(test::arbitrary_timestamp(7));
        ASSERT_EQ(container.size(), 1u);
    }

    // clear bigger time
    {
        details::DataContainer<TypeParam> container;
        container.push_back(test::arbitrary_value<TypeParam>(1));
        container.push_back(test::arbitrary_value<TypeParam>(3));
        container.push_back(test::arbitrary_value<TypeParam>(5));
        container.push_back(test::arbitrary_value<TypeParam>(7));
        ASSERT_EQ(container.size(), 4u);

        container.clear(test::arbitrary_timestamp(10));
        ASSERT_EQ(container.size(), 0u);
    }

    // clear all
    {
        details::DataContainer<TypeParam> container;
        container.push_back(test::arbitrary_value<TypeParam>(1));
        container.push_back(test::arbitrary_value<TypeParam>(3));
        container.push_back(test::arbitrary_value<TypeParam>(5));
        container.push_back(test::arbitrary_value<TypeParam>(7));
        ASSERT_EQ(container.size(), 4u);

        container.clear(the_end_of_time());
        ASSERT_EQ(container.size(), 0u);
    }

    // clear without arguments
    {
        details::DataContainer<TypeParam> container;
        container.push_back(test::arbitrary_value<TypeParam>(1));
        container.push_back(test::arbitrary_value<TypeParam>(3));
        container.push_back(test::arbitrary_value<TypeParam>(5));
        container.push_back(test::arbitrary_value<TypeParam>(7));
        ASSERT_EQ(container.size(), 4u);

        container.clear();
        ASSERT_EQ(container.size(), 0u);
    }
}

/**
 * Test DataContainer::operator [] method
 *
 * CASES:
 * - not const
 *   - first
 *   - last
 *   - middle
 *   - not in
 * - const
 *   - first
 *   - last
 *   - middle
 *   - not in
 */
TYPED_TEST_P(DataContainer_tests, access_operator)
{
    details::DataContainer<TypeParam> container;
    container.push_back(test::arbitrary_value<TypeParam>(5));
    container.push_back(test::arbitrary_value<TypeParam>(1));
    container.push_back(test::arbitrary_value<TypeParam>(7));
    container.push_back(test::arbitrary_value<TypeParam>(3));

    // not const
    {
        // first
        {
            ASSERT_EQ(container[0], test::arbitrary_value<TypeParam>(5));
        }

        // last
        {
            ASSERT_EQ(container[container.size() - 1], test::arbitrary_value<TypeParam>(3));
        }

        // middle
        {
            ASSERT_EQ(container[2], test::arbitrary_value<TypeParam>(7));
        }

        // not in
        {
            ASSERT_THROW(container[4], std::out_of_range);
        }
    }

    // const
    {
        const auto const_container = container;
        // first
        {
            ASSERT_EQ(const_container[0], test::arbitrary_value<TypeParam>(5));
        }

        // last
        {
            ASSERT_EQ(const_container[const_container.size() - 1], test::arbitrary_value<TypeParam>(3));
        }

        // middle
        {
            ASSERT_EQ(const_container[2], test::arbitrary_value<TypeParam>(7));
        }

        // not in
        {
            ASSERT_THROW(const_container[4], std::out_of_range);
        }
    }
}

/**
 * Test DataContainer::get_interval_limits method
 *
 * CASES:
 * - empty vector
 * - only 1 value vector
 *   - inside interval
 *   - outside interval
 * - 2 values vector
 *   - empty interval
 *   - 1st value in interval
 *   - 2nd values in interval
 *   - 2 values in interval
 * - N values vector
 *   - empty interval
 *   - 1 value interval
 *   - n values interval
 *   - all values interval
 * - 3 values vector
 *   - exact value interval
 */
TYPED_TEST_P(DataContainer_tests, get_interval_limits)
{
    // empty vector
    {
        details::DataContainer<TypeParam> container;
        auto res = container.get_interval_limits(test::arbitrary_timestamp(0), test::arbitrary_timestamp(10));
        ASSERT_EQ(res.first, container.end());
        ASSERT_EQ(res.second, container.end());
    }

    // only 1 value vector
    {
        details::DataContainer<TypeParam> container;
        container.push_back(test::arbitrary_value<TypeParam>(1));

        // inside interval
        {
            auto res = container.get_interval_limits(test::arbitrary_timestamp(0), test::arbitrary_timestamp(10));
            ASSERT_EQ(res.first, container.begin());
            ASSERT_EQ(res.second, container.end());
        }

        // outside interval
        {
            auto res = container.get_interval_limits(test::arbitrary_timestamp(2), test::arbitrary_timestamp(10));
            ASSERT_EQ(res.first, container.end());
            ASSERT_EQ(res.second, container.end());
        }
    }

    // 2 values vector
    {
        details::DataContainer<TypeParam> container;
        container.push_back(test::arbitrary_value<TypeParam>(1));
        container.push_back(test::arbitrary_value<TypeParam>(3));

        // empty interval
        {
            auto res = container.get_interval_limits(test::arbitrary_timestamp(4), test::arbitrary_timestamp(10));
            ASSERT_EQ(res.first, container.end());
            ASSERT_EQ(res.second, container.end());
        }

        // 1st value in interval
        {
            auto res = container.get_interval_limits(test::arbitrary_timestamp(0), test::arbitrary_timestamp(2));
            ASSERT_EQ(res.first, container.begin());
            ASSERT_EQ(res.second, ++container.begin());
        }

        // 2nd values in interval
        {
            auto res = container.get_interval_limits(test::arbitrary_timestamp(2), test::arbitrary_timestamp(4));
            ASSERT_EQ(res.first, ++container.begin());
            ASSERT_EQ(res.second, container.end());
        }

        // 2 values in interval
        {
            auto res = container.get_interval_limits(test::arbitrary_timestamp(0), test::arbitrary_timestamp(10));
            ASSERT_EQ(res.first, container.begin());
            ASSERT_EQ(res.second, container.end());
        }
    }

    // N values vector
    {
        details::DataContainer<TypeParam> container;
        container.push_back(test::arbitrary_value<TypeParam>(1));
        container.push_back(test::arbitrary_value<TypeParam>(3));
        container.push_back(test::arbitrary_value<TypeParam>(5));
        container.push_back(test::arbitrary_value<TypeParam>(7));
        container.push_back(test::arbitrary_value<TypeParam>(9));

        // empty interval
        {
            auto res = container.get_interval_limits(test::arbitrary_timestamp(10), test::arbitrary_timestamp(20));
            ASSERT_EQ(res.first, container.end());
            ASSERT_EQ(res.second, container.end());
        }

        // 1 value interval
        {
            auto res = container.get_interval_limits(test::arbitrary_timestamp(4), test::arbitrary_timestamp(6));
            auto element_5 = container.begin();
            element_5++;
            element_5++;
            auto element_7 = element_5;
            element_7++;
            ASSERT_EQ(res.first, element_5);
            ASSERT_EQ(res.second, element_7);
        }

        // n values interval
        {
            auto res = container.get_interval_limits(test::arbitrary_timestamp(2), test::arbitrary_timestamp(8));
            auto element_3 = container.begin();
            element_3++;
            auto element_9 = element_3;
            element_9++;
            element_9++;
            element_9++;
            ASSERT_EQ(res.first, element_3);
            ASSERT_EQ(res.second, element_9);
        }

        // all values interval
        {
            auto res = container.get_interval_limits(test::arbitrary_timestamp(0), test::arbitrary_timestamp(10));
            ASSERT_EQ(res.first, container.begin());
            ASSERT_EQ(res.second, container.end());
        }
    }

    // 3 values vector
    {
        details::DataContainer<TypeParam> container;
        container.push_back(test::arbitrary_value<TypeParam>(1));
        container.push_back(test::arbitrary_value<TypeParam>(3));
        container.push_back(test::arbitrary_value<TypeParam>(5));

        // exact value interval
        {
            auto res = container.get_interval_limits(test::arbitrary_timestamp(3), test::arbitrary_timestamp(3));
            auto element_3 = container.begin();
            element_3++;
            ASSERT_EQ(res.first, element_3);
            ASSERT_EQ(res.second, element_3);
        }
    }
}

/**
 * Test DataContainer::get_interval_limits method
 *
 * CASES:
 * - empty vector
 * - only 1 value vector
 *   - lower
 *   - exact
 *   - bigger
 * - N values vector
 *   - lower than first
 *   - bigger than last
 *   - middle
 */
TYPED_TEST_P(DataContainer_tests, find_by_timestamp_)
{
    // empty vector
    {
        test::DataContainerTest<TypeParam> container;
        auto res = container.find_by_timestamp_(test::arbitrary_timestamp(0));
        ASSERT_EQ(res, container.end());
    }

    // only 1 value vector
    {
        test::DataContainerTest<TypeParam> container;
        container.push_back(test::arbitrary_value<TypeParam>(1));

        // lower
        {
            auto res = container.find_by_timestamp_(test::arbitrary_timestamp(0));
            ASSERT_EQ(res, container.begin());
        }

        // exact
        {
            auto res = container.find_by_timestamp_(test::arbitrary_timestamp(1));
            ASSERT_EQ(res, container.begin());
        }

        // bigger
        {
            auto res = container.find_by_timestamp_(test::arbitrary_timestamp(2));
            ASSERT_EQ(res, container.end());
        }
    }


    // N values vector
    {
        test::DataContainerTest<TypeParam> container;
        container.push_back(test::arbitrary_value<TypeParam>(1));
        container.push_back(test::arbitrary_value<TypeParam>(3));
        container.push_back(test::arbitrary_value<TypeParam>(5));

        // lower than first
        {
            auto res = container.find_by_timestamp_(test::arbitrary_timestamp(0));
            ASSERT_EQ(res, container.begin());
        }

        // bigger than last
        {
            auto res = container.find_by_timestamp_(test::arbitrary_timestamp(6));
            ASSERT_EQ(res, container.end());
        }

        // middle
        {
            auto res = container.find_by_timestamp_(test::arbitrary_timestamp(2));
            ASSERT_EQ(res, ++container.begin());
        }
    }
}

REGISTER_TYPED_TEST_SUITE_P(
    DataContainer_tests,
    trivial,
    clear,
    access_operator,
    get_interval_limits,
    find_by_timestamp_
    );

// Set types used in parametrization
typedef ::testing::Types<
            database::EntityCountSample,
            database::EntityDataSample,
            database::ByteCountSample
            > CaseTypes;

// Generate each test case for each type case
INSTANTIATE_TYPED_TEST_SUITE_P(
    parametrized_DataContainer_tests,
    DataContainer_tests,
    CaseTypes);

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
