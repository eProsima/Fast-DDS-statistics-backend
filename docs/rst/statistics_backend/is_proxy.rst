.. include:: ../exports/alias.include

.. _statistics_backend_is_proxy:

Check whether an entity was discovered through a proxy
--------------------------------------------------------

Statistics messages from one DDS domain can reach the monitor's own domain when something else, most notably a
*Fast DDS Router*, bridges traffic between domains. The entities behind that bridge are reported as *proxy*
entities: they appear in the local domain's data as if they belonged to it, but were discovered through a proxy
message instead of directly.

|is_proxy-api| returns whether a given entity was discovered this way:

.. literalinclude:: /code/StatisticsBackendTests.cpp
    :language: c++
    :start-after: //CONF-IS-PROXY-EXAMPLE
    :end-before: //!
    :dedent: 8

.. _statistics_backend_has_proxy_participants:

Proxy-participant safety check |Pro|
--------------------------------------

*Fast DDS Statistics Backend Pro* adds |has_proxy_participants-api|, which reports whether a given
|DomainId-api| currently has at least one proxy or inferred participant, i.e. whether it is, at least in part,
a *remote* domain from the point of view of the monitor:

.. literalinclude:: /code/StatisticsBackendProTests.cpp
    :language: c++
    :start-after: //CONF-PRO-HAS-PROXY-PARTICIPANTS-EXAMPLE
    :end-before: //!
    :dedent: 8

An application can use this as a safety check before creating a second, independent monitor on what might be
the same physical domain reached through a different path. Creating a monitor on a domain that already has
proxy participants coming from another monitored domain would otherwise double-count the same underlying
entities.
