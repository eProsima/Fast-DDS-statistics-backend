.. add orphan tag when new info added to this file

.. :orphan:

###################
Forthcoming Version
###################

Next release will include the following **API extensions**:

* `StatisticsBackend::get_status` returns the entity status level of a given id.
* `StatisticsBackend::get_info` returns domain participant and endpoint app info.
* `StatisticsBackend::get_domain_view_graph` returns the domain view graph of a given domain.
* `StatisticsBackend::regenerate_domain_graph` regenerates the domain view graph of a given domain.
* `StatisticsBackend::get_status_data` returns an specific status data of a given id.
* Added `status` attribute for entities.
* Added database `domain_view_graph` map.
* Added monitor service topic status data storing and processing.
* Retrieve physical information from discovery packets.
* Physical related entities with an empty name are given the name `Unknown` by default.
