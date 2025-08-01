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


set(LIBRARY_SOURCES
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
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/monitorservice_typesTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/monitorservice_typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/typesPubSubTypes.cxx
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/topic_types/typesTypeObjectSupport.cxx
    ${PROJECT_SOURCE_DIR}/src/cpp/types/EntityId.cpp
    ${PROJECT_SOURCE_DIR}/src/cpp/types/types.cpp)

add_executable(entitymetatraffic_tests EntityMetatrafficTests.cpp ${LIBRARY_SOURCES})

if(MSVC)
    target_compile_definitions(entitymetatraffic_tests PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
endif(MSVC)

target_include_directories(entitymetatraffic_tests PRIVATE
    ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${TINYXML2_INCLUDE_DIR})

target_link_libraries(entitymetatraffic_tests PUBLIC ${GTEST_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY})

get_win32_path_dependencies(entitymetatraffic_tests TEST_FRIENDLY_PATH)

set(ENTITYMETATRAFFIC_TEST_LIST
    metatraffic_topic
    non_metatraffic_topic)

foreach(test_name ${ENTITYMETATRAFFIC_TEST_LIST})
    add_test(NAME entitymetatraffic.${test_name}
            COMMAND entitymetatraffic_tests
            --gtest_filter=entitymetatraffic.${test_name}:*/entitymetatraffic.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(entitymetatraffic.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
endforeach()
