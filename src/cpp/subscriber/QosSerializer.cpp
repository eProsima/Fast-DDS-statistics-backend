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

#include <string>

#include "QosSerializer.hpp"
#include "QosSerializerTags.hpp"

#include <fastdds/dds/core/policy/QosPolicies.hpp>
#include <fastdds/utils/collections/ResourceLimitedContainerConfig.hpp>

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
void serialize<fastdds::dds::Duration_t> (
        const fastdds::dds::Duration_t& qos,
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
    for (const auto& v : qos.getValue())
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
    for (const auto& p : qos)
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
    for (const auto& p : qos.m_value)
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
    datasharing[domain_ids_tag] = database::Qos::array();
    for (const auto& id : qos.domain_ids())
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
            publishmode[flow_controller_tag] = qos.flow_controller_name;
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
    for (const auto& p : qos)
    {
        database::Qos property;
        property[name_tag] = p.first();
        property[value_tag] = p.second();
        properties.push_back(property);
    }
    serialized[fieldname] = properties;
}

template <>
void serialize<fastdds::dds::HistoryQosPolicy> (
        const fastdds::dds::HistoryQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos history;
    switch (qos.kind)
    {
        case fastdds::dds::KEEP_LAST_HISTORY_QOS:
            history[kind_tag] = history_keep_last_tag;
            history[history_depth_tag] = qos.depth;
            break;
        case fastdds::dds::KEEP_ALL_HISTORY_QOS:
            history[kind_tag] = history_keep_all_tag;
            break;
    }

    serialized[fieldname] = history;
}

template <>
void serialize<fastdds::dds::ResourceLimitsQosPolicy> (
        const fastdds::dds::ResourceLimitsQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos resource_limits;
    resource_limits[max_samples_tag] = qos.max_samples;
    resource_limits[max_instances_tag] = qos.max_instances;
    resource_limits[max_samples_per_instance_tag] = qos.max_samples_per_instance;
    resource_limits[allocated_samples_tag] = qos.allocated_samples;
    resource_limits[extra_samples_tag] = qos.extra_samples;

    serialized[fieldname] = resource_limits;
}

template <>
void serialize<fastdds::dds::ReaderDataLifecycleQosPolicy> (
        const fastdds::dds::ReaderDataLifecycleQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos reader_data_lifecycle;
    serialize(qos.autopurge_no_writer_samples_delay, autopurge_no_writer_samples_delay_tag, reader_data_lifecycle);
    serialize(qos.autopurge_disposed_samples_delay, autopurge_disposed_samples_delay_tag, reader_data_lifecycle);
    serialized[fieldname] = reader_data_lifecycle;
}

template <>
void serialize<fastdds::dds::RTPSReliableReaderQos> (
        const fastdds::dds::RTPSReliableReaderQos& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos reliable_reader;
    serialize(qos.times, reader_times_tag, reliable_reader);
    serialize(qos.disable_positive_acks, disable_positive_acks_tag, reliable_reader);
    serialized[fieldname] = reliable_reader;
}

template <>
void serialize<fastdds::dds::RTPSEndpointQos> (
        const fastdds::dds::RTPSEndpointQos& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos endpoint_qos;
    serialize(qos.unicast_locator_list, unicast_locator_list_tag, endpoint_qos);
    serialize(qos.multicast_locator_list, multicast_locator_list_tag, endpoint_qos);
    serialize(qos.remote_locator_list, remote_locators_tag, endpoint_qos);
    serialize(qos.external_unicast_locators, external_unicast_locators_tag, endpoint_qos);
    serialize(qos.ignore_non_matching_locators, ignore_non_matching_locators_tag, endpoint_qos);
    endpoint_qos[user_defined_id_tag] = qos.user_defined_id;
    endpoint_qos[entity_id_tag] = qos.entity_id;
    serialize(qos.history_memory_policy, history_memory_policy_tag, endpoint_qos);
    serialized[fieldname] = endpoint_qos;
}

template <>
void serialize<fastdds::dds::ReaderResourceLimitsQos> (
        const fastdds::dds::ReaderResourceLimitsQos& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos resource_limits;
    serialize(qos.matched_publisher_allocation, matched_publisher_allocation_tag, resource_limits);
    serialize(qos.sample_infos_allocation, sample_infos_allocation_tag, resource_limits);
    serialize(qos.outstanding_reads_allocation, outstanding_reads_allocation_tag, resource_limits);
    resource_limits[max_samples_per_read_tag] = qos.max_samples_per_read;
    serialized[fieldname] = resource_limits;
}

