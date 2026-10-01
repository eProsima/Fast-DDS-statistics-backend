.. include:: ../exports/alias.include

.. _statistics_backend_reset:

Reset Fast DDS Statistics Backend
---------------------------------

|reset-api| restarts *Fast DDS Statistics Backend*, reverting it to its default, freshly-started state:

* All the data in the database is erased.
* All monitors are removed and cannot be restarted afterwards.
* The physical listener is removed (see :ref:`statistics_backend_set_listeners`).
* The physical listener's callback mask is reset to ``CallbackMask::none()``.
* The physical listener's data mask is reset to ``DataKindMask::none()``.

To call |reset-api|, all monitors must be stopped (inactive).
Otherwise it throws |PreconditionNotMet-api|.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-RESET-EXAMPLE
   :end-before: //!
   :dedent: 8
