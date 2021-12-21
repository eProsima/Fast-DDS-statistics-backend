.. include:: ../exports/alias.include

.. _statistics_backend:

StatisticsBackend
=================

Singleton |StatisticsBackend-api| is the entry point for applications that want to gather statistics information about
a *Fast DDS* network using *Fast DDS* Statistics module.
It provides the API necessary for starting and stopping monitorizations on a given domain or *Fast DDS* Discovery Server
network, as well as the functions to extract statistics information about said monitorizations.

*Fast DDS Statistics Backend* can monitor several DDS domains and *Fast DDS* Discovery Server networks at the same time,
notifying applications about changes in the network and arrival of new statistics data using two listeners which
contain a set of callbacks that the application implements.

.. toctree::

    /rst/statistics_backend/init_monitor
    /rst/statistics_backend/stop_restart
    /rst/statistics_backend/clear
    /rst/statistics_backend/reset
    /rst/statistics_backend/set_listeners
    /rst/statistics_backend/get_graph
    /rst/statistics_backend/get_info
    /rst/statistics_backend/get_entities
    /rst/statistics_backend/get_data
    /rst/statistics_backend/get_type
    /rst/statistics_backend/set_alias
    /rst/statistics_backend/is_active
    /rst/statistics_backend/is_metatraffic
    /rst/statistics_backend/dump_load
