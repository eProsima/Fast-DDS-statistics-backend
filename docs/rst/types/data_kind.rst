.. include:: ../exports/alias.include

.. _types_data_kind:

DataKind
========

The *eProsima Fast DDS Statistics Backend* records statistics data of different
kinds, such as latency or message count, as provided by *eProsima Fast DDS Statistics Module*.
The kind of each data value is its |DataKind-api|.

- |FASTDDS_LATENCY-api|: The latency between a write operation
  in the data writer and the moment the data is available in the data reader.

- |NETWORK_LATENCY-api|: The latency in the communication between two locators.

- |PUBLICATION_THROUGHPUT-api|: Amount of data (in Mb/s) sent by a data writer.

- |SUBSCRIPTION_THROUGHPUT-api|: Amount of data (in Mb/s) received by a data reader.

- |RTPS_PACKETS_SENT-api|: Number of packets sent from a participant to a locator.

- |RTPS_BYTES_SENT-api|: Number of bytes sent from a participant to a locator.

- |RTPS_PACKETS_LOST-api|: Number of packets lost from a participant to a locator.

- |RTPS_BYTES_LOST-api|: Number of bytes lost from a participant to a locator.

- |RESENT_DATA-api|: Number of DATA/DATAFRAG sub-messages that had to be resent
  from a data writer.

- |HEARTBEAT_COUNT-api|: Number of HEARTBEATs that a data writer sends.

- |ACKNACK_COUNT-api|: Number of ACKNACKs that a data reader sends.

- |NACKFRAG_COUNT-api|: Number of NACKFRAGs that a data reader sends.

- |GAP_COUNT-api|: Number of GAPs that a data writer sends.

- |DATA_COUNT-api|: Number of DATA/DATAFRAGs that a data writer sends.

- |PDP_PACKETS-api|: Number of PDP packets sent by a participant.

- |EDP_PACKETS-api|: Number of EDP packets sent by a participant.

- |DISCOVERY_TIME-api|: Time when a participant discovers another DDS entity.

- |SAMPLE_DATAS-api|: Number of DATA/DATAFRAGs needed to send a single sample.


Each data kind is measured on one or two :ref:`entities<types_entity_kind>`.
For example, `FASTDDS_LATENCY` is always measured between a data writer
and a data reader, whereas `PDP_PACKETS` is always measured in a participant,
with no other entity involved.
The table lists the entity kinds involved in measuring each data kind:

+-------------------------------+-------------------+---------------+
| Signature                     | Source Entity     | Target Entity |
+===============================+===================+===============+
| |FASTDDS_LATENCY-api|         | DataWriter        | DataReader    |
+-------------------------------+-------------------+---------------+
| |NETWORK_LATENCY-api|         | DomainParticipant | Locator       |
+-------------------------------+-------------------+---------------+
| |PUBLICATION_THROUGHPUT-api|  | DataWriter        | \-            |
+-------------------------------+-------------------+---------------+
| |SUBSCRIPTION_THROUGHPUT-api| | DataReader        | \-            |
+-------------------------------+-------------------+---------------+
| |RTPS_PACKETS_SENT-api|       | DomainParticipant | Locator       |
+-------------------------------+-------------------+---------------+
| |RTPS_BYTES_SENT-api|         | DomainParticipant | Locator       |
+-------------------------------+-------------------+---------------+
| |RTPS_PACKETS_LOST-api|       | DomainParticipant | Locator       |
+-------------------------------+-------------------+---------------+
| |RTPS_BYTES_LOST-api|         | DomainParticipant | Locator       |
+-------------------------------+-------------------+---------------+
| |RESENT_DATA-api|             | DataWriter        | \-            |
+-------------------------------+-------------------+---------------+
| |HEARTBEAT_COUNT-api|         | DataWriter        | \-            |
+-------------------------------+-------------------+---------------+
| |ACKNACK_COUNT-api|           | DataReader        | \-            |
+-------------------------------+-------------------+---------------+
| |NACKFRAG_COUNT-api|          | DataReader        | \-            |
+-------------------------------+-------------------+---------------+
| |GAP_COUNT-api|               | DataWriter        | \-            |
+-------------------------------+-------------------+---------------+
| |DATA_COUNT-api|              | DataWriter        | \-            |
+-------------------------------+-------------------+---------------+
| |PDP_PACKETS-api|             | DomainParticipant | \-            |
+-------------------------------+-------------------+---------------+
| |EDP_PACKETS-api|             | DomainParticipant | \-            |
+-------------------------------+-------------------+---------------+
| |DISCOVERY_TIME-api|          | DomainParticipant | DDSEntity     |
+-------------------------------+-------------------+---------------+
| |SAMPLE_DATAS-api|            | DataWriter        | \-            |
+-------------------------------+-------------------+---------------+

.. warning::
   *Fast DDS Statistics Backend Pro* does not declare every |DataKind-api| listed above. ``NETWORK_LATENCY``,
   ``RTPS_PACKETS_SENT``, ``RTPS_BYTES_SENT``, ``RTPS_PACKETS_LOST``, ``RTPS_BYTES_LOST``, ``DISCOVERY_TIME`` and
   ``SAMPLE_DATAS`` are exclusive to the open-source edition. The underlying *Fast DDS Pro* no longer publishes
   these statistics, so the Pro backend's |DataKind-api| does not include them. Code that switches between
   editions must not assume the two |DataKind-api| enums are interchangeable.
