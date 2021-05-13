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

#include <fastdds/dds/core/policy/QosPolicies.hpp>

#include "subscriber/QosSerializer.hpp"

#include <database/data.hpp>
#include <topic_types/types.h>

#include "gtest/gtest.h"

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

    expected[field.c_str()]["kind"] = "VOLATILE_DURABILITY_QOS";
    EXPECT_EQ(expected, serialized);

    qos.kind = TRANSIENT_LOCAL_DURABILITY_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()]["kind"] = "TRANSIENT_LOCAL_DURABILITY_QOS";
    EXPECT_EQ(expected, serialized);

    qos.kind = TRANSIENT_DURABILITY_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()]["kind"] = "TRANSIENT_DURABILITY_QOS";
    EXPECT_EQ(expected, serialized);

    qos.kind = PERSISTENT_DURABILITY_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()]["kind"] = "PERSISTENT_DURABILITY_QOS";
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, duration)
{
    eprosima::fastrtps::Duration_t qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos = eprosima::fastrtps::Duration_t(5, 10);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()]["seconds"] = 5;
    expected[field.c_str()]["nanoseconds"] = 10;
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, deadline_qos_policy)
{
    DeadlineQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.period = eprosima::fastrtps::Duration_t (5, 0);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    Qos serialized_period;

    eprosima::statistics_backend::subscriber::serialize(qos.period, "period", serialized_period);
    expected[field.c_str()]["period"] = serialized_period["period"];
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, latency_budget_qos_policy)
{
    LatencyBudgetQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.duration = eprosima::fastrtps::Duration_t (0, 5);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    Qos serialized_duration;

    eprosima::statistics_backend::subscriber::serialize(qos.duration, "duration", serialized_duration);
    expected[field.c_str()]["duration"] = serialized_duration["duration"];
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, liveliness_qos_policy)
{
    LivelinessQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.kind =  AUTOMATIC_LIVELINESS_QOS;
    qos.announcement_period = eprosima::fastrtps::Duration_t (10, 10);
    qos.lease_duration = eprosima::fastrtps::Duration_t (10, 10);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    Qos serialized_duration;
    Qos serialized_period;

    eprosima::statistics_backend::subscriber::serialize(qos.lease_duration, "lease_duration", serialized_duration);
    eprosima::statistics_backend::subscriber::serialize(qos.announcement_period, "announcement_period",
            serialized_period);
    expected[field.c_str()]["kind"] = "AUTOMATIC_LIVELINESS_QOS";
    expected[field.c_str()]["lease_duration"] = serialized_duration["lease_duration"];
    expected[field.c_str()]["announcement_period"] = serialized_period["announcement_period"];
    EXPECT_EQ(expected, serialized);

    qos.kind =  MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()]["kind"] = "MANUAL_BY_PARTICIPANT_LIVELINESS_QOS";
    EXPECT_EQ(expected, serialized);

    qos.kind =  MANUAL_BY_TOPIC_LIVELINESS_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()]["kind"] = "MANUAL_BY_TOPIC_LIVELINESS_QOS";
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, reliability_qos_policy)
{
    ReliabilityQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.kind =  BEST_EFFORT_RELIABILITY_QOS;
    qos.max_blocking_time = eprosima::fastrtps::Duration_t (10, 10);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    Qos serialized_blocking_time;

    eprosima::statistics_backend::subscriber::serialize(qos.max_blocking_time, "max_blocking_time",
            serialized_blocking_time);
    expected[field.c_str()]["kind"] = "BEST_EFFORT_RELIABILITY_QOS";
    expected[field.c_str()]["max_blocking_time"] = serialized_blocking_time["max_blocking_time"];
    EXPECT_EQ(expected, serialized);

    qos.kind =  RELIABLE_RELIABILITY_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()]["kind"] = "RELIABLE_RELIABILITY_QOS";
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

    expected[field.c_str()]["kind"] = "SHARED_OWNERSHIP_QOS";
    EXPECT_EQ(expected, serialized);

    qos.kind =  EXCLUSIVE_OWNERSHIP_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()]["kind"] = "EXCLUSIVE_OWNERSHIP_QOS";
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

    expected[field.c_str()]["kind"] = "BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS";
    EXPECT_EQ(expected, serialized);

    qos.kind =  BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()]["kind"] = "BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS";
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, topic_data_qos_policy)
{
    TopicDataQosPolicy qos(PID_PAD);
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
    UserDataQosPolicy qos(PID_PAD);
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
    GroupDataQosPolicy qos(PID_PAD);
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

    qos.minimum_separation = eprosima::fastrtps::Duration_t (10, 10);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    Qos serialized_minimum_separation;

    eprosima::statistics_backend::subscriber::serialize(qos.minimum_separation, "minimum_separation",
            serialized_minimum_separation);
    expected[field.c_str()]["minimum_separation"] = serialized_minimum_separation["minimum_separation"];
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

    expected[field.c_str()]["access_scope"] = "INSTANCE_PRESENTATION_QOS";
    expected[field.c_str()]["coherent_access"] = "true";
    expected[field.c_str()]["ordered_access"] = "false";
    EXPECT_EQ(expected, serialized);

    qos.access_scope =  TOPIC_PRESENTATION_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()]["access_scope"] = "TOPIC_PRESENTATION_QOS";
    EXPECT_EQ(expected, serialized);

    qos.access_scope =  GROUP_PRESENTATION_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()]["access_scope"] = "GROUP_PRESENTATION_QOS";
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
    qos.service_cleanup_delay = eprosima::fastrtps::Duration_t (10, 10);
    qos.history_depth =  10;
    qos.max_instances =  20;
    qos.max_samples =  30;
    qos.max_samples_per_instance =  40;
    qos.history_depth =  50;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    Qos serialized_service_cleanup_delay;

    eprosima::statistics_backend::subscriber::serialize(qos.service_cleanup_delay, "service_cleanup_delay",
            serialized_service_cleanup_delay);
    expected[field.c_str()]["history_kind"] = "KEEP_LAST_HISTORY_QOS";
    expected[field.c_str()]["service_cleanup_delay"] = serialized_service_cleanup_delay["service_cleanup_delay"];
    expected[field.c_str()]["history_depth"] = 10;
    expected[field.c_str()]["max_instances"] = 20;
    expected[field.c_str()]["max_samples"] = 30;
    expected[field.c_str()]["max_samples_per_instance"] = 40;
    expected[field.c_str()]["history_depth"] = 50;
    EXPECT_EQ(expected, serialized);

    qos.history_kind =  KEEP_ALL_HISTORY_QOS;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()]["history_kind"] = "KEEP_ALL_HISTORY_QOS";
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, lifespan_qos_policy)
{
    LifespanQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.duration = eprosima::fastrtps::Duration_t (10, 10);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    Qos serialized_duration;

    eprosima::statistics_backend::subscriber::serialize(qos.duration, "duration", serialized_duration);
    expected[field.c_str()]["duration"] = serialized_duration["duration"];
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

    expected[field.c_str()].push_back("XCDR_DATA_REPRESENTATION");
    expected[field.c_str()].push_back("XML_DATA_REPRESENTATION");
    expected[field.c_str()].push_back("XCDR2_DATA_REPRESENTATION");
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

    expected[field.c_str()]["kind"] = "DISALLOW_TYPE_COERCION";
    expected[field.c_str()]["ignore_sequence_bounds"] = "true";
    expected[field.c_str()]["ignore_string_bounds"] = "false";
    expected[field.c_str()]["ignore_member_names"] = "false";
    expected[field.c_str()]["prevent_type_widening"] = "false";
    expected[field.c_str()]["force_type_validation"] = "true";
    EXPECT_EQ(expected, serialized);

    qos.m_kind =  ALLOW_TYPE_COERCION;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()]["kind"] = "ALLOW_TYPE_COERCION";
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, disable_positive_ack_qos_policy)
{
    DisablePositiveACKsQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.enabled = true;
    qos.duration = eprosima::fastrtps::Duration_t (10, 10);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    Qos serialized_duration;

    eprosima::statistics_backend::subscriber::serialize(qos.duration, "duration", serialized_duration);
    expected[field.c_str()]["duration"] = serialized_duration["duration"];
    expected[field.c_str()]["enabled"] = "true";
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

    expected[field.c_str()]["kind"] = "OFF";
    expected[field.c_str()]["max_domains"] = 0;
    expected[field.c_str()]["shm_directory"] = "";
    expected[field.c_str()]["domain_ids"] = Qos::array();
    EXPECT_EQ(expected, serialized);

    qos.automatic("datasharing_directory");
    qos.add_domain_id((uint16_t) 25);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()]["kind"] = "AUTO";
    expected[field.c_str()]["max_domains"] = 0;
    expected[field.c_str()]["shm_directory"] = "datasharing_directory";
    expected[field.c_str()]["domain_ids"].push_back(25);
    EXPECT_EQ(expected, serialized);    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    qos.on("datasharing_directory");
    qos.set_max_domains(2);
    qos.add_domain_id((uint16_t) 25);
    qos.add_domain_id((uint64_t) 30);
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()]["kind"] = "ON";
    expected[field.c_str()]["max_domains"] = 2;
    expected[field.c_str()]["shm_directory"] = "datasharing_directory";
    expected[field.c_str()]["domain_ids"] = Qos::array();
    expected[field.c_str()]["domain_ids"].push_back(25);
    expected[field.c_str()]["domain_ids"].push_back(30);
    EXPECT_EQ(expected, serialized);    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);
}

