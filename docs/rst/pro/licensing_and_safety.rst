.. include:: ../exports/alias.include

.. _pro_licensing_safety:

Licensing and Safety |Pro|
==========================

License check
---------------

|check_license-api| validates the eProsima license for *Fast DDS Statistics Backend Pro* at process startup. It
is a free function, not a member of |StatisticsBackend-api|, since it validates the library itself rather than
any monitor state.

The license file is read from ``$FASTDDSHOME/licenses/eprosima_license.lic``, falling back to
``~/.config/fastdds/licenses/eprosima_license.lic`` on Linux when ``FASTDDSHOME`` is unset. This file on disk is
the only license-providing mechanism for this library; there is no compile-time embedded alternative. It holds
a JSON payload signed once with an RSA-PSS key, whose ``products`` object may list a ``fastddspro`` entry, a
``safedds`` entry, or both - the same license-generation framework produces both product types in this one
shared format.

.. note::

   *Safe DDS* itself separately consumes a different, compiled-in binary blob for its own runtime licensing,
   generated from the same source data. That format is unrelated to this file and is never read by
   |check_license-api|.

The signature is verified once, then each present product entry is checked against its own validity window; the
license is considered valid if at least one of the two products currently validates:

.. literalinclude:: /code/StatisticsBackendProTests.cpp
    :language: c++
    :start-after: //CONF-PRO-CHECK-LICENSE-EXAMPLE
    :end-before: //!
    :dedent: 8

The cryptographic check itself runs at most once per process lifetime - every subsequent call to
|check_license-api| returns the cached result, so it is cheap to call from more than one place (for example,
both at startup and again before enabling a Pro-only feature).

Proxy-participant safety check
----------------------------------

Statistics messages from one DDS domain can reach the monitor's own domain when something else - most notably a
*Fast DDS Router* - bridges traffic between domains. The entities behind that bridge are reported as *proxy*
entities: they appear in the local domain's data as if they belonged to it, but they were actually discovered
through a proxy message rather than directly.

|has_proxy_participants-api| reports whether a given |DomainId-api| currently has at least one proxy or inferred
participant, i.e. whether it is - at least in part - a *remote* domain from the point of view of the monitor:

.. literalinclude:: /code/StatisticsBackendProTests.cpp
    :language: c++
    :start-after: //CONF-PRO-HAS-PROXY-PARTICIPANTS-EXAMPLE
    :end-before: //!
    :dedent: 8

An application can use this as a safety check before creating a second, independent monitor on what might
actually be the same physical domain reached through a different path: creating a monitor on a domain that
already has proxy participants coming from another monitored domain would otherwise double-count the same
underlying entities.
