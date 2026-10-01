.. include:: ../exports/alias.include

.. _types_alert_kind:

AlertKind
==========

The *eProsima Fast DDS Statistics Backend* tracks the alerts generated in the DDS layout.
The alert kinds are:

- |AlertKind::INVALID-api|: Invalid alert.
- |AlertKind::NEW_DATA-api|: Triggered as soon as any new value greater than ``0.0`` is received for the
  monitored entity. It notifies the reception of new data, so it has no timeout: it fires only when data
  arrives, never on a period of silence.
- |AlertKind::NO_DATA-api|: Triggered when the monitored value drops below a configured threshold. Unlike
  |AlertKind::NEW_DATA-api|, this alert kind has a timeout enabled: besides its threshold-based trigger
  condition, it also triggers every time its timeout period elapses without a new value resetting it.


