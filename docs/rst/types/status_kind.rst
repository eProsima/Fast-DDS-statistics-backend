.. include:: ../exports/alias.include

.. _types_status_kind:

StatusKind
==========

The *eProsima Fast DDS Statistics Backend* records entity status data of different kinds, such as incompatible QoS
or the number of lost samples, as provided by the Monitor Service from *eProsima Fast DDS Statistics Module*. The
kind of each status data value is its |StatusKind-api|.

- |PROXY-api|: Collection of parameters describing the proxy data of that entity.

- |CONNECTION_LIST-api|: List of connections used by this entity. Each element is a connection whose
  connection mode is one of:

  - Intraprocess
  - Data sharing
  - Transport

  It also includes the announced locators and the locator in use with each of the matched entities.

- |INCOMPATIBLE_QOS-api|: Status of the incompatible QoS of that entity.

  - |DATAWRITER-api| Incompatible QoS Offered.
  - |DATAREADER-api| Incompatible QoS Requested.

- |EXTENDED_INCOMPATIBLE_QOS-api|: Current incompatible QoS policies of a |DATAWRITER-api| or
  |DATAREADER-api| with each remote entity it is incompatible with, instead of the entity-wide summary of
  |INCOMPATIBLE_QOS-api|.

.. todo::
  - |INCONSISTENT_TOPIC-api|: Status of inconsistent topics of the topic of that entity. Asked to the topic of the
    requested entity.

- |LIVELINESS_LOST-api|: Number of times that a |DATAWRITER-api| lost liveliness.

- |LIVELINESS_CHANGED-api|: Number of times that the liveliness status changed in a |DATAREADER-api|.

- |DEADLINE_MISSED-api|: Number of missed deadlines registered in that entity.

- |SAMPLE_LOST-api|: Number of times that this entity lost samples.

.. todo::
  - |INCONSISTENT_TOPIC-api| status data not supported yet.

Only |PARTICIPANT-api|, |DATAWRITER-api| and |DATAREADER-api| have associated status data. The table lists the
|StatusKind-api| values each of these :ref:`entities<types_entity_kind>` has:

+---------------------------------+-------------------+------------------+------------------+
| StatusKind                      | |PARTICIPANT-api| | |DATAWRITER-api| | |DATAREADER-api| |
+=================================+===================+==================+==================+
| |PROXY-api|                     | Yes               | Yes              | Yes              |
+---------------------------------+-------------------+------------------+------------------+
| |CONNECTION_LIST-api|           | Yes               | Yes              | Yes              |
+---------------------------------+-------------------+------------------+------------------+
| |INCOMPATIBLE_QOS-api|          | No                | Yes              | Yes              |
+---------------------------------+-------------------+------------------+------------------+
| |EXTENDED_INCOMPATIBLE_QOS-api| | No                | Yes              | Yes              |
+---------------------------------+-------------------+------------------+------------------+
| |LIVELINESS_LOST-api|           | No                | Yes              | No               |
+---------------------------------+-------------------+------------------+------------------+
| |LIVELINESS_CHANGED-api|        | No                | No               | Yes              |
+---------------------------------+-------------------+------------------+------------------+
| |DEADLINE_MISSED-api|           | No                | Yes              | Yes              |
+---------------------------------+-------------------+------------------+------------------+
| |SAMPLE_LOST-api|               | No                | No               | Yes              |
+---------------------------------+-------------------+------------------+------------------+

.. .. todo::
..   | |INCONSISTENT_TOPIC-api|      | No              | Yes            | Yes            |
..   +-------------------------------+-----------------+----------------+----------------+

Each |StatusKind-api| has an associated |StatusLevel-api|, which is |OK-api| when the monitor service message
reports no problem.
An entity's |StatusLevel-api| is derived from all its status data. The table lists the
|StatusLevel-api| values associated with each |StatusKind-api|:

+---------------------------------+------------------------+
| StatusKind                      | StatusLevel's          |
+=================================+========================+
| |PROXY-api|                     | |OK-api|               |
+---------------------------------+------------------------+
| |CONNECTION_LIST-api|           | |OK-api|               |
+---------------------------------+------------------------+
| |INCOMPATIBLE_QOS-api|          | |OK-api|/|ERROR-api|   |
+---------------------------------+------------------------+
| |EXTENDED_INCOMPATIBLE_QOS-api| | |OK-api|/|ERROR-api|   |
+---------------------------------+------------------------+
| |LIVELINESS_LOST-api|           | |OK-api|/|WARNING-api| |
+---------------------------------+------------------------+
| |LIVELINESS_CHANGED-api|        | |OK-api|               |
+---------------------------------+------------------------+
| |DEADLINE_MISSED-api|           | |OK-api|/|WARNING-api| |
+---------------------------------+------------------------+
| |SAMPLE_LOST-api|               | |OK-api|/|WARNING-api| |
+---------------------------------+------------------------+

.. .. todo::
..   | |INCONSISTENT_TOPIC-api|      | \-                    |
..   +-------------------------------+-----------------------+

.. note::

  For entity transitions, |WARNING-api| status level takes precedence over |OK-api|, and |ERROR-api| takes
  precedence over both |WARNING-api| and |OK-api|.
