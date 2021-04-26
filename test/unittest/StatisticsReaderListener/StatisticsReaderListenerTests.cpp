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

#include <database/database.hpp>
#include <database/database_queue.hpp>
#include <subscriber/StatisticsReaderListener.hpp>
#include <topic_types/types.h>

#include "gtest/gtest.h"
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

class statistics_reader_listener_tests : public ::testing::Test
{

public:

    Database database_;
    DatabaseDataQueue data_queue_;
    eprosima::statistics_backend::DataKindMask data_mask_;
    StatisticsReaderListener reader_listener_;
    eprosima::fastdds::dds::DataReader datareader_;

    statistics_reader_listener_tests()
        : database_()
        , data_queue_(&database_)
        , data_mask_(eprosima::statistics_backend::DataKindMask::all())
        , reader_listener_(data_mask_, &data_queue_)
    {
    }

    void add_sample_to_reader_history(
            std::shared_ptr<StatisticsData> data)
    {
        std::shared_ptr<SampleInfo> info = get_default_info();
        datareader_.add_sample(data, info);
    }

    void set_data_mask_bit (
            eprosima::statistics_backend::DataKind bit)
    {
        data_mask_.set(bit);
        reader_listener_.set_mask(data_mask_);
    }

    void clear_data_mask_bit (
            eprosima::statistics_backend::DataKind bit)
    {
        data_mask_.clear(bit);
        reader_listener_.set_mask(data_mask_);
    }

protected:

    std::shared_ptr<SampleInfo> get_default_info()
    {
        std::shared_ptr<SampleInfo> info = std::make_shared<SampleInfo>();

        info->sample_state = NOT_READ_SAMPLE_STATE;
        info->view_state = NOT_NEW_VIEW_STATE;
        info->disposed_generation_count = 0;
        info->no_writers_generation_count = 1;
        info->sample_rank = 0;
        info->generation_rank = 0;
        info->absoulte_generation_rank = 0;
        //        info->source_timestamp = rtps_info->sourceTimestamp;
        //        info->reception_timestamp = rtps_info->receptionTimestamp;
        //        info->instance_handle = rtps_info->iHandle;
        //        info->publication_handle = fastrtps::rtps::InstanceHandle_t(rtps_info->sample_identity.writer_guid());
        //        info->sample_identity = rtps_info->sample_identity;
        //        info->related_sample_identity = rtps_info->related_sample_identity;
        info->valid_data = true;
        info->instance_state = ALIVE_INSTANCE_STATE;

        return info;
    }

};

