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

    add_executable(user_data_reader_listener_tests UserDataReaderListenerTests.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/subscriber/UserDataReaderListener.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/subscriber/UserDataContext.cpp
    )

    target_include_directories(user_data_reader_listener_tests PRIVATE
        ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/test/mock/dds/DataReader
        ${PROJECT_SOURCE_DIR}/test/mock/dds/Subscriber
        ${PROJECT_SOURCE_DIR}/test/types/UserData
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
        ${PROJECT_BINARY_DIR}/include
        ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
        ${PROJECT_SOURCE_DIR}/src/cpp
    )

    if(MSVC)
        target_compile_definitions(user_data_reader_listener_tests
            PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
    endif(MSVC)

    target_link_libraries(user_data_reader_listener_tests PUBLIC
        ${GTEST_LIBRARIES} ${GMOCK_LIBRARIES} fastdds fastcdr)

    get_win32_path_dependencies(user_data_reader_listener_tests TEST_FRIENDLY_PATH)

    set(USERDATAREADERLISTENER_TEST_LIST
        process_data_from_known_type
        process_data_from_unknown_type
        process_invalid_sample
    )

    foreach(test_name ${USERDATAREADERLISTENER_TEST_LIST})
        add_test(NAME user_data_reader_listener_tests.${test_name}
                COMMAND user_data_reader_listener_tests
                --gtest_filter=user_data_reader_listener_tests.${test_name}:*/user_data_reader_listener_tests.${test_name}/*)

        if(TEST_FRIENDLY_PATH)
            set_tests_properties(user_data_reader_listener_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
        endif(TEST_FRIENDLY_PATH)
    endforeach()
endif()
