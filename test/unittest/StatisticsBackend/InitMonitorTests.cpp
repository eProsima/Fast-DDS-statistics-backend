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

#include <list>
#include <string>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/statistics/topic_names.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/listener/CallbackMask.hpp>
#include <fastdds_statistics_backend/listener/DomainListener.hpp>
#include <fastdds_statistics_backend/StatisticsBackend.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/types.hpp>

#include <database/database_queue.hpp>
#include <Monitor.hpp>
#include <StatisticsBackendData.hpp>
#include <topic_types/typesPubSubTypes.h>

using namespace eprosima::statistics_backend;
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::statistics;


class init_monitor_tests : public ::testing::Test
{

public:

    // all bits in the callback mask
    static constexpr const CallbackKind all_callback_kinds_[] = {
        CallbackKind::ON_DATA_AVAILABLE,
        CallbackKind::ON_DATAREADER_DISCOVERY,
        CallbackKind::ON_DATAWRITER_DISCOVERY,
        CallbackKind::ON_HOST_DISCOVERY,
        CallbackKind::ON_LOCATOR_DISCOVERY,
        CallbackKind::ON_PARTICIPANT_DISCOVERY,
        CallbackKind::ON_PROCESS_DISCOVERY,
        CallbackKind::ON_TOPIC_DISCOVERY,
        CallbackKind::ON_USER_DISCOVERY };

    // a mask with all callbacks
    CallbackMask all_callback_mask_;

    // all bits in the datakind mask
    static constexpr const DataKind all_data_kinds_[] = {
        DataKind::ACKNACK_COUNT,
        DataKind::DATA_COUNT,
        DataKind::DISCOVERY_TIME,
        DataKind::EDP_PACKETS,
        DataKind::FASTDDS_LATENCY,
        DataKind::GAP_COUNT,
        DataKind::HEARTBEAT_COUNT,
        DataKind::NACKFRAG_COUNT,
        DataKind::NETWORK_LATENCY,
        DataKind::PDP_PACKETS,
        DataKind::PUBLICATION_THROUGHPUT,
        DataKind::RESENT_DATA,
        DataKind::RTPS_BYTES_LOST,
        DataKind::RTPS_BYTES_SENT,
        DataKind::RTPS_PACKETS_LOST,
        DataKind::RTPS_PACKETS_SENT,
        DataKind::SAMPLE_DATAS,
        DataKind::SUBSCRIPTION_THROUGHPUT };

    // a mask with all data kinds
    DataKindMask all_datakind_mask_;

    // Relation between topic names and data types
    std::map<const char*, TopicDataType*> topic_types_;

    init_monitor_tests()
    {
        // Fill the map of topics and types
        topic_types_[HISTORY_LATENCY_TOPIC] = new WriterReaderDataPubSubType();
        topic_types_[NETWORK_LATENCY_TOPIC] = new Locator2LocatorDataPubSubType();
        topic_types_[PUBLICATION_THROUGHPUT_TOPIC] = new EntityDataPubSubType();
        topic_types_[SUBSCRIPTION_THROUGHPUT_TOPIC] = new EntityDataPubSubType();
        topic_types_[RTPS_SENT_TOPIC] = new Entity2LocatorTrafficPubSubType();
        topic_types_[RTPS_LOST_TOPIC] = new Entity2LocatorTrafficPubSubType();
        topic_types_[RESENT_DATAS_TOPIC] = new EntityCountPubSubType();
        topic_types_[HEARTBEAT_COUNT_TOPIC] = new EntityCountPubSubType();
        topic_types_[ACKNACK_COUNT_TOPIC] = new EntityCountPubSubType();
        topic_types_[NACKFRAG_COUNT_TOPIC] = new EntityCountPubSubType();
        topic_types_[GAP_COUNT_TOPIC] = new EntityCountPubSubType();
        topic_types_[DATA_COUNT_TOPIC] = new EntityCountPubSubType();
        topic_types_[PDP_PACKETS_TOPIC] = new EntityCountPubSubType();
        topic_types_[EDP_PACKETS_TOPIC] = new EntityCountPubSubType();
        topic_types_[DISCOVERY_TOPIC] = new DiscoveryTimePubSubType();
        topic_types_[SAMPLE_DATAS_TOPIC] = new SampleIdentityCountPubSubType();
        topic_types_[PHYSICAL_DATA_TOPIC] = new PhysicalDataPubSubType();

        // Set the bits for the masks
        for (auto callback : init_monitor_tests::all_callback_kinds_)
        {
            all_callback_mask_.set(callback);
        }

        for (auto datakind : init_monitor_tests::all_data_kinds_)
        {
            all_datakind_mask_.set(datakind);
        }

        // Set the profile to ignore discovery data from other processes
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_XML_profiles_file("profile.xml");
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->load_profiles();
    }

