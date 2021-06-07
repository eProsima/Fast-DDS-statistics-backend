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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <exception/Exception.hpp>
#include <Monitor.hpp>
#include <StatisticsBackend.hpp>
#include <StatisticsBackendData.hpp>
#include <Monitor.hpp>
#include <database/database.hpp>
#include <database/database_queue.hpp>
#include <types/types.hpp>

using ::testing::_;
using ::testing::Invoke;
using ::testing::AnyNumber;
using ::testing::Throw;

using namespace eprosima::statistics_backend;

struct EntityDiscoveryArgs
{
    EntityDiscoveryArgs (
            std::function<void(
                EntityId base_entity_id,
                EntityId discovered_entity_id,
                const DomainListener::Status& status)> func)
        : callback_(func)
    {
    }

    void on_discovery(
            EntityId base_entity_id,
            EntityId discovered_entity_id,
            const DomainListener::Status& status)
    {
        base_entity_id_ = base_entity_id;
        discovered_entity_id_ = discovered_entity_id;
        status_ = status;
    }

    std::function<void(
                EntityId base_entity_id,
                EntityId discovered_entity_id,
                const DomainListener::Status& status)> callback_;

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

TEST(calling_user_listeners_tests, host_discovered)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_HOST_DISCOVERY);
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is called
    EntityDiscoveryArgs discovery_args([&](
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

    EXPECT_CALL(physical_listener, on_host_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::HOST);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId participant_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, participant_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_host_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::HOST);
}


TEST(calling_user_listeners_tests, host_discovered_not_in_mask)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_HOST_DISCOVERY;
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_host_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::HOST);
}

TEST(calling_user_listeners_tests, host_discovered_no_listener)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_HOST_DISCOVERY);
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_host_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::HOST);
}

TEST(calling_user_listeners_tests, host_discovered_no_listener_not_in_mask)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_HOST_DISCOVERY;
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_host_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::HOST);
}

TEST(calling_user_listeners_tests, user_discovered)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_USER_DISCOVERY);
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is called
    EntityDiscoveryArgs discovery_args([&](
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

    EXPECT_CALL(physical_listener, on_user_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::USER);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId participant_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, participant_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_user_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::USER);
}

TEST(calling_user_listeners_tests, user_discovered_not_in_mask)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_USER_DISCOVERY;
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_user_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::USER);
}

TEST(calling_user_listeners_tests, user_discovered_no_listener)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_USER_DISCOVERY);
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_user_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::USER);
}

TEST(calling_user_listeners_tests, user_discovered_no_listener_not_in_mask)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_USER_DISCOVERY;
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_user_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::USER);
}

TEST(calling_user_listeners_tests, process_discovered)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_PROCESS_DISCOVERY);
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is called
    EntityDiscoveryArgs discovery_args([&](
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

    EXPECT_CALL(physical_listener, on_process_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::PROCESS);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId participant_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, participant_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_process_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::PROCESS);
}


TEST(calling_user_listeners_tests, process_discovered_not_in_mask)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_PROCESS_DISCOVERY;
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_process_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::PROCESS);
}

TEST(calling_user_listeners_tests, process_discovered_no_listener)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_PROCESS_DISCOVERY);
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_process_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::PROCESS);
}

TEST(calling_user_listeners_tests, process_discovered_no_listener_not_in_mask)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_PROCESS_DISCOVERY;
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_process_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::PROCESS);
}

TEST(calling_user_listeners_tests, locator_discovered)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_LOCATOR_DISCOVERY);
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is called
    EntityDiscoveryArgs discovery_args([&](
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

    EXPECT_CALL(physical_listener, on_locator_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::LOCATOR);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId participant_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, participant_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_locator_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::LOCATOR);
}


TEST(calling_user_listeners_tests, locator_discovered_not_in_mask)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_LOCATOR_DISCOVERY;
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_locator_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::LOCATOR);
}

