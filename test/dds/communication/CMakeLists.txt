# Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
set(DDS_PUBLISHER_SOURCE
    Publisher.cpp
    )
add_executable(DDSSimpleCommunicationPublisher ${DDS_PUBLISHER_SOURCE})

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
target_link_libraries(DDSSimpleCommunicationPublisher fastrtps fastcdr foonathan_memory ${CMAKE_DL_LIBS})

get_win32_path_dependencies(DDSSimpleCommunicationPublisher TEST_FRIENDLY_PATH)

set(DDS_SUBSCRIBER_SOURCE
    Subscriber.cpp
    )
add_executable(DDSSimpleCommunicationSubscriber ${DDS_SUBSCRIBER_SOURCE})

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
target_link_libraries(DDSSimpleCommunicationSubscriber fastrtps fastcdr foonathan_memory ${CMAKE_DL_LIBS})

get_win32_path_dependencies(DDSSimpleCommunicationSubscriber TEST_FRIENDLY_PATH)

set(DDS_MONITOR_SOURCE
    Monitor.cpp
    )
add_executable(DDSSimpleCommunicationMonitor ${DDS_MONITOR_SOURCE}
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
    target_compile_definitions(DDSSimpleCommunicationMonitor PRIVATE
        _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC) 

target_include_directories(DDSSimpleCommunicationMonitor PRIVATE
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp)

target_compile_definitions(DDSSimpleCommunicationMonitor PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_link_libraries(DDSSimpleCommunicationMonitor fastrtps fastcdr foonathan_memory ${CMAKE_DL_LIBS})

get_win32_path_dependencies(DDSSimpleCommunicationMonitor TEST_FRIENDLY_PATH)

###############################################################################
# Necessary files
###############################################################################
#  types test
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_communication.py
    ${CMAKE_CURRENT_BINARY_DIR}/simple_communication.py COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/example_type.xml
    ${CMAKE_CURRENT_BINARY_DIR}/example_type.xml COPYONLY)

###############################################################################
# Tests specification
###############################################################################
if(PYTHONINTERP_FOUND)
    # types test
    add_test(NAME DDSSimpleCommunication
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/simple_communication.py)

    # Set test with label NoMemoryCheck
    set_property(TEST DDSSimpleCommunication PROPERTY LABELS "NoMemoryCheck")

    set_property(TEST DDSSimpleCommunication PROPERTY ENVIRONMENT
        "DDS_SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:DDSSimpleCommunicationPublisher>")
    set_property(TEST DDSSimpleCommunication APPEND PROPERTY ENVIRONMENT
        "DDS_SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:DDSSimpleCommunicationSubscriber>")
    set_property(TEST DDSSimpleCommunication APPEND PROPERTY ENVIRONMENT
        "DDS_SIMPLE_COMMUNICATION_MONITOR_BIN=$<TARGET_FILE:DDSSimpleCommunicationMonitor>")
    if(WIN32)
        string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
        set_property(TEST DDSSimpleCommunication APPEND PROPERTY ENVIRONMENT
            "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
    endif()

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(DDSSimpleCommunication PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)

endif()
