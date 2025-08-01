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


set(LIBRARY_SOURCES
    ${PROJECT_SOURCE_DIR}/src/cpp/database/data.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/database_queue.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/database.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/entities.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/samples.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/StatisticsBackend.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/StatisticsBackendData.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/subscriber/QosSerializer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/subscriber/StatisticsParticipantListener.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/subscriber/StatisticsReaderListener.cpp
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/monitorservice_typesTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/monitorservice_typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/typesTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/types/EntityId.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/types.cpp)

###############################################################################
# Sample tests
###############################################################################

add_executable(sample_tests SampleTests.cpp ${LIBRARY_SOURCES})

if(MSVC)
    target_compile_definitions(sample_tests PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(sample_tests PRIVATE
    ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${TINYXML2_INCLUDE_DIR})

target_link_libraries(sample_tests PUBLIC
    ${GTEST_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY}
    )

get_win32_path_dependencies(sample_tests TEST_FRIENDLY_PATH)

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

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(database.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)

endforeach()

###############################################################################
# Data tests
###############################################################################

add_executable(data_tests DataTests.cpp ${LIBRARY_SOURCES})

if(MSVC)
    target_compile_definitions(data_tests PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(data_tests PRIVATE
    ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${TINYXML2_INCLUDE_DIR})

target_link_libraries(data_tests PUBLIC
        ${GTEST_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY}
    )

get_win32_path_dependencies(data_tests TEST_FRIENDLY_PATH)

set(DATA_TEST_LIST
    domainparticipant_data_clear
    datareader_data_clear
    datawriter_data_clear
)

foreach(test_name ${DATA_TEST_LIST})
    add_test(NAME database.${test_name}
            COMMAND data_tests
            --gtest_filter=database.${test_name}:*/database.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(database.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)

endforeach()

###############################################################################
# Database tests
###############################################################################

add_executable(database_tests DatabaseTests.cpp ${LIBRARY_SOURCES})

if(MSVC)
    target_compile_definitions(database_tests PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
    # Duplications in this testing are intended
    target_compile_options(database_tests PRIVATE /wd4458)
endif(MSVC)

target_include_directories(database_tests PRIVATE
    ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${TINYXML2_INCLUDE_DIR})

target_link_libraries(database_tests PUBLIC
        ${GTEST_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY}
    )

get_win32_path_dependencies(database_tests TEST_FRIENDLY_PATH)

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
    insert_topic_two_same_domain_same_name_same_type
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
    insert_ddsendpoint_two_equal_locators
    # Insert invalid
    insert_invalid
    insert_locator
    insert_locator_two
    insert_locator_two_same_name
    insert_locator_duplicated
    insert_locator_empty_name
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
    insert_sample_rtps_packets_sent_unknown_remote_locator
    insert_sample_rtps_bytes_sent
    insert_sample_rtps_bytes_sent_wrong_entity
    insert_sample_rtps_bytes_sent_unknown_remote_locator
    insert_sample_rtps_packets_lost
    insert_sample_rtps_packets_lost_wrong_entity
    insert_sample_rtps_packets_lost_unknown_remote_locator
    insert_sample_rtps_bytes_lost
    insert_sample_rtps_bytes_lost_wrong_entity
    insert_sample_rtps_bytes_lost_unknown_remote_locator
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
    # Insert Monitor Service Samples
    insert_monitor_service_sample_invalid
    insert_monitor_service_sample_valid_wrong_domain
    insert_monitor_service_sample_proxy
    insert_monitor_service_sample_proxy_wrong_entity
    insert_monitor_service_sample_connection_list
    insert_monitor_service_sample_connection_list_wrong_entity
    insert_monitor_service_sample_incompatible_qos
    insert_monitor_service_sample_incompatible_qos_wrong_entity
    insert_monitor_service_sample_inconsistent_topic
    insert_monitor_service_sample_inconsistent_topic_wrong_entity
    insert_monitor_service_sample_liveliness_lost
    insert_monitor_service_sample_liveliness_lost_wrong_entity
    insert_monitor_service_sample_liveliness_changed
    insert_monitor_service_sample_liveliness_changed_wrong_entity
    insert_monitor_service_sample_deadline_missed
    insert_monitor_service_sample_deadline_missed_wrong_entity
    insert_monitor_service_sample_sample_lost
    insert_monitor_service_sample_sample_lost_wrong_entity
    insert_monitor_service_sample_extended_incompatible_qos
    insert_monitor_service_sample_extended_incompatible_qos_wrong_entity
    # get_invalid_status_data
    get_monitor_service_sample_invalid
    # entity_status_logic
    entity_status_logic
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
    get_entity_kind_by_guid
    # Update entity QoS
    update_entity_qos
    # select
    select_single_entity_invalid_needs_two_entities
    select_double_entity_invalid_needs_one_entity
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
    # get_entity_by_guid
    get_entity_by_guid_host
    get_entity_by_guid_user
    get_entity_by_guid_process
    get_entity_by_guid_domain
    get_entity_by_guid_participant
    get_entity_by_guid_participant_wrong_guid
    get_entity_by_guid_topic
    get_entity_by_guid_datawriter
    get_entity_by_guid_datawriter_wrong_guid
    get_entity_by_guid_datareader
    get_entity_by_guid_datareader_wrong_guid
    get_entity_by_guid_locator
    get_entity_by_guid_invalid
    get_entity_by_guid_other_kind
    # ros2_demangling
    check_ros2_demangler
    )

foreach(test_name ${DATABASE_TEST_LIST})
    add_test(NAME database_tests.${test_name}
            COMMAND database_tests
            --gtest_filter=database_tests.${test_name}:*/database_tests.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(database_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
endforeach()

###############################################################################
# Database process entities tests
###############################################################################

add_executable(database_process_entities_tests DatabaseProcessEntitiesTests.cpp ${LIBRARY_SOURCES})

if(MSVC)
    target_compile_definitions(database_process_entities_tests PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
    # Duplications in this testing are intended
    target_compile_options(database_process_entities_tests PRIVATE /wd4458)
endif(MSVC)

target_include_directories(database_process_entities_tests PRIVATE
    ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${TINYXML2_INCLUDE_DIR})

target_link_libraries(database_process_entities_tests PUBLIC
        ${GTEST_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY}
    )

get_win32_path_dependencies(database_process_entities_tests TEST_FRIENDLY_PATH)

set(DATABASE_PROCESS_ENTITIES_TEST_LIST
        insert_new_participant
        insert_new_participant_already_exists
        insert_new_participant_no_domain
        process_physical_entities
        process_physical_entities_no_link
        process_physical_entities_no_process
        process_physical_entities_no_process_no_user
        process_physical_entities_no_process_no_user_no_host
        process_physical_entities_process_throws
        process_physical_entities_user_throws
        process_physical_entities_host_throws
        is_topic_in_database
        insert_new_topic
        insert_new_topic_already_exists
        insert_new_topic_no_domain
        insert_new_endpoint_datawriter
        insert_new_endpoint_datareader
        insert_new_endpoint_already_exists
        insert_new_endpoint_no_topic
        insert_new_endpoint_no_participant
        insert_new_endpoint_no_locators
    )

foreach(test_name ${DATABASE_PROCESS_ENTITIES_TEST_LIST})
    add_test(NAME database_process_entities_tests.${test_name}
            COMMAND database_process_entities_tests
            --gtest_filter=database_process_entities_tests.${test_name}:*/database_process_entities_tests.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(database_process_entities_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
endforeach()

###############################################################################
# Database Dump tests
###############################################################################

add_executable(database_dump_tests DatabaseDumpTests.cpp ${LIBRARY_SOURCES})

if(MSVC)
    target_compile_definitions(database_dump_tests PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(database_dump_tests PRIVATE
    ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/test/unittest/TestUtils
    ${TINYXML2_INCLUDE_DIR})


target_link_libraries(database_dump_tests PUBLIC
        ${GTEST_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY}
    )

get_win32_path_dependencies(database_dump_tests TEST_FRIENDLY_PATH)

set(DATABASE_DUMP_TEST_LIST
    dump_empty_database
    dump_empty_entities_database
    dump_simple_database
    dump_complex_database
    dump_complex_database_reject_old_data
    dump_unlinked_database
    id_to_string
    time_to_string
    dump_no_process_participant_link
    dump_and_clear_database
    )

foreach(test_name ${DATABASE_DUMP_TEST_LIST})
    add_test(NAME database.${test_name}
            COMMAND database_dump_tests
            --gtest_filter=database.${test_name}:*/database.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(database.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)

    # Hint where the resource files can be found
    set_tests_properties(database.${test_name} PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

endforeach()

###############################################################################
# Database Clear tests
###############################################################################

check_gtest()
check_gmock()

# Specific sources as it needs StatisticsBackendData mock
set(LIBRARY_SOURCES_CLEAR_TEST
    ${PROJECT_SOURCE_DIR}/src/cpp/database/data.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/database_queue.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/database.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/entities.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/samples.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/subscriber/QosSerializer.cpp
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/monitorservice_typesTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/monitorservice_typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/typesTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/types/EntityId.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/types.cpp)

add_executable(database_clear_tests DatabaseClearTests.cpp ${LIBRARY_SOURCES_CLEAR_TEST})

if(MSVC)
    target_compile_definitions(database_clear_tests PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(database_clear_tests PRIVATE
    ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/test/mock/StatisticsBackend
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${TINYXML2_INCLUDE_DIR}
    )

target_link_libraries(database_clear_tests PUBLIC
        ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY}
    )

get_win32_path_dependencies(database_clear_tests TEST_FRIENDLY_PATH)

set(DATABASE_CLEAR_TEST_LIST
    clear_inactive_entities_database_simple
)

foreach(test_name ${DATABASE_CLEAR_TEST_LIST})
    add_test(NAME database.${test_name}
            COMMAND database_clear_tests
            --gtest_filter=database.${test_name}:*/database.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(database.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
endforeach()

###############################################################################
# Database: get_entities tests
###############################################################################

add_executable(database_get_entities_tests DatabaseGetEntitiesTest.cpp ${LIBRARY_SOURCES})

if(MSVC)
    target_compile_definitions(database_get_entities_tests PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(database_get_entities_tests PRIVATE
    ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${TINYXML2_INCLUDE_DIR})

target_link_libraries(database_get_entities_tests PUBLIC
        ${GTEST_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY}
    )

get_win32_path_dependencies(database_get_entities_tests TEST_FRIENDLY_PATH)

set(DATABASE_GET_ENTITIES_TEST_LIST
    get_entities
    get_entity_ids
)

foreach(test_name ${DATABASE_GET_ENTITIES_TEST_LIST})
    add_test(NAME database_get_entities_tests.${test_name}
            COMMAND database_get_entities_tests
            --gtest_filter=database_get_entities_tests.${test_name}:*/database_get_entities_tests.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(database_get_entities_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
endforeach()

###############################################################################
# Database: load_and_dump tests
###############################################################################

add_executable(database_load_tests DatabaseLoadTests.cpp ${LIBRARY_SOURCES})

if(MSVC)
    target_compile_definitions(database_load_tests PRIVATE
        _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(database_load_tests PRIVATE
    ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/test/unittest/TestUtils
    ${TINYXML2_INCLUDE_DIR})

target_link_libraries(database_load_tests PUBLIC
        ${GTEST_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY}
    )

get_win32_path_dependencies(database_load_tests TEST_FRIENDLY_PATH)

set(DATABASE_LOAD_TEST_LIST
    load_and_dump_empty_database
    load_and_dump_empty_entities_database
    load_and_dump_simple_database
    load_and_dump_complex_database
    load_and_dump_old_complex_database
    load_and_dump_complex_erased_database
    load_twice
    load_erased_keys
    load_wrong_values
    load_wrong_references
    string_to_int
    string_to_uint
)

foreach(test_name ${DATABASE_LOAD_TEST_LIST})
    add_test(NAME database_load_tests.${test_name}
            COMMAND database_load_tests
            --gtest_filter=database_load_tests.${test_name}:*/database_load_tests.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(database_load_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
endforeach()


###############################################################################
# Database: load tests
###############################################################################

add_executable(database_load_insert_tests DatabaseLoadInsertTests.cpp ${LIBRARY_SOURCES})

if(MSVC)
    target_compile_definitions(database_load_insert_tests PRIVATE
        _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(database_load_insert_tests PRIVATE
    ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/test/unittest/TestUtils
    ${TINYXML2_INCLUDE_DIR})

target_link_libraries(database_load_insert_tests PUBLIC
        ${GTEST_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY})

get_win32_path_dependencies(database_load_insert_tests TEST_FRIENDLY_PATH)

set(DATABASE_LOAD_INSERT_TEST_LIST
    load_insert
)

foreach(test_name ${DATABASE_LOAD_INSERT_TEST_LIST})
    add_test(NAME database_load_insert_tests.${test_name}
            COMMAND database_load_insert_tests
            --gtest_filter=database_load_insert_tests.${test_name}:*/database_load_insert_tests.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(database_load_insert_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
endforeach()

###############################################################################
# Database: status tests
###############################################################################

add_executable(database_status_tests DatabaseStatusTests.cpp ${LIBRARY_SOURCES})

if(MSVC)
    target_compile_definitions(database_status_tests PRIVATE
        _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(database_status_tests PRIVATE
    ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/test/unittest/TestUtils
    ${TINYXML2_INCLUDE_DIR})

target_link_libraries(database_status_tests PUBLIC
    ${GTEST_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY})

get_win32_path_dependencies(database_status_tests TEST_FRIENDLY_PATH)

set(DATABASE_STATUS_TEST_LIST
    initial_status
    host
    user
    process
    domain
    topic
    participant
    link_active_participant_with_inactive_process
    link_inactive_participant_with_active_process
    link_inactive_participant_with_inactive_process
    endpoints
    locator
)

foreach(test_name ${DATABASE_STATUS_TEST_LIST})
    add_test(NAME database_status_tests.${test_name}
            COMMAND database_status_tests
            --gtest_filter=database_status_tests.${test_name}:*/database_status_tests.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(database_status_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
endforeach()

###############################################################################
# Database: erase tests
###############################################################################

add_executable(database_erase_tests DatabaseEraseTests.cpp ${LIBRARY_SOURCES})

if(MSVC)
    target_compile_definitions(database_erase_tests PRIVATE
        _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(database_erase_tests PRIVATE
    ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/test/unittest/TestUtils
    ${TINYXML2_INCLUDE_DIR})

target_link_libraries(database_erase_tests PUBLIC
        ${GTEST_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY})

get_win32_path_dependencies(database_erase_tests TEST_FRIENDLY_PATH)

set(DATABASE_ERASE_TEST_LIST
    erase_domain_1
    erase_domain_2
    erase_domain_unlinked_participant_process
    erase_wrong_kind
)

foreach(test_name ${DATABASE_ERASE_TEST_LIST})
    add_test(NAME database_erase_tests.${test_name}
            COMMAND database_erase_tests
            --gtest_filter=database_erase_tests.${test_name}:*/database_erase_tests.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(database_erase_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
endforeach()

###############################################################################
# Database: database domain view graph tests
###############################################################################
# Specific sources as it needs StatisticsBackendData mock
add_executable(database_domain_view_graph_tests DatabaseDomainViewGraphTests.cpp
${PROJECT_SOURCE_DIR}/src/cpp/database/data.cpp
${PROJECT_SOURCE_DIR}/src/cpp/database/database_queue.cpp
${PROJECT_SOURCE_DIR}/src/cpp/database/database.cpp
${PROJECT_SOURCE_DIR}/src/cpp/database/entities.cpp
${PROJECT_SOURCE_DIR}/src/cpp/database/samples.cpp
${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
${PROJECT_SOURCE_DIR}/src/cpp/subscriber/QosSerializer.cpp
${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/monitorservice_typesTypeObjectSupport.cxx
${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/monitorservice_typesPubSubTypes.cxx
${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/typesPubSubTypes.cxx
${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/typesTypeObjectSupport.cxx
${PROJECT_SOURCE_DIR}/src/cpp/types/EntityId.cpp
${PROJECT_SOURCE_DIR}/src/cpp/types/types.cpp)

if(MSVC)
    target_compile_definitions(database_domain_view_graph_tests
        PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(database_domain_view_graph_tests PRIVATE
    ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/test/mock/StatisticsBackend
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/test/TestUtils
    ${TINYXML2_INCLUDE_DIR})

target_link_libraries(database_domain_view_graph_tests PUBLIC
    ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY})

get_win32_path_dependencies(database_domain_view_graph_tests TEST_FRIENDLY_PATH)

set(DATABASE_DOMAIN_VIEW_GRAPH_TEST_LIST
    complete_with_two_participants
    host_insert_failure
    user_insert_failure
    process_insert_failure
    regenerate_graph_functionality
    )

foreach(test_name ${DATABASE_DOMAIN_VIEW_GRAPH_TEST_LIST})
    add_test(NAME database_domain_view_graph_tests.${test_name}
            COMMAND database_domain_view_graph_tests
            --gtest_filter=database_domain_view_graph_tests.${test_name}:*/database_domain_view_graph_tests.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(database_domain_view_graph_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)

endforeach()
