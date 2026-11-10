.. include:: ../exports/alias.include

.. _statistics_backend_dump_load:

Saving and restoring the statistics data
-----------------------------------------

*Fast DDS Statistics Backend* can dump the contents of the database to the file system,
as a backup or to analyze the data offline later.
A previously saved dump can also be loaded, so this analysis can be
done with any front-end that communicates with the *Fast DDS Statistics Backend*.

- Use |dump_database-api| to save the content of the Backend's database to a file.
- Use |load_database-api| to load a saved database to the Backend.

|dump_database-api| also has an in-memory overload that takes only the ``clear`` bool and returns a
``DatabaseDump`` object instead of writing it to a file. Use it when the dump is consumed
in-process and not persisted.

For the format of the dumped data, see :ref:`database dumps`.

.. warning::
    A saved database can only be loaded on an empty Backend.
    This means that no monitors were initialized since the Backend started,
    or that the Backend has been reset using |reset-api|.
    If |load_database-api| is used on a non-empty Backend,
    |PreconditionNotMet-api| is thrown.
    |load_database-api| also throws |BadParameter-api| if the given file does not exist.

The following snippet dumps the current database contents to a file,
resets the Backend, and then loads another data set that was saved previously.

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-DUMP-LOAD-EXAMPLE
    :end-before: //!
    :dedent: 8

The bool parameter of |dump_database-api| indicates whether the statistics data
of all entities is cleared after the dump.

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-DUMP-AND_CLEAR-EXAMPLE
    :end-before: //!
    :dedent: 8
