.. include:: ../exports/alias.include

.. _types_alertinfo:

AlertInfo
=========

Represents an alert configuration and its trigger conditions.
The ``AlertInfo`` class stores metadata and logic for determining
when an alert should be triggered based on value comparisons and timing constraints.

The structure contains the following fields:

- **`id`** (`AlertId`) — Unique identifier for the alert.
- **`alert_kind`** (`AlertKind`) — Type of alert (e.g., `NEW_DATA_ALERT`, `NO_DATA_ALERT`).
- **`name`** (`std::string`) — Human-readable name of the alert.
- **`domain_id`** (`EntityId`) — DDS domain ID associated with the alert.
- **`host_name`**, **`user_name`**, **`topic_name`** (`std::string`) — Entity identifiers used to locate monitored entities. Empty strings act as wildcards.
- **`cmp`** (`AlertComparison`) — Comparison operator used for threshold evaluation (`GT` = greater than, `LT` = less than).
- **`trigger_threshold`** (`double`) — Numeric threshold that triggers the alert when crossed.
- **`last_trigger_ts`** (`std::chrono::system_clock::time_point`) — Timestamp of the last time the alert was triggered.
- **`time_between_triggers`** (`std::chrono::milliseconds`) — Minimum time interval between consecutive triggers.
- **`timeout_enabled`** (`bool`) — Whether timeout-based triggering is enabled.
- **`time_to_timeout`** (`std::chrono::milliseconds`) — Duration without data before a timeout alert is raised.
- **`last_timeout_check_ts`** (`std::chrono::system_clock::time_point`) — Timestamp of the last timeout condition check.
- **`notifiers`** (`std::vector<NotifierId>`) — List of notifier identifiers; each notifier represents a callback or action executed when the alert triggers.
