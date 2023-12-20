.. include:: ../exports/alias.include

.. _types_status_kind:

StatusData
==========

The *eProsima Fast DDS Statistics Backend* records entities status data of different nature, as provided by the
Monitor Service from *eProsima Fast DDS Statistics Module*, e.g., incompatible QoS or the number of lost samples. We
refer to the nature of these status data values as their |StatusKind-api|.

- |PROXY-api|: Collection of parameters describing the proxy data of that entity.

- |CONNECTION_LIST-api|: List of connections used by this entity. Each of the elements is a connection where
  the possible values for the connection mode are:

  - Intraprocess
  - Data sharing
  - Transport

  In addition, information comprising the announced locators and locator in use with each one of the matched entities
  is also included.

- |INCOMPATIBLE_QOS-api|: Status of the incompatible QoS of that entity.

  - |DATAWRITER-api| Incompatible QoS Offered.
  - |DATAREADER-api| Incompatible QoS Requested.

.. todo::
  - |INCONSISTENT_TOPIC-api|: Status of inconsistent topics of the topic of that entity. Asked to the topic of the
    requested entity.

- |LIVELINESS_LOST-api|: Tracks the status of the number of times that liveliness was lost by a |DATAWRITER-api|.

- |LIVELINESS_CHANGED-api|: Tracks the status of the number of times that liveliness status changed in a
  |DATAREADER-api|.

- |DEADLINE_MISSED-api|: The status of the number of missed deadlines registered in that entity.

- |SAMPLE_LOST-api|: Tracks the number of times that this entity lost samples.

.. todo::
  - |INCONSISTENT_TOPIC-api| status data not supported yet.

Only |PARTICIPANT-api|, |DATAWRITER-api| and |DATAREADER-api| have associated status data. The following table
describes which |StatusKind-api| each of these :ref:`entities<types_entity_kind>` has:

+-------------------------------+-----------------+----------------+----------------+
| StatusKind                    ||PARTICIPANT-api|||DATAWRITER-api|||DATAREADER-api||
+===============================+=================+================+================+
| |PROXY-api|                   | Yes             | Yes            | Yes            |
+-------------------------------+-----------------+----------------+----------------+
| |CONNECTION_LIST-api|         | Yes             | Yes            | Yes            |
+-------------------------------+-----------------+----------------+----------------+
| |INCOMPATIBLE_QOS-api|        | No              | Yes            | Yes            |
+-------------------------------+-----------------+----------------+----------------+
| |LIVELINESS_LOST-api|         | No              | Yes            | No             |
+-------------------------------+-----------------+----------------+----------------+
| |LIVELINESS_CHANGED-api|      | No              | No             | Yes            |
+-------------------------------+-----------------+----------------+----------------+
| |DEADLINE_MISSED-api|         | No              | Yes            | Yes            |
+-------------------------------+-----------------+----------------+----------------+
| |SAMPLE_LOST-api|             | No              | No             | Yes            |
+-------------------------------+-----------------+----------------+----------------+

.. .. todo::
..   | |INCONSISTENT_TOPIC-api|      | No              | Yes            | Yes            |
..   +-------------------------------+-----------------+----------------+----------------+

Each |StatusKind-api| has an associated |StatusLevel-api|. |OK-api| status is obtained when the monitor service message
reports no problem.
Entity's associated |StatusLevel-api| is obtained from all status data. The following table describes which
|StatusLevel-api|'s are associated with each |StatusKind-api|:

+-------------------------------+-----------------------+
| StatusKind                    |StatusLevel's          |
+===============================+=======================+
| |PROXY-api|                   | |OK-api|              |
+-------------------------------+-----------------------+
| |CONNECTION_LIST-api|         | |OK-api|              |
+-------------------------------+-----------------------+
| |INCOMPATIBLE_QOS-api|        | |OK-api|/|ERROR-api|  |
+-------------------------------+-----------------------+
| |LIVELINESS_LOST-api|         | |OK-api|/|WARNING-api||
+-------------------------------+-----------------------+
| |LIVELINESS_CHANGED-api|      | |OK-api|              |
+-------------------------------+-----------------------+
| |DEADLINE_MISSED-api|         | |OK-api|/|WARNING-api||
+-------------------------------+-----------------------+
| |SAMPLE_LOST-api|             | |OK-api|/|WARNING-api||
+-------------------------------+-----------------------+

.. .. todo::
..   | |INCONSISTENT_TOPIC-api|      | \-                    |
..   +-------------------------------+-----------------------+

.. note::

  For entity transitions, |WARNING-api| status level takes precedence over |OK-api| level, and |ERROR-api| does over
  |WARNING-api| and |OK-api| levels.
