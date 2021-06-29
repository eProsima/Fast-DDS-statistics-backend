.. include:: ../exports/alias.include

.. _statistics_backend_reset:

Reset Fast DDS Statistics Backend
---------------------------------

If the user needs to restart *Fast DDS Statistics Backend* returning to the initial conditions, |reset-api| is provided.
Calling this method clears all the data collected since the first monitor was initialized, erases all monitors (not
being available for restarting afterwards), and removes the physical listener
(see :ref:`statistics_backend_set_listeners` for more information).
In order to call |reset-api|, all monitors have to be stopped (inactive).
Otherwise it throws |PreconditionNotMet-api|.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-RESET-EXAMPLE
   :end-before: //!
   :dedent: 8
