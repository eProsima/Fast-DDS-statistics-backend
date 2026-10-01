.. include:: ../exports/alias.include

.. _statistics_backend_topic_spy:

Topic spy
---------

|start_topic_spy-api| subscribes to a user topic and delivers every sample received on it, already serialized
to JSON, through a callback:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-TOPIC-SPY-EXAMPLE
    :end-before: //!
    :dedent: 8

|start_topic_spy-api| is idempotent: calling it again for a topic that is already being spied in that monitor
has no effect. It throws if the topic's data type has not yet been discovered on the network: a topic can only
be spied once at least one participant publishing or subscribing to it has been seen.

|stop_topic_spy-api| stops a spy started with |start_topic_spy-api|; calling it again after it has stopped has no
effect. It throws |BadParameter-api| if the monitor ID is unknown, or if the given topic was never spied on in
that monitor.

.. note::
   The spy's DataReader is created with QoS matching the *first* writer discovered on that topic. If several
   writers with different, mutually incompatible QoS (for example, differing reliability or durability)
   publish on the same topic, data from the others may not be received.

.. note::
   *Fast DDS Statistics Backend Pro* extends this with a second overload that additionally reports each
   sample's source timestamp, a resolved-by-type-name variant, and a raw (non-JSON) spy for high-throughput
   data such as images (see :ref:`pro_topic_data_interaction`).
