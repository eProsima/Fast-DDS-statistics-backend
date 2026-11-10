.. include:: ../exports/alias.include

.. _statistics_backend_get_domain_view_graph:

Get entities domain view graph
-------------------------------

|get_domain_view_graph-api| retrieves the entire graph of active entities for which the singleton holds
statistics data.
The result is a |Graph-api| tree structure that contains the info of each entity.
Interpreting this tree requires knowing the available entities and the relations between them.
The diagram below shows how the *Fast DDS Statistics Backend* entities relate to each other, and how they are
divided into physical and domain related.
For more information about the different |EntityKind-api|, see :ref:`types_entity_kind`.

.. figure:: /rst/figures/internal_db.svg
    :align: center

    *Fast DDS Statistics Backend* entity relations and their division into physical and domain related.

.. _statistics_backend_get_domain_view_graph_example:

Example
^^^^^^^

The |DomainListener::on_domain_view_graph_update-api| |DomainListener-api| callback notifies when a domain has updated
its graph. Alternatively, the graph can be regenerated manually by calling |regenerate_domain_graph-api|, which
returns ``true`` if a graph for the given domain existed and was regenerated (which also triggers
|DomainListener::on_domain_view_graph_update-api|), or ``false`` if no graph was found for that domain:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-REGENERATE-GRAPH-EXAMPLE
    :end-before: //!
    :dedent: 8

|get_domain_view_graph-api| throws |BadParameter-api| if there is no graph for the specified domain id.

The following example uses a simple scenario: two processes, each running one participant on the same domain,
one with a data reader and the other with a data writer (both in the same topic).
This means that there is only one |USER-api| within a single |HOST-api|.
The application retrieves the network graph as follows:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-GET-GRAPH-EXAMPLE
    :end-before: //!
    :dedent: 8

In this example, the previous call returns a |Graph-api| object similar to the following:

.. todo::

    Provide the `app_metadata` fields in the json.

.. literalinclude:: /code/graph_example.json
    :language: JSON

Then, the application can extract information about the entities from the graph as shown below:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-NAVIGATE-GRAPH-EXAMPLE
    :end-before: //!
    :dedent: 8

Running the previous snippet on this example outputs:

.. code-block:: text

    Domain: 0
        Host alias: "example_host_alias"
        Host status: "OK"
            User alias: "example_user_alias"
            User status: "OK"
                Process alias: "example_process1_alias"
                Process PID:  "1234"
                Process status: "OK"
                    Participant alias: "shapes_demo_participant_1_alias"
                    Participant app_id:  "SHAPES_DEMO"
                    Participant status: "OK"
                        Endpoint alias: "shapes_demo_datawriter_alias"
                        Endpoint kind:  "datawriter"
                        Endpoint app_id:  "SHAPES_DEMO"
                        Endpoint status: "OK"
                Process alias: "example_process2_alias"
                Process PID:  "1235"
                Process status: "OK"
                    Participant alias: "shapes_demo_participant_2_alias"
                    Participant app_id:  "SHAPES_DEMO"
                    Participant status: "OK"
                        Endpoint alias: "shapes_demo_datareader_alias"
                        Endpoint kind:  "datareader"
                        Endpoint app_id:  "SHAPES_DEMO"
                        Endpoint status: "OK"
        Topic alias: "Square"
        Topic metatraffic: false


For the operations available on ``Graph`` objects, see |Graph-api|.
