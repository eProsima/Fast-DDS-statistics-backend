.. _linux_installation:

Linux installation from sources
===============================

This page explains how to install *eProsima Fast DDS Statistics Backend* from sources.

.. contents::
    :local:
    :backlinks: none
    :depth: 2

.. _fastdds_backend_linux:

Fast DDS Statistics Backend installation
""""""""""""""""""""""""""""""""""""""""

To install *eProsima Fast DDS Statistics Backend* from sources in a Linux environment, first meet the
:ref:`requirements_source_linux` and :ref:`dependencies_source_linux` detailed below.
Then follow either the :ref:`colcon <colcon_installation_linux>`
or the :ref:`CMake <cmake_installation_linux>` installation instructions.

.. _requirements_source_linux:


Requirements
------------

Installing *eProsima Fast DDS Statistics Backend* from sources in a Linux environment
requires the following tools:

* :ref:`cmake_gcc_pip3_wget_git_source_linux`
* :ref:`gtest_source_linux` [optional]

.. _cmake_gcc_pip3_wget_git_source_linux:

CMake, g++, pip3, wget and git
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

These packages are needed to install *eProsima Fast DDS Statistics Backend* and its dependencies
from the command line.
Install CMake_, `g++ <https://gcc.gnu.org/>`_, pip3_, wget_ and git_ using the package manager of the appropriate
Linux distribution. For example, on Ubuntu use the command:

.. code-block:: bash

    sudo apt install cmake g++ python3-pip wget git

.. _gtest_source_linux:

Gtest
^^^^^

Gtest is a unit testing library for C++.
By default, *eProsima Fast DDS Statistics Backend* does not compile tests.
To build them, set the corresponding
`CMake configuration options <https://cmake.org/cmake/help/v3.6/manual/cmake.1.html#options>`_
when calling colcon_ or CMake_ (see :ref:`cmake_options`).
For the Gtest installation process, see the
`Gtest Installation Guide <https://github.com/google/googletest>`_.

.. note::

    *eProsima Fast DDS Statistics Backend* depends on Gtest release-1.10.0 or later.


.. _dependencies_source_linux:

Dependencies
------------

*eProsima Fast DDS Statistics Backend* has the following dependencies in a Linux environment:

* :ref:`fastDDS_source_linux`


.. _fastDDS_source_linux:

eProsima Fast DDS
^^^^^^^^^^^^^^^^^

To install it, see the `eProsima Fast DDS <https://fast-dds.docs.eprosima.com/en/latest/installation/binaries/binaries_linux.html#linux-binaries>`_
installation documentation.


.. _colcon_installation_linux:

Colcon installation
-------------------

colcon_ is a command line tool based on CMake_ for building sets of software packages.
To compile *eProsima Fast DDS Statistics Backend* and its dependencies with colcon_:

#. Install the ROS 2 development tools (colcon_ and vcstool_):

   .. code-block:: bash

       pip3 install -U colcon-common-extensions vcstool

   .. note::

       If this fails due to an Environment Error, add the :code:`--user` flag to the :code:`pip3` installation command.

#. Create a :code:`Fast-DDS-statistics-backend` directory and download the `repos` file used to install
   *eProsima Fast DDS Statistics Backend* and its dependencies:

   .. code-block:: bash

       mkdir ~/Fast-DDS-statistics-backend
       cd ~/Fast-DDS-statistics-backend
       wget https://raw.githubusercontent.com/eProsima/Fast-DDS-statistics-backend/main/fastdds_statistics_backend.repos
       mkdir src
       vcs import src < fastdds_statistics_backend.repos

#. Build the packages:

   .. code-block:: bash

       colcon build

.. note::

    Since colcon_ is based on CMake_, the CMake configuration options can be passed to the :code:`colcon build`
    command. For the specific syntax, see the
    `CMake specific arguments <https://colcon.readthedocs.io/en/released/reference/verb/build.html#cmake-specific-arguments>`_
    page of the colcon_ manual.

    The configuration can also be set in a
    `colcon.meta file <https://colcon.readthedocs.io/en/released/user/configuration.html?highlight=meta#meta-files>`_
    instead of on the CLI.
    The *eProsima Fast DDS Statistics Backend* repository already includes a `colcon.meta` file
    with the default configuration, which the user can adjust.


.. _cmake_installation_linux:

CMake installation
------------------

*eProsima Fast DDS Statistics Backend* can be compiled with CMake_,
either :ref:`locally <local_installation_source_linux>` or :ref:`globally <global_installation_source_linux>`.

.. _local_installation_source_linux:

Local installation
^^^^^^^^^^^^^^^^^^

#. Follow the `eProsima Fast DDS local installation guide <https://fast-dds.docs.eprosima.com/en/latest/installation/sources/sources_linux.html#local-installation>`_
   to install *eProsima Fast DDS* and all its dependencies.

#. Install *eProsima Fast DDS Statistics Backend*:

   .. code-block:: bash

       cd ~/Fast-DDS
       git clone https://github.com/eProsima/Fast-DDS-statistics-backend.git
       mkdir Fast-DDS-statistics-backend/build
       cd Fast-DDS-statistics-backend/build
       cmake ..  -DCMAKE_INSTALL_PREFIX=~/Fast-DDS/install -DCMAKE_PREFIX_PATH=~/Fast-DDS/install
       sudo cmake --build . --target install

.. note::

    By default, *eProsima Fast DDS Statistics Backend* does not compile tests.
    To build them, install `Gtest <https://github.com/google/googletest>`_
    and enable :ref:`the corresponding cmake option <cmake_options>`.


.. _global_installation_source_linux:

Global installation
^^^^^^^^^^^^^^^^^^^

#. Follow the `eProsima Fast DDS global installation guide <https://fast-dds.docs.eprosima.com/en/latest/installation/sources/sources_linux.html#global-installation>`_
   to install *eProsima Fast DDS* and all its dependencies.

#. Install *eProsima Fast DDS Statistics Backend*:

   .. code-block:: bash

       cd ~/Fast-DDS
       git clone https://github.com/eProsima/Fast-DDS-statistics-backend.git
       mkdir Fast-DDS-statistics-backend/build
       cd Fast-DDS-statistics-backend/build
       cmake ..
       cmake --build . --target install

.. _run_app_cmake_source_linux:

Run an application
^^^^^^^^^^^^^^^^^^

An application using *eProsima Fast DDS Statistics Backend* must be linked with the library
in the directory where the packages were installed.
For a system-wide installation this is :code:`/usr/local/lib/`
(for a local installation, adjust the directory accordingly).
There are two options:

* Prepare the environment locally:

  .. code-block:: bash

      export LD_LIBRARY_PATH=/usr/local/lib/

* Add it permanently to the :code:`PATH`:

  .. code-block:: bash

      echo 'export LD_LIBRARY_PATH=/usr/local/lib/' >> ~/.bashrc


.. External links

.. _colcon: https://colcon.readthedocs.io/en/released/
.. _CMake: https://cmake.org
.. _pip3: https://docs.python.org/3/installing/index.html
.. _wget: https://www.gnu.org/software/wget/
.. _git: https://git-scm.com/
.. _OpenSSL: https://www.openssl.org/
.. _Gtest: https://github.com/google/googletest
.. _vcstool: https://pypi.org/project/vcstool/
