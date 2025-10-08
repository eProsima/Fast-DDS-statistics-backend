.. include:: ../exports/alias.include

.. _statistics_backend_set_alert:

Set entity alert
----------------

Function |set_alert-api| allows the user to set an alert on a specific entity.
When the alert is triggered, the listener callback *on_alert_triggered* will be called with the corresponding parameters.
In addition, if no entity matches the alert, the listener callback *on_alert_unmatched* will be called periodically

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-SET-ALERT-EXAMPLE
   :end-before: //!
   :dedent: 8
