.. include:: ../exports/alias.include

.. _pro_licensing_safety:

Licensing and Safety |Pro|
==========================

|check_license-api| validates the eProsima license for *Fast DDS Statistics Backend Pro* at process startup. It
is a free function, not a member of |StatisticsBackend-api|, since it validates the library itself rather than
any monitor state.

The license file is read from ``$FASTDDSHOME/licenses/eprosima_license.lic``, falling back to
``~/.config/fastdds/licenses/eprosima_license.lic`` on Linux when ``FASTDDSHOME`` is unset. It accepts a
*Fast DDS Pro* license, a *Safe DDS* license, or both; the license is considered valid if at least one of them
currently validates. If neither is present or valid, |check_license-api| returns ``false``:

.. literalinclude:: /code/StatisticsBackendProTests.cpp
    :language: c++
    :start-after: //CONF-PRO-CHECK-LICENSE-EXAMPLE
    :end-before: //!
    :dedent: 8

The cryptographic check runs at most once per process lifetime. Every subsequent call to |check_license-api|
returns the cached result, so it is cheap to call from more than one place (for example, both at startup and
again before enabling a Pro-only feature).
