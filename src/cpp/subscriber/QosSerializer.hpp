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
 * @file QosSerializer.hpp
 */

#ifndef FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__QUOS_SERIALIZER_HPP
#define FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__QUOS_SERIALIZER_HPP

#include <string>

#include <fastdds/dds/core/Time_t.hpp>
#include <fastdds/rtps/builtin/data/ParticipantBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/utils/collections/ResourceLimitedContainerConfig.hpp>

#include <database/entities.hpp>

namespace eprosima {
namespace statistics_backend {
namespace subscriber {

template <typename T>
void serialize(
        const T& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::DurabilityQosPolicy> (
        const fastdds::dds::DurabilityQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::Duration_t> (
        const fastdds::dds::Duration_t& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::DeadlineQosPolicy> (
        const fastdds::dds::DeadlineQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::LatencyBudgetQosPolicy> (
        const fastdds::dds::LatencyBudgetQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::LivelinessQosPolicy> (
        const fastdds::dds::LivelinessQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::ReliabilityQosPolicy> (
        const fastdds::dds::ReliabilityQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::OwnershipQosPolicy> (
        const fastdds::dds::OwnershipQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::DestinationOrderQosPolicy> (
        const fastdds::dds::DestinationOrderQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::TopicDataQosPolicy> (
        const fastdds::dds::TopicDataQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::UserDataQosPolicy> (
        const fastdds::dds::UserDataQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::GroupDataQosPolicy> (
        const fastdds::dds::GroupDataQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::TimeBasedFilterQosPolicy> (
        const fastdds::dds::TimeBasedFilterQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<bool> (
        const bool& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::PresentationQosPolicy> (
        const fastdds::dds::PresentationQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::PartitionQosPolicy> (
        const fastdds::dds::PartitionQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::DurabilityServiceQosPolicy> (
        const fastdds::dds::DurabilityServiceQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::LifespanQosPolicy> (
        const fastdds::dds::LifespanQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::DataRepresentationQosPolicy> (
        const fastdds::dds::DataRepresentationQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::TypeConsistencyEnforcementQosPolicy> (
        const fastdds::dds::TypeConsistencyEnforcementQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::DisablePositiveACKsQosPolicy> (
        const fastdds::dds::DisablePositiveACKsQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::DataSharingQosPolicy> (
        const fastdds::dds::DataSharingQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::HistoryQosPolicy> (
        const fastdds::dds::HistoryQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::ResourceLimitsQosPolicy> (
        const fastdds::dds::ResourceLimitsQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::ReaderDataLifecycleQosPolicy> (
        const fastdds::dds::ReaderDataLifecycleQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::RTPSReliableReaderQos> (
        const fastdds::dds::RTPSReliableReaderQos& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::RTPSEndpointQos> (
        const fastdds::dds::RTPSEndpointQos& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::ReaderResourceLimitsQos> (
        const fastdds::dds::ReaderResourceLimitsQos& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::rtps::ReaderTimes> (
        const fastdds::rtps::ReaderTimes& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::rtps::LocatorList> (
        const fastdds::rtps::LocatorList& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::rtps::ExternalLocators> (
        const fastdds::rtps::ExternalLocators& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::rtps::MemoryManagementPolicy_t> (
        const fastdds::rtps::MemoryManagementPolicy_t& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::ResourceLimitedContainerConfig> (
        const fastdds::ResourceLimitedContainerConfig& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::TransportPriorityQosPolicy> (
        const fastdds::dds::TransportPriorityQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::WriterDataLifecycleQosPolicy> (
        const fastdds::dds::WriterDataLifecycleQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::PublishModeQosPolicy> (
        const fastdds::dds::PublishModeQosPolicy& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::RTPSReliableWriterQos> (
        const fastdds::dds::RTPSReliableWriterQos& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::WriterResourceLimitsQos> (
        const fastdds::dds::WriterResourceLimitsQos& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::rtps::WriterTimes> (
        const fastdds::rtps::WriterTimes& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::WireProtocolConfigQos> (
        const fastdds::dds::WireProtocolConfigQos& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::rtps::GuidPrefix_t> (
        const fastdds::rtps::GuidPrefix_t& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::rtps::BuiltinAttributes> (
        const fastdds::rtps::BuiltinAttributes& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::rtps::DiscoverySettings> (
        const fastdds::rtps::DiscoverySettings& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::rtps::DiscoveryProtocol> (
        const fastdds::rtps::DiscoveryProtocol& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::rtps::InitialAnnouncementConfig> (
        const fastdds::rtps::InitialAnnouncementConfig& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::rtps::SimpleEDPAttributes> (
        const fastdds::rtps::SimpleEDPAttributes& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::rtps::ParticipantFilteringFlags> (
        const fastdds::rtps::ParticipantFilteringFlags& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::rtps::PortParameters> (
        const fastdds::rtps::PortParameters& qos,
        const std::string& fieldname,
        database::Qos& serialized);

database::Qos reader_proxy_data_to_backend_qos(
        const fastdds::rtps::SubscriptionBuiltinTopicData& reader_data);

database::Qos writer_proxy_data_to_backend_qos(
        const fastdds::rtps::PublicationBuiltinTopicData& info);

database::Qos participant_proxy_data_to_backend_qos(
        const fastdds::rtps::ParticipantBuiltinTopicData& participant_data);

database::Qos optional_qos_to_backend_qos(
        const fastdds::rtps::SubscriptionBuiltinTopicData& reader_data);

database::Qos optional_qos_to_backend_qos(
        const fastdds::rtps::PublicationBuiltinTopicData& writer_data);

database::Qos optional_qos_to_backend_qos(
        const fastdds::rtps::ParticipantBuiltinTopicData& participant_data);

} // namespace subscriber
} // namespace statistics_backend
} // namespace eprosima

#endif // FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__QUOS_SERIALIZER_HPP
