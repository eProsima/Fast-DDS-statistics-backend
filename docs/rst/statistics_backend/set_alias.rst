.. include:: ../exports/alias.include

.. _statistics_backend_set_alias:

Set entity alias
----------------

*Fast DDS Statistics Backend* gives each entity a ``name``, but this default name can be long and hard to understand.
|set_alias-api| applies any alias to an entity so it is easy to identify.
If the entity does not exist, |set_alias-api| throws |BadParameter-api|.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-SET-ALIAS-EXAMPLE
   :end-before: //!
   :dedent: 8
