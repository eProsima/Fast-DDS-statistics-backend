// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <gtest_aux.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/StatisticsBackend.hpp>
#include <fastdds_statistics_backend/types/types.hpp>
#include <database/database_queue.hpp>
#include <database/database.hpp>
#include <Monitor.hpp>
#include <StatisticsBackendData.hpp>

using ::testing::_;
using ::testing::Invoke;
using ::testing::AnyNumber;
using ::testing::Throw;
using ::testing::Mock;

using namespace eprosima::statistics_backend;

using DiscoveryStatus = details::StatisticsBackendData::DiscoveryStatus;
using ArgumentChecker = std::function<void (
                    EntityId,
                    EntityId,
                    const DomainListener::Status& status)>;

struct EntityDiscoveryArgs
{
    EntityDiscoveryArgs (
            ArgumentChecker func)
        : callback_(func)
    {
    }

    EntityDiscoveryArgs ()
    {
    }

    void on_discovery(
            EntityId base_entity_id,
            EntityId discovered_entity_id,
            const DomainListener::Status& status)
    {
        // Save the arguments for future reference
        base_entity_id_ = base_entity_id;
        discovered_entity_id_ = discovered_entity_id;
        status_ = status;

        // Call the callback with the checks
        callback_(base_entity_id, discovered_entity_id, status);
    }

    ArgumentChecker callback_;
    EntityId base_entity_id_;
    EntityId discovered_entity_id_;
    DomainListener::Status status_;
};


class MockedPhysicalListener : public PhysicalListener
{
public:

    MOCK_METHOD3(on_host_discovery, void(
                EntityId participant_id,
                EntityId host_id,
                const Status& status));

    MOCK_METHOD3(on_user_discovery, void(
                EntityId participant_id,
                EntityId user_id,
                const Status& status));

    MOCK_METHOD3(on_process_discovery, void(
                EntityId participant_id,
                EntityId process_id,
                const Status& status));

    MOCK_METHOD3(on_locator_discovery, void(
                EntityId participant_id,
                EntityId locator_id,
                const Status& status));

    MOCK_METHOD3(on_topic_discovery, void(
                EntityId domain_id,
                EntityId topic_id,
                const Status& status));

    MOCK_METHOD3(on_participant_discovery, void(
                EntityId domain_id,
                EntityId participant_id,
                const Status& status));

    MOCK_METHOD3(on_datareader_discovery, void(
                EntityId domain_id,
                EntityId datareader_id,
                const Status& status));

    MOCK_METHOD3(on_datawriter_discovery, void(
                EntityId domain_id,
                EntityId datawriter_id,
                const Status& status));

    MOCK_METHOD3(on_data_available, void(
                EntityId domain_id,
                EntityId entity_id,
                DataKind data_kind));

};

class MockedDomainListener : public DomainListener
{
public:

    MOCK_METHOD3(on_topic_discovery, void(
                EntityId domain_id,
                EntityId topic_id,
                const Status& status));

    MOCK_METHOD3(on_participant_discovery, void(
                EntityId domain_id,
                EntityId participant_id,
                const Status& status));

    MOCK_METHOD3(on_datareader_discovery, void(
                EntityId domain_id,
                EntityId datareader_id,
                const Status& status));

    MOCK_METHOD3(on_datawriter_discovery, void(
                EntityId domain_id,
                EntityId datawriter_id,
                const Status& status));

    MOCK_METHOD3(on_data_available, void(
                EntityId domain_id,
                EntityId entity_id,
                DataKind data_kind));
};

