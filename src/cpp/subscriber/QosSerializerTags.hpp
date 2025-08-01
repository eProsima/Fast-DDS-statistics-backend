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
 * @file QosSerializerTags.hpp
 */

#ifndef FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__QUOS_SERIALIZER_TAGS_HPP
#define FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__QUOS_SERIALIZER_TAGS_HPP

namespace eprosima {
namespace statistics_backend {
namespace subscriber {

// Some generic tags
static constexpr const char* enabled_tag = "enabled";
static constexpr const char* kind_tag = "kind";
static constexpr const char* name_tag = "name";
static constexpr const char* value_tag = "value";

// For boolean serialization
static constexpr const char* false_tag = "false";
static constexpr const char* true_tag = "true";

// Primitive types tags
static constexpr const char* group_data_tag = "group_data";
static constexpr const char* topic_data_tag = "topic_data";
static constexpr const char* user_data_tag = "user_data";
static constexpr const char* partition_tag = "partition";
static constexpr const char* properties_tag = "properties";
static constexpr const char* vendor_id_tag = "vendor_id";
static constexpr const char* user_defined_id_tag = "user_defined_id";
static constexpr const char* entity_id_tag = "entity_id";
static constexpr const char* participant_id_tag = "participant_id";

static constexpr const char* available_builtin_endpoints_tag = "available_builtin_endpoints";

// Locators tags
static constexpr const char* locators_tag = "locators";
static constexpr const char* locators_with_mask_tag = "locators_with_mask";
static constexpr const char* unicast_locator_list_tag = "unicast_locators";
static constexpr const char* metatraffic_unicast_locator_list_tag = "metatraffic_unicast_locators";
static constexpr const char* default_unicast_locator_list_tag = "default_unicast_locators";
static constexpr const char* multicast_locator_list_tag = "multicast_locators";
static constexpr const char* metatraffic_multicast_locator_list_tag = "metatraffic_multicast_locators";
static constexpr const char* default_multicast_locator_list_tag = "default_multicast_locators";
static constexpr const char* remote_locators_tag = "remote_locators";
static constexpr const char* external_unicast_locators_tag = "external_unicast_locators";
static constexpr const char* metatraffic_external_unicast_locators_tag = "metatraffic_external_unicast_locators";
static constexpr const char* default_external_unicast_locators_tag = "default_external_unicast_locators";
static constexpr const char* external_locators_externality_index_tag = "externality_index";
static constexpr const char* external_locators_cost_tag = "cost";
static constexpr const char* ignore_non_matching_locators_tag = "ignore_non_matching_locators";

// QoS-specific tags
#include "QosSerializer/DataSharingTags.ipp"
#include "QosSerializer/DestinationOrderTags.ipp"
#include "QosSerializer/DurabilityTags.ipp"
#include "QosSerializer/DurationTags.ipp"
#include "QosSerializer/LifecycleTags.ipp"
#include "QosSerializer/LivelinessTags.ipp"
#include "QosSerializer/OwnershipTags.ipp"
#include "QosSerializer/PresentationTags.ipp"
#include "QosSerializer/PublishModeTags.ipp"
#include "QosSerializer/ReliabilityTags.ipp"
#include "QosSerializer/ResourceTags.ipp"
#include "QosSerializer/RTPSEndpointTags.ipp"
#include "QosSerializer/TransportTags.ipp"
#include "QosSerializer/WireProtocolConfigTags.ipp"
#include "QosSerializer/XTypesTags.ipp"

} // namespace subscriber
} // namespace statistics_backend
} // namespace eprosima

#endif //FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__QUOS_SERIALIZER_TAGS_HPP
