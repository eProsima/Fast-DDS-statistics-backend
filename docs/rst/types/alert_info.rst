.. include:: ../exports/alias.include

.. _types_alertinfo:

AlertInfo
=========

Represents an alert configuration and its trigger conditions.
The ``AlertInfo`` class stores metadata and logic for determining
when an alert should be triggered based on value comparisons and timing constraints.

**Private Members:**

- ``AlertId id``
    Unique identifier for the alert.

- ``AlertKind alert_kind``
    Type or category of the alert.

- ``std::string name``
    Name of the alert.

- ``EntityId domain_id``
    Identifier of the domain this alert belongs to.

- ``std::string host_name``
    Host name related to the alert condition.

- ``std::string user_name``
    User name related to the alert condition.

- ``std::string topic_name``
    Topic name related to the alert condition.

- ``AlertComparison cmp``
    Comparison operator defining the alert trigger logic (e.g., greater than, less than).

- ``double trigger_threshold``
    Threshold value for triggering the alert.

- ``std::chrono::system_clock::time_point last_trigger``
    Timestamp of the last alert trigger.

- ``std::chrono::milliseconds time_between_triggers``
    Minimum time interval between consecutive triggers.

**Constructors:**

.. cpp:function:: AlertInfo()

    Default constructor.

.. cpp:function:: AlertInfo(AlertKind alert_kind, std::string name, EntityId domain_id, std::string host_name, std::string user_name, std::string topic_name, AlertComparison cmp, double trigger_threshold, std::chrono::milliseconds time_between_triggers)

    Initializes a new alert with the given parameters and sets the last trigger time.

**Public Member Functions:**

.. cpp:function:: void reset_trigger_time()

    Resets the last trigger timestamp to the current system time.

.. cpp:function:: bool entity_matches(std::string stat_host, std::string stat_user, std::string stat_topic) const

    Checks whether the provided host, user, and topic match the alert’s target entities.
    Returns ``true`` if all specified fields match with those defined in the alert. Note that if the AlertInfo
    structure does not have a value for a field (i.e., it is an empty string), that field is considered a match.

.. cpp:function:: bool value_triggers(double value) const

    Evaluates if a given value satisfies the alert’s comparison condition.

.. cpp:function:: bool time_allows_trigger() const

    Checks whether enough time has passed since the last trigger to allow a new one.

.. cpp:function:: bool check_trigger_conditions(std::string host, std::string user, std::string entity, double value) const

    Verifies if all trigger conditions (entity match, value threshold, and time interval)
    are satisfied simultaneously.

.. cpp:function:: bool trigger()

    Marks the alert as triggered by resetting the trigger time.
    Always returns ``true``.

