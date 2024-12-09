# Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

find_package(PythonInterp 3)

###############################################################################
# Binaries
###############################################################################

# types test
file(GLOB COMMUNICATION_SOURCES_CXX "*.cxx")
set(DDS_PUBLISHER_SOURCE
    Publisher.cpp
    )
add_executable(DDSSimpleCommunicationPublisher ${DDS_PUBLISHER_SOURCE} ${COMMUNICATION_SOURCES_CXX})

if(MSVC)
    target_compile_definitions(DDSSimpleCommunicationPublisher PRIVATE
        _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(DDSSimpleCommunicationPublisher PRIVATE
    ${PROJECT_SOURCE_DIR}/test/dds/communication)

target_compile_definitions(DDSSimpleCommunicationPublisher PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_link_libraries(DDSSimpleCommunicationPublisher fastdds fastcdr foonathan_memory ${CMAKE_DL_LIBS})

get_win32_path_dependencies(DDSSimpleCommunicationPublisher TEST_FRIENDLY_PATH)

set(DDS_SUBSCRIBER_SOURCE
    Subscriber.cpp
    )
add_executable(DDSSimpleCommunicationSubscriber ${DDS_SUBSCRIBER_SOURCE} ${COMMUNICATION_SOURCES_CXX})

if(MSVC)
    target_compile_definitions(DDSSimpleCommunicationSubscriber PRIVATE
        _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(DDSSimpleCommunicationSubscriber PRIVATE
    ${PROJECT_SOURCE_DIR}/test/dds/communication)

target_compile_definitions(DDSSimpleCommunicationSubscriber PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_link_libraries(DDSSimpleCommunicationSubscriber fastdds fastcdr foonathan_memory ${CMAKE_DL_LIBS})

get_win32_path_dependencies(DDSSimpleCommunicationSubscriber TEST_FRIENDLY_PATH)

set(DDS_MONITOR_SOURCE
    Monitor.cpp
    )
add_executable(DDSSimpleCommunicationMonitor ${DDS_MONITOR_SOURCE})

if(MSVC)
    target_compile_definitions(DDSSimpleCommunicationMonitor PRIVATE
        _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(DDSSimpleCommunicationMonitor PRIVATE
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS})

target_compile_definitions(DDSSimpleCommunicationMonitor PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_link_libraries(DDSSimpleCommunicationMonitor ${PROJECT_NAME} fastdds fastcdr foonathan_memory ${CMAKE_DL_LIBS})

get_win32_path_dependencies(DDSSimpleCommunicationMonitor TEST_FRIENDLY_PATH)

###############################################################################
# Necessary files
###############################################################################
#  types test
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_communication.py
    ${CMAKE_CURRENT_BINARY_DIR}/simple_communication.py COPYONLY)

###############################################################################
# Tests specification
###############################################################################
if(PYTHONINTERP_FOUND)
    # types test
    add_test(NAME BasicSystemTest
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/simple_communication.py)

    # Set test with label NoMemoryCheck
    set_property(TEST BasicSystemTest PROPERTY LABELS "NoMemoryCheck")

    set_property(TEST BasicSystemTest PROPERTY ENVIRONMENT
        "DDS_SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:DDSSimpleCommunicationPublisher>")
    set_property(TEST BasicSystemTest APPEND PROPERTY ENVIRONMENT
        "DDS_SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:DDSSimpleCommunicationSubscriber>")
    set_property(TEST BasicSystemTest APPEND PROPERTY ENVIRONMENT
        "DDS_SIMPLE_COMMUNICATION_MONITOR_BIN=$<TARGET_FILE:DDSSimpleCommunicationMonitor>")
    if(WIN32)
        string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
        set_property(TEST BasicSystemTest APPEND PROPERTY ENVIRONMENT
            "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
    endif()

    if(TEST_FRIENDLY_PATH)
        set_property(TEST BasicSystemTest APPEND PROPERTY ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)

    # Set as not ASAN test
    # NOTE: this test runs several processes from python, so ASAN report will not be related with the actual backend.
    set_property(TEST BasicSystemTest APPEND PROPERTY LABELS xasan)

endif()
