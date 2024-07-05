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

    add_executable(statistics_participant_listener_tests StatisticsParticipantListenerTests.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/database/data.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/database/database_queue.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/database/entities.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/database/samples.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/subscriber/QosSerializer.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/subscriber/StatisticsParticipantListener.cpp
        ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/monitorservice_typesTypeObjectSupport.cxx
        ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/monitorservice_typesPubSubTypes.cxx
        ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/typesPubSubTypes.cxx
        ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/typesTypeObjectSupport.cxx
        ${PROJECT_SOURCE_DIR}/src/cpp/types/EntityId.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/types/types.cpp)

    if(MSVC)
        target_compile_definitions(statistics_participant_listener_tests
            PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
    endif(MSVC)

    target_include_directories(statistics_participant_listener_tests PRIVATE
        ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/test/mock/StatisticsBackend
        ${PROJECT_SOURCE_DIR}/test/mock/database/database
        ${PROJECT_SOURCE_DIR}/test/mock/dds/DomainParticipant
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
        ${PROJECT_BINARY_DIR}/include
        ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
        ${PROJECT_SOURCE_DIR}/src/cpp)

    target_link_libraries(statistics_participant_listener_tests PUBLIC
        ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} fastdds fastcdr)

    get_win32_path_dependencies(statistics_participant_listener_tests TEST_FRIENDLY_PATH)

    set(STATISTICSPARTICIPANTLISTENER_TEST_LIST
        new_participant_discovered
        new_participant_discovered_empty_name_no_locators
        new_participant_discovered_empty_name_default_unicast_locator
        new_participant_discovered_empty_name_metatraffic_unicast_locator
        new_participant_discovered_empty_name_default_multicast_locator
        new_participant_discovered_empty_name_metatraffic_multicast_locator
        new_participant_discovered_empty_name_localhost_locators
        new_participant_discovered_not_first
        new_participant_discovered_participant_already_exists
        new_participant_no_domain
        new_participant_undiscovered
        new_participant_undiscovered_participant_already_exists
        new_reader_discovered
        new_reader_discovered_reader_already_exists
        new_reader_no_domain
        new_reader_no_participant
        new_reader_no_topic
        new_reader_several_locators
        new_reader_several_locators_no_host
        new_reader_several_topics
        new_reader_undiscovered
        new_reader_undiscovered_reader_already_exists
        new_writer_discovered
        new_writer_discovered_writer_already_exists
        new_writer_no_domain
        new_writer_no_participant
        new_writer_no_topic
        new_writer_several_locators
        new_writer_several_locators_no_host
        new_writer_undiscovered
        new_writer_undiscovered_writer_already_exists
        )

    foreach(test_name ${STATISTICSPARTICIPANTLISTENER_TEST_LIST})
        add_test(NAME statistics_participant_listener_tests.${test_name}
                COMMAND statistics_participant_listener_tests
                --gtest_filter=statistics_participant_listener_tests.${test_name}:*/statistics_participant_listener_tests.${test_name}/*)

        if(TEST_FRIENDLY_PATH)
            set_tests_properties(statistics_participant_listener_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
        endif(TEST_FRIENDLY_PATH)
    endforeach()
endif()
