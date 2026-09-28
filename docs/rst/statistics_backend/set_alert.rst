.. include:: ../exports/alias.include

.. _statistics_backend_set_alert:

Set entity alert
----------------

Function |set_alert-api| allows the user to set an alert that watches for a condition on a specific entity or
group of entities. Two kinds of alert are available (see :ref:`types_alert_kind`): |AlertKind::NEW_DATA-api|,
which triggers as soon as new data is received, and |AlertKind::NO_DATA-api|, which triggers when no data has
been received for a certain time.

* ``alert_name``: a name for the alert, used for identification purposes only.
* ``domain_id``: the |EntityId-api| of the |DOMAIN-api| entity whose data the alert watches (not a raw numeric
  DDS domain id - obtain it, for instance, from |get_entities-api|).
* ``host_name`` / ``user_name`` / ``topic_name``: filters that select which entities the alert applies to. An
  empty string, or the literal string ``"ALL"``, acts as a wildcard for that filter.
* ``alert_kind``: which of the two alert kinds to create.
* ``threshold``: the value the monitored data is compared against.

  .. important::
     ``threshold`` is only honored for |AlertKind::NO_DATA-api| alerts, which trigger when the monitored value
     drops *below* it. For |AlertKind::NEW_DATA-api| alerts, ``threshold`` is ignored entirely - the alert
     always triggers as soon as any new value greater than ``0.0`` is received, regardless of what is passed
     here.

* ``t_between_triggers``: minimum time that must elapse between two consecutive triggers of the same alert, so a
  noisy condition does not fire repeatedly.
* ``alert_timeout``: for a |AlertKind::NO_DATA-api| alert, how long without matching data must elapse before the
  alert is considered timed out.
* ``script_path``: optional path to an executable script that will be run (as a notifier) every time the alert
  is triggered.

When the alert is triggered, the |DomainListener::on_alert_triggered-api| callback of the domain's
|DomainListener-api| is called with the corresponding parameters. For a |AlertKind::NO_DATA-api| alert, if no
matching data has been received for ``alert_timeout``, the |DomainListener::on_alert_timeout-api| callback is
called instead; how often this timeout condition is (re-)evaluated is controlled by |set_alerts_polling_time-api|,
independently of ``t_between_triggers``.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-SET-ALERT-EXAMPLE
   :end-before: //!
   :dedent: 8

|set_alert-api| itself returns nothing: the created alert's |AlertId-api| is obtained afterwards, for example via
|get_alerts-api| or the alert-scoped overload of |get_info-api| (see :ref:`statistics_backend_get_info`). To
remove the alert, the user can call |remove_alert-api| with that id.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-REMOVE-ALERT-EXAMPLE
   :end-before: //!
   :dedent: 8

