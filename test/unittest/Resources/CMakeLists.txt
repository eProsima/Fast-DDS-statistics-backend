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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/empty_entities_dump_unlinked_entities.json
    ${CMAKE_CURRENT_BINARY_DIR}/empty_entities_dump_unlinked_entities.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_dump.json
    ${CMAKE_CURRENT_BINARY_DIR}/simple_dump.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_dump_no_process_participant_link.json
    ${CMAKE_CURRENT_BINARY_DIR}/simple_dump_no_process_participant_link.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/simple_dump_no_process_participant_link_erased_domain.json
    ${CMAKE_CURRENT_BINARY_DIR}/simple_dump_no_process_participant_link_erased_domain.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/complex_dump.json
    ${CMAKE_CURRENT_BINARY_DIR}/complex_dump.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/complex_dump_erased_domain_1.json
    ${CMAKE_CURRENT_BINARY_DIR}/complex_dump_erased_domain_1.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/alternative_complex_dump.json
    ${CMAKE_CURRENT_BINARY_DIR}/alternative_complex_dump.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/alternative_complex_dump_erased_domain_2.json
    ${CMAKE_CURRENT_BINARY_DIR}/alternative_complex_dump_erased_domain_2.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/domain_view_graph_two_participants.json
    ${CMAKE_CURRENT_BINARY_DIR}/domain_view_graph_two_participants.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/domain_view_graph_undiscover_participant.json
    ${CMAKE_CURRENT_BINARY_DIR}/domain_view_graph_undiscover_participant.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/domain_view_graph_undiscover_endpoint.json
    ${CMAKE_CURRENT_BINARY_DIR}/domain_view_graph_undiscover_endpoint.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/domain_view_graph_empty.json
    ${CMAKE_CURRENT_BINARY_DIR}/domain_view_graph_empty.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/domain_view_graph_host_insert_failure.json
    ${CMAKE_CURRENT_BINARY_DIR}/domain_view_graph_host_insert_failure.json
    COPYONLY)
    
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/domain_view_graph_user_insert_failure.json
    ${CMAKE_CURRENT_BINARY_DIR}/domain_view_graph_user_insert_failure.json
    COPYONLY)
    
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/domain_view_graph_process_insert_failure.json
    ${CMAKE_CURRENT_BINARY_DIR}/domain_view_graph_process_insert_failure.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/domain_view_graph_updated_entites.json
    ${CMAKE_CURRENT_BINARY_DIR}/domain_view_graph_updated_entites.json
    COPYONLY)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/old_complex_dump.json
    ${CMAKE_CURRENT_BINARY_DIR}/old_complex_dump.json
    COPYONLY)