TEST(qos_serializer_tests, ownership_strength_qos_policy)
{
    OwnershipStrengthQosPolicy qos;
    Qos serialized;
    Qos expected;
    std::string field("field");

    qos.value =  5;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()]["value"] = 5;
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

    expected[field.c_str()]["kind"] = "SYNCHRONOUS_PUBLISH_MODE";
    EXPECT_EQ(expected, serialized);

    qos.kind =  ASYNCHRONOUS_PUBLISH_MODE;
    eprosima::statistics_backend::subscriber::serialize(qos, field, serialized);

    expected[field.c_str()]["kind"] = "ASYNCHRONOUS_PUBLISH_MODE";
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
    property["name"] = "property_name";
    property["value"] = "property_value";
    expected[field.c_str()].push_back(property);
    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, writer_info_serializer)
{
    eprosima::fastrtps::rtps::WriterDiscoveryInfo info(
        eprosima::fastrtps::rtps::WriterProxyData(10, 10));
    Qos serialized;
    Qos expected;

    // We will use default values here, specific values for each QoS
    // have been tested in other test
    serialized = eprosima::statistics_backend::subscriber::writer_info_to_backend_qos(info);

    Qos serialized_durability;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_durability, "durability",
            serialized_durability);
    expected["durability"] = serialized_durability["durability"];
    Qos serialized_durabilityService;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_durabilityService, "durability_service",
            serialized_durabilityService);
    expected["durability_service"] = serialized_durabilityService["durability_service"];
    Qos serialized_deadline;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_deadline, "deadline", serialized_deadline);
    expected["deadline"] = serialized_deadline["deadline"];
    Qos serialized_latencyBudget;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_latencyBudget, "latency_budget",
            serialized_latencyBudget);
    expected["latency_budget"] = serialized_latencyBudget["latency_budget"];
    Qos serialized_liveliness;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_liveliness, "liveliness",
            serialized_liveliness);
    expected["liveliness"] = serialized_liveliness["liveliness"];
    Qos serialized_reliability;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_reliability, "reliability",
            serialized_reliability);
    expected["reliability"] = serialized_reliability["reliability"];
    Qos serialized_lifespan;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_lifespan, "lifespan", serialized_lifespan);
    expected["lifespan"] = serialized_lifespan["lifespan"];
    Qos serialized_userData;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_userData, "user_data", serialized_userData);
    expected["user_data"] = serialized_userData["user_data"];
    Qos serialized_timeBasedFilter;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_timeBasedFilter, "time_based_filter",
            serialized_timeBasedFilter);
    expected["time_based_filter"] = serialized_timeBasedFilter["time_based_filter"];
    Qos serialized_ownership;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_ownership, "ownership", serialized_ownership);
    expected["ownership"] = serialized_ownership["ownership"];
    Qos serialized_ownershipStrength;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_ownershipStrength, "ownership_strength",
            serialized_ownershipStrength);
    expected["ownership_strength"] = serialized_ownershipStrength["ownership_strength"];
    Qos serialized_destinationOrder;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_destinationOrder, "destination_order",
            serialized_destinationOrder);
    expected["destination_order"] = serialized_destinationOrder["destination_order"];
    Qos serialized_presentation;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_presentation, "presentation",
            serialized_presentation);
    expected["presentation"] = serialized_presentation["presentation"];
    Qos serialized_partition;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_partition, "partition", serialized_partition);
    expected["partition"] = serialized_partition["partition"];
    Qos serialized_topicData;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_topicData, "topic_data",
            serialized_topicData);
    expected["topic_data"] = serialized_topicData["topic_data"];
    Qos serialized_groupData;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_groupData, "group_data",
            serialized_groupData);
    expected["group_data"] = serialized_groupData["group_data"];
    Qos serialized_publishMode;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_publishMode, "publish_mode",
            serialized_publishMode);
    expected["publish_mode"] = serialized_publishMode["publish_mode"];
    Qos serialized_representation;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.representation, "representation",
            serialized_representation);
    expected["representation"] = serialized_representation["representation"];
    Qos serialized_m_disablePositiveACKs;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_disablePositiveACKs, "disable_positive_acks",
            serialized_m_disablePositiveACKs);
    expected["disable_positive_acks"] = serialized_m_disablePositiveACKs["disable_positive_acks"];
    Qos serialized_data_sharing;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.data_sharing, "data_sharing",
            serialized_data_sharing);
    expected["data_sharing"] = serialized_data_sharing["data_sharing"];

    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, reader_info_serializer)
{
    eprosima::fastrtps::rtps::ReaderDiscoveryInfo info(
        eprosima::fastrtps::rtps::ReaderProxyData(10, 10));
    Qos serialized;
    Qos expected;

    // We will use default values here, specific values for each QoS
    // have been tested in other test
    serialized = eprosima::statistics_backend::subscriber::reader_info_to_backend_qos(info);

    Qos serialized_durability;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_durability, "durability",
            serialized_durability);
    expected["durability"] = serialized_durability["durability"];
    Qos serialized_durabilityService;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_durabilityService, "durability_service",
            serialized_durabilityService);
    expected["durability_service"] = serialized_durabilityService["durability_service"];
    Qos serialized_deadline;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_deadline, "deadline", serialized_deadline);
    expected["deadline"] = serialized_deadline["deadline"];
    Qos serialized_latencyBudget;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_latencyBudget, "latency_budget",
            serialized_latencyBudget);
    expected["latency_budget"] = serialized_latencyBudget["latency_budget"];
    Qos serialized_liveliness;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_liveliness, "liveliness",
            serialized_liveliness);
    expected["liveliness"] = serialized_liveliness["liveliness"];
    Qos serialized_reliability;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_reliability, "reliability",
            serialized_reliability);
    expected["reliability"] = serialized_reliability["reliability"];
    Qos serialized_lifespan;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_lifespan, "lifespan", serialized_lifespan);
    expected["lifespan"] = serialized_lifespan["lifespan"];
    Qos serialized_userData;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_userData, "user_data", serialized_userData);
    expected["user_data"] = serialized_userData["user_data"];
    Qos serialized_timeBasedFilter;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_timeBasedFilter, "time_based_filter",
            serialized_timeBasedFilter);
    expected["time_based_filter"] = serialized_timeBasedFilter["time_based_filter"];
    Qos serialized_ownership;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_ownership, "ownership", serialized_ownership);
    expected["ownership"] = serialized_ownership["ownership"];
    Qos serialized_destinationOrder;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_destinationOrder, "destination_order",
            serialized_destinationOrder);
    expected["destination_order"] = serialized_destinationOrder["destination_order"];
    Qos serialized_presentation;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_presentation, "presentation",
            serialized_presentation);
    expected["presentation"] = serialized_presentation["presentation"];
    Qos serialized_partition;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_partition, "partition", serialized_partition);
    expected["partition"] = serialized_partition["partition"];
    Qos serialized_topicData;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_topicData, "topic_data",
            serialized_topicData);
    expected["topic_data"] = serialized_topicData["topic_data"];
    Qos serialized_groupData;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_groupData, "group_data",
            serialized_groupData);
    expected["group_data"] = serialized_groupData["group_data"];
    Qos serialized_representation;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.representation, "representation",
            serialized_representation);
    expected["representation"] = serialized_representation["representation"];
    Qos serialized_m_disablePositiveACKs;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.m_disablePositiveACKs, "disable_positive_acks",
            serialized_m_disablePositiveACKs);
    expected["disable_positive_acks"] = serialized_m_disablePositiveACKs["disable_positive_acks"];
    Qos serialized_data_sharing;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.data_sharing, "data_sharing",
            serialized_data_sharing);
    expected["data_sharing"] = serialized_data_sharing["data_sharing"];
    Qos serialized_type_consistency;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_qos.type_consistency, "type_consistency",
            serialized_type_consistency);
    expected["type_consistency"] = serialized_type_consistency["type_consistency"];

    EXPECT_EQ(expected, serialized);
}

