.. include:: ../exports/alias.include

.. _statistics_backend_get_domain_view_graph:

Get entities' domain view graph
-------------------------------

*Fast DDS Statistics Backend* allows to retrieve the entire graph of active entities for which the singleton holds
statistics data.
The result of this query is a |Graph-api| tree structure that contains the info of each entity.
To be able to understand and interpret this tree, it is required to know about all the available entities and the inner
relations between them.
Following, there is a diagram of the relation between the *Fast DDS Statistics Backend* entities, and how are they
divided into physical and domain related.
For more information about the different |EntityKind-api| please refer to :ref:`types_entity_kind`.

.. figure:: /rst/figures/internal_db.svg
    :align: center

    *Fast DDS Statistics Backend* entity relations and their division into physical and domain related.

The |DomainListener::on_domain_view_graph_update-api| |DomainListener-api| callback notifies when a domain has updated
its graph. Alternatively, the graph can be regenerated manually by calling |regenerate_domain_graph-api|:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-REGENERATE-GRAPH-EXAMPLE
    :end-before: //!
    :dedent: 8

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

Then, the application can extract information about the entities from the graph as shown below:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-NAVIGATE-GRAPH-EXAMPLE
    :end-before: //!
    :dedent: 8

Running the previous snippet on the given example should output:

.. code-block:: text

    Domain: 0
        Host alias: "example_host_alias"
        Host status: "OK"
            User alias: "example_user_alias"
            User status: "OK"
                Process alias: "example_process1_alias"
                Process PID:  "1234"
                Process status: "OK"
                    Participant alias: "example_participant1_alias"
                    Participant app_id:  "UNKNOWN_APP"
                    Participant status: "OK"
                        Endpoint alias: "example_publisher_alias"
                        Endpoint kind:  "datawriter"
                        Endpoint status: "OK"
                Process alias: "example_process2_alias"
                Process PID:  "1235"
                Process status: "OK"
                    Participant alias: "example_participant2_alias"
                    Participant app_id:  "UNKNOWN_APP"
                    Participant status: "OK"
                        Endpoint alias: "example_subscriber_alias"
                        Endpoint kind:  "datareader"
                        Endpoint status: "OK"
        Topic alias: "HelloWorld"
        Topic metatraffic: false


For more information about the operations available with ``Graph`` objects, please refer to |Graph-api|.
