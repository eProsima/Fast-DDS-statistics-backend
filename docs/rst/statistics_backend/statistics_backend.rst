.. include:: ../exports/alias.include

.. _statistics_backend:

StatisticsBackend
=================

The |StatisticsBackend-api| singleton is the entry point for applications that gather statistics information about
a *Fast DDS* network using the *Fast DDS* Statistics module.
It has the API to start and stop monitorizations on a given domain or *Fast DDS* Discovery Server
network, and the functions to extract statistics information from those monitorizations.

*Fast DDS Statistics Backend* can monitor several DDS domains and *Fast DDS* Discovery Server networks at the same time.
It notifies applications about changes in the network and the arrival of new statistics data through two listeners,
which contain a set of callbacks that the application implements. It can also watch for specific conditions on the
monitored data by configuring alerts (see :ref:`statistics_backend_set_alert`), which trigger a listener callback
or run a notifier script when the condition they were set for is met.

.. toctree::

    /rst/statistics_backend/init_monitor
    /rst/statistics_backend/stop_restart
    /rst/statistics_backend/clear
    /rst/statistics_backend/reset
    /rst/statistics_backend/set_listeners
    /rst/statistics_backend/get_domain_view_graph
    /rst/statistics_backend/get_info
    /rst/statistics_backend/get_entities
    /rst/statistics_backend/get_data
    /rst/statistics_backend/get_status_data
    /rst/statistics_backend/get_status
    /rst/statistics_backend/get_type
    /rst/statistics_backend/topic_spy
    /rst/statistics_backend/set_alias
    /rst/statistics_backend/is_active
    /rst/statistics_backend/is_metatraffic
    /rst/statistics_backend/dump_load
    /rst/statistics_backend/set_alert
