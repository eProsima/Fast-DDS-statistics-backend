// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/dds/core/policy/RTPSReliableReaderQos.hpp>
#include <fastdds/utils/IPLocator.hpp>

#include <database/data.hpp>
#include <subscriber/QosSerializer.hpp>
#include <subscriber/QosSerializerTags.hpp>
#include <fastdds_statistics_backend/topic_types/types.hpp>
#include <fastdds_statistics_backend/topic_types/monitorservice_types.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::statistics_backend::subscriber;
using namespace eprosima::statistics_backend::database;


TEST(qos_serializer_tests, durability_qos_policy)
{
    DurabilityQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.kind = VOLATILE_DURABILITY_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = durability_volatile_tag;
    EXPECT_EQ(expected, serialized);

    qos.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = durability_transient_local_tag;
    EXPECT_EQ(expected, serialized);

    qos.kind = TRANSIENT_DURABILITY_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = durability_transient_tag;
    EXPECT_EQ(expected, serialized);

    qos.kind = PERSISTENT_DURABILITY_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = durability_persistent_tag;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, duration)
{
    eprosima::fastdds::dds::Duration_t qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos = eprosima::fastdds::dds::Duration_t(5, 10);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][duration_seconds_tag] = 5;
    expected[field.c_str()][duration_nanoseconds_tag] = 10;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, deadline_qos_policy)
{
    DeadlineQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.period = eprosima::fastdds::dds::Duration_t (5, 0);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    Qos serialized_period;

    eprosima::statistics_backend::subscriber::serialize(qos.period, duration_period_tag, serialized_period);
    expected[field.c_str()][duration_period_tag] = serialized_period[duration_period_tag];
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, latency_budget_qos_policy)
{
    LatencyBudgetQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.duration = eprosima::fastdds::dds::Duration_t (0, 5);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    Qos serialized_duration;

    eprosima::statistics_backend::subscriber::serialize(qos.duration, duration_tag, serialized_duration);
    expected[field.c_str()][duration_tag] = serialized_duration[duration_tag];
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, liveliness_qos_policy)
{
    LivelinessQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.kind =  AUTOMATIC_LIVELINESS_QOS;
    qos.announcement_period = eprosima::fastdds::dds::Duration_t (10, 10);
    qos.lease_duration = eprosima::fastdds::dds::Duration_t (10, 10);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    Qos serialized_duration;
    Qos serialized_period;

    eprosima::statistics_backend::subscriber::serialize(qos.lease_duration, lease_duration_tag, serialized_duration);
    eprosima::statistics_backend::subscriber::serialize(qos.announcement_period, announcement_period_tag,
            serialized_period);
    expected[field.c_str()][kind_tag] = liveliness_automatic_tag;
    expected[field.c_str()][lease_duration_tag] = serialized_duration[lease_duration_tag];
    expected[field.c_str()][announcement_period_tag] = serialized_period[announcement_period_tag];
    EXPECT_EQ(expected, serialized);

    qos.kind =  MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = liveliness_manual_participant_tag;
    EXPECT_EQ(expected, serialized);

    qos.kind =  MANUAL_BY_TOPIC_LIVELINESS_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = liveliness_manual_topic_tag;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, reliability_qos_policy)
{
    ReliabilityQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.kind =  BEST_EFFORT_RELIABILITY_QOS;
    qos.max_blocking_time = eprosima::fastdds::dds::Duration_t (10, 10);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    Qos serialized_blocking_time;

    eprosima::statistics_backend::subscriber::serialize(qos.max_blocking_time, max_blocking_time_tag,
            serialized_blocking_time);
    expected[field.c_str()][kind_tag] = reliability_best_effort_tag;
    expected[field.c_str()][max_blocking_time_tag] = serialized_blocking_time[max_blocking_time_tag];
    EXPECT_EQ(expected, serialized);

    qos.kind =  RELIABLE_RELIABILITY_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = reliability_reliable_tag;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, ownership_qos_policy)
{
    OwnershipQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.kind =  SHARED_OWNERSHIP_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = ownership_shared_tag;
    EXPECT_EQ(expected, serialized);

    qos.kind =  EXCLUSIVE_OWNERSHIP_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = ownership_exclusive_tag;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, destination_order_qos_policy)
{
    DestinationOrderQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.kind =  BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = destination_order_reception_tag;
    EXPECT_EQ(expected, serialized);

    qos.kind =  BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = destination_order_source_tag;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, topic_data_qos_policy)
{
    TopicDataQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    // Empty data
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    std::stringstream value;
    expected[field.c_str()] = value.str();
    EXPECT_EQ(expected, serialized);

    // Some data
    std::vector<unsigned char> data;
    for (unsigned char d = 0; d < std::numeric_limits<unsigned char>::max(); ++d)
    {
        data.push_back(d);
    }
    qos.setValue(data);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    for (unsigned char d = 0; d < std::numeric_limits<unsigned char>::max(); ++d)
    {
        value << std::hex << (int)d;
    }
    expected[field.c_str()] = value.str();
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, user_data_qos_policy)
{
    UserDataQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    // Empty data
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    std::stringstream value;
    expected[field.c_str()] = value.str();
    EXPECT_EQ(expected, serialized);
    std::vector<unsigned char> data;

    for (unsigned char d = 0; d < std::numeric_limits<unsigned char>::max(); ++d)
    {
        data.push_back(d);
    }
    qos.setValue(data);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    for (unsigned char d = 0; d < std::numeric_limits<unsigned char>::max(); ++d)
    {
        value << std::hex << (int)d;
    }
    expected[field.c_str()] = value.str();
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, group_data_qos_policy)
{
    GroupDataQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    // Empty data
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    std::stringstream value;
    expected[field.c_str()] = value.str();
    EXPECT_EQ(expected, serialized);

    std::vector<unsigned char> data;
    for (unsigned char d = 0; d < std::numeric_limits<unsigned char>::max(); ++d)
    {
        data.push_back(d);
    }
    qos.setValue(data);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    for (unsigned char d = 0; d < std::numeric_limits<unsigned char>::max(); ++d)
    {
        value << std::hex << (int)d;
    }
    expected[field.c_str()] = value.str();
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, time_based_filter_qos_policy)
{
    TimeBasedFilterQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.minimum_separation = eprosima::fastdds::dds::Duration_t (10, 10);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    Qos serialized_minimum_separation;

    eprosima::statistics_backend::subscriber::serialize(qos.minimum_separation, minimum_separation_tag,
            serialized_minimum_separation);
    expected[field.c_str()][minimum_separation_tag] = serialized_minimum_separation[minimum_separation_tag];
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, presentation_qos_policy)
{
    PresentationQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.access_scope =  INSTANCE_PRESENTATION_QOS;
    qos.coherent_access =  true;
    qos.ordered_access =  false;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][access_scope_tag] = access_scope_instance_tag;
    expected[field.c_str()][coherent_access_tag] = true_tag;
    expected[field.c_str()][ordered_access_tag] = false_tag;
    EXPECT_EQ(expected, serialized);

    qos.access_scope =  TOPIC_PRESENTATION_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][access_scope_tag] = access_scope_topic_tag;
    EXPECT_EQ(expected, serialized);

    qos.access_scope =  GROUP_PRESENTATION_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][access_scope_tag] = access_scope_group_tag;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, partition_qos_policy)
{
    PartitionQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    // No partition exists
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()] = Qos::array();
    EXPECT_EQ(expected, serialized);

    // Some partitions exist
    qos.push_back("PartitionA");
    qos.push_back("PartitionB");
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()].push_back("PartitionA");
    expected[field.c_str()].push_back("PartitionB");
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, durability_service_qos_policy)
{
    DurabilityServiceQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.history_kind =  KEEP_LAST_HISTORY_QOS;
    qos.service_cleanup_delay = eprosima::fastdds::dds::Duration_t (10, 10);
    qos.max_instances =  20;
    qos.max_samples =  30;
    qos.max_samples_per_instance =  40;
    qos.history_depth =  50;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    Qos serialized_service_cleanup_delay;

    eprosima::statistics_backend::subscriber::serialize(qos.service_cleanup_delay, service_cleanup_delay_tag,
            serialized_service_cleanup_delay);
    expected[field.c_str()][history_kind_tag] = history_keep_last_tag;
    expected[field.c_str()][service_cleanup_delay_tag] = serialized_service_cleanup_delay[service_cleanup_delay_tag];
    expected[field.c_str()][max_instances_tag] = 20;
    expected[field.c_str()][max_samples_tag] = 30;
    expected[field.c_str()][max_samples_per_instance_tag] = 40;
    expected[field.c_str()][history_depth_tag] = 50;
    EXPECT_EQ(expected, serialized);

    qos.history_kind =  KEEP_ALL_HISTORY_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][history_kind_tag] = history_keep_all_tag;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, lifespan_qos_policy)
{
    LifespanQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.duration = eprosima::fastdds::dds::Duration_t (10, 10);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    Qos serialized_duration;

    eprosima::statistics_backend::subscriber::serialize(qos.duration, duration_tag, serialized_duration);
    expected[field.c_str()][duration_tag] = serialized_duration[duration_tag];
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, data_representation_qos_policy)
{
    DataRepresentationQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    // No representation exists
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()] = Qos::array();
    EXPECT_EQ(expected, serialized);

    // Some representation exist
    qos.m_value.push_back(XCDR_DATA_REPRESENTATION);
    qos.m_value.push_back(XML_DATA_REPRESENTATION);
    qos.m_value.push_back(XCDR2_DATA_REPRESENTATION);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()].push_back(representation_xcdr_tag);
    expected[field.c_str()].push_back(representation_xml_tag);
    expected[field.c_str()].push_back(representation_xcdr2_tag);
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, type_consistency_qos_policy)
{
    TypeConsistencyEnforcementQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.m_kind =  DISALLOW_TYPE_COERCION;
    qos.m_ignore_sequence_bounds = true;
    qos.m_ignore_string_bounds =  false;
    qos.m_ignore_member_names =  false;
    qos.m_prevent_type_widening =  false;
    qos.m_force_type_validation =  true;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = type_consistency_coercion_disallow_tag;
    expected[field.c_str()][ignore_sequence_bounds_tag] = true_tag;
    expected[field.c_str()][ignore_string_bounds_tag] = false_tag;
    expected[field.c_str()][ignore_member_names_tag] = false_tag;
    expected[field.c_str()][prevent_type_widening_tag] = false_tag;
    expected[field.c_str()][force_type_validation_tag] = true_tag;
    EXPECT_EQ(expected, serialized);

    qos.m_kind =  ALLOW_TYPE_COERCION;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = type_consistency_coercion_allow_tag;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, disable_positive_ack_qos_policy)
{
    DisablePositiveACKsQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.enabled = true;
    qos.duration = eprosima::fastdds::dds::Duration_t (10, 10);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    Qos serialized_duration;

    eprosima::statistics_backend::subscriber::serialize(qos.duration, duration_tag, serialized_duration);
    expected[field.c_str()][duration_tag] = serialized_duration[duration_tag];
    expected[field.c_str()][enabled_tag] = true_tag;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, data_sharing_qos_policy)
{
    DataSharingQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.off();
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = data_sharing_off_tag;
    expected[field.c_str()][max_domains_tag] = 0;
    expected[field.c_str()][domain_ids_tag] = Qos::array();
    EXPECT_EQ(expected, serialized);

    qos.automatic("datasharing_directory");
    qos.add_domain_id((uint16_t) 25);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = data_sharing_auto_tag;
    expected[field.c_str()][max_domains_tag] = 0;
    expected[field.c_str()][domain_ids_tag].push_back(25);
    EXPECT_EQ(expected, serialized);

    qos.on("datasharing_directory");
    qos.set_max_domains(2);
    qos.add_domain_id((uint16_t) 25);
    qos.add_domain_id((uint64_t) 30);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = data_sharing_on_tag;
    expected[field.c_str()][max_domains_tag] = 2;
    expected[field.c_str()][domain_ids_tag] = Qos::array();
    expected[field.c_str()][domain_ids_tag].push_back(25);
    expected[field.c_str()][domain_ids_tag].push_back(30);
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, ownership_strength_qos_policy)
{
    OwnershipStrengthQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.value =  5;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][value_tag] = 5;
    EXPECT_EQ(expected, serialized);
}


