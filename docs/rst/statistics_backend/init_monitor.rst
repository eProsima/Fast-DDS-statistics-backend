.. include:: ../exports/alias.include

.. _statistics_backend_init:

Initialize a monitor
--------------------


Initializing a monitor on a certain Domain ID makes *eProsima Fast DDS Statistics Backend* start monitoring the statistics data and entity discoveries on that domain.
No statistics data will be gathered unless there is a monitor initialized in the required domain.

|StatisticsBackend-api| provides several overloads of |init_monitor-api| that can be used to start a monitorization on a DDS domain or a *Fast DDS* Discovery Server network.

Additionally, it is possible to initialize a monitor using an XML profile with the new |init_monitor_with_profile-api| method.
This allows the monitor to be configured according to the settings defined in the XML profile, providing greater flexibility and integration with existing Fast DDS XML configuration workflows.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-INIT-MONITOR-EXAMPLE
   :end-before: //!
   :dedent: 8


The following example demonstrates how to initialize a monitor using an XML profile:

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-INIT-MONITOR-XML-PROFILE-EXAMPLE
   :end-before: //!
   :dedent: 8


Furthermore, it is possible to initialize a monitor with a custom |DomainListener-api|.
Please refer to :ref:`listeners_domain_listener` for more information about the ``DomainListener`` and its functionality.

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


Similarly, when initializing a monitor with an XML profile, you can also specify a custom |DomainListener-api| and callback masks as needed.

|init_monitor-api| throws exceptions in the following cases:

* |BadParameter-api| if a monitor is already created for the given DDS domain or *Fast DDS* Discovery Server network.
* |Error-api| if the creation of the monitor fails
