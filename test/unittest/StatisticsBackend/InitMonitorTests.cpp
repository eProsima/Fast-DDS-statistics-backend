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

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/rtps/attributes/ServerAttributes.h>
#include <fastdds/rtps/common/GuidPrefix_t.hpp>
#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
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
using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::statistics;
using namespace eprosima::fastrtps::rtps;


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

    std::map<EntityId, std::shared_ptr<eprosima::statistics_backend::details::Monitor>> init_monitors(
            DomainId domain_id,
            DomainListener* domain_listener,
            const std::string& server_guid_prefix,
            const std::string& server_locators,
            const CallbackMask& callback_mask,
            const DataKindMask& datakind_mask)
    {
        EntityId monitor_id = StatisticsBackend::init_monitor(
            domain_id,
            domain_listener,
            callback_mask,
            datakind_mask);
        EntityId monitor_id_1 = StatisticsBackend::init_monitor(
            server_locators,
            domain_listener,
            callback_mask,
            datakind_mask);
        EntityId monitor_id_2 = StatisticsBackend::init_monitor(
            server_guid_prefix,
            server_locators,
            domain_listener,
            callback_mask,
            datakind_mask);

        EXPECT_TRUE(monitor_id.is_valid());
        EXPECT_TRUE(monitor_id_1.is_valid());
        EXPECT_TRUE(monitor_id_2.is_valid());

        auto domain_monitors = details::StatisticsBackendData::get_instance()->monitors_by_entity_;

        /* Check that three monitors are created */
        EXPECT_EQ(domain_monitors.size(), 3);

        return domain_monitors;
    }

    void check_locator(
            const RemoteServerAttributes& server_qos,
            int32_t kind,
            const std::string& address,
            uint32_t port,
            bool is_unicast = true)
    {
        // Build the locator
        Locator_t locator(kind, port);
        if (kind == LOCATOR_KIND_UDPv4 || kind == LOCATOR_KIND_TCPv4)
        {
            IPLocator::setIPv4(locator, address);
        }
        else if (kind == LOCATOR_KIND_UDPv6 || kind == LOCATOR_KIND_TCPv6)
        {
            IPLocator::setIPv6(locator, address);
        }
        else
        {
            FAIL() << "Invalid locator kind";
        }

        // Look for it
        if (is_unicast)
        {
            EXPECT_NE(std::find(server_qos.metatrafficUnicastLocatorList.begin(),
                server_qos.metatrafficUnicastLocatorList.end(), locator), server_qos.metatrafficUnicastLocatorList.end());
        }
        else
        {
            EXPECT_NE(std::find(server_qos.metatrafficMulticastLocatorList.begin(),
                server_qos.metatrafficMulticastLocatorList.end(), locator), server_qos.metatrafficMulticastLocatorList.end());
        }
    }

    void check_participant_qos(
            const DomainParticipantQos& participant_qos,
            const std::string& server_guid_prefix)
    {
        EXPECT_EQ(participant_qos.wire_protocol().builtin.discovery_config.discoveryProtocol,
            eprosima::fastrtps::rtps::DiscoveryProtocol_t::SUPER_CLIENT);

        RemoteServerAttributes server_qos =
            participant_qos.wire_protocol().builtin.discovery_config.m_DiscoveryServers.front();
        GuidPrefix_t guid_prefix;
        std::istringstream(server_guid_prefix) >> guid_prefix;
        EXPECT_EQ(server_qos.guidPrefix, guid_prefix);

        check_locator(server_qos, LOCATOR_KIND_UDPv4, "127.0.0.1", 11811);
    }

    void check_dds_entities(
            const std::shared_ptr<eprosima::statistics_backend::details::Monitor> monitor,
            const std::string& server_guid_prefix = "")
    {
        EXPECT_NE(nullptr, monitor->participant);

        if (!server_guid_prefix.empty())
        {
            DomainParticipantQos participant_qos;
            monitor->participant->get_qos(participant_qos);
            check_participant_qos(participant_qos, server_guid_prefix);
        }

        EXPECT_NE(nullptr, monitor->subscriber);

        for (auto topic : topic_types_)
        {
            EXPECT_NE(nullptr, monitor->topics[topic.first]);
            EXPECT_NE(nullptr, monitor->readers[topic.first]);
        }
    }

};


constexpr const CallbackKind init_monitor_tests::all_callback_kinds_[];
constexpr const DataKind init_monitor_tests::all_data_kinds_[];

