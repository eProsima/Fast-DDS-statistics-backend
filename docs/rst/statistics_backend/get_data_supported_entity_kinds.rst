.. include:: ../exports/alias.include

.. _statistics_backend_get_data_supported_entity_kinds:

Get entity kinds related to a data kind
---------------------------------------

Each |DataKind-api| value is related to one or two Entities of specific |EntityKind-api| values.
For example, |FASTDDS_LATENCY-api| measures the latency between a write operation on the data writer side
and the notification to the user when the data is available on the reader side,
whereas |HEARTBEAT_COUNT-api| contains the amount of HEARTBEATs sent on a reader.
Some |DataKind-api| relate to more than one |EntityKind-api|,
like the |DISCOVERED_ENTITY-api|, that contains the times at which a participant discovered
either another participant, a writer or a reader.

|get_data_supported_entity_kinds-api| returns all the |EntityKind-api| pairs
related to the given |DataKind-api|.

- For a |DataKind-api| that only relates to one Entity, the first element of the pair is the |EntityKind-api| of such Entity,
  while the second element is |EntityKind_INVALID-api|.
- For a |DataKind-api| that relates to two Entities, the first element of the pair is the |EntityKind-api|
  of the source Entity, while the second element is the |EntityKind-api| of the target Entity.

The source and target pairs returned by this method are the source and target |EntityKind-api| accepted by |get_data-api|
for the given |DataKind-api|.
This is convenient to prepare a call to |get_data-api| from an |EntityKind-api|.
First, call |get_data_supported_entity_kinds-api| with the |DataKind-api| to get the |EntityKind-api| of the related entities.
Then, call |get_entities-api| to get the available entities of that kind.
Finally, call |get_data-api| with the pairs that |get_entities-api| returns.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-GET-FASTDDS-LATENCY-SUPPORTED-ENTITY-KINDS
   :end-before: //!
   :dedent: 8

To learn more about the relation between the |DataKind-api| and the |EntityKind-api|, please refer to
:ref:`types_data_kind`.
