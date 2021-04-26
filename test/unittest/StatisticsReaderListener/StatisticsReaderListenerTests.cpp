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
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The reader exists and has ID 2
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(2)))));

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
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

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
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The reader exists and has ID 2
    EXPECT_CALL(database_, get_entities_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(2)))));

    // Precondition: The event is filtered out
    clear_data_mask_bit(eprosima::statistics_backend::DataKind::FASTDDS_LATENCY);

    // Expectation: The insert method is never called
    EXPECT_CALL(database_, insert(_, _, _)).Times(0);

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
