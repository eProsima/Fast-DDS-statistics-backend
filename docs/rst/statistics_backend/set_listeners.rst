.. include:: ../exports/alias.include

.. _statistics_backend_set_listeners:

Set listeners
-------------

.. warning::
    This feature is currently not supported.
    It will be implemented on a future release of *Fast DDS Statistics Backend*.

As explained in :ref:`listeners`, each *Fast DDS Statistics Backend* monitor has two listeners:

* |PhysicalListener-api|: Registers events about changes in the physical aspects of the communication (hosts, users,
  processes, and locators).
* |DomainListener-api|: Registers events about changes in the DDS network (domain, participants, topics, data readers,
  and data writers).

Since the physical aspects of the communication can be shared across different DDS domains and *Fast DDS* Discovery
Server networks, only one ``PhysicalListener`` can be set for the entire application.

.. important::
    Even though the |PhysicalListener-api| can be set at any time, it is recommended to set it prior to initializing any
    monitoring, so that no physical events are missed.

Furthermore, it is possible to change the |DomainListener-api|, |CallbackMask-api|, and |DataKindMask-api| of any
monitor at any time.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-SET-LISTENERS-EXAMPLE
   :end-before: //!
   :dedent: 8
