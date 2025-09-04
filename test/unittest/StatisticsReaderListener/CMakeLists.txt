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
    ${PROJECT_SOURCE_DIR}/src/cpp/database/data.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/database_queue.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/entities.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/samples.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/subscriber/StatisticsReaderListener.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/subscriber/QosSerializer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/subscriber/ProxyDiscoveryInfo.cpp
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/monitorservice_typesTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/monitorservice_typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/typesTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/types/EntityId.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/types.cpp)

    target_include_directories(statistics_reader_listener_tests PRIVATE
        ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/test/mock/StatisticsBackend
        ${PROJECT_SOURCE_DIR}/test/mock/database/database
        ${PROJECT_SOURCE_DIR}/test/mock/dds/DataReader
        ${PROJECT_SOURCE_DIR}/test/mock/dds/Subscriber
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
        ${PROJECT_BINARY_DIR}/include
        ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
        ${PROJECT_SOURCE_DIR}/src/cpp
        ${TINYXML2_INCLUDE_DIR})

    if(MSVC)
        target_compile_definitions(statistics_reader_listener_tests
            PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
    endif(MSVC)

    target_link_libraries(statistics_reader_listener_tests PUBLIC
        ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY})

    get_win32_path_dependencies(statistics_reader_listener_tests TEST_FRIENDLY_PATH)

    set(STATISTICSDATALISTENER_TEST_LIST
        not_valid_data
        new_history_latency_received
        new_network_latency_received
        new_publication_throughput_received
        new_subscription_throughput_received
        new_rtps_sent_received
        new_rtps_lost_received
        new_resent_datas_received
        new_heartbeat_count_received
        new_acknack_count_received
        new_nackfrag_count_received
        new_gap_count_received
        new_data_count_received
        new_pdp_count_received
        new_edp_count_received
        new_discovery_times_received
        new_sample_datas_received
        new_monitor_service_sample_received
        )

    foreach(test_name ${STATISTICSDATALISTENER_TEST_LIST})
        add_test(NAME statistics_reader_listener_tests.${test_name}
                COMMAND statistics_reader_listener_tests
                --gtest_filter=statistics_reader_listener_tests.${test_name}:*/statistics_reader_listener_tests.${test_name}/*)

        if(TEST_FRIENDLY_PATH)
            set_tests_properties(statistics_reader_listener_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
        endif(TEST_FRIENDLY_PATH)
    endforeach()
endif()
