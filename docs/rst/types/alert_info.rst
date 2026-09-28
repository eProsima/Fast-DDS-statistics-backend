.. include:: ../exports/alias.include

.. _types_alertinfo:

AlertInfo
=========

Represents an alert configuration and its trigger conditions.
The ``AlertInfo`` class stores metadata and logic for determining
when an alert should be triggered based on value comparisons and timing constraints.

The structure contains the following fields:

- ``id`` (|AlertId-api|) - Unique identifier for the alert.
- ``alert_kind`` (|AlertKind-api|) - Type of alert (|AlertKind::NEW_DATA-api| or |AlertKind::NO_DATA-api|).
- ``name`` (``std::string``) - Human-readable name of the alert.
- ``domain_id`` (|EntityId-api|) - |EntityId-api| of the |DOMAIN-api| entity monitored by the alert.
- ``host_name``, ``user_name``, ``topic_name`` (``std::string``) - Filters used to select which entities the
  alert applies to. An empty string, or the literal string ``"ALL"``, acts as a wildcard for that filter.
- ``cmp`` (``AlertComparison``) - Comparison operator used for threshold evaluation: ``GT_ALERT_CMP`` (greater
  than) or ``LT_ALERT_CMP`` (less than). Set automatically from ``alert_kind`` when the alert is created via
  |set_alert-api|.
- ``trigger_threshold`` (``double``) - Numeric threshold the monitored value is compared against.
- ``last_trigger_ts`` (``std::chrono::system_clock::time_point``) - Timestamp of the last time the alert was triggered.
- ``time_between_triggers`` (``std::chrono::milliseconds``) - Minimum time interval between consecutive triggers.
- ``timeout_enabled`` (``bool``) - Whether timeout-based triggering is enabled for this alert.
- ``time_to_timeout`` (``std::chrono::milliseconds``) - Duration without matching data before a timeout is raised.
- ``last_timeout_check_ts`` (``std::chrono::system_clock::time_point``) - Timestamp of the last timeout condition check.
- ``notifiers`` (``std::vector<NotifierId>``) - List of notifier identifiers; each notifier represents an action
  (see :ref:`types_notifier`) executed when the alert triggers.

When the alert's trigger condition is checked, the outcome is reported as an ``AlertTriggerCause``:
``NO_TRIGGER``, ``THRESHOLD_TRIGGER`` (the comparison against ``trigger_threshold`` matched), or
``TIMEOUT_TRIGGER`` (``time_to_timeout`` elapsed without new matching data).
