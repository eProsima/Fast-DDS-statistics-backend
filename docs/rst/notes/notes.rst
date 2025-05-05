.. _release_notes:

Version 2.2.0
#############

This release includes the following **features**:

* Add EntityId getters and delete Endpoint kind prefixes.
* Add support for Extended Incompatible QoS.
* Calculate the DDS vendor and add it to the participant info.
* Expose `get_entity_by_guid` function.
* Apply demangling to ROS 2 topics in IDL view.
* Support ROS2 Easy Mode.

This release includes the following **improvements and bugfixes**:

* Delete backend debug traces in release mode.
* Reject old samples.
* Add expiration check-in in `fragile_ptr`.
* Update IDL when a new type is received.

This release includes the following **updates**:

* Update README and GitHub actions.

This release includes the following **dependencies update**:

.. list-table::
    :header-rows: 1

    *   -
        - Repository
        - Old Version
        - New Version
    *   - Fast CDR
        - `eProsima/Fast-CDR <https://github.com/eProsima/Fast-CDR>`__
        - `v2.2.5 <https://github.com/eProsima/Fast-CDR/releases/tag/v2.2.5>`__
        - `v2.3.0 <https://github.com/eProsima/Fast-CDR/releases/tag/v2.3.0>`__
    *   - Fast DDS
        - `eProsima/Fast-DDS <https://github.com/eProsima/Fast-DDS>`__
        - `v3.1.0 <https://github.com/eProsima/Fast-DDS/releases/tag/v3.1.0>`__
        - `v3.2.2 <https://github.com/eProsima/Fast-DDS/releases/tag/v3.2.2>`__
    *   - Fast DDS Gen
        - `eProsima/Fast-DDS-Gen <https://github.com/eProsima/Fast-DDS-Gen>`__
        - `v4.0.2 <https://github.com/eProsima/Fast-DDS-Gen/releases/tag/v4.0.2>`__
        - `v4.0.4 <https://github.com/eProsima/Fast-DDS-Gen/releases/tag/v4.0.4>`__
    *   - IDL Parser
        - `eProsima/IDL-Parser <https://github.com/eProsima/IDL-Parser.git>`__
        - `v4.0.2 <https://github.com/eProsima/IDL-Parser/releases/tag/v4.0.2>`__
        - `v4.0.4 <https://github.com/eProsima/IDL-Parser/releases/tag/v4.0.4>`__

Previous versions
#################

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
