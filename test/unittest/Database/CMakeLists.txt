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
    # Clear sample tests
    statistics_sample_clear
    entitydata_sample_clear
    entitycount_sample_clear
    bytecount_sample_clear
    # Comparison operator tests
    statisticssample_operator_comparison
    entitydatasample_operator_comparison
    entitycountsample_operator_comparison
    entitycountsample_operator_minus
    bytecountsample_operator_comparison
    bytecountsample_operator_minus
    timepointsample_operator_comparison
    entitytolocatorcountsample_operator_comparison
    bytetolocatorcountsample_operator_comparison
    historylatencysample_operator_comparison
    networklatencysample_operator_comparison
    publicationthroughputsample_operator_comparison
    subscriptionthroughputsample_operator_comparison
    rtpspacketssentsample_operator_comparison
    rtpsbytessentsample_operator_comparison
    rtpspacketslostsample_operator_comparison
    rtpsbyteslostsample_operator_comparison
    resentdatasample_operator_comparison
    heartbeatcountsample_operator_comparison
    acknackcountsample_operator_comparison
    nackfragcountsample_operator_comparison
    gapcountsample_operator_comparison
    datacountsample_operator_comparison
    pdpcountsample_operator_comparison
    edpcountsample_operator_comparison
    discoverytimesample_operator_comparison
    sampledatascountsample_operator_comparison
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
    insert_host_two_same_name
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
    insert_process_two_diff_host_same_pid
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
    # Insert invalid
    insert_invalid
    insert_locator
    # Link participant with process
    link_participant_with_process_unlinked
    link_participant_with_process_wrong_participant
    link_participant_with_process_wrong_process
    link_participant_with_process_linked_participant
    # Insert samples
    insert_sample_history_latency
    insert_sample_history_latency_wrong_entity
    insert_sample_network_latency
    insert_sample_network_latency_wrong_entity
    insert_sample_publication_throughput
    insert_sample_publication_throughput_wrong_entity
    insert_sample_subscription_throughput
    insert_sample_subscription_throughput_wrong_entity
    insert_sample_rtps_packets_sent
    insert_sample_rtps_packets_sent_wrong_entity
    insert_sample_rtps_bytes_sent
    insert_sample_rtps_bytes_sent_wrong_entity
    insert_sample_rtps_packets_lost
    insert_sample_rtps_packets_lost_wrong_entity
    insert_sample_rtps_bytes_lost
    insert_sample_rtps_bytes_lost_wrong_entity
    insert_sample_resent_data
    insert_sample_resent_data_wrong_entity
    insert_sample_heartbeat_count
    insert_sample_heartbeat_count_wrong_entity
    insert_sample_acknack_count
    insert_sample_acknack_count_wrong_entity
    insert_sample_nackfrag_count
    insert_sample_nackfrag_count_wrong_entity
    insert_sample_gap_count
    insert_sample_gap_count_wrong_entity
    insert_sample_data_count
    insert_sample_data_count_wrong_entity
    insert_sample_pdp_packets
    insert_sample_pdp_packets_wrong_entity
    insert_sample_edp_packets
    insert_sample_edp_packets_wrong_entity
    insert_sample_discovery_time
    insert_sample_discovery_time_wrong_entity
    insert_sample_sample_datas
    insert_sample_sample_datas_wrong_entity
    insert_sample_invalid
    insert_sample_valid_wrong_domain
    # get_entity
    get_entity_host
    get_entity_process
    get_entity_user
    get_entity_domain
    get_entity_topic
    get_entity_participant
    get_entity_datareader
    get_entity_datawriter
    get_entity_locator
    get_entity_no_existing
    # get_entities_by_name
    get_entities_by_name_host
    get_entities_by_name_host_wrong_name
    get_entities_by_name_user
    get_entities_by_name_user_wrong_name
    get_entities_by_name_process
    get_entities_by_name_process_wrong_name
    get_entities_by_name_domain
    get_entities_by_name_domain_wrong_name
    get_entities_by_name_participant
    get_entities_by_name_participant_wrong_name
    get_entities_by_name_topic
    get_entities_by_name_topic_wrong_name
    get_entities_by_name_datawriter
    get_entities_by_name_datawriter_wrong_name
    get_entities_by_name_datareader
    get_entities_by_name_datareader_wrong_name
    get_entities_by_name_locator
    get_entities_by_name_locator_wrong_name
    get_entities_by_name_invalid
    get_entities_by_name_other_kind
    # get_entity_kind
    get_entity_kind
    # select
    select_single_entity_invalid_needs_two_entities
    select_double_entity_invalid_needs_one_entity
    select_sample_datas_invalid_wrong_entity
    select_invalid_entities
    select_invalid_entity_id
    select_invalid_timestamps
    select_fastdds_latency
    select_network_latency
    select_publication_throughput
    select_subscription_throughput
    select_rtps_packets_sent
    select_rtps_bytes_sent
    select_rtps_packets_lost
    select_rtps_bytes_lost
    select_resent_data
    select_heartbeat_count
    select_acknack_count
    select_nackfrag_count
    select_gap_count
    select_data_count
    select_pdp_packets
    select_edp_packets
    select_discovery_time
    select_sample_datas
    )

foreach(test_name ${DATABASE_TEST_LIST})
    add_test(NAME database_tests.${test_name}
            COMMAND database_tests
            --gtest_filter=database_tests.${test_name}:*/database_tests.${test_name}/*)
endforeach()

###############################################################################
# Database Dump tests
###############################################################################

add_executable(database_dump_tests DatabaseDumpTests.cpp)

target_include_directories(database_dump_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/src/cpp)

target_link_libraries(database_dump_tests PUBLIC
    ${PROJECT_NAME}
    gtest)


file(COPY
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/empty_dump.json
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/empty_entities_dump.json
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/simple_dump.json
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/complex_dump.json
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)


set(DATABASE_DUMP_TEST_LIST
    dump_empty_database
    dump_empty_entities_database
    dump_simple_database
    dump_complex_database
)

foreach(test_name ${DATABASE_DUMP_TEST_LIST})
    add_test(NAME database.${test_name}
            COMMAND database_dump_tests
            --gtest_filter=database.${test_name}:*/database.${test_name}/*)
endforeach()

###############################################################################
# Database: get_entities tests
###############################################################################

add_executable(database_get_entities_tests DatabaseGetEntitiesTest.cpp)

target_include_directories(database_get_entities_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/src/cpp)

target_link_libraries(database_get_entities_tests PUBLIC
    ${PROJECT_NAME}
    gtest)

set(DATABASE_GET_ENTITIES_TEST_LIST
    get_entities
)

foreach(test_name ${DATABASE_GET_ENTITIES_TEST_LIST})
    add_test(NAME database_get_entities_tests.${test_name}
            COMMAND database_get_entities_tests
            --gtest_filter=database_get_entities_tests.${test_name}:*/database_get_entities_tests.${test_name}/*)
endforeach()

