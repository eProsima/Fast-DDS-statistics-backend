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
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/StatisticsBackend.hpp>
#include <fastdds_statistics_backend/types/types.hpp>
#include <fastdds_statistics_backend/types/app_names.h>
#include <database/database_queue.hpp>
#include <database/database.hpp>
#include <database/entities.hpp>
#include <subscriber/StatisticsParticipantListener.hpp>
#include <Monitor.hpp>
#include <StatisticsBackendData.hpp>

#include <iomanip>
#include <sstream>
#include <utility>

using ::testing::_;
using ::testing::Invoke;
using ::testing::AnyNumber;
using ::testing::Throw;
using ::testing::Mock;

using namespace eprosima::statistics_backend;

using DiscoveryStatus = details::StatisticsBackendData::DiscoveryStatus;
using PhysicalArgumentChecker = std::function<void (
                    EntityId,
                    const DomainListener::Status& status)>;
using DomainArgumentChecker = std::function<void (
                    EntityId,
                    EntityId,
                    const DomainListener::Status& status)>;

struct PhysicalEntityDiscoveryArgs
{
    PhysicalEntityDiscoveryArgs (
            PhysicalArgumentChecker func)
        : callback_(func)
    {
    }

    PhysicalEntityDiscoveryArgs ()
    {
    }

    void on_discovery(
            EntityId discovered_entity_id,
            const DomainListener::Status& status)
    {
        // Save the arguments for future reference
        discovered_entity_id_ = discovered_entity_id;
        status_ = status;

        // Call the callback with the checks
        callback_(discovered_entity_id, status);
    }

    PhysicalArgumentChecker callback_;
    EntityId discovered_entity_id_;
    DomainListener::Status status_;
};

struct DomainEntityDiscoveryArgs
{
    DomainEntityDiscoveryArgs (
            DomainArgumentChecker func)
        : callback_(func)
    {
    }

    DomainEntityDiscoveryArgs ()
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

    DomainArgumentChecker callback_;
    EntityId base_entity_id_;
    EntityId discovered_entity_id_;
    DomainListener::Status status_;
};


class MockedPhysicalListener : public PhysicalListener
{
public:

    MOCK_METHOD2(on_host_discovery, void(
                EntityId host_id,
                const Status& status));

    MOCK_METHOD2(on_user_discovery, void(
                EntityId user_id,
                const Status& status));

    MOCK_METHOD2(on_process_discovery, void(
                EntityId process_id,
                const Status& status));

    MOCK_METHOD2(on_locator_discovery, void(
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

    MOCK_METHOD3(on_status_reported, void(
                EntityId domain_id,
                EntityId entity_id,
                StatusKind status_kind));

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

    MOCK_METHOD3(on_status_reported, void(
                EntityId domain_id,
                EntityId entity_id,
                StatusKind status_kind));
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
    PhysicalEntityDiscoveryArgs discovery_args_;
    EntityKind entity_kind_;
    CallbackKind callback_kind_;

    enum ListenerKind
    {
        NONE,
        PHYSICAL,
    };

    void test_entity_discovery(
            ListenerKind listener_kind,
            PhysicalArgumentChecker checker = [] (
                EntityId,
                const DomainListener::Status&)
    {
    },
            details::StatisticsBackendData::DiscoveryStatus const& discovery_status
            = details::StatisticsBackendData::DISCOVERY)
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
                    EXPECT_CALL(physical_listener_, on_host_discovery(EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &PhysicalEntityDiscoveryArgs::on_discovery));
                }
                else
                {
                    EXPECT_CALL(physical_listener_, on_host_discovery(_, _)).Times(0);
                }

                // Execution
                details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                    EntityId(1),
                    EntityKind::HOST,
                    discovery_status);

