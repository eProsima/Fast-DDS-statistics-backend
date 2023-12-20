.. include:: ../exports/alias.include

.. _statistics_backend_get_status_data:

Get status data
---------------

*Fast DDS Statistics Backend* provides a template of |get_status_data-api| to retrieve monitor service status data
sample of a given |StatusKind-api| (for more information about all the reported |StatusKind-api|, please refer to
:ref:`types_status_kind`).

The sample is passed as an argument to the function  along with the |EntityId-api| of the entity whose status is to
be known. This sample is populated with the most recent status data of that kind.

Only |PARTICIPANT-api|, |DATAWRITER-api| and |DATAREADER-api| have associated status data. The following table
describes which |StatusKind-api| each of these :ref:`entities<types_entity_kind>` has:

+-------------------------------+-----------------+----------------+----------------+
|StatusKind                     ||PARTICIPANT-api|||DATAWRITER-api|||DATAREADER-api||
+===============================+=================+================+================+
| |PROXY-api|                   | Yes             | Yes            | Yes            |
+-------------------------------+-----------------+----------------+----------------+
| |CONNECTION_LIST-api|         | Yes             | Yes            | Yes            |
+-------------------------------+-----------------+----------------+----------------+
| |INCOMPATIBLE_QOS-api|        | No              | Yes            | Yes            |
+-------------------------------+-----------------+----------------+----------------+
| |INCONSISTENT_TOPIC-api|      | No              | Yes            | Yes            |
+-------------------------------+-----------------+----------------+----------------+
| |LIVELINESS_LOST-api|         | No              | Yes            | No             |
+-------------------------------+-----------------+----------------+----------------+
| |LIVELINESS_CHANGED-api|      | No              | No             | Yes            |
+-------------------------------+-----------------+----------------+----------------+
| |DEADLINE_MISSED-api|         | No              | Yes            | Yes            |
+-------------------------------+-----------------+----------------+----------------+
| |SAMPLE_LOST-api|             | No              | No             | Yes            |
+-------------------------------+-----------------+----------------+----------------+

.. note::

  For entity transitions, |WARNING-api| status level takes precedence over |OK-api| level, and |ERROR-api| does over
  |WARNING-api| and |OK-api| levels.


|get_status_data-api| throws |BadParameter-api| in the following cases:

- If the |EntityId-api| does not reference a Entity contained in the database.
- If there is no specialization template for the requested |StatusKind-api|.
- If the |EntityKind-api| of the Entity doesn't have the associated |StatusKind-api|.

Every time new status data is available there will be a callback to Domain Listener's
|DomainListener::on_status_reported-api| (for more information about |DomainListener-api| callbacks, please refer to
:ref:`listeners_domain_listener`).

.. _statistics_backend_get_status_data_examples:

Examples
^^^^^^^^

Following, some example queries are provided to serve a inspiration for applications using
*Fast DDS Statistics Backend*.

Proxy example
""""""""""""""""""""""""""""""""""""""""""""

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-GET-STATUS-DATA-PROXY
   :end-before: //!
   :dedent: 8

Connection List example
"""""""""""""""""""""""""""""""""""""

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-GET-STATUS-DATA-CONNECTION-LIST
   :end-before: //!
   :dedent: 8

Incompatible QoS example
"""""""""""""""""""""""""""""""""""""""

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-GET-STATUS-DATA-INCOMPATIBLE-QOS
   :end-before: //!
   :dedent: 8

Inconsistent Topic example
""""""""""""""""""""""""""""""""""""""""""""""""

.. warning::
  |INCONSISTENT_TOPIC-api| status data not supported yet.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-GET-STATUS-DATA-INCONSISTENT-TOPIC
   :end-before: //!
   :dedent: 8

Liveliness Lost example
"""""""""""""""""""""""""""""""""""""""

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-GET-STATUS-DATA-LIVELINESS-LOST
   :end-before: //!
   :dedent: 8

Liveliness Changed example
"""""""""""""""""""""""""""""""""""""""

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-GET-STATUS-DATA-LIVELINESS-CHANGED
   :end-before: //!
   :dedent: 8

Deadline Missed example
"""""""""""""""""""""""""""""""""""""""

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-GET-STATUS-DATA-DEADLINE-MISSED
   :end-before: //!
   :dedent: 8

Sample Lost example
"""""""""""""""""""""""""""""""""""""""

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-GET-STATUS-DATA-SAMPLE-LOST
   :end-before: //!
   :dedent: 8