template <>
void serialize<fastdds::rtps::ReaderTimes> (
        const fastdds::rtps::ReaderTimes& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos times;
    serialize(qos.initial_acknack_delay, initial_acknack_delay_tag, times);
    serialize(qos.heartbeat_response_delay, heartbeat_response_delay_tag, times);
    serialized[fieldname] = times;
}

template <>
void serialize<fastdds::rtps::LocatorList> (
        const fastdds::rtps::LocatorList& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos locators = database::Qos::array();
    for (auto it = qos.begin(); it != qos.end(); ++it)
    {
        std::ostringstream locator_stream;
        locator_stream << *it;
        locators.push_back(locator_stream.str());
    }
    serialized[fieldname] = locators;
}

template <>
void serialize<fastdds::rtps::ExternalLocators> (
        const fastdds::rtps::ExternalLocators& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos external_locators = database::Qos::array();

    for (const auto& externality_it : qos)
    {
        database::Qos externality;
        externality[external_locators_externality_index_tag] = externality_it.first;
        database::Qos locators_by_externality = database::Qos::array();
        for (const auto& cost_it : externality_it.second)
        {
            database::Qos cost;
            cost[external_locators_cost_tag] = cost_it.first;
            database::Qos locators_by_cost = database::Qos::array();
            for (const auto& locator_it : cost_it.second)
            {
                std::ostringstream ss;
                ss << locator_it;
                locators_by_cost.push_back(ss.str());
            }
            cost[locators_with_mask_tag] = locators_by_cost;

            locators_by_externality.push_back(cost);
        }
        externality[locators_tag] = locators_by_externality;
        external_locators.push_back(externality);
    }

    serialized[fieldname] = external_locators;
}

template <>
void serialize<fastdds::rtps::MemoryManagementPolicy_t> (
        const fastdds::rtps::MemoryManagementPolicy_t& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    switch (qos)
    {
        case fastdds::rtps::MemoryManagementPolicy::PREALLOCATED_MEMORY_MODE:
            serialized[fieldname] = history_memory_policy_preallocated_tag;
            break;
        case fastdds::rtps::MemoryManagementPolicy::PREALLOCATED_WITH_REALLOC_MEMORY_MODE:
            serialized[fieldname] = history_memory_policy_preallocated_realloc_tag;
            break;
        case fastdds::rtps::MemoryManagementPolicy::DYNAMIC_RESERVE_MEMORY_MODE:
            serialized[fieldname] = history_memory_policy_dynamic_reserve_tag;
            break;
        case fastdds::rtps::MemoryManagementPolicy::DYNAMIC_REUSABLE_MEMORY_MODE:
            serialized[fieldname] = history_memory_policy_dynamic_reusable_tag;
            break;
    }
}

template <>
void serialize<fastdds::ResourceLimitedContainerConfig> (
        const fastdds::ResourceLimitedContainerConfig& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos resource_limited_config;
    resource_limited_config[resource_limited_container_initial_number_tag] = qos.initial;
    resource_limited_config[resource_limited_container_maximum_number_tag] = qos.maximum;
    resource_limited_config[resource_limited_container_increment_tag] = qos.increment;
    serialized[fieldname] = resource_limited_config;
}

template <>
void serialize<fastdds::dds::TransportPriorityQosPolicy> (
        const fastdds::dds::TransportPriorityQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    serialized[fieldname] = qos.value;
}

template <>
void serialize<fastdds::dds::WriterDataLifecycleQosPolicy> (
        const fastdds::dds::WriterDataLifecycleQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos writer_data_lifecycle;
    serialize(qos.autodispose_unregistered_instances, autodispose_unregistered_instances_tag, writer_data_lifecycle);
    serialized[fieldname] = writer_data_lifecycle;
}

template <>
void serialize<fastdds::dds::RTPSReliableWriterQos> (
        const fastdds::dds::RTPSReliableWriterQos& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos reliable_writer;
    serialize(qos.times, writer_times_tag, reliable_writer);
    serialize(qos.disable_positive_acks, disable_positive_acks_tag, reliable_writer);
    serialize(qos.disable_heartbeat_piggyback, disable_heartbeat_piggyback_tag, reliable_writer);
    serialized[fieldname] = reliable_writer;
}

