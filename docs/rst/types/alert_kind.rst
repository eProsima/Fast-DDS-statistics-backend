.. include:: ../exports/alias.include

.. _types_alert_kind:

AlertKind
==========

The *eProsima Fast DDS Statistics Backend* keeps track of the alerts generated in the DDS layout.
The following list shows the different alerts that are tracked:

- |AlertKind::INVALID-api|: Invalid alert.
- |AlertKind::NEW_DATA-api|: Triggered as soon as any new value greater than ``0.0`` is received for the
  monitored entity. Its purpose is to notify the reception of new data, so it does not have a timeout: it never
  fires on a period of silence, only on the arrival of data.
- |AlertKind::NO_DATA-api|: Triggered when the monitored value drops below a configured threshold. Unlike
  |AlertKind::NEW_DATA-api|, this alert kind does have a timeout enabled: in addition to its own threshold-based
  trigger condition, it also triggers every time its timeout period elapses without a new value having reset it.


