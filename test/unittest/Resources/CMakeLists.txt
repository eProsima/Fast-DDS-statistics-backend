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

# Auxiliary files
##################################################################

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/database_dump.json
    ${CMAKE_CURRENT_BINARY_DIR}/database_dump.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/empty_dump.json
    ${CMAKE_CURRENT_BINARY_DIR}/empty_dump.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/empty_entities_dump.json
    ${CMAKE_CURRENT_BINARY_DIR}/empty_entities_dump.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_dump.json
    ${CMAKE_CURRENT_BINARY_DIR}/simple_dump.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_dump_no_process_participant_link.json
    ${CMAKE_CURRENT_BINARY_DIR}/simple_dump_no_process_participant_link.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/complex_dump.json
    ${CMAKE_CURRENT_BINARY_DIR}/complex_dump.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/old_complex_dump.json
    ${CMAKE_CURRENT_BINARY_DIR}/old_complex_dump.json
    COPYONLY)