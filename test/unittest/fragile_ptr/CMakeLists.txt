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

if(GTEST_FOUND)
    add_executable(parametrized_fragile_ptr_tests
        fragile_ptr_tests.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/exception/Exception.cpp
    )

    if(MSVC)
        target_compile_definitions(parametrized_fragile_ptr_tests
            PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
    endif(MSVC)

    target_include_directories(parametrized_fragile_ptr_tests PRIVATE
        ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
        ${PROJECT_BINARY_DIR}/include
        ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
        ${PROJECT_SOURCE_DIR}/src/cpp
        ${TINYXML2_INCLUDE_DIR})

    target_link_libraries(parametrized_fragile_ptr_tests PUBLIC
        ${GTEST_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY})

    get_win32_path_dependencies(parametrized_fragile_ptr_tests TEST_FRIENDLY_PATH)

    set(QOSSERIALIZER_TEST_LIST
            expired
            reset
            null_comparison
            serialize
            use_ptr_negative
        )

    foreach(test_name ${QOSSERIALIZER_TEST_LIST})
        add_test(NAME parametrized_fragile_ptr_tests.${test_name}
                COMMAND parametrized_fragile_ptr_tests
                --gtest_filter=parametrized_fragile_ptr_tests**.${test_name}:**/parametrized_fragile_ptr_tests**.${test_name}/**)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(parametrized_fragile_ptr_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
    endforeach()
endif()