TEST(qos_serializer_tests, participant_info_serializer)
{
    eprosima::fastrtps::rtps::RTPSParticipantAllocationAttributes attributes;
    eprosima::fastrtps::rtps::ParticipantProxyData data(attributes);
    data.m_VendorId = eprosima::fastrtps::rtps::c_VendorId_eProsima;
    data.m_availableBuiltinEndpoints = 101;
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo info(data);
    Qos serialized;
    Qos expected;

    // We will use default values here, specific values for each QoS
    // have been tested in other test
    serialized = eprosima::statistics_backend::subscriber::participant_info_to_backend_qos(info);

    Qos serialized_leaseDuration;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_leaseDuration, "lease_duration",
            serialized_leaseDuration);
    expected["lease_duration"] = serialized_leaseDuration["lease_duration"];
    Qos serialized_properties;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_properties, "properties", serialized_properties);
    expected["properties"] = serialized_properties["properties"];
    Qos serialized_userData;
    eprosima::statistics_backend::subscriber::serialize(info.info.m_userData, "user_data", serialized_userData);
    expected["user_data"] = serialized_userData["user_data"];

    expected["available_builtin_endpoints"] = 101;
    expected["vendor_id"] = Qos::array();
    expected["vendor_id"].push_back(0x01);
    expected["vendor_id"].push_back(0x0F);

    EXPECT_EQ(expected, serialized);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
