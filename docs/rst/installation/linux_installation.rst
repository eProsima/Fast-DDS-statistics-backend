.. _linux_installation:

Linux installation from sources
===============================

The instructions for installing the *eProsima Fast DDS Statistics Backend* from sources are provided in this page.
It is organized as follows:

.. contents::
    :local:
    :backlinks: none
    :depth: 2

.. _fastdds_backend_linux:

Fast DDS Statistics Backend installation
""""""""""""""""""""""""""""""""""""""""

This section describes the instructions for installing *eProsima Fast DDS Statistics Backend*
in a Linux environment from sources.
First of all, the :ref:`requirements_source_linux` and :ref:`dependencies_source_linux`
detailed below need to be met.
Afterwards, the user can choose whether to follow either the :ref:`colcon <colcon_installation_linux>`
or the :ref:`CMake <cmake_installation_linux>` installation instructions.

.. _requirements_source_linux:


Requirements
------------

The installation of *eProsima Fast DDS Statistics Backend* in a Linux environment from sources
requires the following tools to be installed in the system:

* :ref:`cmake_gcc_pip3_wget_git_source_linux`
* :ref:`gtest_source_linux` [optional]

.. _cmake_gcc_pip3_wget_git_source_linux:

CMake, g++, pip3, wget and git
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

These packages provide the tools required to install *eProsima Fast DDS Statistics Backend* and its dependencies
from command line.
Install CMake_, `g++ <https://gcc.gnu.org/>`_, pip3_, wget_ and git_ using the package manager of the appropriate
Linux distribution. For example, on Ubuntu use the command:

.. code-block:: bash

    sudo apt install cmake g++ python3-pip wget git

.. _gtest_source_linux:

Gtest
^^^^^

Gtest is a unit testing library for C++.
By default, *eProsima Fast DDS Statistics Backend* does not compile tests.
It is possible to activate them with the opportune
`CMake configuration options <https://cmake.org/cmake/help/v3.6/manual/cmake.1.html#options>`_
when calling colcon_ or CMake_.
For more details, please refer to the :ref:`cmake_options` section.
For a detailed description of the Gtest installation process, please refer to the
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

Please, refer to the `eProsima Fast DDS <https://fast-dds.docs.eprosima.com/en/latest/installation/binaries/binaries_linux.html#linux-binaries>`_
installation documentation to learn the installing procedure


.. _colcon_installation_linux:

Colcon installation
-------------------

colcon_ is a command line tool based on CMake_ aimed at building sets of software packages.
This section explains how to use it to compile *eProsima Fast DDS Statistics Backend* and its dependencies.

#. Install the ROS 2 development tools (colcon_ and vcstool_) by executing the following command:

   .. code-block:: bash

       pip3 install -U colcon-common-extensions vcstool

   .. note::

       If this fails due to an Environment Error, add the :code:`--user` flag to the :code:`pip3` installation command.

#. Create a :code:`Fast-DDS-statistics-backend` directory and download the `repos` file that will be used to install
   *eProsima Fast DDS Statistics Backend* and its dependencies:

   .. code-block:: bash

       mkdir ~/Fast-DDS-statistics-backend
       cd ~/Fast-DDS-statistics-backend
       wget https://raw.githubusercontent.com/eProsima/Fast-DDS-statistics-backend/v2.1.0/fastdds_statistics_backend.repos
       mkdir src
       vcs import src < fastdds_statistics_backend.repos

#. Build the packages:

   .. code-block:: bash

       colcon build

.. note::

    Being based on CMake_, it is possible to pass the CMake configuration options to the :code:`colcon build`
    command. For more information on the specific syntax, please refer to the
    `CMake specific arguments <https://colcon.readthedocs.io/en/released/reference/verb/build.html#cmake-specific-arguments>`_
    page of the colcon_ manual.

    Instead of passing CMake configuration options on the CLI, it is also possible to use a
    `colcon.meta file <https://colcon.readthedocs.io/en/released/user/configuration.html?highlight=meta#meta-files>`_
    to set the configuration.
    The *eProsima Fast DDS Statistics Backend* repository already includes a `colcon.meta` file
    with the default configuration, which can be tuned by the user.


.. _cmake_installation_linux:

CMake installation
------------------

This section explains how to compile *eProsima Fast DDS Statistics Backend* with CMake_,
either :ref:`locally <local_installation_source_linux>` or :ref:`globally <global_installation_source_linux>`.

.. _local_installation_source_linux:

Local installation
^^^^^^^^^^^^^^^^^^

#. Follow the `eProsima Fast DDS local installation guide <https://fast-dds.docs.eprosima.com/en/latest/installation/sources/sources_linux.html#local-installation>`_
   to install *eProsmia Fast DDS* and all its dependencies

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
    However, they can be activated by downloading and installing `Gtest <https://github.com/google/googletest>`_,
    and enabling :ref:`the corresponding cmake option <cmake_options>`.


.. _global_installation_source_linux:

Global installation
^^^^^^^^^^^^^^^^^^^

#. Follow the `eProsima Fast DDS global installation guide <https://fast-dds.docs.eprosima.com/en/latest/installation/sources/sources_linux.html#global-installation>`_
   to install *eProsmia Fast DDS* and all its dependencies

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

When running an instance of an application using *eProsima Fast DDS Statistics Backend*,
it must be linked with the library where the packages have been installed,
which in the case of system-wide installation  is: :code:`/usr/local/lib/`
(if local installation is used, adjust for the correct directory).
There are two possibilities:

* Prepare the environment locally by typing the command:

  .. code-block:: bash

      export LD_LIBRARY_PATH=/usr/local/lib/

* Add it permanently it to the :code:`PATH`, by typing:

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
