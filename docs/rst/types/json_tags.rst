.. include:: ../exports/alias.include

.. _types_tags:

JSON Tags
=========

The |StatisticsBackend-api| use JSON format to retrieve information in many methods as |get_info-api|,
|get_graph-api| or |dump_database-api|.

.. todo::

    In order to access every item in every JSON generated, the following tags are provided:
    Create table

Dump Tags Example
-----------------

The following snippet shows an example of a database dump, result of calling |dump_database-api| in a database
with one entity of each |EntityKind-api|, and one data of each |DataKind-api|:

.. literalinclude:: /code/dump_example.json
   :language: JSON
