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
  If Fast DDS has been compiled with statistics and they are explicitly enabled and statistical data are not correctly
  received, only few data arrive or even none, configure the Fast DDS endpoints publishing statistics data with a less
  restrictive memory constraints.
  Please check the following
  `documentation <https://fast-dds.docs.eprosima.com/en/latest/fastdds/statistics/dds_layer/troubleshooting.html#troubleshooting>`_
  for more details on how to do this.
