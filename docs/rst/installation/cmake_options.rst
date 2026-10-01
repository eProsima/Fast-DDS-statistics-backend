.. include:: ../exports/roles.include

.. _cmake_options:

CMake options
=============

*eProsima Fast DDS Statistics Backend* has the following CMake options to configure the library build.

.. list-table::
    :header-rows: 1

    *   - Option
        - Description
        - Possible values
        - Default
    *   - :class:`BUILD_DOCS`
        - Build the library documentation. Set to ``ON`` if |br|
          :class:`BUILD_DOCS_TESTS` is set to ``ON``.
        - ``ON`` ``OFF``
        - ``OFF``
    *   - :class:`BUILD_TESTS`
        - Build the library tests.
        - ``ON`` ``OFF``
        - ``OFF``
    *   - :class:`BUILD_DOCS_TESTS`
        - Build the library documentation tests. Setting this |br|
          ``ON`` will set :class:`BUILD_DOCS` to ``ON``
        - ``ON`` ``OFF``
        - ``OFF``
    *   - :class:`BUILD_SHARED_LIBS`
        - Builds internal libraries as shared libraries, i.e. |br|
          causes add_library() CMake function to create |br|
          shared libraries if on. All libraries are built |br|
          shared unless the library was explicitly added as |br|
          a static library.
        - ``ON`` ``OFF``
        - ``ON``
    *   - :class:`COMPILE_EXAMPLES`
        - Build the example applications distributed with the |br|
          library (see :ref:`full_example`).
        - ``ON`` ``OFF``
        - ``OFF``
