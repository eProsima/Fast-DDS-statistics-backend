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


// A wrapper around StatisticsBackend to access the listeners to test
class StatisticsBackendWrapper : public StatisticsBackend
{
public:

    using StatisticsBackend::DiscoveryStatus;

    static void on_domain_entity_discovery(
            EntityId domain_id,
            EntityId entity_id,
            EntityKind entity_kind,
            DiscoveryStatus discovery_status)
    {
        StatisticsBackend::on_domain_entity_discovery(
                domain_id, entity_id, entity_kind, discovery_status);
    }

    static void on_physical_entity_discovery(
            EntityId participant_id,
            EntityId entity_id,
            EntityKind entity_kind)
    {
        StatisticsBackend::on_physical_entity_discovery(
                participant_id, entity_id, entity_kind);
    }

    static void on_data_available(
            EntityId domain_id,
            EntityId entity_id,
            DataKind data_kind)
    {
        StatisticsBackend::on_data_available(
                domain_id, entity_id, data_kind);
    }

    // TODO: Remove this method once the init_monitor of StatisticsBackend is merged
    static EntityId init_monitor(
        DomainId /*domain*/,
        DomainListener* domain_listener = nullptr,
        CallbackMask callback_mask = CallbackMask::all(),
        DataKindMask data_mask = DataKindMask::none())
    {
        monitors_[0] = new Monitor;
        monitors_[0]->domain_listener = domain_listener;
        monitors_[0]->domain_callback_mask = callback_mask;
        monitors_[0]->data_mask = data_mask;
        return EntityId(0);
    }

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
    StatisticsBackendWrapper::set_physical_listener(
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
    StatisticsBackendWrapper::on_physical_entity_discovery(
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
    StatisticsBackendWrapper::on_physical_entity_discovery(
            EntityId(0),
            EntityId(1),
            EntityKind::HOST);
}


TEST(calling_user_listeners_tests, host_discovered_not_in_mask)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_HOST_DISCOVERY;
    StatisticsBackendWrapper::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_host_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_physical_entity_discovery(
            EntityId(0),
            EntityId(1),
            EntityKind::HOST);
}

TEST(calling_user_listeners_tests, host_discovered_no_listener)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_HOST_DISCOVERY);
    StatisticsBackendWrapper::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_host_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_physical_entity_discovery(
            EntityId(0),
            EntityId(1),
            EntityKind::HOST);
}

TEST(calling_user_listeners_tests, host_discovered_no_listener_not_in_mask)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_HOST_DISCOVERY;
    StatisticsBackendWrapper::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_host_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_physical_entity_discovery(
            EntityId(0),
            EntityId(1),
            EntityKind::HOST);
}

TEST(calling_user_listeners_tests, user_discovered)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_USER_DISCOVERY);
    StatisticsBackendWrapper::set_physical_listener(
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
    StatisticsBackendWrapper::on_physical_entity_discovery(
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
    StatisticsBackendWrapper::on_physical_entity_discovery(
            EntityId(0),
            EntityId(1),
            EntityKind::USER);
}

TEST(calling_user_listeners_tests, user_discovered_not_in_mask)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_USER_DISCOVERY;
    StatisticsBackendWrapper::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_user_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_physical_entity_discovery(
            EntityId(0),
            EntityId(1),
            EntityKind::USER);
}

TEST(calling_user_listeners_tests, user_discovered_no_listener)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_USER_DISCOVERY);
    StatisticsBackendWrapper::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_user_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_physical_entity_discovery(
            EntityId(0),
            EntityId(1),
            EntityKind::USER);
}

TEST(calling_user_listeners_tests, user_discovered_no_listener_not_in_mask)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_USER_DISCOVERY;
    StatisticsBackendWrapper::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_user_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_physical_entity_discovery(
            EntityId(0),
            EntityId(1),
            EntityKind::USER);
}

