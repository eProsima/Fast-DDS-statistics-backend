.. include:: exports/alias.include

.. _statistics_backend:

StatisticsBackend
=================

Singleton |StatisticsBackend-api| is the entry point for applications that want to gather statistics information about
a *Fast DDS* network using *Fast DDS* Statistics module.
It provides the API necessary for starting and stopping monitorizations on a given domain or *Fast DDS* Discovery Server
network, as well as the functions to extract statistics information about said monitorizations.

*Fast DDS Statistics Backend* can monitor several DDS domains and *Fast DDS* Discovery Server network at the same time,
notifying applications about changes in the network and arrival of new statistics data using two listeners which
contain a set of callback that the application implements.

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

In addition, |init_monitor-api| allows for specifying which monitorization event should be notified.
This is done by setting a |CallbackMask-api| where the active callback from the listener are specified.
Moreover, a mask on statistics data kind of interest can be set creating a |DataKindMask-api|

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-INIT-MONITOR-MASKS-EXAMPLE
   :end-before: //!
   :dedent: 8

.. _statistics_backend_stop_restart:

Stop and restart a monitor
--------------------------

.. warning::
    This feature is currently not supported.
    It will be implemented on a future release of *Fast DDS Statistics Backend*.

*Fast DDS Statistics Backend* allows for a monitorization to be stopped and restarted at any time.
Stopping a monitorization merely means that the internal statistics DataReaders are disabled, but the already received
data is still accessible to applications through the query API (see :ref:`statistics_backend_get_data`).
Is is important to note that:

* Calls to |stop_monitor-api| on an already stopped monitor take no effect.
* Calls to |restart_monitor-api| on an already running monitor take no effect.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-STOP-RESTART-EXAMPLE
   :end-before: //!
   :dedent: 8


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

.. _statistics_backend_get_data:

Retrieve statistical data
-------------------------

.. warning::
    This feature is currently not supported.
    It will be implemented on a future release of *Fast DDS Statistics Backend*.

*Fast DDS Statistics Backend* provides to overloads of |get_data-api| to retrieve statistical data of a given
|DataKind-api| within a time frame.
The result of this operation is a set of |StatisticsData-api| elements of length equal to the number of specified bins,
which have an equal size of :math:`(t_to - t_from)/(# of bins)`.
Each of this bins contains the result of applying a given |StatisticKind-api| to all the the data points in it.

Depending on the |DataKind-api|, the data is related to one or two entities, e.g. |FASTDDS_LATENCY-api| measures the
latency between a write operation on the data writer side and notification to the user when the data is available in
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

.. _statistics_backend_get_graph:

Get entities' graph
-------------------

.. warning::
    This feature is currently not supported.
    It will be implemented on a future release of *Fast DDS Statistics Backend*.

*Fast DDS Statistics Backend* allows to retrieve the entire graph of entities for which the singleton holds statistics
data.
The result of this query is a |Graph-api| tree structure that contains the |EntityId-api| and names of each entity.
To be able to understand and interpret this tree, it is required to know about all the available entities and the inner
relations between them.
Following, there is a diagram of the relation between the *Fast DDS Statistics Backend*, and how are they divided into
physical and domain related.
For more information about the different |EntityKind-api| please refer to :ref:`types_entity_kind`.

.. figure:: /rst/figures/internal_db.svg
    :align: center

    *Fast DDS Statistics Backend* entity relations and their division into physical and domain related.

For the following example, a simple scenario is considered, where there is one process running two participants on the
same domain; one with a data reader and the other one with a data writer (both in the same topic).
This means that there is only one |USER-api| within a single |HOST-api|.
The application can retrieve the network graph by:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-GET-GRAPH-EXAMPLE
    :end-before: //!
    :dedent: 8

In this example, the previous call would return a |Graph-api| object similar to the following:

.. literalinclude:: /code/graph_example.json
    :language: JSON

Then, the application can extract information from the graph as shown below:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-NAVIGATE-GRAPH-EXAMPLE
    :end-before: //!
    :dedent: 8

Running the previous snippet on the given example should output:

