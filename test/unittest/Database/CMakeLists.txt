# Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

add_executable(database_tests DatabaseTests.cpp)

target_include_directories(database_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/src/cpp)

target_link_libraries(database_tests PUBLIC
    ${PROJECT_NAME}
    gtest)

set(ENTITYID_TEST_LIST
    insert_host
    insert_host_two
    insert_host_duplicated
    insert_user_valid
    insert_user_two_valid
    insert_user_duplicated
    insert_user_wrong_host
    insert_user_duplicated_name
    insert_process_valid
    insert_process_two_valid
    insert_process_duplicated
    insert_process_wrong_user
    insert_process_empty_pid
    insert_process_two_same_user_diff_pid
    insert_process_two_same_user_same_pid
    insert_process_two_diff_user_same_pid
    insert_domain_valid
    insert_domain_two_valid
    insert_domain_duplicated
    insert_domain_empty_name
    insert_domain_same_name
    insert_invalid)

foreach(test_name ${ENTITYID_TEST_LIST})
    add_test(NAME database.${test_name}
            COMMAND database_tests
            --gtest_filter=database.${test_name}:*/database.${test_name}/*)
endforeach()
