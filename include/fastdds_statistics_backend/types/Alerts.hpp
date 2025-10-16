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

#include <fastdds_statistics_backend/types/Notifiers.hpp>
#include <fastdds_statistics_backend/types/types.hpp>
#include <fastdds/rtps/common/Guid.hpp>

namespace eprosima {
namespace statistics_backend {

typedef uint32_t AlertId;

/*
 * Available alert kinds
 */
enum class AlertKind
{
    // Invalid alert kind
    INVALID_ALERT,
    // Alert triggered when new data is received
    NEW_DATA_ALERT,
    // Alert triggered when no data is received for a certain time
    NO_DATA_ALERT
};

// Comparison kinds for alert triggering
enum class AlertComparison
{
    // Invalid comparison
    INVALID_ALERT_CMP,
    // Greater than
    GT_ALERT_CMP,
    // Less than
    LT_ALERT_CMP
};

class AlertInfo
{
private:

    // Unique identifier for the alert
    AlertId id;
    // Kind of alert
    AlertKind alert_kind;
    // String name of alert
    std::string name;
    // Domain id
    EntityId domain_id;
    // These names are kept to be able to locate entities even if the alert
    // is created before the entity is discovered
    std::string host_name;
    std::string user_name;
    std::string topic_name;
    // Conditions for triggering
    AlertComparison cmp;
    double trigger_threshold;
    // Last trigger
    std::chrono::system_clock::time_point last_trigger;
    std::chrono::milliseconds time_between_triggers;
    // List of notifiers
    std::vector<NotifierId> notifiers;

public:

    //! Default constructor
    AlertInfo() = default;

    //! Constructor
    AlertInfo(
            AlertKind alert_kind,
            std::string name,
            EntityId domain_id,
            std::string host_name,
            std::string user_name,
            std::string topic_name,
            AlertComparison cmp,
            double trigger_threshold,
            std::chrono::milliseconds time_between_triggers)
        : alert_kind(alert_kind)
        , name(name)
        , domain_id(domain_id)
        , host_name(host_name)
        , user_name(user_name)
        , topic_name(topic_name)
        , cmp(cmp)
        , trigger_threshold(trigger_threshold)
        , time_between_triggers(time_between_triggers)
    {
        reset_trigger_time();
    }

    //! Reset the last trigger time to now
    void reset_trigger_time()
    {
        last_trigger = std::chrono::system_clock::now();
    }

    //! Check if the entity parameters match (empty parameters are wildcards)
    bool entity_matches(
            std::string stat_host,
            std::string stat_user,
            std::string stat_topic) const
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

    //! Check if the value triggers the alert based on the comparison and threshold
    bool value_triggers(
            double value) const
    {
        switch (cmp)
        {
            case AlertComparison::GT_ALERT_CMP:
                return value > trigger_threshold;
            case AlertComparison::LT_ALERT_CMP:
                return value < trigger_threshold;
            default:
                return false;
        }
    }

    //! Check if the value triggers the alert based on the comparison and threshold
    bool value_triggers(
            uint64_t value) const
    {
        switch (cmp)
        {
            case AlertComparison::GT_ALERT_CMP:
                return value > trigger_threshold;
            case AlertComparison::LT_ALERT_CMP:
                return value < trigger_threshold;
            default:
                return false;
        }
    }

    //! Check if enough time has passed since the last trigger
    bool time_allows_trigger() const
    {
        auto now_ts = std::chrono::system_clock::now();
        return (now_ts - last_trigger) > time_between_triggers;
    }

    //! Check trigger conditions for a specific alert that uses a double value
    bool check_trigger_conditions(
            std::string host,
            std::string user,
            std::string topic,
            double value) const
    {
        return entity_matches(host, user, topic) && value_triggers(value) && time_allows_trigger();
    }

    //! Check trigger conditions for a specific alert that uses an integer value
    bool check_trigger_conditions(
            std::string host,
            std::string user,
            std::string topic,
            uint64_t value) const
    {
        return entity_matches(host, user, topic) && value_triggers(value) && time_allows_trigger();
    }

    //! Trigger the alert (reset time and return true)
    bool trigger()
    {
        reset_trigger_time();
        return true;
    }

    //! Set the alert ID
    void set_id(
            AlertId alert_id)
    {
        id = alert_id;
    }

    //! Get the alert ID
    AlertId get_alert_id() const
    {
        return id;
    }

    //! Get the alert kind
    AlertKind get_alert_kind() const
    {
        return alert_kind;
    }

    //! Get the domain id
    EntityId get_domain_id() const
    {
        return domain_id;
    }

    //! Get the alert name
    std::string get_alert_name() const
    {
        return name;
    }

    //! Get the host name
    std::string get_host_name() const
    {
        return host_name;
    }

    //! Get the user name
    std::string get_user_name() const
    {
        return user_name;
    }

    //! Get the topic name
    std::string get_topic_name() const
    {
        return topic_name;
    }

    //! Get the trigger threshold
    double get_trigger_threshold() const
    {
        return trigger_threshold;
    }

    //! Get the minimum time between triggers
    std::chrono::milliseconds get_time_between_triggers() const
    {
        return time_between_triggers;
    }

    //! Add a notifier to the alert
    void add_notifier(
            NotifierId notifier_id)
    {
        notifiers.push_back(notifier_id);
    }

    //! Remove a notifier from the alert
    void remove_notifier(
            NotifierId notifier_id)
    {
        notifiers.erase(std::remove(notifiers.begin(), notifiers.end(), notifier_id), notifiers.end());
    }

    //! Get the list of notifiers
    const std::vector<NotifierId>& get_notifiers() const
    {
        return notifiers;
    }

};

//! Specialization for NewDataAlert
struct NewDataAlertInfo : AlertInfo
{

    //! Constructor with default values for NewDataAlert
    NewDataAlertInfo(
            std::string name,
            EntityId domain_id,
            std::string host_name,
            std::string user_name,
            std::string topic_name,
            std::chrono::milliseconds time_between_triggers)
        : AlertInfo(AlertKind::NEW_DATA_ALERT, name, domain_id, host_name, user_name, topic_name,
                AlertComparison::GT_ALERT_CMP, 0.0,
                time_between_triggers)
    {
    }

};

//! Specialization for NoDataAlert
struct NoDataAlertInfo : AlertInfo
{
    //! Constructor with default values for NoDataAlert
    NoDataAlertInfo(
            std::string name,
            EntityId domain_id,
            std::string host_name,
            std::string user_name,
            std::string topic_name,
            double threshold,
            std::chrono::milliseconds time_between_triggers)
        : AlertInfo(AlertKind::NO_DATA_ALERT, name, domain_id, host_name, user_name, topic_name,
                AlertComparison::LT_ALERT_CMP,
                threshold,
                time_between_triggers)
    {
    }

};

} //namespace statistics_backend
} //namespace eprosima


#endif // ifndef FASTDDS_STATISTICS_BACKEND_TYPES__ALERTS_HPP
