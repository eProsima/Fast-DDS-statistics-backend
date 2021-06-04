.. include:: ../exports/alias.include

.. _statistics_backend_get_entities_all:

Get entities of a given kind
----------------------------

The |StatisticsBackend-api| singleton can be queried about all the entities of a given |EntityKind-api|.
For example, |get_entities-api| function can be used to retrieve all the |HOST-api| for which statistics are reported.

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-GET-ENTITIES-ALL-EXAMPLE
    :end-before: //!
    :dedent: 8

.. _statistics_backend_get_entities:

Get entities of a given kind related to another entity
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The |StatisticsBackend-api| singleton can be queried about all the entities of a given |EntityKind-api| that are related
to any entity.
For example, |get_entities-api| function can be used to retrieve all the |PARTICIPANT-api| running on a given
|HOST-api|.

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-GET-ENTITIES-EXAMPLE
    :end-before: //!
    :dedent: 8

Throws eprosima::statistics_backend::BadParameter if there is no entity with the given ID and is not Entity::all().

This function returns the related entities according to the following table:

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

* **Itself**: Means that the return will only contain the entity by which the query is performed, i.e. when asking for
  all the |HOST-api| related to a given |HOST-api|, the return will simply be the |HOST-api| itself.
* **Contains**: The returned entities will be the ones that the entity by which the query is performed contains, i.e.
  when asking for all the |PARTICIPANT-api| related to a |PROCESS-api|, the return will be all the
  |PARTICIPANT-api| that the |PROCESS-api| contains.
* **Sub-contains**: The returned entities will be the ones that the entity by which the query is performed sub-contains,
  i.e. when asking for all the |DataWriter-api| related to a |USER-api|, the return will be all the |DataWriter-api|
  that are contained in each of the |PARTICIPANT-api| in each of the |PROCESS-api| that the |USER-api| contains.
* **Contained**: The returned entity will be that one in which the entity by which the query is performed is contained,
  i.e. when asking for all the |TOPIC-api| related to a |DataReader-api|, the return will be the |TOPIC-api| in which
  the |DataReader-api| is contained.
* **Sub-contained**: The returned entity will be the one in which the entity by which the query is performed is
  sub-contained, i.e. when asking for all the |HOST-api| related to a |PARTICIPANT-api|, the return will be the
  |HOST-api| in which the |PARTICIPANT-api| is sub-contained.
* **By DomainParticipant**: The returned entities will be the ones that are related to the entity by which the query is
  performed through the DomainParticipant, i.e. when asking for all the |HOST-api| related to a |DOMAIN-api|, the result
  will be all the |HOST-api| that have a |PARTICIPANT-api| running on said |DOMAIN-api|.
* **By Endpoints**: The returned entities will be the ones that are related to the entity by which the query is
  performed through the endpoints (|DataReader-api| and |DataWriter-api|), i.e. when asking for all the |LOCATOR-api|
  related to a |TOPIC-api|, the result will be all the |LOCATOR-api| that are used by all the |DataReader-api| and
  |DataWriter-api| present in the |TOPIC-api|.
