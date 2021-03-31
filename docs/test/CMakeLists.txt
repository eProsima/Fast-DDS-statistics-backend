# Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

add_executable(
    ${PROJECT_NAME}-documentation-test
    ${PROJECT_SOURCE_DOCS_DIR}/code/StatisticsBackendTests.cpp
    )
target_include_directories(
    ${PROJECT_NAME}-documentation-test
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/src/cpp
    )
target_link_libraries(${PROJECT_NAME}-documentation-test ${PROJECT_NAME})

####################################################################################################
# Add tests
####################################################################################################

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/grep.py
    ${CMAKE_CURRENT_BINARY_DIR}/grep.py COPYONLY)

configure_file(${PROJECT_SOURCE_DOCS_DIR}/code/graph_example.json
    ${CMAKE_CURRENT_BINARY_DIR}/graph_example.json COPYONLY)

find_package(Python COMPONENTS Interpreter Development)
if(NOT(${Python_FOUND} AND (${Python_VERSION} VERSION_GREATER "3.6")))
    message(FATAL_ERROR "Building the documentation tests requires Python 3.7")
endif()

# Check docs style
find_program(DOC8 doc8 DOC "path to doc8 executable")
message(STATUS "doc8 found: ${DOC8}")
if(DOC8)
    add_test(
        NAME documentation.doc8
        COMMAND ${DOC8}
        --max-line-length 120 ${PROJECT_SOURCE_DOCS_DIR})
else()
    message(FATAL_ERROR "Could not find doc8")
endif()

# Check that all cpp snippets are on the CPP files so they are compiled
add_test(NAME documentation.code_block_check.cpp
    COMMAND
    ${Python_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/grep.py
    --directory ${PROJECT_SOURCE_DOCS_DIR}
    --regex "code-block::.*c[+p][+p]")

# Check docs spelling
add_test(NAME documentation.spell_check
    COMMAND
    ${SPHINX_EXECUTABLE} -Q -W --keep-going
    -D breathe_projects.fastdds-statistics-backend=${DOXYGEN_OUTPUT_DIR}/xml
    -b spelling
    -d "${PROJECT_BINARY_DOCS_DIR}/doctrees"
    ${PROJECT_SOURCE_DOCS_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/spelling)

add_test(NAME documentation.get_graph_parse COMMAND ${PROJECT_NAME}-documentation-test get_graph_parse)