TEST(qos_serializer_tests, publish_mode_qos_policy)
{
    PublishModeQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");
    std::string flow_controller_name("test_flow_controller");

    qos.kind = SYNCHRONOUS_PUBLISH_MODE;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = publish_mode_sync_tag;
    EXPECT_EQ(expected, serialized);

    qos.kind = ASYNCHRONOUS_PUBLISH_MODE;
    qos.flow_controller_name = flow_controller_name;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = publish_mode_async_tag;
    expected[field.c_str()][flow_controller_tag] = flow_controller_name;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, parameter_property_list_qos_policy)
{
    ParameterPropertyList_t qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    // No property exists
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()] = Qos::array();
    EXPECT_EQ(expected, serialized);

    // Some property exist
    qos.push_back(std::make_pair("property_name", "property_value"));
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    Qos property;
    property[name_tag] = "property_name";
    property[value_tag] = "property_value";
    expected[field.c_str()].push_back(property);
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, history_qos_policy)
{
    HistoryQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.kind = KEEP_LAST_HISTORY_QOS;
    qos.depth = 10;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = history_keep_last_tag;
    expected[field.c_str()][history_depth_tag] = 10;
    EXPECT_EQ(expected, serialized);

    expected.clear();
    serialized.clear();

    qos.kind = KEEP_ALL_HISTORY_QOS;
    qos.depth = 0; // depth is not used in KEEP_ALL_HISTORY_QOS
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = history_keep_all_tag;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, resource_limits_qos_policy)
{
    ResourceLimitsQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.max_samples = 6000;
    qos.max_instances = 20;
    qos.max_samples_per_instance = 500;
    qos.allocated_samples = 50;
    qos.extra_samples = 2;

    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][max_samples_tag] = 6000;
    expected[field.c_str()][max_instances_tag] = 20;
    expected[field.c_str()][max_samples_per_instance_tag] = 500;
    expected[field.c_str()][allocated_samples_tag] = 50;
    expected[field.c_str()][extra_samples_tag] = 2;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, reader_data_lifecycle_qos_policy)
{
    ReaderDataLifecycleQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");
    eprosima::fastdds::dds::Duration_t duration(10, 10);

    qos.autopurge_no_writer_samples_delay = duration;
    qos.autopurge_disposed_samples_delay = duration;

    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);
    eprosima::statistics_backend::subscriber::serialize(qos.autopurge_no_writer_samples_delay,
            autopurge_no_writer_samples_delay_tag, expected[field.c_str()]);
    eprosima::statistics_backend::subscriber::serialize(qos.autopurge_disposed_samples_delay,
            autopurge_disposed_samples_delay_tag, expected[field.c_str()]);
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, reader_times)
{
    eprosima::fastdds::rtps::ReaderTimes times;
    Qos serialized;
    Qos expected;
    std::string field("field");
    eprosima::fastdds::dds::Duration_t duration(10, 10);
    times.initial_acknack_delay = duration;
    times.heartbeat_response_delay = duration;

    eprosima::statistics_backend::subscriber::serialize(times, field, serialized);
    eprosima::statistics_backend::subscriber::serialize(times.initial_acknack_delay, initial_acknack_delay_tag,
            expected[field.c_str()]);
    eprosima::statistics_backend::subscriber::serialize(times.heartbeat_response_delay, heartbeat_response_delay_tag,
            expected[field.c_str()]);
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, rtps_reliable_reader_qos_policy)
{
    RTPSReliableReaderQos qos;
    Qos serialized;
    Qos expected;
    std::string field("field");
    eprosima::fastdds::dds::Duration_t duration(10, 10);
    qos.times.initial_acknack_delay = duration;
    qos.times.heartbeat_response_delay = duration;
    qos.disable_positive_acks.enabled = true;
    qos.disable_positive_acks.duration = duration;

    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);
    eprosima::statistics_backend::subscriber::serialize(qos.times, reader_times_tag, expected[field.c_str()]);
    eprosima::statistics_backend::subscriber::serialize(qos.disable_positive_acks, disable_positive_acks_tag,
            expected[field.c_str()]);
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, locator_list)
{
    eprosima::fastdds::rtps::LocatorList qos;
    Qos serialized;
    Qos expected;
    std::string field("field");
    uint32_t port = 1234;
    eprosima::fastdds::rtps::Locator_t locator1(port);
    eprosima::fastdds::rtps::Locator_t locator2(port);
    eprosima::fastdds::rtps::IPLocator::setIPv4(locator1, 127, 0, 0, 1);
    eprosima::fastdds::rtps::IPLocator::setIPv4(locator2, 127, 0, 0, 2);

    // Empty locator list
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()] = Qos::array();
    EXPECT_EQ(expected, serialized);
    serialized.clear();
    expected.clear();

    // Some locators exist
    qos.push_back(locator1);
    qos.push_back(locator2);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);
    expected[field.c_str()].push_back("UDPv4:[127.0.0.1]:1234");
    expected[field.c_str()].push_back("UDPv4:[127.0.0.2]:1234");
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, external_locators)
{
    eprosima::fastdds::rtps::ExternalLocators qos;
    Qos serialized;
    Qos expected;
    std::string field("field");
    uint32_t port = 1234;
    uint8_t mask = 16;

    // Empty external locators
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);
    EXPECT_EQ(Qos::array(), serialized[field.c_str()]);

    // Non-empty external locators
    expected[field.c_str()] = Qos::array();

    for (uint8_t externality = 2; externality > 0; --externality)
    {
        Qos locators_by_externality;
        locators_by_externality[external_locators_externality_index_tag] = externality;
        locators_by_externality[locators_tag] = Qos::array();

        for (uint8_t cost = 0; cost < 2; ++cost)
        {
            Qos locators_by_cost;

            eprosima::fastdds::rtps::LocatorWithMask locator_1;
            locator_1.port = port;
            eprosima::fastdds::rtps::IPLocator::setIPv4(
                locator_1, 127, 0, 0, static_cast<eprosima::fastdds::rtps::octet>(externality + cost + 1));
            locator_1.mask(mask);
            eprosima::fastdds::rtps::LocatorWithMask locator_2;
            locator_2.port = port;
            eprosima::fastdds::rtps::IPLocator::setIPv4(
                locator_2, 127, 0, 0, static_cast<eprosima::fastdds::rtps::octet>(externality + cost + 2));
            locator_2.mask(mask);

            qos[externality][cost].push_back(locator_1);
            qos[externality][cost].push_back(locator_2);
            locators_by_cost[external_locators_cost_tag] = cost;
            locators_by_cost[locators_with_mask_tag] = Qos::array();
            locators_by_cost[locators_with_mask_tag].push_back("UDPv4:[127.0.0." +
                    std::to_string(static_cast<int>(externality + cost + 1)) + "/" + std::to_string(mask) + "]:"
                    + std::to_string(port));
            locators_by_cost[locators_with_mask_tag].push_back("UDPv4:[127.0.0." +
                    std::to_string(static_cast<int>(externality + cost + 2)) + "/" + std::to_string(mask) + "]:"
                    + std::to_string(port));
            locators_by_externality[locators_tag].push_back(locators_by_cost);
        }

        expected[field.c_str()].push_back(locators_by_externality);
    }

    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, memory_management_policy)
{
    eprosima::fastdds::rtps::MemoryManagementPolicy_t qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos = eprosima::fastdds::rtps::MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()] = history_memory_policy_preallocated_tag;
    EXPECT_EQ(expected, serialized);

    qos = eprosima::fastdds::rtps::MemoryManagementPolicy_t::PREALLOCATED_WITH_REALLOC_MEMORY_MODE;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()] = history_memory_policy_preallocated_realloc_tag;
    EXPECT_EQ(expected, serialized);

    qos = eprosima::fastdds::rtps::MemoryManagementPolicy_t::DYNAMIC_RESERVE_MEMORY_MODE;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()] = history_memory_policy_dynamic_reserve_tag;
    EXPECT_EQ(expected, serialized);

    qos = eprosima::fastdds::rtps::MemoryManagementPolicy_t::DYNAMIC_REUSABLE_MEMORY_MODE;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()] = history_memory_policy_dynamic_reusable_tag;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, rtps_endpoint_qos_policy)
{
    RTPSEndpointQos qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    eprosima::fastdds::rtps::Locator_t unicast_locator(1234);
    eprosima::fastdds::rtps::IPLocator::setIPv4(unicast_locator, 127, 0, 0, 1);
    eprosima::fastdds::rtps::Locator_t multicast_locator(1234);
    eprosima::fastdds::rtps::IPLocator::setIPv4(multicast_locator, 224, 0, 0, 1);
    eprosima::fastdds::rtps::LocatorWithMask locator_with_mask;
    locator_with_mask.port = 1234;
    eprosima::fastdds::rtps::IPLocator::setIPv4(locator_with_mask, 127, 0, 0, 1);
    locator_with_mask.mask(16);
    eprosima::fastdds::rtps::LocatorList unicast_locators;
    eprosima::fastdds::rtps::LocatorList multicast_locators;
    eprosima::fastdds::rtps::LocatorList remote_locators;
    eprosima::fastdds::rtps::ExternalLocators external_locators;
    unicast_locators.push_back(unicast_locator);
    qos.unicast_locator_list = unicast_locators;
    multicast_locators.push_back(multicast_locator);
    qos.multicast_locator_list = multicast_locators;
    remote_locators.push_back(unicast_locator);
    remote_locators.push_back(multicast_locator);
    qos.remote_locator_list = remote_locators;
    external_locators[2][0].push_back(locator_with_mask);
    qos.external_unicast_locators = external_locators;
    qos.ignore_non_matching_locators = true;
    qos.user_defined_id = 1;
    qos.entity_id = 2;
    qos.history_memory_policy = eprosima::fastdds::rtps::MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE;

    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);
    expected[field.c_str()][unicast_locator_list_tag] = {"UDPv4:[127.0.0.1]:1234"};
    expected[field.c_str()][multicast_locator_list_tag] = {"UDPv4:[224.0.0.1]:1234"};
    expected[field.c_str()][remote_locators_tag] = {"UDPv4:[127.0.0.1]:1234", "UDPv4:[224.0.0.1]:1234"};
    expected[field.c_str()][external_unicast_locators_tag] = {
        {
            {external_locators_externality_index_tag, 2},
            {locators_tag,
                {{
                    {external_locators_cost_tag, 0},
                    {locators_with_mask_tag,
                        {"UDPv4:[127.0.0.1/16]:1234"}
                    }
                }}
            }
        }
    };
    expected[field.c_str()][ignore_non_matching_locators_tag] = true_tag;
    expected[field.c_str()][user_defined_id_tag] = 1;
    expected[field.c_str()][entity_id_tag] = 2;
    expected[field.c_str()][history_memory_policy_tag] = history_memory_policy_preallocated_tag;

    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, resource_limited_container_config)
{
    eprosima::fastdds::ResourceLimitedContainerConfig config;
    Qos serialized;
    Qos expected;
    std::string field("field");

    config.initial = 10;
    config.maximum = 100;
    config.increment = 2;

    eprosima::statistics_backend::subscriber::serialize(config, field, serialized);
    expected[field.c_str()][resource_limited_container_initial_number_tag] = 10;
    expected[field.c_str()][resource_limited_container_maximum_number_tag] = 100;
    expected[field.c_str()][resource_limited_container_increment_tag] = 2;

    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, reader_resource_limits_qos_policy)
{
    ReaderResourceLimitsQos qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.matched_publisher_allocation = eprosima::fastdds::ResourceLimitedContainerConfig(10, 100, 2);
    qos.sample_infos_allocation = eprosima::fastdds::ResourceLimitedContainerConfig(20, 200, 4);
    qos.outstanding_reads_allocation = eprosima::fastdds::ResourceLimitedContainerConfig(40, 400, 8);
    qos.max_samples_per_read = 10;

    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);
    expected[field.c_str()][matched_publisher_allocation_tag] = {
        {resource_limited_container_initial_number_tag, 10},
        {resource_limited_container_maximum_number_tag, 100},
        {resource_limited_container_increment_tag, 2}
    };
    expected[field.c_str()][sample_infos_allocation_tag] = {
        {resource_limited_container_initial_number_tag, 20},
        {resource_limited_container_maximum_number_tag, 200},
        {resource_limited_container_increment_tag, 4}
    };
    expected[field.c_str()][outstanding_reads_allocation_tag] = {
        {resource_limited_container_initial_number_tag, 40},
        {resource_limited_container_maximum_number_tag, 400},
        {resource_limited_container_increment_tag, 8}
    };
    expected[field.c_str()][max_samples_per_read_tag] = 10;

    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, transport_priority_qos_policy)
{
    TransportPriorityQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.value = 5;

    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);
    expected[field.c_str()][value_tag] = 5;

    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, writer_data_lifecycle_qos_policy)
{
    WriterDataLifecycleQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.autodispose_unregistered_instances = false;

    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);
    expected[field.c_str()][autodispose_unregistered_instances_tag] = false_tag;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, writer_times)
{
    eprosima::fastdds::rtps::WriterTimes times;
    Qos serialized;
    Qos expected;
    std::string field("field");

    times.initial_heartbeat_delay = Duration_t(5, 5);
    times.heartbeat_period = Duration_t(10, 10);
    times.nack_response_delay = Duration_t(20, 20);
    times.nack_supression_duration = Duration_t(1, 1);

    eprosima::statistics_backend::subscriber::serialize(times, field, serialized);
    expected[field.c_str()][initial_heartbeat_delay_tag] = {
        {duration_seconds_tag, 5},
        {duration_nanoseconds_tag, 5}
    };
    expected[field.c_str()][heartbeat_period_tag] = {
        {duration_seconds_tag, 10},
        {duration_nanoseconds_tag, 10}
    };
    expected[field.c_str()][nack_response_delay_tag] = {
        {duration_seconds_tag, 20},
        {duration_nanoseconds_tag, 20}
    };
    expected[field.c_str()][nack_supression_duration_tag] = {
        {duration_seconds_tag, 1},
        {duration_nanoseconds_tag, 1}
    };
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, rtps_reliable_writer_qos_policy)
{
    RTPSReliableWriterQos qos;
    Qos serialized;
    Qos expected;
    std::string field("field");
    Qos duration_object = {
        {duration_seconds_tag, 10},
        {duration_nanoseconds_tag, 10}
    };

    qos.times.initial_heartbeat_delay = Duration_t(10, 10);
    qos.times.heartbeat_period = Duration_t(10, 10);
    qos.times.nack_response_delay = Duration_t(10, 10);
    qos.times.nack_supression_duration = Duration_t(10, 10);
    qos.disable_positive_acks.enabled = true;
    qos.disable_positive_acks.duration = Duration_t(10, 10);
    qos.disable_heartbeat_piggyback = true;

    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);
    expected[field.c_str()] = {
        {writer_times_tag,
            {
                {initial_heartbeat_delay_tag, duration_object},
                {heartbeat_period_tag, duration_object},
                {nack_response_delay_tag, duration_object},
                {nack_supression_duration_tag, duration_object}
            }
        },
        {disable_positive_acks_tag,
            {
                {enabled_tag, true_tag},
                {duration_tag, duration_object}
            }
        },
        {disable_heartbeat_piggyback_tag, true_tag}
    };
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, writer_resource_limits_qos_policy)
{
    WriterResourceLimitsQos qos;
    Qos serialized;
    Qos expected;
    std::string field("field");
    Qos resource_limited_contained_object = {
        {resource_limited_container_initial_number_tag, 10},
        {resource_limited_container_maximum_number_tag, 100},
        {resource_limited_container_increment_tag, 2}
    };

    qos.matched_subscriber_allocation.initial = 10;
    qos.matched_subscriber_allocation.maximum = 100;
    qos.matched_subscriber_allocation.increment = 2;
    qos.reader_filters_allocation.initial = 10;
    qos.reader_filters_allocation.maximum = 100;
    qos.reader_filters_allocation.increment = 2;

    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);
    expected[field.c_str()] = {
        {matched_subscriber_allocation_tag, resource_limited_contained_object},
        {reader_filters_allocation_tag, resource_limited_contained_object}
    };
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, guid_prefix_t)
{
    eprosima::fastdds::rtps::GuidPrefix_t guid_prefix;
    Qos serialized;
    Qos expected;
    std::string field("field");

    for (unsigned char i = 0; i < 12; ++i)
    {
        guid_prefix.value[i] = i;
    }

    eprosima::statistics_backend::subscriber::serialize(guid_prefix, field, serialized);
    expected[field.c_str()] = "00.01.02.03.04.05.06.07.08.09.0a.0b";
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, discovery_protocol)
{
    eprosima::fastdds::rtps::DiscoveryProtocol discovery_protocol;
    Qos serialized;
    Qos expected;
    std::string field("field");

    discovery_protocol = eprosima::fastdds::rtps::DiscoveryProtocol::NONE;
    eprosima::statistics_backend::subscriber::serialize(discovery_protocol, field, serialized);

    expected[field.c_str()] = discovery_protocol_none_tag;
    EXPECT_EQ(expected, serialized);

    discovery_protocol = eprosima::fastdds::rtps::DiscoveryProtocol::SIMPLE;
    eprosima::statistics_backend::subscriber::serialize(discovery_protocol, field, serialized);

    expected[field.c_str()] = discovery_protocol_simple_tag;
    EXPECT_EQ(expected, serialized);

    discovery_protocol = eprosima::fastdds::rtps::DiscoveryProtocol::EXTERNAL;
    eprosima::statistics_backend::subscriber::serialize(discovery_protocol, field, serialized);

    expected[field.c_str()] = discovery_protocol_external_tag;
    EXPECT_EQ(expected, serialized);

    discovery_protocol = eprosima::fastdds::rtps::DiscoveryProtocol::CLIENT;
    eprosima::statistics_backend::subscriber::serialize(discovery_protocol, field, serialized);

    expected[field.c_str()] = discovery_protocol_client_tag;
    EXPECT_EQ(expected, serialized);

    discovery_protocol = eprosima::fastdds::rtps::DiscoveryProtocol::SERVER;
    eprosima::statistics_backend::subscriber::serialize(discovery_protocol, field, serialized);

    expected[field.c_str()] = discovery_protocol_server_tag;
    EXPECT_EQ(expected, serialized);

    discovery_protocol = eprosima::fastdds::rtps::DiscoveryProtocol::BACKUP;
    eprosima::statistics_backend::subscriber::serialize(discovery_protocol, field, serialized);

    expected[field.c_str()] = discovery_protocol_backup_tag;
    EXPECT_EQ(expected, serialized);

    discovery_protocol = eprosima::fastdds::rtps::DiscoveryProtocol::SUPER_CLIENT;
    eprosima::statistics_backend::subscriber::serialize(discovery_protocol, field, serialized);

    expected[field.c_str()] = discovery_protocol_super_client_tag;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, initial_announcement_config)
{
    eprosima::fastdds::rtps::InitialAnnouncementConfig config;
    Qos serialized;
    Qos expected;
    std::string field("field");

    config.count = 10;
    config.period = eprosima::fastdds::dds::Duration_t(5, 5);

    eprosima::statistics_backend::subscriber::serialize(config, field, serialized);
    expected[field.c_str()][initial_announcements_count_tag] = 10;
    expected[field.c_str()][initial_announcements_period_tag] = {
        {duration_seconds_tag, 5},
        {duration_nanoseconds_tag, 5}
    };

    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, simple_edp_attributes)
{
    eprosima::fastdds::rtps::SimpleEDPAttributes attributes;
    Qos serialized;
    Qos expected;
    std::string field("field");

    attributes.use_PublicationWriterANDSubscriptionReader = false;
    attributes.use_PublicationReaderANDSubscriptionWriter = false;
#if HAVE_SECURITY
    attributes.enable_builtin_secure_publications_writer_and_subscriptions_reader = false;
    attributes.enable_builtin_secure_subscriptions_writer_and_publications_reader = false;
#endif // if HAVE_SECURITY

    eprosima::statistics_backend::subscriber::serialize(attributes, field, serialized);

    expected[field.c_str()][simple_edp_use_pub_writer_sub_reader_tag] = false_tag;
    expected[field.c_str()][simple_edp_use_pub_reader_sub_writer_tag] = false_tag;
#if HAVE_SECURITY
    expected[field.c_str()][simple_edp_enable_builtin_secure_pub_writer_sub_reader_tag] = false_tag;
    expected[field.c_str()][simple_edp_enable_builtin_secure_sub_writer_pub_reader_tag] = false_tag;
#endif // if HAVE_SECURITY

    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, participant_filtering_flags)
{
    eprosima::fastdds::rtps::ParticipantFilteringFlags flag;
    Qos serialized;
    Qos expected;
    std::string field("field");

    flag = eprosima::fastdds::rtps::ParticipantFilteringFlags::NO_FILTER;
    eprosima::statistics_backend::subscriber::serialize(flag, field, serialized);

    expected[field.c_str()] = participant_filtering_no_filter_tag;
    EXPECT_EQ(expected, serialized);

    flag = eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_DIFFERENT_HOST;
    eprosima::statistics_backend::subscriber::serialize(flag, field, serialized);

    expected[field.c_str()] = participant_filtering_filter_different_host_tag;
    EXPECT_EQ(expected, serialized);

    flag = eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_DIFFERENT_PROCESS;
    eprosima::statistics_backend::subscriber::serialize(flag, field, serialized);

    expected[field.c_str()] = participant_filtering_filter_different_process_tag;
    EXPECT_EQ(expected, serialized);

    flag = eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_SAME_PROCESS;
    eprosima::statistics_backend::subscriber::serialize(flag, field, serialized);

    expected[field.c_str()] = participant_filtering_filter_same_process_tag;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, discovery_settings)
{
    eprosima::fastdds::rtps::DiscoverySettings settings;
    Qos serialized;
    Qos expected;
    std::string field("field");
    Qos duration_object = {
        {duration_seconds_tag, 10},
        {duration_nanoseconds_tag, 10}
    };
    Qos initial_announcements_object = {
        {initial_announcements_count_tag, 10},
        {initial_announcements_period_tag, duration_object}
    };
    Qos simple_edp_object = {
        {simple_edp_use_pub_writer_sub_reader_tag, false_tag},
        {simple_edp_use_pub_reader_sub_writer_tag, false_tag},
#if HAVE_SECURITY
        {simple_edp_enable_builtin_secure_pub_writer_sub_reader_tag, false_tag},
        {simple_edp_enable_builtin_secure_sub_writer_pub_reader_tag, false_tag}
#endif // if HAVE_SECURITY
    };
    Qos ds_object = {
        "UDPv4:[127.0.0.1]:1234"
    };
    eprosima::fastdds::rtps::Locator_t ds_locator(1234);
    eprosima::fastdds::rtps::LocatorList ds_locators;
    eprosima::fastdds::rtps::IPLocator::setIPv4(ds_locator, 127, 0, 0, 1);
    ds_locators.push_back(ds_locator);

    settings.discoveryProtocol = eprosima::fastdds::rtps::DiscoveryProtocol::CLIENT;
    settings.use_SIMPLE_EndpointDiscoveryProtocol = false;
    settings.use_STATIC_EndpointDiscoveryProtocol = true;
    settings.leaseDuration = Duration_t(10, 10);
    settings.leaseDuration_announcementperiod = Duration_t(10, 10);
    settings.initial_announcements.count = 10;
    settings.initial_announcements.period = Duration_t(10, 10);
    settings.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = false;
    settings.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = false;
#if HAVE_SECURITY
    settings.m_simpleEDP.enable_builtin_secure_publications_writer_and_subscriptions_reader = false;
    settings.m_simpleEDP.enable_builtin_secure_subscriptions_writer_and_publications_reader = false;
#endif // if HAVE_SECURITY
    settings.discoveryServer_client_syncperiod = Duration_t(10, 10);
    settings.m_DiscoveryServers = ds_locators;
    settings.ignoreParticipantFlags = eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_DIFFERENT_HOST;
    settings.static_edp_xml_config("xml_config");

    eprosima::statistics_backend::subscriber::serialize(settings, field, serialized);
    expected[field.c_str()] = {
        {discovery_protocol_tag, discovery_protocol_client_tag},
        {use_simple_edp_tag, false_tag},
        {use_static_edp_tag, true_tag},
        {lease_duration_tag, duration_object},
        {announcement_period_tag, duration_object},
        {initial_announcements_tag, initial_announcements_object},
        {simple_edp_attributes_tag, simple_edp_object},
        {discovery_server_client_syncperiod_tag, duration_object},
        {discovery_servers_tag, ds_object},
        {participant_filtering_tag, participant_filtering_filter_different_host_tag},
        {static_edp_xml_config_tag, "xml_config"}
    };

    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, builtin_attributes)
{
    eprosima::fastdds::rtps::BuiltinAttributes attr;
    Qos serialized;
    Qos expected;
    std::string field("field");
    Qos duration_object = {
        {duration_seconds_tag, 10},
        {duration_nanoseconds_tag, 10}
    };
    Qos unicast_locators_object = {
        "UDPv4:[127.0.0.1]:1234"
    };
    Qos multicast_locators_object = {
        "UDPv4:[224.0.0.1]:1234"
    };
    Qos external_locators_object = {
        {
            {external_locators_externality_index_tag, 2},
            {locators_tag,
                {{
                    {external_locators_cost_tag, 0},
                    {locators_with_mask_tag,
                        {"UDPv4:[127.0.0.1/16]:1234"}
                    }
                }}
            }
        }
    };

    eprosima::fastdds::rtps::LocatorList unicast_locators;
    eprosima::fastdds::rtps::LocatorList multicast_locators;
    eprosima::fastdds::rtps::Locator_t unicast_locator(1234);
    eprosima::fastdds::rtps::IPLocator::setIPv4(unicast_locator, 127, 0, 0, 1);
    unicast_locators.push_back(unicast_locator);
    eprosima::fastdds::rtps::Locator_t multicast_locator(1234);
    eprosima::fastdds::rtps::IPLocator::setIPv4(multicast_locator, 224, 0, 0, 1);
    multicast_locators.push_back(multicast_locator);
    eprosima::fastdds::rtps::LocatorWithMask locator_with_mask;
    locator_with_mask.port = 1234;
    eprosima::fastdds::rtps::IPLocator::setIPv4(locator_with_mask, 127, 0, 0, 1);
    locator_with_mask.mask(16);
    eprosima::fastdds::rtps::ExternalLocators external_locators;
    external_locators[2][0].push_back(locator_with_mask);

    /* DiscoverySettings */
    Qos initial_announcements_object = {
        {initial_announcements_count_tag, 10},
        {initial_announcements_period_tag, duration_object}
    };
    Qos simple_edp_object = {
        {simple_edp_use_pub_writer_sub_reader_tag, false_tag},
        {simple_edp_use_pub_reader_sub_writer_tag, false_tag},
#if HAVE_SECURITY
        {simple_edp_enable_builtin_secure_pub_writer_sub_reader_tag, false_tag},
        {simple_edp_enable_builtin_secure_sub_writer_pub_reader_tag, false_tag}
#endif // if HAVE_SECURITY
    };

    attr.discovery_config.discoveryProtocol = eprosima::fastdds::rtps::DiscoveryProtocol::CLIENT;
    attr.discovery_config.use_SIMPLE_EndpointDiscoveryProtocol = false;
    attr.discovery_config.use_STATIC_EndpointDiscoveryProtocol = true;
    attr.discovery_config.leaseDuration = Duration_t(10, 10);
    attr.discovery_config.leaseDuration_announcementperiod = Duration_t(10, 10);
    attr.discovery_config.initial_announcements.count = 10;
    attr.discovery_config.initial_announcements.period = Duration_t(10, 10);
    attr.discovery_config.m_simpleEDP.use_PublicationWriterANDSubscriptionReader = false;
    attr.discovery_config.m_simpleEDP.use_PublicationReaderANDSubscriptionWriter = false;
#if HAVE_SECURITY
    attr.discovery_config.m_simpleEDP.enable_builtin_secure_publications_writer_and_subscriptions_reader = false;
    attr.discovery_config.m_simpleEDP.enable_builtin_secure_subscriptions_writer_and_publications_reader = false;
#endif // if HAVE_SECURITY
    attr.discovery_config.discoveryServer_client_syncperiod = Duration_t(10, 10);
    attr.discovery_config.m_DiscoveryServers = unicast_locators;
    attr.discovery_config.ignoreParticipantFlags = eprosima::fastdds::rtps::ParticipantFilteringFlags::FILTER_DIFFERENT_HOST;
    attr.discovery_config.static_edp_xml_config("xml_config");

    expected[field.c_str()][discovery_config_tag] = {
        {discovery_protocol_tag, discovery_protocol_client_tag},
        {use_simple_edp_tag, false_tag},
        {use_static_edp_tag, true_tag},
        {lease_duration_tag, duration_object},
        {announcement_period_tag, duration_object},
        {initial_announcements_tag, initial_announcements_object},
        {simple_edp_attributes_tag, simple_edp_object},
        {discovery_server_client_syncperiod_tag, duration_object},
        {discovery_servers_tag, unicast_locators_object},
        {participant_filtering_tag, participant_filtering_filter_different_host_tag},
        {static_edp_xml_config_tag, "xml_config"}
    };

    /* Writer liveliness protocol */
    attr.use_WriterLivelinessProtocol = false;
    expected[field.c_str()][use_writer_liveliness_protocol_tag] = false_tag;

    /* Network configuration */
    attr.network_configuration = 1;
    expected[field.c_str()][network_configuration_tag] = 1;

    /* Metatraffic unicast locators */
    attr.metatrafficUnicastLocatorList = unicast_locators;
    expected[field.c_str()][metatraffic_unicast_locator_list_tag] = unicast_locators_object;

    /* Metatraffic multicast locators */
    attr.metatrafficMulticastLocatorList = multicast_locators;
    expected[field.c_str()][metatraffic_multicast_locator_list_tag] = multicast_locators_object;

    /* Metatraffic external unicast locators */
    attr.metatraffic_external_unicast_locators = external_locators;
    expected[field.c_str()][metatraffic_external_unicast_locators_tag] = external_locators_object;

    /* Initial peers */
    attr.initialPeersList = unicast_locators;
    expected[field.c_str()][initial_peers_tag] = unicast_locators_object;

    /* Reader history memory policy */
    attr.readerHistoryMemoryPolicy = eprosima::fastdds::rtps::MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE;
    expected[field.c_str()][reader_history_memory_policy_tag] = history_memory_policy_preallocated_tag;

    /* Reader payload size */
    attr.readerPayloadSize = 10;
    expected[field.c_str()][reader_payload_size_tag] = 10;

    /* Writer history memory policy */
    attr.writerHistoryMemoryPolicy = eprosima::fastdds::rtps::MemoryManagementPolicy_t::PREALLOCATED_MEMORY_MODE;
    expected[field.c_str()][writer_history_memory_policy_tag] = history_memory_policy_preallocated_tag;

    /* Writer payload size */
    attr.writerPayloadSize = 20;
    expected[field.c_str()][writer_payload_size_tag] = 20;

    /* Mutation tries */
    attr.mutation_tries = 3;
    expected[field.c_str()][mutation_tries_tag] = 3;

    /* Avoid builtin multicast */
    attr.avoid_builtin_multicast = false;
    expected[field.c_str()][avoid_builtin_multicast_tag] = false_tag;

    /* Flow controller name */
    attr.flow_controller_name = "flow_controller";
    expected[field.c_str()][flow_controller_tag] = "flow_controller";

    eprosima::statistics_backend::subscriber::serialize(attr, field, serialized);
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, port_parameters)
{
    eprosima::fastdds::rtps::PortParameters port_parameters;
    Qos serialized;
    Qos expected;
    std::string field("field");

    /* portBase */
    port_parameters.portBase = 1234;
    expected[field.c_str()][port_parameters_port_base_tag] = 1234;

    /* domainIDGain */
    port_parameters.domainIDGain = 10;
    expected[field.c_str()][port_parameters_domain_id_gain_tag] = 10;

    /* participantIDGain */
    port_parameters.participantIDGain = 10;
    expected[field.c_str()][port_parameters_participant_id_gain_tag] = 10;

    /* Offsets */
    port_parameters.offsetd0 = 1;
    expected[field.c_str()][port_parameters_offsetd0_tag] = 1;
    port_parameters.offsetd1 = 2;
    expected[field.c_str()][port_parameters_offsetd1_tag] = 2;
    port_parameters.offsetd2 = 3;
    expected[field.c_str()][port_parameters_offsetd2_tag] = 3;
    port_parameters.offsetd3 = 4;
    expected[field.c_str()][port_parameters_offsetd3_tag] = 4;
    port_parameters.offsetd4 = 5;
    expected[field.c_str()][port_parameters_offsetd4_tag] = 5;

    eprosima::statistics_backend::subscriber::serialize(port_parameters, field, serialized);
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, wire_protocol_config_qos_policy)
{
    WireProtocolConfigQos qos;
    Qos serialized;
    Qos expected;
    std::string field("field");
    Qos unicast_locators_object = {
        "UDPv4:[127.0.0.1]:1234"
    };
    Qos multicast_locators_object = {
        "UDPv4:[224.0.0.1]:1234"
    };
    Qos external_locators_object = {
        {
            {external_locators_externality_index_tag, 2},
            {locators_tag,
                {{
                    {external_locators_cost_tag, 0},
                    {locators_with_mask_tag,
                        {"UDPv4:[127.0.0.1/16]:1234"}
                    }
                }}
            }
        }
    };
    Qos empty_array = Qos::array();

    eprosima::fastdds::rtps::LocatorList unicast_locators;
    eprosima::fastdds::rtps::LocatorList multicast_locators;
    eprosima::fastdds::rtps::Locator_t unicast_locator(1234);
    eprosima::fastdds::rtps::IPLocator::setIPv4(unicast_locator, 127, 0, 0, 1);
    unicast_locators.push_back(unicast_locator);
    eprosima::fastdds::rtps::Locator_t multicast_locator(1234);
    eprosima::fastdds::rtps::IPLocator::setIPv4(multicast_locator, 224, 0, 0, 1);
    multicast_locators.push_back(multicast_locator);
    eprosima::fastdds::rtps::LocatorWithMask locator_with_mask;
    locator_with_mask.port = 1234;
    eprosima::fastdds::rtps::IPLocator::setIPv4(locator_with_mask, 127, 0, 0, 1);
    locator_with_mask.mask(16);
    eprosima::fastdds::rtps::ExternalLocators external_locators;
    external_locators[2][0].push_back(locator_with_mask);

    /* GuidPrefix_t */
    for (unsigned char i = 0; i < 12; ++i)
    {
        qos.prefix.value[i] = i;
    }

    expected[field.c_str()][guid_prefix_tag] = "00.01.02.03.04.05.06.07.08.09.0a.0b";

    /* participant_id */
    qos.participant_id = 1;
    expected[field.c_str()][participant_id_tag] = 1;

    /* BuiltinAttributes (default configuration) */
    expected[field.c_str()][builtin_attributes_tag] = {
        {discovery_config_tag,
            {
                {discovery_protocol_tag, discovery_protocol_simple_tag},
                {use_simple_edp_tag, true_tag},
                {use_static_edp_tag, false_tag},
                {lease_duration_tag,
                    {
                        {duration_seconds_tag, 20},
                        {duration_nanoseconds_tag, 0}
                    }
                },
                {announcement_period_tag,
                    {
                        {duration_seconds_tag, 3},
                        {duration_nanoseconds_tag, 0}
                    }
                },
                {initial_announcements_tag,
                    {
                        {initial_announcements_count_tag, 5},
                        {initial_announcements_period_tag,
                            {
                                {duration_seconds_tag, 0},
                                {duration_nanoseconds_tag, 100000000}
                            }
                        }
                    }
                },
                {simple_edp_attributes_tag,
                    {
                        {simple_edp_use_pub_writer_sub_reader_tag, true_tag},
                        {simple_edp_use_pub_reader_sub_writer_tag, true_tag},
                    #if HAVE_SECURITY
                        {simple_edp_enable_builtin_secure_pub_writer_sub_reader_tag, true_tag},
                        {simple_edp_enable_builtin_secure_sub_writer_pub_reader_tag, true_tag}
                    #endif // if HAVE_SECURITY
                    }
                },
                {discovery_server_client_syncperiod_tag,
                    {
                        {duration_seconds_tag, 0},
                        {duration_nanoseconds_tag, 450 * 1000000}
                    }
                },
                {discovery_servers_tag, empty_array},
                {participant_filtering_tag, participant_filtering_no_filter_tag},
                {static_edp_xml_config_tag, ""}
            }
        },
        {use_writer_liveliness_protocol_tag, true_tag},
        {network_configuration_tag, 0},
        {metatraffic_unicast_locator_list_tag, empty_array},
        {metatraffic_multicast_locator_list_tag, empty_array},
        {metatraffic_external_unicast_locators_tag, empty_array},
        {initial_peers_tag, empty_array},
        {reader_history_memory_policy_tag, history_memory_policy_preallocated_realloc_tag},
        {reader_payload_size_tag, BUILTIN_DATA_MAX_SIZE},
        {writer_history_memory_policy_tag, history_memory_policy_preallocated_realloc_tag},
        {writer_payload_size_tag, BUILTIN_DATA_MAX_SIZE},
        {mutation_tries_tag, 100},
        {avoid_builtin_multicast_tag, true_tag},
        {flow_controller_tag, ""}
    };
    
    /* default_unicast_locator_list */
    qos.default_unicast_locator_list = unicast_locators;
    expected[field.c_str()][default_unicast_locator_list_tag] = unicast_locators_object;
    
    /* default_multicast_locator_list */
    qos.default_multicast_locator_list = multicast_locators;
    expected[field.c_str()][default_multicast_locator_list_tag] = multicast_locators_object;
    
    /* default_external_unicast_locators */
    qos.default_external_unicast_locators = external_locators;
    expected[field.c_str()][default_external_unicast_locators_tag] = external_locators_object;
    qos.ignore_non_matching_locators = true;
    expected[field.c_str()][ignore_non_matching_locators_tag] = true_tag;

    /* Port parameters (default configuration) */
    expected[field.c_str()][port_parameters_tag] = {
        {port_parameters_port_base_tag, 7400},
        {port_parameters_domain_id_gain_tag, 250},
        {port_parameters_participant_id_gain_tag, 2},
        {port_parameters_offsetd0_tag, 0},
        {port_parameters_offsetd1_tag, 10},
        {port_parameters_offsetd2_tag, 1},
        {port_parameters_offsetd3_tag, 11},
        {port_parameters_offsetd4_tag, 2}
    };
    qos.default_multicast_locator_list = multicast_locators;
    expected[field.c_str()][default_multicast_locator_list_tag] = multicast_locators_object;

    /* default_external_unicast_locators */
    qos.default_external_unicast_locators = external_locators;
    expected[field.c_str()][default_external_unicast_locators_tag] = external_locators_object;

    /* ignore_non_matching_locators */
    qos.ignore_non_matching_locators = true;
    expected[field.c_str()][ignore_non_matching_locators_tag] = true_tag;

    /* easy_mode */
    qos.easy_mode("127.0.0.1");
    expected[field.c_str()][easy_mode_ip_tag] = "127.0.0.1";

    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, writer_info_serializer)
{
    eprosima::fastdds::rtps::PublicationBuiltinTopicData info;
    Qos serialized;
    Qos expected;

    // We will use default values here, specific values for each QoS
    // have been tested in other test
    serialized = eprosima::statistics_backend::subscriber::writer_proxy_data_to_backend_qos(info);

    Qos serialized_durability;
    eprosima::statistics_backend::subscriber::serialize(info.durability, durability_tag,
            serialized_durability);
    expected[durability_tag] = serialized_durability[durability_tag];
    Qos serialized_durability_service;
    eprosima::statistics_backend::subscriber::serialize(info.durability_service, durability_service_tag,
            serialized_durability_service);
    expected[durability_service_tag] = serialized_durability_service[durability_service_tag];
    Qos serialized_deadline;
    eprosima::statistics_backend::subscriber::serialize(info.deadline, deadline_tag, serialized_deadline);
    expected[deadline_tag] = serialized_deadline[deadline_tag];
    Qos serialized_latency_budget;
    eprosima::statistics_backend::subscriber::serialize(info.latency_budget, latency_budget_tag,
            serialized_latency_budget);
    expected[latency_budget_tag] = serialized_latency_budget[latency_budget_tag];
    Qos serialized_liveliness;
    eprosima::statistics_backend::subscriber::serialize(info.liveliness, liveliness_tag,
            serialized_liveliness);
    expected[liveliness_tag] = serialized_liveliness[liveliness_tag];
    Qos serialized_reliability;
    eprosima::statistics_backend::subscriber::serialize(info.reliability, reliability_tag,
            serialized_reliability);
    expected[reliability_tag] = serialized_reliability[reliability_tag];
    Qos serialized_lifespan;
    eprosima::statistics_backend::subscriber::serialize(info.lifespan, lifespan_tag, serialized_lifespan);
    expected[lifespan_tag] = serialized_lifespan[lifespan_tag];
    Qos serialized_user_data;
    eprosima::statistics_backend::subscriber::serialize(info.user_data, user_data_tag, serialized_user_data);
    expected[user_data_tag] = serialized_user_data[user_data_tag];
    Qos serialized_ownership;
    eprosima::statistics_backend::subscriber::serialize(info.ownership, ownership_tag,
            serialized_ownership);
    expected[ownership_tag] = serialized_ownership[ownership_tag];
    Qos serialized_ownership_strength;
    eprosima::statistics_backend::subscriber::serialize(info.ownership_strength, ownership_strength_tag,
            serialized_ownership_strength);
    expected[ownership_strength_tag] = serialized_ownership_strength[ownership_strength_tag];
    Qos serialized_destination_order;
    eprosima::statistics_backend::subscriber::serialize(info.destination_order, destination_order_tag,
            serialized_destination_order);
    expected[destination_order_tag] = serialized_destination_order[destination_order_tag];
    Qos serialized_presentation;
    eprosima::statistics_backend::subscriber::serialize(info.presentation, presentation_tag,
            serialized_presentation);
    expected[presentation_tag] = serialized_presentation[presentation_tag];
    Qos serialized_partition;
    eprosima::statistics_backend::subscriber::serialize(info.partition, partition_tag,
            serialized_partition);
    expected[partition_tag] = serialized_partition[partition_tag];
    Qos serialized_topic_data;
    eprosima::statistics_backend::subscriber::serialize(info.topic_data, topic_data_tag,
            serialized_topic_data);
    expected[topic_data_tag] = serialized_topic_data[topic_data_tag];
    Qos serialized_group_data;
    eprosima::statistics_backend::subscriber::serialize(info.group_data, group_data_tag,
            serialized_group_data);
    expected[group_data_tag] = serialized_group_data[group_data_tag];
    Qos serialized_representation;
    eprosima::statistics_backend::subscriber::serialize(info.representation, representation_tag,
            serialized_representation);
    expected[representation_tag] = serialized_representation[representation_tag];
    Qos serialized_disable_positive_acks;
    eprosima::statistics_backend::subscriber::serialize(info.disable_positive_acks,
            disable_positive_acks_tag, serialized_disable_positive_acks);
    expected[disable_positive_acks_tag] = serialized_disable_positive_acks[disable_positive_acks_tag];
    Qos serialized_data_sharing;
    eprosima::statistics_backend::subscriber::serialize(info.data_sharing, data_sharing_tag,
            serialized_data_sharing);
    expected[data_sharing_tag] = serialized_data_sharing[data_sharing_tag];

    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, reader_info_serializer)
{
    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData info;
    Qos serialized;
    Qos expected;

    // We will use default values here, specific values for each QoS
    // have been tested in other test
    serialized = eprosima::statistics_backend::subscriber::reader_proxy_data_to_backend_qos(info);

    Qos serialized_durability;
    eprosima::statistics_backend::subscriber::serialize(info.durability, durability_tag,
            serialized_durability);
    expected[durability_tag] = serialized_durability[durability_tag];
    Qos serialized_durabilityService;
    Qos serialized_deadline;
    eprosima::statistics_backend::subscriber::serialize(info.deadline, deadline_tag, serialized_deadline);
    expected[deadline_tag] = serialized_deadline[deadline_tag];
    Qos serialized_latency_budget;
    eprosima::statistics_backend::subscriber::serialize(info.latency_budget, latency_budget_tag,
            serialized_latency_budget);
    expected[latency_budget_tag] = serialized_latency_budget[latency_budget_tag];
    Qos serialized_liveliness;
    eprosima::statistics_backend::subscriber::serialize(info.liveliness, liveliness_tag,
            serialized_liveliness);
    expected[liveliness_tag] = serialized_liveliness[liveliness_tag];
    Qos serialized_reliability;
    eprosima::statistics_backend::subscriber::serialize(info.reliability, reliability_tag,
            serialized_reliability);
    expected[reliability_tag] = serialized_reliability[reliability_tag];
    Qos serialized_lifespan;
    eprosima::statistics_backend::subscriber::serialize(info.lifespan, lifespan_tag, serialized_lifespan);
    expected[lifespan_tag] = serialized_lifespan[lifespan_tag];
    Qos serialized_user_data;
    eprosima::statistics_backend::subscriber::serialize(info.user_data, user_data_tag, serialized_user_data);
    expected[user_data_tag] = serialized_user_data[user_data_tag];
    Qos serialized_time_based_filter;
    eprosima::statistics_backend::subscriber::serialize(info.time_based_filter, time_based_filter_tag,
            serialized_time_based_filter);
    expected[time_based_filter_tag] = serialized_time_based_filter[time_based_filter_tag];
    Qos serialized_ownership;
    eprosima::statistics_backend::subscriber::serialize(info.ownership, ownership_tag,
            serialized_ownership);
    expected[ownership_tag] = serialized_ownership[ownership_tag];
    Qos serialized_destination_order;
    eprosima::statistics_backend::subscriber::serialize(info.destination_order, destination_order_tag,
            serialized_destination_order);
    expected[destination_order_tag] = serialized_destination_order[destination_order_tag];
    Qos serialized_presentation;
    eprosima::statistics_backend::subscriber::serialize(info.presentation, presentation_tag,
            serialized_presentation);
    expected[presentation_tag] = serialized_presentation[presentation_tag];
    Qos serialized_partition;
    eprosima::statistics_backend::subscriber::serialize(info.partition, partition_tag,
            serialized_partition);
    expected[partition_tag] = serialized_partition[partition_tag];
    Qos serialized_topic_data;
    eprosima::statistics_backend::subscriber::serialize(info.topic_data, topic_data_tag,
            serialized_topic_data);
    expected[topic_data_tag] = serialized_topic_data[topic_data_tag];
    Qos serialized_group_data;
    eprosima::statistics_backend::subscriber::serialize(info.group_data, group_data_tag,
            serialized_group_data);
    expected[group_data_tag] = serialized_group_data[group_data_tag];
    Qos serialized_representation;
    eprosima::statistics_backend::subscriber::serialize(info.representation, representation_tag,
            serialized_representation);
    expected[representation_tag] = serialized_representation[representation_tag];
    Qos serialized_disable_positive_acks;
    eprosima::statistics_backend::subscriber::serialize(info.disable_positive_acks,
            disable_positive_acks_tag, serialized_disable_positive_acks);
    expected[disable_positive_acks_tag] = serialized_disable_positive_acks[disable_positive_acks_tag];
    Qos serialized_data_sharing;
    eprosima::statistics_backend::subscriber::serialize(info.data_sharing, data_sharing_tag,
            serialized_data_sharing);
    expected[data_sharing_tag] = serialized_data_sharing[data_sharing_tag];
    Qos serialized_type_consistency;
    eprosima::statistics_backend::subscriber::serialize(info.type_consistency, type_consistency_tag,
            serialized_type_consistency);
    expected[type_consistency_tag] = serialized_type_consistency[type_consistency_tag];

    EXPECT_EQ(expected, serialized);
}

