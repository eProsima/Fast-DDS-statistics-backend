# Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

cmake_minimum_required(VERSION 2.8.12)

if(NOT CMAKE_VERSION VERSION_LESS 3.0)
    cmake_policy(SET CMP0048 NEW)
endif()

project(ROS2Prometheus)

find_package(fastcdr REQUIRED)
find_package(fastdds REQUIRED)
find_package(fastdds_statistics_backend REQUIRED)
find_package(prometheus-cpp CONFIG REQUIRED)

#Check C++11
include(CheckCXXCompilerFlag)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
        CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
    if(NOT SUPPORTS_CXX11)
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()

message(STATUS "Configuring HelloWorld example...")
file(GLOB HELLOWORLD_EXAMPLE_SOURCES_CXX "*.cxx")
file(GLOB HELLOWORLD_EXAMPLE_SOURCES_CPP "*.cpp")

add_executable(ROS2Prometheus ${HELLOWORLD_EXAMPLE_SOURCES_CXX} ${HELLOWORLD_EXAMPLE_SOURCES_CPP})
target_compile_definitions(ROS2Prometheus PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
)
target_link_libraries(ROS2Prometheus fastdds fastcdr fastdds_statistics_backend prometheus-cpp::pull)
install(TARGETS ROS2Prometheus
    RUNTIME DESTINATION examples/cpp/ROS2Prometheus/${BIN_INSTALL_DIR})
