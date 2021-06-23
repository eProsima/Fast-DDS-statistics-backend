.. include:: ../exports/alias.include

.. _statistics_backend_get_type:

Get entity type
---------------

It is also possible to retrieve the |EntityKind-api| of an entity given its |EntityId-api|:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-GET-TYPE-EXAMPLE
    :end-before: //!
    :dedent: 8

|get_type-api| throws |BadParameter-api| if there is no entity with the given ID.
