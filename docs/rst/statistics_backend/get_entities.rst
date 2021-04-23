.. include:: ../exports/alias.include

.. _statistics_backend_get_entities_all:

Get entities of a given kind
----------------------------

.. warning::
    This feature is currently not supported.
    It will be implemented on a future release of *Fast DDS Statistics Backend*.

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

This function returns the related entities according to the following table:

.. list-table:: Entity relations
   :header-rows: 1

   * - :class:`EntityId` / :class:`EntityKind`
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
     - By DomainParticipant
   * - User
     - Contained
     - Itself
     - Contains
     - By Participant
     - By Participant
     - Subcontains
     - Subcontains
     - Subcontains
     - By Participant
   * - Process
     - Subcontained
     - Contained
     - Itself
     - By Participant
     - By Participant
     - Contains
     - Subcontains
     - Subcontains
     - By Participant
   * - Domain
     - By Participant
     - By Participant
     - By Participant
     - Itself
     - Contains
     - Contains
     - Subcontains
     - Subcontains
     - By Participant
   * - Topic
     - By Participant
     - By Participant
     - By Participant
     - Contained
     - Itself
     - By Endpoints
     - Contains
     - Contains
     - By Endpoints
   * - DParticipant
     - Subcontained
     - Subcontained
     - Contained
     - Contained
     - By Endpoints
     - Itself
     - Contains
     - Contains
     - By Endpoints
   * - DataWriter
     - Subcontained
     - Subcontained
     - Subcontained
     - Subcontained
     - Contained
     - Contained
     - Itself
     - By topic
     - Contains
   * - DataReader
     - Subcontained
     - Subcontained
     - Subcontained
     - Subcontained
     - Contained
     - Contained
     - By topic
     - Itself
     - Contains
   * - Locator
     - Subcontained
     - By Endpoints
     - By Endpoints
     - By Endpoints
     - By Endpoints
     - By Endpoints
     - Contained
     - Contained
     - Itself
