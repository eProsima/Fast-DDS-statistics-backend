.. _release_notes:

Version 0.8.0
=============

This release includes the following **features**:

1. New API to clear statistic data and remove inactive entities from database.

This release includes the following **improvements**:

1. CI improvements:

    1. Include address-sanitizer job.
    2. Flaky tests are run in a specific job.

1. Internal implementation improvements:

    1. Remove database unused collections.
    2. Smart pointers refactor using unique instead of shared pointers.

1. Example:

    1. Improve example including new API.

This release includes the following **bugfixes**:

1. Memory leaks fixes reported by address-sanitizer.

Previous versions
=================

.. include:: previous_versions/v0.7.1.rst
.. include:: previous_versions/v0.7.0.rst
.. include:: previous_versions/v0.6.0.rst
.. include:: previous_versions/v0.5.0.rst
.. include:: previous_versions/v0.4.0.rst
.. include:: previous_versions/v0.3.0.rst
.. include:: previous_versions/v0.2.0.rst
.. include:: previous_versions/v0.1.0.rst