TEST_F(init_monitor_tests, init_monitor_domain_id_all_callback_all_data)
{
    DomainId domain_id = 0;
    DomainListener domain_listener;
    std::string server_guid_prefix = "44.53.01.5f.45.50.52.4f.53.49.4d.41";
    std::string server_locators = "UDPv4:[127.0.0.1]:11811";

    auto domain_monitors = init_monitors(domain_id, &domain_listener, server_guid_prefix, server_locators,
        all_callback_mask_, all_datakind_mask_);

    std::vector<EntityId> monitor_ids;
    for (const auto& monitor : domain_monitors)
    {
        /* Check that the domain listener is set correctly */
        EXPECT_EQ(&domain_listener, domain_monitors[monitor.first]->domain_listener);

        /* Check that the CallbackMask is set correctly */
        for (const auto& callback : init_monitor_tests::all_callback_kinds_)
        {
            EXPECT_TRUE(domain_monitors[monitor.first]->domain_callback_mask.is_set(callback));
        }

        /* Check that the DataKindMask is set correctly */
        for (const auto& datakind : init_monitor_tests::all_data_kinds_)
        {
            EXPECT_TRUE(domain_monitors[monitor.first]->data_mask.is_set(datakind));
        }

        monitor_ids.push_back(monitor.first);
    }

    /* Check the created DDS entities */
    check_dds_entities(domain_monitors[monitor_ids[0]]);
    check_dds_entities(domain_monitors[monitor_ids[1]], DEFAULT_ROS2_SERVER_GUIDPREFIX);
    check_dds_entities(domain_monitors[monitor_ids[2]], server_guid_prefix);

    // Stop the monitor to avoid interfering on the next test
    for (const auto& monitor : domain_monitors)
    {
        StatisticsBackend::stop_monitor(monitor.first);
    }
}

TEST_F(init_monitor_tests, init_monitor_domain_id_no_callback_all_data)
{
    DomainId domain_id = 0;
    DomainListener domain_listener;
    std::string server_guid_prefix = "44.53.01.5f.45.50.52.4f.53.49.4d.41";
    std::string server_locators = "UDPv4:[127.0.0.1]:11811";

    auto domain_monitors = init_monitors(domain_id, &domain_listener, server_guid_prefix, server_locators,
        CallbackMask::none(), all_datakind_mask_);

    std::vector<EntityId> monitor_ids;
    for (const auto& monitor : domain_monitors)
    {
        /* Check that the domain listener is set correctly */
        EXPECT_EQ(&domain_listener, domain_monitors[monitor.first]->domain_listener);

        /* Check that the CallbackMask is set correctly */
        for (auto callback : init_monitor_tests::all_callback_kinds_)
        {
            EXPECT_FALSE(domain_monitors[monitor.first]->domain_callback_mask.is_set(callback));
        }

        /* Check that the DataKindMask is set correctly */
        for (auto datakind : init_monitor_tests::all_data_kinds_)
        {
            EXPECT_TRUE(domain_monitors[monitor.first]->data_mask.is_set(datakind));
        }

        monitor_ids.push_back(monitor.first);
    }

    /* Check the created DDS entities */
    check_dds_entities(domain_monitors[monitor_ids[0]]);
    check_dds_entities(domain_monitors[monitor_ids[1]], DEFAULT_ROS2_SERVER_GUIDPREFIX);
    check_dds_entities(domain_monitors[monitor_ids[2]], server_guid_prefix);

    // Stop the monitor to avoid interfering on the next test
    for (const auto& monitor : domain_monitors)
    {
        StatisticsBackend::stop_monitor(monitor.first);
    }
}

TEST_F(init_monitor_tests, init_monitor_domain_id_all_callback_no_data)
{
    DomainId domain_id = 0;
    DomainListener domain_listener;
    std::string server_guid_prefix = "44.53.01.5f.45.50.52.4f.53.49.4d.41";
    std::string server_locators = "UDPv4:[127.0.0.1]:11811";

    auto domain_monitors = init_monitors(domain_id, &domain_listener, server_guid_prefix, server_locators,
        all_callback_mask_, DataKindMask::none());

    std::vector<EntityId> monitor_ids;
    for (const auto& monitor : domain_monitors)
    {
        /* Check that the domain listener is set correctly */
        EXPECT_EQ(&domain_listener, domain_monitors[monitor.first]->domain_listener);

        /* Check that the CallbackMask is set correctly */
        for (auto callback : init_monitor_tests::all_callback_kinds_)
        {
            EXPECT_TRUE(domain_monitors[monitor.first]->domain_callback_mask.is_set(callback));
        }

        /* Check that the DataKindMask is set correctly */
        for (auto datakind : init_monitor_tests::all_data_kinds_)
        {
            EXPECT_FALSE(domain_monitors[monitor.first]->data_mask.is_set(datakind));
        }

        monitor_ids.push_back(monitor.first);
    }

    /* Check the created DDS entities */
    check_dds_entities(domain_monitors[monitor_ids[0]]);
    check_dds_entities(domain_monitors[monitor_ids[1]], DEFAULT_ROS2_SERVER_GUIDPREFIX);
    check_dds_entities(domain_monitors[monitor_ids[2]], server_guid_prefix);

    // Stop the monitor to avoid interfering on the next test
    for (const auto& monitor : domain_monitors)
    {
        StatisticsBackend::stop_monitor(monitor.first);
    }
}

