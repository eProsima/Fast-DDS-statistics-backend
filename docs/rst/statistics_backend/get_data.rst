.. include:: ../exports/alias.include

.. _statistics_backend_get_data:

Retrieve statistical data
-------------------------

.. warning::
    This feature is currently not supported.
    It will be implemented on a future release of *Fast DDS Statistics Backend*.

*Fast DDS Statistics Backend* provides two overloads of |get_data-api| to retrieve statistical data of a given
|DataKind-api| within a time frame.
This time interval is evenly divided into the specified number of bins, each one with size
:math:`(t_to - t_from)/(# of bins)`.
For each of these bins, a new |StatisticsData-api| value is calculated applying the given |StatisticKind-api| to all the
data points in it.
The result is a collection of |StatisticsData-api| elements with size equal to the number of specified bins.

Depending on the |DataKind-api|, the data is related to one or two entities, e.g. |FASTDDS_LATENCY-api| measures the
latency between a write operation on the data writer side and the notification to the user when the data is available on
reader side, whereas |HEARTBEAT_COUNT-api| contains the amount of sent HEARTBEATs.
Because of this difference, |get_data-api| can take either one or two |EntityId-api| related to the |DataKind-api| in
question.
The following table illustrates the expected inputs depending on the query's |DataKind-api| passed to |get_data-api|:

+-------------------------------+------------------------------------+------------------------------------+
| |DataKind-api|                | Source collection |EntityKind-api| | Target collection |EntityKind-api| |
+===============================+====================================+====================================+
| |FASTDDS_LATENCY-api|         | |DATAWRITER-api|                   | |DATAREADER-api|                   |
+-------------------------------+------------------------------------+------------------------------------+
| |NETWORK_LATENCY-api|         | |LOCATOR-api|                      | |LOCATOR-api|                      |
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
| |DISCOVERED_ENTITY-api|       | |PARTICIPANT-api|                  | Not applicable                     |
+-------------------------------+------------------------------------+------------------------------------+
| |SAMPLE_DATAS-api|            | |DATAWRITER-api|                   | Not applicable                     |
+-------------------------------+------------------------------------+------------------------------------+



.. _statistics_backend_get_data_examples:

Examples
^^^^^^^^

Following, some example queries are provided to serve a inspiration for applications using
*Fast DDS Statistics Backend*.

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

It is also possible to retrieve all the data points of a given |DataKind-api| within the time frame.
This is done by setting the number of bins to 0.
In this case, the |StatisticKind-api| is ignored so it can be left to its default value.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-GET-ALL-POINTS-EXAMPLE
   :end-before: //!
   :dedent: 8

For more information about the available |DataKind-api| and |StatisticKind-api| please refer to :ref:`types_data_kind`
and :ref:`types_statistic_kind` respectively.
