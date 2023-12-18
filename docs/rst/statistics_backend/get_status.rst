.. include:: ../exports/alias.include

.. _statistics_backend_get_status:

Get entity status
-----------------

It is also possible to retrieve the |StatusLevel-api| of an entity given its |EntityId-api|:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-GET-STATUS-EXAMPLE
    :end-before: //!
    :dedent: 8

|get_status-api| throws |BadParameter-api| if there is no entity with the given ID.
