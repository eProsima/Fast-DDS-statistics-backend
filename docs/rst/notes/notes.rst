.. _release_notes:

Version 0.4.0
=============

This release has the following **API extensions**:

* `StatisticsBackend::is_metatraffic` allows the user to know if a specific topic or endpoint is related to metatraffic
data.
* 

This release adds the following **features**:

* Add HelloWorld Example.

This release includes the following **bug fixes and improvements**:

* Return the end of the time interval as the data point's timestamps instead of the initial one.
* Avoid adding the same locator twice to the database.
* Fix deadlock when accessing the database within a callback implementation.

Previous versions
=================

.. include:: previous_versions/v0.1.0.rst
.. include:: previous_versions/v0.2.0.rst
.. include:: previous_versions/v0.3.0.rst