    ~init_monitor_tests()
    {
        // Clear memory
        for (auto it : topic_types_)
        {
            delete it.second;
        }

        details::StatisticsBackendData::reset_instance();
    }

};

constexpr const CallbackKind init_monitor_tests::all_callback_kinds_[];
constexpr const DataKind init_monitor_tests::all_data_kinds_[];

TEST_F(init_monitor_tests, init_monitor_domain_id_all_callback_all_data)
{
    DomainId domain_id = 0;
    DomainListener domain_listener;

    EntityId monitor_id = StatisticsBackend::init_monitor(
        domain_id,
        &domain_listener,
        all_callback_mask_,
        all_datakind_mask_);

    EXPECT_TRUE(monitor_id.is_valid());

    auto domain_monitors = details::StatisticsBackendData::get_instance()->monitors_by_entity_;

    /* Check that only one monitor is created */
    EXPECT_EQ(domain_monitors.size(), 1);

    /* Check that the domain listener is set correctly */
    EXPECT_EQ(&domain_listener, domain_monitors[monitor_id]->domain_listener);

    /* Check that the CallbackMask is set correctly */
    for (auto callback : init_monitor_tests::all_callback_kinds_)
    {
        EXPECT_TRUE(domain_monitors[monitor_id]->domain_callback_mask.is_set(callback));
    }

    /* Check that the DataKindMask is set correctly */
    for (auto datakind : init_monitor_tests::all_data_kinds_)
    {
        EXPECT_TRUE(domain_monitors[monitor_id]->data_mask.is_set(datakind));
    }

    /* Check the created DDS entities */
    EXPECT_NE(nullptr, domain_monitors[monitor_id]->participant);
    EXPECT_NE(nullptr, domain_monitors[monitor_id]->subscriber);
    for (auto topic : topic_types_)
    {
        EXPECT_NE(nullptr, domain_monitors[monitor_id]->topics[topic.first]);
        EXPECT_NE(nullptr, domain_monitors[monitor_id]->readers[topic.first]);
    }

    // Stop the monitor to avoid interfering on the next test
    StatisticsBackend::stop_monitor(monitor_id);
}

TEST_F(init_monitor_tests, init_monitor_domain_id_no_callback_all_data)
{
    DomainId domain_id = 0;
    DomainListener domain_listener;

    EntityId monitor_id = StatisticsBackend::init_monitor(
        domain_id,
        &domain_listener,
        CallbackMask::none(),
        all_datakind_mask_);

    EXPECT_TRUE(monitor_id.is_valid());

    auto domain_monitors = details::StatisticsBackendData::get_instance()->monitors_by_entity_;

    /* Check that only one monitor is created */
    EXPECT_EQ(domain_monitors.size(), 1);

    /* Check that the domain listener is set correctly */
    EXPECT_EQ(&domain_listener, domain_monitors[monitor_id]->domain_listener);

    /* Check that the CallbackMask is set correctly */
    for (auto callback : init_monitor_tests::all_callback_kinds_)
    {
        EXPECT_FALSE(domain_monitors[monitor_id]->domain_callback_mask.is_set(callback));
    }

    /* Check that the DataKindMask is set correctly */
    for (auto datakind : init_monitor_tests::all_data_kinds_)
    {
        EXPECT_TRUE(domain_monitors[monitor_id]->data_mask.is_set(datakind));
    }

    /* Check the created DDS entities */
    EXPECT_NE(nullptr, domain_monitors[monitor_id]->participant);
    EXPECT_NE(nullptr, domain_monitors[monitor_id]->subscriber);
    for (auto topic : topic_types_)
    {
        EXPECT_NE(nullptr, domain_monitors[monitor_id]->topics[topic.first]);
        EXPECT_NE(nullptr, domain_monitors[monitor_id]->readers[topic.first]);
    }

    // Stop the monitor to avoid interfering on the next test
    StatisticsBackend::stop_monitor(monitor_id);
}

