:orphan:

.. TODO uncomment when there are forthcoming notes
.. .. include:: forthcoming_version.rst

###################
Version 3.0.0 |Pro|
###################

This release includes the following **internal changes**:

* Preparation of Monitor Pro repository
* License update from GPL-3.0 to eProsima Software License Agreement Rev 03
* Creation of self-hosted CI and mirror

This release includes the following **bugfixes**:

* Fix race condition in backend data access
* Several spy mode stability fixes, including QoS compatibility and XML profile initialization
* Fix incorrect enum initialization causing undefined entity discovery behavior
* Fix endpoint undiscovery handling
* Fix Windows deadlock in the alerts timeout checker
* Fix UserDataReaderListener test callback signature

This release includes the following **features**:

* Publisher pane backend API to publish DDS data from a dockable monitor panel
* Image pane backend API to display fixed-format image data from DDS topics
* License check API for automatic license validation at startup
* Multi-domain alias synchronization refreshing all domain graphs when a shared entity alias is updated
* Topic spy with source timestamps via extended start_topic_spy() API
* Safety check to prevent creating a monitor when proxy entities are present
* Database access optimized to query each DataKind once, reducing redundant lookups
* EntityKind forwarded as a function argument for faster data retrieval

This release includes the following **dependencies**:

.. list-table::
    :header-rows: 1

    *   -
        - Repository
        - Version
    *   - Fast CDR
        - `eProsima/Fast-CDR <https://github.com/eProsima/Fast-CDR>`__
        - `v2.3.6 <https://github.com/eProsima/Fast-CDR/releases/tag/v2.3.6>`__
    *   - Fast DDS |Pro|
        - `eProsima/Fast-DDS-Pro <https://github.com/eProsima/Fast-DDS-Pro>`__
        - `v3.6.1.1 <https://github.com/eProsima/Fast-DDS-Pro/releases/tag/v3.6.1.1>`__
    *   - Fast DDS Gen
        - `eProsima/Fast-DDS-Gen <https://github.com/eProsima/Fast-DDS-Gen>`__
        - `v4.3.0 <https://github.com/eProsima/Fast-DDS-Gen/releases/tag/v4.3.0>`__
    *   - IDL Parser
        - `eProsima/IDL-Parser <https://github.com/eProsima/IDL-Parser.git>`__
        - `v4.3.0 <https://github.com/eProsima/IDL-Parser/releases/tag/v4.3.0>`__


##############
Version 2.5.0
##############

This release includes the following **improvements and bugfixes**:

* Fix Windows deadlock in check_alerts_timeouts()
* Hotfix/undiscovery endpoints
* Fix Spy DataReader created with incompatible QoS
* Fix failure of Spy mode when statistics are enabled on XML
* Initialize `enums` correctly in EntityDiscoveryInfo
* Fix monitor subscriber partition issue with *
* Add undiscovery of proxy entities
* Allow statistics topics to be spied

This release includes the following **updates**:

* Update types for Fast DDS Gen v4.3.0 Release

This release includes the following CI improvements:

* Upgrade `Linters` CI runner image to `ubuntu-24.04`

This release includes the following **dependencies update**:

.. list-table::
    :header-rows: 1

    *   -
        - Repository
        - Old Version
        - New Version
    *   - Fast CDR
        - `eProsima/Fast-CDR <https://github.com/eProsima/Fast-CDR>`__
        - `v2.3.4 <https://github.com/eProsima/Fast-CDR/releases/tag/v2.3.4>`__
        - `v2.3.5 <https://github.com/eProsima/Fast-CDR/releases/tag/v2.3.5>`__
    *   - Fast DDS
        - `eProsima/Fast-DDS <https://github.com/eProsima/Fast-DDS>`__
        - `v3.4.1 <https://github.com/eProsima/Fast-DDS/releases/tag/v3.4.1>`__
        - `v3.6.1 <https://github.com/eProsima/Fast-DDS/releases/tag/v3.6.1>`__
    *   - Fast DDS Gen
        - `eProsima/Fast-DDS-Gen <https://github.com/eProsima/Fast-DDS-Gen>`__
        - `v4.2.0 <https://github.com/eProsima/Fast-DDS-Gen/releases/tag/v4.2.0>`__
        - `v4.3.0 <https://github.com/eProsima/Fast-DDS-Gen/releases/tag/v4.3.0>`__
    *   - IDL Parser
        - `eProsima/IDL-Parser <https://github.com/eProsima/IDL-Parser.git>`__
        - `v4.2.0 <https://github.com/eProsima/IDL-Parser/releases/tag/v4.2.0>`__
        - `v4.3.0 <https://github.com/eProsima/IDL-Parser/releases/tag/v4.3.0>`__


Previous versions
=================

.. include:: previous_versions/v2.4.0.rst
.. include:: previous_versions/v2.3.0.rst
.. include:: previous_versions/v2.2.0.rst
.. include:: previous_versions/v2.1.0.rst
.. include:: previous_versions/v2.0.0.rst
.. include:: previous_versions/v1.1.0.rst
.. include:: previous_versions/v1.0.0.rst
.. include:: previous_versions/v0.11.0.rst
.. include:: previous_versions/v0.10.0.rst
.. include:: previous_versions/v0.9.0.rst
.. include:: previous_versions/v0.8.0.rst
.. include:: previous_versions/v0.7.1.rst
.. include:: previous_versions/v0.7.0.rst
.. include:: previous_versions/v0.6.0.rst
.. include:: previous_versions/v0.5.0.rst
.. include:: previous_versions/v0.4.0.rst
.. include:: previous_versions/v0.3.0.rst
.. include:: previous_versions/v0.2.0.rst
.. include:: previous_versions/v0.1.0.rst
