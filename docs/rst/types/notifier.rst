.. include:: ../exports/alias.include

.. _types_notifier:

Notifier
========

Alerts can have notification mechanisms attached, which perform an action
when the alert is triggered. The ``Notifier`` class represents these notification mechanisms.

Currently, the only available notifiers are script notifiers,
which execute a user-defined script when the alert is triggered. The script path is passed as
a parameter of |set_alert-api|.