class calling_user_listeners_tests_physical_entities : public ::testing::TestWithParam<std::tuple<EntityKind,
            CallbackKind>>
{
public:

    calling_user_listeners_tests_physical_entities()
        : entity_kind_(std::get<0>(GetParam()))
        , callback_kind_(std::get<1>(GetParam()))

    {
        // Set the profile to ignore discovery data from other processes
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_XML_profiles_file("profile.xml");
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_profiles();
    }

    ~calling_user_listeners_tests_physical_entities()
    {
        StatisticsBackend::set_physical_listener(
            nullptr,
            CallbackMask::none(),
            DataKindMask::none());

        details::StatisticsBackendData::reset_instance();
    }

    MockedPhysicalListener physical_listener_;
    EntityDiscoveryArgs discovery_args_;
    EntityKind entity_kind_;
    CallbackKind callback_kind_;

    enum ListenerKind
    {
        NONE,
        PHYSICAL,
    };

    void test_entity_discovery(
            ListenerKind listener_kind,
            ArgumentChecker checker = [] (
                EntityId,
                EntityId,
                const DomainListener::Status&)
    {
    })
    {
        // Set the callback of the expectations
        discovery_args_.callback_ = checker;

        switch (entity_kind_)
        {
            case EntityKind::HOST:
            {
                // Set the expectations
                if (listener_kind == PHYSICAL)
                {
                    EXPECT_CALL(physical_listener_, on_host_discovery(EntityId(0), EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &EntityDiscoveryArgs::on_discovery));
                }
                else
                {
                    EXPECT_CALL(physical_listener_, on_host_discovery(_, _, _)).Times(0);
                }

                // Execution
                details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                    EntityId(0),
                    EntityId(1),
                    EntityKind::HOST);

                break;
            }
            case EntityKind::USER:
            {
                if (listener_kind == PHYSICAL)
                {
                    EXPECT_CALL(physical_listener_, on_user_discovery(EntityId(0), EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &EntityDiscoveryArgs::on_discovery));
                }
                else
                {
                    EXPECT_CALL(physical_listener_, on_user_discovery(_, _, _)).Times(0);
                }

                // Execution
                details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                    EntityId(0),
                    EntityId(1),
                    EntityKind::USER);

                break;
            }
            case EntityKind::PROCESS:
            {
                if (listener_kind == PHYSICAL)
                {
                    EXPECT_CALL(physical_listener_, on_process_discovery(EntityId(0), EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &EntityDiscoveryArgs::on_discovery));
                }
                else
                {
                    EXPECT_CALL(physical_listener_, on_process_discovery(_, _, _)).Times(0);
                }

                // Execution
                details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                    EntityId(0),
                    EntityId(1),
                    EntityKind::PROCESS);

                break;
            }
            case EntityKind::LOCATOR:
            {
                if (listener_kind == PHYSICAL)
                {
                    EXPECT_CALL(physical_listener_, on_locator_discovery(EntityId(0), EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &EntityDiscoveryArgs::on_discovery));
                }
                else
                {
                    EXPECT_CALL(physical_listener_, on_locator_discovery(_, _, _)).Times(0);
                }

                // Execution
                details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                    EntityId(0),
                    EntityId(1),
                    EntityKind::LOCATOR);

                break;
            }
            default:
            {
                FAIL() << "This fixture can be used for physical entities only";
            }
        }

        Mock::VerifyAndClearExpectations(&physical_listener_);
    }

    /*
     * This method extends the tests for the testcases where there is no callback triggered
     * in the starting configuration.
     *
     * It sets the appropriate physical listener and retests.
     */
    void extend_no_callback_tests()
    {
        // Set the physical listener and test
        CallbackMask mask = CallbackMask::none();
        mask.set(callback_kind_);
        StatisticsBackend::set_physical_listener(
            &physical_listener_,
            mask,
            DataKindMask::all());

        // Expectation: Only the physical listener is called
        test_entity_discovery(PHYSICAL,
                [&](
                    EntityId domain_id,
                    EntityId entity_id,
                    const DomainListener::Status& status)
                {
                    EXPECT_EQ(0, domain_id);
                    EXPECT_EQ(1, entity_id);
                    EXPECT_EQ(2, status.total_count);
                    EXPECT_EQ(2, status.total_count_change);
                    EXPECT_EQ(2, status.current_count);
                    EXPECT_EQ(2, status.current_count_change);
                });

        // Expectation: Only the physical listener is called again
        test_entity_discovery(PHYSICAL,
                [&](
                    EntityId domain_id,
                    EntityId entity_id,
                    const DomainListener::Status& status)
                {
                    EXPECT_EQ(0, domain_id);
                    EXPECT_EQ(1, entity_id);
                    EXPECT_EQ(3, status.total_count);
                    EXPECT_EQ(1, status.total_count_change);
                    EXPECT_EQ(3, status.current_count);
                    EXPECT_EQ(1, status.current_count_change);
                });
    }

};

