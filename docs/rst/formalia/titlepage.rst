.. raw:: html

  <h1>
    eProsima Fast DDS Statistics Backend Documentation
  </h1>

.. image:: /rst/figures/logo.png
  :height: 100px
  :width: 100px
  :align: left
  :alt: eProsima
  :target: http://www.eprosima.com/

Description
^^^^^^^^^^^

*eProsima Fast DDS Statistics Backend* is a C++ library to collect data from the *Fast DDS Statistics module*,
generate statistical information to be used by applications.

This database-like tool enhances the monitoring of the health of *Fast DDS* entities. Additionally, it offers a
useful depiction of the Fast DDS system in a graph-like format. This visualization aids in understanding the
system's structure and behavior in an accessible manner.

.. warning::

  In order to monitor a DDS network deployed using *Fast DDS* library, compiling the latter with statistics and
  explicitly activating the statistics module is required. See `Statistics Module DDS Layer
  <https://fast-dds.docs.eprosima.com/en/latest/fastdds/statistics/dds_layer/statistics_dds_layer.html>`_
  for more details.

.. warning::
  If Fast DDS has been compiled with statistics and they are explicitly enabled and statistical data are not correctly
  received, only few data arrive or even none, configure the Fast DDS endpoints publishing statistics data with a less
  restrictive memory constraints.
  Please check the following
  `documentation <https://fast-dds.docs.eprosima.com/en/latest/fastdds/statistics/dds_layer/troubleshooting.html#troubleshooting>`_
  for more details on how to do this.


###############################
Contacts and Commercial support
###############################

Find more about us at `eProsima's webpage <https://eprosima.com/>`_.

Support available at:

* Email: support@eprosima.com
* Phone: +34 91 804 34 48

#################################
Contributing to the documentation
#################################

*Fast DDS Statistics Backend Documentation* is an open source project, and as such all contributions, both in the form of
feedback and content generation, are most welcomed.
To make such contributions, please refer to the
`Contribution Guidelines <https://github.com/eProsima/all-docs/blob/master/CONTRIBUTING.md>`_ hosted in our GitHub
repository.

##############################
Structure of the documentation
##############################

This documentation is organized into the sections below.

* :ref:`Installation Manual <linux_installation>`
* :ref:`Fast DDS Statistics Backend <statistics_backend>`
* :ref:`Release Notes <release_notes>`
