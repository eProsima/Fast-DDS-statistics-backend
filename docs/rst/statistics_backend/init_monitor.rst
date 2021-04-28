.. include:: ../exports/alias.include

.. _statistics_backend_init:

Initialize a monitor
--------------------

.. warning::
    This feature is currently not supported.
    It will be implemented on a future release of *Fast DDS Statistics Backend*.

|StatisticsBackend-api| provides two overloads of |init_monitor-api| that can be used to start a monitorization on a
DDS domain or a *Fast DDS* Discovery Server network.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-INIT-MONITOR-EXAMPLE
   :end-before: //!
   :dedent: 8

Furthermore, it is possible to initialize a monitor with a custom |DomainListener-api|.
Please refer to :ref:`listeners_domain_listener` for more information about the ``DomainListener`` and its
functionality.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-INIT-MONITOR-LISTENER-EXAMPLE
   :end-before: //!
   :dedent: 8

In addition, |init_monitor-api| allows for specifying which monitorization events should be notified.
This is done by setting a |CallbackMask-api| where the active callbacks from the listener are specified.
Moreover, a mask on statistics data kind of interest can be set creating a |DataKindMask-api|

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-INIT-MONITOR-MASKS-EXAMPLE
   :end-before: //!
   :dedent: 8
