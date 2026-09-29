.. include:: ../exports/alias.include

.. _pro_topic_data_interaction:

Topic Data Interaction |Pro|
============================

*Fast DDS Statistics Backend* itself only observes a topic's data through |start_topic_spy-api| (see
:ref:`statistics_backend_topic_spy`), which reports every sample serialized as a JSON string. *Fast DDS
Statistics Backend Pro* extends this with source-timestamp
reporting on the same JSON spy, the ability to publish samples on a topic, to spy a topic without paying the
cost of JSON serialization, and to describe a topic's dynamic type as a JSON schema that an application can use
to build its own editor or field-mapping UI.

Topic spy with source timestamps
-----------------------------------

A second overload of |start_topic_spy-api| additionally reports the DDS sample's source timestamp, in
nanoseconds since epoch, alongside the same JSON-serialized data the single-argument overload already provides:

.. literalinclude:: /code/StatisticsBackendProTests.cpp
    :language: c++
    :start-after: //CONF-PRO-TOPIC-SPY-TIMESTAMP-EXAMPLE
    :end-before: //!
    :dedent: 8

This overload also accepts an optional ``type_name``: when given, the type is resolved by that (already
discovered, or :ref:`registered <pro_type_registration>`) data type name instead, allowing a spy on a
user-chosen topic name whose type was never itself discovered.

.. note::

   Any sequence or array member with more than 256 elements is truncated in the serialized JSON and labeled as
   such, rather than being serialized in full. A sample containing a truncated collection is display-only: it
   cannot be round-tripped back through |publish_topic_sample-api| or any other JSON deserialization.

Publishing samples on a topic
-------------------------------

|start_topic_publisher-api| creates a user-data DataWriter for a given topic, using the dynamic type already
discovered on that topic. The DataWriter is created on the monitor's own spy participant and reused on
subsequent calls, so starting a publisher that is already active is a no-op:

.. literalinclude:: /code/StatisticsBackendProTests.cpp
    :language: c++
    :start-after: //CONF-PRO-TOPIC-PUBLISHER-EXAMPLE
    :end-before: //!
    :dedent: 8

* |publish_topic_sample-api| publishes a single sample from a JSON representation of the topic's dynamic data.
  The JSON string must match the dynamic type of the topic, in the same EPROSIMA JSON format that the spy
  serialization itself produces - so a sample captured from |start_topic_spy-api| can be replayed verbatim with
  |publish_topic_sample-api|.
* |stop_topic_publisher-api| tears down the DataWriter created by |start_topic_publisher-api|. It is idempotent:
  calling it when no publisher is active for the topic is safe.

An optional ``type_name`` parameter on |start_topic_publisher-api| resolves the dynamic type by that
(already-discovered, or :ref:`registered <pro_type_registration>`) type name instead, letting the publisher be
created on an arbitrary, user-chosen topic name that itself need not have been discovered.

Raw topic spy
--------------

|start_topic_spy_raw-api| delivers a topic's fields directly as a |RawImageSample-api| value, bypassing JSON
serialization entirely. It exists for high-throughput topics - most notably image and video streams - where
JSON serialization of a large byte buffer on every sample is itself a bottleneck.

.. literalinclude:: /code/StatisticsBackendProTests.cpp
    :language: c++
    :start-after: //CONF-PRO-TOPIC-SPY-RAW-EXAMPLE
    :end-before: //!
    :dedent: 8

|RawImageSample-api| carries the byte buffer plus whichever of ``width``, ``height``, ``step``, ``encoding`` and
``format`` the topic type actually provides; fields the type does not have stay at their zero/empty default. A
ROS 2 ``sensor_msgs::msg::CompressedImage`` publisher, for instance, only ever populates ``data`` and ``format``.

|stop_topic_spy_raw-api| stops a raw spy started with |start_topic_spy_raw-api|. It is idempotent: calling it
when no raw spy is active for the topic is safe.

Mapping a custom type onto RawImageSample
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A second overload of |start_topic_spy_raw-api| takes a |RawImageFieldMap-api| so a topic whose type does not use
the canonical image member names (``data`` / ``width`` / ``height`` / ``step`` / ``encoding`` / ``format``) can
still be delivered as a |RawImageSample-api|. Each |RawImageFieldMap-api| slot holds a field path - a list of
member-name segments from the type's root down to the target member, for example ``{"header", "payload"}`` for
a nested ``header.payload`` field - or is left empty to leave the corresponding |RawImageSample-api| field at
its default.

.. list-table::
    :header-rows: 1

    * - |RawImageFieldMap-api| slot
      - Meaning
    * - ``data``
      - Byte buffer field path. Required for every mode.
    * - ``width`` / ``height`` / ``step``
      - Integer field paths (raw pixel-buffer mode). ``step`` is optional.
    * - ``encoding``
      - String or enum field path naming the pixel encoding (raw pixel-buffer mode).
    * - ``format``
      - String field path naming the codec (compressed-image modes).

Each slot also has a corresponding ``_const`` field (``width_const``, ``height_const``, ``step_const``,
``encoding_const``, ``format_const``); when set, a constant takes precedence over the field path for that slot,
for a type that has no suitable field to map at all. A path segment is normally a member name, but a decimal
integer segment is also supported, to address a sequence or array element by index.

Topic type schema
-------------------

|get_topic_type_schema-api| returns a JSON description of a topic's dynamic type, suitable for building a form
or a field-mapping editor such as the one described above, in the shape:

.. code-block:: json

    {
      "name": "<topic type name>",
      "kind": "struct",
      "members": [
        { "name": "<field name>", "kind": "<int32|uint32|...|struct|sequence|array|string|unsupported>",
          "path": "<dotted path>", "supported": true, "members": [], "element": {} }
      ]
    }

By default, when the discovery-built type is missing nested enum or bitmask metadata, the type is re-parsed
from its stored IDL to recover it - needed for a full type view, though it can log a recoverable parser error
for a type whose serialized IDL cannot be round-tripped. Pass ``reparse_idl=false`` to skip this and avoid that
noise when only the top-level field kinds are needed, for example for a quick feasibility check.

|get_topic_type_schema_by_type_name-api| is the same, but resolved directly by type name from the database
instead of by ``(monitor_id, topic_name)``. It needs no live monitor or discovery-populated context, so unlike
|get_topic_type_schema-api| it also works for a topic that never went through DDS discovery at all - most
notably, every topic loaded from an offline recording.
