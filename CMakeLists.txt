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

###############################################################################
# CMake build rules for Fast DDS Statistics Backend
###############################################################################
cmake_minimum_required(VERSION 3.5)
cmake_policy(VERSION 3.5...3.13)

###############################################################################
# Project
###############################################################################
project(fastdds-statistics-backend)

###############################################################################
# Load external projects.
###############################################################################
# No requirements

###############################################################################
# Test system configuration
###############################################################################
set(FORCE_CXX "14" CACHE STRING "C++ standard fulfillment selection")

###############################################################################
# Warning level
###############################################################################
if(MSVC OR MSVC_IDE)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
else()
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Wno-unknown-pragmas -Wno-error=deprecated-declarations")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-undefined,error")
    endif()
endif()

###############################################################################
# Documentation
###############################################################################
option(BUILD_DOCUMENTATION "Generate documentation" ON)
if (BUILD_DOCUMENTATION)
    add_subdirectory(docs)
endif()

###############################################################################
# Installation paths
###############################################################################
set(BIN_INSTALL_DIR bin/ CACHE PATH "Installation directory for binaries")
set(INCLUDE_INSTALL_DIR include/ CACHE PATH "Installation directory for C++ headers")
set(LIB_INSTALL_DIR lib/ CACHE PATH "Installation directory for libraries")
set(DATA_INSTALL_DIR share/ CACHE PATH "Installation directory for data")

###############################################################################
# Default shared libraries
###############################################################################
# Global flag to cause add_library() to create shared libraries if on.
# If set to true, this will cause all libraries to be built shared
# unless the library was explicitly added as a static library.
option(BUILD_SHARED_LIBS "Create shared libraries by default" ON)

###############################################################################
# Install headers
###############################################################################
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include
    ${ARGN}
    DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}
    COMPONENT headers
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp")

get_property(${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES GLOBAL PROPERTY ${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES_PROPERTY)
set(${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES ${${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES} ${PROJECT_SOURCE_DIR}/include)
set_property(GLOBAL PROPERTY ${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES_PROPERTY ${${PROJECT_NAME}_PUBLIC_HEADERS_DIRECTORIES})
