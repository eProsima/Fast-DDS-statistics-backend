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

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_SUBSCRIBER_QOSSERIALIZER_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_SUBSCRIBER_QOSSERIALIZER_HPP_

#include <string>

#include <fastdds/dds/subscriber/qos/ReaderQos.hpp>
#include <fastdds/dds/publisher/qos/WriterQos.hpp>
#include <database/entities.hpp>

namespace eprosima {
namespace statistics_backend {
namespace subscriber {

template <typename T>
static void serialize(
        const T& qos,
        const std::string& fieldname,
        database::Qos& serialized);

template <>
void serialize<fastdds::dds::DurabilityQosPolicy> (
            const fastdds::dds::DurabilityQosPolicy& qos,
            const std::string& fieldname,
            database::Qos& serialized);

template <>
void serialize<fastrtps::Duration_t> (
            const fastrtps::Duration_t& qos,
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

database::Qos reader_qos_to_backend_qos(
            const fastdds::dds::ReaderQos& reader_qos);

database::Qos writer_qos_to_backend_qos(
            const fastdds::dds::WriterQos& writer_qos);


} // namespace subscriber
} // namespace statistics_backend
} // namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_SUBSCRIBER_QOSSERIALIZER_HPP_
