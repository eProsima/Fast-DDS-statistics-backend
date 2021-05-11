.. include:: ../exports/alias.include

.. _types:

Types
=====

.. _types_data_kind:

DataKind
--------

.. _types_statistic_kind:

StatisticKind
-------------

.. _types_entity_kind:

EntityKind
----------

.. _types_entityid:

EntityId
--------

When monitoring a domain (see :ref:`statistics_backend_init`), *Fast DDS Statistics Backend* labels all the different
discovered entities with an |EntityId-api| identifier that is unique in the context of the |StatisticsBackend-api|
instance.
This |EntityId-api| is used by the application, among other things, to query statistical data to the backend (see
:ref:`statistics_backend_get_data`).
To ease the use of the *Fast DDS Statistics Backend* API, |EntityId-api| exposes certain commonly used operations:

.. _types_entityid_all:

EntityId wildcard
^^^^^^^^^^^^^^^^^

|EntityId-api| allows for retrieving an ID that represents all the `EntityIds`:

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //ENTITYID-ALL-EXAMPLE
   :end-before: //!
   :dedent: 8

.. _types_entityid_invalid:

Invalid EntityId
^^^^^^^^^^^^^^^^

|EntityId-api| allows for retrieving an invalid ID:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //ENTITYID-INVALID-EXAMPLE
    :end-before: //!
    :dedent: 8

.. _types_entityid_invalidate:

Invalidate an EntityId
^^^^^^^^^^^^^^^^^^^^^^

It is also possible to invalidate an |EntityId-api|:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //ENTITYID-INVALIDATE-EXAMPLE
    :end-before: //!
    :dedent: 8

.. _types_entityid_valid:

Check validity of an EntityId
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

It can be checked whether an |EntityId-api| is valid:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //ENTITYID-VALID-EXAMPLE
    :end-before: //!
    :dedent: 8

.. _types_entityid_comparison:

Comparison operations
^^^^^^^^^^^^^^^^^^^^^

|EntityIds-api| can be compared between them:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //ENTITYID-COMPARE-EXAMPLE
    :lines: 1-3,5,7,9,11,13-
    :end-before: //!
    :dedent: 8

.. _types_entityid_ostream:

Output to OStream
^^^^^^^^^^^^^^^^^

|EntityIds-api| can be output to :class:`std::ostream`:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //ENTITYID-OSTREAM-EXAMPLE
    :end-before: //!
    :dedent: 8
