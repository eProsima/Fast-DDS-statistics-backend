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

    add_definitions(
        -DBOOST_ASIO_STANDALONE
        -DASIO_STANDALONE
        )
        
    include_directories(${Asio_INCLUDE_DIR})

    ###############################################################################
    # Binaries
    ###############################################################################

    # Dynamic types test
    set(DDS_PUBLISHER_DYNAMIC_SOURCE
        PublisherDynamic.cpp
        )
    add_executable(DDSSimpleCommunicationDynamicPublisher ${DDS_PUBLISHER_DYNAMIC_SOURCE})
    target_compile_definitions(DDSSimpleCommunicationDynamicPublisher PRIVATE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_link_libraries(DDSSimpleCommunicationDynamicPublisher fastrtps fastcdr foonathan_memory ${CMAKE_DL_LIBS})

    set(DDS_SUBSCRIBER_DYNAMIC_SOURCE
        SubscriberDynamic.cpp
        )
    add_executable(DDSSimpleCommunicationDynamicSubscriber ${DDS_SUBSCRIBER_DYNAMIC_SOURCE})
    target_compile_definitions(DDSSimpleCommunicationDynamicSubscriber PRIVATE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_link_libraries(DDSSimpleCommunicationDynamicSubscriber fastrtps fastcdr foonathan_memory ${CMAKE_DL_LIBS})

    set(DDS_MONITOR_DYNAMIC_SOURCE
        MonitorDynamic.cpp
        )
    add_executable(DDSSimpleCommunicationDynamicMonitor ${DDS_MONITOR_DYNAMIC_SOURCE}
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

    target_include_directories(DDSSimpleCommunicationDynamicMonitor PRIVATE
        ${GTEST_INCLUDE_DIRS}
        ${GMOCK_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
        ${PROJECT_BINARY_DIR}/include
        ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
        ${PROJECT_SOURCE_DIR}/src/cpp
        ${PROJECT_SOURCE_DIR}/test/thirdparty/asio)

    target_compile_definitions(DDSSimpleCommunicationDynamicMonitor PRIVATE
        $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
        $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
        )
    target_link_libraries(DDSSimpleCommunicationDynamicMonitor fastrtps fastcdr foonathan_memory ${CMAKE_DL_LIBS})

    ###############################################################################
    # Necessary files
    ###############################################################################
    # Dynamic types test
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_communication_dynamic.py
        ${CMAKE_CURRENT_BINARY_DIR}/simple_communication_dynamic.py COPYONLY)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/example_type.xml
        ${CMAKE_CURRENT_BINARY_DIR}/example_type.xml COPYONLY)

    ###############################################################################
    # Tests specification
    ###############################################################################
    if(PYTHONINTERP_FOUND)
        # Dynamic types test
        add_test(NAME DDSSimpleCommunicationTypeDiscovery
            COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/simple_communication_dynamic.py)

        # Set test with label NoMemoryCheck
        set_property(TEST DDSSimpleCommunicationTypeDiscovery PROPERTY LABELS "NoMemoryCheck")

        set_property(TEST DDSSimpleCommunicationTypeDiscovery PROPERTY ENVIRONMENT
            "DDS_SIMPLE_COMMUNICATION_PUBLISHER_BIN=$<TARGET_FILE:DDSSimpleCommunicationDynamicPublisher>")
        set_property(TEST DDSSimpleCommunicationTypeDiscovery APPEND PROPERTY ENVIRONMENT
            "DDS_SIMPLE_COMMUNICATION_SUBSCRIBER_BIN=$<TARGET_FILE:DDSSimpleCommunicationDynamicSubscriber>")
        set_property(TEST DDSSimpleCommunicationTypeDiscovery APPEND PROPERTY ENVIRONMENT
            "DDS_SIMPLE_COMMUNICATION_MONITOR_BIN=$<TARGET_FILE:DDSSimpleCommunicationDynamicMonitor>")
        if(WIN32)
            string(REPLACE ";" "\\;" WIN_PATH "$ENV{PATH}")
            set_property(TEST DDSSimpleCommunicationTypeDiscovery APPEND PROPERTY ENVIRONMENT
                "PATH=$<TARGET_FILE_DIR:${PROJECT_NAME}>\\;$<TARGET_FILE_DIR:fastcdr>\\;${WIN_PATH}")
        endif()

endif()
