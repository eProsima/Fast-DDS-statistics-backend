.. include:: ../exports/alias.include

.. _statistics_backend_is_active:

Check whether an entity is active
---------------------------------

*Fast DDS Statistics Backend* keeps the statistical data record of
all the entities that have at some point been detected by a monitor.
However, it is possible that some of this entities have already abandoned the network, thus becoming inactive.
For this reason, |StatisticsBackend-api| exposes a |is_active-api| function that returns whether an entity is active,
given its |EntityId-api|.

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-IS-ACTIVE-EXAMPLE
    :end-before: //!
    :dedent: 8
