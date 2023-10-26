.. include:: ../exports/alias.include

.. _types_status_kind:

StatusData
==========

The *eProsima Fast DDS Statistics Backend* records entities' status data of different nature, as provided by the Monitor Service from *eProsima Fast DDS 
Statistics Module*, e.g., incompatible QoS or the number of lost samples. We refer to the nature of these status data values as 
their |StatusKind-api|.

- |PROXY-api|: Collection of Parameters describing the Proxy Data of that entity.

- |CONNECTION_LIST-api|: List of connections that this entity is using. Each of the elements is a Connection in which the possible 
  values for the ConnectionMode are: intraprocess, datasharing, transport. In addition, information comprising the announced locators 
  and locator in use with each one of the matched entities is also included.

- |INCOMPATIBLE_QOS-api|: Status of the Incompatible QoS of that entity.
  - |DATAWRITER-api| Incompatible QoS Offered.
  - |DATAREADER-api| Incompatible QoS Requested.

- |INCONSISTENT_TOPIC-api|: Status of Inconsistent topics of the topic of that entity. Asked to the topic of the requested entity.

- |LIVELINESS_LOST-api|: Tracks the status of the number of times that liveliness was lost by a |DATAWRITER-api|.

- |LIVELINESS_CHANGED-api|: Tracks the status of the number of times that liveliness status changed in a |DATAREADER-api|.

- |DEADLINE_MISSED-api|: The Status of the number of deadlines missed that were registered in that entity.

- |SAMPLE_LOST-api|: Tracks the number of times that this entity lost samples.

.. warning::
  |INCONSISTENT_TOPIC-api| status data not supported yet.

Only |PARTICIPANT-api|, |DATAWRITER-api| and |DATAREADER-api| have associated status data. The following table describes which |StatusKind-api| 
each of these :ref:`entities<types_entity_kind>` has:

+-------------------------------+-----------------+----------------+----------------+
| Signature                     ||PARTICIPANT-api|||DATAWRITER-api|||DATAREADER-api||
+===============================+=================+================+================+
| |PROXY-api|                   | Yes             | Yes            | Yes            |
+-------------------------------+-----------------+----------------+----------------+
| |CONNECTION_LIST-api|         | Yes             | Yes            | Yes            |
+-------------------------------+-----------------+----------------+----------------+
| |INCOMPATIBLE_QOS-api|        | No              | Yes            | Yes            |
+-------------------------------+-----------------+----------------+----------------+
| |INCONSISTENT_TOPIC-api|      | No              | Yes            | Yes            |
+-------------------------------+-----------------+----------------+----------------+
| |LIVELINESS_LOST-api|         | No              | Yes            | No             |
+-------------------------------+-----------------+----------------+----------------+
| |LIVELINESS_CHANGED-api|      | No              | No             | Yes            |
+-------------------------------+-----------------+----------------+----------------+
| |DEADLINE_MISSED-api|         | No              | Yes            | Yes            |
+-------------------------------+-----------------+----------------+----------------+
| |SAMPLE_LOST-api|             | No              | No             | Yes            |
+-------------------------------+-----------------+----------------+----------------+

Each |StatusKind-api| has an associated |StatusLevel-api|. |OK-api| status is obtained when the monitor service message reports no issue.
Entity's associated |StatusLevel-api| is obtained from  all status data. The following table describes which status levels are associated 
with each |StatusKind-api|, and to which |StatusLevel-api| they can take an entity:

+-------------------------------+------------------------+-------------------------+
| Signature                     |Associated status levels|Entity status transitions|
+===============================+========================+=========================+
| |PROXY-api|                   | |OK-api|               | |OK-api|                |
+-------------------------------+------------------------+-------------------------+
| |CONNECTION_LIST-api|         | |OK-api|               | |OK-api|                |
+-------------------------------+------------------------+-------------------------+
| |INCOMPATIBLE_QOS-api|        | |OK-api|/|ERROR-api|   | |OK-api|/|ERROR-api|    |
+-------------------------------+------------------------+-------------------------+
| |INCONSISTENT_TOPIC-api|      | \-                     | \-                      |
+-------------------------------+------------------------+-------------------------+
| |LIVELINESS_LOST-api|         | |OK-api|/|WARNING-api| | |OK-api|/|WARNING-api|  |
+-------------------------------+------------------------+-------------------------+
| |LIVELINESS_CHANGED-api|      | |OK-api|               | |OK-api|                |
+-------------------------------+------------------------+-------------------------+
| |DEADLINE_MISSED-api|         | |OK-api|/|ERROR-api|   | |OK-api|/|WARNING-api|  |
+-------------------------------+------------------------+-------------------------+
| |SAMPLE_LOST-api|             | |OK-api|/|ERROR-api|   | |OK-api|/|WARNING-api|  |
+-------------------------------+------------------------+-------------------------+

.. note::

  Deadline missed or a loss of a sample are considered errors, and therefore their corresponding |StatusKind-api| sample may have the |ERROR-api| status. 
  However, this doesn't necessarily mean that the normal operation of the entity is disrupted, and therefore they only take the status of the entity to 
  the |WARNING-api| status level.

.. note::

  For entity transitions, |WARNING-api| status level takes precedence over |OK-api| level, and |ERROR-api| does over |WARNING-api| and |OK-api| levels.
