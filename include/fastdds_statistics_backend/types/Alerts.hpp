/* Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
#include <fastdds/rtps/common/Guid.hpp>

namespace eprosima {
namespace statistics_backend {


using GUID_t = fastdds::rtps::GUID_t;
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

enum class AlertComparison
{
    INVALID,
    GT,
    LT,
};

class AlertInfo
{
    private:
        // Kind of alert
        AlertKind alert_kind;
        // String name of alert
        std::string name;
        // These names are kept to be able to locate entities even if the alert
        // is created before the entity is discovered
        std::string host_name;
        std::string user_name;
        std::string topic_name;
        // std::string alias;        // ALIAS alerts are defined by <alias>
        // TODO: Additionally, the GUID can be used to match the alert
        // GUID_t entity_guid;
        // Entity to which the alert applies, if known
        // Structure that contains the subentitiesto which the alert applies
        // std::map<EntityId, bool> subentities_trigger;
        // EntityId entity_id;
        // Conditions for triggering
        AlertComparison cmp;
        double trigger_threshold;
        // Last trigger
        std::chrono::system_clock::time_point last_trigger;
        std::chrono::milliseconds time_between_triggers;

    public:

    AlertInfo(AlertKind alert_kind, std::string name, std::string host_name, std::string user_name, std::string topic_name, AlertComparison cmp, double trigger_threshold, std::chrono::milliseconds time_between_triggers)
        : alert_kind(alert_kind)
        , name(name)
        , host_name(host_name)
        , user_name(user_name)
        , topic_name(topic_name)
        , cmp(cmp)
        , trigger_threshold(trigger_threshold)
        , time_between_triggers(time_between_triggers)
    {
        reset_trigger_time();
    }

    void reset_trigger_time()
    {
        last_trigger = std::chrono::system_clock::now();
    }

    bool entity_matches(std::string stat_host, std::string stat_user, std::string stat_topic) const
    {
        bool match = true;
        if (!(host_name.empty()))
        {
            match &= (host_name == stat_host);
        }
        if (match && !(user_name.empty()))
        {
            match &= (user_name == stat_user);
        }
        if (match && !(topic_name.empty()))
        {
            match &= (topic_name == stat_topic);
        }
        return match;
    }

    bool value_triggers(double value) const
    {
        switch (cmp)
        {
        case AlertComparison::GT:
            return value > trigger_threshold;
        case AlertComparison::LT:
            return value < trigger_threshold;
        default:
            return false;
        }
    }

    bool check_trigger_conditions(
            std::string host,
            std::string user,
            std::string entity,
            double value) const
    {
        return entity_matches(host, user, entity) && value_triggers(value);
    }

    AlertKind get_alert_kind() const
    {
        return alert_kind;
    }
};

struct NewDataAlertInfo : AlertInfo
{
    NewDataAlertInfo(std::string name, std::string host_name, std::string user_name, std::string topic_name, std::chrono::milliseconds time_between_triggers)
        : AlertInfo(AlertKind::NEW_DATA, name, host_name, user_name, topic_name, AlertComparison::GT, 0.0, time_between_triggers)
    {
    }
};

struct NoDataAlertInfo : AlertInfo
{
    NoDataAlertInfo(std::string name, std::string host_name, std::string user_name, std::string topic_name, double threshold, std::chrono::milliseconds time_between_triggers)
    : AlertInfo(AlertKind::NO_DATA, name, host_name, user_name, topic_name, AlertComparison::LT, threshold, time_between_triggers)
    {
    }
};

} //namespace statistics_backend
} //namespace eprosima


#endif
