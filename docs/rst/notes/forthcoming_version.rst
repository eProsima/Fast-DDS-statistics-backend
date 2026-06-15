.. add orphan tag when new info added to this file

:orphan:

###################
Forthcoming Version
###################

This release includes the following **new features**:

* ``init_monitor`` now supports topic-name DataReader profile lookup:
  when creating a DataReader for a statistics topic, a loaded XML profile whose name matches
  the topic name is automatically applied, giving users full control over QoS fields such as
  history, durability and memory policy.
  If no matching profile exists, or if the topic is the monitor service topic, the default
  statistics QoS is used unchanged.
  For more details, see :ref:`statistics_backend_init_datareader_profiles`.

This release includes the following **documentation updates**:

* Document topic-name DataReader profile lookup for :ref:`init_monitor <statistics_backend_init>`,
  including fallback behavior when no matching profile exists and the special handling
  of the monitor service topic.
