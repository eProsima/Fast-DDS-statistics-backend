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

include(${PROJECT_SOURCE_DIR}/cmake/common/gtest.cmake)
check_gtest()

if(GTEST_FOUND)
    find_package(Threads REQUIRED)

    add_executable(qos_serializer_tests QosSerializerTests.cpp
        ${PROJECT_SOURCE_DIR}/src/cpp/subscriber/QosSerializer.cpp)

    if(MSVC)
        target_compile_definitions(qos_serializer_tests
            PRIVATE _CRT_DECLARE_NONSTDC_NAMES=0 FASTDDS_STATISTICS_BACKEND_SOURCE)
    endif(MSVC)

    target_include_directories(qos_serializer_tests PRIVATE
        ${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}
        ${PROJECT_BINARY_DIR}/include
        ${PROJECT_BINARY_DIR}/include/${PROJECT_NAME}
        ${PROJECT_SOURCE_DIR}/src/cpp
        ${TINYXML2_INCLUDE_DIR})

    target_link_libraries(qos_serializer_tests PUBLIC
        ${GTEST_LIBRARIES} fastdds fastcdr ${TINYXML2_LIBRARY})

    get_win32_path_dependencies(qos_serializer_tests TEST_FRIENDLY_PATH)

    set(QOSSERIALIZER_TEST_LIST
        durability_qos_policy
        duration
        deadline_qos_policy
        latency_budget_qos_policy
        liveliness_qos_policy
        reliability_qos_policy
        ownership_qos_policy
        destination_order_qos_policy
        topic_data_qos_policy
        user_data_qos_policy
        group_data_qos_policy
        time_based_filter_qos_policy
        boolean_qos_policy
        presentation_qos_policy
        partition_qos_policy
        durability_service_qos_policy
        lifespan_qos_policy
        data_representation_qos_policy
        type_consistency_qos_policy
        disable_positive_ack_qos_policy
        data_sharing_qos_policy
        ownership_strength_qos_policy
        publish_mode_qos_policy
        parameter_property_list_qos_policy
        history_qos_policy
        resource_limits_qos_policy
        reader_data_lifecycle_qos_policy
        reader_times
        rtps_reliable_reader_qos_policy
        locator_list
        external_locators
        memory_management_policy
        rtps_endpoint_qos_policy
        resource_limited_container_config
        reader_resource_limits_qos_policy
        transport_priority_qos_policy
        writer_data_lifecycle_qos_policy
        writer_times
        rtps_reliable_writer_qos_policy
        writer_resource_limits_qos_policy
        guid_prefix_t
        discovery_protocol
        initial_announcement_config
        simple_edp_attributes
        participant_filtering_flags
        discovery_settings
        builtin_attributes
        port_parameters
        wire_protocol_config_qos_policy
        writer_info_serializer
        reader_info_serializer
        participant_info_serializer
        reader_optional_qos_serializer
        writer_optional_qos_serializer
        participant_optional_qos_serializer
        )

    foreach(test_name ${QOSSERIALIZER_TEST_LIST})
        add_test(NAME qos_serializer_tests.${test_name}
                COMMAND qos_serializer_tests
                --gtest_filter=qos_serializer_tests.${test_name}:*/qos_serializer_tests.${test_name}/*)

    if(TEST_FRIENDLY_PATH)
        set_tests_properties(qos_serializer_tests.${test_name} PROPERTIES ENVIRONMENT "PATH=${TEST_FRIENDLY_PATH}")
    endif(TEST_FRIENDLY_PATH)
    endforeach()
endif()