TEST_P(calling_user_listeners_tests_physical_entities, entity_discovered)
{
    CallbackMask mask = CallbackMask::none();
    mask.set(callback_kind_);
    StatisticsBackend::set_physical_listener(
        &physical_listener_,
        mask,
        DataKindMask::all());

    // Expectation: The physical listener is called
    test_entity_discovery(PHYSICAL,
            [&](
                EntityId participant_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, participant_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    // Expectation: The physical listener is called again
    test_entity_discovery(PHYSICAL,
            [&](
                EntityId participant_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, participant_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });
}

TEST_P(calling_user_listeners_tests_physical_entities, entity_discovered_not_in_mask)
{
    CallbackMask mask = CallbackMask::all();
    mask ^= callback_kind_;
    StatisticsBackend::set_physical_listener(
        &physical_listener_,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    test_entity_discovery(NONE);

    extend_no_callback_tests();
}

TEST_P(calling_user_listeners_tests_physical_entities, entity_discovered_no_listener)
{
    CallbackMask mask = CallbackMask::none();
    mask.set(callback_kind_);
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    test_entity_discovery(NONE);

    extend_no_callback_tests();
}

TEST_P(calling_user_listeners_tests_physical_entities, entity_discovered_no_listener_not_in_mask)
{
    CallbackMask mask = CallbackMask::all();
    mask ^= callback_kind_;
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    test_entity_discovery(NONE);

    extend_no_callback_tests();
}

#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_SUITE_P(x, y, z)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_CASE_P(x, y, z)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(
    calling_user_listeners_tests_physical_entities,
    calling_user_listeners_tests_physical_entities,
    ::testing::Values(
        std::make_tuple(EntityKind::HOST, CallbackKind::ON_HOST_DISCOVERY),
        std::make_tuple(EntityKind::USER, CallbackKind::ON_USER_DISCOVERY),
        std::make_tuple(EntityKind::PROCESS, CallbackKind::ON_PROCESS_DISCOVERY),
        std::make_tuple(EntityKind::LOCATOR, CallbackKind::ON_LOCATOR_DISCOVERY)
        ));



class calling_user_listeners_tests_domain_entities
    : public ::testing::TestWithParam<std::tuple<EntityKind, CallbackKind>>
{
public:

    calling_user_listeners_tests_domain_entities()
        : entity_kind_(std::get<0>(GetParam()))
        , callback_kind_(std::get<1>(GetParam()))
    {
        // Set the profile to ignore discovery data from other processes
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_XML_profiles_file("profile.xml");
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_profiles();

        monitor_id_ = StatisticsBackend::init_monitor(0, nullptr, CallbackMask::none(), DataKindMask::none());
    }

    ~calling_user_listeners_tests_domain_entities()
    {
        StatisticsBackend::set_physical_listener(
            nullptr,
            CallbackMask::none(),
            DataKindMask::none());

        // Stop the monitor to avoid interfering on next tests
        StatisticsBackend::stop_monitor(monitor_id_);
        details::StatisticsBackendData::reset_instance();
    }

    EntityKind entity_kind_;
    CallbackKind callback_kind_;
    MockedPhysicalListener physical_listener_;
    MockedDomainListener domain_listener_;
    EntityDiscoveryArgs discovery_args_;
    EntityId monitor_id_;

    enum ListenerKind
    {
        NONE,
        PHYSICAL,
        DOMAIN
    };

    void test_entity_discovery(
            ListenerKind listener_kind,
            DiscoveryStatus discovery_status = DiscoveryStatus::DISCOVERY,
            ArgumentChecker checker = [] (
                EntityId,
                EntityId,
                const DomainListener::Status&)
    {
    })
    {
        // Set the callback of the expectations
        discovery_args_.callback_ = checker;

        switch (entity_kind_)
        {
            case EntityKind::PARTICIPANT:
            {
                if (listener_kind == PHYSICAL)
                {
                    EXPECT_CALL(physical_listener_, on_participant_discovery(monitor_id_, EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &EntityDiscoveryArgs::on_discovery));
                    EXPECT_CALL(domain_listener_, on_participant_discovery(_, _, _)).Times(0);
                }
                else if (listener_kind == DOMAIN)
                {
                    EXPECT_CALL(physical_listener_, on_participant_discovery(_, _, _)).Times(0);
                    EXPECT_CALL(domain_listener_, on_participant_discovery(monitor_id_, EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &EntityDiscoveryArgs::on_discovery));
                }
                else
                {
                    EXPECT_CALL(physical_listener_, on_participant_discovery(_, _, _)).Times(0);
                    EXPECT_CALL(domain_listener_, on_participant_discovery(_, _, _)).Times(0);
                }

                // Execution
                details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                    monitor_id_,
                    EntityId(1),
                    EntityKind::PARTICIPANT,
                    discovery_status);

                break;
            }
            case EntityKind::TOPIC:
            {
                if (listener_kind == PHYSICAL)
                {
                    EXPECT_CALL(physical_listener_, on_topic_discovery(monitor_id_, EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &EntityDiscoveryArgs::on_discovery));
                    EXPECT_CALL(domain_listener_, on_topic_discovery(_, _, _)).Times(0);
                }
                else if (listener_kind == DOMAIN)
                {
                    EXPECT_CALL(physical_listener_, on_topic_discovery(_, _, _)).Times(0);
                    EXPECT_CALL(domain_listener_, on_topic_discovery(monitor_id_, EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &EntityDiscoveryArgs::on_discovery));
                }
                else
                {
                    EXPECT_CALL(physical_listener_, on_topic_discovery(_, _, _)).Times(0);
                    EXPECT_CALL(domain_listener_, on_topic_discovery(_, _, _)).Times(0);
                }

                // Execution
                details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                    monitor_id_,
                    EntityId(1),
                    EntityKind::TOPIC,
                    discovery_status);

                break;
            }
            case EntityKind::DATAREADER:
            {
                if (listener_kind == PHYSICAL)
                {
                    EXPECT_CALL(physical_listener_, on_datareader_discovery(monitor_id_, EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &EntityDiscoveryArgs::on_discovery));
                    EXPECT_CALL(domain_listener_, on_datareader_discovery(_, _, _)).Times(0);
                }
                else if (listener_kind == DOMAIN)
                {
                    EXPECT_CALL(physical_listener_, on_datareader_discovery(_, _, _)).Times(0);
                    EXPECT_CALL(domain_listener_, on_datareader_discovery(monitor_id_, EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &EntityDiscoveryArgs::on_discovery));
                }
                else
                {
                    EXPECT_CALL(physical_listener_, on_datareader_discovery(_, _, _)).Times(0);
                    EXPECT_CALL(domain_listener_, on_datareader_discovery(_, _, _)).Times(0);
                }

                // Execution
                details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                    monitor_id_,
                    EntityId(1),
                    EntityKind::DATAREADER,
                    discovery_status);

                break;
            }
            case EntityKind::DATAWRITER:
            {
                if (listener_kind == PHYSICAL)
                {
                    EXPECT_CALL(physical_listener_, on_datawriter_discovery(monitor_id_, EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &EntityDiscoveryArgs::on_discovery));
                    EXPECT_CALL(domain_listener_, on_datawriter_discovery(_, _, _)).Times(0);
                }
                else if (listener_kind == DOMAIN)
                {
                    EXPECT_CALL(physical_listener_, on_datawriter_discovery(_, _, _)).Times(0);
                    EXPECT_CALL(domain_listener_, on_datawriter_discovery(monitor_id_, EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &EntityDiscoveryArgs::on_discovery));
                }
                else
                {
                    EXPECT_CALL(physical_listener_, on_datawriter_discovery(_, _, _)).Times(0);
                    EXPECT_CALL(domain_listener_, on_datawriter_discovery(_, _, _)).Times(0);
                }

                // Execution
                details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                    monitor_id_,
                    EntityId(1),
                    EntityKind::DATAWRITER,
                    discovery_status);

                break;
            }
            default:
            {
                FAIL() << "This fixture can be used for domain entities only";
            }
        }

        Mock::VerifyAndClearExpectations(&physical_listener_);
        Mock::VerifyAndClearExpectations(&domain_listener_);
    }

    /*
     * This method extends the tests for the testcases where there is no callback triggered
     * in the starting configuration.
     *
     * First it sets the appropriate physical listener and retests with DISCOVERY, UPDATE and UNDISCOVERY.
     * Then sets the domain listener and retests with DISCOVERY, UPDATE and UNDISCOVERY.
     */
    void extend_no_callback_tests()
    {
        // Set the physical listener and test
        CallbackMask mask = CallbackMask::none();
        mask.set(callback_kind_);
        StatisticsBackend::set_physical_listener(
            &physical_listener_,
            mask,
            DataKindMask::all());

        // Expectation: Only the physical listener is called
        test_entity_discovery(PHYSICAL, DiscoveryStatus::DISCOVERY,
                [&](
                    EntityId domain_id,
                    EntityId entity_id,
                    const DomainListener::Status& status)
                {
                    EXPECT_EQ(monitor_id_, domain_id);
                    EXPECT_EQ(1, entity_id);
                    EXPECT_EQ(2, status.total_count);
                    EXPECT_EQ(2, status.total_count_change);
                    EXPECT_EQ(1, status.current_count);
                    EXPECT_EQ(1, status.current_count_change);
                });

        // Expectation: Only the physical listener is called again
        test_entity_discovery(PHYSICAL, DiscoveryStatus::DISCOVERY,
                [&](
                    EntityId domain_id,
                    EntityId entity_id,
                    const DomainListener::Status& status)
                {
                    EXPECT_EQ(monitor_id_, domain_id);
                    EXPECT_EQ(1, entity_id);
                    EXPECT_EQ(3, status.total_count);
                    EXPECT_EQ(1, status.total_count_change);
                    EXPECT_EQ(2, status.current_count);
                    EXPECT_EQ(1, status.current_count_change);
                });

        // Expectation: The physical listener is called again with updates
        test_entity_discovery(PHYSICAL, DiscoveryStatus::UPDATE,
                [&](
                    EntityId domain_id,
                    EntityId entity_id,
                    const DomainListener::Status& status)
                {
                    EXPECT_EQ(monitor_id_, domain_id);
                    EXPECT_EQ(1, entity_id);
                    EXPECT_EQ(3, status.total_count);
                    EXPECT_EQ(0, status.total_count_change);
                    EXPECT_EQ(2, status.current_count);
                    EXPECT_EQ(0, status.current_count_change);
                });

        // Expectation: The physical listener is called again with removal
        test_entity_discovery(PHYSICAL, DiscoveryStatus::UNDISCOVERY,
                [&](
                    EntityId domain_id,
                    EntityId entity_id,
                    const DomainListener::Status& status)
                {
                    EXPECT_EQ(monitor_id_, domain_id);
                    EXPECT_EQ(1, entity_id);
                    EXPECT_EQ(3, status.total_count);
                    EXPECT_EQ(0, status.total_count_change);
                    EXPECT_EQ(1, status.current_count);
                    EXPECT_EQ(-1, status.current_count_change);
                });

        // Set the domain listener and retest
        StatisticsBackend::set_domain_listener(
            monitor_id_,
            &domain_listener_,
            mask,
            DataKindMask::all());

        // Expectation: Only the domain listener is called
        test_entity_discovery(DOMAIN, DiscoveryStatus::DISCOVERY,
                [&](
                    EntityId domain_id,
                    EntityId entity_id,
                    const DomainListener::Status& status)
                {
                    EXPECT_EQ(monitor_id_, domain_id);
                    EXPECT_EQ(1, entity_id);
                    EXPECT_EQ(4, status.total_count);
                    EXPECT_EQ(1, status.total_count_change);
                    EXPECT_EQ(2, status.current_count);
                    EXPECT_EQ(1, status.current_count_change);
                });

        // Expectation: Only the domain listener is called again
        test_entity_discovery(DOMAIN, DiscoveryStatus::DISCOVERY,
                [&](
                    EntityId domain_id,
                    EntityId entity_id,
                    const DomainListener::Status& status)
                {
                    EXPECT_EQ(monitor_id_, domain_id);
                    EXPECT_EQ(1, entity_id);
                    EXPECT_EQ(5, status.total_count);
                    EXPECT_EQ(1, status.total_count_change);
                    EXPECT_EQ(3, status.current_count);
                    EXPECT_EQ(1, status.current_count_change);
                });

        // Expectation: The domain listener is called again with updates
        test_entity_discovery(DOMAIN, DiscoveryStatus::UPDATE,
                [&](
                    EntityId domain_id,
                    EntityId entity_id,
                    const DomainListener::Status& status)
                {
                    EXPECT_EQ(monitor_id_, domain_id);
                    EXPECT_EQ(1, entity_id);
                    EXPECT_EQ(5, status.total_count);
                    EXPECT_EQ(0, status.total_count_change);
                    EXPECT_EQ(3, status.current_count);
                    EXPECT_EQ(0, status.current_count_change);
                });

        // Expectation: The domain listener is called again with removal
        test_entity_discovery(DOMAIN, DiscoveryStatus::UNDISCOVERY,
                [&](
                    EntityId domain_id,
                    EntityId entity_id,
                    const DomainListener::Status& status)
                {
                    EXPECT_EQ(monitor_id_, domain_id);
                    EXPECT_EQ(1, entity_id);
                    EXPECT_EQ(5, status.total_count);
                    EXPECT_EQ(0, status.total_count_change);
                    EXPECT_EQ(2, status.current_count);
                    EXPECT_EQ(-1, status.current_count_change);
                });
    }

};

