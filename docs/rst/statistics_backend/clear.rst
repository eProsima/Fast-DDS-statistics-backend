.. include:: ../exports/alias.include

.. _statistics_backend_clear:

Clear all data related to a monitor
-----------------------------------

.. warning::
    This feature is currently not supported.
    It will be implemented on a future release of *Fast DDS Statistics Backend*.

In case applications are no longer interested in statistical data of a given DDS domain or *Fast DDS* Discovery Server
network, it is possible to delete said data completely.
To do so, the corresponding monitor needs to be stopped before the |clear_monitor-api| operation can be performed.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-CLEAR-EXAMPLE
   :end-before: //!
   :dedent: 8
