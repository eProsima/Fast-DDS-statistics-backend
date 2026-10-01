.. include:: ../exports/alias.include

.. _types_tags:

JSON Tags
=========

Many |StatisticsBackend-api| methods return information in JSON format, such as |get_info-api|,
|get_domain_view_graph-api| or |dump_database-api|.

.. todo::

    To access every item in every generated JSON, use the following tags:

    Pending table creation

Dump Tags Example
-----------------

Example of a database dump, the result of calling |dump_database-api| on a database
with one entity of each |EntityKind-api| and one data of each |DataKind-api|:

.. todo::

    Provide `app_metadata` fields in the json.

.. literalinclude:: /code/dump_example.json
   :language: JSON