TEST(calling_user_listeners_tests, process_discovered)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_PROCESS_DISCOVERY);
    StatisticsBackendWrapper::set_physical_listener(
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
    StatisticsBackendWrapper::on_physical_entity_discovery(
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
    StatisticsBackendWrapper::on_physical_entity_discovery(
            EntityId(0),
            EntityId(1),
            EntityKind::PROCESS);
}


TEST(calling_user_listeners_tests, process_discovered_not_in_mask)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_PROCESS_DISCOVERY;
    StatisticsBackendWrapper::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_process_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_physical_entity_discovery(
            EntityId(0),
            EntityId(1),
            EntityKind::PROCESS);
}

TEST(calling_user_listeners_tests, process_discovered_no_listener)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_PROCESS_DISCOVERY);
    StatisticsBackendWrapper::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_process_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_physical_entity_discovery(
            EntityId(0),
            EntityId(1),
            EntityKind::PROCESS);
}

TEST(calling_user_listeners_tests, process_discovered_no_listener_not_in_mask)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_PROCESS_DISCOVERY;
    StatisticsBackendWrapper::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_process_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_physical_entity_discovery(
            EntityId(0),
            EntityId(1),
            EntityKind::PROCESS);
}

TEST(calling_user_listeners_tests, locator_discovered)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_LOCATOR_DISCOVERY);
    StatisticsBackendWrapper::set_physical_listener(
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
    StatisticsBackendWrapper::on_physical_entity_discovery(
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
    StatisticsBackendWrapper::on_physical_entity_discovery(
            EntityId(0),
            EntityId(1),
            EntityKind::LOCATOR);
}


TEST(calling_user_listeners_tests, locator_discovered_not_in_mask)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_LOCATOR_DISCOVERY;
    StatisticsBackendWrapper::set_physical_listener(
        &physical_listener,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_locator_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_physical_entity_discovery(
            EntityId(0),
            EntityId(1),
            EntityKind::LOCATOR);
}

TEST(calling_user_listeners_tests, locator_discovered_no_listener)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_LOCATOR_DISCOVERY);
    StatisticsBackendWrapper::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_locator_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_physical_entity_discovery(
            EntityId(0),
            EntityId(1),
            EntityKind::LOCATOR);
}

