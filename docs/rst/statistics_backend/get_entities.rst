.. include:: ../exports/alias.include

.. _statistics_backend_get_entities_all:

Get entities of a given kind
----------------------------

|get_entities-api| returns all the entities of a given |EntityKind-api|.
For example, it can retrieve all the |HOST-api| for which statistics are reported.

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-GET-ENTITIES-DEFAULT-EXAMPLE
    :end-before: //!
    :dedent: 8

.. _statistics_backend_get_entities:

This call to |get_entities-api| is the same as:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-GET-ENTITIES-ALL-EXAMPLE
    :end-before: //!
    :dedent: 8

Get entities of a given kind related to another entity
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

|get_entities-api| can also return all the entities of a given |EntityKind-api| that are related to another entity.
For example, it can retrieve all the |PARTICIPANT-api| running on a given |HOST-api|.

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-GET-ENTITIES-EXAMPLE
    :end-before: //!
    :dedent: 8

|get_entities-api| throws |BadParameter-api| in the following cases:

* if the |EntityKind-api| is |EntityKind_INVALID-api|
* if the |EntityId-api| does not reference an entity contained in the database or is not |EntityId:all-api|.
* if the |EntityKind-api| of the |EntityId-api| is |EntityKind_INVALID-api|

It returns the related entities according to the following table:

.. list-table:: Entity relations
   :header-rows: 1

   * - :class:`EntityId` \ :class:`EntityKind`
     - Host
     - User
     - Process
     - Domain
     - Topic
     - DomainParticipant
     - DataWriter
     - DataReader
     - Locator
   * - Host
     - Itself
     - Contains
     - Sub-contains
     - By DomainParticipant
     - By DomainParticipant
     - Sub-contains
     - Sub-contains
     - Sub-contains
     - Sub-contains
   * - User
     - Contained
     - Itself
     - Contains
     - By DomainParticipant
     - By DomainParticipant
     - Sub-contains
     - Sub-contains
     - Sub-contains
     - By Endpoints
   * - Process
     - Sub-contained
     - Contained
     - Itself
     - By DomainParticipant
     - By DomainParticipant
     - Contains
     - Sub-contains
     - Sub-contains
     - By Endpoints
   * - Domain
     - By DomainParticipant
     - By DomainParticipant
     - By DomainParticipant
     - Itself
     - Contains
     - Contains
     - Sub-contains
     - Sub-contains
     - By Endpoints
   * - Topic
     - By DomainParticipant
     - By DomainParticipant
     - By DomainParticipant
     - Contained
     - Itself
     - By Endpoints
     - Contains
     - Contains
     - By Endpoints
   * - DomainParticipant
     - Sub-contained
     - Sub-contained
     - Contained
     - Contained
     - By Endpoints
     - Itself
     - Contains
     - Contains
     - By Endpoints
   * - DataWriter
     - Sub-contained
     - Sub-contained
     - Sub-contained
     - Sub-contained
     - Contained
     - Contained
     - Itself
     - By topic
     - Contains
   * - DataReader
     - Sub-contained
     - Sub-contained
     - Sub-contained
     - Sub-contained
     - Contained
     - Contained
     - By topic
     - Itself
     - Contains
   * - Locator
     - Sub-contained
     - By Endpoints
     - By Endpoints
     - By Endpoints
     - By Endpoints
     - By Endpoints
     - Contained
     - Contained
     - Itself

* **Itself**: The result contains only the queried entity. For example, asking for all the |HOST-api| related to a
  given |HOST-api| returns that |HOST-api|.
* **Contains**: The result is the entities that the queried entity contains. For example, asking for all the
  |PARTICIPANT-api| related to a |PROCESS-api| returns all the |PARTICIPANT-api| that the |PROCESS-api| contains.
* **Sub-contains**: The result is the entities that the queried entity sub-contains. For example, asking for all the
  |DATAWRITER-api| related to a |USER-api| returns all the |DATAWRITER-api| contained in each of the
  |PARTICIPANT-api| in each of the |PROCESS-api| that the |USER-api| contains.
* **Contained**: The result is the entity that contains the queried entity. For example, asking for all the
  |TOPIC-api| related to a |DATAREADER-api| returns the |TOPIC-api| in which the |DATAREADER-api| is contained.
* **Sub-contained**: The result is the entity in which the queried entity is sub-contained. For example, asking for
  all the |HOST-api| related to a |PARTICIPANT-api| returns the |HOST-api| in which the |PARTICIPANT-api| is
  sub-contained.
* **By DomainParticipant**: The result is the entities related to the queried entity through the DomainParticipant.
  For example, asking for all the |HOST-api| related to a |DOMAIN-api| returns all the |HOST-api| that have a
  |PARTICIPANT-api| running on that |DOMAIN-api|.
* **By Endpoints**: The result is the entities related to the queried entity through the endpoints (|DATAREADER-api|
  and |DATAWRITER-api|). For example, asking for all the |LOCATOR-api| related to a |TOPIC-api| returns all the
  |LOCATOR-api| used by all the |DATAREADER-api| and |DATAWRITER-api| present in the |TOPIC-api|.
