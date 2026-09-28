.. include:: ../exports/alias.include

.. _statistics_backend_dump_load:

Saving and restoring the statistics data
-----------------------------------------

*Fast DDS Statistics Backend* allows to dump the contents of the database to the file system.
This may be used as a backup procedure, or as a means of analyzing the data offline later.
It is also possible to load a dump previously saved, which allows for this analysis to be
done with any front-end that communicates with the *Fast DDS Statistics Backend*.

- Use |dump_database-api| to save the content of the Backend's database to a file.
- Use |load_database-api| to loaded a saved database to the Backend.

|dump_database-api| also has an in-memory overload, taking only the ``clear`` bool and returning a
``DatabaseDump`` object directly instead of writing it to a file - useful when the dump is going to be consumed
in-process rather than persisted.

For information about the format of the dumped data, please, refer to :ref:`database dumps`.

.. warning::
    Loading a saved database can only be done on an empty Backend.
    This means that no monitors were initialized since the Backend started,
    or that the Backend has been reset using |reset-api|.
    If |load_database-api| is used on a non-empty Backend,
    |PreconditionNotMet-api| is thrown.
    |load_database-api| also throws |BadParameter-api| if the given file does not exist.

The following snippet shows how to dump the current database contents to a file,
and then load another data set that was saved previously,
resetting the Backend in between.

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-DUMP-LOAD-EXAMPLE
    :end-before: //!
    :dedent: 8

The bool parameter of |dump_database-api| indicates if the statistics data
of all entities must be cleared after the dump.

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-DUMP-AND_CLEAR-EXAMPLE
    :end-before: //!
    :dedent: 8