TEST(calling_user_listeners_tests, locator_discovered_no_listener_not_in_mask)
{
    MockedPhysicalListener physical_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_LOCATOR_DISCOVERY;
    StatisticsBackendWrapper::set_physical_listener(
        nullptr,
        mask,
        DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(physical_listener, on_locator_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_physical_entity_discovery(
            EntityId(0),
            EntityId(1),
            EntityKind::LOCATOR);
}

TEST(calling_user_listeners_tests, participant_discovered)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_PARTICIPANT_DISCOVERY);
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, &domain_listener, mask, DataKindMask::all());

    // Expectation: The user listener is called
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
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::PARTICIPANT,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

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
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::PARTICIPANT,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

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
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::PARTICIPANT,
            StatisticsBackendWrapper::DiscoveryStatus::UPDATE);

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
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::PARTICIPANT,
            StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, participant_discovered_not_in_mask)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_PARTICIPANT_DISCOVERY;
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, &domain_listener, mask, DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_participant_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::PARTICIPANT,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::PARTICIPANT,
            StatisticsBackendWrapper::DiscoveryStatus::UPDATE);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::PARTICIPANT,
            StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, participant_discovered_no_listener)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_PARTICIPANT_DISCOVERY);
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, nullptr, mask, DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_participant_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::PARTICIPANT,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::PARTICIPANT,
            StatisticsBackendWrapper::DiscoveryStatus::UPDATE);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::PARTICIPANT,
            StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, participant_discovered_no_listener_not_in_mask)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_PARTICIPANT_DISCOVERY;
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, nullptr, mask, DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_participant_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::PARTICIPANT,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::PARTICIPANT,
            StatisticsBackendWrapper::DiscoveryStatus::UPDATE);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::PARTICIPANT,
            StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, topic_discovered)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_TOPIC_DISCOVERY);
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, &domain_listener, mask, DataKindMask::all());

    // Expectation: The user listener is called
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
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::TOPIC,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

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
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::TOPIC,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

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
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::TOPIC,
            StatisticsBackendWrapper::DiscoveryStatus::UPDATE);

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
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::TOPIC,
            StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, topic_discovered_not_in_mask)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_TOPIC_DISCOVERY;
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, &domain_listener, mask, DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_topic_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::TOPIC,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::TOPIC,
            StatisticsBackendWrapper::DiscoveryStatus::UPDATE);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::TOPIC,
            StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, topic_discovered_no_listener)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_TOPIC_DISCOVERY);
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, nullptr, mask, DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_topic_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::TOPIC,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::TOPIC,
            StatisticsBackendWrapper::DiscoveryStatus::UPDATE);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::TOPIC,
            StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, topic_discovered_no_listener_not_in_mask)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_TOPIC_DISCOVERY;
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, nullptr, mask, DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_topic_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::TOPIC,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::TOPIC,
            StatisticsBackendWrapper::DiscoveryStatus::UPDATE);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::TOPIC,
            StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, datareader_discovered)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_DATAREADER_DISCOVERY);
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, &domain_listener, mask, DataKindMask::all());

    // Expectation: The user listener is called
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
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAREADER,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

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
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAREADER,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

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
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAREADER,
            StatisticsBackendWrapper::DiscoveryStatus::UPDATE);

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
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAREADER,
            StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, datareader_discovered_not_in_mask)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_DATAREADER_DISCOVERY;
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, &domain_listener, mask, DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_datareader_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAREADER,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAREADER,
            StatisticsBackendWrapper::DiscoveryStatus::UPDATE);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAREADER,
            StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, datareader_discovered_no_listener)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_DATAREADER_DISCOVERY);
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, nullptr, mask, DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_datareader_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAREADER,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAREADER,
            StatisticsBackendWrapper::DiscoveryStatus::UPDATE);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAREADER,
            StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, datareader_discovered_no_listener_not_in_mask)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_DATAREADER_DISCOVERY;
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, nullptr, mask, DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_datareader_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAREADER,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAREADER,
            StatisticsBackendWrapper::DiscoveryStatus::UPDATE);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAREADER,
            StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, datawriter_discovered)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_DATAWRITER_DISCOVERY);
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, &domain_listener, mask, DataKindMask::all());

    // Expectation: The user listener is called
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
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAWRITER,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

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
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAWRITER,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

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
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAWRITER,
            StatisticsBackendWrapper::DiscoveryStatus::UPDATE);

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
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAWRITER,
            StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, datawriter_discovered_not_in_mask)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_DATAWRITER_DISCOVERY;
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, &domain_listener, mask, DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_datawriter_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAWRITER,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAWRITER,
            StatisticsBackendWrapper::DiscoveryStatus::UPDATE);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAWRITER,
            StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, datawriter_discovered_no_listener)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::none();
    mask.set(CallbackKind::ON_DATAWRITER_DISCOVERY);
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, nullptr, mask, DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_datawriter_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAWRITER,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAWRITER,
            StatisticsBackendWrapper::DiscoveryStatus::UPDATE);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAWRITER,
            StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, datawriter_discovered_no_listener_not_in_mask)
{
    MockedDomainListener domain_listener;
    CallbackMask mask = CallbackMask::all();
    mask ^= CallbackKind::ON_DATAWRITER_DISCOVERY;
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, nullptr, mask, DataKindMask::all());

    // Expectation: The user listener is never called
    EXPECT_CALL(domain_listener, on_datawriter_discovery(_, _, _)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAWRITER,
            StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAWRITER,
            StatisticsBackendWrapper::DiscoveryStatus::UPDATE);

    StatisticsBackendWrapper::on_domain_entity_discovery(
            monitor_id,
            EntityId(1),
            EntityKind::DATAWRITER,
            StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY);
}

