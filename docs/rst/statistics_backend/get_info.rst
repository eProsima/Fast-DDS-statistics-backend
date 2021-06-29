.. include:: ../exports/alias.include

.. _statistics_backend_get_info:

Get entity meta information
---------------------------

*Fast DDS Statistics Backend* includes the possibility of retrieving the meta information of any given entity present
in the network.
The returned tree always includes the basic information about the entity: ``kind``, ``id``, ``name``, ``alias`` and
if the entity is ``alive``.
Depending on the |EntityKind-api|, the returned object can contain extra information such as ``pid``, ``guid``, ``qos``,
``locators`` or ``data_type``.
|get_info-api| returns a |Info-api| object.

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

.. literalinclude:: /code/participant_info_example.json
    :language: JSON

.. _statistics_backend_get_info_datareader:

DataReader Info example
^^^^^^^^^^^^^^^^^^^^^^^

.. literalinclude:: /code/datareader_info_example.json
    :language: JSON

.. _statistics_backend_get_info_datawriter:

DataWriter Info example
^^^^^^^^^^^^^^^^^^^^^^^

.. literalinclude:: /code/datawriter_info_example.json
    :language: JSON

.. _statistics_backend_get_info_topic:

Topic Info example
^^^^^^^^^^^^^^^^^^

.. literalinclude:: /code/topic_info_example.json
    :language: JSON
