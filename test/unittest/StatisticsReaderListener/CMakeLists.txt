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
    find_package(Threads REQUIRED)

    add_executable(statistics_reader_listener_tests StatisticsReaderListenerTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/topic_types/types.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/types/EntityId.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/samples.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/database_queue.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/subscriber/StatisticsReaderListener.cpp)

    target_include_directories(statistics_reader_listener_tests PRIVATE
        ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/test/mock/database/database
        ${PROJECT_SOURCE_DIR}/test/mock/dds/DataReader
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
        ${PROJECT_BINARY_DIR}/include
        ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
        ${PROJECT_SOURCE_DIR}/src/cpp)
            
    target_link_libraries(statistics_reader_listener_tests PUBLIC
        ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} fastrtps)

    set(STATISTICSDATALISTENER_TEST_LIST
        mask
        not_valid_data
        new_history_latency_received
        new_history_latency_received_not_in_mask
        new_network_latency_received
        new_network_latency_received_not_in_mask
        new_publication_throughput_received
        new_publication_throughput_received_not_in_mask
        new_subscription_throughput_received
        new_subscription_throughput_received_not_in_mask
        new_rtps_sent_received
        new_rtps_sent_received_not_in_mask
        new_rtps_sent_received_packets_not_in_mask
        new_rtps_sent_received_bytes_not_in_mask
        new_rtps_lost_received
        new_rtps_lost_received_not_in_mask
        new_rtps_lost_received_packets_not_in_mask
        new_rtps_lost_received_bytes_not_in_mask
        new_resent_datas_received
        new_resent_datas_received_not_in_mask
        new_heartbeat_count_received
        new_heartbeat_count_received_not_in_mask
        new_acknack_count_received
        new_acknack_count_received_not_in_mask
        new_nackfrag_count_received
        new_nackfrag_count_received_not_in_mask
        new_gap_count_received
        new_gap_count_received_not_in_mask
        new_data_count_received
        new_data_count_received_not_in_mask
        new_pdp_count_received
        new_pdp_count_received_not_in_mask
        new_edp_count_received
        new_edp_count_received_not_in_mask
        new_discovery_times_received
        new_discovery_times_received_not_in_mask
        new_sample_datas_received
        new_sample_datas_received_not_in_mask
        new_physical_data_received
        new_physical_data_received_no_mask
        )

    foreach(test_name ${STATISTICSDATALISTENER_TEST_LIST})
        add_test(NAME statistics_reader_listener_tests.${test_name}
                COMMAND statistics_reader_listener_tests
                --gtest_filter=statistics_reader_listener_tests.${test_name}:*/statistics_reader_listener_tests.${test_name}/*)
    endforeach()
endif()