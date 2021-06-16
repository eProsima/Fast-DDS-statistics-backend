.. include:: ../exports/alias.include

.. _listeners_domain_listener:

DomainListener
==============

|DomainListener-api| is an abstract class defining the callbacks
that will be triggered in response to changes in the DDS network
(discovery of domain, participants, topics, data readers, data writers,
and arrival of new statistics data).
By default, all these callbacks are empty and do nothing.
The user should implement a specialization of this class overriding the callbacks
that are needed on the application.
Callbacks that are not overridden will maintain their empty implementation.

DomainListener defines the following callbacks:

* |DomainListener::on_data_available-api|:
  New statistics data has been received by the backend.
  The arguments in the callback specifies the kind of the received data
  and the entity to which this data refers.

* |DomainListener::on_topic_discovery-api|:
  A new topic has been discovered in the monitored domain,
  or an already known topic has been updated with a new QoS value.
  The topics are never *undiscovered*.
  The arguments in the callback specifies the ID of the topic and the domain
  to which it belongs.

* |DomainListener::on_participant_discovery-api|:
  A new participant has been discovered in the monitored domain,
  or an already known participant has been updated with a new
  Quality of Service (QoS) value,
  or an already known participant has been removed from the network.
  The arguments in the callback specifies the ID of the participant and the domain
  to which it belongs.

* |DomainListener::on_datareader_discovery-api|:
  A new data reader has been discovered in the monitored domain,
  or an already known data reader has been updated with a new QoS value,
  or an already known data reader has been removed from the network.
  The arguments in the callback specifies the ID of the data reader and the domain
  to which it belongs.

* |DomainListener::on_datawriter_discovery-api|:
  A new data writer has been discovered in the monitored domain,
  or an already known data writer has been updated with a new QoS value,
  or an already known data writer has been removed from the network.
  The arguments in the callback specifies the ID of the data writer and the domain
  to which it belongs.

