.. include:: ../exports/alias.include

.. _types_notifier:

Notifier
========

Alerts can have notification mechanisms attached, so that when they are triggered,
an action is performed. The ``Notifier`` class represents these notification mechanisms.

In the current implementation, the only available notifiers are script notifiers,
that execute a user-defined script when the alert is triggered. The script path is passed as
a parameter in the ´set_alert´ method.




