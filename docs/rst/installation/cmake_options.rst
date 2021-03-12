.. include:: ../exports/roles.include

.. _cmake_options:

CMake options
=============

*eProsima Fast DDS Statistics Backend* provides several CMake options for build configuration of the library.

.. list-table::
    :header-rows: 1

    *   - Option
        - Description
        - Possible values
        - Default
    *   - :class:`BUILD_DOCUMENTATION`
        - Build the library documentation
        - ``ON`` ``OFF``
        - ``ON``
    *   - :class:`FASTDDS_STATISTICS_BACKEND_DOCS_BUILDER`
        - Select the sphinx builder used to |br|
          build the documentation.
        - ``html`` ``dirhtml`` ``singlehtml`` ``pickle`` |br|
          ``json`` ``htmlhelp`` ``qthelp`` ``applehelp`` |br|
          ``devhelp`` ``epub`` ``epub3`` ``latex`` ``text`` |br|
          ``man`` ``texinfo`` ``gettext`` ``changes`` |br|
          ``xml`` ``pseudoxml`` ``pdf`` ``latexpdf`` |br|
          ``latexpdfja`` ``info``
        - ``html``
    *   - :class:`BUILD_DOCUMENTATION_TESTS`
        - Build the library documentation tests
        - ``ON`` ``OFF``
        - ``OFF``