.. code-block:: text

    User name: "example_user"
        Process name: "example_process_1"
        Process PID:  "9564"
            Participant name: "participant_1"
            Participant GUID: "01.0f.22.cd.59.64.04.00.02.00.00.00|00.00.01.c1"
                Datawriter name: "datawriter_1"
                Datawriter GUID: "01.0f.22.cd.59.64.04.00.02.00.00.00|00.00.01.03"
        Process name: "example_process_2"
        Process PID:  "9565"
            Participant name: "participant_2"
            Participant GUID: "01.0f.22.cd.59.64.04.00.05.00.00.00|00.00.01.c1"
                Datareader name: "datareader_1"
                Datareader GUID: "01.0f.22.cd.59.64.04.00.05.00.00.00|00.00.01.04"

For more information about the operations available with ``Graph`` objects, please refer to |Graph-api|.

.. _statistics_backend_get_entities:

Get entities of a given kind related to another entity
------------------------------------------------------

.. warning::
    This feature is currently not supported.
    It will be implemented on a future release of *Fast DDS Statistics Backend*.

The |StatisticsBackend-api| singleton can be queried about all the entities of a given |EntityKind-api| that are related
to any entity. For example, |get_entities-api| function can be used to retrieve all the |PARTICIPANT-api| running on a
given |HOST-api|.

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-GET-ENTITIES-EXAMPLE
    :end-before: //!
    :dedent: 8

.. _statistics_backend_get_qos:

Get entity QoS
--------------

.. warning::
    This feature is currently not supported.
    It will be implemented on a future release of *Fast DDS Statistics Backend*.

*Fast DDS Statistics Backend* includes the possibility of retrieving the QoS settings of the DDS entities present in the
network.
This operation returns a |Qos-api| object.

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-GET-QOS-EXAMPLE
    :end-before: //!
    :dedent: 8

.. important::

    |get_qos-api| can only be called for entities which |EntityKind-api| is |PARTICIPANT-api|, |DATAWRITER-api|, or
    |DATAREADER-api|.

.. _statistics_backend_get_qos_participant:

Participant QoS example
^^^^^^^^^^^^^^^^^^^^^^^

.. literalinclude:: /code/participant_qos_example.json
    :language: JSON

.. _statistics_backend_get_qos_datareader:

DataReader QoS example
^^^^^^^^^^^^^^^^^^^^^^

.. literalinclude:: /code/datareader_qos_example.json
    :language: JSON

.. _statistics_backend_get_qos_datawriter:

DataWriter QoS example
^^^^^^^^^^^^^^^^^^^^^^

.. literalinclude:: /code/datawriter_qos_example.json
    :language: JSON

.. _statistics_backend_get_name:

Get entity name
---------------

.. warning::
    This feature is currently not supported.
    It will be implemented on a future release of *Fast DDS Statistics Backend*.

All the *Fast DDS Statistics Backend* entities have a name.
Such name can be retrieved given the |EntityId-api| of the entity.

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-GET-NAME-EXAMPLE
    :end-before: //!
    :dedent: 8

.. warning::
    The entity names are not guaranteed to be unique.
    For a unique identifier for the entity, please use its |EntityId-api|.

.. _statistics_backend_get_type:

Get entity type
---------------

.. warning::
    This feature is currently not supported.
    It will be implemented on a future release of *Fast DDS Statistics Backend*.

Is is also possible to retrieve the |EntityKind-api| of an entity given its |EntityId-api|:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-GET-TYPE-EXAMPLE
    :end-before: //!
    :dedent: 8


.. _statistics_backend_is_active:

Check whether an entity is active
---------------------------------

.. warning::
    This feature is currently not supported.
    It will be implemented on a future release of *Fast DDS Statistics Backend*.

Unless |clear_monitor-api| is called for a monitor, *Fast DDS Statistics Backend* keeps the statistical data record of
all the entities that have at some point been detected by a monitor.
However, it is possible that some of this entities have already abandoned the network, thus becoming inactive.
For this reason, |StatisticsBackend-api| exposes a |is_active-api| function that returns whether an entity is active,
given its |EntityId-api|.

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-IS-ACTIVE-EXAMPLE
    :end-before: //!
    :dedent: 8