TEST_F(init_monitor_tests, init_monitor_domain_id_null_listener_all_data)
{
    DomainId domain_id = 0;
    std::string server_guid_prefix = "44.53.01.5f.45.50.52.4f.53.49.4d.41";
    std::string server_locators = "UDPv4:[127.0.0.1]:11811";

    auto domain_monitors = init_monitors(domain_id, nullptr, server_guid_prefix, server_locators,
        all_callback_mask_, all_datakind_mask_);

    std::vector<EntityId> monitor_ids;
    for (const auto& monitor : domain_monitors)
    {
        /* Check that the domain listener is set correctly */
        EXPECT_EQ(nullptr, domain_monitors[monitor.first]->domain_listener);

        /* Check that the CallbackMask is set correctly */
        for (auto callback : init_monitor_tests::all_callback_kinds_)
        {
            EXPECT_TRUE(domain_monitors[monitor.first]->domain_callback_mask.is_set(callback));
        }

        /* Check that the DataKindMask is set correctly */
        for (auto datakind : init_monitor_tests::all_data_kinds_)
        {
            EXPECT_TRUE(domain_monitors[monitor.first]->data_mask.is_set(datakind));
        }

        monitor_ids.push_back(monitor.first);
    }

    /* Check the created DDS entities */
    check_dds_entities(domain_monitors[monitor_ids[0]]);
    check_dds_entities(domain_monitors[monitor_ids[1]], DEFAULT_ROS2_SERVER_GUIDPREFIX);
    check_dds_entities(domain_monitors[monitor_ids[2]], server_guid_prefix);

    // Stop the monitor to avoid interfering on the next test
    for (const auto& monitor : domain_monitors)
    {
        StatisticsBackend::stop_monitor(monitor.first);
    }
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
    check_dds_entities(domain_monitors[monitor_id1]);
    check_dds_entities(domain_monitors[monitor_id2]);

    // Stop the monitor to avoid interfering on the next test
    StatisticsBackend::stop_monitor(monitor_id1);
    StatisticsBackend::stop_monitor(monitor_id2);
}

TEST_F(init_monitor_tests, init_monitor_twice)
{
    DomainId domain_id = 0;
    DomainListener domain_listener;
    std::string server_guid_prefix = "44.53.01.5f.45.50.52.4f.53.49.4d.41";
    std::string server_locators = "UDPv4:[127.0.0.1]:11811";

    init_monitors(domain_id, &domain_listener, server_guid_prefix, server_locators,
        all_callback_mask_, all_datakind_mask_);

    EXPECT_THROW(StatisticsBackend::init_monitor(
                domain_id,
                nullptr,
                CallbackMask::none(),
                DataKindMask::none()), BadParameter);
    EXPECT_THROW(StatisticsBackend::init_monitor(
                server_locators,
                nullptr,
                CallbackMask::none(),
                DataKindMask::none()), BadParameter);
    EXPECT_THROW(StatisticsBackend::init_monitor(
                server_guid_prefix,
                server_locators,
                nullptr,
                CallbackMask::none(),
                DataKindMask::none()), BadParameter);

    auto domain_monitors = details::StatisticsBackendData::get_instance()->monitors_by_entity_;

        /* Check that three monitors are created */
    EXPECT_EQ(domain_monitors.size(), 3);

    std::vector<EntityId> monitor_ids;
    for (const auto& monitor : domain_monitors)
    {
        /* Check that the domain listener is set correctly */
        EXPECT_EQ(&domain_listener, domain_monitors[monitor.first]->domain_listener);

        /* Check that the CallbackMask is set correctly */
        for (auto callback : init_monitor_tests::all_callback_kinds_)
        {
            EXPECT_TRUE(domain_monitors[monitor.first]->domain_callback_mask.is_set(callback));
        }

        /* Check that the DataKindMask is set correctly */
        for (auto datakind : init_monitor_tests::all_data_kinds_)
        {
            EXPECT_TRUE(domain_monitors[monitor.first]->data_mask.is_set(datakind));
        }

        monitor_ids.push_back(monitor.first);
    }

    /* Check the created DDS entities */
    check_dds_entities(domain_monitors[monitor_ids[0]]);
    check_dds_entities(domain_monitors[monitor_ids[1]], DEFAULT_ROS2_SERVER_GUIDPREFIX);
    check_dds_entities(domain_monitors[monitor_ids[2]], server_guid_prefix);

    // Stop the monitor to avoid interfering on the next test
    for (const auto& monitor : domain_monitors)
    {
        StatisticsBackend::stop_monitor(monitor.first);
    }
}