template <>
void serialize<fastdds::dds::WriterResourceLimitsQos> (
        const fastdds::dds::WriterResourceLimitsQos& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos resource_limits;
    serialize(qos.matched_subscriber_allocation, matched_subscriber_allocation_tag, resource_limits);
    serialize(qos.reader_filters_allocation, reader_filters_allocation_tag, resource_limits);
    serialized[fieldname] = resource_limits;
}

template <>
void serialize<fastdds::rtps::WriterTimes> (
        const fastdds::rtps::WriterTimes& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos times;
    serialize(qos.initial_heartbeat_delay, initial_heartbeat_delay_tag, times);
    serialize(qos.heartbeat_period, heartbeat_period_tag, times);
    serialize(qos.nack_response_delay, nack_response_delay_tag, times);
    serialize(qos.nack_supression_duration, nack_supression_duration_tag, times);
    serialized[fieldname] = times;
}

template <>
void serialize<fastdds::dds::WireProtocolConfigQos> (
        const fastdds::dds::WireProtocolConfigQos& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos wire_protocol;
    serialize(qos.prefix, guid_prefix_tag, wire_protocol);
    wire_protocol[participant_id_tag] = qos.participant_id;
    serialize(qos.builtin, builtin_attributes_tag, wire_protocol);
    serialize(qos.port, port_parameters_tag, wire_protocol);
    serialize(qos.default_unicast_locator_list, default_unicast_locator_list_tag, wire_protocol);
    serialize(qos.default_multicast_locator_list, default_multicast_locator_list_tag, wire_protocol);
    serialize(qos.default_external_unicast_locators, default_external_unicast_locators_tag, wire_protocol);
    serialize(qos.ignore_non_matching_locators, ignore_non_matching_locators_tag, wire_protocol);
    wire_protocol[easy_mode_ip_tag] = qos.easy_mode();
    serialized[fieldname] = wire_protocol;
}

template <>
void serialize<fastdds::rtps::GuidPrefix_t> (
        const fastdds::rtps::GuidPrefix_t& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    std::ostringstream ss;
    ss << qos;
    serialized[fieldname] = ss.str();
}

template <>
void serialize<fastdds::rtps::BuiltinAttributes> (
        const fastdds::rtps::BuiltinAttributes& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos builtin_attributes;
    serialize(qos.discovery_config, discovery_config_tag, builtin_attributes);
    serialize(qos.use_WriterLivelinessProtocol, use_writer_liveliness_protocol_tag, builtin_attributes);
    builtin_attributes[network_configuration_tag] = qos.network_configuration;
    serialize(qos.metatrafficUnicastLocatorList, metatraffic_unicast_locator_list_tag, builtin_attributes);
    serialize(qos.metatrafficMulticastLocatorList, metatraffic_multicast_locator_list_tag, builtin_attributes);
    serialize(qos.metatraffic_external_unicast_locators, metatraffic_external_unicast_locators_tag, builtin_attributes);
    serialize(qos.initialPeersList, initial_peers_tag, builtin_attributes);
    serialize(qos.readerHistoryMemoryPolicy, reader_history_memory_policy_tag, builtin_attributes);
    builtin_attributes[reader_payload_size_tag] = qos.readerPayloadSize;
    serialize(qos.writerHistoryMemoryPolicy, writer_history_memory_policy_tag, builtin_attributes);
    builtin_attributes[writer_payload_size_tag] = qos.writerPayloadSize;
    builtin_attributes[mutation_tries_tag] = qos.mutation_tries;
    serialize(qos.avoid_builtin_multicast, avoid_builtin_multicast_tag, builtin_attributes);
    builtin_attributes[flow_controller_tag] = qos.flow_controller_name;
    serialized[fieldname] = builtin_attributes;
}

template <>
void serialize<fastdds::rtps::DiscoverySettings> (
        const fastdds::rtps::DiscoverySettings& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos discovery_settings;
    serialize(qos.discoveryProtocol, discovery_protocol_tag, discovery_settings);
    serialize(qos.use_SIMPLE_EndpointDiscoveryProtocol, use_simple_edp_tag, discovery_settings);
    serialize(qos.use_STATIC_EndpointDiscoveryProtocol, use_static_edp_tag, discovery_settings);
    serialize(qos.leaseDuration, lease_duration_tag, discovery_settings);
    serialize(qos.leaseDuration_announcementperiod, announcement_period_tag, discovery_settings);
    serialize(qos.initial_announcements, initial_announcements_tag, discovery_settings);
    serialize(qos.m_simpleEDP, simple_edp_attributes_tag, discovery_settings);
    serialize(qos.discoveryServer_client_syncperiod, discovery_server_client_syncperiod_tag, discovery_settings);
    serialize(qos.m_DiscoveryServers, discovery_servers_tag, discovery_settings);
    serialize(qos.ignoreParticipantFlags, participant_filtering_tag, discovery_settings);
    discovery_settings[static_edp_xml_config_tag] = std::string(qos.static_edp_xml_config());
    serialized[fieldname] = discovery_settings;
}