TEST_P(calling_user_listeners_tests_domain_entities, entity_discovered)
{
    CallbackMask mask = CallbackMask::none();
    mask.set(callback_kind_);

    StatisticsBackend::set_domain_listener(
        monitor_id_,
        &domain_listener_,
        mask,
        DataKindMask::all());

    StatisticsBackend::set_physical_listener(
        &physical_listener_,
        CallbackMask::all(),
        DataKindMask::all());

    // Expectation: Only the domain listener is called
    test_entity_discovery(DOMAIN, DiscoveryStatus::DISCOVERY,
            [&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    // Expectation: The domain listener is called again
    test_entity_discovery(DOMAIN, DiscoveryStatus::DISCOVERY,
            [&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    // Expectation: The domain listener is called with updates
    test_entity_discovery(DOMAIN, DiscoveryStatus::UPDATE,
            [&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(0, status.current_count_change);
            });

    // Expectation: The user listener is called with removal
    test_entity_discovery(DOMAIN, DiscoveryStatus::UNDISCOVERY,
            [&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    // Unset the domain listener and the physical listener
    StatisticsBackend::set_domain_listener(
        monitor_id_,
        nullptr,
        mask,
        DataKindMask::all());

    StatisticsBackend::set_physical_listener(
        nullptr,
        CallbackMask::all(),
        DataKindMask::all());

    // Expectation: No listener is ever called
    test_entity_discovery(NONE, DiscoveryStatus::DISCOVERY);
    test_entity_discovery(NONE, DiscoveryStatus::UPDATE);
    test_entity_discovery(NONE, DiscoveryStatus::UNDISCOVERY);
}

TEST_P(calling_user_listeners_tests_domain_entities, entity_discovered_not_in_mask)
{
    CallbackMask mask = CallbackMask::all();
    mask ^= callback_kind_;

    StatisticsBackend::set_domain_listener(
        monitor_id_,
        &domain_listener_,
        mask,
        DataKindMask::all());

    StatisticsBackend::set_physical_listener(
        &physical_listener_,
        mask,
        DataKindMask::all());

    // Expectation: No listener is ever called
    test_entity_discovery(NONE, DiscoveryStatus::DISCOVERY);
    test_entity_discovery(NONE, DiscoveryStatus::UPDATE);
    test_entity_discovery(NONE, DiscoveryStatus::UNDISCOVERY);

    extend_no_callback_tests();
}

TEST_P(calling_user_listeners_tests_domain_entities, entity_discovered_no_listener)
{
    CallbackMask mask = CallbackMask::none();
    mask.set(callback_kind_);

    StatisticsBackend::set_domain_listener(
        monitor_id_,
        nullptr,
        mask,
        DataKindMask::all());

    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: No listener is ever called
    test_entity_discovery(NONE, DiscoveryStatus::DISCOVERY);
    test_entity_discovery(NONE, DiscoveryStatus::UPDATE);
    test_entity_discovery(NONE, DiscoveryStatus::UNDISCOVERY);

    extend_no_callback_tests();
}

TEST_P(calling_user_listeners_tests_domain_entities, entity_discovered_no_listener_not_in_mask)
{
    CallbackMask mask = CallbackMask::all();
    mask ^= callback_kind_;

    StatisticsBackend::set_domain_listener(
        monitor_id_,
        nullptr,
        mask,
        DataKindMask::all());

    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: No listener is ever called
    test_entity_discovery(NONE, DiscoveryStatus::DISCOVERY);
    test_entity_discovery(NONE, DiscoveryStatus::UPDATE);
    test_entity_discovery(NONE, DiscoveryStatus::UNDISCOVERY);

    extend_no_callback_tests();
}


GTEST_INSTANTIATE_TEST_MACRO(
    calling_user_listeners_tests_domain_entities,
    calling_user_listeners_tests_domain_entities,
    ::testing::Values(
        std::make_tuple(EntityKind::PARTICIPANT, CallbackKind::ON_PARTICIPANT_DISCOVERY),
        std::make_tuple(EntityKind::TOPIC, CallbackKind::ON_TOPIC_DISCOVERY),
        std::make_tuple(EntityKind::DATAREADER, CallbackKind::ON_DATAREADER_DISCOVERY),
        std::make_tuple(EntityKind::DATAWRITER, CallbackKind::ON_DATAWRITER_DISCOVERY)
        ));


class calling_user_listeners_tests : public ::testing::Test
{
public:

    ~calling_user_listeners_tests()
    {
        StatisticsBackend::set_physical_listener(
            nullptr,
            CallbackMask::none(),
            DataKindMask::none());

        details::StatisticsBackendData::reset_instance();
    }

};

using  calling_user_listeners_DeathTest = calling_user_listeners_tests;
TEST_F(calling_user_listeners_DeathTest, wrong_entity_kind)
{
    MockedPhysicalListener physical_listener;
    MockedDomainListener domain_listener;

    auto monitor_id = StatisticsBackend::init_monitor(0, &domain_listener, CallbackMask::all(),
                    DataKindMask::all());
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        CallbackMask::all(),
        DataKindMask::all());

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                EntityId(0),
                EntityId(1),
                EntityKind::DOMAIN,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                EntityId(0),
                EntityId(1),
                EntityKind::PARTICIPANT,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                EntityId(0),
                EntityId(1),
                EntityKind::TOPIC,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                EntityId(0),
                EntityId(1),
                EntityKind::DATAREADER,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                EntityId(0),
                EntityId(1),
                EntityKind::DATAWRITER,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                EntityId(0),
                EntityId(1),
                EntityKind::INVALID,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::HOST,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY),
            ".*");
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::HOST,
                details::StatisticsBackendData::DiscoveryStatus::UPDATE),
            ".*");
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::HOST,
                details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::USER,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY),
            ".*");
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::USER,
                details::StatisticsBackendData::DiscoveryStatus::UPDATE),
            ".*");
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::USER,
                details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::PROCESS,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY),
            ".*");
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::PROCESS,
                details::StatisticsBackendData::DiscoveryStatus::UPDATE),
            ".*");
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::PROCESS,
                details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::LOCATOR,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY),
            ".*");
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::LOCATOR,
                details::StatisticsBackendData::DiscoveryStatus::UPDATE),
            ".*");
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::LOCATOR,
                details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::INVALID,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY),
            ".*");
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::INVALID,
                details::StatisticsBackendData::DiscoveryStatus::UPDATE),
            ".*");
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::INVALID,
                details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY),
            ".*");

    // Stop the monitor to avoid interfering on next tests
    StatisticsBackend::stop_monitor(monitor_id);
}

