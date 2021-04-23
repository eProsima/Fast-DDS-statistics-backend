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

file(GLOB_RECURSE ALL_HEADERS
    ${PROJECT_SOURCE_DIR}/include/**/*.h
    ${PROJECT_SOURCE_DIR}/include/**/*.hpp
    ${PROJECT_SOURCE_DIR}/src/cpp/**/*.h
    ${PROJECT_SOURCE_DIR}/src/cpp/**/*.hpp
    )

set(${PROJECT_NAME}_SOURCES
    ${ALL_HEADERS}
    database/data.cpp
    database/database.cpp
    database/database_queue.cpp
    database/entities.cpp
    database/samples.cpp
    exception/Exception.cpp
    types/EntityId.cpp
    StatisticsBackend.cpp
    subscriber/StatisticsParticipantListener.cpp
    subscriber/StatisticsReaderListener.cpp
    topic_types/types.cxx
    topic_types/typesPubSubTypes.cxx
    )

# Configure config file
configure_file(${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/config.h.in
    ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}/config.h)

# Definitions
# If Windows, set define for export dll functions
add_definitions(-DFASTDDS_STATISTICS_BACKEND_SOURCE)
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0501)
endif()

# Create library
add_library(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    FASTDDS_STATISTICS_BACKEND_SOURCE
    EPROSIMA_USER_DLL_EXPORT
    )

# Define public headers
target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/exception>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/listener>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/nlohmann-json>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/types>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/subscriber>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/cpp>
        $<INSTALL_INTERFACE:include>
        )

target_link_libraries(${PROJECT_NAME}
        fastcdr
        fastrtps
        )

if(MSVC OR MSVC_IDE)
    set_target_properties(${PROJECT_NAME} PROPERTIES RELEASE_POSTFIX -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
    set_target_properties(${PROJECT_NAME} PROPERTIES RELWITHDEBINFO_POSTFIX -${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})
    set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX d-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR})

    get_target_property(TARGET_TYPE ${PROJECT_NAME} TYPE)
    if(TARGET_TYPE STREQUAL "SHARED_LIBRARY")
        # Export symbols in DLL library
        target_compile_definitions(${PROJECT_NAME} PUBLIC FASTDDS_STATISTICS_BACKEND_DYN_LINK)
    else()
        # Rename the library to have a "lib" before.
        set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME lib${PROJECT_NAME})
    endif()

    if("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
        set_target_properties(${PROJECT_NAME} PROPERTIES VS_WINRT_COMPONENT "true")
    endif()
endif()

###############################################################################
# Packaging
###############################################################################

# Install headers
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
    DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPONENT headers
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
    )

# Install config.h header
install(FILES ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}/config.h
DESTINATION ${INCLUDE_INSTALL_DIR}/${PROJECT_NAME}
COMPONENT headers
)

set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C++ Headers" PARENT_SCOPE)
set(CPACK_COMPONENT_HEADERS_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} C++ Headers" PARENT_SCOPE)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    COMPONENT libraries
    )

export(TARGETS ${PROJECT_NAME} FILE ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-targets.cmake)
install(EXPORT ${PROJECT_NAME}-targets
    DESTINATION ${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME}
    COMPONENT cmake
    )

if(MSVC OR MSVC_IDE)
    if(TARGET_TYPE STREQUAL "SHARED_LIBRARY")
        # Install pdb files
        install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Debug/
            DESTINATION ${LIB_INSTALL_DIR}
            COMPONENT libraries
            CONFIGURATIONS Debug
            FILES_MATCHING
            PATTERN "*.pdb"
            )
    else()
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dir/Debug/${PROJECT_NAME}.pdb
            DESTINATION ${LIB_INSTALL_DIR}
            COMPONENT libraries
            CONFIGURATIONS Debug
            )
    endif()
endif()

###############################################################################
# Create CMake package config file
###############################################################################
include(CMakePackageConfigHelpers)

configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/packaging/Config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    INSTALL_DESTINATION ${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME}
    PATH_VARS BIN_INSTALL_DIR INCLUDE_INSTALL_DIR LIB_INSTALL_DIR
    )
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
    )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    DESTINATION ${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME}
    COMPONENT cmake
    )

set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Libraries" PARENT_SCOPE)
set(CPACK_COMPONENT_LIBRARIES_DESCRIPTION "eProsima ${PROJECT_NAME_LARGE} libraries" PARENT_SCOPE)