template <>
void serialize<fastdds::rtps::DiscoveryProtocol> (
        const fastdds::rtps::DiscoveryProtocol& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    switch (qos)
    {
        case fastdds::rtps::DiscoveryProtocol::NONE:
            serialized[fieldname] = discovery_protocol_none_tag;
            break;
        case fastdds::rtps::DiscoveryProtocol::SIMPLE:
            serialized[fieldname] = discovery_protocol_simple_tag;
            break;
        case fastdds::rtps::DiscoveryProtocol::EXTERNAL:
            serialized[fieldname] = discovery_protocol_external_tag;
            break;
        case fastdds::rtps::DiscoveryProtocol::CLIENT:
            serialized[fieldname] = discovery_protocol_client_tag;
            break;
        case fastdds::rtps::DiscoveryProtocol::SERVER:
            serialized[fieldname] = discovery_protocol_server_tag;
            break;
        case fastdds::rtps::DiscoveryProtocol::BACKUP:
            serialized[fieldname] = discovery_protocol_backup_tag;
            break;
        case fastdds::rtps::DiscoveryProtocol::SUPER_CLIENT:
            serialized[fieldname] = discovery_protocol_super_client_tag;
            break;
    }
}

template <>
void serialize<fastdds::rtps::InitialAnnouncementConfig> (
        const fastdds::rtps::InitialAnnouncementConfig& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos initial_announcements;
    initial_announcements[initial_announcements_count_tag] = qos.count;
    serialize(qos.period, initial_announcements_period_tag, initial_announcements);
    serialized[fieldname] = initial_announcements;
}

template <>
void serialize<fastdds::rtps::SimpleEDPAttributes> (
        const fastdds::rtps::SimpleEDPAttributes& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos simple_edp;
    serialize(qos.use_PublicationWriterANDSubscriptionReader, simple_edp_use_pub_writer_sub_reader_tag, simple_edp);
    serialize(qos.use_PublicationReaderANDSubscriptionWriter, simple_edp_use_pub_reader_sub_writer_tag, simple_edp);
#if HAVE_SECURITY
    serialize(qos.enable_builtin_secure_publications_writer_and_subscriptions_reader,
            simple_edp_enable_builtin_secure_pub_writer_sub_reader_tag, simple_edp);
    serialize(qos.enable_builtin_secure_subscriptions_writer_and_publications_reader,
            simple_edp_enable_builtin_secure_sub_writer_pub_reader_tag, simple_edp);
#endif // if HAVE_SECURITY
    serialized[fieldname] = simple_edp;
}

template <>
void serialize<fastdds::rtps::ParticipantFilteringFlags> (
        const fastdds::rtps::ParticipantFilteringFlags& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    switch (qos)
    {
        case fastdds::rtps::ParticipantFilteringFlags::NO_FILTER:
            serialized[fieldname] = participant_filtering_no_filter_tag;
            break;
        case fastdds::rtps::ParticipantFilteringFlags::FILTER_DIFFERENT_HOST:
            serialized[fieldname] = participant_filtering_filter_different_host_tag;
            break;
        case fastdds::rtps::ParticipantFilteringFlags::FILTER_DIFFERENT_PROCESS:
            serialized[fieldname] = participant_filtering_filter_different_process_tag;
            break;
        case fastdds::rtps::ParticipantFilteringFlags::FILTER_SAME_PROCESS:
            serialized[fieldname] = participant_filtering_filter_same_process_tag;
            break;
    }
}

