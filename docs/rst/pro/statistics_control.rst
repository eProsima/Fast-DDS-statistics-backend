.. include:: ../exports/alias.include

.. _pro_statistics_control:

Statistics Collection Control |Pro|
====================================

A monitor started with |init_monitor-api| only creates the statistics DataReaders it strictly needs at
initialization: the monitor service and physical data readers. In *Fast DDS Statistics Backend*, every other
statistics topic is subscribed to as soon as its type is enabled on the observed participants. *Fast DDS
Statistics Backend Pro* instead lets an application create the remaining readers on demand, so a monitor only
consumes the resources needed for the statistics being observed. It also lets an application query several
|StatisticKind-api| values from a single pass over the database instead of one |get_data-api| call per kind.
Several of Pro's internal query paths are also optimized to reduce per-call database access overhead, whether or
not an application uses the features above.

On-demand statistics readers
-------------------------------

* |enable_statistics_reader-api| creates the statistics DataReader for a given statistics topic in a monitor.
  It is idempotent: enabling an already-enabled reader has no effect.
* |disable_statistics_reader-api| destroys it again. It is also idempotent. The always-on readers (monitor
  service and physical data) cannot be disabled; attempting to do so throws |BadParameter-api|.
* |get_enabled_statistics_readers-api| returns the names of the statistics topics that currently have an active
  reader in a given monitor.

.. literalinclude:: /code/StatisticsBackendProTests.cpp
    :language: c++
    :start-after: //CONF-PRO-STATISTICS-READERS-EXAMPLE
    :end-before: //!
    :dedent: 8

Both |enable_statistics_reader-api| and |disable_statistics_reader-api| take the name of a statistics topic (see
``fastdds/statistics/topic_names.hpp`` in *Fast DDS*), and throw |BadParameter-api| if the monitor ID is unknown
or the topic name is not a recognized statistics topic.

If an XML DataReader profile whose name matches the statistics topic name is loaded in the process,
|enable_statistics_reader-api| creates the reader with that profile instead of the library's default QoS (for
example, to raise the history depth on a high-rate topic). Otherwise the default QoS is used, as in *Fast DDS
Statistics Backend*.

Computing several statistics in one query
--------------------------------------------

|get_data-api| in :ref:`statistics_backend_get_data` computes a single |StatisticKind-api| over a time series
split into bins. *Fast DDS Statistics Backend Pro* adds two overloads that compute a collection of
|StatisticKind-api| values (for example ``MEAN``, ``MAX`` and ``MIN`` together) from raw data retrieved from the
database once per source-target (or single) entity pair, instead of once per requested statistic:

.. literalinclude:: /code/StatisticsBackendProTests.cpp
    :language: c++
    :start-after: //CONF-PRO-GET-DATA-MULTIPLE-STATISTICS-EXAMPLE
    :end-before: //!
    :dedent: 8

The two overloads mirror the single-statistic |get_data-api| overloads described in
:ref:`statistics_backend_get_data`: one takes separate source and target entity-id collections for a
two-entity |DataKind-api|, the other takes a single entity-id collection for a single-entity |DataKind-api|.
Both return a map keyed by the requested |StatisticKind-api|, each value being the same |StatisticsData-api|
series that |get_data-api| would return for that one statistic. If the database has no data for a given bin,
the reported value is ``NaN``, as with |get_data-api|. Both throw |BadParameter-api| if the
``data_type`` requires a different entity-collection shape than the one given, if the timestamps are invalid, or
if any entity ID has the wrong kind.