// Windows dll do not export ParticipantBuiltinTopicData class members (private APIs)
#if !defined(_WIN32)
TEST(qos_serializer_tests, participant_info_serializer)
{
    eprosima::fastdds::rtps::ParticipantBuiltinTopicData data;
    data.vendor_id = eprosima::fastdds::rtps::c_VendorId_eProsima;
    Qos serialized;
    Qos expected;

    // We will use default values here, specific values for each QoS
    // have been tested in other test
    serialized = eprosima::statistics_backend::subscriber::participant_proxy_data_to_backend_qos(data);

    Qos serialized_leaseDuration;
    eprosima::statistics_backend::subscriber::serialize(data.lease_duration, lease_duration_tag,
            serialized_leaseDuration);
    expected[lease_duration_tag] = serialized_leaseDuration[lease_duration_tag];
    Qos serialized_properties;
    eprosima::statistics_backend::subscriber::serialize(data.properties, properties_tag, serialized_properties);
    expected[properties_tag] = serialized_properties[properties_tag];
    Qos serialized_userData;
    eprosima::statistics_backend::subscriber::serialize(data.user_data, user_data_tag, serialized_userData);
    expected[user_data_tag] = serialized_userData[user_data_tag];
    expected[vendor_id_tag] = Qos::array();
    expected[vendor_id_tag].push_back(0x01);
    expected[vendor_id_tag].push_back(0x0F);

    EXPECT_EQ(expected, serialized);
}
#endif //!defined(_WIN32)