TEST_F(statistics_reader_listener_tests, new_history_latency_received)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string reader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the reader GUID
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsWriterReaderData inner_data;
    inner_data.data(1.0);
    inner_data.writer_guid(writer_guid);
    inner_data.reader_guid(reader_guid);

    std::shared_ptr<StatisticsData> data = std::make_shared<StatisticsData>();
    data->writer_reader_data(inner_data);
    data->_d(EventKind::HISTORY2HISTORY_LATENCY);

    add_sample_to_reader_history(data);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The reader exists and has ID 2
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
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
                EXPECT_EQ(sample.kind, DataKind::FASTDDS_LATENCY);
                EXPECT_EQ(dynamic_cast<const HistoryLatencySample&>(sample).reader, 2);
                EXPECT_EQ(dynamic_cast<const HistoryLatencySample&>(sample).data, 1.0);
            });

    EXPECT_CALL(database_, insert(_, _, _)).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_history_latency_received_not_in_mask)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string reader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the reader GUID
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsWriterReaderData inner_data;
    inner_data.data(1.0);
    inner_data.writer_guid(writer_guid);
    inner_data.reader_guid(reader_guid);

    std::shared_ptr<StatisticsData> data = std::make_shared<StatisticsData>();
    data->writer_reader_data(inner_data);
    data->_d(EventKind::HISTORY2HISTORY_LATENCY);

    add_sample_to_reader_history(data);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The reader exists and has ID 2
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Precondition: The event is filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::FASTDDS_LATENCY);

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, _)).Times(0);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_network_latency_received)
{
    std::array<uint8_t, 16> src_locator_address = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    uint32_t src_locator_port = 1024;
    std::string src_locator_str = "TCPv4:[13.14.15.16]:1024";
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the source locator
    DatabaseDataQueue::StatisticsLocator src_locator;
    src_locator.kind(LOCATOR_KIND_TCPv4);
    src_locator.port(src_locator_port);
    src_locator.address(src_locator_address);

    // Build the destination locator
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsLocator2LocatorData inner_data;
    inner_data.data(1.0);
    inner_data.src_locator(src_locator);
    inner_data.dst_locator(dst_locator);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->locator2locator_data(inner_data);
    data->_d(EventKind::NETWORK_LATENCY);

    add_sample_to_reader_history(data);

    // Precondition: The source locator exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_name(EntityKind::LOCATOR, src_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

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

    EXPECT_CALL(database_, insert(_, _, _)).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_network_latency_received_not_in_mask)
{
    std::array<uint8_t, 16> src_locator_address = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    uint32_t src_locator_port = 1024;
    std::string src_locator_str = "TCPv4:[13.14.15.16]:1024";
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the source locator
    DatabaseDataQueue::StatisticsLocator src_locator;
    src_locator.kind(LOCATOR_KIND_TCPv4);
    src_locator.port(src_locator_port);
    src_locator.address(src_locator_address);

    // Build the destination locator
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsLocator2LocatorData inner_data;
    inner_data.data(1.0);
    inner_data.src_locator(src_locator);
    inner_data.dst_locator(dst_locator);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->locator2locator_data(inner_data);
    data->_d(EventKind::NETWORK_LATENCY);

    add_sample_to_reader_history(data);

    // Precondition: The source locator exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_name(EntityKind::LOCATOR, src_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The destination locator exists and has ID 2
    EXPECT_CALL(database_, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Precondition: The event is filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::NETWORK_LATENCY);

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, _)).Times(0);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_publication_throughput_received)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityData inner_data;
    inner_data.data(1.0);
    inner_data.guid(writer_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_data(inner_data);
    data->_d(EventKind::PUBLICATION_THROUGHPUT);

    add_sample_to_reader_history(data);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

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

    EXPECT_CALL(database_, insert(_, _, _)).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_publication_throughput_received_not_in_mask)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityData inner_data;
    inner_data.data(1.0);
    inner_data.guid(writer_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_data(inner_data);
    data->_d(EventKind::PUBLICATION_THROUGHPUT);

    add_sample_to_reader_history(data);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The event is filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::PUBLICATION_THROUGHPUT);

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, _)).Times(0);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_subscription_throughput_received)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityData inner_data;
    inner_data.data(1.0);
    inner_data.guid(reader_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_data(inner_data);
    data->_d(EventKind::SUBSCRIPTION_THROUGHPUT);

    add_sample_to_reader_history(data);

    // Precondition: The reader exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

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

    EXPECT_CALL(database_, insert(_, _, _)).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_subscription_throughput_received_not_in_mask)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityData inner_data;
    inner_data.data(1.0);
    inner_data.guid(reader_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_data(inner_data);
    data->_d(EventKind::SUBSCRIPTION_THROUGHPUT);

    add_sample_to_reader_history(data);

    // Precondition: The reader exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The event is filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::SUBSCRIPTION_THROUGHPUT);

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, _)).Times(0);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_rtps_sent_received)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKind::RTPS_SENT);

    add_sample_to_reader_history(data);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

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
                EXPECT_EQ(dynamic_cast<const RtpsPacketsSentSample&>(sample).count, 1024);
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
                EXPECT_EQ(dynamic_cast<const RtpsBytesSentSample&>(sample).count, 2048);
                EXPECT_EQ(dynamic_cast<const RtpsBytesSentSample&>(sample).magnitude_order, 10);
                EXPECT_EQ(dynamic_cast<const RtpsBytesSentSample&>(sample).remote_locator, 2);
            });

    EXPECT_CALL(database_, insert(_, _, _)).Times(2)
            .WillOnce(Invoke(&args1, &InsertDataArgs::insert))
            .WillOnce(Invoke(&args2, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_rtps_sent_received_not_in_mask)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKind::RTPS_SENT);

    add_sample_to_reader_history(data);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The locator exists and has ID 2
    EXPECT_CALL(database_, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Precondition: The event is filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::RTPS_PACKETS_SENT);
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::RTPS_BYTES_SENT);

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, _)).Times(0);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_rtps_lost_received)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKind::RTPS_LOST);

    add_sample_to_reader_history(data);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

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
                EXPECT_EQ(dynamic_cast<const RtpsPacketsLostSample&>(sample).count, 1024);
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
                EXPECT_EQ(dynamic_cast<const RtpsBytesLostSample&>(sample).count, 2048);
                EXPECT_EQ(dynamic_cast<const RtpsBytesLostSample&>(sample).magnitude_order, 10);
                EXPECT_EQ(dynamic_cast<const RtpsBytesLostSample&>(sample).remote_locator, 2);
            });

    EXPECT_CALL(database_, insert(_, _, _)).Times(2)
            .WillOnce(Invoke(&args1, &InsertDataArgs::insert))
            .WillOnce(Invoke(&args2, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_rtps_lost_received_not_in_mask)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKind::RTPS_LOST);

    add_sample_to_reader_history(data);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The locator exists and has ID 2
    EXPECT_CALL(database_, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Precondition: The event is filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::RTPS_PACKETS_LOST);
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::RTPS_BYTES_LOST);

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, _)).Times(0);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_resent_datas_received)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::RESENT_DATAS);

    add_sample_to_reader_history(data);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::RESENT_DATA);
                EXPECT_EQ(dynamic_cast<const ResentDataSample&>(sample).count, 1024);
            });

    EXPECT_CALL(database_, insert(_, _, _)).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_resent_datas_received_not_in_mask)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::RESENT_DATAS);

    add_sample_to_reader_history(data);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The event is filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::RESENT_DATA);

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, _)).Times(0);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_heartbeat_count_received)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::HEARTBEAT_COUNT);

    add_sample_to_reader_history(data);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::HEARTBEAT_COUNT);
                EXPECT_EQ(dynamic_cast<const HeartbeatCountSample&>(sample).count, 1024);
            });

    EXPECT_CALL(database_, insert(_, _, _)).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_heartbeat_count_received_not_in_mask)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::HEARTBEAT_COUNT);

    add_sample_to_reader_history(data);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The event is filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::HEARTBEAT_COUNT);

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, _)).Times(0);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_acknack_count_received)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(reader_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::ACKNACK_COUNT);

    add_sample_to_reader_history(data);

    // Precondition: The reader exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::ACKNACK_COUNT);
                EXPECT_EQ(dynamic_cast<const AcknackCountSample&>(sample).count, 1024);
            });

    EXPECT_CALL(database_, insert(_, _, _)).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_acknack_count_received_not_in_mask)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(reader_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::ACKNACK_COUNT);

    add_sample_to_reader_history(data);

    // Precondition: The reader exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The event is filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::ACKNACK_COUNT);

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, _)).Times(0);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_nackfrag_count_received)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(reader_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::NACKFRAG_COUNT);

    add_sample_to_reader_history(data);

    // Precondition: The reader exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::NACKFRAG_COUNT);
                EXPECT_EQ(dynamic_cast<const NackfragCountSample&>(sample).count, 1024);
            });

    EXPECT_CALL(database_, insert(_, _, _)).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_nackfrag_count_received_not_in_mask)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(reader_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::NACKFRAG_COUNT);

    add_sample_to_reader_history(data);

    // Precondition: The reader exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The event is filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::NACKFRAG_COUNT);

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, _)).Times(0);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_gap_count_received)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::GAP_COUNT);

    add_sample_to_reader_history(data);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::GAP_COUNT);
                EXPECT_EQ(dynamic_cast<const GapCountSample&>(sample).count, 1024);
            });

    EXPECT_CALL(database_, insert(_, _, _)).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_gap_count_received_not_in_mask)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::GAP_COUNT);

    add_sample_to_reader_history(data);

    // Precondition: The event is filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::GAP_COUNT);

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, _)).Times(0);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_data_count_received)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::DATA_COUNT);

    add_sample_to_reader_history(data);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::DATA_COUNT);
                EXPECT_EQ(dynamic_cast<const DataCountSample&>(sample).count, 1024);
            });

    EXPECT_CALL(database_, insert(_, _, _)).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_data_count_received_not_in_mask)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::DATA_COUNT);

    add_sample_to_reader_history(data);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The event is filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::DATA_COUNT);

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, _)).Times(0);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_pdp_count_received)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(participant_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::PDP_PACKETS);

    add_sample_to_reader_history(data);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::PDP_PACKETS);
                EXPECT_EQ(dynamic_cast<const PdpCountSample&>(sample).count, 1024);
            });

    EXPECT_CALL(database_, insert(_, _, _)).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_pdp_count_received_not_in_mask)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(participant_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::PDP_PACKETS);

    add_sample_to_reader_history(data);

    // Precondition: The event is filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::PDP_PACKETS);

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, _)).Times(0);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_edp_count_received)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(participant_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::EDP_PACKETS);

    add_sample_to_reader_history(data);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::EDP_PACKETS);
                EXPECT_EQ(dynamic_cast<const EdpCountSample&>(sample).count, 1024);
            });

    EXPECT_CALL(database_, insert(_, _, _)).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_edp_count_received_not_in_mask)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(participant_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::EDP_PACKETS);

    add_sample_to_reader_history(data);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The event is filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::EDP_PACKETS);

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, _)).Times(0);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_discovery_times_received)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::array<uint8_t, 4> entity_id = {0, 0, 0, 1};
    uint64_t discovery_time = 1024;
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";
    std::string remote_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    std::chrono::steady_clock::time_point discovery_timestamp = std::chrono::steady_clock::time_point(std::chrono::nanoseconds(
                        discovery_time));

    // Build the participant GUID
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the remote GUID
    DatabaseDataQueue::StatisticsGuidPrefix remote_prefix;
    remote_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId remote_entity_id;
    remote_entity_id.value(entity_id);
    DatabaseDataQueue::StatisticsGuid remote_guid;
    remote_guid.guidPrefix(remote_prefix);
    remote_guid.entityId(remote_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsDiscoveryTime inner_data;
    inner_data.local_participant_guid(participant_guid);
    inner_data.remote_entity_guid(remote_guid);
    inner_data.time(discovery_time);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->discovery_time(inner_data);
    data->_d(EventKind::DISCOVERED_ENTITY);

    add_sample_to_reader_history(data);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The remote entity exists and has ID 2
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::PARTICIPANT, remote_guid_str)).Times(AnyNumber())
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
                EXPECT_EQ(sample.kind, DataKind::DISCOVERY_TIME);
                EXPECT_EQ(dynamic_cast<const DiscoveryTimeSample&>(sample).remote_entity, 2);
                EXPECT_EQ(dynamic_cast<const DiscoveryTimeSample&>(sample).time, discovery_timestamp);
            });

    EXPECT_CALL(database_, insert(_, _, _)).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_discovery_times_received_not_in_mask)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::array<uint8_t, 4> entity_id = {0, 0, 0, 1};
    uint64_t discovery_time = 1024;
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";
    std::string remote_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the participant GUID
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the remote GUID
    DatabaseDataQueue::StatisticsGuidPrefix remote_prefix;
    remote_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId remote_entity_id;
    remote_entity_id.value(entity_id);
    DatabaseDataQueue::StatisticsGuid remote_guid;
    remote_guid.guidPrefix(remote_prefix);
    remote_guid.entityId(remote_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsDiscoveryTime inner_data;
    inner_data.local_participant_guid(participant_guid);
    inner_data.remote_entity_guid(remote_guid);
    inner_data.time(discovery_time);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->discovery_time(inner_data);
    data->_d(EventKind::DISCOVERED_ENTITY);

    add_sample_to_reader_history(data);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The remote entity exists and has ID 2
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::PARTICIPANT, remote_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Precondition: The event is filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::DISCOVERY_TIME);

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, _)).Times(0);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_sample_datas_received)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    int32_t sn_high = 2048;
    uint32_t sn_low = 4096;
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    eprosima::fastrtps::rtps::SequenceNumber_t sn (sn_high, sn_low);

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    DatabaseDataQueue::StatisticsSequenceNumber sequence_number;
    sequence_number.high(sn_high);
    sequence_number.low(sn_low);

    DatabaseDataQueue::StatisticsSampleIdentity sample_identity;
    sample_identity.writer_guid(writer_guid);
    sample_identity.sequence_number(sequence_number);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsSampleIdentityCount inner_data;
    inner_data.count(1024);
    inner_data.sample_id(sample_identity);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->sample_identity_count(inner_data);
    data->_d(EventKind::SAMPLE_DATAS);

    add_sample_to_reader_history(data);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::SAMPLE_DATAS);
                EXPECT_EQ(dynamic_cast<const SampleDatasCountSample&>(sample).count, 1024);
                EXPECT_EQ(dynamic_cast<const SampleDatasCountSample&>(sample).sequence_number, sn.to64long());
            });
    EXPECT_CALL(database_, insert(_, _, _)).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertDataArgs::insert));

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_sample_datas_received_not_in_mask)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    int32_t sn_high = 2048;
    uint32_t sn_low = 4096;
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    eprosima::fastrtps::rtps::SequenceNumber_t sn (sn_high, sn_low);

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    DatabaseDataQueue::StatisticsSequenceNumber sequence_number;
    sequence_number.high(sn_high);
    sequence_number.low(sn_low);

    DatabaseDataQueue::StatisticsSampleIdentity sample_identity;
    sample_identity.writer_guid(writer_guid);
    sample_identity.sequence_number(sequence_number);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsSampleIdentityCount inner_data;
    inner_data.count(1024);
    inner_data.sample_id(sample_identity);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->sample_identity_count(inner_data);
    data->_d(EventKind::SAMPLE_DATAS);

    add_sample_to_reader_history(data);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The event is filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::SAMPLE_DATAS);

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, _)).Times(0);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_physical_data_received)
{
    std::string processname = "command";
    std::string pid = "1234";
    std::string username = "user";
    std::string hostname = "host";
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the process name
    std::stringstream ss;
    ss << processname << ":" << pid;
    std::string processname_pid = ss.str();

    // Build the Statistics data
    DatabaseDataQueue::StatisticsPhysicalData inner_data;
    inner_data.host(hostname);
    inner_data.user(username);
    inner_data.process(processname_pid);
    inner_data.participant_guid(participant_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->physical_data(inner_data);
    data->_d(EventKind::PHYSICAL_DATA);

    add_sample_to_reader_history(data);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The host exists and has ID 2
    EXPECT_CALL(database_, get_entities_by_name(EntityKind::HOST, hostname)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    auto host = std::make_shared<Host>(hostname);
    host->id = EntityId(2);
    EXPECT_CALL(database_, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(host));

    // Precondition: The user exists and has ID 3
    EXPECT_CALL(database_, get_entities_by_name(EntityKind::USER, username)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(3)))));

    auto user = std::make_shared<User>(username, host);
    user->id = EntityId(3);
    EXPECT_CALL(database_, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(user));

    // Precondition: The process exists and has ID 4
    EXPECT_CALL(database_, get_entities_by_name(EntityKind::PROCESS, processname)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(4)))));

    auto process = std::make_shared<Process>(processname, pid, user);
    process->id = EntityId(4);
    EXPECT_CALL(database_, get_entity(EntityId(4))).Times(AnyNumber())
            .WillRepeatedly(Return(process));

    // Expectation: The link method is called with appropriate arguments
    EXPECT_CALL(database_, link_participant_with_process(EntityId(1), EntityId(4))).Times(1);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

