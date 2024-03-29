# Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

add_executable(entityid_tests
    EntityIdTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/EntityId.cpp
)

if(MSVC)
    target_compile_definitions(entityid_tests PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(entityid_tests PRIVATE
    ${GTEST_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp)

target_link_libraries(entityid_tests PUBLIC ${GTEST_LIBRARIES})

get_win32_path_dependencies(entityid_tests TEST_FRIENDLY_PATH)

set(ENTITYID_TEST_LIST
    all
    is_all
    invalid
    invalidate
    valid
    value
    valid_and_unique
    operator_ostream
    operator_less
    operator_less_equal
    operator_greater
    operator_greater_equal
    operator_equal
    operator_different)

foreach(test_name ${ENTITYID_TEST_LIST})
    add_test(NAME entityid.${test_name}
            COMMAND entityid_tests
            --gtest_filter=entityid.${test_name}:*/entityid.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(entityid.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
endforeach()
