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
latency between a write operation on the data writer side and the notification to the user when the data is available in
reader side, whereas |HEARTBEAT_COUNT-api| contains the amount of sent HEARTBEATs.
Because of this difference, |get_data-api| can take either one or two |EntityId-api| related to the |DataKind-api| in
question.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-GET-DATA-OVERLOAD-EXAMPLE
   :end-before: //!
   :dedent: 8

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
