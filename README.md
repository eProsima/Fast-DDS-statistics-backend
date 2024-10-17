# Fast DDS Statistics Backend

<a href="http://www.eprosima.com"><img src="https://encrypted-tbn3.gstatic.com/images?q=tbn:ANd9GcSd0PDlVz1U_7MgdTe0FRIWD0Jc9_YH-gGi0ZpLkr-qgCI6ZEoJZ5GBqQ" align="left" hspace="8" vspace="2" width="100" height="100" ></a>

[![License](https://img.shields.io/github/license/eProsima/Fast-DDS-statistics-backend.svg)](https://opensource.org/licenses/Apache-2.0)
[![Releases](https://img.shields.io/github/v/release/eProsima/Fast-DDS-statistics-backend?sort=semver)](https://github.com/eProsima/Fast-DDS-statistics-backend/releases)
[![Issues](https://img.shields.io/github/issues/eProsima/Fast-DDS-statistics-backend.svg)](https://github.com/eProsima/Fast-DDS-statistics-backend/issues)
[![Forks](https://img.shields.io/github/forks/eProsima/Fast-DDS-statistics-backend.svg)](https://github.com/eProsima/Fast-DDS-statistics-backend/network/members)
[![Stars](https://img.shields.io/github/stars/eProsima/Fast-DDS-statistics-backend.svg)](https://github.com/eProsima/Fast-RTPS/stargazers)
[![test](https://github.com/eProsima/Fast-DDS-statistics-backend/actions/workflows/test.yml/badge.svg)](https://github.com/eProsima/Fast-DDS-statistics-backend/actions/workflows/test.yml)
[![codecov](https://codecov.io/gh/eProsima/Fast-DDS-statistics-backend/branch/v2.1.0/graph/badge.svg?token=6NA5PVA9QL)](https://codecov.io/gh/eProsima/Fast-DDS-statistics-backend)


*eProsima Fast DDS Statistics Backend* is a C++ library that provides collection and procession the statistics measurements reported by *Fast DDS Statistics Module*.
The Backend provides a easy-to-use and user friendly API that abstracts the details of receiving and processing
*Fast DDS* statistical data, so that applications can focus on providing great user experiences.
*Fast DDS Statistics Backend* allows for:

* Monitoring the various aspects reported by the Statistics Module for as many DDS Domains and *Fast DDS* Discovery Server network as desired.
* Retrieve the DDS network graph.
* Retrieve QoS settings about the DDS entities deployed in the network.
* Receive notification on statistics updates for various events using a listener-callback mechanism.
* Check whether an entity is active.
* Retrieve statistics on all the aspects of the communication reported by the *Fast DDS Statistics Module*, with the possibility of specifying time windows, bins, and different statistics to apply.

## Commercial support

Looking for commercial support? Write us to info@eprosima.com

Find more about us at [eProsima’s webpage](https://eprosima.com/).

## Installation guide

This tutorial shows how to build and install *Fast DDS Statistics Backend* using [colcon](https://colcon.readthedocs.io), a command line tool to build sets of software packages.
To do so, `colcon` and `vcstool` need to be installed:

```bash
pip install -U colcon-common-extensions vcstool
```

### Dependencies

*Fast DDS Statistics Backend* depends on [Fast DDS](https://github.com/eProsima/Fast-DDS) and [Fast CDR](https://github.com/eProsima/Fast-CDR).
For simplicity, this tutorial will build these dependencies alongside the backend.
More advanced users can build or link to this packages separately.
Install *Fast DDS* dependencies running:

```bash
sudo apt update
sudo apt install -y \
    libasio-dev \
    libtinyxml2-dev
```

### Build and install

```bash
# Change directory to the location where the colcon workspace will be created
cd <path_to_ws>
# Create workspace directory
mkdir -p fastdds_statistics_backend_ws/src
cd fastdds_statistics_backend_ws
# Get workspace setup file
wget https://raw.githubusercontent.com/eProsima/Fast-DDS-statistics-backend/v2.1.0/fastdds_statistics_backend.repos
# Get build options file
wget https://raw.githubusercontent.com/eProsima/Fast-DDS-statistics-backend/v2.1.0/colcon.meta
# Download repositories
vcs import src < fastdds_statistics_backend.repos
# Build the workspace
colcon build
```

### Build and install documentation

*Fast DDS Statistics Backend Documentation* is generated using [Sphinx](https://www.sphinx-doc.org).

#### Dependencies

Before being able to build the documentation, some dependencies need to be installed:

```bash
sudo apt update
sudo apt install -y \
    doxygen \
    python3 \
    python3-pip \
    python3-venv \
    python3-sphinxcontrib.spelling \
    imagemagick
```

#### Virtual environment

This tutorial will create a python3 virtual environment to avoid polluting user's python installation.

```bash
# Change directory to the root of the colcon workspace
cd <path_to_ws>/fastdds_statistics_backend_ws
# Create a python3 virtual environment
python3 -m venv fastdds_statistics_backend_venv
# Activate the environment
source fastdds_statistics_backend_venv/bin/activate
# Install dependencies within the environment
pip3 install -r src/fastdds_statistics_backend/docs/requirements.txt
# Patch sphinx to correct an issue with warnings when generating the API reference
cd fastdds_statistics_backend_venv/lib/<python3-version>/site-packages
curl https://patch-diff.githubusercontent.com/raw/sphinx-doc/sphinx/pull/7851.diff | git apply
```

The version of python3 used in the virtual environment can be seen by running the following command within the virtual environment:

```bash
python3 -V
```

#### Build documentation

To enable the documentation building, edit `<path_to_ws>/fastdds_statistics_backend_ws/colcon.meta`
to set `-DBUILD_DOCS` to `ON` within the `fastdds_statistics_backend` project.
Then, activate the virtual environment and build the documentation.

```bash
# Change directory to the root of the colcon workspace
cd <path_to_ws>/fastdds_statistics_backend_ws
# Activate the environment
source fastdds_statistics_backend_venv/bin/activate
# Build documentation
colcon build
```

Once the build completes, the generated documentation can be found in `<path_to_ws>/fastdds_statistics_backend_ws/install/fastdds_statistics_backend/docs/fastdds_statistics_backend/sphinx/html/index.html`

##### Simulating the Read The Docs build

Read the Docs generates the documentation using Sphinx and [conf.py](docs/conf.py).
This means that it does not execute any colcon or CMake commands.
[conf.py](docs/conf.py) provides some extra logic to generate the Doxygen documentation when running on a Read the Docs environment.
This is done by means of the environment variable `READTHEDOCS`. When this variable is set to `True`, [conf.py](docs/conf.py) will detect the Read The Docs environment and act accordingly.
Simulating the ReadTheDocs operation can be achieved from within the repository, independently from the `colcon` build.
To do that, make sure there is no `build` directory on the repository's root directory.
Then, set `READTHEDOCS` and run sphinx:

```bash
# Source the python virtual environment
source <path_to_ws>/fastdds_statistics_backend_venv/bin/activate
# Change directories to the repository directory
cd <path_to_ws>/src/fastdds_statistics_backend
# Make sure that the are no build directories
rm -rf build
# Run sphinx
READTHEDOCS=True sphinx-build \
    -b html \
    -D breathe_projects.FastDDS=<abs_path_to_ws>/src/fastdds_statistics_backend/build/doxygen/xml \
    -d <abs_path_to_ws>/src/fastdds_statistics_backend/build/doctrees \
    docs <abs_path_to_ws>/src/fastdds_statistics_backend/build/html
```

#### Troubleshooting

Python versions 3.7 and newer produce `Duplicate declaration` and `Error when parsing function declaration` warnings when building the documentation.
This is due to a difference in the Sphinx 3.0.3 module code which prevents the patch from working, even though the version is the same.