TEST(calling_user_listeners_tests, locator_discovered_no_listener)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_LOCATOR_DISCOVERY);
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_locator_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::LOCATOR);
}

TEST(calling_user_listeners_tests, locator_discovered_no_listener_not_in_mask)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_LOCATOR_DISCOVERY;
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_locator_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        EntityId(0),
        EntityId(1),
        EntityKind::LOCATOR);
}

TEST(calling_user_listeners_tests, participant_discovered)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_PARTICIPANT_DISCOVERY);
    auto monitor_id = StatisticsBackend::init_monitor(0, &domain_listener, mask, DataKindMask::all());

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        nullptr,
        CallbackMask::all(),
        DataKindMask::all());

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_participant_discovery(_, _, _)).Times(0);

    // Expectation: The user domain listener is called
    EntityDiscoveryArgs discovery_args([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener, on_participant_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener, on_participant_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called with updates
    EntityDiscoveryArgs discovery_args_3([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(0, status.current_count_change);
            });

    EXPECT_CALL(domain_listener, on_participant_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_3, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    // Expectation: The user listener is called with removel
    EntityDiscoveryArgs discovery_args_4([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener, on_participant_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_4, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, participant_discovered_not_in_mask)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_PARTICIPANT_DISCOVERY;
    auto monitor_id = StatisticsBackend::init_monitor(0, &domain_listener, mask, DataKindMask::all());

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_participant_discovery(_, _, _)).Times(0);

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_participant_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);

    // Set the physical listener and retest
    mask = CallbackMask::none();
    mask.set(CallbackKind::ON_PARTICIPANT_DISCOVERY);

    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is called
    EntityDiscoveryArgs discovery_args([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_participant_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_participant_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called with updates
    EntityDiscoveryArgs discovery_args_3([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(0, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_participant_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_3, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    // Expectation: The user listener is called with removel
    EntityDiscoveryArgs discovery_args_4([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_participant_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_4, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, participant_discovered_no_listener)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_PARTICIPANT_DISCOVERY);
    auto monitor_id = StatisticsBackend::init_monitor(0, nullptr, mask, DataKindMask::all());

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_participant_discovery(_, _, _)).Times(0);

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_participant_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);

    // Set the physical listener and retest
    mask = CallbackMask::none();
    mask.set(CallbackKind::ON_PARTICIPANT_DISCOVERY);

    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is called
    EntityDiscoveryArgs discovery_args([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_participant_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_participant_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called with updates
    EntityDiscoveryArgs discovery_args_3([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(0, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_participant_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_3, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    // Expectation: The user listener is called with removel
    EntityDiscoveryArgs discovery_args_4([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_participant_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_4, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, participant_discovered_no_listener_not_in_mask)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_PARTICIPANT_DISCOVERY;
    auto monitor_id = StatisticsBackend::init_monitor(0, nullptr, mask, DataKindMask::all());

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_participant_discovery(_, _, _)).Times(0);

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_participant_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);

    // Set the physical listener and retest
    mask = CallbackMask::none();
    mask.set(CallbackKind::ON_PARTICIPANT_DISCOVERY);

    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is called
    EntityDiscoveryArgs discovery_args([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_participant_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_participant_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called with updates
    EntityDiscoveryArgs discovery_args_3([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(0, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_participant_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_3, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    // Expectation: The user listener is called with removel
    EntityDiscoveryArgs discovery_args_4([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_participant_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_4, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::PARTICIPANT,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, topic_discovered)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_TOPIC_DISCOVERY);
    auto monitor_id = StatisticsBackend::init_monitor(0, &domain_listener, mask, DataKindMask::all());

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        CallbackMask::all(),
        DataKindMask::all());

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_topic_discovery(_, _, _)).Times(0);

    // Expectation: The user domain listener is called
    EntityDiscoveryArgs discovery_args([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener, on_topic_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener, on_topic_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called with updates
    EntityDiscoveryArgs discovery_args_3([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(0, status.current_count_change);
            });

    EXPECT_CALL(domain_listener, on_topic_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_3, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    // Expectation: The user listener is called with removel
    EntityDiscoveryArgs discovery_args_4([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener, on_topic_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_4, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, topic_discovered_not_in_mask)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_TOPIC_DISCOVERY;
    auto monitor_id = StatisticsBackend::init_monitor(0, &domain_listener, mask, DataKindMask::all());

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_topic_discovery(_, _, _)).Times(0);

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_topic_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);

    // Set the physical listener and retest
    mask = CallbackMask::none();
    mask.set(CallbackKind::ON_TOPIC_DISCOVERY);

    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is called
    EntityDiscoveryArgs discovery_args([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_topic_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_topic_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called with updates
    EntityDiscoveryArgs discovery_args_3([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(0, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_topic_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_3, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    // Expectation: The user listener is called with removel
    EntityDiscoveryArgs discovery_args_4([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_topic_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_4, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, topic_discovered_no_listener)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_TOPIC_DISCOVERY);
    auto monitor_id = StatisticsBackend::init_monitor(0, nullptr, mask, DataKindMask::all());

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_topic_discovery(_, _, _)).Times(0);

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_topic_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);

    // Set the physical listener and retest
    mask = CallbackMask::none();
    mask.set(CallbackKind::ON_TOPIC_DISCOVERY);

    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is called
    EntityDiscoveryArgs discovery_args([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_topic_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_topic_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called with updates
    EntityDiscoveryArgs discovery_args_3([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(0, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_topic_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_3, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    // Expectation: The user listener is called with removel
    EntityDiscoveryArgs discovery_args_4([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_topic_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_4, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, topic_discovered_no_listener_not_in_mask)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_TOPIC_DISCOVERY;
    auto monitor_id = StatisticsBackend::init_monitor(0, nullptr, mask, DataKindMask::all());

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_topic_discovery(_, _, _)).Times(0);

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_topic_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);

    // Set the physical listener and retest
    mask = CallbackMask::none();
    mask.set(CallbackKind::ON_TOPIC_DISCOVERY);

    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is called
    EntityDiscoveryArgs discovery_args([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_topic_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_topic_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called with updates
    EntityDiscoveryArgs discovery_args_3([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(0, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_topic_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_3, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    // Expectation: The user listener is called with removel
    EntityDiscoveryArgs discovery_args_4([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_topic_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_4, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::TOPIC,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, datareader_discovered)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_DATAREADER_DISCOVERY);
    auto monitor_id = StatisticsBackend::init_monitor(0, &domain_listener, mask, DataKindMask::all());

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        CallbackMask::all(),
        DataKindMask::all());

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_datareader_discovery(_, _, _)).Times(0);

    // Expectation: The user domain listener is called
    EntityDiscoveryArgs discovery_args([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener, on_datareader_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener, on_datareader_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called with updates
    EntityDiscoveryArgs discovery_args_3([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(0, status.current_count_change);
            });

    EXPECT_CALL(domain_listener, on_datareader_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_3, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    // Expectation: The user listener is called with removel
    EntityDiscoveryArgs discovery_args_4([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener, on_datareader_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_4, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, datareader_discovered_not_in_mask)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_DATAREADER_DISCOVERY;
    auto monitor_id = StatisticsBackend::init_monitor(0, &domain_listener, mask, DataKindMask::all());

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_datareader_discovery(_, _, _)).Times(0);

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_datareader_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);

    // Set the physical listener and retest
    mask = CallbackMask::none();
    mask.set(CallbackKind::ON_DATAREADER_DISCOVERY);

    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is called
    EntityDiscoveryArgs discovery_args([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datareader_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datareader_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called with updates
    EntityDiscoveryArgs discovery_args_3([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(0, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datareader_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_3, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    // Expectation: The user listener is called with removel
    EntityDiscoveryArgs discovery_args_4([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datareader_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_4, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, datareader_discovered_no_listener)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_DATAREADER_DISCOVERY);
    auto monitor_id = StatisticsBackend::init_monitor(0, nullptr, mask, DataKindMask::all());

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_datareader_discovery(_, _, _)).Times(0);

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_datareader_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);

    // Set the physical listener and retest
    mask = CallbackMask::none();
    mask.set(CallbackKind::ON_DATAREADER_DISCOVERY);

    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is called
    EntityDiscoveryArgs discovery_args([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datareader_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datareader_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called with updates
    EntityDiscoveryArgs discovery_args_3([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(0, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datareader_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_3, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    // Expectation: The user listener is called with removel
    EntityDiscoveryArgs discovery_args_4([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datareader_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_4, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, datareader_discovered_no_listener_not_in_mask)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_DATAREADER_DISCOVERY;
    auto monitor_id = StatisticsBackend::init_monitor(0, nullptr, mask, DataKindMask::all());

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_datareader_discovery(_, _, _)).Times(0);

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_datareader_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);

    // Set the physical listener and retest
    mask = CallbackMask::none();
    mask.set(CallbackKind::ON_DATAREADER_DISCOVERY);

    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is called
    EntityDiscoveryArgs discovery_args([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datareader_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datareader_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called with updates
    EntityDiscoveryArgs discovery_args_3([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(0, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datareader_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_3, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    // Expectation: The user listener is called with removel
    EntityDiscoveryArgs discovery_args_4([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datareader_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_4, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAREADER,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, datawriter_discovered)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_DATAWRITER_DISCOVERY);
    auto monitor_id = StatisticsBackend::init_monitor(0, &domain_listener, mask, DataKindMask::all());

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        CallbackMask::all(),
        DataKindMask::all());

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_datawriter_discovery(_, _, _)).Times(0);

    // Expectation: The user domain listener is called
    EntityDiscoveryArgs discovery_args([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener, on_datawriter_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener, on_datawriter_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called with updates
    EntityDiscoveryArgs discovery_args_3([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(0, status.current_count_change);
            });

    EXPECT_CALL(domain_listener, on_datawriter_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_3, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    // Expectation: The user listener is called with removel
    EntityDiscoveryArgs discovery_args_4([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(domain_listener, on_datawriter_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_4, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, datawriter_discovered_not_in_mask)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_DATAWRITER_DISCOVERY;
    auto monitor_id = StatisticsBackend::init_monitor(0, &domain_listener, mask, DataKindMask::all());

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_datawriter_discovery(_, _, _)).Times(0);

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_datawriter_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);

    // Set the physical listener and retest
    mask = CallbackMask::none();
    mask.set(CallbackKind::ON_DATAWRITER_DISCOVERY);

    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is called
    EntityDiscoveryArgs discovery_args([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datawriter_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datawriter_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called with updates
    EntityDiscoveryArgs discovery_args_3([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(0, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datawriter_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_3, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    // Expectation: The user listener is called with removel
    EntityDiscoveryArgs discovery_args_4([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datawriter_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_4, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, datawriter_discovered_no_listener)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_DATAWRITER_DISCOVERY);
    auto monitor_id = StatisticsBackend::init_monitor(0, nullptr, mask, DataKindMask::all());

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_datawriter_discovery(_, _, _)).Times(0);

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_datawriter_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);

    // Set the physical listener and retest
    mask = CallbackMask::none();
    mask.set(CallbackKind::ON_DATAWRITER_DISCOVERY);

    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is called
    EntityDiscoveryArgs discovery_args([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datawriter_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datawriter_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called with updates
    EntityDiscoveryArgs discovery_args_3([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(0, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datawriter_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_3, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    // Expectation: The user listener is called with removel
    EntityDiscoveryArgs discovery_args_4([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datawriter_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_4, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, datawriter_discovered_no_listener_not_in_mask)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_DATAWRITER_DISCOVERY;
    auto monitor_id = StatisticsBackend::init_monitor(0, nullptr, mask, DataKindMask::all());

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_datawriter_discovery(_, _, _)).Times(0);

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_datawriter_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);

    // Set the physical listener and retest
    mask = CallbackMask::none();
    mask.set(CallbackKind::ON_DATAWRITER_DISCOVERY);

    StatisticsBackend::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user physical listener is called
    EntityDiscoveryArgs discovery_args([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(0, domain_id);
                EXPECT_EQ(1, entity_id);
                EXPECT_EQ(1, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datawriter_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called again
    EntityDiscoveryArgs discovery_args_2([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(1, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datawriter_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_2, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);

    // Expectation: The user listener is called with updates
    EntityDiscoveryArgs discovery_args_3([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(2, status.current_count);
                EXPECT_EQ(0, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datawriter_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_3, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::UPDATE);

    // Expectation: The user listener is called with removel
    EntityDiscoveryArgs discovery_args_4([&](
                EntityId domain_id,
                EntityId entity_id,
                const DomainListener::Status& status)
            {
                EXPECT_EQ(2, domain_id);
                EXPECT_EQ(3, entity_id);
                EXPECT_EQ(2, status.total_count);
                EXPECT_EQ(0, status.total_count_change);
                EXPECT_EQ(1, status.current_count);
                EXPECT_EQ(-1, status.current_count_change);
            });

    EXPECT_CALL(physical_listener, on_datawriter_discovery(_, _, _)).Times(1)
            .WillOnce(Invoke(&discovery_args_4, &EntityDiscoveryArgs::on_discovery));

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
        monitor_id,
        EntityId(1),
        EntityKind::DATAWRITER,
        details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, wrong_entity_kind)
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
                EntityKind::DOMAIN),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                EntityId(0),
                EntityId(1),
                EntityKind::PARTICIPANT),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                EntityId(0),
                EntityId(1),
                EntityKind::TOPIC),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                EntityId(0),
                EntityId(1),
                EntityKind::DATAREADER),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                EntityId(0),
                EntityId(1),
                EntityKind::DATAWRITER),
            ".*");

    // Expectation: The call asserts
    ASSERT_DEATH(details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                EntityId(0),
                EntityId(1),
                EntityKind::INVALID),
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
}

class calling_user_data_listeners_tests : public ::testing::TestWithParam<std::tuple<DomainId, DataKind>>
{
    // Each of these tests initialize a new Monitor with appropriate params
    // However, the Backend is not reset from one parameterized run to the next
    // so initializing another monitor with the same DomainID results in an error.
    // Therefore, each parameterized run is created with its own DomainId, set in the tuple.
};

TEST_P(calling_user_data_listeners_tests, data_available)
{
    DataKind data_kind = std::get<1>(GetParam());

    MockedDomainListener domain_listener;
    CallbackMask callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
    DataKindMask data_mask = DataKindMask::none();
    data_mask.set(data_kind);
    auto monitor_id = StatisticsBackend::init_monitor(std::get<0>(GetParam()), &domain_listener, callback_mask, data_mask);

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        CallbackMask::all(),
        DataKindMask::all());

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_data_available(_, _, _)).Times(0);

    // Expectation: The user listener is called
    EXPECT_CALL(domain_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(1);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_data_available(
        monitor_id,
        EntityId(1),
        data_kind);

    // Expectation: The user listener is called again
    EXPECT_CALL(domain_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(1);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_data_available(
        monitor_id,
        EntityId(1),
        data_kind);
}

TEST_P(calling_user_data_listeners_tests, data_available_callback_not_in_mask)
{
    DataKind data_kind = std::get<1>(GetParam());

    MockedDomainListener domain_listener;
    CallbackMask callback_mask = CallbackMask::all();
    callback_mask ^= CallbackKind::ON_DATA_AVAILABLE;
    DataKindMask data_mask = DataKindMask::none();
    data_mask.set(data_kind);
    auto monitor_id = StatisticsBackend::init_monitor(std::get<0>(GetParam()), &domain_listener, callback_mask, data_mask);

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        callback_mask,
        data_mask);

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_data_available(_, _, _)).Times(0);

    // Expectation: The user listener is not called
    EXPECT_CALL(domain_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_data_available(
        monitor_id,
        EntityId(1),
        data_kind);

    // Set the physical listener and retest
    callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
    data_mask = DataKindMask::none();
    data_mask.set(data_kind);

    StatisticsBackend::set_physical_listener(
        &physical_listener,
        callback_mask,
        data_mask);

    // Expectation: The user listener is called
    EXPECT_CALL(physical_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(1);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_data_available(
        monitor_id,
        EntityId(1),
        data_kind);
}

TEST_P(calling_user_data_listeners_tests, data_available_data_not_in_mask)
{
    DataKind data_kind = std::get<1>(GetParam());

    MockedDomainListener domain_listener;
    CallbackMask callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
    DataKindMask data_mask = DataKindMask::all();
    data_mask ^= data_kind;
    auto monitor_id = StatisticsBackend::init_monitor(std::get<0>(GetParam()), &domain_listener, callback_mask, data_mask);

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        callback_mask,
        data_mask);

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_data_available(_, _, _)).Times(0);

    // Expectation: The user listener is not called
    EXPECT_CALL(domain_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_data_available(
        monitor_id,
        EntityId(1),
        data_kind);

    // Set the physical listener and retest
    callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
    data_mask = DataKindMask::none();
    data_mask.set(data_kind);

    StatisticsBackend::set_physical_listener(
        &physical_listener,
        callback_mask,
        data_mask);

    // Expectation: The user listener is called
    EXPECT_CALL(physical_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(1);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_data_available(
        monitor_id,
        EntityId(1),
        data_kind);
}

TEST_P(calling_user_data_listeners_tests, data_available_no_listener)
{
    DataKind data_kind = std::get<1>(GetParam());

    MockedDomainListener domain_listener;
    CallbackMask callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
    DataKindMask data_mask = DataKindMask::none();
    data_mask.set(data_kind);
    auto monitor_id = StatisticsBackend::init_monitor(std::get<0>(GetParam()), nullptr, callback_mask, data_mask);

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        nullptr,
        callback_mask,
        data_mask);

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_data_available(_, _, _)).Times(0);

    // Expectation: The user listener is not called
    EXPECT_CALL(domain_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_data_available(
        monitor_id,
        EntityId(1),
        data_kind);

    // Set the physical listener and retest
    callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
    data_mask = DataKindMask::none();
    data_mask.set(data_kind);

    StatisticsBackend::set_physical_listener(
        &physical_listener,
        callback_mask,
        data_mask);

    // Expectation: The user listener is called
    EXPECT_CALL(physical_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(1);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_data_available(
        monitor_id,
        EntityId(1),
        data_kind);
}

TEST_P(calling_user_data_listeners_tests, data_available_no_listener_callback_not_in_mask)
{
    DataKind data_kind = std::get<1>(GetParam());

    MockedDomainListener domain_listener;
    CallbackMask callback_mask = CallbackMask::all();
    callback_mask ^= CallbackKind::ON_DATA_AVAILABLE;
    DataKindMask data_mask = DataKindMask::none();
    data_mask.set(data_kind);
    auto monitor_id = StatisticsBackend::init_monitor(std::get<0>(GetParam()), nullptr, callback_mask, data_mask);

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        callback_mask,
        data_mask);

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_data_available(_, _, _)).Times(0);

    // Expectation: The user listener is not called
    EXPECT_CALL(domain_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_data_available(
        monitor_id,
        EntityId(1),
        data_kind);

    // Set the physical listener and retest
    callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
    data_mask = DataKindMask::none();
    data_mask.set(data_kind);

    StatisticsBackend::set_physical_listener(
        &physical_listener,
        callback_mask,
        data_mask);

    // Expectation: The user listener is called
    EXPECT_CALL(physical_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(1);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_data_available(
        monitor_id,
        EntityId(1),
        data_kind);
}

TEST_P(calling_user_data_listeners_tests, data_available_no_listener_data_not_in_mask)
{
    DataKind data_kind = std::get<1>(GetParam());

    MockedDomainListener domain_listener;
    CallbackMask callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
    DataKindMask data_mask = DataKindMask::all();
    data_mask ^= data_kind;
    auto monitor_id = StatisticsBackend::init_monitor(std::get<0>(GetParam()), nullptr, callback_mask, data_mask);

    MockedPhysicalListener physical_listener;
    StatisticsBackend::set_physical_listener(
        &physical_listener,
        callback_mask,
        data_mask);

    // Expectation: The user physical listener is never called
    EXPECT_CALL(physical_listener, on_data_available(_, _, _)).Times(0);

    // Expectation: The user listener is not called
    EXPECT_CALL(domain_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(0);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_data_available(
        monitor_id,
        EntityId(1),
        data_kind);

    // Set the physical listener and retest
    callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
    data_mask = DataKindMask::none();
    data_mask.set(data_kind);

    StatisticsBackend::set_physical_listener(
        &physical_listener,
        callback_mask,
        data_mask);

    // Expectation: The user listener is called
    EXPECT_CALL(physical_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(1);

    // Execution: Call the listener
    details::StatisticsBackendData::get_instance()->on_data_available(
        monitor_id,
        EntityId(1),
        data_kind);
}


#ifdef INSTANTIATE_TEST_SUITE_P
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_SUITE_P(x, y, z)
#else
#define GTEST_INSTANTIATE_TEST_MACRO(x, y, z) INSTANTIATE_TEST_CASE_P(x, y, z)
#endif // ifdef INSTANTIATE_TEST_SUITE_P

GTEST_INSTANTIATE_TEST_MACRO(
    calling_user_data_listeners_tests,
    calling_user_data_listeners_tests,
    ::testing::Values(
        std::make_tuple(DomainId(1), DataKind::FASTDDS_LATENCY),
        std::make_tuple(DomainId(2), DataKind::NETWORK_LATENCY),
        std::make_tuple(DomainId(3), DataKind::PUBLICATION_THROUGHPUT),
        std::make_tuple(DomainId(4), DataKind::SUBSCRIPTION_THROUGHPUT),
        std::make_tuple(DomainId(5), DataKind::RTPS_PACKETS_SENT),
        std::make_tuple(DomainId(6), DataKind::RTPS_BYTES_SENT),
        std::make_tuple(DomainId(7), DataKind::RTPS_PACKETS_LOST),
        std::make_tuple(DomainId(8), DataKind::RTPS_BYTES_LOST),
        std::make_tuple(DomainId(9), DataKind::RESENT_DATA),
        std::make_tuple(DomainId(10), DataKind::HEARTBEAT_COUNT),
        std::make_tuple(DomainId(11), DataKind::ACKNACK_COUNT),
        std::make_tuple(DomainId(12), DataKind::NACKFRAG_COUNT),
        std::make_tuple(DomainId(13), DataKind::GAP_COUNT),
        std::make_tuple(DomainId(14), DataKind::DATA_COUNT),
        std::make_tuple(DomainId(15), DataKind::PDP_PACKETS),
        std::make_tuple(DomainId(16), DataKind::EDP_PACKETS),
        std::make_tuple(DomainId(17), DataKind::DISCOVERY_TIME),
        std::make_tuple(DomainId(18), DataKind::SAMPLE_DATAS)
        ));


int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
