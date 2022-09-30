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

*eProsima Fast DDS Statistics Backend* is a C++ library to collect data from the *Fast DDS Statistics module*, and
generate statistical information to be used by applications.

.. warning::
  This library is in the early stages of development.
  For this reason, the API should not be considered stable, as API breaks may occur before the first official release.
  Furthermore, several features may not be implemented yet.

.. warning::
  Be aware that the statistics module is not compiled and used by default by *Fast DDS* but it has to be specifically
  configured to send statistical data of an specific entity.
  Therefore, *eProsima Fast DDS* library must be compiled with `FASTDDS_STATISTICS` CMake option enabled and use either
  the `FASTDDS_STATISTICS` environment variable or the DomainParticipantQos to activate the statistics DataWriters.
  For more information, please refer to the
  `Statistics Module documentation <https://fast-dds.docs.eprosima.com/en/latest/fastdds/statistics/statistics.html#statistics-module>`_.

.. warning::
  If Fast DDS has been compiled with statistics and they are explicitly enabled and statistical data are not correctly
  received, only few data arrive or even none, configure the Fast DDS endpoints publishing statistics data with a less
  restrictive memory constraints.
  Please check the following
  `documentation <https://fast-dds.docs.eprosima.com/en/latest/fastdds/statistics/dds_layer/qos.html?highlight=Troubleshooting#statistics-qos-troubleshooting>`_
  for more details on how to do this.