class calling_user_listeners_tests_datas : public ::testing::TestWithParam<std::tuple<DataKind>>
{
public:

    calling_user_listeners_tests_datas()
        : data_kind_(std::get<0>(GetParam()))
    {
        // Set the profile to ignore discovery data from other processes
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_XML_profiles_file("profile.xml");
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_profiles();

        monitor_id_ = StatisticsBackend::init_monitor(0, nullptr, CallbackMask::none(), DataKindMask::none());
    }

    ~calling_user_listeners_tests_datas()
    {
        StatisticsBackend::set_physical_listener(
            nullptr,
            CallbackMask::none(),
            DataKindMask::none());

        StatisticsBackend::stop_monitor(monitor_id_);
        details::StatisticsBackendData::reset_instance();
    }

    DataKind data_kind_;
    CallbackKind callback_kind_;
    MockedPhysicalListener physical_listener_;
    MockedDomainListener domain_listener_;
    EntityId monitor_id_;

    enum ListenerKind
    {
        NONE,
        PHYSICAL,
        DOMAIN
    };

    void test_data_availability(
            ListenerKind listener_kind)
    {

        if (listener_kind == PHYSICAL)
        {
            EXPECT_CALL(physical_listener_, on_data_available(monitor_id_, EntityId(1), data_kind_)).Times(1);
            EXPECT_CALL(domain_listener_, on_data_available(_, _, _)).Times(0);

        }
        else if (listener_kind == DOMAIN)
        {
            EXPECT_CALL(physical_listener_, on_data_available(_, _, _)).Times(0);
            EXPECT_CALL(domain_listener_, on_data_available(monitor_id_, EntityId(1), data_kind_)).Times(1);
        }
        else
        {
            EXPECT_CALL(physical_listener_, on_data_available(_, _, _)).Times(0);
            EXPECT_CALL(domain_listener_, on_data_available(_, _, _)).Times(0);
        }

        // Execution
        details::StatisticsBackendData::get_instance()->on_data_available(
            monitor_id_,
            EntityId(1),
            data_kind_);
    }

