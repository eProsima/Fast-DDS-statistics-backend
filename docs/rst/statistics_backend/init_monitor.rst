.. include:: ../exports/alias.include

.. _statistics_backend_init:

Initialize a monitor
--------------------


Initializing a monitor on a certain Domain ID makes *eProsima Fast DDS Statistics Backend* start monitoring the statistics data and entity discoveries on that domain.
No statistics data is gathered unless there is a monitor initialized in the required domain.

|StatisticsBackend-api| has several overloads of |init_monitor-api| to start a monitorization on a DDS domain or a *Fast DDS* Discovery Server network.

|init_monitor_with_profile-api| initializes a monitor from an XML profile, configuring it with the settings defined in that profile.
This fits into existing Fast DDS XML configuration workflows.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-INIT-MONITOR-EXAMPLE
   :end-before: //!
   :dedent: 8

The example above shows both overloads: monitoring a DDS domain directly by its :cpp:type:`DomainId
<eprosima::statistics_backend::DomainId>`, and monitoring the network of a *Fast DDS* Discovery Server by its
locators instead. Each locator in the Discovery Server overload must follow the format ``kind:[IP]:port``, where
``kind`` is one of ``UDPv4``, ``TCPv4``, ``UDPv6`` or ``TCPv6``; several locators are given as a
semicolon-separated list.

.. warning::
   Shared-memory (SHM) locators are not supported by this overload. For a server that has also been configured
   with SHM locators, initialize the monitor using only its non-shared-memory locators.

All three |init_monitor-api| overloads (domain, Discovery Server locators, and |init_monitor_with_profile-api|)
also accept an ``app_id`` and ``app_metadata`` pair identifying the monitor's own participant. These
appear, for instance, as the discovered monitor participant's metadata in the output of |get_domain_view_graph-api|.
Only the domain-based overload also accepts an ``easy_mode_ip``, the IP address of the remote Discovery
Server used when the monitored domain relies on ROS 2 Easy Mode.

The following example initializes a monitor using an XML profile:

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-INIT-MONITOR-XML-PROFILE-EXAMPLE
   :end-before: //!
   :dedent: 8


A monitor can also be initialized with a custom |DomainListener-api|.
For more information about the ``DomainListener``, see :ref:`listeners_domain_listener`.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-INIT-MONITOR-LISTENER-EXAMPLE
   :end-before: //!
   :dedent: 8


|init_monitor-api| can also specify which monitorization events are notified, through a |CallbackMask-api| that sets
the active callbacks of the listener.
A mask on the statistics data kinds of interest can also be set with a |DataKindMask-api|.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-INIT-MONITOR-MASKS-EXAMPLE
   :end-before: //!
   :dedent: 8


A custom |DomainListener-api| and callback masks can also be specified when initializing a monitor with an XML profile.

|init_monitor-api| throws exceptions in the following cases:

* |BadParameter-api| if a monitor is already created for the given DDS domain or *Fast DDS* Discovery Server network.
* |Error-api| if the creation of the monitor fails
