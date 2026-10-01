.. include:: ../exports/alias.include

.. _listeners_physical_listener:

PhysicalListener
================

|PhysicalListener-api| is an abstract class defining the callbacks
triggered in response to changes in the physical aspects
of the communication (hosts, users, processes, and locators).
By default, all these callbacks are empty and do nothing.
The user implements a specialization of this class that overrides the callbacks
the application needs. Callbacks that are not overridden keep their empty implementation.

PhysicalListener defines the following callbacks:

* |PhysicalListener::on_host_discovery-api|:
  A new host has been discovered in the monitored network.
  Hosts are never *undiscovered*.
  The callback argument is the ID of the discovered host.

* |PhysicalListener::on_user_discovery-api|:
  A new user has been discovered in the monitored network.
  Users are never *undiscovered*.
  The callback argument is the ID of the discovered user.

* |PhysicalListener::on_process_discovery-api|:
  A new process has been discovered in the monitored network.
  Processes are never *undiscovered*.
  The callback argument is the ID of the discovered process.

* |PhysicalListener::on_locator_discovery-api|:
  A new locator has been discovered in the monitored network.
  Locators are never *undiscovered*.
  The callback argument is the ID of the discovered locator.