template <>
void serialize<fastdds::rtps::PortParameters> (
        const fastdds::rtps::PortParameters& qos,
        const std::string& fieldname,
        database::Qos& serialized)
{
    database::Qos port_parameters;
    port_parameters[port_parameters_port_base_tag] = qos.portBase;
    port_parameters[port_parameters_domain_id_gain_tag] = qos.domainIDGain;
    port_parameters[port_parameters_participant_id_gain_tag] = qos.participantIDGain;
    port_parameters[port_parameters_offsetd0_tag] = qos.offsetd0;
    port_parameters[port_parameters_offsetd1_tag] = qos.offsetd1;
    port_parameters[port_parameters_offsetd2_tag] = qos.offsetd2;
    port_parameters[port_parameters_offsetd3_tag] = qos.offsetd3;
    port_parameters[port_parameters_offsetd4_tag] = qos.offsetd4;
    serialized[fieldname] = port_parameters;
}

database::Qos reader_proxy_data_to_backend_qos(
        const fastdds::rtps::SubscriptionBuiltinTopicData& reader_data)
{
    database::Qos reader;

    serialize(reader_data.durability, durability_tag, reader);
    serialize(reader_data.deadline, deadline_tag, reader);
    serialize(reader_data.latency_budget, latency_budget_tag, reader);
    serialize(reader_data.lifespan, lifespan_tag, reader);
    serialize(reader_data.liveliness, liveliness_tag, reader);
    serialize(reader_data.reliability, reliability_tag, reader);
    serialize(reader_data.ownership, ownership_tag, reader);
    serialize(reader_data.destination_order, destination_order_tag, reader);
    serialize(reader_data.user_data, user_data_tag, reader);
    serialize(reader_data.time_based_filter, time_based_filter_tag, reader);
    serialize(reader_data.presentation, presentation_tag, reader);
    serialize(reader_data.partition, partition_tag, reader);
    serialize(reader_data.topic_data, topic_data_tag, reader);
    serialize(reader_data.group_data, group_data_tag, reader);
    serialize(reader_data.representation, representation_tag, reader);
    serialize(reader_data.type_consistency, type_consistency_tag, reader);
    serialize(reader_data.disable_positive_acks, disable_positive_acks_tag, reader);
    serialize(reader_data.data_sharing, data_sharing_tag, reader);
    fastdds::dds::HistoryQosPolicy history =
            reader_data.history.has_value() ? reader_data.history.value() : fastdds::dds::HistoryQosPolicy();
    serialize(history, history_tag, reader);

    return reader;
}

database::Qos writer_proxy_data_to_backend_qos(
        const fastdds::rtps::PublicationBuiltinTopicData& info)
{
    database::Qos writer;

    serialize(info.durability, durability_tag, writer);
    serialize(info.durability_service, durability_service_tag, writer);
    serialize(info.deadline, deadline_tag, writer);
    serialize(info.latency_budget, latency_budget_tag, writer);
    serialize(info.liveliness, liveliness_tag, writer);
    serialize(info.reliability, reliability_tag, writer);
    serialize(info.lifespan, lifespan_tag, writer);
    serialize(info.user_data, user_data_tag, writer);
    serialize(info.ownership, ownership_tag, writer);
    serialize(info.ownership_strength, ownership_strength_tag, writer);
    serialize(info.destination_order, destination_order_tag, writer);
    serialize(info.presentation, presentation_tag, writer);
    serialize(info.partition, partition_tag, writer);
    serialize(info.topic_data, topic_data_tag, writer);
    serialize(info.group_data, group_data_tag, writer);
    serialize(info.representation, representation_tag, writer);
    serialize(info.disable_positive_acks, disable_positive_acks_tag, writer);
    serialize(info.data_sharing, data_sharing_tag, writer);
    fastdds::dds::HistoryQosPolicy history =
            info.history.has_value() ? info.history.value() : fastdds::dds::HistoryQosPolicy();
    serialize(history, history_tag, writer);

    return writer;
}

database::Qos participant_proxy_data_to_backend_qos(
        const fastdds::rtps::ParticipantBuiltinTopicData& participant_data)
{
    database::Qos participant;

    serialize(participant_data.lease_duration, lease_duration_tag, participant);
    serialize(participant_data.properties, properties_tag, participant);
    serialize(participant_data.user_data, user_data_tag, participant);
    participant[vendor_id_tag] = participant_data.vendor_id;

    return participant;
}

