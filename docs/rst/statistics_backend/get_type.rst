.. include:: ../exports/alias.include

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
