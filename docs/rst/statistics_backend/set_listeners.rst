.. include:: ../exports/alias.include

.. _statistics_backend_set_listeners:

Set listeners
-------------

As explained in :ref:`listeners`, each *Fast DDS Statistics Backend* monitor has two listeners:

* |PhysicalListener-api|: Registers events about changes in the physical aspects of the communication (hosts, users,
  processes, and locators).
* |DomainListener-api|: Registers events about changes in the DDS network (domain, participants, topics, data readers,
  and data writers).

Since the physical aspects of the communication can be shared across different DDS domains and *Fast DDS* Discovery
Server networks, only one ``PhysicalListener`` can be set for the entire application.

.. important::
    The |PhysicalListener-api| can be set at any time, but it is recommended to set it before initializing any
    monitoring, so that no physical events are missed.

The |DomainListener-api|, |CallbackMask-api|, and |DataKindMask-api| of any monitor can be changed
at any time. Setting a new |DomainListener-api| (or |PhysicalListener-api|) replaces any listener already
configured; listeners do not stack. The listener pointer can also be ``nullptr``, which removes the
currently-configured listener for that monitor (or, for |PhysicalListener-api|, for the whole application)
without installing a new one.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-SET-LISTENERS-EXAMPLE
   :end-before: //!
   :dedent: 8

|set_domain_listener-api| throws |BadParameter-api| if the given monitor ID is not yet registered.
