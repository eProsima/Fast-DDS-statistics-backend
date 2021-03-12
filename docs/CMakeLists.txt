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

####################################################################################################
# Build Doxygen documentation
####################################################################################################

file(GLOB_RECURSE HPP_FILES "${PROJECT_SOURCE_DIR}/include/**/*.h*")
set(PROJECT_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_SOURCE_DOCS_DIR ${PROJECT_SOURCE_DIR}/docs)
set(PROJECT_BINARY_DOCS_DIR ${PROJECT_BINARY_DIR}/docs)

find_package(Doxygen REQUIRED)
# Create doxygen directories
add_custom_target(doc-dirs
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DOCS_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DOCS_DIR}/doxygen
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DOCS_DIR}/html
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DOCS_DIR}/doxygen/html
    COMMENT "Creating documentation directories" VERBATIM)

# Doxygen related variables
set(DOXYGEN_INPUT_DIR "${PROJECT_INCLUDE_DIR}")
set(DOXYGEN_OUTPUT_DIR "${PROJECT_BINARY_DOCS_DIR}/doxygen")
set(DOXYGEN_INDEX_FILE "${PROJECT_BINARY_DOCS_DIR}/doxygen/xml/index.xml")
set(DOXYFILE_IN "${CMAKE_CURRENT_SOURCE_DIR}/doxygen-config.in")
set(DOXYFILE_OUT ${PROJECT_BINARY_DOCS_DIR}/doxygen-config)

# Configure doxygen
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

# Doxygen command
add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
    DEPENDS ${HPP_FILES}
    MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
    COMMENT "Generating doxygen documentation")

# Generate API reference
add_custom_target(doxygen ALL
    DEPENDS ${DOXYGEN_INDEX_FILE}
    COMMENT "Generated API documentation with doxygen" VERBATIM)
add_dependencies(doxygen doc-dirs)

# Install doxygen generated XML files
install(DIRECTORY ${DOXYGEN_OUTPUT_DIR}/xml
    DESTINATION docs/doxygen
    COMPONENT backend-doxygen)
set(CPACK_COMPONENT_backend-doxygen_DISPLAY_NAME "Fast DDS Statistics Backend doxygen")
set(CPACK_COMPONENT_backend-doxygen_DESCRIPTION
    "eProsima Fast DDS Statistics Backend  doxygen documentation")
set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} doxygen)

####################################################################################################
# Build Sphinx documentation
####################################################################################################

# Find sphinx
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
find_package(Sphinx REQUIRED)

# Users can specify the sphinx builder with -DFASTDDS_STATISTICS_BACKEND_DOCS_BUILDER. This option
# takes one value of the lists ALLOWED_BUILDERS or TWO_STEP_BUILDERS. This second list is for
# targets that require two steps in order to build the documentation. This way, the CMake poject can
# cover all the options that the Makefile had. FASTDDS_STATISTICS_BACKEND_DOCS_BUILDER defaults to
# html
set(ALLOWED_BUILDERS
    html dirhtml singlehtml pickle json htmlhelp
    qthelp applehelp devhelp epub epub3 latex
    text man texinfo gettext changes xml pseudoxml pdf)
set(TWO_STEP_BUILDERS latexpdf latexpdfja info)

set(FASTDDS_STATISTICS_BACKEND_DOCS_BUILDER html CACHE STRING "Select sphinx builder")
set_property(
    CACHE FASTDDS_STATISTICS_BACKEND_DOCS_BUILDER
    PROPERTY STRINGS "${ALLOWED_BUILDERS};${TWO_STEP_BUILDERS}")

# Check that specified value is allowed
message(STATUS "Selected builder: ${FASTDDS_STATISTICS_BACKEND_DOCS_BUILDER}")
if((NOT FASTDDS_STATISTICS_BACKEND_DOCS_BUILDER IN_LIST ALLOWED_BUILDERS) AND
        (NOT FASTDDS_STATISTICS_BACKEND_DOCS_BUILDER IN_LIST TWO_STEP_BUILDERS))
    message(FATAL_ERROR,
        "Wrong configuration of FASTDDS_STATISTICS_BACKEND_DOCS_BUILDER.
        Allowed values: ${ALLOWED_BUILDERS};${TWO_STEP_BUILDERS}")
endif()

# Check whether a second step is required, and set option accordingly
if("${FASTDDS_STATISTICS_BACKEND_DOCS_BUILDER}" STREQUAL "latexpdf")
    set(INTERNAL_BUILDER "latex")
    set(SECOND_STEP_ARGS "all-pdf")
    set(DO_SECOND_STEP ON)
elseif("${FASTDDS_STATISTICS_BACKEND_DOCS_BUILDER}" STREQUAL "latexpdfja")
    set(INTERNAL_BUILDER "latex")
    set(SECOND_STEP_ARGS "all-pdf-ja")
    set(DO_SECOND_STEP ON)
elseif("${FASTDDS_STATISTICS_BACKEND_DOCS_BUILDER}" STREQUAL "info")
    set(INTERNAL_BUILDER "texinfo")
    set(SECOND_STEP_ARGS "info")
    set(DO_SECOND_STEP ON)
else()
    set(INTERNAL_BUILDER ${FASTDDS_STATISTICS_BACKEND_DOCS_BUILDER})
    set(DO_SECOND_STEP OFF)
endif()

# Generate the sphinx documentation
add_custom_target(Sphinx ALL
    COMMAND
    ${SPHINX_EXECUTABLE} -b ${INTERNAL_BUILDER}
    # Tell Breathe where to find the Doxygen output
    -D breathe_projects.fastdds-statistics-backend=${DOXYGEN_OUTPUT_DIR}/xml
    -d "${PROJECT_BINARY_DOCS_DIR}/doctrees"
    ${PROJECT_SOURCE_DOCS_DIR}
    ${PROJECT_BINARY_DOCS_DIR}/${INTERNAL_BUILDER}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS doxygen
    COMMENT "Generating documentation with Sphinx")

# Do the second step if needed
if(DO_SECOND_STEP)
    find_program(MAKE make DOC "path to make executable")
    message(STATUS "Make found: ${MAKE}")
    if(MAKE)
        add_custom_target(COMMAND ${MAKE}
            -C ${PROJECT_BINARY_DOCS_DIR}/${INTERNAL_BUILDER}
            ${SECOND_STEP_ARGS})
    else()
        message(FATAL_ERROR "Could not find Make")
    endif()
endif()

# Install the generated docs
install(DIRECTORY ${PROJECT_BINARY_DOCS_DIR}/${INTERNAL_BUILDER}
    DESTINATION docs/${PROJECT_NAME}
    COMPONENT backend-sphinx
    PATTERN ".buildinfo" EXCLUDE)
set(CPACK_COMPONENT_backend-sphinx_DISPLAY_NAME "Fast DDS Statistics Backend Sphinx")
set(CPACK_COMPONENT_backend-sphinx_DESCRIPTION
    "eProsima Fast DDS Statistics Backend documentation in ${INTERNAL_BUILDER} format")
set(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} ${INTERNAL_BUILDER})

# Add tests if required
if(BUILD_DOCUMENTATION_TESTS)
    message(STATUS "Adding documentation tests")
    add_subdirectory(test)
endif()
