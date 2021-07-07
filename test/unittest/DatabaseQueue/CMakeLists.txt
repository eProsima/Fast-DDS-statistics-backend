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

include(${PROJECT_SOURCE_DIR}/cmake/common/gtest.cmake)
check_gtest()
check_gmock()

if(GTEST_FOUND AND GMOCK_FOUND)
    add_executable(database_queue_tests DatabaseQueueTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/topic_types/types.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/types/EntityId.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/data.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/entities.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/samples.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/database_queue.cpp)

    if(MSVC)
        target_compile_definitions(database_queue_tests
            PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
    endif(MSVC)

    target_include_directories(database_queue_tests PRIVATE
        ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/test/mock/database/database
        ${PROJECT_SOURCE_DIR}/test/mock/StatisticsBackend
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
        ${PROJECT_BINARY_DIR}/include
        ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
        ${PROJECT_SOURCE_DIR}/src/cpp)

    target_link_libraries(database_queue_tests PUBLIC
        ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} fastrtps fastcdr)

    get_win32_path_dependencies(database_queue_tests TEST_FRIENDLY_PATH)

    set(DATABASEQUEUE_TEST_LIST
        start_stop_flush
        push_host
        push_host_throws
        push_user
        push_user_throws
        push_process
        push_process_throws
        push_domain
        push_domain_throws
        push_participant_process_exists
        push_participant_no_process_exists
        push_participant_throws
        push_topic
        push_topic_throws
        push_datawriter
        push_datawriter_throws
        push_datareader
        push_datareader_throws
        push_locator
        push_locator_throws
        push_history_latency
        push_history_latency_no_reader
        push_history_latency_no_writer
        push_network_latency
        push_network_latency_no_source_locator
        push_network_latency_no_destination_locator
        push_network_latency_no_locators
        push_publication_throughput
        push_publication_throughput_no_writer
        push_subscription_throughput
        push_subscription_throughput_no_reder
        push_rtps_sent
        push_rtps_sent_no_writer
        push_rtps_sent_no_locator
        push_rtps_lost
        push_rtps_lost_no_writer
        push_rtps_lost_no_locator
        push_resent_datas
        push_resent_datas_no_writer
        push_heartbeat_count
        push_heartbeat_count_no_writer
        push_acknack_count
        push_acknack_count_no_reader
        push_nackfrag_count
        push_nackfrag_count_no_reader
        push_gap_count
        push_gap_count_no_writer
        push_data_count
        push_data_count_no_writer
        push_pdp_count
        push_pdp_count_no_participant
        push_edp_count
        push_edp_count_no_participant
        push_discovery_times
        push_discovery_times_no_participant
        push_discovery_times_no_entity
        push_sample_datas
        push_sample_datas_no_writer
        push_physical_data_process_exists
        push_physical_data_no_participant_exists
        push_physical_data_no_process_exists
        push_physical_data_no_process_exists_process_insert_throws
        push_physical_data_no_process_no_user_exists
        push_physical_data_no_process_no_user_exists_user_insert_throws
        push_physical_data_no_process_no_user_no_host_exists
        push_physical_data_no_process_no_user_no_host_exists_host_insert_throws
        push_physical_data_wrong_processname_format
        )

    foreach(test_name ${DATABASEQUEUE_TEST_LIST})
        add_test(NAME database_queue_tests.${test_name}
                COMMAND database_queue_tests
                --gtest_filter=database_queue_tests.${test_name}:*/database_queue_tests.${test_name}/*)

        if(TEST_FRIENDLY_PATH)
            set_tests_properties(database_queue_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
        endif(TEST_FRIENDLY_PATH)

    endforeach()
endif()