TEST(calling_user_listeners_tests, wrong_entity_kind)
{
    MockedPhysicalListener physical_listener;
    MockedDomainListener domain_listener;

    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, &domain_listener, CallbackMask::all(), DataKindMask::all());
    StatisticsBackendWrapper::set_physical_listener(
        &physical_listener,
        CallbackMask::all(),
        DataKindMask::all());

    // Expectation: The call throws
    ASSERT_THROW(StatisticsBackendWrapper::on_physical_entity_discovery(
                EntityId(0),
                EntityId(1),
                EntityKind::DOMAIN),
            eprosima::statistics_backend::Error);

    // Expectation: The call throws
    ASSERT_THROW(StatisticsBackendWrapper::on_physical_entity_discovery(
                EntityId(0),
                EntityId(1),
                EntityKind::PARTICIPANT),
            eprosima::statistics_backend::Error);

    // Expectation: The call throws
    ASSERT_THROW(StatisticsBackendWrapper::on_physical_entity_discovery(
                EntityId(0),
                EntityId(1),
                EntityKind::TOPIC),
            eprosima::statistics_backend::Error);

    // Expectation: The call throws
    ASSERT_THROW(StatisticsBackendWrapper::on_physical_entity_discovery(
                EntityId(0),
                EntityId(1),
                EntityKind::DATAREADER),
            eprosima::statistics_backend::Error);

    // Expectation: The call throws
    ASSERT_THROW(StatisticsBackendWrapper::on_physical_entity_discovery(
                EntityId(0),
                EntityId(1),
                EntityKind::DATAWRITER),
            eprosima::statistics_backend::Error);

    // Expectation: The call throws
    ASSERT_THROW(StatisticsBackendWrapper::on_physical_entity_discovery(
                EntityId(0),
                EntityId(1),
                EntityKind::INVALID),
            eprosima::statistics_backend::Error);

    // Expectation: The call throws
    ASSERT_THROW(StatisticsBackendWrapper::on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::HOST,
                StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY),
            eprosima::statistics_backend::Error);
    ASSERT_THROW(StatisticsBackendWrapper::on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::HOST,
                StatisticsBackendWrapper::DiscoveryStatus::UPDATE),
            eprosima::statistics_backend::Error);
    ASSERT_THROW(StatisticsBackendWrapper::on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::HOST,
                StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY),
            eprosima::statistics_backend::Error);

    // Expectation: The call throws
    ASSERT_THROW(StatisticsBackendWrapper::on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::USER,
                StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY),
            eprosima::statistics_backend::Error);
    ASSERT_THROW(StatisticsBackendWrapper::on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::USER,
                StatisticsBackendWrapper::DiscoveryStatus::UPDATE),
            eprosima::statistics_backend::Error);
    ASSERT_THROW(StatisticsBackendWrapper::on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::USER,
                StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY),
            eprosima::statistics_backend::Error);

    // Expectation: The call throws
    ASSERT_THROW(StatisticsBackendWrapper::on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::PROCESS,
                StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY),
            eprosima::statistics_backend::Error);
    ASSERT_THROW(StatisticsBackendWrapper::on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::PROCESS,
                StatisticsBackendWrapper::DiscoveryStatus::UPDATE),
            eprosima::statistics_backend::Error);
    ASSERT_THROW(StatisticsBackendWrapper::on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::PROCESS,
                StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY),
            eprosima::statistics_backend::Error);

    // Expectation: The call throws
    ASSERT_THROW(StatisticsBackendWrapper::on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::LOCATOR,
                StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY),
            eprosima::statistics_backend::Error);
    ASSERT_THROW(StatisticsBackendWrapper::on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::LOCATOR,
                StatisticsBackendWrapper::DiscoveryStatus::UPDATE),
            eprosima::statistics_backend::Error);
    ASSERT_THROW(StatisticsBackendWrapper::on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::LOCATOR,
                StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY),
            eprosima::statistics_backend::Error);

    // Expectation: The call throws
    ASSERT_THROW(StatisticsBackendWrapper::on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::INVALID,
                StatisticsBackendWrapper::DiscoveryStatus::DISCOVERY),
            eprosima::statistics_backend::Error);
    ASSERT_THROW(StatisticsBackendWrapper::on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::INVALID,
                StatisticsBackendWrapper::DiscoveryStatus::UPDATE),
            eprosima::statistics_backend::Error);
    ASSERT_THROW(StatisticsBackendWrapper::on_domain_entity_discovery(
                monitor_id,
                EntityId(1),
                EntityKind::INVALID,
                StatisticsBackendWrapper::DiscoveryStatus::UNDISCOVERY),
            eprosima::statistics_backend::Error);
}

