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
            kind["kind"] = "VOLATILE_DURABILITY_QOS";
            break;
        case fastdds::dds::TRANSIENT_LOCAL_DURABILITY_QOS:
            kind["kind"] = "TRANSIENT_LOCAL_DURABILITY_QOS";
            break;
        case fastdds::dds::TRANSIENT_DURABILITY_QOS:
            kind["kind"] = "TRANSIENT_DURABILITY_QOS";
            break;
        case fastdds::dds::PERSISTENT_DURABILITY_QOS:
            kind["kind"] = "PERSISTENT_DURABILITY_QOS";
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
    duration["seconds"] = qos.seconds;
    duration["nanoseconds"] = qos.nanosec;
    serialized[fieldname] = duration;
}

template <>
void serialize<fastdds::dds::DeadlineQosPolicy> (
            const fastdds::dds::DeadlineQosPolicy& qos,
            const std::string& fieldname,
            database::Qos& serialized)
{
    database::Qos deadline;
    serialize(qos.period, "period", deadline);
    serialized[fieldname] = deadline;
}

template <>
void serialize<fastdds::dds::LatencyBudgetQosPolicy> (
            const fastdds::dds::LatencyBudgetQosPolicy& qos,
            const std::string& fieldname,
            database::Qos& serialized)
{
    database::Qos latency;
    serialize(qos.duration, "duration", latency);
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
            liveliness["kind"] = "AUTOMATIC_LIVELINESS_QOS";
            break;
        case fastdds::dds::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
            liveliness["kind"] = "MANUAL_BY_PARTICIPANT_LIVELINESS_QOS";
            break;
        case fastdds::dds::MANUAL_BY_TOPIC_LIVELINESS_QOS:
            liveliness["kind"] = "MANUAL_BY_TOPIC_LIVELINESS_QOS";
            break;
    }
    serialize(qos.announcement_period, "announcement_period", liveliness);
    serialize(qos.lease_duration, "lease_duration", liveliness);
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
            reliability["kind"] = "BEST_EFFORT_RELIABILITY_QOS";
            break;
        case fastdds::dds::RELIABLE_RELIABILITY_QOS:
            reliability["kind"] = "RELIABLE_RELIABILITY_QOS";
            break;
    }
    serialize(qos.max_blocking_time, "max_blocking_time", reliability);
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
            ownership["kind"] = "SHARED_OWNERSHIP_QOS";
            break;
        case fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS:
            ownership["kind"] = "EXCLUSIVE_OWNERSHIP_QOS";
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
        case fastdds::dds::SHARED_OWNERSHIP_QOS:
            destination_order["kind"] = "SHARED_OWNERSHIP_QOS";
            break;
        case fastdds::dds::EXCLUSIVE_OWNERSHIP_QOS:
            destination_order["kind"] = "EXCLUSIVE_OWNERSHIP_QOS";
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
    serialize(qos.minimum_separation, "minimum_separation", filter);
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
            serialized[fieldname] = "true";
    }
    else
    {
            serialized[fieldname] = "false";
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
            presentation["access_scope"] = "INSTANCE_PRESENTATION_QOS";
            break;
        case fastdds::dds::TOPIC_PRESENTATION_QOS:
            presentation["access_scope"] = "TOPIC_PRESENTATION_QOS";
            break;
        case fastdds::dds::GROUP_PRESENTATION_QOS:
            presentation["access_scope"] = "GROUP_PRESENTATION_QOS";
            break;
    }

    serialize(qos.coherent_access, "coherent_access", presentation);
    serialize(qos.ordered_access, "ordered_access", presentation);
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
    durability["history_depth"] = qos.history_depth;
    durability["max_instances"] = qos.max_instances;
    durability["max_samples"] = qos.max_samples;
    durability["max_samples_per_instance"] = qos.max_samples_per_instance;
    durability["history_depth"] = qos.history_depth;
    switch (qos.history_kind)
    {
        case fastdds::dds::KEEP_LAST_HISTORY_QOS:
            durability["history_kind"] = "KEEP_LAST_HISTORY_QOS";
            break;
        case fastdds::dds::KEEP_ALL_HISTORY_QOS:
            durability["history_kind"] = "KEEP_ALL_HISTORY_QOS";
            break;
    }
    serialize(qos.service_cleanup_delay, "service_cleanup_delay", durability);
    serialized[fieldname] = durability;
}

