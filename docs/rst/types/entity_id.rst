.. include:: ../exports/alias.include

.. _types_entityid:

EntityId
========

When monitoring a domain (see :ref:`statistics_backend_init`), *Fast DDS Statistics Backend* labels each discovered
entity with an |EntityId-api| identifier that is unique within the |StatisticsBackend-api| instance.
The application uses this |EntityId-api|, among other things, to query statistical data from the backend (see
:ref:`statistics_backend_get_data`).
|EntityId-api| also exposes some commonly used operations:

.. _types_entityid_all:

EntityId wildcard
-----------------

|EntityId-api| can return an ID that represents all the `EntityIds`:

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //ENTITYID-ALL-EXAMPLE
   :end-before: //!
   :dedent: 8

.. _types_entityid_invalid:

Invalid EntityId
----------------

|EntityId-api| can return an invalid ID:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //ENTITYID-INVALID-EXAMPLE
    :end-before: //!
    :dedent: 8

.. _types_entityid_invalidate:

Invalidate an EntityId
----------------------

An |EntityId-api| can be invalidated:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //ENTITYID-INVALIDATE-EXAMPLE
    :end-before: //!
    :dedent: 8

.. _types_entityid_valid:

Check validity of an EntityId
-----------------------------

To check whether an |EntityId-api| is valid:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //ENTITYID-VALID-EXAMPLE
    :end-before: //!
    :dedent: 8

.. _types_entityid_is_all:

Check EntityId represents all Entities
--------------------------------------

To check whether an |EntityId-api| represents all the `EntityIds`:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //ENTITYID-IS_ALL-EXAMPLE
    :end-before: //!
    :dedent: 8

.. _types_entityid_valid_and_unique:

Check validity and uniqueness of an EntityId
--------------------------------------------

To check whether an |EntityId-api| is valid and unique:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //ENTITYID-VALID_AND_UNIQUE-EXAMPLE
    :end-before: //!
    :dedent: 8

.. _types_entityid_comparison:

Comparison operations
---------------------

|EntityIds-api| can be compared with each other:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //ENTITYID-COMPARE-EXAMPLE
    :lines: 1-3,5,7,9,11,13-
    :end-before: //!
    :dedent: 8

.. _types_entityid_ostream:

Output to OStream
-----------------

|EntityIds-api| can be output to :class:`std::ostream`:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //ENTITYID-OSTREAM-EXAMPLE
    :end-before: //!
    :dedent: 8
