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

###############################################################################
# Sample tests
###############################################################################

add_executable(sample_tests SampleTests.cpp)

target_include_directories(sample_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/src/cpp)

target_link_libraries(sample_tests PUBLIC
    ${PROJECT_NAME}
    gtest)

set(SAMPLES_TEST_LIST
    statistics_sample_clear
    entitydata_sample_clear
    entitycount_sample_clear
    bytecount_sample_clear
)

foreach(test_name ${SAMPLES_TEST_LIST})
    add_test(NAME database.${test_name}
            COMMAND sample_tests
            --gtest_filter=database.${test_name}:*/database.${test_name}/*)
endforeach()

###############################################################################
# Data tests
###############################################################################

add_executable(data_tests DataTests.cpp)

target_include_directories(data_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/src/cpp)

target_link_libraries(data_tests PUBLIC
    ${PROJECT_NAME}
    gtest)

set(DATA_TEST_LIST
    domainparticipant_data_clear
    datareader_data_clear
    datawriter_data_clear
    locator_data_clear
)

foreach(test_name ${DATA_TEST_LIST})
    add_test(NAME database.${test_name}
            COMMAND data_tests
            --gtest_filter=database.${test_name}:*/database.${test_name}/*)
endforeach()

###############################################################################
# Database tests
###############################################################################

add_executable(database_tests DatabaseTests.cpp)

target_include_directories(database_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/src/cpp)

target_link_libraries(database_tests PUBLIC
    ${PROJECT_NAME}
    gtest)

set(DATABASE_TEST_LIST
    # Insert Host test
    insert_host
    insert_host_two
    insert_host_duplicated
    insert_host_empty_name
    # Insert User tests
    insert_user_valid
    insert_user_two_valid
    insert_user_duplicated
    insert_user_wrong_host
    insert_user_empty_name
    insert_user_duplicated_name
    # Insert Process tests
    insert_process_valid
    insert_process_two_valid
    insert_process_duplicated
    insert_process_wrong_user
    insert_process_empty_name
    insert_process_empty_pid
    insert_process_two_same_user_diff_pid
    insert_process_two_same_user_same_pid
    insert_process_two_diff_user_same_pid
    # Insert Domain tests
    insert_domain_valid
    insert_domain_two_valid
    insert_domain_duplicated
    insert_domain_empty_name
    insert_domain_same_name
    # Insert Topic tests
    insert_topic_valid
    insert_topic_two_valid
    insert_topic_duplicated
    insert_topic_wrong_domain
    insert_topic_empty_name
    insert_topic_empty_datatype
    insert_topic_two_same_domain_same_name
    insert_topic_two_same_domain_diff_name_same_type
    # Insert DomainParticipant tests
    insert_participant_valid
    insert_participant_two_valid
    insert_participant_duplicated
    insert_participant_wrong_domain
    insert_participant_empty_name
    insert_participant_empty_qos
    insert_participant_empty_guid
    insert_participant_two_same_domain_same_guid
    insert_participant_two_diff_domain_same_guid
    # Insert DDSEndpoint tests
    insert_ddsendpoint_valid
    insert_ddsendpoint_two_valid
    insert_ddsendpoint_duplicated
    insert_ddsendpoint_wrong_participant
    insert_ddsendpoint_wrong_topic
    insert_ddsendpoint_empty_name
    insert_ddsendpoint_empty_qos
    insert_ddsendpoint_empty_guid
    insert_ddsendpoint_empty_locators
    insert_ddsendpoint_two_same_domain_same_guid
    insert_ddsendpoint_two_diff_domain_same_guid
    insert_invalid)

foreach(test_name ${DATABASE_TEST_LIST})
    add_test(NAME database.${test_name}
            COMMAND database_tests
            --gtest_filter=database.${test_name}:*/database.${test_name}/*)
endforeach()
