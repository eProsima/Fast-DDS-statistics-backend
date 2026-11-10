.. include:: ../exports/alias.include

.. _types_entity_kind:

EntityKind
==========

The *eProsima Fast DDS Statistics Backend* tracks the following entities discovered
in the DDS layout:

- |HOST-api|: The host or machine where a participant is allocated.
- |USER-api|: The user that executed a participant.
- |PROCESS-api|: The process where the participant is running.
- |DOMAIN-api|: Abstract DDS network by Domain or by Discovery Server.
- |TOPIC-api|: DDS Topic.
- |PARTICIPANT-api|: DDS Domain Participant.
- |DATAWRITER-api|: DDS DataWriter.
- |DATAREADER-api|: DDS DataReader.
- |LOCATOR-api|: Physical locator that a communication is using (IP + port, or SHM + port). Stores the locator
  statistic data.
