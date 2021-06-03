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

add_executable(entityid_tests EntityIdTests.cpp)
target_link_libraries(entityid_tests PUBLIC ${PROJECT_NAME} gtest)

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
endforeach()