                break;
            }
            case EntityKind::USER:
            {
                if (listener_kind == PHYSICAL)
                {
                    EXPECT_CALL(physical_listener_, on_user_discovery(EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &PhysicalEntityDiscoveryArgs::on_discovery));
                }
                else
                {
                    EXPECT_CALL(physical_listener_, on_user_discovery( _, _)).Times(0);
                }

                // Execution
                details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                    EntityId(1),
                    EntityKind::USER,
                    discovery_status);

                break;
            }
            case EntityKind::PROCESS:
            {
                if (listener_kind == PHYSICAL)
                {
                    EXPECT_CALL(physical_listener_, on_process_discovery(EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &PhysicalEntityDiscoveryArgs::on_discovery));
                }
                else
                {
                    EXPECT_CALL(physical_listener_, on_process_discovery(_, _)).Times(0);
                }

                // Execution
                details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                    EntityId(1),
                    EntityKind::PROCESS,
                    discovery_status);

                break;
            }
            case EntityKind::LOCATOR:
            {
                if (listener_kind == PHYSICAL)
                {
                    EXPECT_CALL(physical_listener_, on_locator_discovery(EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &PhysicalEntityDiscoveryArgs::on_discovery));
                }
                else
                {
                    EXPECT_CALL(physical_listener_, on_locator_discovery(_, _)).Times(0);
                }

                // Execution
                details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                    EntityId(1),
                    EntityKind::LOCATOR,
                    discovery_status);

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
                    EntityId entity_id,
                    const DomainListener::Status& status)
                {
                    EXPECT_EQ(1, entity_id);
                    EXPECT_EQ(2, status.total_count);
                    EXPECT_EQ(2, status.total_count_change);
                    EXPECT_EQ(2, status.current_count);
                    EXPECT_EQ(2, status.current_count_change);
                });

        // Expectation: Only the physical listener is called again
        test_entity_discovery(PHYSICAL,
                [&](
                    EntityId entity_id,
                    const DomainListener::Status& status)
                {
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
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    // Expectation: The physical listener is called again
    test_entity_discovery(PHYSICAL,
            [&](
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

#ifndef NDEBUG
    // Expectation: The user listener will fail assert with update
    ASSERT_DEATH(test_entity_discovery(PHYSICAL,
            [&](
                EntityId,
                const DomainListener::Status&)
            {
            },
            details::StatisticsBackendData::DiscoveryStatus::UPDATE), "");
#endif // ifndef NDEBUG

    // Expectation: The user listener is called with removel
    test_entity_discovery(PHYSICAL,
            [&](
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            },
            details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
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
    DomainEntityDiscoveryArgs discovery_args_;
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
            DomainArgumentChecker checker = [] (
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
                            .WillOnce(Invoke(&discovery_args_, &DomainEntityDiscoveryArgs::on_discovery));
                    EXPECT_CALL(domain_listener_, on_participant_discovery(_, _, _)).Times(0);
                }
                else if (listener_kind == DOMAIN)
                {
                    EXPECT_CALL(physical_listener_, on_participant_discovery(_, _, _)).Times(0);
                    EXPECT_CALL(domain_listener_, on_participant_discovery(monitor_id_, EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &DomainEntityDiscoveryArgs::on_discovery));
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
                            .WillOnce(Invoke(&discovery_args_, &DomainEntityDiscoveryArgs::on_discovery));
                    EXPECT_CALL(domain_listener_, on_topic_discovery(_, _, _)).Times(0);
                }
                else if (listener_kind == DOMAIN)
                {
                    EXPECT_CALL(physical_listener_, on_topic_discovery(_, _, _)).Times(0);
                    EXPECT_CALL(domain_listener_, on_topic_discovery(monitor_id_, EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &DomainEntityDiscoveryArgs::on_discovery));
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
                            .WillOnce(Invoke(&discovery_args_, &DomainEntityDiscoveryArgs::on_discovery));
                    EXPECT_CALL(domain_listener_, on_datareader_discovery(_, _, _)).Times(0);
                }
                else if (listener_kind == DOMAIN)
                {
                    EXPECT_CALL(physical_listener_, on_datareader_discovery(_, _, _)).Times(0);
                    EXPECT_CALL(domain_listener_, on_datareader_discovery(monitor_id_, EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &DomainEntityDiscoveryArgs::on_discovery));
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
                            .WillOnce(Invoke(&discovery_args_, &DomainEntityDiscoveryArgs::on_discovery));
                    EXPECT_CALL(domain_listener_, on_datawriter_discovery(_, _, _)).Times(0);
                }
                else if (listener_kind == DOMAIN)
                {
                    EXPECT_CALL(physical_listener_, on_datawriter_discovery(_, _, _)).Times(0);
                    EXPECT_CALL(domain_listener_, on_datawriter_discovery(monitor_id_, EntityId(1), _)).Times(1)
                            .WillOnce(Invoke(&discovery_args_, &DomainEntityDiscoveryArgs::on_discovery));
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
                EntityId(1),
                EntityKind::DOMAIN,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                EntityId(1),
                EntityKind::PARTICIPANT,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                EntityId(1),
                EntityKind::TOPIC,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                EntityId(1),
                EntityKind::DATAREADER,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                EntityId(1),
                EntityKind::DATAWRITER,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
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

class calling_user_listeners_tests_monitor_datas : public ::testing::TestWithParam<std::tuple<StatusKind>>
{
public:

    calling_user_listeners_tests_monitor_datas()
        : status_kind_(std::get<0>(GetParam()))
    {
        // Set the profile to ignore discovery data from other processes
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_XML_profiles_file("profile.xml");
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_profiles();

        monitor_id_ = StatisticsBackend::init_monitor(0, nullptr, CallbackMask::none(), DataKindMask::none());
    }

    ~calling_user_listeners_tests_monitor_datas()
    {
        StatisticsBackend::set_physical_listener(
            nullptr,
            CallbackMask::none(),
            DataKindMask::none());

        StatisticsBackend::stop_monitor(monitor_id_);
        details::StatisticsBackendData::reset_instance();
    }

    StatusKind status_kind_;
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
            EXPECT_CALL(physical_listener_, on_status_reported(monitor_id_, EntityId(1), status_kind_)).Times(1);
            EXPECT_CALL(domain_listener_, on_status_reported(_, _, _)).Times(0);

        }
        else if (listener_kind == DOMAIN)
        {
            EXPECT_CALL(physical_listener_, on_status_reported(_, _, _)).Times(0);
            EXPECT_CALL(domain_listener_, on_status_reported(monitor_id_, EntityId(1), status_kind_)).Times(1);
        }
        else
        {
            EXPECT_CALL(physical_listener_, on_status_reported(_, _, _)).Times(0);
            EXPECT_CALL(domain_listener_, on_status_reported(_, _, _)).Times(0);
        }

        // Execution
        details::StatisticsBackendData::get_instance()->on_status_reported(
            monitor_id_,
            EntityId(1),
            status_kind_);
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
        callback_mask.set(CallbackKind::ON_STATUS_REPORTED);
        DataKindMask data_mask = DataKindMask::all();

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

TEST_P(calling_user_listeners_tests_monitor_datas, monitor_data_available)
{
    CallbackMask callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_STATUS_REPORTED);
    DataKindMask data_mask = DataKindMask::all();

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

TEST_P(calling_user_listeners_tests_monitor_datas, monitor_data_available_callback_not_in_mask)
{
    CallbackMask callback_mask = CallbackMask::all();
    callback_mask ^= CallbackKind::ON_STATUS_REPORTED;
    DataKindMask data_mask = DataKindMask::all();

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

TEST_P(calling_user_listeners_tests_monitor_datas, monitor_data_available_no_listener)
{
    CallbackMask callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_STATUS_REPORTED);
    DataKindMask data_mask = DataKindMask::all();

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

TEST_P(calling_user_listeners_tests_monitor_datas, monitor_data_available_no_listener_callback_not_in_mask)
{
    CallbackMask callback_mask = CallbackMask::all();
    callback_mask ^= CallbackKind::ON_STATUS_REPORTED;
    DataKindMask data_mask = DataKindMask::all();

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
    calling_user_listeners_tests_monitor_datas,
    calling_user_listeners_tests_monitor_datas,
    ::testing::Values(
        std::make_tuple(StatusKind::PROXY),
        std::make_tuple(StatusKind::CONNECTION_LIST),
        std::make_tuple(StatusKind::INCOMPATIBLE_QOS),
        std::make_tuple(StatusKind::INCONSISTENT_TOPIC),
        std::make_tuple(StatusKind::LIVELINESS_LOST),
        std::make_tuple(StatusKind::LIVELINESS_CHANGED),
        std::make_tuple(StatusKind::DEADLINE_MISSED),
        std::make_tuple(StatusKind::SAMPLE_LOST),
        std::make_tuple(StatusKind::STATUSES_SIZE)
        ));

using ::testing::StrictMock;

class calling_user_listeners_tests_end_to_end : public ::testing::Test
{
public:

    calling_user_listeners_tests_end_to_end()
    {
        // Set the profile to ignore discovery data from other processes
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_XML_profiles_file("profile.xml");
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_profiles();

        monitor_id_ = StatisticsBackend::init_monitor(0, &domain_listener_, CallbackMask::all(), DataKindMask::all());
        StatisticsBackend::set_physical_listener(
            &physical_listener_,
            CallbackMask::all(),
            DataKindMask::all());

        // Get the participant listener of the created monitor
        monitor_ = details::StatisticsBackendData::get_instance()->monitors_by_entity_[monitor_id_].get();
        participant_listener_ = monitor_->participant_listener;
        reader_listener_ = monitor_->reader_listener;
        participant_ = monitor_->participant;

        // Initialize other attributes
        std::stringstream(participant_guid_str_) >> participant_guid_;
        std::stringstream(datareader_guid_str_) >> datareader_guid_;
        std::stringstream(datawriter_guid_str_) >> datawriter_guid_;

    }

    ~calling_user_listeners_tests_end_to_end()
    {
        StatisticsBackend::set_physical_listener(
            nullptr,
            CallbackMask::none(),
            DataKindMask::none());

        details::StatisticsBackendData::reset_instance();
    }

    MockedPhysicalListener physical_listener_;
    MockedDomainListener domain_listener_;
    EntityId monitor_id_;
    details::Monitor* monitor_;

    eprosima::fastdds::dds::DomainParticipantListener* participant_listener_;
    eprosima::fastdds::dds::DataReaderListener* reader_listener_;

    eprosima::fastdds::dds::DomainParticipant* participant_;
    eprosima::fastdds::rtps::GUID_t participant_guid_;
    std::string participant_guid_str_ = "01.0f.00.00.00.00.00.00.00.00.00.00|0.0.1.c1";
    std::string participant_name_ = "Participant";

    eprosima::fastdds::rtps::GUID_t datareader_guid_;
    std::string datareader_guid_str_ = "01.0f.00.00.00.00.00.00.00.00.00.00|0.0.0.1";
    eprosima::fastdds::rtps::GUID_t datawriter_guid_;
    std::string datawriter_guid_str_ = "01.0f.00.00.00.00.00.00.00.00.00.00|0.0.0.2";

    std::string topic_name_ = "Topic";
    std::string topic_type_ = "DataType";

};

// Windows dll does not export ParticipantBuiltinTopicData class members (private APIs)
#if !defined(_WIN32)
/*
 * This test is a pseudo-blackbox that checks that user listeners are called
 * when new entities are discovered and undiscovered.
 * While other tests in the 'unittest' folder rely on mocks,
 * this tests does not: Its entry point is the internal DDS discovery
 * listener, where a discovery notification is simulated, and it uses
 * a real backend and database from there on. Hence the 'pseudo-blackbox'
 *
 * This was necessary because some end user notifications have complex trigger
 * configurations, and are not easily tested with pure unit testing,
 * which leads to some cases being easily overlooked and not correctly tested.
 */
TEST_F(calling_user_listeners_tests_end_to_end, entity_discovery_end_to_end)
{
    /* PARTICIPANT */
    DomainEntityDiscoveryArgs participant_discovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_participant_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&participant_discovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    DomainEntityDiscoveryArgs metatraffic_topic_discovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_topic_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&metatraffic_topic_discovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    DomainEntityDiscoveryArgs metatraffic_endpoint_discovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_datawriter_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&metatraffic_endpoint_discovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    PhysicalEntityDiscoveryArgs participant_locator_discovery_args([&](
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener_, on_locator_discovery(_, _)).Times(1)
            .WillOnce(Invoke(&participant_locator_discovery_args, &PhysicalEntityDiscoveryArgs::on_discovery));

    // Simulate the discovery of a participant
    eprosima::fastdds::rtps::ParticipantBuiltinTopicData participant_data;
    participant_data.guid = participant_guid_;
    participant_data.participant_name = participant_name_;

    // The participant locator
    eprosima::fastdds::rtps::Locator_t participant_locator(LOCATOR_KIND_UDPv4, 2049);
    participant_locator.address[12] = 127;
    participant_locator.address[15] = 1;
    participant_data.default_locators.add_unicast_locator(participant_locator);

    // Finish building the discovered participant info
    eprosima::fastdds::rtps::ParticipantDiscoveryStatus status =
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT;

    // Execution: Call the listener
    bool should_be_ignored = false; // Set to false to avoid ignoring the participant
    participant_listener_->on_participant_discovery(participant_, status, participant_data, should_be_ignored);
    details::StatisticsBackendData::get_instance()->entity_queue_->flush();

    // Check that the participant was created OK
    const std::shared_ptr<const database::DomainParticipant> participant =
            std::dynamic_pointer_cast<const database::DomainParticipant>(
        details::StatisticsBackendData::get_instance()->database_->get_entity(participant_discovery_args.
                discovered_entity_id_));
    ASSERT_TRUE(participant);
    EXPECT_TRUE(participant->active);
    EXPECT_EQ(monitor_id_, participant->domain->id);
    EXPECT_EQ(participant_guid_str_, participant->guid);
    EXPECT_EQ(participant_name_, participant->name);
    EXPECT_EQ("Unknown", participant->process->name);
    EXPECT_EQ("Unknown", participant->process->user->name);
    EXPECT_EQ("Unknown", participant->process->user->host->name);
    EXPECT_TRUE(participant->data_readers.empty());
    ASSERT_EQ(1u, participant->data_writers.size()); // There is the metatraffic endpoint

    /* TOPIC */
    // the topic will be discovered with the datawriter
    DomainEntityDiscoveryArgs topic_discovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_topic_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&topic_discovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    /* DATAWRITER */
    DomainEntityDiscoveryArgs datawriter_discovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_datawriter_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&datawriter_discovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    PhysicalEntityDiscoveryArgs writer_locator_discovery_args([&](
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener_, on_locator_discovery(_, _)).Times(1)
            .WillOnce(Invoke(&writer_locator_discovery_args, &PhysicalEntityDiscoveryArgs::on_discovery));

    // Start building the discovered writer info
    eprosima::fastdds::rtps::PublicationBuiltinTopicData writer_info;

    // The discovered writer is in the participant
    writer_info.guid = datawriter_guid_;

    // The discovered writer is in the topic
    writer_info.topic_name = topic_name_;
    writer_info.type_name = topic_type_;

    // The discovered writer contains the locator
    eprosima::fastdds::rtps::Locator_t writer_locator(LOCATOR_KIND_UDPv4, 1024);
    writer_locator.address[12] = 127;
    writer_locator.address[15] = 1;
    writer_info.remote_locators.add_unicast_locator(writer_locator);

    // Finish building the discovered writer info
    eprosima::fastdds::rtps::WriterDiscoveryStatus writer_status =
            eprosima::fastdds::rtps::WriterDiscoveryStatus::DISCOVERED_WRITER;

    // Execution: Call the listener
    participant_listener_->on_data_writer_discovery(participant_, writer_status, writer_info, should_be_ignored);
    details::StatisticsBackendData::get_instance()->entity_queue_->flush();

    // Check that the writer was created OK
    const std::shared_ptr<const database::DataWriter> writer =
            std::dynamic_pointer_cast<const database::DataWriter>(
        details::StatisticsBackendData::get_instance()->database_->get_entity(datawriter_discovery_args.
                discovered_entity_id_));
    ASSERT_TRUE(writer);
    EXPECT_TRUE(writer->active);
    EXPECT_EQ(participant->id, writer->participant->id);

    // Check that the topic was created OK
    ASSERT_TRUE(writer->topic);
    const std::shared_ptr<const database::Topic> topic = writer->topic;
    EXPECT_TRUE(topic->active);
    EXPECT_EQ(monitor_id_, topic->domain->id);
    EXPECT_EQ(topic_name_, topic->name);
    EXPECT_EQ(topic_type_, topic->data_type);
    ASSERT_EQ(1u, topic->data_writers.size());
    EXPECT_EQ(writer.get(), topic->data_writers.find(writer->id)->second.get());
    EXPECT_TRUE(topic->data_readers.empty());

    // Check that the locator was created OK
    ASSERT_EQ(1u, writer->locators.size());
    const std::shared_ptr<const database::Locator> wlocator =
            writer->locators.begin()->second;
    EXPECT_TRUE(wlocator->active);
    std::stringstream s;
    s << writer_locator;
    EXPECT_EQ(s.str(), wlocator->name);
    ASSERT_EQ(1u, wlocator->data_writers.size());
    EXPECT_EQ(writer.get(), wlocator->data_writers.find(writer->id)->second.get());

    /* DATAREADER */
    DomainEntityDiscoveryArgs datareader_discovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_datareader_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&datareader_discovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    PhysicalEntityDiscoveryArgs reader_locator_discovery_args([&](
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(3, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(3, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener_, on_locator_discovery(_, _)).Times(1)
            .WillOnce(Invoke(&reader_locator_discovery_args, &PhysicalEntityDiscoveryArgs::on_discovery));

    // Start building the discovered reader info
    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData reader_data;

    // Set max number of unicast/multicast locators
    reader_data.remote_locators.unicast =
            eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);
    reader_data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(
        1);

    // The discovered reader is in the participant
    reader_data.guid = datareader_guid_;

    // The discovered reader is in the topic
    reader_data.topic_name = topic_name_;
    reader_data.type_name = topic_type_;

    // The discovered reader contains the locator
    eprosima::fastdds::rtps::Locator_t reader_locator(LOCATOR_KIND_UDPv4, 2048);
    reader_locator.address[12] = 127;
    reader_locator.address[15] = 1;
    reader_data.remote_locators.add_unicast_locator(reader_locator);

    // Finish building the discovered reader info
    eprosima::fastdds::rtps::ReaderDiscoveryStatus reader_status =
            eprosima::fastdds::rtps::ReaderDiscoveryStatus::DISCOVERED_READER;

    // Execution: Call the listener
    participant_listener_->on_data_reader_discovery(participant_, reader_status, reader_data, should_be_ignored);
    details::StatisticsBackendData::get_instance()->entity_queue_->flush();

    // Check that the reader was created OK
    const std::shared_ptr<const database::DataReader> reader =
            std::dynamic_pointer_cast<const database::DataReader>(
        details::StatisticsBackendData::get_instance()->database_->get_entity(datareader_discovery_args.
                discovered_entity_id_));
    ASSERT_TRUE(reader);
    EXPECT_TRUE(reader->active);
    EXPECT_EQ(participant->id, reader->participant->id);

    // Check that the topic was created OK
    ASSERT_TRUE(reader->topic);
    EXPECT_EQ(writer->topic.get(), reader->topic.get());
    EXPECT_TRUE(topic->active);
    EXPECT_EQ(monitor_id_, topic->domain->id);
    EXPECT_EQ(topic_name_, topic->name);
    EXPECT_EQ(topic_type_, topic->data_type);
    ASSERT_EQ(1u, topic->data_readers.size());
    EXPECT_EQ(reader.get(), topic->data_readers.find(reader->id)->second.get());
    ASSERT_EQ(1u, topic->data_writers.size());
    EXPECT_EQ(writer.get(), topic->data_writers.find(writer->id)->second.get());

    // Check that the locator was created OK
    ASSERT_EQ(1u, reader->locators.size());
    const std::shared_ptr<const database::Locator> rlocator =
            reader->locators.begin()->second;
    EXPECT_TRUE(rlocator->active);
    s.str(std::string());
    s << reader_locator;
    EXPECT_EQ(s.str(), rlocator->name);
    ASSERT_EQ(1u, rlocator->data_readers.size());
    EXPECT_EQ(reader.get(), rlocator->data_readers.find(reader->id)->second.get());


    /* A DATAREADER on another topic, on the writer's locator */
    DomainEntityDiscoveryArgs topic2_discovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(3, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(3, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_topic_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&topic2_discovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    DomainEntityDiscoveryArgs datareader2_discovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_datareader_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&datareader2_discovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    // Start building the discovered reader info
    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData reader2_data;

    // Set max number of unicast/multicast locators
    reader2_data.remote_locators.unicast =
            eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);
    reader2_data.remote_locators.multicast =
            eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);

    // The discovered reader is in the participant
    eprosima::fastdds::rtps::GUID_t datareader2_guid_;
    std::stringstream("01.0f.00.00.00.00.00.00.00.00.00.00|0.0.0.3") >> datareader2_guid_;
    reader2_data.guid = datareader2_guid_;

    // The discovered reader is in the topic
    std::string topic2_name = "Topic2";
    reader2_data.topic_name = topic2_name;
    reader2_data.type_name = topic_type_;

    // The discovered reader contains the locator
    reader2_data.remote_locators.add_unicast_locator(writer_locator);

    // Finish building the discovered reader info
    eprosima::fastdds::rtps::ReaderDiscoveryStatus reader2_status =
            eprosima::fastdds::rtps::ReaderDiscoveryStatus::DISCOVERED_READER;

    // Execution: Call the listener
    participant_listener_->on_data_reader_discovery(participant_, reader2_status, reader2_data, should_be_ignored);
    details::StatisticsBackendData::get_instance()->entity_queue_->flush();

    // Check that the reader was created OK
    const std::shared_ptr<const database::DataReader> reader2 =
            std::dynamic_pointer_cast<const database::DataReader>(
        details::StatisticsBackendData::get_instance()->database_->get_entity(datareader2_discovery_args.
                discovered_entity_id_));
    ASSERT_TRUE(reader2);
    EXPECT_TRUE(reader2->active);
    EXPECT_EQ(participant->id, reader2->participant->id);

    // Check that the topic was created OK
    ASSERT_TRUE(reader2->topic);
    const std::shared_ptr<const database::Topic> topic2 = reader2->topic;
    EXPECT_TRUE(topic2->active);
    EXPECT_EQ(monitor_id_, topic2->domain->id);
    EXPECT_EQ(topic2_name, topic2->name);
    EXPECT_EQ(topic_type_, topic2->data_type);
    ASSERT_EQ(1u, topic2->data_readers.size());
    EXPECT_EQ(reader2.get(), topic2->data_readers.find(reader2->id)->second.get());
    EXPECT_TRUE(topic2->data_writers.empty());

    // Check that the locator is OK
    ASSERT_EQ(1u, reader2->locators.size());
    EXPECT_EQ(wlocator.get(), reader2->locators.begin()->second.get());
    EXPECT_TRUE(wlocator->active);
    ASSERT_EQ(1u, wlocator->data_readers.size());
    EXPECT_EQ(reader2.get(), wlocator->data_readers.find(reader2->id)->second.get());
    ASSERT_EQ(1u, wlocator->data_writers.size());
    EXPECT_EQ(writer.get(), wlocator->data_writers.find(writer->id)->second.get());

    /* A DATAWRITER on the second topic, on the reader's locator */
    DomainEntityDiscoveryArgs datawriter2_discovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(3, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(3, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_datawriter_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&datawriter2_discovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    // Start building the discovered writer info
    eprosima::fastdds::rtps::PublicationBuiltinTopicData writer2_info;

    // The discovered writer is in the participant
    eprosima::fastdds::rtps::GUID_t datawriter2_guid_;
    std::stringstream("01.0f.00.00.00.00.00.00.00.00.00.00|0.0.0.4") >> datawriter2_guid_;
    writer2_info.guid = datawriter2_guid_;

    // The discovered writer contains the locator
    writer2_info.remote_locators.add_unicast_locator(reader_locator);

    // The discovered writer is in the topic
    writer2_info.topic_name = topic2_name;
    writer2_info.type_name = topic_type_;

    // Finish building the discovered writer info
    eprosima::fastdds::rtps::WriterDiscoveryStatus writer2_status =
            eprosima::fastdds::rtps::WriterDiscoveryStatus::DISCOVERED_WRITER;

    // Execution: Call the listener
    participant_listener_->on_data_writer_discovery(participant_, writer2_status, writer2_info, should_be_ignored);
    details::StatisticsBackendData::get_instance()->entity_queue_->flush();

    // Check that the writer was created OK
    const std::shared_ptr<const database::DataWriter> writer2 =
            std::dynamic_pointer_cast<const database::DataWriter>(
        details::StatisticsBackendData::get_instance()->database_->get_entity(datawriter2_discovery_args.
                discovered_entity_id_));
    ASSERT_TRUE(writer2);
    EXPECT_TRUE(writer2->active);
    EXPECT_EQ(participant->id, writer2->participant->id);

    // Check that the topic is OK
    ASSERT_TRUE(writer2->topic);
    EXPECT_EQ(topic2.get(), writer2->topic.get());
    EXPECT_TRUE(topic2->active);
    ASSERT_EQ(1u, topic2->data_readers.size());
    EXPECT_EQ(reader2.get(), topic2->data_readers.find(reader2->id)->second.get());
    ASSERT_EQ(1u, topic2->data_writers.size());
    EXPECT_EQ(writer2.get(), topic2->data_writers.find(writer2->id)->second.get());

    // Check that the locator is OK
    ASSERT_EQ(1u, writer2->locators.size());
    EXPECT_EQ(rlocator.get(), writer2->locators.begin()->second.get());
    EXPECT_TRUE(rlocator->active);
    ASSERT_EQ(1u, rlocator->data_readers.size());
    EXPECT_EQ(reader.get(), rlocator->data_readers.find(reader->id)->second.get());
    ASSERT_EQ(1u, rlocator->data_writers.size());
    EXPECT_EQ(writer2.get(), rlocator->data_writers.find(writer2->id)->second.get());

    /* Remove the DATAWRITER and DATAREADER on the first TOPIC */
    DomainEntityDiscoveryArgs datawriter_undiscovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(3, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_datawriter_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&datawriter_undiscovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    eprosima::fastdds::rtps::PublicationBuiltinTopicData writer_undiscovery_info = writer_info;
    eprosima::fastdds::rtps::WriterDiscoveryStatus writer_undiscovery_status =
            eprosima::fastdds::rtps::WriterDiscoveryStatus::REMOVED_WRITER;

    // Execution: Call the listener
    participant_listener_->on_data_writer_discovery(participant_, writer_undiscovery_status, writer_undiscovery_info,
            should_be_ignored);
    details::StatisticsBackendData::get_instance()->entity_queue_->flush();

    EXPECT_FALSE(writer->active);
    EXPECT_TRUE(reader->active);
    EXPECT_TRUE(topic->active);
    ASSERT_EQ(1u, topic->data_readers.size());
    EXPECT_EQ(reader.get(), topic->data_readers.find(reader->id)->second.get());
    ASSERT_EQ(1u, topic->data_writers.size());
    EXPECT_EQ(writer.get(), topic->data_writers.find(writer->id)->second.get());
    EXPECT_TRUE(wlocator->active);
    EXPECT_TRUE(rlocator->active);
    EXPECT_TRUE(participant->active);
    ASSERT_EQ(2u, participant->data_readers.size());
    EXPECT_EQ(reader.get(), participant->data_readers.find(reader->id)->second.get());
    ASSERT_EQ(3u, participant->data_writers.size());
    EXPECT_EQ(writer.get(), participant->data_writers.find(writer->id)->second.get());

    DomainEntityDiscoveryArgs datareader_undiscovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_datareader_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&datareader_undiscovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    DomainEntityDiscoveryArgs topic_undiscovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(3, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_topic_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&topic_undiscovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData reader_undiscovery_info = reader_data;
    eprosima::fastdds::rtps::ReaderDiscoveryStatus reader_undiscovery_status =
            eprosima::fastdds::rtps::ReaderDiscoveryStatus::REMOVED_READER;

    // Execution: Call the listener
    participant_listener_->on_data_reader_discovery(participant_, reader_undiscovery_status, reader_undiscovery_info,
            should_be_ignored);
    details::StatisticsBackendData::get_instance()->entity_queue_->flush();

    EXPECT_FALSE(writer->active);
    EXPECT_FALSE(reader->active);
    EXPECT_FALSE(topic->active);
    ASSERT_EQ(1u, topic->data_readers.size());
    EXPECT_EQ(reader.get(), topic->data_readers.find(reader->id)->second.get());
    ASSERT_EQ(1u, topic->data_writers.size());
    EXPECT_EQ(writer.get(), topic->data_writers.find(writer->id)->second.get());
    EXPECT_TRUE(wlocator->active);
    EXPECT_TRUE(rlocator->active);
    EXPECT_TRUE(participant->active);
    ASSERT_EQ(2u, participant->data_readers.size());
    EXPECT_EQ(reader.get(), participant->data_readers.find(reader->id)->second.get());
    ASSERT_EQ(3u, participant->data_writers.size());
    EXPECT_EQ(writer.get(), participant->data_writers.find(writer->id)->second.get());

    /* Remove the DATAWRITER and DATAREADER on the second TOPIC */
    DomainEntityDiscoveryArgs datawriter2_undiscovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(3, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_datawriter_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&datawriter2_undiscovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    eprosima::fastdds::rtps::PublicationBuiltinTopicData writer2_undiscovery_info = writer2_info;
    eprosima::fastdds::rtps::WriterDiscoveryStatus writer2_undiscovery_status =
            eprosima::fastdds::rtps::WriterDiscoveryStatus::REMOVED_WRITER;

    // Execution: Call the listener
    participant_listener_->on_data_writer_discovery(participant_, writer2_undiscovery_status, writer2_undiscovery_info,
            should_be_ignored);
    details::StatisticsBackendData::get_instance()->entity_queue_->flush();

    EXPECT_FALSE(writer2->active);
    EXPECT_TRUE(reader2->active);
    EXPECT_TRUE(topic2->active);
    ASSERT_EQ(1u, topic2->data_readers.size());
    EXPECT_EQ(reader2.get(), topic2->data_readers.find(reader2->id)->second.get());
    ASSERT_EQ(1u, topic2->data_writers.size());
    EXPECT_EQ(writer2.get(), topic2->data_writers.find(writer2->id)->second.get());
    EXPECT_TRUE(wlocator->active);
    EXPECT_TRUE(rlocator->active);
    EXPECT_TRUE(participant->active);
    ASSERT_EQ(2u, participant->data_readers.size());
    EXPECT_EQ(reader2.get(), participant->data_readers.find(reader2->id)->second.get());
    ASSERT_EQ(3u, participant->data_writers.size());
    EXPECT_EQ(writer2.get(), participant->data_writers.find(writer2->id)->second.get());

    DomainEntityDiscoveryArgs datareader2_undiscovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(0, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_datareader_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&datareader2_undiscovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    DomainEntityDiscoveryArgs topic2_undiscovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(3, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_topic_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&topic2_undiscovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData reader2_undiscovery_info = reader2_data;
    eprosima::fastdds::rtps::ReaderDiscoveryStatus reader2_undiscovery_info_status =
            eprosima::fastdds::rtps::ReaderDiscoveryStatus::REMOVED_READER;

    // Execution: Call the listener
    participant_listener_->on_data_reader_discovery(participant_, reader2_undiscovery_info_status,
            reader2_undiscovery_info, should_be_ignored);
    details::StatisticsBackendData::get_instance()->entity_queue_->flush();

    EXPECT_FALSE(writer2->active);
    EXPECT_FALSE(reader2->active);
    EXPECT_FALSE(topic2->active);
    ASSERT_EQ(1u, topic2->data_readers.size());
    EXPECT_EQ(reader2.get(), topic2->data_readers.find(reader2->id)->second.get());
    ASSERT_EQ(1u, topic2->data_writers.size());
    EXPECT_EQ(writer2.get(), topic2->data_writers.find(writer2->id)->second.get());
    EXPECT_TRUE(wlocator->active);
    EXPECT_TRUE(rlocator->active);
    EXPECT_TRUE(participant->active);
    ASSERT_EQ(2u, participant->data_readers.size());
    EXPECT_EQ(reader2.get(), participant->data_readers.find(reader2->id)->second.get());
    ASSERT_EQ(3u, participant->data_writers.size());
    EXPECT_EQ(writer2.get(), participant->data_writers.find(writer2->id)->second.get());

    /* Remove the PARTICIPANT */
    DomainEntityDiscoveryArgs participant_undiscovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(0, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_participant_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&participant_undiscovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    DomainEntityDiscoveryArgs metatraffic_topic_undiscovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(3, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(0, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_topic_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&metatraffic_topic_undiscovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    DomainEntityDiscoveryArgs metatraffic_endpoint_undiscovery_args([&](
                EntityId domain_id,
                EntityId /*entity_id*/,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(monitor_id_, domain_id);
                EXPECT_EQ(3, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(0, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener_, on_datawriter_discovery(monitor_id_, _, _)).Times(1)
            .WillOnce(Invoke(&metatraffic_endpoint_undiscovery_args, &DomainEntityDiscoveryArgs::on_discovery));

    status = eprosima::fastdds::rtps::ParticipantDiscoveryStatus::REMOVED_PARTICIPANT;

    // Execution: Call the listener
    participant_listener_->on_participant_discovery(participant_, status, participant_data, should_be_ignored);
    details::StatisticsBackendData::get_instance()->entity_queue_->flush();

    EXPECT_FALSE(writer2->active);
    EXPECT_FALSE(reader2->active);
    EXPECT_FALSE(topic2->active);
    ASSERT_EQ(1u, topic2->data_readers.size());
    EXPECT_EQ(reader2.get(), topic2->data_readers.find(reader2->id)->second.get());
    ASSERT_EQ(1u, topic2->data_writers.size());
    EXPECT_EQ(writer2.get(), topic2->data_writers.find(writer2->id)->second.get());
    EXPECT_TRUE(wlocator->active);
    EXPECT_TRUE(rlocator->active);
    EXPECT_FALSE(participant->active);
    ASSERT_EQ(2u, participant->data_readers.size());
    EXPECT_EQ(reader2.get(), participant->data_readers.find(reader2->id)->second.get());
    ASSERT_EQ(3u, participant->data_writers.size());
    EXPECT_EQ(writer2.get(), participant->data_writers.find(writer2->id)->second.get());
}
#endif //!defined(_WIN32)

// Windows dll does not export ParticipantProxyData class members (private APIs)
#if !defined(_WIN32)
/*
 * This test is a pseudo-blackbox that checks ParticipantProxyData is properly
 * registered.
 * While other tests in the 'unittest' folder rely on mocks,
 * this tests does not: Its entry point is the internal DDS discovery
 * listener, where a discovery notification is simulated, and it uses
 * a real backend and database from there on. Hence the 'pseudo-blackbox'
 *
 * This was necessary because some end user notifications have complex trigger
 * configurations, and are not easily tested with pure unit testing,
 * which leads to some cases being easily overlooked and not correctly tested.
 */
TEST_F(calling_user_listeners_tests_end_to_end, participant_proxy_data_end_to_end)
{
    EXPECT_CALL(physical_listener_, on_host_discovery(_, _)).Times(AnyNumber());
    EXPECT_CALL(physical_listener_, on_user_discovery(_, _)).Times(AnyNumber());
    EXPECT_CALL(physical_listener_, on_process_discovery(_, _)).Times(AnyNumber());
    EXPECT_CALL(domain_listener_, on_participant_discovery(_, _, _)).Times(AnyNumber());
    EXPECT_CALL(domain_listener_, on_topic_discovery(_, _, _)).Times(AnyNumber());
    EXPECT_CALL(physical_listener_, on_locator_discovery(_, _)).Times(AnyNumber());
    EXPECT_CALL(domain_listener_, on_datawriter_discovery(_, _, _)).Times(AnyNumber());

    std::string host_name_base = "host_name_";
    std::string user_name_base = "user_name_";
    std::string process_name_base = "process_name_";
    std::string participant_name_base = "participant_name_";

    std::string participant_guid_str_prefix_base = "01.0f.00.00.00.00.00.00.00.00.00.";
    std::string participant_guid_str_suffix = "0.0.1.c1";


    auto app_ids = {
        AppId::UNKNOWN,
        AppId::FASTDDS_MONITOR,
        AppId::DDS_ROUTER,
        AppId::SHAPES_DEMO,
        AppId::INTEGRATION_SERVICE,
        AppId::FASTDDS_VISUALIZER,
        AppId::FASTDDS_SPY,
        AppId::DDS_RECORDER,
        AppId::DDS_REPLAYER,
        AppId::AML_IP,
        AppId::SUSTAINML};

    for (auto app_id : app_ids)
    {
        // Create new entities on each iteration
        std::string host_name = host_name_base + std::to_string(int(app_id));
        std::string user_name = user_name_base + std::to_string(int(app_id));
        std::string process_name = process_name_base + std::to_string(int(app_id));
        std::string participant_name_ = participant_name_base + std::to_string(int(app_id));
        std::string metadata_ = "metadata_" + std::to_string(int(app_id));

        // Increase GUID (hexadecimal)
        std::stringstream stream;
        stream << std::hex << std::setw(2) << std::setfill('0') << int(app_id);
        std::string hexString = stream.str();
        std::string participant_guid_str_prefix = participant_guid_str_prefix_base + hexString;

        std::string participant_guid_str = participant_guid_str_prefix + "|" + participant_guid_str_suffix;

        eprosima::fastdds::rtps::GUID_t participant_guid_;
        std::stringstream(participant_guid_str) >> participant_guid_;

        // Simulate the discovery of a participant
        eprosima::fastdds::rtps::RTPSParticipantAllocationAttributes attributes;
        eprosima::fastdds::rtps::ParticipantBuiltinTopicData participant_data;
        participant_data.guid = participant_guid_;
        participant_data.participant_name = participant_name_;

        participant_data.properties.push_back("fastdds.application.id", app_id_str[int(app_id)]);
        participant_data.properties.push_back("fastdds.application.metadata", metadata_);
        participant_data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_host, host_name);
        participant_data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_user, user_name);
        participant_data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_process,
                process_name);

        // The participant locator
        eprosima::fastdds::rtps::Locator_t participant_locator(LOCATOR_KIND_UDPv4, 2049);
        participant_locator.address[12] = 127;
        participant_locator.address[15] = 1;
        participant_data.default_locators.add_unicast_locator(participant_locator);

        // Finish building the discovered participant info
        eprosima::fastdds::rtps::ParticipantDiscoveryStatus status =
                eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT;

        // Execution: Call the listener
        bool should_be_ignored = false; // Set to false to avoid ignoring the entity
        participant_listener_->on_participant_discovery(participant_, status, participant_data, should_be_ignored);
        details::StatisticsBackendData::get_instance()->entity_queue_->flush();

        // Check that the participant was created OK
        EntityId participant_id =
                details::StatisticsBackendData::get_instance()->database_->get_entity_by_guid(
            EntityKind::PARTICIPANT, participant_guid_str).second;
        const std::shared_ptr<const database::DomainParticipant> participant =
                std::dynamic_pointer_cast<const database::DomainParticipant>(
            details::StatisticsBackendData::get_instance()->database_->get_entity(participant_id));
        EXPECT_EQ(app_id, participant->app_id);
        EXPECT_EQ(metadata_, participant->app_metadata);
        EXPECT_EQ(host_name, participant->process->user->host->name);
        EXPECT_EQ(user_name, participant->process->user->name);
        EXPECT_EQ(process_name, participant->process->name);
    }
}
#endif //!defined(_WIN32)

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
