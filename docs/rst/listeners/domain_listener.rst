.. include:: ../exports/alias.include

.. _listeners_domain_listener:

DomainListener
==============

|DomainListener-api| is an abstract class defining the callbacks
triggered in response to changes in the DDS network
(discovery of domain, participants, topics, data readers, data writers,
and arrival of new statistics data).
By default, all these callbacks are empty and do nothing.
The user implements a specialization of this class that overrides the callbacks
the application needs. Callbacks that are not overridden keep their empty implementation.

The topic, participant, data reader, and data writer discovery callbacks take a ``const DomainListener::Status&``
argument, besides the discovered entity's ID, that describes the discovery event:

* ``total_count``: total cumulative count of entities of that kind discovered so far. Increases monotonically
  with every new discovery.
* ``total_count_change``: the change in ``total_count`` since the listener was last called for that entity kind.
  Positive if entities were discovered since the last call, zero otherwise (it never decreases, since undiscovered
  entities are not subtracted from ``total_count``).
* ``current_count``: the number of currently discovered entities of that kind (never negative).
* ``current_count_change``: the change in ``current_count`` since the listener was last called.
  Positive, negative, or zero, depending on whether entities were discovered, undiscovered, or only had a QoS
  change since the last call.

DomainListener defines the following callbacks:

* |DomainListener::on_data_available-api|:
  New statistics data has been received by the backend.
  The callback arguments specify the kind of the received data
  and the entity it refers to.

* |DomainListener::on_topic_discovery-api|:
  A new topic has been discovered in the monitored domain,
  or a known topic has been updated with a new QoS value.
  Topics are never *undiscovered*.
  The callback arguments specify the ID of the topic and the domain
  it belongs to.

* |DomainListener::on_participant_discovery-api|:
  A new participant has been discovered in the monitored domain,
  or a known participant has been updated with a new
  Quality of Service (QoS) value or removed from the network.
  The callback arguments specify the ID of the participant and the domain
  it belongs to.

* |DomainListener::on_datareader_discovery-api|:
  A new data reader has been discovered in the monitored domain,
  or a known data reader has been updated with a new QoS value
  or removed from the network.
  The callback arguments specify the ID of the data reader and the domain
  it belongs to.

* |DomainListener::on_datawriter_discovery-api|:
  A new data writer has been discovered in the monitored domain,
  or a known data writer has been updated with a new QoS value
  or removed from the network.
  The callback arguments specify the ID of the data writer and the domain
  it belongs to.

* |DomainListener::on_domain_view_graph_update-api|:
  A domain view graph has been updated.
  The callback arguments specify the ID of the domain whose graph has been updated.

* |DomainListener::on_status_reported-api|:
  New status data has been received from the backend.
  The callback arguments specify the status kind of the received data and the entity it refers to.

* |DomainListener::on_alert_triggered-api|:
  An alert has been triggered in the monitored domain.
  The callback arguments contain the domain, the ID of the entity the triggering data refers to,
  the alert information, the GUID (as a string) of the entity that reported the triggering data, and
  the data that triggered the alert.

* |DomainListener::on_alert_timeout-api|:
  An alert has been unmatched, or created without any matching entities in the monitored domain.
  The callback arguments contain the alert information and the domain the alert belongs to.