    /*
     * This method extends the tests for the testcases where there is no callback triggered
     * in the starting configuration.
     *
     * First it sets the appropriate physical listener and retests.
     * Then sets the domain listener and retests.
     */
    void extend_no_callback_tests()
    {
        // Set the physical listener and test
        CallbackMask callback_mask = CallbackMask::none();
        callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
        DataKindMask data_mask = DataKindMask::none();
        data_mask.set(data_kind_);

        StatisticsBackend::set_physical_listener(
            &physical_listener_,
            callback_mask,
            data_mask);

        // Expectation: Only the physical listener is called
        test_data_availability(PHYSICAL);

        // Set the domain listener and retest
        StatisticsBackend::set_domain_listener(
            monitor_id_,
            &domain_listener_,
            callback_mask,
            data_mask);

        // Expectation: Only the domain listener is called
        test_data_availability(DOMAIN);
    }

};

TEST_P(calling_user_listeners_tests_datas, data_available)
{
    CallbackMask callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
    DataKindMask data_mask = DataKindMask::none();
    data_mask.set(data_kind_);

    StatisticsBackend::set_domain_listener(
        monitor_id_,
        &domain_listener_,
        callback_mask,
        data_mask);

    StatisticsBackend::set_physical_listener(
        &physical_listener_,
        CallbackMask::all(),
        DataKindMask::all());

    // Expectation: Only the domain listener is called
    test_data_availability(DOMAIN);

    // Expectation: The domain listener is called again
    test_data_availability(DOMAIN);
}

