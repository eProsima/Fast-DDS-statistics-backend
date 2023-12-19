Version 1.0.0
-----------

This release includes the following **improvement**:

* Regenerate types with Fast DDS-Gen v3.0.0.

This release includes the following **features**:
* Added status, app id and app metadata attributes for entities.
* Added database domain view graph.
* Added monitor service topic status data storing and processing.
* `StatisticsBackend::get_status` returns the entity status level of a given id.
* `StatisticsBackend::get_domain_view_graph` returns the domain view graph of a given domain.
* `StatisticsBackend::regenerate_domain_graph` regenerates the domain view graph of a given domain.
* `StatisticsBackend::get_status_data` returns an specific status data of a given id.
* Get physical information from discovery packets.

This release includes the following **dependencies update**:
|  | Repository | Old Version | New Version |
|---|---|---|---|
| Fast CDR | [eProsima/Fast-CDR](https://github.com/eProsima/Fast-CDR) | [v1.1.0](https://github.com/eProsima/Fast-CDR/releases/tag/v1.1.0) | [v2.1.1](https://github.com/eProsima/Fast-CDR/releases/tag/v2.1.1) |
| Fast DDS | [eProsima/Fast-DDS](https://github.com/eProsima/Fast-DDS) | [v2.11.0](https://github.com/eProsima/Fast-DDS/releases/tag/v2.11.0) | [v2.13.0](https://github.com/eProsima/Fast-DDS/releases/tag/v2.13.0) |
| Fast DDS Gen | [eProsima/Fast-DDS-Gen](https://github.com/eProsima/Fast-DDS-Gen) | [v2.5.1](https://github.com/eProsima/Fast-DDS-Gen/releases/tag/v2.5.1) | [v3.0.0](https://github.com/eProsima/Fast-DDS-Gen/releases/tag/v3.0.0) |
| IDL Parser | [eProsima/IDL-Parser](https://github.com/eProsima/IDL-Parser) | [v1.6.0](https://github.com/eProsima/IDL-Parser/releases/tag/v1.6.0) | [v1.7.2](https://github.com/eProsima/IDL-Parser/releases/tag/v1.7.2) |


Version 0.11.0
--------------

This release includes the following **improvement**:

* Regenerate TypeSupport with Fast DDS-Gen v2.5.1.

This release includes the following **bugfix**:

* Remove obsolete warning in documentation. Fast DDS v2.9.0 changed the default behavior by building with **FASTDDS_STATISTICS** enabled by default.

This release includes the following **dependencies update**:

|  | Repository | Old Version | New Version |
|---|---|---|---|
| Foonathan Memory Vendor | [eProsima/foonathan_memory_vendor](https://github.com/eProsima/foonathan_memory_vendor) | [v1.3.0](https://github.com/eProsima/foonathan_memory_vendor/releases/tag/v1.3.0) | [v1.3.1](https://github.com/eProsima/foonathan_memory_vendor/releases/tag/v1.3.1) |
| Fast CDR | [eProsima/Fast-CDR](https://github.com/eProsima/Fast-CDR) | [v1.0.27](https://github.com/eProsima/Fast-CDR/releases/tag/v1.0.27) | [v1.1.0](https://github.com/eProsima/Fast-CDR/releases/tag/v1.1.0) |
| Fast DDS | [eProsima/Fast-DDS](https://github.com/eProsima/Fast-DDS) | [v2.10.1](https://github.com/eProsima/Fast-DDS/releases/tag/v2.10.1) | [v2.11.0](https://github.com/eProsima/Fast-DDS/releases/tag/v2.11.0) |
| Fast DDS Gen | [eProsima/Fast-DDS-Gen](https://github.com/eProsima/Fast-DDS-Gen) | [v2.4.0](https://github.com/eProsima/Fast-DDS-Gen/releases/tag/v2.4.0) | [v2.5.1](https://github.com/eProsima/Fast-DDS-Gen/releases/tag/v2.5.1) |
| IDL Parser | [eProsima/IDL-Parser](https://github.com/eProsima/IDL-Parser) | [v1.5.0](https://github.com/eProsima/IDL-Parser/releases/tag/v1.5.0) | [v1.6.0](https://github.com/eProsima/IDL-Parser/releases/tag/v1.6.0) |

Version 0.10.0
--------------

This release includes the following **feature**:

* Extend method :code:`clear_statistics_data` to remove internal statistical data previous to a time given.

Version 0.9.0
-------------

This release includes the following **improvements**:

* Regenerate TypeSupport with Fast DDS-Gen v2.4.0.

This release includes the following **bugfixes**:

* Fix documentation dependencies security vulnerabilities.
* Install fixed gcovr version.
* Fix build issues adding ignored Info statuses.

Version 0.8.0
-------------

This release has the following **API extensions**:

* `StatisticsBackend::clear_statistics_data` clears the statistics data from every entity.
* `StatisticsBackend::clear_inactive_entities` removes every inactive entity from the database.

Version 0.7.0
-------------

This release adds the following **feature**:

* Possibility of loading Fast DDS Statistics Backend's `DomainParticipant` from Fast DDS XML configuration files.

This release adds the following **improvements**:

* Warn the user the Fast DDS Statistics module needs to be enabled.
* Update documentation regarding `locator` specification.
* Fix destruction error in the Database's queue thread.

Version 0.6.0
-------------

This release adds the following **improvement**:

* Update statistics type support including physical data in `DISCOVERY_TOPIC`

Version 0.5.0
-------------

This release adds the following **improvements**

* Improvements on Windows CI
* Refactor on processing queues to avoid data races with entity creation order
* Update Statistics Module type support

Version 0.4.0
-------------

This release has the following **API extensions**:

* `StatisticsBackend::is_metatraffic` allows the user to know if a specific topic or endpoint is related to metatraffic data.

This release adds the following **features**:

* Add HelloWorld Example.

This release includes the following **bug fixes and improvements**:

* Return the end of the time interval as the data point's timestamps instead of the initial one.
* Avoid adding the same locator twice to the database.
* Fix deadlock when accessing the database within a callback implementation.

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
