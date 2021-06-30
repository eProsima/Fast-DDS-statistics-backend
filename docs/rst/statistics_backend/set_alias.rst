.. include:: ../exports/alias.include

.. _statistics_backend_set_alias:

Set entity alias
----------------

Even though the *Fast DDS Statistics Backend* provides a ``name`` for each entity, this default name can be lengthy and
not really self-explanatory and user-friendly.
Therefore, |set_alias-api| allows the user to apply any alias to the desired entity so it can be easily identified.
If the entity provided does not exist |set_alias-api| throws |BadParameter-api|.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-SET-ALIAS-EXAMPLE
   :end-before: //!
   :dedent: 8
