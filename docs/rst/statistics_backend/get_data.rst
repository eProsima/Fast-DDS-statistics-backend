.. include:: ../exports/alias.include

.. _statistics_backend_get_data:

Get statistical data
--------------------

*Fast DDS Statistics Backend* has four overloads of |get_data-api| to retrieve statistical data of a given
|DataKind-api| within a time frame (for all the reported |DataKind-api|, see :ref:`types_data_kind`).
Two of them take an explicit ``t_from``/``t_to`` time range, which defaults to the whole recorded history when
omitted. The other two omit the time range arguments and always use that same default. They exist only so
``bins`` and ``statistic`` can be passed without also naming the time-range parameters (see
:ref:`statistics_backend_get_data_no_time_overloads` below).
The time interval is evenly divided into the specified number of bins, each one with size
:math:`(t_{to} - t_{from})/(\# bins)`.
For each bin, a new |StatisticsData-api| value is calculated by applying the given |StatisticKind-api| to all the
data points in it.
The result is a collection of |StatisticsData-api| elements with size equal to the number of specified bins.

.. important::
   If the number of bins is set to zero, then all data points are returned and no statistic is calculated for the
   series.


Depending on the |DataKind-api|, the data relates to one or two entities. For example, |FASTDDS_LATENCY-api| measures the
latency between a write operation on the data writer side and the notification to the user when the data is available on
reader side, whereas |HEARTBEAT_COUNT-api| contains the number of sent HEARTBEATs.
For this reason, |get_data-api| can take either one or two |EntityId-api| related to the |DataKind-api| in
question.
The table below lists the expected inputs for each |DataKind-api| passed to |get_data-api|:

+-------------------------------+------------------------------------+------------------------------------+
| |DataKind-api|                | Source collection |EntityKind-api| | Target collection |EntityKind-api| |
+===============================+====================================+====================================+
| |FASTDDS_LATENCY-api|         | |DATAWRITER-api|                   | |DATAREADER-api|                   |
+-------------------------------+------------------------------------+------------------------------------+
| |NETWORK_LATENCY-api|         | |PARTICIPANT-api|                  | |LOCATOR-api|                      |
+-------------------------------+------------------------------------+------------------------------------+
| |PUBLICATION_THROUGHPUT-api|  | |DATAWRITER-api|                   | Not applicable                     |
+-------------------------------+------------------------------------+------------------------------------+
| |SUBSCRIPTION_THROUGHPUT-api| | |DATAREADER-api|                   | Not applicable                     |
+-------------------------------+------------------------------------+------------------------------------+
| |RTPS_PACKETS_SENT-api|       | |PARTICIPANT-api|                  | |LOCATOR-api|                      |
+-------------------------------+------------------------------------+------------------------------------+
| |RTPS_BYTES_SENT-api|         | |PARTICIPANT-api|                  | |LOCATOR-api|                      |
+-------------------------------+------------------------------------+------------------------------------+
| |RTPS_PACKETS_LOST-api|       | |PARTICIPANT-api|                  | |LOCATOR-api|                      |
+-------------------------------+------------------------------------+------------------------------------+
| |RTPS_BYTES_LOST-api|         | |PARTICIPANT-api|                  | |LOCATOR-api|                      |
+-------------------------------+------------------------------------+------------------------------------+
| |RESENT_DATA-api|             | |DATAWRITER-api|                   | Not applicable                     |
+-------------------------------+------------------------------------+------------------------------------+
| |HEARTBEAT_COUNT-api|         | |DATAWRITER-api|                   | Not applicable                     |
+-------------------------------+------------------------------------+------------------------------------+
| |ACKNACK_COUNT-api|           | |DATAREADER-api|                   | Not applicable                     |
+-------------------------------+------------------------------------+------------------------------------+
| |NACKFRAG_COUNT-api|          | |DATAREADER-api|                   | Not applicable                     |
+-------------------------------+------------------------------------+------------------------------------+
| |GAP_COUNT-api|               | |DATAWRITER-api|                   | Not applicable                     |
+-------------------------------+------------------------------------+------------------------------------+
| |DATA_COUNT-api|              | |DATAWRITER-api|                   | Not applicable                     |
+-------------------------------+------------------------------------+------------------------------------+
| |PDP_PACKETS-api|             | |PARTICIPANT-api|                  | Not applicable                     |
+-------------------------------+------------------------------------+------------------------------------+
| |EDP_PACKETS-api|             | |PARTICIPANT-api|                  | Not applicable                     |
+-------------------------------+------------------------------------+------------------------------------+
| |DISCOVERY_TIME-api|          | |PARTICIPANT-api|                  | See note below                     |
+-------------------------------+------------------------------------+------------------------------------+
| |SAMPLE_DATAS-api|            | |DATAWRITER-api|                   | Not applicable                     |
+-------------------------------+------------------------------------+------------------------------------+

