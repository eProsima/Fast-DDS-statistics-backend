.. include:: ../exports/alias.include

.. _pro_type_registration:

Type Registration |Pro|
========================

Every method described in :ref:`statistics_backend` that needs a topic's data type (spying, charting, dynamic
type inspection) relies on that type having been *discovered* on the network: a remote participant must have
propagated its type information over DDS before the backend can use it. *Type Registration* removes that
requirement by letting an application supply a type definition directly, so it can be used for topics whose
publisher has not been started yet, or whose type will never be advertised over DDS at all (for example a
*Safe DDS* topic).

A registered type is made available globally: it is added to every active monitor and to any monitor created
afterwards, and it is resolvable by name from the same spy, publisher, topic-chart and topic-type-schema flows
described in :ref:`pro_topic_data_interaction`, for a topic name of the caller's choosing. Registration is
session-only - registered types are not persisted across a process restart, and must be registered again if
needed.

Registering from IDL
---------------------

|register_type-api| parses a type from its IDL definition:

.. literalinclude:: /code/StatisticsBackendProTests.cpp
    :language: c++
    :start-after: //CONF-PRO-REGISTER-TYPE-EXAMPLE
    :end-before: //!
    :dedent: 8

The type is parsed into a ``DynamicType`` and registered under ``type_name``. The ``type_name`` given does not
need to match a struct defined in the IDL: when it differs from an explicitly given ``struct_name``, the built
struct is wrapped in an XTypes alias named ``type_name``, since DDS/XTypes allows registering a ``TypeSupport``
under any type name independently of the type's own internal name. When ``struct_name`` is left empty (the
default), ``type_name`` must name a struct in the IDL, and a bare name is resolved to its fully qualified form
(for example ``Log`` to ``rcl_interfaces::msg::Log``). The type is then registered under that fully qualified
struct name, with no alias involved.

The IDL may reference auxiliary files through ``#include`` directives; pass their contents via the optional
``aux_files`` map, keyed by the relative filename used in the ``#include`` (for example
``"common/Header.idl"``), so the includes resolve while parsing.

On failure, |register_type-api| returns a status code below and leaves ``error_message`` filled in with a
human-readable description; nothing already registered is modified.

.. list-table::
    :header-rows: 1

    * - |RegisterTypeStatus-api| value
      - Meaning
    * - ``OK``
      - The type was parsed and registered successfully.
    * - ``INVALID_INPUT``
      - The type name or the IDL was empty.
    * - ``PARSE_ERROR``
      - The IDL could not be parsed.
    * - ``TYPE_NAME_NOT_IN_IDL``
      - The struct to build (``struct_name``, or ``type_name`` when ``struct_name`` is empty) does not appear in
        the IDL.
    * - ``ALREADY_EXISTS``
      - A type with the given name is already known, either discovered or previously registered.

Registering from a serialized TypeObject
------------------------------------------

|register_type_from_type_object-api| achieves the same effect without going through the IDL text parser: the
given XTypes ``CompleteTypeObject``, serialized as an XCDRv2 byte string, is deserialized and a ``DynamicType``
is built directly from it - the same path live discovery itself uses to build a type. This is required for
types the IDL grammar parser cannot yet handle, such as ``bitset`` or ``bitmask`` members: those build correctly
from a ``TypeObject`` but fail to re-parse from their own serialized IDL. This is the path used when replaying
an offline recording, where every recorded type already carries its own ``TypeObject``.

.. important::

   Any dependency ``TypeObject`` referenced by the one being registered must already be registered in the
   ``DomainParticipantFactory``'s type object registry. Offline recording playback registers every recorded type
   before building any of them, for exactly this reason.

Inspecting registered types
------------------------------

* |get_registered_type_names-api| returns the Topic Type Names of every type registered via
  |register_type-api| or |register_type_from_type_object-api|. Discovered types are not included - only
  user-registered ones - so a caller can offer them as a distinct list (for example, in a type-registration
  form).
* |get_registered_type_struct_name-api| returns the underlying struct name of a registered type: for a type
  registered under an alias this is the aliased struct's name, otherwise it is the same name. This lets a
  caller that persists a registered type (for example, a saved workspace) later re-register it with the correct
  ``struct_name`` so the alias is rebuilt identically.
* |get_all_type_idls-api| returns every known type that has a stored IDL, whether discovered or registered, as
  a map of type name to IDL text - useful for offering existing types as a starting point when registering a
  new one.
