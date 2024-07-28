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
    eprosima::fastdds::dds::
            Duration_t qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos = eprosima::fastdds::dds::
                    Duration_t(5, 10);
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

    qos.period = eprosima::fastdds::dds::
                    Duration_t (5, 0);
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

    qos.duration = eprosima::fastdds::dds::
                    Duration_t (0, 5);
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
    qos.announcement_period = eprosima::fastdds::dds::
                    Duration_t (10, 10);
    qos.lease_duration = eprosima::fastdds::dds::
                    Duration_t (10, 10);
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
    qos.max_blocking_time = eprosima::fastdds::dds::
                    Duration_t (10, 10);
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

    qos.minimum_separation = eprosima::fastdds::dds::
                    Duration_t (10, 10);
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
    qos.service_cleanup_delay = eprosima::fastdds::dds::
                    Duration_t (10, 10);
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

    qos.duration = eprosima::fastdds::dds::
                    Duration_t (10, 10);
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
    qos.duration = eprosima::fastdds::dds::
                    Duration_t (10, 10);
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

    qos.kind =  SYNCHRONOUS_PUBLISH_MODE;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = publish_mode_sync_tag;
    EXPECT_EQ(expected, serialized);

    qos.kind =  ASYNCHRONOUS_PUBLISH_MODE;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()][kind_tag] = publish_mode_async_tag;
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

TEST(qos_serializer_tests, writer_info_serializer)
{
    eprosima::fastdds::rtps::PublicationBuiltinTopicData info;
    Qos serialized;
    Qos expected;

    // We will use default values here, specific values for each QoS
    // have been tested in other test
    serialized = eprosima::statistics_backend::subscriber::writer_proxy_data_to_backend_qos(info);

    // TODO: Implement serialization for the following fields when available in Fast DDS
    // Qos serialized_timeBasedFilter;
    // eprosima::statistics_backend::subscriber::serialize(info.timeBasedFilter, time_based_filter_tag,
    //         serialized_timeBasedFilter);
    // expected[time_based_filter_tag] = serialized_timeBasedFilter[time_based_filter_tag];

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
    // Qos serialized_publishMode;
    // eprosima::statistics_backend::subscriber::serialize(info.publishMode, publish_mode_tag,
    // serialized_publishMode);
    // expected[publish_mode_tag] = serialized_publishMode[publish_mode_tag];
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

    // TODO: Implement when available in Fast DDS
    // eprosima::statistics_backend::subscriber::serialize(info.durabilityService, durability_service_tag,
    //         serialized_durabilityService);
    // expected[durability_service_tag] = serialized_durabilityService[durability_service_tag];
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

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
