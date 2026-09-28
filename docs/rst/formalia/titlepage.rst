##################################################
eProsima Fast DDS Statistics Backend Documentation
##################################################

.. image:: /rst/figures/logo.png
  :height: 100px
  :width: 100px
  :align: left
  :alt: eProsima
  :target: http://www.eprosima.com/

*eProsima Fast DDS Statistics Backend* is a C++ library to collect data from the *Fast DDS Statistics module* and
generate statistical information to be used by applications.

##################
Commercial support
##################

Looking for commercial support? Write us to info@eprosima.com.

Find more about us at `eProsima's webpage <https://eprosima.com/>`_.

##################
Feature Comparison
##################

The following table summarizes the differences between *Fast DDS Statistics Backend* and *Fast DDS Statistics
Backend Pro*. See :ref:`statistics_backend_pro` for the full description of every Pro-only feature listed here.

.. raw:: html

  <style>
    .md-table {
      width: 100%;
      border-collapse: collapse;
      font-family: sans-serif;
      font-size: 0.95em;
    }
    .md-table th, .md-table td {
      border: 1px solid var(--color-background-border, #dfe2e5);
      padding: 10px 16px;
      text-align: left;
      color: var(--color-foreground-primary, inherit);
      background-color: var(--color-background-primary, transparent);
    }
    .md-table thead tr {
      background-color: var(--color-background-secondary, #f6f8fa) !important;
      font-weight: bold;
      text-align: center;
    }
    .md-table tbody tr:nth-child(even) td {
      background-color: var(--color-background-secondary, #f6f8fa);
    }
    .md-table tbody tr:hover td {
      background-color: var(--color-background-hover, #eef2f5);
    }
  </style>

  <table class="md-table">
    <thead>
      <tr>
        <th style="width:30%"></th>
        <th style="width:35%; text-align:center;">Fast DDS Statistics Backend Pro</th>
        <th style="width:35%; text-align:center;">Fast DDS Statistics Backend (Community)</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <th>Target usage</th>
        <td>Production systems, robotics, industrial, defense</td>
        <td>Evaluation, prototyping, development, research</td>
      </tr>
      <tr>
        <th>License</th>
        <td>Commercial (eProsima Software License Agreement)</td>
        <td>Open Source (Apache-2.0)</td>
      </tr>
      <tr>
        <th>Type registration from IDL / TypeObject</th>
        <td>✅</td>
        <td>❌</td>
      </tr>
      <tr>
        <th>Publish samples on a topic</th>
        <td>✅ Topic Publisher API</td>
        <td>❌</td>
      </tr>
      <tr>
        <th>Topic spy</th>
        <td>✅ JSON spy with source timestamps, plus raw (non-JSON) high-throughput spy</td>
        <td>✅ JSON spy only, no source timestamps</td>
      </tr>
      <tr>
        <th>Topic type schema (JSON, for building forms/mappings)</th>
        <td>✅</td>
        <td>❌</td>
      </tr>
      <tr>
        <th>Statistics DataReaders</th>
        <td>✅ Enabled/disabled on demand per topic</td>
        <td>⚠️ Always-on for every enabled statistics topic</td>
      </tr>
      <tr>
        <th>Multi-statistic queries</th>
        <td>✅ Several StatisticKind values per get_data() call</td>
        <td>⚠️ One StatisticKind per get_data() call</td>
      </tr>
      <tr>
        <th>Proxy-domain safety check</th>
        <td>✅ has_proxy_participants()</td>
        <td>❌</td>
      </tr>
      <tr>
        <th>Clear monitor (entities + data, reusable domain)</th>
        <td>✅ Fully implemented</td>
        <td>❌ Not yet implemented (no-op)</td>
      </tr>
      <tr>
        <th>DataKind coverage</th>
        <td>⚠️ 7 fewer kinds (Fast DDS Pro no longer publishes them)</td>
        <td>✅ Full set</td>
      </tr>
      <tr>
        <th>License validation API</th>
        <td>✅ check_license()</td>
        <td>Not applicable</td>
      </tr>
      <tr>
        <th>Support</th>
        <td>✅ Direct engineering support</td>
        <td>❌ Community-based</td>
      </tr>
      <tr>
        <th>Maintenance / LTS</th>
        <td>✅ Long-term support with backports</td>
        <td>❌ No guaranteed maintenance</td>
      </tr>
    </tbody>
  </table>

  <div style="height: 2em;"></div>

########
Overview
########

This database-like tool enhances the monitoring of the health of *Fast DDS* entities. Additionally, it offers a
useful depiction of the *Fast DDS* system in a graph-like format. This visualization aids in understanding the
system's structure and behavior in an accessible manner.

.. warning::

  To monitor a DDS network deployed using the *Fast DDS* library, it must be compiled with statistics and
  the statistics module must be explicitly enabled. See `Statistics Module DDS Layer
  <https://fast-dds.docs.eprosima.com/en/latest/fastdds/statistics/dds_layer/statistics_dds_layer.html>`_
  for more details.

.. warning::
  If *Fast DDS* has been compiled with statistics and they are explicitly enabled and statistical data are not correctly
  received, only few data arrive or even none, configure the Fast DDS endpoints publishing statistics data with a less
  restrictive memory constraints.
  Please check the following
  `documentation <https://fast-dds.docs.eprosima.com/en/latest/fastdds/statistics/dds_layer/troubleshooting.html#troubleshooting>`_
  for more details on how to do this.




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
* :ref:`Fast DDS Statistics Backend Pro <statistics_backend_pro>`
* :ref:`Fast DDS Statistics Backend <statistics_backend>`
* :ref:`Release Notes <release_notes>`
