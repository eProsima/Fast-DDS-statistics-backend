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

# Auxiliary files
##################################################################

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/profile.xml
    ${CMAKE_CURRENT_BINARY_DIR}/profile.xml
    COPYONLY)

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

if(MSVC)
    target_compile_definitions(statistics_backend_tests
        PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(statistics_backend_tests PRIVATE
    ${GTEST_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp)

target_link_libraries(statistics_backend_tests PUBLIC
    ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} fastrtps fastcdr)

get_win32_path_dependencies(statistics_backend_tests TEST_FRIENDLY_PATH)

set(BACKEND_TEST_LIST
    get_info
    get_entities
    get_type
    get_data_supported_entity_kinds
    set_alias
    internal_callbacks_negative_cases
    set_listener_non_existent_monitor
    )

foreach(test_name ${BACKEND_TEST_LIST})
    add_test(NAME statistics_backend_tests.${test_name}
            COMMAND statistics_backend_tests
            --gtest_filter=statistics_backend_tests.${test_name}:*/statistics_backend_tests.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(statistics_backend_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)

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
    ${PROJECT_SOURCE_DIR}/src/cpp/topic_types/types.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/topic_types/typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/types/EntityId.cpp)

target_include_directories(calling_user_listeners_tests PRIVATE
    ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp)

target_link_libraries(calling_user_listeners_tests PUBLIC
    ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} fastrtps fastcdr)

if(MSVC)
    target_compile_definitions(calling_user_listeners_tests
        PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

get_win32_path_dependencies(calling_user_listeners_tests TEST_FRIENDLY_PATH)

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
    )

