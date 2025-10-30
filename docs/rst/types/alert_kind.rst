.. include:: ../exports/alias.include

.. _types_alert_kind:

AlertKind
==========

The *eProsima Fast DDS Statistics Backend* keeps track of the alerts generated in the DDS layout.
The following list shows the different alerts that are tracked:

- |AlertKind::INVALID-api|: Invalid alert.
- |AlertKind::NEW_DATA-api|: Triggered when a statistics message with a ``DATA_COUNT`` greater than 0 is received. Note that a NO_DATA alert
                            does not have timeouts enabled, as its purpose is to notify the reception of new data.
- |AlertKind::NO_DATA-api|: Triggered when a statistics message with a ``SUBSCRIPTION_THROUGHPUT`` lower than a configured threshold is received.
                            In addition to their own trigger conditions, ``NO_DATA``` alerts will also trigger every time their timeout period elapses
                            without having been triggered.