TEST_P(calling_user_listeners_tests_datas, data_available_callback_not_in_mask)
{
    CallbackMask callback_mask = CallbackMask::all();
    callback_mask ^= CallbackKind::ON_DATA_AVAILABLE;
    DataKindMask data_mask = DataKindMask::none();
    data_mask.set(data_kind_);

    StatisticsBackend::set_domain_listener(
        monitor_id_,
        &domain_listener_,
        callback_mask,
        data_mask);

    StatisticsBackend::set_physical_listener(
        &physical_listener_,
        callback_mask,
        data_mask);

    // Expectation: No listener is called
    test_data_availability(NONE);

    extend_no_callback_tests();
}

TEST_P(calling_user_listeners_tests_datas, data_available_data_not_in_mask)
{
    CallbackMask callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
    DataKindMask data_mask = DataKindMask::all();
    data_mask ^= data_kind_;

    StatisticsBackend::set_domain_listener(
        monitor_id_,
        &domain_listener_,
        callback_mask,
        data_mask);

    StatisticsBackend::set_physical_listener(
        &physical_listener_,
        callback_mask,
        data_mask);

    // Expectation: No listener is called
    test_data_availability(NONE);

    extend_no_callback_tests();
}

TEST_P(calling_user_listeners_tests_datas, data_available_no_listener)
{
    CallbackMask callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
    DataKindMask data_mask = DataKindMask::none();
    data_mask.set(data_kind_);

    StatisticsBackend::set_domain_listener(
        monitor_id_,
        nullptr,
        callback_mask,
        data_mask);

    StatisticsBackend::set_physical_listener(
        nullptr,
        callback_mask,
        data_mask);

    // Expectation: No listener is called
    test_data_availability(NONE);

    extend_no_callback_tests();
}