TEST(qos_serializer_tests, reader_optional_qos_serializer)
{
    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData info;
    Qos serialized;
    Qos expected;

    // We will use default values here, specific values for each QoS
    // have been tested in other test

    /* 1. If no optional QoS information is provided, check that default values are serialized for each policy */
    ASSERT_FALSE(info.history.has_value());
    ASSERT_FALSE(info.resource_limits.has_value());
    ASSERT_FALSE(info.reader_data_lifecycle.has_value());
    ASSERT_FALSE(info.rtps_reliable_reader.has_value());
    ASSERT_FALSE(info.endpoint.has_value());
    ASSERT_FALSE(info.reader_resource_limits.has_value());

    serialized = eprosima::statistics_backend::subscriber::optional_qos_to_backend_qos(info);
    Qos serialized_history;
    eprosima::statistics_backend::subscriber::serialize(eprosima::fastdds::dds::HistoryQosPolicy(),
            history_tag, serialized_history);
    expected[history_tag] = serialized_history[history_tag];
    Qos serialized_resource_limits;
    eprosima::statistics_backend::subscriber::serialize(eprosima::fastdds::dds::ResourceLimitsQosPolicy(),
            resource_limits_tag, serialized_resource_limits);
    expected[resource_limits_tag] = serialized_resource_limits[resource_limits_tag];
    Qos serialized_reader_data_lifecycle;
    eprosima::statistics_backend::subscriber::serialize(eprosima::fastdds::dds::ReaderDataLifecycleQosPolicy(),
            reader_data_lifecycle_tag, serialized_reader_data_lifecycle);
    expected[reader_data_lifecycle_tag] = serialized_reader_data_lifecycle[reader_data_lifecycle_tag];
    Qos serialized_rtps_reliable_reader;
    eprosima::statistics_backend::subscriber::serialize(eprosima::fastdds::dds::RTPSReliableReaderQos(),
            rtps_reliable_reader_tag, serialized_rtps_reliable_reader);
    expected[rtps_reliable_reader_tag] = serialized_rtps_reliable_reader[rtps_reliable_reader_tag];
    Qos serialized_rtps_endpoint;
    eprosima::statistics_backend::subscriber::serialize(eprosima::fastdds::dds::RTPSEndpointQos(),
            rtps_endpoint_tag, serialized_rtps_endpoint);
    expected[rtps_endpoint_tag] = serialized_rtps_endpoint[rtps_endpoint_tag];
    Qos serialized_reader_resource_limits;
    eprosima::statistics_backend::subscriber::serialize(eprosima::fastdds::dds::ReaderResourceLimitsQos(),
            reader_resource_limits_tag, serialized_reader_resource_limits);
    expected[reader_resource_limits_tag] = serialized_reader_resource_limits[reader_resource_limits_tag];

    EXPECT_EQ(expected, serialized);
    expected.clear();
    serialized.clear();

    // 2. Set custom values for each optional QoS policy and check that they are serialized correctly
    info.history = eprosima::fastdds::dds::HistoryQosPolicy();
    info.history.value().kind = eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
    serialized_history.clear();
    eprosima::statistics_backend::subscriber::serialize(info.history.value(), history_tag, serialized_history);
    expected[history_tag] = serialized_history[history_tag];
    info.resource_limits = eprosima::fastdds::dds::ResourceLimitsQosPolicy();
    info.resource_limits.value().max_samples = 100;
    serialized_resource_limits.clear();
    eprosima::statistics_backend::subscriber::serialize(info.resource_limits.value(), resource_limits_tag,
            serialized_resource_limits);
    expected[resource_limits_tag] = serialized_resource_limits[resource_limits_tag];
    info.reader_data_lifecycle = eprosima::fastdds::dds::ReaderDataLifecycleQosPolicy();
    info.reader_data_lifecycle.value().autopurge_no_writer_samples_delay =
            eprosima::fastdds::dds::Duration_t(10, 0);
    serialized_reader_data_lifecycle.clear();
    eprosima::statistics_backend::subscriber::serialize(info.reader_data_lifecycle.value(),
            reader_data_lifecycle_tag, serialized_reader_data_lifecycle);
    expected[reader_data_lifecycle_tag] = serialized_reader_data_lifecycle[reader_data_lifecycle_tag];
    info.rtps_reliable_reader = eprosima::fastdds::dds::RTPSReliableReaderQos();
    info.rtps_reliable_reader.value().times.initial_acknack_delay = eprosima::fastdds::dds::Duration_t(0, 0);
    serialized_rtps_reliable_reader.clear();
    eprosima::statistics_backend::subscriber::serialize(
            info.rtps_reliable_reader.value(), rtps_reliable_reader_tag, serialized_rtps_reliable_reader);
    expected[rtps_reliable_reader_tag] = serialized_rtps_reliable_reader[rtps_reliable_reader_tag];
    info.endpoint = eprosima::fastdds::dds::RTPSEndpointQos();
    info.endpoint.value().entity_id = 10;
    serialized_rtps_endpoint.clear();
    eprosima::statistics_backend::subscriber::serialize(info.endpoint.value(), rtps_endpoint_tag,
            serialized_rtps_endpoint);
    expected[rtps_endpoint_tag] = serialized_rtps_endpoint[rtps_endpoint_tag];
    info.reader_resource_limits = eprosima::fastdds::dds::ReaderResourceLimitsQos();
    info.reader_resource_limits.value().max_samples_per_read = 1234;
    serialized_reader_resource_limits.clear();
    eprosima::statistics_backend::subscriber::serialize(
            info.reader_resource_limits.value(), reader_resource_limits_tag, serialized_reader_resource_limits);
    expected[reader_resource_limits_tag] = serialized_reader_resource_limits[reader_resource_limits_tag];

    serialized = eprosima::statistics_backend::subscriber::optional_qos_to_backend_qos(info);
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, writer_optional_qos_serializer)
{
    eprosima::fastdds::rtps::PublicationBuiltinTopicData info;
    Qos serialized;
    Qos expected;

    // We will use default values here, specific values for each QoS
    // have been tested in other test

    /* 1. If no optional QoS information is provided, check that default values are serialized for each policy */
    ASSERT_FALSE(info.history.has_value());
    ASSERT_FALSE(info.resource_limits.has_value());
    ASSERT_FALSE(info.transport_priority.has_value());
    ASSERT_FALSE(info.writer_data_lifecycle.has_value());
    ASSERT_FALSE(info.publish_mode.has_value());
    ASSERT_FALSE(info.rtps_reliable_writer.has_value());
    ASSERT_FALSE(info.endpoint.has_value());
    ASSERT_FALSE(info.writer_resource_limits.has_value());

    serialized = eprosima::statistics_backend::subscriber::optional_qos_to_backend_qos(info);
    Qos serialized_history;
    eprosima::statistics_backend::subscriber::serialize(eprosima::fastdds::dds::HistoryQosPolicy(),
            history_tag, serialized_history);
    expected[history_tag] = serialized_history[history_tag];
    Qos serialized_resource_limits;
    eprosima::statistics_backend::subscriber::serialize(eprosima::fastdds::dds::ResourceLimitsQosPolicy(),
            resource_limits_tag, serialized_resource_limits);
    expected[resource_limits_tag] = serialized_resource_limits[resource_limits_tag];
    Qos serialized_transport_priority;
    eprosima::statistics_backend::subscriber::serialize(eprosima::fastdds::dds::TransportPriorityQosPolicy(),
            transport_priority_tag, serialized_transport_priority);
    expected[transport_priority_tag] = serialized_transport_priority[transport_priority_tag];
    Qos serialized_writer_data_lifecycle;
    eprosima::statistics_backend::subscriber::serialize(eprosima::fastdds::dds::WriterDataLifecycleQosPolicy(),
            writer_data_lifecycle_tag, serialized_writer_data_lifecycle);
    expected[writer_data_lifecycle_tag] = serialized_writer_data_lifecycle[writer_data_lifecycle_tag];
    Qos serialized_publish_mode;
    eprosima::statistics_backend::subscriber::serialize(eprosima::fastdds::dds::PublishModeQosPolicy(),
            publish_mode_tag, serialized_publish_mode);
    expected[publish_mode_tag] = serialized_publish_mode[publish_mode_tag];
    Qos serialized_rtps_reliable_writer;
    eprosima::statistics_backend::subscriber::serialize(eprosima::fastdds::dds::RTPSReliableWriterQos(),
            rtps_reliable_writer_tag, serialized_rtps_reliable_writer);
    expected[rtps_reliable_writer_tag] = serialized_rtps_reliable_writer[rtps_reliable_writer_tag];
    Qos serialized_rtps_endpoint;
    eprosima::statistics_backend::subscriber::serialize(eprosima::fastdds::dds::RTPSEndpointQos(),
            rtps_endpoint_tag, serialized_rtps_endpoint);
    expected[rtps_endpoint_tag] = serialized_rtps_endpoint[rtps_endpoint_tag];
    Qos serialized_writer_resource_limits;
    eprosima::statistics_backend::subscriber::serialize(eprosima::fastdds::dds::WriterResourceLimitsQos(),
            writer_resource_limits_tag, serialized_writer_resource_limits);
    expected[writer_resource_limits_tag] = serialized_writer_resource_limits[writer_resource_limits_tag];

    EXPECT_EQ(expected, serialized);
    expected.clear();
    serialized.clear();

    // 2. Set custom values for each optional QoS policy and check that they are serialized correctly
    info.history = eprosima::fastdds::dds::HistoryQosPolicy();
    info.history.value().kind = eprosima::fastdds::dds::HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
    serialized_history.clear();
    eprosima::statistics_backend::subscriber::serialize(info.history.value(), history_tag, serialized_history);
    expected[history_tag] = serialized_history[history_tag];
    info.resource_limits = eprosima::fastdds::dds::ResourceLimitsQosPolicy();
    info.resource_limits.value().max_samples = 100;
    serialized_resource_limits.clear();
    eprosima::statistics_backend::subscriber::serialize(info.resource_limits.value(), resource_limits_tag,
            serialized_resource_limits);
    expected[resource_limits_tag] = serialized_resource_limits[resource_limits_tag];
    info.transport_priority = eprosima::fastdds::dds::TransportPriorityQosPolicy();
    info.transport_priority.value().value = 10;
    serialized_transport_priority.clear();
    eprosima::statistics_backend::subscriber::serialize(info.transport_priority.value(),
            transport_priority_tag, serialized_transport_priority);
    expected[transport_priority_tag] = serialized_transport_priority[transport_priority_tag];
    info.writer_data_lifecycle = eprosima::fastdds::dds::WriterDataLifecycleQosPolicy();
    info.writer_data_lifecycle.value().autodispose_unregistered_instances = false;
    serialized_writer_data_lifecycle.clear();
    eprosima::statistics_backend::subscriber::serialize(info.writer_data_lifecycle.value(),
            writer_data_lifecycle_tag, serialized_writer_data_lifecycle);
    expected[writer_data_lifecycle_tag] = serialized_writer_data_lifecycle[writer_data_lifecycle_tag];
    info.publish_mode = eprosima::fastdds::dds::PublishModeQosPolicy();
    info.publish_mode.value().kind = eprosima::fastdds::dds::PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE;
    serialized_publish_mode.clear();
    eprosima::statistics_backend::subscriber::serialize(info.publish_mode.value(), publish_mode_tag,
            serialized_publish_mode);
    expected[publish_mode_tag] = serialized_publish_mode[publish_mode_tag];
    info.rtps_reliable_writer = eprosima::fastdds::dds::RTPSReliableWriterQos();
    info.rtps_reliable_writer.value().disable_heartbeat_piggyback = true;
    serialized_rtps_reliable_writer.clear();
    eprosima::statistics_backend::subscriber::serialize(
            info.rtps_reliable_writer.value(), rtps_reliable_writer_tag, serialized_rtps_reliable_writer);
    expected[rtps_reliable_writer_tag] = serialized_rtps_reliable_writer[rtps_reliable_writer_tag];
    info.endpoint = eprosima::fastdds::dds::RTPSEndpointQos();
    info.endpoint.value().entity_id = 10;
    serialized_rtps_endpoint.clear();
    eprosima::statistics_backend::subscriber::serialize(info.endpoint.value(), rtps_endpoint_tag,
            serialized_rtps_endpoint);
    expected[rtps_endpoint_tag] = serialized_rtps_endpoint[rtps_endpoint_tag];
    info.writer_resource_limits = eprosima::fastdds::dds::WriterResourceLimitsQos();
    info.writer_resource_limits.value().matched_subscriber_allocation =
            eprosima::fastdds::ResourceLimitedContainerConfig(0, 1, 2);
    serialized_writer_resource_limits.clear();
    eprosima::statistics_backend::subscriber::serialize(
            info.writer_resource_limits.value(), writer_resource_limits_tag, serialized_writer_resource_limits);
    expected[writer_resource_limits_tag] = serialized_writer_resource_limits[writer_resource_limits_tag];

    serialized = eprosima::statistics_backend::subscriber::optional_qos_to_backend_qos(info);
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, participant_optional_qos_serializer)
{
    eprosima::fastdds::rtps::ParticipantBuiltinTopicData info;
    Qos serialized;
    Qos expected;

    // We will use default values here, specific values for each QoS
    // have been tested in other test

    /* 1. If no optional QoS information is provided, check that default values are serialized for each policy */
    ASSERT_FALSE(info.wire_protocol.has_value());

    serialized = eprosima::statistics_backend::subscriber::optional_qos_to_backend_qos(info);
    Qos serialized_wire_protocol;
    eprosima::statistics_backend::subscriber::serialize(eprosima::fastdds::dds::WireProtocolConfigQos(),
            wire_protocol_config_tag, serialized_wire_protocol);
    expected[wire_protocol_config_tag] = serialized_wire_protocol[wire_protocol_config_tag];

    EXPECT_EQ(expected, serialized);
    expected.clear();
    serialized.clear();

    // 2. Set custom values for each optional QoS policy and check that they are serialized correctly
    info.wire_protocol = eprosima::fastdds::dds::WireProtocolConfigQos();
    info.wire_protocol.value().participant_id = 10;
    serialized_wire_protocol.clear();
    eprosima::statistics_backend::subscriber::serialize(info.wire_protocol.value(), wire_protocol_config_tag, serialized_wire_protocol);
    expected[wire_protocol_config_tag] = serialized_wire_protocol[wire_protocol_config_tag];

    serialized = eprosima::statistics_backend::subscriber::optional_qos_to_backend_qos(info);
    EXPECT_EQ(expected, serialized);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
