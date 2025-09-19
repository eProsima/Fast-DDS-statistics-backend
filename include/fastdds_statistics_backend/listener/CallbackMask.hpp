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
 * @file CallbackMask.hpp
 */

#ifndef FASTDDS_STATISTICS_BACKEND_LISTENER__CALLBACKMASK_HPP
#define FASTDDS_STATISTICS_BACKEND_LISTENER__CALLBACKMASK_HPP

#include <fastdds_statistics_backend/types/Bitmask.hpp>

#include <cstdint>

namespace eprosima {
namespace statistics_backend {

/**
 * Each value identifies one of the user callbacks available on the library.
 * These values can be combined with the '|' operator to form a mask
 * and configure which events are going to be notified to the user.
 *
 * \sa CallbackMask
 */
enum class CallbackKind : int32_t
{
    /// Represents the on_topic_discovery() callback
    ON_TOPIC_DISCOVERY          = 1 << 0,

    /// Represents the on_participant_discovery() callback
    ON_PARTICIPANT_DISCOVERY    = 1 << 1,

    /// Represents the on_datawriter_discovery() callback
    ON_DATAWRITER_DISCOVERY     = 1 << 2,

    /// Represents the on_datareader_discovery() callback
    ON_DATAREADER_DISCOVERY     = 1 << 3,

    /// Represents the on_host_discovery() callback
    ON_HOST_DISCOVERY           = 1 << 4,

    /// Represents the on_user_discovery() callback
    ON_USER_DISCOVERY           = 1 << 5,

    /// Represents the on_process_discovery() callback
    ON_PROCESS_DISCOVERY        = 1 << 6,

    /// Represents the on_locator_discovery() callback
    ON_LOCATOR_DISCOVERY        = 1 << 7,

    /// Represents the on_data_available() callback
    ON_DATA_AVAILABLE           = 1 << 8,

    /// Represents the on_domain_view_graph_update() callback
    ON_DOMAIN_VIEW_GRAPH_UPDATE      = 1 << 9,

    /// Represents the on_status_reported() callback
    ON_STATUS_REPORTED         = 1 << 10,

    /// Represents the on_alert_reported() callback
    ON_ALERT_REPORTED          = 1 << 11
};

/**
 * @brief Bitmask of callback kinds
 *
 * values of CallbackKind can be combined with the '|' operator to build the mask:
 *
 * \code{.cpp}
 *     CallbackMask mask = CallbackKind::ON_DATAWRITER_DISCOVERY | CallbackKind::ON_DATAREADER_DISCOVERY;
 * \endcode
 *
 * \sa Bitmask
 */
using CallbackMask = Bitmask<CallbackKind>;


} // namespace statistics_backend
} // namespace eprosima

#endif // FASTDDS_STATISTICS_BACKEND_LISTENER__CALLBACKMASK_HPP
