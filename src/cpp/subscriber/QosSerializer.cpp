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

/**
 * @file QosSerializer.cpp
 */

#include "QosSerializer.hpp"
#include "QosSerializerTags.hpp"

#include <fastdds/dds/core/policy/QosPolicies.hpp>


namespace eprosima {
namespace statistics_backend {
namespace subscriber {


template <>
void serialize<fastdds::dds::DurabilityQosPolicy> (
        const fastdds::dds::DurabilityQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{

    database::Qos kind;
    switch (qos.durabilityKind())
    {
        case fastdds::dds::VOLATILE_DURABILITY_QOS:
            kind[kind_tag] = durability_volatile_tag;
            break;
        case fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS:
            kind[kind_tag] = durability_transient_local_tag;
            break;
        case fastdds::dds::TRANSIENT_DURABILITY_QOS:
            kind[kind_tag] = durability_transient_tag;
            break;
        case fastdds::dds::PERSISTENT_DURABILITY_QOS:
            kind[kind_tag] = durability_persistent_tag;
            break;
    }
    serialized[fieldname] = kind;
}

template <>
void serialize<fastrtps::Duration_t> (
        const fastrtps::Duration_t& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos duration;
    duration[duration_seconds_tag] = qos.seconds;
    duration[duration_nanoseconds_tag] = qos.nanosec;
    serialized[fieldname] = duration;
}

template <>
void serialize<fastdds::dds::DeadlineQosPolicy> (
        const fastdds::dds::DeadlineQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos deadline;
    serialize(qos.period, duration_period_tag, deadline);
    serialized[fieldname] = deadline;
}

template <>
void serialize<fastdds::dds::LatencyBudgetQosPolicy> (
        const fastdds::dds::LatencyBudgetQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos latency;
    serialize(qos.duration, duration_tag, latency);
    serialized[fieldname] = latency;
}

template <>
void serialize<fastdds::dds::LivelinessQosPolicy> (
        const fastdds::dds::LivelinessQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos liveliness;
    switch (qos.kind)
    {
        case fastdds::dds::AUTOMATIC_LIVELINESS_QOS:
            liveliness[kind_tag] = liveliness_automatic_tag;
            break;
        case fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
            liveliness[kind_tag] = liveliness_manual_participant_tag;
            break;
        case fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS:
            liveliness[kind_tag] = liveliness_manual_topic_tag;
            break;
    }
    serialize(qos.announcement_period, announcement_period_tag, liveliness);
    serialize(qos.lease_duration, lease_duration_tag, liveliness);
    serialized[fieldname] = liveliness;
}

template <>
void serialize<fastdds::dds::ReliabilityQosPolicy> (
        const fastdds::dds::ReliabilityQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos reliability;
    switch (qos.kind)
    {
        case fastdds::dds::BEST_EFFORT_RELIABILITY_QOS:
            reliability[kind_tag] = reliability_best_effort_tag;
            break;
        case fastdds::dds::RELIABLE_RELIABILITY_QOS:
            reliability[kind_tag] = reliability_reliable_tag;
            break;
    }
    serialize(qos.max_blocking_time, max_blocking_time_tag, reliability);
    serialized[fieldname] = reliability;
}

template <>
void serialize<fastdds::dds::OwnershipQosPolicy> (
        const fastdds::dds::OwnershipQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos ownership;
    switch (qos.kind)
    {
        case fastdds::dds::SHARED_OWNERSHIP_QOS:
            ownership[kind_tag] = ownership_shared_tag;
            break;
        case fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS:
            ownership[kind_tag] = ownership_exclusive_tag;
            break;
    }
    serialized[fieldname] = ownership;
}

template <>
void serialize<fastdds::dds::DestinationOrderQosPolicy> (
        const fastdds::dds::DestinationOrderQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos destination_order;
    switch (qos.kind)
    {
        case fastdds::dds::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS:
            destination_order[kind_tag] = destination_order_reception_tag;
            break;
        case fastdds::dds::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS:
            destination_order[kind_tag] = destination_order_source_tag;
            break;
    }
    serialized[fieldname] = destination_order;
}

template <>
void serialize<fastdds::dds::GenericDataQosPolicy> (
        const fastdds::dds::GenericDataQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    std::stringstream data;
    data << std::hex;
    for (auto v : qos.getValue())
    {
        data << static_cast<int>(v);
    }
    serialized[fieldname] = data.str();
}

template <>
void serialize<fastdds::dds::TopicDataQosPolicy> (
        const fastdds::dds::TopicDataQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    serialize<fastdds::dds::GenericDataQosPolicy>(qos, fieldname, serialized);
}

template <>
void serialize<fastdds::dds::UserDataQosPolicy> (
        const fastdds::dds::UserDataQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    serialize<fastdds::dds::GenericDataQosPolicy>(qos, fieldname, serialized);
}

template <>
void serialize<fastdds::dds::GroupDataQosPolicy> (
        const fastdds::dds::GroupDataQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    serialize<fastdds::dds::GenericDataQosPolicy>(qos, fieldname, serialized);
}

template <>
void serialize<fastdds::dds::TimeBasedFilterQosPolicy> (
        const fastdds::dds::TimeBasedFilterQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos filter;
    serialize(qos.minimum_separation, minimum_separation_tag, filter);
    serialized[fieldname] = filter;
}

template <>
void serialize<bool> (
        const bool& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    if (qos)
    {
        serialized[fieldname] = true_tag;
    }
    else
    {
        serialized[fieldname] = false_tag;
    }
}

template <>
void serialize<fastdds::dds::PresentationQosPolicy> (
        const fastdds::dds::PresentationQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos presentation;
    switch (qos.access_scope)
    {
        case fastdds::dds::INSTANCE_PRESENTATION_QOS:
            presentation[access_scope_tag] = access_scope_instance_tag;
            break;
        case fastdds::dds::TOPIC_PRESENTATION_QOS:
            presentation[access_scope_tag] = access_scope_topic_tag;
            break;
        case fastdds::dds::GROUP_PRESENTATION_QOS:
            presentation[access_scope_tag] = access_scope_group_tag;
            break;
    }

    serialize(qos.coherent_access, coherent_access_tag, presentation);
    serialize(qos.ordered_access, ordered_access_tag, presentation);
    serialized[fieldname] = presentation;
}

template <>
void serialize<fastdds::dds::PartitionQosPolicy> (
        const fastdds::dds::PartitionQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos partition = database::Qos::array();
    for (auto p : qos)
    {
        partition.push_back(p.name());
    }
    serialized[fieldname] = partition;
}

template <>
void serialize<fastdds::dds::DurabilityServiceQosPolicy> (
        const fastdds::dds::DurabilityServiceQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos durability;
    durability[max_instances_tag] = qos.max_instances;
    durability[max_samples_tag] = qos.max_samples;
    durability[max_samples_per_instance_tag] = qos.max_samples_per_instance;
    durability[history_depth_tag] = qos.history_depth;
    switch (qos.history_kind)
    {
        case fastdds::dds::KEEP_LAST_HISTORY_QOS:
            durability[history_kind_tag] = history_keep_last_tag;
            break;
        case fastdds::dds::KEEP_ALL_HISTORY_QOS:
            durability[history_kind_tag] = history_keep_all_tag;
            break;
    }
    serialize(qos.service_cleanup_delay, service_cleanup_delay_tag, durability);
    serialized[fieldname] = durability;
}

template <>
void serialize<fastdds::dds::LifespanQosPolicy> (
        const fastdds::dds::LifespanQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos lifespan;
    serialize(qos.duration, duration_tag, lifespan);
    serialized[fieldname] = lifespan;
}

template <>
void serialize<fastdds::dds::DataRepresentationQosPolicy> (
        const fastdds::dds::DataRepresentationQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos representation = database::Qos::array();
    for (auto p : qos.m_value)
    {
        switch (p)
        {
            case fastdds::dds::XCDR_DATA_REPRESENTATION:
                representation.push_back(representation_xcdr_tag);
                break;
            case fastdds::dds::XML_DATA_REPRESENTATION:
                representation.push_back(representation_xml_tag);
                break;
            case fastdds::dds::XCDR2_DATA_REPRESENTATION:
                representation.push_back(representation_xcdr2_tag);
                break;
        }
    }
    serialized[fieldname] = representation;
}

template <>
void serialize<fastdds::dds::TypeConsistencyEnforcementQosPolicy> (
        const fastdds::dds::TypeConsistencyEnforcementQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos type_consistency;
    switch (qos.m_kind)
    {
        case fastdds::dds::DISALLOW_TYPE_COERCION:
            type_consistency[kind_tag] = type_consistency_coercion_disallow_tag;
            break;
        case fastdds::dds::ALLOW_TYPE_COERCION:
            type_consistency[kind_tag] = type_consistency_coercion_allow_tag;
            break;
    }
    serialize(qos.m_ignore_sequence_bounds, ignore_sequence_bounds_tag, type_consistency);
    serialize(qos.m_ignore_string_bounds, ignore_string_bounds_tag, type_consistency);
    serialize(qos.m_ignore_member_names, ignore_member_names_tag, type_consistency);
    serialize(qos.m_prevent_type_widening, prevent_type_widening_tag, type_consistency);
    serialize(qos.m_force_type_validation, force_type_validation_tag, type_consistency);
    serialized[fieldname] = type_consistency;
}

template <>
void serialize<fastdds::dds::DisablePositiveACKsQosPolicy> (
        const fastdds::dds::DisablePositiveACKsQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos disable_acks;
    serialize(qos.enabled, enabled_tag, disable_acks);
    serialize(qos.duration, duration_tag, disable_acks);
    serialized[fieldname] = disable_acks;
}

template <>
void serialize<fastdds::dds::DataSharingQosPolicy> (
        const fastdds::dds::DataSharingQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos datasharing;
    switch (qos.kind())
    {
        case fastdds::dds::AUTO:
            datasharing[kind_tag] = data_sharing_auto_tag;
            break;
        case fastdds::dds::ON:
            datasharing[kind_tag] = data_sharing_on_tag;
            break;
        case fastdds::dds::OFF:
            datasharing[kind_tag] = data_sharing_off_tag;
            break;
    }
    datasharing[max_domains_tag] = qos.max_domains();
    datasharing[shm_directory_tag] = qos.shm_directory();
    datasharing[domain_ids_tag] = database::Qos::array();
    for (auto id : qos.domain_ids())
    {
        datasharing[domain_ids_tag].push_back(id);
    }
    serialized[fieldname] = datasharing;
}

template <>
void serialize<fastdds::dds::OwnershipStrengthQosPolicy> (
        const fastdds::dds::OwnershipStrengthQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos strength;
    strength[value_tag] = qos.value;
    serialized[fieldname] = strength;
}

template <>
void serialize<fastdds::dds::PublishModeQosPolicy> (
        const fastdds::dds::PublishModeQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos publishmode;
    switch (qos.kind)
    {
        case fastdds::dds::SYNCHRONOUS_PUBLISH_MODE:
            publishmode[kind_tag] = publish_mode_sync_tag;
            break;
        case fastdds::dds::ASYNCHRONOUS_PUBLISH_MODE:
            publishmode[kind_tag] = publish_mode_async_tag;
            break;
    }
    serialized[fieldname] = publishmode;
}

template <>
void serialize<fastdds::dds::ParameterPropertyList_t> (
        const fastdds::dds::ParameterPropertyList_t& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos properties = database::Qos::array();
    for (auto p : qos)
    {
        database::Qos property;
        property[name_tag] = p.first();
        property[value_tag] = p.second();
        properties.push_back(property);
    }
    serialized[fieldname] = properties;
}

database::Qos reader_info_to_backend_qos(
        const fastrtps::rtps::ReaderDiscoveryInfo& reader_info)
{
    database::Qos reader;

    serialize(reader_info.info.m_qos.m_durability, durability_tag, reader);
    serialize(reader_info.info.m_qos.m_deadline, deadline_tag, reader);
    serialize(reader_info.info.m_qos.m_latencyBudget, latency_budget_tag, reader);
    serialize(reader_info.info.m_qos.m_liveliness, liveliness_tag, reader);
    serialize(reader_info.info.m_qos.m_reliability, reliability_tag, reader);
    serialize(reader_info.info.m_qos.m_ownership, ownership_tag, reader);
    serialize(reader_info.info.m_qos.m_destinationOrder, destination_order_tag, reader);
    serialize(reader_info.info.m_qos.m_userData, user_data_tag, reader);
    serialize(reader_info.info.m_qos.m_timeBasedFilter, time_based_filter_tag, reader);
    serialize(reader_info.info.m_qos.m_presentation, presentation_tag, reader);
    serialize(reader_info.info.m_qos.m_partition, partition_tag, reader);
    serialize(reader_info.info.m_qos.m_topicData, topic_data_tag, reader);
    serialize(reader_info.info.m_qos.m_groupData, group_data_tag, reader);
    serialize(reader_info.info.m_qos.m_durabilityService, durability_service_tag, reader);
    serialize(reader_info.info.m_qos.m_lifespan, lifespan_tag, reader);
    serialize(reader_info.info.m_qos.representation, representation_tag, reader);
    serialize(reader_info.info.m_qos.type_consistency, type_consistency_tag, reader);
    serialize(reader_info.info.m_qos.m_disablePositiveACKs, disable_positive_acks_tag, reader);
    serialize(reader_info.info.m_qos.data_sharing, data_sharing_tag, reader);

    return reader;
}

database::Qos writer_info_to_backend_qos(
        const fastrtps::rtps::WriterDiscoveryInfo& writer_info)
{
    database::Qos writer;

    serialize(writer_info.info.m_qos.m_durability, durability_tag, writer);
    serialize(writer_info.info.m_qos.m_durabilityService, durability_service_tag, writer);
    serialize(writer_info.info.m_qos.m_deadline, deadline_tag, writer);
    serialize(writer_info.info.m_qos.m_latencyBudget, latency_budget_tag, writer);
    serialize(writer_info.info.m_qos.m_liveliness, liveliness_tag, writer);
    serialize(writer_info.info.m_qos.m_reliability, reliability_tag, writer);
    serialize(writer_info.info.m_qos.m_lifespan, lifespan_tag, writer);
    serialize(writer_info.info.m_qos.m_userData, user_data_tag, writer);
    serialize(writer_info.info.m_qos.m_timeBasedFilter, time_based_filter_tag, writer);
    serialize(writer_info.info.m_qos.m_ownership, ownership_tag, writer);
    serialize(writer_info.info.m_qos.m_ownershipStrength, ownership_strength_tag, writer);
    serialize(writer_info.info.m_qos.m_destinationOrder, destination_order_tag, writer);
    serialize(writer_info.info.m_qos.m_presentation, presentation_tag, writer);
    serialize(writer_info.info.m_qos.m_partition, partition_tag, writer);
    serialize(writer_info.info.m_qos.m_topicData, topic_data_tag, writer);
    serialize(writer_info.info.m_qos.m_groupData, group_data_tag, writer);
    serialize(writer_info.info.m_qos.m_publishMode, publish_mode_tag, writer);
    serialize(writer_info.info.m_qos.representation, representation_tag, writer);
    serialize(writer_info.info.m_qos.m_disablePositiveACKs, disable_positive_acks_tag, writer);
    serialize(writer_info.info.m_qos.data_sharing, data_sharing_tag, writer);

    return writer;
}

database::Qos participant_info_to_backend_qos(
        const fastrtps::rtps::ParticipantDiscoveryInfo& participant_info)
{
    database::Qos participant;

    participant[available_builtin_endpoints_tag] = participant_info.info.m_availableBuiltinEndpoints;
    serialize(participant_info.info.m_leaseDuration, lease_duration_tag, participant);
    serialize(participant_info.info.m_properties, properties_tag, participant);
    serialize(participant_info.info.m_userData, user_data_tag, participant);
    participant[vendor_id_tag] = participant_info.info.m_VendorId;

    return participant;
}

} // namespace subscriber
} // namespace statistics_backend
} // namespace eprosima