TEST_F(statistics_reader_listener_tests, new_physical_data_received_no_mask)
{
    std::string processname = "command";
    std::string pid = "1234";
    std::string username = "user";
    std::string hostname = "host";
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the process name
    std::stringstream ss;
    ss << processname << ":" << pid;
    std::string processname_pid = ss.str();

    // Build the Statistics data
    DatabaseDataQueue::StatisticsPhysicalData inner_data;
    inner_data.host(hostname);
    inner_data.user(username);
    inner_data.process(processname_pid);
    inner_data.participant_guid(participant_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->physical_data(inner_data);
    data->_d(EventKind::PHYSICAL_DATA);

    add_sample_to_reader_history(data);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The host exists and has ID 2
    EXPECT_CALL(database_, get_entities_by_name(EntityKind::HOST, hostname)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    auto host = std::make_shared<Host>(hostname);
    host->id = EntityId(2);
    EXPECT_CALL(database_, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(host));

    // Precondition: The user exists and has ID 3
    EXPECT_CALL(database_, get_entities_by_name(EntityKind::USER, username)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(3)))));

    auto user = std::make_shared<User>(username, host);
    user->id = EntityId(3);
    EXPECT_CALL(database_, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(user));

    // Precondition: The process exists and has ID 4
    EXPECT_CALL(database_, get_entities_by_name(EntityKind::PROCESS, processname)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(4)))));

    auto process = std::make_shared<Process>(processname, pid, user);
    process->id = EntityId(4);
    EXPECT_CALL(database_, get_entity(EntityId(4))).Times(AnyNumber())
            .WillRepeatedly(Return(process));

    // Precondition: All events are filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::FASTDDS_LATENCY);

    // Expectation: The link method is called with appropriate arguments
    EXPECT_CALL(database_, link_participant_with_process(EntityId(1), EntityId(4))).Times(1);

    // Insert the data on the queue and wait until processed
    reader_listener_.on_data_available(&datareader_);
    data_queue_.flush();
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
