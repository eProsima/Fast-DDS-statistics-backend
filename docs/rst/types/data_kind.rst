.. include:: ../exports/alias.include

.. _types_data_kind:

DataKind
========

The *eProsima Fast DDS Statistics Backend* records statistics data of different
nature, as provided by *eProsima Fast DDS*, e.g., latency or message count.
We refer to the nature of these data values as their |DataKind-api|.

- |FASTDDS_LATENCY-api|: The latency between a write operation
  in the data writer and the moment the data is available in the data reader

- |NETWORK_LATENCY-api|: The Latency in the communication between two locators

- |PUBLICATION_THROUGHPUT-api|: Amount of data (in Mb/s) sent by a data writer

- |SUBSCRIPTION_THROUGHPUT-api|: Amount of data (in Mb/s) received by a data reader

- |RTPS_PACKETS_SENT-api|: Amount of packets sent from a participant to a Locator

- |RTPS_BYTES_SENT-api|: Amount of bytes sent from a participant to a Locator

- |RTPS_PACKETS_LOST-api|: Amount of packets lost from a participant to a Locator

- |RTPS_BYTES_LOST-api|: Amount of bytes lost from a participant to a Locator

- |RESENT_DATA-api|: Amount of DATA/DATAFRAG sub-messages that had to be resent
  from a data writer

- |HEARTBEAT_COUNT-api|: Amount of HEARTBEATs that a data writer sends

- |ACKNACK_COUNT-api|: Amount of ACKNACKs that a data reader sends

- |NACKFRAG_COUNT-api|: Amount of NACKFRAGs that a data reader sends

- |GAP_COUNT-api|: Amount of GAPs that a data writer sends

- |DATA_COUNT-api|: Amount of DATA/DATAFRAGs that a data writer sends

- |PDP_PACKETS-api|: Amount of PDP packets sent by  Participant

- |EDP_PACKETS-api|: Amount of EDP packets sent by  Participant

- |DISCOVERED_ENTITY-api|: Time when a participant discovers another DDS entity

- |SAMPLE_DATAS-api|: Amount of DATA/DATAFRAGs needed to send a single sample


Each data kind may relate to one or two entities where they are measured.
For example, a `FASTDDS_LATENCY` is always measured between a data data writer
and a data reader, whereas `PDP_PACKETS` is always measured in a participant,
with no other entity involved in the measurement.
The following table describes which entity kinds are involved in the
measurement of each data kind:

+-------------------------------+-------------------+---------------+
| Signature                     | Entities source   | Entity target |
+===============================+===================+===============+
| |FASTDDS_LATENCY-api|         | DataWriter        | DataReader    |
+-------------------------------+-------------------+---------------+
| |NETWORK_LATENCY-api|         | Locator           | Locator       |
+-------------------------------+-------------------+---------------+
| |PUBLICATION_THROUGHPUT-api|  | DataWriter        | -             |
+-------------------------------+-------------------+---------------+
| |SUBSCRIPTION_THROUGHPUT-api| | DataReader        | -             |
+-------------------------------+-------------------+---------------+
| |RTPS_PACKETS_SENT-api|       | DomainParticipant | Locator       |
+-------------------------------+-------------------+---------------+
| |RTPS_BYTES_SENT-api|         | DomainParticipant | Locator       |
+-------------------------------+-------------------+---------------+
| |RTPS_PACKETS_LOST-api|       | DomainParticipant | Locator       |
+-------------------------------+-------------------+---------------+
| |RTPS_BYTES_LOST-api|         | DomainParticipant | Locator       |
+-------------------------------+-------------------+---------------+
| |RESENT_DATA-api|             | DataWriter        | -             |
+-------------------------------+-------------------+---------------+
| |HEARTBEAT_COUNT-api|         | DataWriter        | -             |
+-------------------------------+-------------------+---------------+
| |ACKNACK_COUNT-api|           | DataReader        | -             |
+-------------------------------+-------------------+---------------+
| |NACKFRAG_COUNT-api|          | DataReader        | -             |
+-------------------------------+-------------------+---------------+
| |GAP_COUNT-api|               | DataWriter        | -             |
+-------------------------------+-------------------+---------------+
| |DATA_COUNT-api|              | DataWriter        | -             |
+-------------------------------+-------------------+---------------+
| |PDP_PACKETS-api|             | DomainParticipant | -             |
+-------------------------------+-------------------+---------------+
| |EDP_PACKETS-api|             | DomainParticipant | -             |
+-------------------------------+-------------------+---------------+
| |DISCOVERED_ENTITY-api|       | DomainParticipant | DDSEntity     |
+-------------------------------+-------------------+---------------+
| |SAMPLE_DATAS-api|            | DataWriter        | -             |
+-------------------------------+-------------------+---------------+

