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

#include <string>
#include <sstream>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <types/fragile_ptr.hpp>
#include <fastdds_statistics_backend/exception/Exception.hpp>

using namespace eprosima::statistics_backend::details;


// Empty class to parametrized tests
template<class T>
struct fragile_ptr_tests : public ::testing::Test
{};
// Needed gtest macro
TYPED_TEST_SUITE_P(fragile_ptr_tests);

// Auxiliary methods for test
namespace test {

//! Alloc a new value of type T
template <typename T>
T* new_value();

template <>
bool* new_value<bool>()
{
    return new bool(true);
}

template <>
int* new_value<int>()
{
    return new int(42);
}

template <>
std::string* new_value<std::string>()
{
    return new std::string("wololo");
}


//! Alloc a new value of type T
template <typename T>
std::string to_string(const T& t)
{
    std::stringstream ss;
    ss << "{" << t << "}";
    return ss.str();
}

} // namespace test


/**
 * Test \c fragile_ptr::expired method.
 *
 * CASES:
 * - void ptr
 * - ptr with invalid reference
 * - ptr with valid reference
 * - ptr with valid reference already removed
 */
TYPED_TEST_P(fragile_ptr_tests, expired)
{
    // void ptr
    {
        fragile_ptr<TypeParam> ptr;
        ASSERT_TRUE(ptr.expired());
    }

    // ptr with invalid reference
    {
        std::shared_ptr<TypeParam> shared_reference;
        fragile_ptr<TypeParam> ptr(shared_reference);
        ASSERT_TRUE(ptr.expired());
    }

    // ptr with valid reference
    {
        std::shared_ptr<TypeParam> shared_reference = std::shared_ptr<TypeParam>(test::new_value<TypeParam>());
        fragile_ptr<TypeParam> ptr(shared_reference);
        ASSERT_FALSE(ptr.expired());
    }

    // ptr with valid reference already removed
    {
        std::shared_ptr<TypeParam> shared_reference = std::shared_ptr<TypeParam>(test::new_value<TypeParam>());
        fragile_ptr<TypeParam> ptr(shared_reference);
        shared_reference.reset();
        ASSERT_TRUE(ptr.expired());
    }
}

/**
 * Test \c fragile_ptr::reset method.
 *
 * CASES:
 * - void ptr (expected: nothing happens)
 * - ptr with invalid reference (expected: nothing happens)
 * - ptr with valid reference (expected: internal ref is lost but shared ptr still has a valid object)
 * - ptr with valid reference already removed (expected: nothing happens)
 */
TYPED_TEST_P(fragile_ptr_tests, reset)
{
    // void ptr
    {
        fragile_ptr<TypeParam> ptr;
        ptr.reset();
        ASSERT_TRUE(ptr.expired());
    }

    // ptr with invalid reference
    {
        std::shared_ptr<TypeParam> shared_reference;
        fragile_ptr<TypeParam> ptr(shared_reference);
        ptr.reset();
        ASSERT_TRUE(ptr.expired());
    }

    // ptr with valid reference
    {
        std::shared_ptr<TypeParam> shared_reference = std::shared_ptr<TypeParam>(test::new_value<TypeParam>());
        fragile_ptr<TypeParam> ptr(shared_reference);
        ptr.reset();
        ASSERT_TRUE(ptr.expired());
        ASSERT_TRUE(shared_reference.operator bool());
    }

    // ptr with valid reference already removed
    {
        std::shared_ptr<TypeParam> shared_reference = std::shared_ptr<TypeParam>(test::new_value<TypeParam>());
        fragile_ptr<TypeParam> ptr(shared_reference);
        shared_reference.reset();
        ptr.reset();
        ASSERT_TRUE(ptr.expired());
    }
}

/**
 * Test \c fragile_ptr::null_comparison method.
 *
 * CASES:
 * - void ptr
 * - ptr with invalid reference
 * - ptr with valid reference
 *
 * FOR EVERY CASE:
 * - == null
 * - null ==
 * - != null
 * - null !=
 */
