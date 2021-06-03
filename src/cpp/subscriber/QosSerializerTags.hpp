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

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_SUBSCRIBER_QOSSERIALIZERTAGS_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_SUBSCRIBER_QOSSERIALIZERTAGS_HPP_

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

#include "QosSerializer/DurabilityTags.ipp"
#include "QosSerializer/DurationTags.ipp"
#include "QosSerializer/LivelinessTags.ipp"
#include "QosSerializer/ReliabilityTags.ipp"

} // namespace subscriber
} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_SUBSCRIBER_QOSSERIALIZERTAGS_HPP_
