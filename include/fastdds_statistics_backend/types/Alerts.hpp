/* Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file Alerts.hpp
 */

#ifndef FASTDDS_STATISTICS_BACKEND_TYPES__ALERTS_HPP
#define FASTDDS_STATISTICS_BACKEND_TYPES__ALERTS_HPP

#include <fastdds_statistics_backend/types/types.hpp>

namespace eprosima {
namespace statistics_backend {

typedef uint32_t AlertId;

/*
 * Available alert kinds
 */
enum class AlertKind
{
    NONE,
    NEW_DATA,
    NO_DATA
};

/**
 * @struct AlertMessage
 * Base class for all alert messages. It adds the alert kind and source timestamp to the message
 */
struct AlertMessage
{
    AlertMessage(
            AlertKind sample_kind = AlertKind::NONE)
        : kind(sample_kind)
    {
    }

    virtual ~AlertMessage() = default;

    AlertKind kind;
    std::chrono::system_clock::time_point src_ts;
};

/**
 * @struct NoDataAlertMessage
 * Alert message for NO_DATA alert kind
 */
struct NoDataAlertMessage : AlertMessage
{
    NoDataAlertMessage()
        : AlertMessage(AlertKind::NO_DATA)
    {
    }

    virtual ~NoDataAlertMessage() = default;
};

/**
 * @struct NewDataAlertMessage
 * Alert message for NEW_DATA alert kind
 */
struct NewDataAlertMessage : AlertMessage
{
    NewDataAlertMessage()
        : AlertMessage(AlertKind::NEW_DATA)
    {
    }

    virtual ~NewDataAlertMessage() = default;
};


struct AlertInfo
{
    // Identifier
    std::string id;
    // String name of alert
    std::string name;
    // Entity to which the alert applies
    EntityKind entity_kind;
    EntityId entity;
    // The name as plain text is kept to be able to set alerts before the entity id is created
    std::string entity_name;
    // Kind of alert
    AlertKind alert_kind;

    // Conditions for triggering
    bool gt_or_lt;
    double threshold;

    bool triggers(double value)
    {
        if (gt_or_lt)
        {
            return value > threshold;
        }
        else
        {
            return value < threshold;
        }
    }
};

struct NewDataAlertInfo : AlertInfo
{
    NewDataAlertInfo()
    {
        alert_kind = AlertKind::NEW_DATA;
        threshold = 0.0;
        gt_or_lt = true; // gt than 0
    }
};

} //namespace statistics_backend
} //namespace eprosima


#endif