TEST_F(init_monitor_tests, init_monitor_domain_id_all_callback_no_data)
{
    DomainId domain_id = 0;
    DomainListener domain_listener;

    EntityId monitor_id = StatisticsBackend::init_monitor(
        domain_id,
        &domain_listener,
        all_callback_mask_,
        DataKindMask::none());

    EXPECT_TRUE(monitor_id.is_valid());

    auto domain_monitors = details::StatisticsBackendData::get_instance()->monitors_by_entity_;

    /* Check that only one monitor is created */
    EXPECT_EQ(domain_monitors.size(), 1);

    /* Check that the domain listener is set correctly */
    EXPECT_EQ(&domain_listener, domain_monitors[monitor_id]->domain_listener);

    /* Check that the CallbackMask is set correctly */
    for (auto callback : init_monitor_tests::all_callback_kinds_)
    {
        EXPECT_TRUE(domain_monitors[monitor_id]->domain_callback_mask.is_set(callback));
    }

    /* Check that the DataKindMask is set correctly */
    for (auto datakind : init_monitor_tests::all_data_kinds_)
    {
        EXPECT_FALSE(domain_monitors[monitor_id]->data_mask.is_set(datakind));
    }

    /* Check the created DDS entities */
    EXPECT_NE(nullptr, domain_monitors[monitor_id]->participant);
    EXPECT_NE(nullptr, domain_monitors[monitor_id]->subscriber);
    for (auto topic : topic_types_)
    {
        EXPECT_NE(nullptr, domain_monitors[monitor_id]->topics[topic.first]);
        EXPECT_NE(nullptr, domain_monitors[monitor_id]->readers[topic.first]);
    }

    // Stop the monitor to avoid interfering on the next test
    StatisticsBackend::stop_monitor(monitor_id);
}

TEST_F(init_monitor_tests, init_monitor_domain_id_null_listener_all_data)
{
    DomainId domain_id = 0;

    EntityId monitor_id = StatisticsBackend::init_monitor(
        domain_id,
        nullptr,
        all_callback_mask_,
        all_datakind_mask_);

    EXPECT_TRUE(monitor_id.is_valid());

    auto domain_monitors = details::StatisticsBackendData::get_instance()->monitors_by_entity_;

    /* Check that only one monitor is created */
    EXPECT_EQ(domain_monitors.size(), 1);

    /* Check that the domain listener is set correctly */
    EXPECT_EQ(nullptr, domain_monitors[monitor_id]->domain_listener);

    /* Check that the CallbackMask is set correctly */
    for (auto callback : init_monitor_tests::all_callback_kinds_)
    {
        EXPECT_TRUE(domain_monitors[monitor_id]->domain_callback_mask.is_set(callback));
    }

    /* Check that the DataKindMask is set correctly */
    for (auto datakind : init_monitor_tests::all_data_kinds_)
    {
        EXPECT_TRUE(domain_monitors[monitor_id]->data_mask.is_set(datakind));
    }

    /* Check the created DDS entities */
    EXPECT_NE(nullptr, domain_monitors[monitor_id]->participant);
    EXPECT_NE(nullptr, domain_monitors[monitor_id]->subscriber);
    for (auto topic : topic_types_)
    {
        EXPECT_NE(nullptr, domain_monitors[monitor_id]->topics[topic.first]);
        EXPECT_NE(nullptr, domain_monitors[monitor_id]->readers[topic.first]);
    }

    // Stop the monitor to avoid interfering on the next test
    StatisticsBackend::stop_monitor(monitor_id);
}

TEST_F(init_monitor_tests, init_monitor_several_monitors)
{
    DomainId domain_id1 = 0;
    DomainListener domain_listener;
    EntityId monitor_id1 = StatisticsBackend::init_monitor(
        domain_id1,
        &domain_listener,
        all_callback_mask_,
        all_datakind_mask_);

    EXPECT_TRUE(monitor_id1.is_valid());

    DomainId domain_id2 = 10;
    EntityId monitor_id2 = StatisticsBackend::init_monitor(
        domain_id2,
        nullptr,
        CallbackMask::none(),
        DataKindMask::none());

    EXPECT_TRUE(monitor_id2.is_valid());

    auto domain_monitors = details::StatisticsBackendData::get_instance()->monitors_by_entity_;

    /* Check that two monitors are created */
    EXPECT_EQ(domain_monitors.size(), 2);

    /* Check that the domain listener is set correctly */
    EXPECT_EQ(&domain_listener, domain_monitors[monitor_id1]->domain_listener);
    EXPECT_EQ(nullptr, domain_monitors[monitor_id2]->domain_listener);

    /* Check that the CallbackMask is set correctly */
    for (auto callback : init_monitor_tests::all_callback_kinds_)
    {
        EXPECT_TRUE(domain_monitors[monitor_id1]->domain_callback_mask.is_set(callback));
        EXPECT_FALSE(domain_monitors[monitor_id2]->domain_callback_mask.is_set(callback));
    }

    /* Check that the DataKindMask is set correctly */
    for (auto datakind : init_monitor_tests::all_data_kinds_)
    {
        EXPECT_TRUE(domain_monitors[monitor_id1]->data_mask.is_set(datakind));
        EXPECT_FALSE(domain_monitors[monitor_id2]->data_mask.is_set(datakind));
    }

    /* Check the created DDS entities */
    EXPECT_NE(nullptr, domain_monitors[monitor_id1]->participant);
    EXPECT_NE(nullptr, domain_monitors[monitor_id2]->participant);

    EXPECT_NE(nullptr, domain_monitors[monitor_id1]->subscriber);
    EXPECT_NE(nullptr, domain_monitors[monitor_id2]->subscriber);

    for (auto topic : topic_types_)
    {
        EXPECT_NE(nullptr, domain_monitors[monitor_id1]->topics[topic.first]);
        EXPECT_NE(nullptr, domain_monitors[monitor_id2]->topics[topic.first]);

        EXPECT_NE(nullptr, domain_monitors[monitor_id1]->readers[topic.first]);
        EXPECT_NE(nullptr, domain_monitors[monitor_id2]->readers[topic.first]);
    }

    // Stop the monitor to avoid interfering on the next test
    StatisticsBackend::stop_monitor(monitor_id1);
    StatisticsBackend::stop_monitor(monitor_id2);
}

