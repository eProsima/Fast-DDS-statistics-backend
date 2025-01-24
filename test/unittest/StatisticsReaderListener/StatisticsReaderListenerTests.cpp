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

#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/statistics/topic_names.hpp>

#include <fastdds/utils/IPLocator.hpp>

#include <database/database.hpp>
#include <database/database_queue.hpp>
#include <subscriber/StatisticsReaderListener.hpp>
#include <fastdds_statistics_backend/topic_types/types.hpp>

#include <fastdds_statistics_backend/topic_types/monitorservice_types.hpp>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::AnyNumber;

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::statistics;
using namespace eprosima::statistics_backend::database;
using namespace eprosima::statistics_backend::subscriber;
using namespace eprosima::statistics_backend::database;

using StatisticsData = eprosima::fastdds::statistics::Data;
using EntityId = eprosima::statistics_backend::EntityId;
using EntityKind = eprosima::statistics_backend::EntityKind;
using DataKind = eprosima::statistics_backend::DataKind;

struct InsertDataArgs
{
    InsertDataArgs (
            std::function<void(
                const EntityId&,
                const EntityId&,
                const StatisticsSample&)> func)
        : callback_(func)
    {
    }

    void insert(
            const EntityId& domain_id,
            const EntityId& id,
            const StatisticsSample& sample)
    {
        return callback_(domain_id, id, sample);
    }

    std::function<void(
                const EntityId&,
                const EntityId&,
                const StatisticsSample&)> callback_;
};

struct InsertMonitorServiceDataArgs
{
    InsertMonitorServiceDataArgs (
            std::function<bool(
                const EntityId&,
                const EntityId&,
                const eprosima::statistics_backend::MonitorServiceSample&)> func)
        : callback_(func)
    {
    }

    bool insert(
            const EntityId& domain_id,
            const EntityId& id,
            const eprosima::statistics_backend::MonitorServiceSample& sample)
    {
        return callback_(domain_id, id, sample);
    }

    std::function<bool(
                const EntityId&,
                const EntityId&,
                const eprosima::statistics_backend::MonitorServiceSample&)> callback_;
};

class statistics_reader_listener_tests : public ::testing::Test
{

public:

    Database database_;
    DatabaseDataQueue<eprosima::fastdds::statistics::Data> data_queue_;
    DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData> monitor_service_data_queue_;
    eprosima::statistics_backend::DataKindMask data_mask_;
    StatisticsReaderListener reader_listener_;
    eprosima::fastdds::dds::DataReader datareader_;

    statistics_reader_listener_tests()
        : database_()
        , data_queue_(&database_)
        , monitor_service_data_queue_(&database_)
        , data_mask_(eprosima::statistics_backend::DataKindMask::all())
        , reader_listener_(&data_queue_, &monitor_service_data_queue_)
    {
    }

    void add_sample_to_reader_history(
            std::shared_ptr<StatisticsData> data,
            std::shared_ptr<SampleInfo> info)
    {
        datareader_.add_sample(data, info);
    }

    void add_monitor_sample_to_reader_history(
            std::shared_ptr<MonitorServiceStatusData> data,
            std::shared_ptr<SampleInfo> info)
    {
        datareader_.add_monitor_sample(data, info);
    }

    std::shared_ptr<SampleInfo> get_default_info()
    {
        std::shared_ptr<SampleInfo> info = std::make_shared<SampleInfo>();

        info->sample_state = NOT_READ_SAMPLE_STATE;
        info->view_state = NOT_NEW_VIEW_STATE;
        info->disposed_generation_count = 0;
        info->no_writers_generation_count = 1;
        info->sample_rank = 0;
        info->generation_rank = 0;
        info->absolute_generation_rank = 0;
        info->valid_data = true;
        info->instance_state = ALIVE_INSTANCE_STATE;

        return info;
    }

};

