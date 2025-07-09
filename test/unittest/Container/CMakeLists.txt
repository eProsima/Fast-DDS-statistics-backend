# Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

if(GTEST_FOUND)

    #######################################################
    # DataContainer

    add_executable(parametrized_DataContainer_tests
        DataContainer_tests.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/database/samples.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/types/EntityId.cpp
    )

    if(MSVC)
        target_compile_definitions(parametrized_DataContainer_tests
            PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
    endif(MSVC)

    target_include_directories(parametrized_DataContainer_tests PRIVATE
        ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
        ${PROJECT_BINARY_DIR}/include
        ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
        ${PROJECT_SOURCE_DIR}/src/cpp
        ${TINYXML2_INCLUDE_DIR})

    target_link_libraries(parametrized_DataContainer_tests PUBLIC
        ${GTEST_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY})

    get_win32_path_dependencies(parametrized_DataContainer_tests TEST_FRIENDLY_PATH)

    set(QOSSERIALIZER_TEST_LIST
            trivial
            clear
            access_operator
            get_interval_limits
            find_by_timestamp_
        )

    foreach(test_name ${QOSSERIALIZER_TEST_LIST})
        add_test(NAME parametrized_DataContainer_tests.${test_name}
                COMMAND parametrized_DataContainer_tests
                --gtest_filter=parametrized_DataContainer_tests**.${test_name}:**/parametrized_DataContainer_tests**.${test_name}/**)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(parametrized_DataContainer_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
    endforeach()

    #######################################################
    # MapDataContainer

    add_executable(parametrized_MapDataContainer_tests
        MapDataContainer_tests.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/database/samples.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/types/EntityId.cpp
    )

    if(MSVC)
        target_compile_definitions(parametrized_MapDataContainer_tests
            PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
    endif(MSVC)

    target_include_directories(parametrized_MapDataContainer_tests PRIVATE
        ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
        ${PROJECT_BINARY_DIR}/include
        ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
        ${PROJECT_SOURCE_DIR}/src/cpp
        ${TINYXML2_INCLUDE_DIR})

    target_link_libraries(parametrized_MapDataContainer_tests PUBLIC
        ${GTEST_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY})

    get_win32_path_dependencies(parametrized_MapDataContainer_tests TEST_FRIENDLY_PATH)

    set(QOSSERIALIZER_TEST_LIST
            trivial
            clear
        )

    foreach(test_name ${QOSSERIALIZER_TEST_LIST})
        add_test(NAME parametrized_MapDataContainer_tests.${test_name}
                COMMAND parametrized_MapDataContainer_tests
                --gtest_filter=parametrized_MapDataContainer_tests**.${test_name}:**/parametrized_MapDataContainer_tests**.${test_name}/**)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(parametrized_MapDataContainer_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
    endforeach()

endif()