TYPED_TEST_P(fragile_ptr_tests, null_comparison)
{
    // void ptr
    {
        fragile_ptr<TypeParam> ptr;

        ASSERT_TRUE(ptr == nullptr);
        ASSERT_TRUE(nullptr == ptr);
        ASSERT_FALSE(ptr != nullptr);
        ASSERT_FALSE(nullptr != ptr);
    }

    // ptr with invalid reference
    {
        std::shared_ptr<TypeParam> shared_reference;
        fragile_ptr<TypeParam> ptr(shared_reference);

        ASSERT_TRUE(ptr == nullptr);
        ASSERT_TRUE(nullptr == ptr);
        ASSERT_FALSE(ptr != nullptr);
        ASSERT_FALSE(nullptr != ptr);
    }

    // ptr with valid reference
    {
        std::shared_ptr<TypeParam> shared_reference = std::shared_ptr<TypeParam>(test::new_value<TypeParam>());
        fragile_ptr<TypeParam> ptr(shared_reference);

        ASSERT_FALSE(ptr == nullptr);
        ASSERT_FALSE(nullptr == ptr);
        ASSERT_TRUE(ptr != nullptr);
        ASSERT_TRUE(nullptr != ptr);
    }
}

/**
 * Test \c fragile_ptr::serialize method.
 *
 * CASES:
 * - void ptr
 * - ptr with valid reference
 */
TYPED_TEST_P(fragile_ptr_tests, serialize)
{
    // void ptr
    {
        fragile_ptr<TypeParam> ptr;

        std::stringstream ss;
        ss << ptr;
        ASSERT_EQ(ss.str(), "{nullptr}");
    }

    // ptr with valid reference
    {
        std::shared_ptr<TypeParam> shared_reference = std::shared_ptr<TypeParam>(test::new_value<TypeParam>());
        fragile_ptr<TypeParam> ptr(shared_reference);

        std::string string_result;
        {
            std::stringstream ss;
            ss << shared_reference;
            string_result += ss.str();
        }

        std::stringstream ss;
        ss << ptr;
        ASSERT_EQ(ss.str(), string_result);
    }
}

/**
 * Test \c fragile_ptr::use_ptr_negative method.
 *
 * CASES:
 * - void ptr
 * - ptr with invalid reference
 * - ptr with valid reference already removed
 *
 * NOTE: it uses operator* as -> could not be used equally for every parameter
 */
TYPED_TEST_P(fragile_ptr_tests, use_ptr_negative)
{
    // void ptr
    {
        fragile_ptr<TypeParam> ptr;

        ASSERT_THROW(*ptr, eprosima::statistics_backend::Inconsistency);
    }

    // ptr with invalid reference
    {
        std::shared_ptr<TypeParam> shared_reference;
        fragile_ptr<TypeParam> ptr(shared_reference);

        ASSERT_THROW(*ptr, eprosima::statistics_backend::Inconsistency);
    }

    // ptr with valid reference already removed
    {
        std::shared_ptr<TypeParam> shared_reference = std::shared_ptr<TypeParam>(test::new_value<TypeParam>());
        fragile_ptr<TypeParam> ptr(shared_reference);
        shared_reference.reset();

        ASSERT_THROW(*ptr, eprosima::statistics_backend::Inconsistency);
    }
}

REGISTER_TYPED_TEST_SUITE_P(
    fragile_ptr_tests,
    expired,
    reset,
    null_comparison,
    serialize,
    use_ptr_negative
);

// Set types used in parametrization
typedef ::testing::Types<
        bool,
        int,
        std::string
    > CaseTypes;

// Generate each test case for each type case
INSTANTIATE_TYPED_TEST_SUITE_P(
    parametrized_fragile_ptr_tests,
    fragile_ptr_tests,
    CaseTypes);

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