TEST_F(init_monitor_tests, init_monitor_twice)
{
    DomainId domain_id = 0;
    DomainListener domain_listener;
    EntityId monitor_id = StatisticsBackend::init_monitor(
        domain_id,
        &domain_listener,
        all_callback_mask_,
        all_datakind_mask_);

    EXPECT_TRUE(monitor_id.is_valid());

    EXPECT_THROW(StatisticsBackend::init_monitor(
                domain_id,
                nullptr,
                CallbackMask::none(),
                DataKindMask::none()), BadParameter);

    auto domain_monitors = details::StatisticsBackendData::get_instance()->monitors_by_entity_;

    /* Check that only one monitor is created */
    EXPECT_EQ(domain_monitors.size(), 1);

    /* Check that the domain listener is set correctly */
    EXPECT_EQ(&domain_listener, domain_monitors[monitor_id]->domain_listener);

    /* Check that the CallbackMask is set correctly */
    for (auto callback : init_monitor_tests::all_callback_kinds_)
    {
        EXPECT_TRUE(domain_monitors[monitor_id]->domain_callback_mask.is_set(callback));
    }

    /* Check that the DataKindMask is set correctly */
    for (auto datakind : init_monitor_tests::all_data_kinds_)
    {
        EXPECT_TRUE(domain_monitors[monitor_id]->data_mask.is_set(datakind));
    }

    /* Check the created DDS entities */
    EXPECT_NE(nullptr, domain_monitors[monitor_id]->participant);
    EXPECT_NE(nullptr, domain_monitors[monitor_id]->subscriber);

    for (auto topic : topic_types_)
    {
        EXPECT_NE(nullptr, domain_monitors[monitor_id]->topics[topic.first]);
        EXPECT_NE(nullptr, domain_monitors[monitor_id]->readers[topic.first]);
    }

    // Stop the monitor to avoid interfering on the next test
    StatisticsBackend::stop_monitor(monitor_id);
}

TEST_F(init_monitor_tests, stop_monitor)
{
    DomainId domain_id = 0;
    DomainListener domain_listener;
    EntityId monitor_id = StatisticsBackend::init_monitor(
        domain_id,
        &domain_listener,
        all_callback_mask_,
        all_datakind_mask_);

    EXPECT_TRUE(monitor_id.is_valid());

    // Try stopping another monitor
    EntityId other_monitor_id = EntityId(100);
    EXPECT_THROW(StatisticsBackend::is_active(other_monitor_id), BadParameter);
    EXPECT_THROW(StatisticsBackend::stop_monitor(other_monitor_id), BadParameter);
    EXPECT_THROW(StatisticsBackend::is_active(other_monitor_id), BadParameter);

    // Stop the proper monitor
    EXPECT_TRUE(StatisticsBackend::is_active(monitor_id));
    EXPECT_NO_THROW(StatisticsBackend::stop_monitor(monitor_id));
    EXPECT_FALSE(StatisticsBackend::is_active(monitor_id));

    // Reopen stopped monitor
    EXPECT_THROW(StatisticsBackend::init_monitor(
                domain_id,
                &domain_listener,
                all_callback_mask_,
                all_datakind_mask_),
            BadParameter);

    // Open another monitor
    DomainId other_domain_id = 100;
    other_monitor_id = StatisticsBackend::init_monitor(
        other_domain_id,
        &domain_listener,
        all_callback_mask_,
        all_datakind_mask_);

    EXPECT_TRUE(other_monitor_id.is_valid());
    EXPECT_TRUE(StatisticsBackend::is_active(other_monitor_id));

    // Reset the singleton instead of removing the monitor
    details::StatisticsBackendData::reset_instance();
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
