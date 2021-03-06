Forthcoming
-----------

Version 0.3.0
-------------

This release has the following **API breaks**:

* `StatisticsBackend::dump_database` methods now have an additional argument `clear`.

This release adds the following **features**:

* Dumping the database can optionally delete the traffic data from the internal data structures
  after the dump is completed, in order to reduce memory footprint.
* Support to create monitors on discovery server networks.
* Statistics data related to meta-traffic are now collected under the builtin metaraffic endpoint
  created on each participant.
* Entities removed from the network now have a *non-active* status.
* Transitions between active and non-active status are notified to the user listeners.
* It is possible to change the domain listener and mask after the monitor is created.

This release includes the following **bug fixes and improvements**:

* By default, statistics data is received using UDP transport, shared memory is disabled.
* Network latency data now relates to a source participant and a destination locator
  (previously a source locator and a destination locator).
* Statistics data can now trigger the discovery of a new locator.
* Improved entity names and aliases to be more user friendly.
* Solved an issue that may cause the internal database to freeze.
* Allow for topics with the same name to be on different domains.

Version 0.2.0
-------------

This minor release is API compatible with the previous minor release, but introduces **ABI breaks**:

* Methods and attributes have been added on several classes, so indexes of
  symbols on dynamic libraries may have changed.

This release adds the following **features**:

* Support for Windows platforms
* Dumped data can now be loaded to the backend
* Backend can now be reset to a clean state (deleting all the data and monitors)
* StatisticsBackend::get_data implementation for SAMPLE_DATAS and DISCOVERY_TIME sample types
* StatisticsBackend::get_data implementation for zero bins

It also includes the following **improvements**:

* The participant info now contains all the locators of the participant
* Entities have an alias that can be set by the user to facilitate identification

Some important **bugfixes** are also included:

* Fixed errors when the same topic name is used on different domains
* Fixed crashes on database queues when database operation fails

Version 0.1.0
-------------

This first release includes the following features:

* Starting and stopping monitoring a DDS domain ID
* Keeping track of discovered entities (hosts, users, processes, participants,
  topics, data readers, data writers, and locators).
* Listening and recording statistics data related to the discovered entities,
  as reported by the *Fast DDS* statistics module.
* Retrieving the recorded statistics data, preprocessed with the requested statistic
  (mean, standard deviation, maximum, minimum, median)
* Persisting the recorded statistics to a file.
* Loading the statistics from a persisted file.
