.. include:: ../exports/alias.include

.. _statistics_backend_reset:

Reset Fast DDS Statistics Backend
---------------------------------

If the user needs to restart *Fast DDS Statistics Backend* returning to the initial conditions, |reset-api| is provided.
Calling this method reverts the backend to its default, freshly-started state:

* All the data in the database is erased.
* All monitors are removed and cannot be restarted afterwards.
* The physical listener is removed (see :ref:`statistics_backend_set_listeners` for more information).
* The physical listener's callback mask is reset to ``CallbackMask::none()``.
* The physical listener's data mask is reset to ``DataMask::none()``.

In order to call |reset-api|, all monitors have to be stopped (inactive).
Otherwise it throws |PreconditionNotMet-api|.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-RESET-EXAMPLE
   :end-before: //!
   :dedent: 8