TEST_P(calling_user_listeners_tests_datas, data_available_no_listener_callback_not_in_mask)
{
    CallbackMask callback_mask = CallbackMask::all();
    callback_mask ^= CallbackKind::ON_DATA_AVAILABLE;
    DataKindMask data_mask = DataKindMask::none();
    data_mask.set(data_kind_);

    StatisticsBackend::set_domain_listener(
        monitor_id_,
        nullptr,
        callback_mask,
        data_mask);

    StatisticsBackend::set_physical_listener(
        &physical_listener_,
        callback_mask,
        data_mask);

    // Expectation: No listener is called
    test_data_availability(NONE);

    extend_no_callback_tests();
}

TEST_P(calling_user_listeners_tests_datas, data_available_no_listener_data_not_in_mask)
{
    CallbackMask callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
    DataKindMask data_mask = DataKindMask::all();
    data_mask ^= data_kind_;

    StatisticsBackend::set_domain_listener(
        monitor_id_,
        nullptr,
        callback_mask,
        data_mask);

    StatisticsBackend::set_physical_listener(
        &physical_listener_,
        callback_mask,
        data_mask);

    // Expectation: No listener is called
    test_data_availability(NONE);

    extend_no_callback_tests();
}


#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_SUITE_P(x, y, z)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_CASE_P(x, y, z)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(
    calling_user_listeners_tests_datas,
    calling_user_listeners_tests_datas,
    ::testing::Values(
        std::make_tuple(DataKind::FASTDDS_LATENCY),
        std::make_tuple(DataKind::NETWORK_LATENCY),
        std::make_tuple(DataKind::PUBLICATION_THROUGHPUT),
        std::make_tuple(DataKind::SUBSCRIPTION_THROUGHPUT),
        std::make_tuple(DataKind::RTPS_PACKETS_SENT),
        std::make_tuple(DataKind::RTPS_BYTES_SENT),
        std::make_tuple(DataKind::RTPS_PACKETS_LOST),
        std::make_tuple(DataKind::RTPS_BYTES_LOST),
        std::make_tuple(DataKind::RESENT_DATA),
        std::make_tuple(DataKind::HEARTBEAT_COUNT),
        std::make_tuple(DataKind::ACKNACK_COUNT),
        std::make_tuple(DataKind::NACKFRAG_COUNT),
        std::make_tuple(DataKind::GAP_COUNT),
        std::make_tuple(DataKind::DATA_COUNT),
        std::make_tuple(DataKind::PDP_PACKETS),
        std::make_tuple(DataKind::EDP_PACKETS),
        std::make_tuple(DataKind::DISCOVERY_TIME),
        std::make_tuple(DataKind::SAMPLE_DATAS)
        ));


int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