TEST_F(statistics_reader_listener_tests, not_valid_data)
{
    // Simulate a control message
    std::shared_ptr<SampleInfo> info = get_default_info();
    info->valid_data = false;

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Insert the data on the queue and wait until processed
    {
        // Initialize data object with proper type
        std::shared_ptr<StatisticsData> data = std::make_shared<StatisticsData>();
        data->writer_reader_data(WriterReaderData());

        add_sample_to_reader_history(data, info);
        datareader_.set_topic_name(HISTORY_LATENCY_TOPIC);
        reader_listener_.on_data_available(&datareader_);
        data_queue_.flush();
    }

    {
        // Initialize data object with proper type
        std::shared_ptr<StatisticsData> data = std::make_shared<StatisticsData>();
        data->locator2locator_data(Locator2LocatorData());

        // Insert the data on the queue and wait until processed
        add_sample_to_reader_history(data, info);
        datareader_.set_topic_name(NETWORK_LATENCY_TOPIC);
        reader_listener_.on_data_available(&datareader_);
        data_queue_.flush();
    }

    {
        // Initialize data object with proper type
        std::shared_ptr<StatisticsData> data = std::make_shared<StatisticsData>();
        data->entity_data(EntityData());

        // Insert the data on the queue and wait until processed
        add_sample_to_reader_history(data, info);
        datareader_.set_topic_name(PUBLICATION_THROUGHPUT_TOPIC);
        reader_listener_.on_data_available(&datareader_);
        data_queue_.flush();

        // Insert the data on the queue and wait until processed
        add_sample_to_reader_history(data, info);
        datareader_.set_topic_name(SUBSCRIPTION_THROUGHPUT_TOPIC);
        reader_listener_.on_data_available(&datareader_);
        data_queue_.flush();
    }

    {
        // Initialize data object with proper type
        std::shared_ptr<StatisticsData> data = std::make_shared<StatisticsData>();
        data->entity2locator_traffic(Entity2LocatorTraffic());

        // Insert the data on the queue and wait until processed
        add_sample_to_reader_history(data, info);
        datareader_.set_topic_name(RTPS_SENT_TOPIC);
        reader_listener_.on_data_available(&datareader_);
        data_queue_.flush();

        // Insert the data on the queue and wait until processed
        add_sample_to_reader_history(data, info);
        datareader_.set_topic_name(RTPS_LOST_TOPIC);
        reader_listener_.on_data_available(&datareader_);
        data_queue_.flush();
    }

    {
        // Initialize data object with proper type
        std::shared_ptr<StatisticsData> data = std::make_shared<StatisticsData>();
        data->entity_count(EntityCount());

        // Insert the data on the queue and wait until processed
        add_sample_to_reader_history(data, info);
        datareader_.set_topic_name(RESENT_DATAS_TOPIC);
        reader_listener_.on_data_available(&datareader_);
        data_queue_.flush();

        // Insert the data on the queue and wait until processed
        add_sample_to_reader_history(data, info);
        datareader_.set_topic_name(HEARTBEAT_COUNT_TOPIC);
        reader_listener_.on_data_available(&datareader_);
        data_queue_.flush();

        // Insert the data on the queue and wait until processed
        add_sample_to_reader_history(data, info);
        datareader_.set_topic_name(ACKNACK_COUNT_TOPIC);
        reader_listener_.on_data_available(&datareader_);
        data_queue_.flush();

        // Insert the data on the queue and wait until processed
        add_sample_to_reader_history(data, info);
        datareader_.set_topic_name(NACKFRAG_COUNT_TOPIC);
        reader_listener_.on_data_available(&datareader_);
        data_queue_.flush();

        // Insert the data on the queue and wait until processed
        add_sample_to_reader_history(data, info);
        datareader_.set_topic_name(GAP_COUNT_TOPIC);
        reader_listener_.on_data_available(&datareader_);
        data_queue_.flush();

        // Insert the data on the queue and wait until processed
        add_sample_to_reader_history(data, info);
        datareader_.set_topic_name(DATA_COUNT_TOPIC);
        reader_listener_.on_data_available(&datareader_);
        data_queue_.flush();

        // Insert the data on the queue and wait until processed
        add_sample_to_reader_history(data, info);
        datareader_.set_topic_name(PDP_PACKETS_TOPIC);
        reader_listener_.on_data_available(&datareader_);
        data_queue_.flush();

        // Insert the data on the queue and wait until processed
        add_sample_to_reader_history(data, info);
        datareader_.set_topic_name(EDP_PACKETS_TOPIC);
        reader_listener_.on_data_available(&datareader_);
        data_queue_.flush();
    }

    {
        // Initialize data object with proper type
        std::shared_ptr<StatisticsData> data = std::make_shared<StatisticsData>();
        data->discovery_time(DiscoveryTime());

        // Insert the data on the queue and wait until processed
        add_sample_to_reader_history(data, info);
        datareader_.set_topic_name(DISCOVERY_TOPIC);
        reader_listener_.on_data_available(&datareader_);
        data_queue_.flush();
    }

    {
        // Initialize data object with proper type
        std::shared_ptr<StatisticsData> data = std::make_shared<StatisticsData>();
        data->sample_identity_count(SampleIdentityCount());

        // Insert the data on the queue and wait until processed
        add_sample_to_reader_history(data, info);
        datareader_.set_topic_name(SAMPLE_DATAS_TOPIC);
        reader_listener_.on_data_available(&datareader_);
        data_queue_.flush();
    }

    {
        // Initialize data object with proper type
        std::shared_ptr<StatisticsData> data = std::make_shared<StatisticsData>();
        data->physical_data(PhysicalData());

        // Insert the data on the queue and wait until processed
        add_sample_to_reader_history(data, info);
        datareader_.set_topic_name(PHYSICAL_DATA_TOPIC);
        reader_listener_.on_data_available(&datareader_);
        data_queue_.flush();
    }

    // Try again now with an empty queue
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_history_latency_received)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the reader GUID
    DatabaseDataQueue<StatisticsData>::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue<StatisticsData>::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue<StatisticsData>::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the writer GUID
    DatabaseDataQueue<StatisticsData>::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue<StatisticsData>::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue<StatisticsData>::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue<StatisticsData>::StatisticsWriterReaderData inner_data;
    inner_data.data(1.0);
    inner_data.writer_guid(writer_guid);
    inner_data.reader_guid(reader_guid);

    std::shared_ptr<StatisticsData> data = std::make_shared<StatisticsData>();
    data->writer_reader_data(inner_data);
    data->_d(EventKind::HISTORY2HISTORY_LATENCY);

    add_sample_to_reader_history(data, get_default_info());

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The reader exists and has ID 2
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(2))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::FASTDDS_LATENCY);
                EXPECT_EQ(dynamic_cast<const HistoryLatencySample&>(sample).reader, 2);
                EXPECT_EQ(dynamic_cast<const HistoryLatencySample&>(sample).data, 1.0);
            });

    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    datareader_.set_topic_name(HISTORY_LATENCY_TOPIC);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();

    // Expectation: The insert method is not called if there is no data in the queue
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_network_latency_received)
{
    std::array<uint8_t, 16> src_locator_address = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    eprosima::fastdds::rtps::Locator_t src_locator_t;
    uint16_t src_locator_t_physical_port = 0;
    uint16_t src_locator_t_logical_port = 0;
    eprosima::fastdds::rtps::IPLocator::setPhysicalPort(src_locator_t, src_locator_t_physical_port);
    eprosima::fastdds::rtps::IPLocator::setLogicalPort(src_locator_t, src_locator_t_logical_port);
    uint32_t src_locator_port = src_locator_t.port;
    std::string src_locator_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|d.e.f.10";

    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    eprosima::fastdds::rtps::Locator_t dst_locator_t;
    uint16_t dst_locator_t_physical_port = 2048;
    uint16_t dst_locator_t_logical_port = 0;
    eprosima::fastdds::rtps::IPLocator::setPhysicalPort(dst_locator_t, dst_locator_t_physical_port);
    eprosima::fastdds::rtps::IPLocator::setLogicalPort(dst_locator_t, dst_locator_t_logical_port);
    uint32_t dst_locator_port = dst_locator_t.port;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:" + std::to_string(dst_locator_t_physical_port) + "-" +
            std::to_string(dst_locator_t_logical_port);

    // Build the source locator
    DatabaseDataQueue<StatisticsData>::StatisticsLocator src_locator;
    src_locator.kind(LOCATOR_KIND_TCPv4);
    src_locator.port(src_locator_port);
    src_locator.address(src_locator_address);

    // Build the destination locator
    DatabaseDataQueue<StatisticsData>::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue<StatisticsData>::StatisticsLocator2LocatorData inner_data;
    inner_data.data(1.0);
    inner_data.src_locator(src_locator);
    inner_data.dst_locator(dst_locator);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->locator2locator_data(inner_data);
    data->_d(EventKind::NETWORK_LATENCY);

    add_sample_to_reader_history(data, get_default_info());

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::PARTICIPANT, src_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::pair<EntityId, EntityId>(std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The destination locator exists and has ID 2
    EXPECT_CALL(database_, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::NETWORK_LATENCY);
                EXPECT_EQ(dynamic_cast<const NetworkLatencySample&>(sample).remote_locator, 2);
            });

    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    datareader_.set_topic_name(NETWORK_LATENCY_TOPIC);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();

    // Expectation: The insert method is not called if there is no data in the queue
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_publication_throughput_received)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue<StatisticsData>::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue<StatisticsData>::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue<StatisticsData>::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue<StatisticsData>::StatisticsEntityData inner_data;
    inner_data.data(1.0);
    inner_data.guid(writer_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_data(inner_data);
    data->_d(EventKind::PUBLICATION_THROUGHPUT);

    add_sample_to_reader_history(data, get_default_info());

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::PUBLICATION_THROUGHPUT);
                EXPECT_EQ(dynamic_cast<const PublicationThroughputSample&>(sample).data, 1.0);
            });

    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    datareader_.set_topic_name(PUBLICATION_THROUGHPUT_TOPIC);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();

    // Expectation: The insert method is not called if there is no data in the queue
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_subscription_throughput_received)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueue<StatisticsData>::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue<StatisticsData>::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue<StatisticsData>::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueue<StatisticsData>::StatisticsEntityData inner_data;
    inner_data.data(1.0);
    inner_data.guid(reader_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_data(inner_data);
    data->_d(EventKind::SUBSCRIPTION_THROUGHPUT);

    add_sample_to_reader_history(data, get_default_info());

    // Precondition: The reader exists and has ID 1
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::SUBSCRIPTION_THROUGHPUT);
                EXPECT_EQ(dynamic_cast<const SubscriptionThroughputSample&>(sample).data, 1.0);
            });

    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    datareader_.set_topic_name(SUBSCRIPTION_THROUGHPUT_TOPIC);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();

    // Expectation: The insert method is not called if there is no data in the queue
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_rtps_sent_received)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    eprosima::fastdds::rtps::Locator_t dst_locator_t;
    uint16_t dst_locator_t_physical_port = 2048;
    uint16_t dst_locator_t_logical_port = 0;
    eprosima::fastdds::rtps::IPLocator::setPhysicalPort(dst_locator_t, dst_locator_t_physical_port);
    eprosima::fastdds::rtps::IPLocator::setLogicalPort(dst_locator_t, dst_locator_t_logical_port);
    uint32_t dst_locator_port = dst_locator_t.port;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:" + std::to_string(dst_locator_t_physical_port) + "-" +
            std::to_string(dst_locator_t_logical_port);

    // Build the writer GUID
    DatabaseDataQueue<StatisticsData>::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue<StatisticsData>::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue<StatisticsData>::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueue<StatisticsData>::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue<StatisticsData>::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKind::RTPS_SENT);

    add_sample_to_reader_history(data, get_default_info());

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The locator exists and has ID 2
    EXPECT_CALL(database_, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args1([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::RTPS_PACKETS_SENT);
                EXPECT_EQ(dynamic_cast<const RtpsPacketsSentSample&>(sample).count, 1024u);
                EXPECT_EQ(dynamic_cast<const RtpsPacketsSentSample&>(sample).remote_locator, 2);
            });

    InsertDataArgs args2([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::RTPS_BYTES_SENT);
                EXPECT_EQ(dynamic_cast<const RtpsBytesSentSample&>(sample).count, 2048u);
                EXPECT_EQ(dynamic_cast<const RtpsBytesSentSample&>(sample).magnitude_order, 10);
                EXPECT_EQ(dynamic_cast<const RtpsBytesSentSample&>(sample).remote_locator, 2);
            });

    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(2)
            .WillOnce(Invoke(&args1, &InsertDataArgs::insert))
            .WillOnce(Invoke(&args2, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    datareader_.set_topic_name(RTPS_SENT_TOPIC);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();

    // Expectation: The insert method is not called if there is no data in the queue
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_rtps_lost_received)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    eprosima::fastdds::rtps::Locator_t dst_locator_t;
    uint16_t dst_locator_t_physical_port = 2048;
    uint16_t dst_locator_t_logical_port = 0;
    eprosima::fastdds::rtps::IPLocator::setPhysicalPort(dst_locator_t, dst_locator_t_physical_port);
    eprosima::fastdds::rtps::IPLocator::setLogicalPort(dst_locator_t, dst_locator_t_logical_port);
    uint32_t dst_locator_port = dst_locator_t.port;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:" + std::to_string(dst_locator_t_physical_port) + "-" +
            std::to_string(dst_locator_t_logical_port);

    // Build the writer GUID
    DatabaseDataQueue<StatisticsData>::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue<StatisticsData>::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue<StatisticsData>::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueue<StatisticsData>::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue<StatisticsData>::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKind::RTPS_LOST);

    add_sample_to_reader_history(data, get_default_info());

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The locator exists and has ID 2
    EXPECT_CALL(database_, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args1([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::RTPS_PACKETS_LOST);
                EXPECT_EQ(dynamic_cast<const RtpsPacketsLostSample&>(sample).count, 1024u);
                EXPECT_EQ(dynamic_cast<const RtpsPacketsLostSample&>(sample).remote_locator, 2);
            });

    InsertDataArgs args2([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::RTPS_BYTES_LOST);
                EXPECT_EQ(dynamic_cast<const RtpsBytesLostSample&>(sample).count, 2048u);
                EXPECT_EQ(dynamic_cast<const RtpsBytesLostSample&>(sample).magnitude_order, 10);
                EXPECT_EQ(dynamic_cast<const RtpsBytesLostSample&>(sample).remote_locator, 2);
            });

    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(2)
            .WillOnce(Invoke(&args1, &InsertDataArgs::insert))
            .WillOnce(Invoke(&args2, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    datareader_.set_topic_name(RTPS_LOST_TOPIC);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();

    // Expectation: The insert method is not called if there is no data in the queue
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_resent_datas_received)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue<StatisticsData>::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue<StatisticsData>::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue<StatisticsData>::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue<StatisticsData>::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::RESENT_DATAS);

    add_sample_to_reader_history(data, get_default_info());

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::RESENT_DATA);
                EXPECT_EQ(dynamic_cast<const ResentDataSample&>(sample).count, 1024u);
            });

    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    datareader_.set_topic_name(RESENT_DATAS_TOPIC);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();

    // Expectation: The insert method is not called if there is no data in the queue
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_heartbeat_count_received)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue<StatisticsData>::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue<StatisticsData>::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue<StatisticsData>::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue<StatisticsData>::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::HEARTBEAT_COUNT);

    add_sample_to_reader_history(data, get_default_info());

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::HEARTBEAT_COUNT);
                EXPECT_EQ(dynamic_cast<const HeartbeatCountSample&>(sample).count, 1024u);
            });

    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    datareader_.set_topic_name(HEARTBEAT_COUNT_TOPIC);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();

    // Expectation: The insert method is not called if there is no data in the queue
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_acknack_count_received)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueue<StatisticsData>::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue<StatisticsData>::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue<StatisticsData>::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueue<StatisticsData>::StatisticsEntityCount inner_data;
    inner_data.guid(reader_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::ACKNACK_COUNT);

    add_sample_to_reader_history(data, get_default_info());

    // Precondition: The reader exists and has ID 1
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::ACKNACK_COUNT);
                EXPECT_EQ(dynamic_cast<const AcknackCountSample&>(sample).count, 1024u);
            });

    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    datareader_.set_topic_name(ACKNACK_COUNT_TOPIC);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();

    // Expectation: The insert method is not called if there is no data in the queue
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_nackfrag_count_received)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueue<StatisticsData>::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue<StatisticsData>::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue<StatisticsData>::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueue<StatisticsData>::StatisticsEntityCount inner_data;
    inner_data.guid(reader_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::NACKFRAG_COUNT);

    add_sample_to_reader_history(data, get_default_info());

    // Precondition: The reader exists and has ID 1
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::NACKFRAG_COUNT);
                EXPECT_EQ(dynamic_cast<const NackfragCountSample&>(sample).count, 1024u);
            });

    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    datareader_.set_topic_name(NACKFRAG_COUNT_TOPIC);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();

    // Expectation: The insert method is not called if there is no data in the queue
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_gap_count_received)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue<StatisticsData>::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue<StatisticsData>::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue<StatisticsData>::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue<StatisticsData>::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::GAP_COUNT);

    add_sample_to_reader_history(data, get_default_info());

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::GAP_COUNT);
                EXPECT_EQ(dynamic_cast<const GapCountSample&>(sample).count, 1024u);
            });

    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    datareader_.set_topic_name(GAP_COUNT_TOPIC);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();

    // Expectation: The insert method is not called if there is no data in the queue
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_data_count_received)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue<StatisticsData>::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue<StatisticsData>::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue<StatisticsData>::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue<StatisticsData>::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::DATA_COUNT);

    add_sample_to_reader_history(data, get_default_info());

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::DATA_COUNT);
                EXPECT_EQ(dynamic_cast<const DataCountSample&>(sample).count, 1024u);
            });

    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    datareader_.set_topic_name(DATA_COUNT_TOPIC);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();

    // Expectation: The insert method is not called if there is no data in the queue
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_pdp_count_received)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    DatabaseDataQueue<StatisticsData>::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue<StatisticsData>::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue<StatisticsData>::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the Statistics data
    DatabaseDataQueue<StatisticsData>::StatisticsEntityCount inner_data;
    inner_data.guid(participant_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::PDP_PACKETS);

    add_sample_to_reader_history(data, get_default_info());

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::PDP_PACKETS);
                EXPECT_EQ(dynamic_cast<const PdpCountSample&>(sample).count, 1024u);
            });

    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    datareader_.set_topic_name(PDP_PACKETS_TOPIC);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();

    // Expectation: The insert method is not called if there is no data in the queue
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_edp_count_received)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    DatabaseDataQueue<StatisticsData>::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue<StatisticsData>::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue<StatisticsData>::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the Statistics data
    DatabaseDataQueue<StatisticsData>::StatisticsEntityCount inner_data;
    inner_data.guid(participant_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::EDP_PACKETS);

    add_sample_to_reader_history(data, get_default_info());

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::EDP_PACKETS);
                EXPECT_EQ(dynamic_cast<const EdpCountSample&>(sample).count, 1024u);
            });

    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    datareader_.set_topic_name(EDP_PACKETS_TOPIC);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();

    // Expectation: The insert method is not called if there is no data in the queue
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_discovery_times_received)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::array<uint8_t, 4> entity_id = {0, 0, 0, 1};
    uint64_t discovery_time = 1024;
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";
    std::string remote_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    std::chrono::system_clock::time_point discovery_timestamp
        = eprosima::statistics_backend::nanoseconds_to_systemclock(discovery_time);

    // Build the participant GUID
    DatabaseDataQueue<StatisticsData>::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue<StatisticsData>::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue<StatisticsData>::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the remote GUID
    DatabaseDataQueue<StatisticsData>::StatisticsGuidPrefix remote_prefix;
    remote_prefix.value(prefix);
    DatabaseDataQueue<StatisticsData>::StatisticsEntityId remote_entity_id;
    remote_entity_id.value(entity_id);
    DatabaseDataQueue<StatisticsData>::StatisticsGuid remote_guid;
    remote_guid.guidPrefix(remote_prefix);
    remote_guid.entityId(remote_entity_id);

    // Build the Statistics data
    DatabaseDataQueue<StatisticsData>::StatisticsDiscoveryTime inner_data;
    inner_data.local_participant_guid(participant_guid);
    inner_data.remote_entity_guid(remote_guid);
    inner_data.time(discovery_time);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->discovery_time(inner_data);
    data->_d(EventKind::DISCOVERED_ENTITY);

    add_sample_to_reader_history(data, get_default_info());

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The remote entity exists and has ID 2
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::PARTICIPANT, remote_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(2))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::DISCOVERY_TIME);
                EXPECT_EQ(dynamic_cast<const DiscoveryTimeSample&>(sample).remote_entity, 2);
                EXPECT_EQ(dynamic_cast<const DiscoveryTimeSample&>(sample).time, discovery_timestamp);
            });

    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    datareader_.set_topic_name(DISCOVERY_TOPIC);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();

    // Expectation: The insert method is not called if there is no data in the queue
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_sample_datas_received)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    int32_t sn_high = 2048;
    uint32_t sn_low = 4096;
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    eprosima::fastdds::rtps::SequenceNumber_t sn (sn_high, sn_low);

    // Build the writer GUID
    DatabaseDataQueue<StatisticsData>::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue<StatisticsData>::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue<StatisticsData>::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    DatabaseDataQueue<StatisticsData>::StatisticsSequenceNumber sequence_number;
    sequence_number.high(sn_high);
    sequence_number.low(sn_low);

    DatabaseDataQueue<StatisticsData>::StatisticsSampleIdentity sample_identity;
    sample_identity.writer_guid(writer_guid);
    sample_identity.sequence_number(sequence_number);

    // Build the Statistics data
    DatabaseDataQueue<StatisticsData>::StatisticsSampleIdentityCount inner_data;
    inner_data.count(1024);
    inner_data.sample_id(sample_identity);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->sample_identity_count(inner_data);
    data->_d(EventKind::SAMPLE_DATAS);

    add_sample_to_reader_history(data, get_default_info());

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::SAMPLE_DATAS);
                EXPECT_EQ(dynamic_cast<const SampleDatasCountSample&>(sample).count, 1024u);
                EXPECT_EQ(dynamic_cast<const SampleDatasCountSample&>(sample).sequence_number, sn.to64long());
            });
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    datareader_.set_topic_name(SAMPLE_DATAS_TOPIC);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();

    // Expectation: The insert method is not called if there is no data in the queue
    EXPECT_CALL(database_, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_monitor_service_sample_received)
{
    // Build the writer GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    int32_t sn_high = 2048;
    uint32_t sn_low = 4096;
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    eprosima::fastdds::rtps::SequenceNumber_t sn (sn_high, sn_low);

    // Build the writer GUID
    DatabaseDataQueue<MonitorServiceData>::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue<MonitorServiceData>::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue<MonitorServiceData>::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);
    StatusKind::StatusKind kind = StatusKind::PROXY;
    MonitorServiceData value;
    std::vector<uint8_t> entity_proxy = {1, 2, 3, 4, 5};
    value.entity_proxy(entity_proxy);

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    data->local_entity(writer_guid);
    data->status_kind(kind);
    data->value(value);

    add_monitor_sample_to_reader_history(data, get_default_info());

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database_, get_entity_kind_by_guid(writer_guid)).Times(1)
            .WillOnce(Return(EntityKind::DATAWRITER));

    // Expectation: The insert method is called with appropriate arguments
    InsertMonitorServiceDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const eprosima::statistics_backend::MonitorServiceSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, eprosima::statistics_backend::StatusKind::PROXY);
                EXPECT_EQ(sample.status, eprosima::statistics_backend::StatusLevel::OK_STATUS);
                EXPECT_EQ(dynamic_cast<const eprosima::statistics_backend::ProxySample&>(sample).entity_proxy,
                entity_proxy);

                return false;
            });
    EXPECT_CALL(database_,
            insert(_, _, testing::Matcher<const eprosima::statistics_backend::MonitorServiceSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertMonitorServiceDataArgs::insert));

    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_status_reported(EntityId(0), EntityId(1), eprosima::statistics_backend::StatusKind::PROXY)).Times(1);

    // Insert the data on the queue and wait until processed
    datareader_.set_topic_name(MONITOR_SERVICE_TOPIC);
    reader_listener_.on_data_available(&datareader_);
    monitor_service_data_queue_.flush();

    // Expectation: The insert method is not called if there is no data in the queue
    EXPECT_CALL(database_,
            insert(_, _, testing::Matcher<const eprosima::statistics_backend::MonitorServiceSample&>(_))).Times(0);
    reader_listener_.on_data_available(&datareader_);
    monitor_service_data_queue_.flush();
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
