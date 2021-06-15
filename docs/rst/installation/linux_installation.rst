.. _linux_installation:

Linux installation
==================

The instructions for installing the Fast DDS statistics backend from sources are provided in this page.
It is organized as follows:

.. contents::
    :local:
    :backlinks: none
    :depth: 2

.. _fastdds_backend_linux:

Fast DDS Statistics Backend installation
""""""""""""""""""""""""""""""""""""""""

This section describes the instructions for installing *eProsima Fast DDS Statistics Backend*
in a Linux environment from sources. The following packages will be installed:

* :code:`foonathan_memory_vendor`, an STL compatible C++ memory allocator
  `library <https://github.com/foonathan/memory>`_.
* :code:`fastcdr`, a C++ library that serializes according to the
  `standard CDR <https://www.omg.org/cgi-bin/doc?formal/02-06-51>`_ serialization mechanism.
* :code:`fastrtps`, the core library of *eProsima Fast DDS* library.

First of all, the :ref:`requirements_sl` and :ref:`dependencies_sl` detailed below need to be met.
Afterwards, the user can choose whether to follow either the :ref:`colcon <colcon_installation_linux>`)
or the :ref:`CMake <cmake_installation_linux>`) installation instructions.

.. _requirements_sl:


Requirements
------------

The installation of *eProsima Fast DDS Statistics Backend* in a Linux environment from sources
requires the following tools to be installed in the system:

* :ref:`cmake_gcc_pip3_wget_git_sl`
* :ref:`gtest_sl` [optional]

.. _cmake_gcc_pip3_wget_git_sl:

CMake, g++, pip3, wget and git
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

These packages provide the tools required to install *eProsima Fast DDS Statistics Backend* and its dependencies
from command line.
Install CMake_, `g++ <https://gcc.gnu.org/>`_, pip3_, wget_ and git_ using the package manager of the appropriate
Linux distribution. For example, on Ubuntu use the command:

.. code-block:: bash

    sudo apt install cmake g++ python3-pip wget git

.. _gtest_sl:

Gtest
^^^^^

GTest is a unit testing library for C++.
By default, *eProsima Fast DDS Statistics Backend* does not compile tests.
It is possible to activate them with the opportune
`CMake configuration options <https://cmake.org/cmake/help/v3.6/manual/cmake.1.html#options>`_
when calling colcon_ or CMake_.
For more details, please refer to the :ref:`cmake_options` section.
For a detailed description of the Gtest installation process, please refer to the
`Gtest Installation Guide <https://github.com/google/googletest>`_.

.. note::

    *eProsima Fast DDS Statistics Backend* depends on Gtest release-1.10.0 or later.


.. _dependencies_sl:

Dependencies
------------

*eProsima Fast DDS Statistics Backend* has the following dependencies in a Linux environment:

* :ref:`fastDDS_sl`


.. _fastDDS_sl:

eProsima Fast DDS
^^^^^^^^^^^^^^^^^

Please, refer to the `*eProsima Fast DDS*<https://fast-dds.docs.eprosima.com/en/latest/installation/binaries/binaries_linux.html#linux-binaries>`
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

#. Create a :code:`Fast-DDS-statistics-backend` directory and download the repos file that will be used to install
   *eProsima Fast DDS Statistics Backend* and its dependencies:

   .. |wget-statistics-backend| replace:: wget https://raw.githubusercontent.com/eProsima/Fast-DDS-statistics-backend/master/fastdds_statistics_backend.repos

   .. code-block:: bash

       mkdir ~/Fast-DDS-statistics-backend
       cd ~/Fast-DDS-statistics-backend
       |wget-statistics-backend|
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

.. _run_app_colcon_sl:

Run an application
^^^^^^^^^^^^^^^^^^

When running an instance of an application using *eProsima Fast DDS Statistics Backend*,
the colcon overlay built in the dedicated :code:`Fast-DDS-statistics-backend` directory
must be sourced.
There are two possibilities:

* Every time a new shell is opened, prepare the environment locally by typing the
  command:

  .. code-block:: bash

      source ~/Fast-DDS-statistics-backend/install/setup.bash

* Add the sourcing of the colcon overlay permanently to the :code:`PATH`, by typing the following:

  .. code-block:: bash

      echo 'source ~/Fast-DDS-statistics-backend/install/setup.bash' >> ~/.bashrc


.. _cmake_installation_linux:

CMake installation
------------------

This section explains how to compile *eProsima Fast DDS Statistics Backend* with CMake_,
either :ref:`locally <local_installation_sl>` or :ref:`globally <global_installation_sl>`.

.. _local_installation_sl:

Local installation
^^^^^^^^^^^^^^^^^^

#. Follow the `*eProsima Fast DDS* local installation guide<https://fast-dds.docs.eprosima.com/en/latest/installation/sources/sources_linux.html#local-installation>`
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
    However, they can be activated by downloading and installing `Gtest <https://github.com/google/googletest>`_.


.. _global_installation_sl:

Global installation
^^^^^^^^^^^^^^^^^^^

#. Follow the `*eProsima Fast DDS* global installation guide<https://fast-dds.docs.eprosima.com/en/latest/installation/sources/sources_linux.html#global-installation>`
   to install *eProsmia Fast DDS* and all its dependencies

#. Install *eProsima Fast DDS Statistics Backend*:

   .. code-block:: bash

       cd ~/Fast-DDS
       git clone https://github.com/eProsima/Fast-DDS-statistics-backend.git
       mkdir Fast-DDS-statistics-backend/build
       cd Fast-DDS-statistics-backend/build
       cmake ..
       sudo cmake --build . --target install

.. _run_app_cmake_sl:

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
