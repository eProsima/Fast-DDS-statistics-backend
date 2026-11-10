.. include:: ../exports/alias.include

.. _types_alertid:

AlertId
=======

Global identifier of an alert, equivalent to an unsigned integer.

|set_alert-api| returns nothing, so an alert's |AlertId-api| is obtained afterwards from |get_alerts-api| (which
lists every configured alert) or from the alert-scoped overload of |get_info-api|. The |AlertId-api| is then used to
remove the alert with |remove_alert-api| or to query its configuration with |get_info-api| (see
:ref:`types_alertinfo`).