TEST_F(init_monitor_tests, init_server_monitor_several_locators)
{
    std::string server_locators =
            // unicast addresses
            "UDPv4:[127.0.0.1]:11811;TCPv4:[127.0.0.1]:11812;UDPv6:[::1]:11813;TCPv6:[::1]:11814;"
            // multicast addresses
            "UDPv4:[239.255.0.1]:11821;UDPv6:[ff1e::ffff:efff:1]:11823";
    EntityId monitor_id =  StatisticsBackend::init_monitor(server_locators);

    EXPECT_TRUE(monitor_id.is_valid());

    auto domain_monitors = details::StatisticsBackendData::get_instance()->monitors_by_entity_;

    /* Check that a monitor is created */
    EXPECT_EQ(domain_monitors.size(), 1);

    DomainParticipantQos participant_qos;
    domain_monitors[monitor_id]->participant->get_qos(participant_qos);

    EXPECT_EQ(participant_qos.wire_protocol().builtin.discovery_config.discoveryProtocol,
        eprosima::fastrtps::rtps::DiscoveryProtocol_t::SUPER_CLIENT);

    const RemoteServerAttributes& server_qos =
        participant_qos.wire_protocol().builtin.discovery_config.m_DiscoveryServers.front();

    check_locator(server_qos, LOCATOR_KIND_UDPv4, "127.0.0.1", 11811);
    check_locator(server_qos, LOCATOR_KIND_TCPv4, "127.0.0.1", 11812);
    check_locator(server_qos, LOCATOR_KIND_UDPv6, "::1", 11813);
    check_locator(server_qos, LOCATOR_KIND_TCPv6, "::1", 11814);

    check_locator(server_qos, LOCATOR_KIND_UDPv4, "239.255.0.1", 11821, false);
    check_locator(server_qos, LOCATOR_KIND_UDPv6, "ff1e::ffff:efff:1", 11823, false);

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

TEST_F(init_monitor_tests, init_monitor_check_participant_name)
{
    DomainId domain_id = 0;
    DomainListener domain_listener;

    EntityId monitor_id = StatisticsBackend::init_monitor(
        domain_id,
        &domain_listener,
        all_callback_mask_,
        all_datakind_mask_);

    EXPECT_TRUE(monitor_id.is_valid());

    std::map<EntityId, std::shared_ptr<details::Monitor>> domain_monitors =
            details::StatisticsBackendData::get_instance()->monitors_by_entity_;

    /* Check that only one monitor is created */
    EXPECT_EQ(domain_monitors.size(), 1);

    eprosima::fastdds::dds::DomainParticipant* participant = domain_monitors[monitor_id]->participant;
    eprosima::fastdds::dds::DomainParticipantQos participant_qos = participant->get_qos();

    /* Check that the DomainParticipant name is set correctly */
    EXPECT_EQ(participant_qos.name(), "monitor_domain_0");


    /* Stop the monitor to avoid interfering on the next test */
    StatisticsBackend::stop_monitor(monitor_id);
}

TEST_F(init_monitor_tests, init_monitor_check_participant_transport)
{
    DomainId domain_id = 0;
    DomainListener domain_listener;

    EntityId monitor_id = StatisticsBackend::init_monitor(
        domain_id,
        &domain_listener,
        all_callback_mask_,
        all_datakind_mask_);

    EXPECT_TRUE(monitor_id.is_valid());

    std::map<EntityId, std::shared_ptr<details::Monitor>> domain_monitors =
            details::StatisticsBackendData::get_instance()->monitors_by_entity_;

    /* Check that only one monitor is created */
    EXPECT_EQ(domain_monitors.size(), 1);

    eprosima::fastdds::dds::DomainParticipant* participant = domain_monitors[monitor_id]->participant;
    eprosima::fastdds::dds::DomainParticipantQos participant_qos = participant->get_qos();

    /* Check that the DomainParticipant builtin transports are disabled */
    EXPECT_FALSE(participant_qos.transport().use_builtin_transports);

    /* Check that the DomainParticipant has only one transport set and it is a UDPv4TransportDescriptor */
    EXPECT_EQ(participant_qos.transport().user_transports.size(), 1);
    std::shared_ptr<eprosima::fastdds::rtps::UDPv4TransportDescriptor> participant_transport =
            std::dynamic_pointer_cast<eprosima::fastdds::rtps::UDPv4TransportDescriptor>(
        participant_qos.transport().user_transports.back());
    EXPECT_TRUE(nullptr != participant_transport);

    /* Stop the monitor to avoid interfering on the next test */
    StatisticsBackend::stop_monitor(monitor_id);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
