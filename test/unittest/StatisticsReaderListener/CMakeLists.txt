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
        new_history_latency_received
        )

    foreach(test_name ${STATISTICSDATALISTENER_TEST_LIST})
        add_test(NAME statistics_reader_listener_tests.${test_name}
                COMMAND statistics_reader_listener_tests
                --gtest_filter=statistics_reader_listener_tests.${test_name}:*/statistics_reader_listener_tests.${test_name}/*)
    endforeach()
endif()
