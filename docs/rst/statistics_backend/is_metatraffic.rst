.. include:: ../exports/alias.include

.. _statistics_backend_is_metatraffic:

Check whether an entity is builtin
----------------------------------

*eProsima Fast DDS Statistics Backend* discovers any DDS entity in the monitored domain or *Fast DDS* Discovery Server
network, including the builtin entities used to exchange metatraffic data that allows mutual discovery.
|is_metatraffic-api| returns whether the entity is related to these builtin entities or not.
The possible DDS builtin entities are always of |TOPIC-api|, |DATAWRITER-api|, or |DATAREADER-api| kind (refer to
:ref:`types_entity_kind` for more information).
This function allows the user to discriminate between the topics and endpoints exchanging metatraffic data and those
that are exchanging user data.

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-IS-METATRAFFIC-EXAMPLE
    :end-before: //!
    :dedent: 8