class calling_user_data_listeners_tests : public ::testing::TestWithParam<std::tuple<DataKind>>
{

};

TEST_P(calling_user_data_listeners_tests, data_available)
{
    DataKind data_kind = std::get<0>(GetParam());

    MockedDomainListener domain_listener;
    CallbackMask callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
    DataKindMask data_mask = DataKindMask::none();
    data_mask.set(data_kind);
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, &domain_listener, callback_mask, data_mask);

    // Expectation: The user listener is called
    EXPECT_CALL(domain_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(1);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_data_available(
            monitor_id,
            EntityId(1),
            data_kind);

    // Expectation: The user listener is called again
    EXPECT_CALL(domain_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(1);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_data_available(
            monitor_id,
            EntityId(1),
            data_kind);
}

TEST_P(calling_user_data_listeners_tests, data_available_callback_not_in_mask)
{
    DataKind data_kind = std::get<0>(GetParam());

    MockedDomainListener domain_listener;
    CallbackMask callback_mask = CallbackMask::all();
    callback_mask ^= CallbackKind::ON_DATA_AVAILABLE;
    DataKindMask data_mask = DataKindMask::none();
    data_mask.set(data_kind);
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, &domain_listener, callback_mask, data_mask);

    // Expectation: The user listener is not called
    EXPECT_CALL(domain_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_data_available(
            monitor_id,
            EntityId(1),
            data_kind);
}

TEST_P(calling_user_data_listeners_tests, data_available_data_not_in_mask)
{
    DataKind data_kind = std::get<0>(GetParam());

    MockedDomainListener domain_listener;
    CallbackMask callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
    DataKindMask data_mask = DataKindMask::all();
    data_mask ^= data_kind;
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, &domain_listener, callback_mask, data_mask);

    // Expectation: The user listener is not called
    EXPECT_CALL(domain_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_data_available(
            monitor_id,
            EntityId(1),
            data_kind);
}

TEST_P(calling_user_data_listeners_tests, data_available_no_listener)
{
    DataKind data_kind = std::get<0>(GetParam());

    MockedDomainListener domain_listener;
    CallbackMask callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
    DataKindMask data_mask = DataKindMask::none();
    data_mask.set(data_kind);
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, nullptr, callback_mask, data_mask);

    // Expectation: The user listener is not called
    EXPECT_CALL(domain_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_data_available(
            monitor_id,
            EntityId(1),
            data_kind);
}

TEST_P(calling_user_data_listeners_tests, data_available_no_listener_callback_not_in_mask)
{
    DataKind data_kind = std::get<0>(GetParam());

    MockedDomainListener domain_listener;
    CallbackMask callback_mask = CallbackMask::all();
    callback_mask ^= CallbackKind::ON_DATA_AVAILABLE;
    DataKindMask data_mask = DataKindMask::none();
    data_mask.set(data_kind);
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, nullptr, callback_mask, data_mask);

    // Expectation: The user listener is not called
    EXPECT_CALL(domain_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_data_available(
            monitor_id,
            EntityId(1),
            data_kind);
}

TEST_P(calling_user_data_listeners_tests, data_available_no_listener_data_not_in_mask)
{
    DataKind data_kind = std::get<0>(GetParam());

    MockedDomainListener domain_listener;
    CallbackMask callback_mask = CallbackMask::none();
    callback_mask.set(CallbackKind::ON_DATA_AVAILABLE);
    DataKindMask data_mask = DataKindMask::all();
    data_mask ^= data_kind;
    auto monitor_id = StatisticsBackendWrapper::init_monitor(0, nullptr, callback_mask, data_mask);

    // Expectation: The user listener is not called
    EXPECT_CALL(domain_listener, on_data_available(monitor_id, EntityId(1), data_kind)).Times(0);

    // Execution: Call the listener
    StatisticsBackendWrapper::on_data_available(
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
