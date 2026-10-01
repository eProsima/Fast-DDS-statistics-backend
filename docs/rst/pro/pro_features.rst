.. include:: ../exports/alias.include

.. _statistics_backend_pro:

Pro Features Overview
======================

*Fast DDS Statistics Backend Pro* is the commercial edition of *Fast DDS Statistics Backend*.
It links against the same |StatisticsBackend-api| singleton and the same |DomainListener-api| /
|PhysicalListener-api| callback interfaces described in :ref:`statistics_backend`, so almost every application
built against the open-source API works unmodified against Pro. Most of what Pro adds is new methods and types on
the same shared class. The few exceptions, where a method that exists in both editions behaves differently or a
shared enum's contents differ, are listed in
:ref:`differences from the open-source edition <pro_differences_from_basic>` below.

*Fast DDS Statistics Backend Pro* is the library that *DDS Monitor Pro* is built on: every Pro-only
feature described here is the backend counterpart of a *DDS Monitor Pro* feature (Register Type, the Publisher
Pane, the Image Pane, On-Demand Statistics Readers, and so on). An application can also use this API directly,
without *DDS Monitor Pro*, for example to build its own tooling around a *Fast DDS* deployment.

.. note::

   All features described in this section are exclusive to *Fast DDS Statistics Backend Pro* and require a
   valid *Fast DDS Pro* or *Safe DDS* license (see :ref:`pro_licensing_safety`).

Besides all the functionality inherited from *Fast DDS Statistics Backend*, it includes the following Pro
features:

* :ref:`Type Registration <pro_type_registration>` |Pro| to register a user-supplied data type from its IDL
  definition (or from a serialized XTypes ``TypeObject``), so it becomes usable on topics whose type was never
  discovered on the network.

* :ref:`Topic Data Interaction <pro_topic_data_interaction>` |Pro| to publish samples on a topic, to spy a
  topic's raw (non-JSON-serialized) data for high-throughput streams such as images, and to retrieve a topic's
  dynamic type as a JSON schema suitable for building a form or a field-mapping editor.

* :ref:`Statistics Collection Control <pro_statistics_control>` |Pro| to create and destroy statistics
  DataReaders on demand instead of subscribing to every statistics topic up front, and to compute several
  |StatisticKind-api| values from a single database query.

* :ref:`Licensing and Safety <pro_licensing_safety>` |Pro| to validate the *Fast DDS Pro* / *Safe DDS* license at
  startup, and to detect whether a domain has proxied (remote) participants before a monitor is created on it.

.. _pro_differences_from_basic:

.. rubric:: Differences from the open-source edition

Besides the new API described above, a few methods and types shared by both editions behave differently in
each:

* |clear_monitor-api| is a no-op in the open-source edition (not yet implemented there; see
  :ref:`statistics_backend_clear`), but is fully functional in *Fast DDS Statistics Backend Pro*: it stops the
  monitor automatically if still active, and removes the domain and all of its entities (not only their
  statistical data), so the same domain can be monitored again from scratch.
* |DataKind-api| has *fewer* values in *Fast DDS Statistics Backend Pro* than in the open-source edition:
  ``NETWORK_LATENCY``, ``RTPS_PACKETS_SENT``, ``RTPS_BYTES_SENT``, ``RTPS_PACKETS_LOST``, ``RTPS_BYTES_LOST``,
  ``DISCOVERY_TIME`` and ``SAMPLE_DATAS`` are exclusive to the open-source edition (see :ref:`types_data_kind`).
  The underlying *Fast DDS Pro* no longer publishes these statistics. This is the one case where Pro is a strict
  subset rather than a superset of the open-source API.
* An additional recognized application identifier, ``AppId::DDS_SOMEIP_BRIDGE``, is available in *Fast DDS
  Statistics Backend Pro* for a DDS-SOME/IP Bridge application, alongside the identifiers shared with the
  open-source edition.
* Both editions create an internal *spy* participant to back the topic spy/publisher features, but only *Fast
  DDS Statistics Backend Pro* filters it out of discovery, so it never appears as an extra entity in
  |get_domain_view_graph-api|'s output.
