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

# statistics_backend_tests
##################################################################

add_executable(statistics_backend_tests StatisticsBackendTests.cpp
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
    ${PROJECT_SOURCE_DIR}/src/cpp/topic_types/types.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/topic_types/typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/types/EntityId.cpp)
    
target_include_directories(statistics_backend_tests PRIVATE
    ${GTEST_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/test/unittest/TestUtils)

target_link_libraries(statistics_backend_tests PUBLIC
    ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} fastrtps)

set(BACKEND_TEST_LIST
    get_info
    get_entities
    get_type
    get_data_supported_entity_kinds
    )

foreach(test_name ${BACKEND_TEST_LIST})
    add_test(NAME statistics_backend_tests.${test_name}
            COMMAND statistics_backend_tests
            --gtest_filter=statistics_backend_tests.${test_name}:*/statistics_backend_tests.${test_name}/*)
endforeach()

# calling_user_listeners
##################################################################

add_executable(calling_user_listeners_tests CallingUserListenersTests.cpp
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
    ${PROJECT_SOURCE_DIR}/src/cpp/topic_types/typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/topic_types/types.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/types/EntityId.cpp)

target_include_directories(calling_user_listeners_tests PRIVATE
    ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp)

target_link_libraries(calling_user_listeners_tests PUBLIC
    ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} fastrtps)

set(USER_LISTENERS_TEST_LIST
    host_discovered
    host_discovered_not_in_mask
    host_discovered_no_listener
    host_discovered_no_listener_not_in_mask
    user_discovered
    user_discovered_not_in_mask
    user_discovered_no_listener
    user_discovered_no_listener_not_in_mask
    process_discovered
    process_discovered_not_in_mask
    process_discovered_no_listener
    process_discovered_no_listener_not_in_mask
    locator_discovered
    locator_discovered_not_in_mask
    locator_discovered_no_listener
    locator_discovered_no_listener_not_in_mask
    participant_discovered
    participant_discovered_not_in_mask
    participant_discovered_no_listener
    participant_discovered_no_listener_not_in_mask
    topic_discovered
    topic_discovered_not_in_mask
    topic_discovered_no_listener
    topic_discovered_no_listener_not_in_mask
    datareader_discovered
    datareader_discovered_not_in_mask
    datareader_discovered_no_listener
    datareader_discovered_no_listener_not_in_mask
    datawriter_discovered
    datawriter_discovered_not_in_mask
    datawriter_discovered_no_listener
    datawriter_discovered_no_listener_not_in_mask
    wrong_entity_kind
    )

foreach(test_name ${USER_LISTENERS_TEST_LIST})
    add_test(NAME calling_user_listeners_tests.${test_name}
        COMMAND calling_user_listeners_tests
        --gtest_filter=calling_user_listeners_tests.${test_name}:*/calling_user_listeners_tests.${test_name}/*)
endforeach()

set(USER_DATA_LISTENERS_TEST_LIST
    data_available
    data_available_callback_not_in_mask
    data_available_data_not_in_mask
    data_available_no_listener
    data_available_no_listener_callback_not_in_mask
    data_available_no_listener_data_not_in_mask
    )

foreach(test_name ${USER_DATA_LISTENERS_TEST_LIST})
    add_test(NAME calling_user_listeners_tests.${test_name}
            COMMAND calling_user_listeners_tests
            --gtest_filter=calling_user_data_listeners_tests.${test_name}:*/calling_user_data_listeners_tests.${test_name}/*)
endforeach()

# init_monitor_tests
##################################################################

add_executable(init_monitor_tests InitMonitorTests.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/StatisticsBackend.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/subscriber/StatisticsParticipantListener.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/subscriber/StatisticsReaderListener.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/subscriber/QosSerializer.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/database_queue.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/database.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/samples.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/entities.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/database/data.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/EntityId.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/topic_types/types.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/topic_types/typesPubSubTypes.cxx)

target_include_directories(init_monitor_tests PRIVATE
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp)

target_link_libraries(init_monitor_tests PUBLIC
    ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} fastrtps)

set(INIT_MONITOR_TEST_LIST
    init_monitor_domain_id_all_callback_all_data
    init_monitor_domain_id_no_callback_all_data
    init_monitor_domain_id_all_callback_no_data
    init_monitor_domain_id_null_listener_all_data
    init_monitor_several_monitors
    init_monitor_dds_factory_fails
    )

foreach(test_name ${INIT_MONITOR_TEST_LIST})
    add_test(NAME init_monitor_tests.${test_name}
            COMMAND init_monitor_tests
            --gtest_filter=init_monitor_tests.${test_name}:*/init_monitor_tests.${test_name}/*)
endforeach()
