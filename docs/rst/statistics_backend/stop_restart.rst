.. include:: ../exports/alias.include

.. _statistics_backend_stop_restart:

.. .. todo::
..    Stop and restart a monitor
..    --------------------------

Stop a monitor
--------------

.. todo::
   Restarting a stopped monitor is currently not supported.
   It will be implemented on a future release of *Fast DDS Statistics Backend*.

*Fast DDS Statistics Backend* allows for a monitorization to be stopped at any time.
Stopping a monitorization merely means that the internal statistics DataReaders are disabled, but the already received
data is still accessible to applications through the query API (see :ref:`statistics_backend_get_data`).
Is is important to note that:

* Calls to |stop_monitor-api| on an already stopped monitor take no effect.
* |stop_monitor-api| must be called before calling |clear_monitor-api|.
* |stop_monitor-api| throws |BadParameter-api| if the provided monitor ID is not yet registered.

.. todo::
   * Calls to |restart_monitor-api| on an already running monitor take no effect.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-STOP-RESTART-EXAMPLE
   :end-before: //!
   :dedent: 8