database::Qos optional_qos_to_backend_qos(
        const fastdds::rtps::SubscriptionBuiltinTopicData& reader_data)
{
    database::Qos reader_qos;

    fastdds::dds::ResourceLimitsQosPolicy resource_limits =
            reader_data.resource_limits.has_value() ? reader_data.resource_limits.value() : fastdds::dds::
                    ResourceLimitsQosPolicy();
    fastdds::dds::ReaderDataLifecycleQosPolicy reader_data_lifecycle =
            reader_data.reader_data_lifecycle.has_value() ? reader_data.reader_data_lifecycle.value() : fastdds::dds::
                    ReaderDataLifecycleQosPolicy();
    fastdds::dds::RTPSReliableReaderQos rtps_reliable_reader =
            reader_data.rtps_reliable_reader.has_value() ? reader_data.rtps_reliable_reader.value() : fastdds::dds::
                    RTPSReliableReaderQos();
    fastdds::dds::RTPSEndpointQos rtps_endpoint =
            reader_data.endpoint.has_value() ? reader_data.endpoint.value() : fastdds::dds::RTPSEndpointQos();
    fastdds::dds::ReaderResourceLimitsQos reader_resource_limits =
            reader_data.reader_resource_limits.has_value() ? reader_data.reader_resource_limits.value() : fastdds::dds::
                    ReaderResourceLimitsQos();

    serialize(resource_limits, resource_limits_tag, reader_qos);
    serialize(reader_data_lifecycle, reader_data_lifecycle_tag, reader_qos);
    serialize(rtps_reliable_reader, rtps_reliable_reader_tag, reader_qos);
    serialize(rtps_endpoint, rtps_endpoint_tag, reader_qos);
    serialize(reader_resource_limits, reader_resource_limits_tag, reader_qos);

    return reader_qos;
}

database::Qos optional_qos_to_backend_qos(
        const fastdds::rtps::PublicationBuiltinTopicData& writer_data)
{
    database::Qos writer_qos;

    fastdds::dds::ResourceLimitsQosPolicy resource_limits =
            writer_data.resource_limits.has_value() ? writer_data.resource_limits.value() : fastdds::dds::
                    ResourceLimitsQosPolicy();
    fastdds::dds::TransportPriorityQosPolicy transport_priority =
            writer_data.transport_priority.has_value() ? writer_data.transport_priority.value() : fastdds::dds::
                    TransportPriorityQosPolicy();
    fastdds::dds::WriterDataLifecycleQosPolicy writer_data_lifecycle =
            writer_data.writer_data_lifecycle.has_value() ? writer_data.writer_data_lifecycle.value() : fastdds::dds::
                    WriterDataLifecycleQosPolicy();
    fastdds::dds::PublishModeQosPolicy publish_mode =
            writer_data.publish_mode.has_value() ? writer_data.publish_mode.value() : fastdds::dds::PublishModeQosPolicy();
    fastdds::dds::RTPSReliableWriterQos rtps_reliable_writer =
            writer_data.rtps_reliable_writer.has_value() ? writer_data.rtps_reliable_writer.value() : fastdds::dds::
                    RTPSReliableWriterQos();
    fastdds::dds::RTPSEndpointQos rtps_endpoint =
            writer_data.endpoint.has_value() ? writer_data.endpoint.value() : fastdds::dds::RTPSEndpointQos();
    fastdds::dds::WriterResourceLimitsQos writer_resource_limits =
            writer_data.writer_resource_limits.has_value() ? writer_data.writer_resource_limits.value() : fastdds::dds::
                    WriterResourceLimitsQos();

    serialize(resource_limits, resource_limits_tag, writer_qos);
    serialize(transport_priority, transport_priority_tag, writer_qos);
    serialize(writer_data_lifecycle, writer_data_lifecycle_tag, writer_qos);
    serialize(publish_mode, publish_mode_tag, writer_qos);
    serialize(rtps_reliable_writer, rtps_reliable_writer_tag, writer_qos);
    serialize(rtps_endpoint, rtps_endpoint_tag, writer_qos);
    serialize(writer_resource_limits, writer_resource_limits_tag, writer_qos);

    return writer_qos;
}

database::Qos optional_qos_to_backend_qos(
        const fastdds::rtps::ParticipantBuiltinTopicData& participant_data)
{
    database::Qos participant_qos;

    fastdds::dds::WireProtocolConfigQos wire_protocol_config =
            participant_data.wire_protocol.has_value() ? participant_data.wire_protocol.value() : fastdds::dds::
                    WireProtocolConfigQos();

    serialize(wire_protocol_config, wire_protocol_config_tag, participant_qos);

    return participant_qos;
}

} // namespace subscriber
} // namespace statistics_backend
} // namespace eprosima