template <>
void serialize<fastdds::dds::LifespanQosPolicy> (
            const fastdds::dds::LifespanQosPolicy& qos,
            const std::string& fieldname,
            database::Qos& serialized)
{
    database::Qos lifespan;
    serialize(qos.duration, "duration", lifespan);
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
                representation.push_back("XCDR_DATA_REPRESENTATION");
                break;
            case fastdds::dds::XML_DATA_REPRESENTATION:
                representation.push_back("XML_DATA_REPRESENTATION");
                break;
            case fastdds::dds::XCDR2_DATA_REPRESENTATION:
                representation.push_back("XCDR2_DATA_REPRESENTATION");
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
            type_consistency["kind"] = "DISALLOW_TYPE_COERCION";
            break;
        case fastdds::dds::ALLOW_TYPE_COERCION:
            type_consistency["kind"] = "ALLOW_TYPE_COERCION";
            break;
    }
    serialize(qos.m_ignore_sequence_bounds, "ignore_sequence_bounds", type_consistency);
    serialize(qos.m_ignore_string_bounds, "ignore_string_bounds", type_consistency);
    serialize(qos.m_ignore_member_names, "ignore_member_names", type_consistency);
    serialize(qos.m_prevent_type_widening, "prevent_type_widening", type_consistency);
    serialize(qos.m_force_type_validation, "force_type_validation", type_consistency);
    serialized[fieldname] = type_consistency;
}

template <>
void serialize<fastdds::dds::DisablePositiveACKsQosPolicy> (
            const fastdds::dds::DisablePositiveACKsQosPolicy& qos,
            const std::string& fieldname,
            database::Qos& serialized)
{
    database::Qos disable_acks;
    serialize(qos.enabled, "enabled", disable_acks);
    serialize(qos.duration, "duration", disable_acks);
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
            datasharing["kind"] = "AUTO";
            break;
        case fastdds::dds::ON:
            datasharing["kind"] = "ON";
            break;
        case fastdds::dds::OFF:
            datasharing["kind"] = "OFF";
            break;
    }
    datasharing["max_domains"] = qos.max_domains();
    datasharing["shm_directory"] = qos.shm_directory();
    datasharing["domain_ids"] = database::Qos::array();
    for (auto id : qos.domain_ids())
    {
        datasharing["domain_ids"].push_back(id);
    }
    serialized[fieldname] = datasharing;
}

database::Qos reader_qos_to_backend_qos(
            const fastdds::dds::ReaderQos& reader_qos)
{
    database::Qos reader;

    serialize(reader_qos.m_durability, "durability", reader);
    serialize(reader_qos.m_deadline, "deadline", reader);
    serialize(reader_qos.m_latencyBudget, "latency_budget", reader);
    serialize(reader_qos.m_liveliness, "liveliness", reader);
    serialize(reader_qos.m_reliability, "reliablility", reader);
    serialize(reader_qos.m_ownership, "ownership", reader);
    serialize(reader_qos.m_destinationOrder, "ownership", reader);
    serialize(reader_qos.m_userData, "user_data", reader);
    serialize(reader_qos.m_timeBasedFilter, "time_based_filter", reader);
    serialize(reader_qos.m_presentation, "presentation", reader);
    serialize(reader_qos.m_partition, "partition", reader);
    serialize(reader_qos.m_topicData, "topic_data", reader);
    serialize(reader_qos.m_groupData, "group_data", reader);
    serialize(reader_qos.m_durabilityService, "durability_service", reader);
    serialize(reader_qos.m_lifespan, "lifespan", reader);
    serialize(reader_qos.representation, "representation", reader);
    serialize(reader_qos.type_consistency, "type_consistency", reader);
    serialize(reader_qos.m_disablePositiveACKs, "disable_positive_acks", reader);
    serialize(reader_qos.data_sharing, "data_sharing", reader);

    return reader;
}




} // namespace subscriber
} // namespace statistics_backend
} // namespace eprosima
