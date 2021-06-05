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

check_gtest()

add_executable(TrafficInjectorTests TrafficInjectorTests.cpp)
target_include_directories(TrafficInjectorTests PRIVATE
    ${PROJECT_SOURCE_DIR}/src/cpp
    ${PROJECT_SOURCE_DIR}/test/TrafficInjector
    ${GTEST_INCLUDE_DIRS})

if(MSVC)
    target_compile_definitions(TrafficInjectorTests PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0)
endif(MSVC)

target_link_libraries(TrafficInjectorTests PUBLIC ${PROJECT_NAME} ${GTEST_LIBRARIES})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/traffic_dump.json
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(TRAFFIC_INJECTOR_TEST_LIST
    Basic)

foreach(test_name ${TRAFFIC_INJECTOR_TEST_LIST})
    add_test(NAME TrafficInjector.${test_name}
            COMMAND TrafficInjectorTests
            --gtest_filter=TrafficInjectorTests.${test_name}:*/TrafficInjectorTests.${test_name}/*)
endforeach()
