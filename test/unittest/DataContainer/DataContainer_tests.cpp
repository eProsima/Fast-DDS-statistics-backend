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

#include <types/DataContainer.hpp>
#include <fastdds_statistics_backend/exception/Exception.hpp>

using namespace eprosima::statistics_backend::details;


// Empty class to parametrized tests
template<class T>
struct DataContainer_tests : public ::testing::Test
{};
// Needed gtest macro
TYPED_TEST_SUITE_P(DataContainer_tests);

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
std::string to_string(
        const T& t)
{
    std::stringstream ss;
    ss << "{" << t << "}";
    return ss.str();
}

} // namespace test

TYPED_TEST_P(DataContainer_tests, trivial)
{
    // TODO
}

TYPED_TEST_P(DataContainer_tests, clear)
{
    // TODO
}

TYPED_TEST_P(DataContainer_tests, iteration)
{
    // TODO
}

REGISTER_TYPED_TEST_SUITE_P(
    DataContainer_tests,
    trivial,
    clear,
    iteration
    );

// Set types used in parametrization
typedef ::testing::Types<
            bool,
            int,
            std::string
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
