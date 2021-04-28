.. include:: ../exports/alias.include

.. _statistics_backend_get_graph:

Get entities' graph
-------------------

.. warning::
    This feature is currently not supported.
    It will be implemented on a future release of *Fast DDS Statistics Backend*.
    Furthermore, the graph structure may change.

*Fast DDS Statistics Backend* allows to retrieve the entire graph of entities for which the singleton holds statistics
data.
The result of this query is a |Graph-api| tree structure that contains the |EntityId-api| and names of each entity.
To be able to understand and interpret this tree, it is required to know about all the available entities and the inner
relations between them.
Following, there is a diagram of the relation between the *Fast DDS Statistics Backend* entities, and how are they
divided into physical and domain related.
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

    Host name: "example_host"
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
