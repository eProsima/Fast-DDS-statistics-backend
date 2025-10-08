.. include:: ../exports/alias.include

.. _types_alert_kind:

AlertKind
==========

The *eProsima Fast DDS Statistics Backend* keeps track of the alerts generated in the DDS layout.
The following list shows the different alerts that are tracked:

- |AlertKind::INVALID|: Invalid alert.
- |AlertKind::NEW_DATA|: Triggered when a statistics message with a ``DATA_COUNT`` greater than 0 is received.
- |AlertKind::NO_DATA|: Triggered when a statistics message with a ``SUBSCRIPTION_THROUGHPUT`` lower than a configured threshold is received.

In addition to their own trigger conditions, all alerts will also trigger if there are no entitites that match the alert's entity filter.
