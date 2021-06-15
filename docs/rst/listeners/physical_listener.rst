.. include:: ../exports/alias.include

.. _listeners_physical_listener:

PhysicalListener
================

|PhysicalListener-api| is an abstract class defining the callbacks
that will be triggered in response to changes in the physical aspects
of the communication (hosts, users, processes, and locators)
By default, all these callbacks are empty and do nothing.
The user should implement a specialization of this class overriding the callbacks
that are needed on the application.
Callbacks that are not overridden will maintain their empty implementation.

PhysicalListener defines the following callbacks:

* |PhysicalListener::on_host_discovery-api|:
  A new host has been discovered in the monitored network.
  Hosts are never *undiscovered*.
  The arguments in the callback specifies the ID of the participant
  that discovered the host.

* |PhysicalListener::on_user_discovery-api|:
  A new user has been discovered in the monitored network.
  Users are never *undiscovered*.
  The arguments in the callback specifies the ID of the participant
  that discovered the user.

* |PhysicalListener::on_process_discovery-api|:
  A new process has been discovered in the monitored network.
  Processes are never *undiscovered*.
  The arguments in the callback specifies the ID of the participant
  that discovered the process.

* |PhysicalListener::on_locator_discovery-api|:
  A new locator has been discovered in the monitored network.
  Locators are never *undiscovered*.
  The arguments in the callback specifies the ID of the participant
  that discovered the locator.
