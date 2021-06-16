.. include:: ../exports/alias.include

.. _types_entity_kind:

EntityKind
==========

The *eProsima Fast DDS Statistics Backend* keeps track of the entities discovered
in the DDS layout.
The following list shows the different entities that are tracked:

- |HOST-api|: The host or machine where a participant is allocated.
- |USER-api|: The user that has executed a participant.
- |PROCESS-api|: The process where the participant is running.
- |DOMAIN-api|: Abstract DDS network by Domain or by Discovery Server.
- |TOPIC-api|: DDS Topic.
- |PARTICIPANT-api|: DDS Domain Participant.
- |DATAWRITER-api|: DDS DataWriter.
- |DATAREADER-api|: DDS DataReader.
- |LOCATOR-api|: Physical locator that a communication is using.