foreach(test_name ${USER_LISTENERS_TEST_LIST})

    add_test(NAME calling_user_listeners_tests.${test_name}
        COMMAND calling_user_listeners_tests
        --gtest_filter=calling_user_listeners_tests.${test_name}:*/calling_user_listeners_tests.${test_name}/*
    )

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(calling_user_listeners_tests.${test_name}
            PROPERTIES
                ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}"
        )
    endif(TEST_FRIENDLY_PATH)

endforeach()

set(USER_LISTENERS_DEATH_TEST_LIST
    wrong_entity_kind
    )

foreach(test_name ${USER_LISTENERS_DEATH_TEST_LIST})
    add_test(NAME calling_user_listeners_DeathTest.${test_name}
        COMMAND calling_user_listeners_tests
        --gtest_filter=calling_user_listeners_DeathTest.${test_name}:*/calling_user_listeners_DeathTest.${test_name}/*)

    set_tests_properties(calling_user_listeners_DeathTest.${test_name}
        PROPERTIES
            DISABLED $<AND:$<STREQUAL:${test_name},wrong_entity_kind>,$<NOT:$<CONFIG:Debug>>>
    )

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(calling_user_listeners_DeathTest.${test_name}
            PROPERTIES
                ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}"
        )
    endif(TEST_FRIENDLY_PATH)
endforeach()

set(USER_LISTENERS_TEST_LIST_PHYSICAL_ENTITIES
    entity_discovered
    entity_discovered_not_in_mask
    entity_discovered_no_listener
    entity_discovered_no_listener_not_in_mask
    )

foreach(test_name ${USER_LISTENERS_TEST_LIST_PHYSICAL_ENTITIES})

    add_test(NAME calling_user_listeners_tests_physical_entities.${test_name}
        COMMAND calling_user_listeners_tests
        --gtest_filter=calling_user_listeners_tests_physical_entities.${test_name}:*/calling_user_listeners_tests_physical_entities.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(calling_user_listeners_tests_physical_entities.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)

endforeach()

    set(USER_LISTENERS_TEST_LIST_DOMAIN_ENTITIES
    entity_discovered
    entity_discovered_not_in_mask
    entity_discovered_no_listener
    entity_discovered_no_listener_not_in_mask
    )

foreach(test_name ${USER_LISTENERS_TEST_LIST_DOMAIN_ENTITIES})

    add_test(NAME calling_user_listeners_tests_domain_entities.${test_name}
        COMMAND calling_user_listeners_tests
        --gtest_filter=calling_user_listeners_tests_domain_entities.${test_name}:*/calling_user_listeners_tests_domain_entities.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(calling_user_listeners_tests_domain_entities.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)

endforeach()

set(USER_LISTENERS_TEST_LIST_DATAS
    data_available
    data_available_callback_not_in_mask
    data_available_data_not_in_mask
    data_available_no_listener
    data_available_no_listener_callback_not_in_mask
    data_available_no_listener_data_not_in_mask
    )

foreach(test_name ${USER_LISTENERS_TEST_LIST_DATAS})

    add_test(NAME calling_user_listeners_tests_datas.${test_name}
            COMMAND calling_user_listeners_tests
            --gtest_filter=calling_user_listeners_tests_datas.${test_name}:*/calling_user_listeners_tests_datas.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(calling_user_listeners_tests_datas.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)

endforeach()


# init_monitor_tests
##################################################################

add_executable(init_monitor_tests InitMonitorTests.cpp
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

if(MSVC)
    target_compile_definitions(init_monitor_tests
        PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(init_monitor_tests PRIVATE
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp)

target_link_libraries(init_monitor_tests PUBLIC
    ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} fastrtps fastcdr)

get_win32_path_dependencies(init_monitor_tests TEST_FRIENDLY_PATH)

set(INIT_MONITOR_TEST_LIST
    init_monitor_domain_id_all_callback_all_data
    init_monitor_domain_id_no_callback_all_data
    init_monitor_domain_id_all_callback_no_data
    init_monitor_domain_id_null_listener_all_data
    init_monitor_several_monitors
    init_monitor_twice
    stop_monitor
    )

foreach(test_name ${INIT_MONITOR_TEST_LIST})
    add_test(NAME init_monitor_tests.${test_name}
            COMMAND init_monitor_tests
            --gtest_filter=init_monitor_tests.${test_name}:*/init_monitor_tests.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(init_monitor_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
endforeach()

# init_monitor_factory_fails_tests
##################################################################

add_executable(init_monitor_factory_fails_tests InitMonitorFactoryFailsTests.cpp
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

if(MSVC)
    target_compile_definitions(init_monitor_factory_fails_tests
        PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(init_monitor_factory_fails_tests PRIVATE
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/test/mock/dds/DataReader
    ${PROJECT_SOURCE_DIR}/test/mock/dds/DomainParticipant
    ${PROJECT_SOURCE_DIR}/test/mock/dds/DomainParticipantFactory
    ${PROJECT_SOURCE_DIR}/test/mock/dds/Subscriber
    ${PROJECT_SOURCE_DIR}/test/mock/dds/Topic
    ${PROJECT_SOURCE_DIR}/test/mock/dds/TopicDescription
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp)

target_link_libraries(init_monitor_factory_fails_tests PUBLIC
    ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} fastrtps fastcdr)

get_win32_path_dependencies(init_monitor_factory_fails_tests TEST_FRIENDLY_PATH)

set(INIT_MONITOR_TEST_LIST
    init_monitor_participant_creation_fails
    init_monitor_subscriber_creation_fails
    init_monitor_datareader_creation_fails
    init_monitor_topic_creation_fails
    init_monitor_register_type_fails
    init_monitor_topic_exists
    init_monitor_topic_exists_with_another_type
    )

foreach(test_name ${INIT_MONITOR_TEST_LIST})
    add_test(NAME init_monitor_factory_fails_tests.${test_name}
            COMMAND init_monitor_factory_fails_tests
            --gtest_filter=init_monitor_factory_fails_tests.${test_name}:*/init_monitor_factory_fails_tests.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(init_monitor_factory_fails_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
endforeach()

# get_data
##################################################################

add_executable(get_data_tests GetDataTests.cpp
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

if(MSVC)
    target_compile_definitions(get_data_tests
        PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(get_data_tests PRIVATE
    ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp)

target_link_libraries(get_data_tests PUBLIC
    ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} fastrtps fastcdr)

get_win32_path_dependencies(get_data_tests TEST_FRIENDLY_PATH)

set(GET_DATA_NO_DATA_TEST_LIST
    no_data
    )

foreach(test_name ${GET_DATA_NO_DATA_TEST_LIST})
    add_test(NAME get_data_no_data_tests.${test_name}
        COMMAND get_data_tests
        --gtest_filter=get_data_no_data_tests.${test_name}:*/get_data_no_data_tests.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(get_data_no_data_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
endforeach()

set(GET_DATA_WITH_DATA_TEST_LIST
    invalid_entity_kind
    nonexistent_entity_id
    get_sum_data
    get_max_data
    get_min_data
    get_count_data
    get_mean_data
    get_median_data
    get_stdev_data
    get_none_data
    )

foreach(test_name ${GET_DATA_WITH_DATA_TEST_LIST})
    add_test(NAME get_data_with_data_tests.${test_name}
        COMMAND get_data_tests
        --gtest_filter=get_data_with_data_tests.${test_name}:*/get_data_with_data_tests.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(get_data_with_data_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
endforeach()

set(GET_DATAUNSUPPORTED_ENTITY_TEST_LIST
    unsupported_entity_kind
    )

foreach(test_name ${GET_DATAUNSUPPORTED_ENTITY_TEST_LIST})
    add_test(NAME get_data_unsupported_entities_tests.${test_name}
        COMMAND get_data_tests
        --gtest_filter=get_data_unsupported_entities_tests.${test_name}:*/get_data_unsupported_entities_tests.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(get_data_unsupported_entities_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
endforeach()

# is_active_tests
###############################################################################

if(GTEST_FOUND AND GMOCK_FOUND)
    find_package(Threads REQUIRED)
        
    add_executable(is_active_tests IsActiveTests.cpp
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

    if(MSVC)
        target_compile_definitions(is_active_tests
            PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
    endif(MSVC)

    target_include_directories(is_active_tests PRIVATE
        ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/test/unittest/TestUtils
        ${PROJECT_SOURCE_DIR}/test/mock/dds/DomainParticipant
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
        ${PROJECT_BINARY_DIR}/include
        ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
        ${PROJECT_SOURCE_DIR}/src/cpp)

    target_link_libraries(is_active_tests PUBLIC
        ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} fastrtps fastcdr)

    get_win32_path_dependencies(is_active_tests TEST_FRIENDLY_PATH)

    set(IS_ACTIVE_TEST_LIST
        participant
        datawriter
        datareader
        endpoints
        discover_datawriter_on_inactive_domain
        discover_datareader_on_inactive_domain
    )

    foreach(test_name ${IS_ACTIVE_TEST_LIST})
        add_test(NAME is_active_tests.${test_name}
                COMMAND is_active_tests
                --gtest_filter=is_active_tests.${test_name}:*/is_active_tests.${test_name}/*)

        if(TEST_FRIENDLY_PATH)
            set_tests_properties(is_active_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
        endif(TEST_FRIENDLY_PATH)
    endforeach()
endif()

# load_database and dump_database
##################################################################

add_executable(backend_dump_tests BackendDumpTests.cpp
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

if(MSVC)
    target_compile_definitions(backend_dump_tests
        PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(backend_dump_tests PRIVATE
    ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp)

target_link_libraries(backend_dump_tests PUBLIC
    ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} fastrtps fastcdr)

get_win32_path_dependencies(backend_dump_tests TEST_FRIENDLY_PATH)

set(BACKEND_DUMP_TEST_LIST
    database_dump_load
    database_dump_and_clear
    reset
    )

foreach(test_name ${BACKEND_DUMP_TEST_LIST})
    add_test(NAME backend_dump_tests.${test_name}
        COMMAND backend_dump_tests
        --gtest_filter=backend_dump_tests.${test_name}:*/backend_dump_tests.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(backend_dump_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)

endforeach()