.. note::
   Unlike every other two-entity |DataKind-api|, |DISCOVERY_TIME-api| does not relate to a single fixed pair of
   |EntityKind-api|. The source is always the discovering |PARTICIPANT-api|, but the target (the discovered
   entity) can be a |PARTICIPANT-api|, a |DATAWRITER-api|, or a |DATAREADER-api|, depending on what was
   discovered.

|get_data-api| throws |BadParameter-api| if the calling parameters are not consistent.

|get_data_supported_entity_kinds-api| returns all the |EntityKind-api|
pairs suitable for a given |DataKind-api|, according to this table.

- For a |DataKind-api| that only relates to one Entity,
  the first element of the pair is the |EntityKind-api| of such Entity,
  while the second element is |EntityKind_INVALID-api|.
- For a |DataKind-api| that relates to two Entities, the first element of the pair is the |EntityKind-api|
  of the source Entity, while the second element is the |EntityKind-api| of the target Entity.

These pairs are the source and target |EntityKind-api| that |get_data-api| accepts for the given |DataKind-api|.
To prepare a call to |get_data-api| from an |EntityKind-api|, first call |get_data_supported_entity_kinds-api| with the |DataKind-api|
to get the |EntityKind-api| of the related entities.
Then, call |get_entities-api| to get the available entities of that kind.
Finally, call |get_data-api| with the pairs that |get_entities-api| returns.

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-GET-FASTDDS-LATENCY-SUPPORTED-ENTITY-KINDS
    :end-before: //!
    :dedent: 8

.. warning::
   If *Fast DDS Statistics Backend* has no data for a given bin, the value returned for it is the one supplied by
   `std::numeric_limits<double>::quiet_NaN <https://en.cppreference.com/w/cpp/types/numeric_limits/quiet_NaN>`_.

.. _statistics_backend_get_data_no_time_overloads:

Overloads without a time range
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Both forms of |get_data-api| described above (source/target, and single-entity) have a second
overload that takes ``bins`` and ``statistic`` but omits ``t_from``/``t_to``. It always uses the default
time range of those two parameters (the whole recorded history):

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-GET-DATA-NO-TIME-RANGE-OVERLOADS
    :end-before: //!
    :dedent: 8

These overloads let ``bins`` and ``statistic`` be passed positionally without also naming
``t_from``/``t_to``. They have the same preconditions, |BadParameter-api| behavior, and NaN-on-no-data semantics as
the overloads that take a time range.

.. _statistics_backend_get_data_examples:

Examples
^^^^^^^^

Applications using *Fast DDS Statistics Backend* can use the following example queries as a starting point.

.. todo::
   Include an output example for each example here.

DataWriter's Fast DDS Latency median example
""""""""""""""""""""""""""""""""""""""""""""

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-GET-DATA-DATAWRITER-FASTDDS_LATENCY
   :end-before: //!
   :dedent: 8

Topic's Fast DDS Latency mean example
"""""""""""""""""""""""""""""""""""""

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-GET-DATA-TOPIC-FASTDDS_LATENCY
   :end-before: //!
   :dedent: 8

Topic's Heartbeat count maximum example
"""""""""""""""""""""""""""""""""""""""

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-GET-DATA-TOPIC-HEARTBEAT_COUNT
   :end-before: //!
   :dedent: 8

Host to Host Fast DDS Latency all points example
""""""""""""""""""""""""""""""""""""""""""""""""

To retrieve all the data points of a given |DataKind-api| within the time frame, set the number of bins to 0.
In this case, the |StatisticKind-api| is ignored, so it can be left at its default value.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-GET-ALL-POINTS-EXAMPLE
   :end-before: //!
   :dedent: 8

For the available |DataKind-api| and |StatisticKind-api| values, see :ref:`types_data_kind`
and :ref:`types_statistic_kind` respectively.
