.. include:: ../exports/alias.include

.. _statistics_backend_get_info:

Get entity meta information
---------------------------

|get_info-api| retrieves the meta information of any entity present in the network and returns it as a |Info-api|
object.
The returned tree always includes the basic information about the entity: ``kind``, ``id``, ``name``, ``alias`` and
if the entity is ``alive``.
Depending on the |EntityKind-api|, the returned object can contain extra information such as ``pid``, ``guid``, ``qos``,
``locators`` or ``data_type``.

A second overload of |get_info-api| takes an |AlertId-api| instead of an |EntityId-api|. It returns a |Info-api|
object describing that alert's configuration: its name, kind, domain, host/user/topic filters, threshold,
and timing parameters (see :ref:`types_alertinfo`).

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-GET-QOS-EXAMPLE
    :end-before: //!
    :dedent: 8

.. _statistics_backend_get_info_host:

Host Info example
^^^^^^^^^^^^^^^^^

.. literalinclude:: /code/host_info_example.json
    :language: JSON

.. _statistics_backend_get_info_user:

User Info example
^^^^^^^^^^^^^^^^^

.. literalinclude:: /code/user_info_example.json
    :language: JSON

.. _statistics_backend_get_info_process:

Process Info example
^^^^^^^^^^^^^^^^^^^^

.. literalinclude:: /code/process_info_example.json
    :language: JSON

.. _statistics_backend_get_info_locator:

Locator Info example
^^^^^^^^^^^^^^^^^^^^

.. literalinclude:: /code/locator_info_example.json
    :language: JSON

.. _statistics_backend_get_info_domain:

Domain Info example
^^^^^^^^^^^^^^^^^^^

.. literalinclude:: /code/domain_info_example.json
    :language: JSON

.. _statistics_backend_get_info_participant:

Participant Info example
^^^^^^^^^^^^^^^^^^^^^^^^

.. todo::

    Provide `app_metadata` fields in the json.

.. literalinclude:: /code/participant_info_example.json
    :language: JSON

.. _statistics_backend_get_info_datareader:

DataReader Info example
^^^^^^^^^^^^^^^^^^^^^^^

.. todo::

    Provide `app_metadata` fields in the json.

.. literalinclude:: /code/datareader_info_example.json
    :language: JSON

.. _statistics_backend_get_info_datawriter:

DataWriter Info example
^^^^^^^^^^^^^^^^^^^^^^^

.. todo::

    Provide `app_metadata` fields in the json.

.. literalinclude:: /code/datawriter_info_example.json
    :language: JSON

.. _statistics_backend_get_info_topic:

Topic Info example
^^^^^^^^^^^^^^^^^^

.. literalinclude:: /code/topic_info_example.json
    :language: JSON
