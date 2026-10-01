.. include:: ../exports/alias.include

.. _statistics_backend_clear:

Clearing data
-------------

*eProsima Fast DDS Statistics Backend* monitors both the entities discovered in a certain DDS domain or *Fast DDS*
Discovery Server network, and the statistic data related to these entities.
|StatisticsBackend-api| provides several methods to clear the data contained in the internal database:

* |clear_statistics_data-api| commands the deletion of old statistics data contained within the database.
  The timestamp refers to the time from where to keep data.
  Use :code:`the_end_of_time()` to remove all data efficiently (used by default).
* |clear_inactive_entities-api| deletes from the database those :ref:`entities <types_entity_kind>` that are no longer
  alive and communicating (see :ref:`statistics_backend_is_active` for more information).

|clear_monitor-api| is not implemented in the open-source edition: calling it has no effect.

.. todo::
  * |clear_monitor-api| clears all data (entities and statistics) related to a specific monitor.
  * It will be implemented on a future release of *Fast DDS Statistics Backend*.

.. note::
   |clear_monitor-api| is fully implemented in *Fast DDS Statistics Backend Pro*: it stops the monitor
   automatically if it is still active, and it fully
   removes the domain and all of its entities - not only their statistical data - so the same domain can be
   monitored again from scratch with |init_monitor-api|.

.. literalinclude:: /code/StatisticsBackendTests.cpp
   :language: c++
   :start-after: //CONF-CLEAR-EXAMPLE
   :end-before: //!
   :dedent: 8
