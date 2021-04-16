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

#include <database/database.hpp>
#include <database/database_queue.hpp>
#include <topic_types/types.h>

#include "gtest/gtest.h"
#include <gmock/gmock.h>

#include <iostream>

using namespace eprosima::statistics_backend::database;
using EntityId = eprosima::statistics_backend::EntityId;
using DataKind = eprosima::statistics_backend::DataKind;
using EntityKind = eprosima::statistics_backend::EntityKind;
using Timestamp = eprosima::statistics_backend::Timestamp;
using StatisticsData = eprosima::fastdds::statistics::Data;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::AnyNumber;

using namespace eprosima::fastdds::statistics;
using namespace eprosima::statistics_backend::database;

// Wrapper class to expose the internal attributes of the queue
class DatabaseEntityQueueWrapper : public DatabaseEntityQueue
{

public:

    DatabaseEntityQueueWrapper(
            Database* database)
        : DatabaseEntityQueue(database)
    {
    }

    const std::queue<queue_item_type> get_foreground_queue()
    {
        return *foreground_queue_;
    }

    const std::queue<queue_item_type> get_background_queue()
    {
        return *background_queue_;
    }

    void do_swap()
    {
        swap();
    }

    /**
     * @brief Processes one sample and removes it from the front queue
     * 
     * This is necessary to check exception handling on the consumer
     * Consumers must be stopped and the queues swapped manually
     * 
     * @return true if anything was consumed
     */
    bool consume_sample()
    {
        if (empty())
        {
            return false;
        }

        process_sample();
        pop();
        return true;
    }
};

// Wrapper class to expose the internal attributes of the queue
class DatabaseDataQueueWrapper : public DatabaseDataQueue
{

public:

    DatabaseDataQueueWrapper(
            Database* database)
        : DatabaseDataQueue(database)
    {
    }

    const std::queue<queue_item_type> get_foreground_queue()
    {
        return *foreground_queue_;
    }

    const std::queue<queue_item_type> get_background_queue()
    {
        return *background_queue_;
    }

    void do_swap()
    {
        swap();
    }

    /**
     * @brief Processes one sample and removes it from the front queue
     * 
     * This is necessary to check exception handling on the consumer
     * Consumers must be stopped and the queues swapped manually
     * 
     * @return true if anything was consumed
     */
    bool consume_sample()
    {
        if (empty())
        {
            return false;
        }

        process_sample();
        pop();
        return true;
    }
};

struct InsertDataArgs
{
    void insert(
            const EntityId& id,
            const StatisticsSample& sample)
    {
        entities_.emplace_back(id);
        samples_.emplace_back(sample);
    }

    std::vector<EntityId> entities_;
    std::vector<std::reference_wrapper<const StatisticsSample>> samples_;
};

struct InsertEntityArgs
{
    InsertEntityArgs(
            EntityId returned_entity_id)
    {
        returned_entity_ids_.emplace_back(returned_entity_id);
    }

    EntityId insert(
            std::shared_ptr<Entity> entity)
    {
        entities_.emplace_back(entity);
        return returned_entity_ids_[counter_++];
    }

    unsigned int add_expected_id(
            EntityId returned_entity_id)
    {
        returned_entity_ids_.push_back(returned_entity_id);
        return returned_entity_ids_.size() - 1;
    }

    unsigned int counter_ = 0;
    std::vector<std::shared_ptr<Entity>> entities_;
    std::vector<EntityId> returned_entity_ids_;

};

class database_queue_tests: public ::testing::Test
{

public:

    Database database;
    DatabaseEntityQueueWrapper entity_queue;
    DatabaseDataQueueWrapper data_queue;

    database_queue_tests()
        : entity_queue(&database)
        , data_queue(&database)
    {
        ON_CALL(database, insert(_)).WillByDefault(Return(EntityId(1)));
        ON_CALL(database, get_entities_by_guid(_, _)).WillByDefault(Return(std::vector<EntityId>(1, EntityId(1))));
        ON_CALL(database, get_entities_by_name(_, _)).WillByDefault(Return(std::vector<EntityId>(1, EntityId(1))));
    }
};

TEST_F(database_queue_tests, start_stop_flush)
{
    // Generate some data
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
    std::shared_ptr<Host> host = std::make_shared<Host>("hostname");
    std::shared_ptr<User> user = std::make_shared<User>("username", host);
    std::shared_ptr<Process> process = std::make_shared<Process>("processname", "1", user);

    // Add something to the stopped queue
    EXPECT_CALL(database, insert(_)).Times(0);
    EXPECT_TRUE(entity_queue.stop_consumer());
    entity_queue.push(timestamp, host);
    entity_queue.push(timestamp, user);
    entity_queue.push(timestamp, process);

    EXPECT_TRUE(entity_queue.get_foreground_queue().empty());
    EXPECT_EQ(3, entity_queue.get_background_queue().size());

    // Flushing a stopped queue does nothing
    EXPECT_CALL(database, insert(_)).Times(0);
    entity_queue.flush();
    EXPECT_TRUE(entity_queue.get_foreground_queue().empty());
    EXPECT_EQ(3, entity_queue.get_background_queue().size());

    // Start the queue and flush
    EXPECT_CALL(database, insert(_)).Times(3);
    EXPECT_TRUE(entity_queue.start_consumer());
    entity_queue.flush();
    EXPECT_TRUE(entity_queue.get_foreground_queue().empty());
    EXPECT_TRUE(entity_queue.get_background_queue().empty());

    // Start the consumer when it is already started
    EXPECT_CALL(database, insert(_)).Times(0);
    EXPECT_FALSE(entity_queue.start_consumer());
    EXPECT_TRUE(entity_queue.get_foreground_queue().empty());
    EXPECT_TRUE(entity_queue.get_background_queue().empty());

    // Flush on an empty queue with running consumer
    EXPECT_CALL(database, insert(_)).Times(0);
    entity_queue.flush();
    EXPECT_TRUE(entity_queue.get_foreground_queue().empty());
    EXPECT_TRUE(entity_queue.get_background_queue().empty());

    // Flush on an empty queue with a stopped consumer
    EXPECT_TRUE(entity_queue.stop_consumer());
    EXPECT_CALL(database, insert(_)).Times(0);
    entity_queue.flush();
    EXPECT_TRUE(entity_queue.get_foreground_queue().empty());
    EXPECT_TRUE(entity_queue.get_background_queue().empty());

    // Stop the consumer when it is already stopped
    EXPECT_CALL(database, insert(_)).Times(0);
    EXPECT_FALSE(entity_queue.stop_consumer());
    EXPECT_TRUE(entity_queue.get_foreground_queue().empty());
    EXPECT_TRUE(entity_queue.get_background_queue().empty());

    // Start the consumer with an empty queue
    EXPECT_CALL(database, insert(_)).Times(0);
    EXPECT_TRUE(entity_queue.start_consumer());
    EXPECT_TRUE(entity_queue.get_foreground_queue().empty());
    EXPECT_TRUE(entity_queue.get_background_queue().empty());
}

TEST_F(database_queue_tests, push_host)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
    std::shared_ptr<Host> host = std::make_shared<Host>("hostname");
    EXPECT_CALL(database, insert(_)).Times(1);
    entity_queue.push(timestamp, host);
    entity_queue.flush();
}

TEST_F(database_queue_tests, push_user)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
    std::shared_ptr<Host> host = std::make_shared<Host>("hostname");
    std::shared_ptr<User> user = std::make_shared<User>("username", host);
    EXPECT_CALL(database, insert(_)).Times(1);
    entity_queue.push(timestamp, user);
    entity_queue.flush();
}

TEST_F(database_queue_tests, push_process)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
    std::shared_ptr<Host> host = std::make_shared<Host>("hostname");
    std::shared_ptr<User> user = std::make_shared<User>("username", host);
    std::shared_ptr<Process> process = std::make_shared<Process>("processname", "1", user);
    EXPECT_CALL(database, insert(_)).Times(1);
    entity_queue.push(timestamp, process);
    entity_queue.flush();
}

TEST_F(database_queue_tests, push_history_latency)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    DatabaseDataQueue::StatisticsWriterReaderData inner_data;
    inner_data.data(1.0);
    inner_data.writer_guid(writer_guid);
    inner_data.reader_guid(reader_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->writer_reader_data(inner_data);
    data->_d(EventKind::HISTORY2HISTORY_LATENCY);
    InsertArgs args;
    EXPECT_CALL(database, insert(_, _)).Times(1)
        .WillOnce(Invoke(&args, &InsertArgs::insert));
    data_queue.push(timestamp, data);
    data_queue.flush();

    EXPECT_EQ(args.id_, 1);
    EXPECT_EQ(args.sample_.kind, DataKind::FASTDDS_LATENCY);
    EXPECT_EQ(args.sample_.src_ts, timestamp);
}

TEST_F(database_queue_tests, push_network_latency)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    std::array<uint8_t, 16> src_locator_address = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

    DatabaseDataQueue::StatisticsLocator src_locator;
    src_locator.kind(LOCATOR_KIND_TCPv4);
    src_locator.port(1024);
    src_locator.address(src_locator_address);

    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(2048);
    dst_locator.address(dst_locator_address);

    DatabaseDataQueue::StatisticsLocator2LocatorData inner_data;
    inner_data.data(1.0);
    inner_data.src_locator(src_locator);
    inner_data.dst_locator(dst_locator);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->locator2locator_data(inner_data);
    data->_d(EventKind::NETWORK_LATENCY);
    InsertArgs args;
    EXPECT_CALL(database, insert(_, _)).Times(1)
        .WillOnce(Invoke(&args, &InsertArgs::insert));
    data_queue.push(timestamp, data);
    data_queue.flush();

    EXPECT_EQ(args.id_, 1);
    EXPECT_EQ(args.sample_.kind, DataKind::NETWORK_LATENCY);
    EXPECT_EQ(args.sample_.src_ts, timestamp);
}

TEST_F(database_queue_tests, push_publication_throughput)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    DatabaseDataQueue::StatisticsEntityData inner_data;
    inner_data.data(1.0);
    inner_data.guid(writer_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_data(inner_data);
    data->_d(EventKind::PUBLICATION_THROUGHPUT);
    InsertArgs args;
    EXPECT_CALL(database, insert(_, _)).Times(1)
        .WillOnce(Invoke(&args, &InsertArgs::insert));
    data_queue.push(timestamp, data);
    data_queue.flush();

    EXPECT_EQ(args.id_, 1);
    EXPECT_EQ(args.sample_.kind, DataKind::PUBLICATION_THROUGHPUT);
    EXPECT_EQ(args.sample_.src_ts, timestamp);
}

TEST_F(database_queue_tests, push_subscription_throughput)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    DatabaseDataQueue::StatisticsEntityData inner_data;
    inner_data.data(1.0);
    inner_data.guid(reader_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_data(inner_data);
    data->_d(EventKind::SUBSCRIPTION_THROUGHPUT);
    InsertArgs args;
    EXPECT_CALL(database, insert(_, _)).Times(1)
        .WillOnce(Invoke(&args, &InsertArgs::insert));
    data_queue.push(timestamp, data);
    data_queue.flush();

    EXPECT_EQ(args.id_, 1);
    EXPECT_EQ(args.sample_.kind, DataKind::SUBSCRIPTION_THROUGHPUT);
    EXPECT_EQ(args.sample_.src_ts, timestamp);
}

TEST_F(database_queue_tests, push_rtps_sent)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(2048);
    dst_locator.address(dst_locator_address);

    DatabaseDataQueue::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKind::RTPS_SENT);
    InsertArgs args1;
    InsertArgs args2;
    EXPECT_CALL(database, insert(_, _)).Times(2)
        .WillOnce(Invoke(&args1, &InsertArgs::insert))
        .WillOnce(Invoke(&args2, &InsertArgs::insert));
    data_queue.push(timestamp, data);
    data_queue.flush();

    EXPECT_EQ(args1.id_, 1);
    EXPECT_EQ(args1.sample_.kind, DataKind::RTPS_PACKETS_SENT);
    EXPECT_EQ(args1.sample_.src_ts, timestamp);

    EXPECT_EQ(args2.id_, 1);
    EXPECT_EQ(args2.sample_.kind, DataKind::RTPS_BYTES_SENT);
    EXPECT_EQ(args2.sample_.src_ts, timestamp);
}

TEST_F(database_queue_tests, push_rtps_lost)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(2048);
    dst_locator.address(dst_locator_address);

    DatabaseDataQueue::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKind::RTPS_LOST);
    InsertArgs args1;
    InsertArgs args2;
    EXPECT_CALL(database, insert(_, _)).Times(2)
        .WillOnce(Invoke(&args1, &InsertArgs::insert))
        .WillOnce(Invoke(&args2, &InsertArgs::insert));
    data_queue.push(timestamp, data);
    data_queue.flush();

    EXPECT_EQ(args1.id_, 1);
    EXPECT_EQ(args1.sample_.kind, DataKind::RTPS_PACKETS_LOST);
    EXPECT_EQ(args1.sample_.src_ts, timestamp);

    EXPECT_EQ(args2.id_, 1);
    EXPECT_EQ(args2.sample_.kind, DataKind::RTPS_BYTES_LOST);
    EXPECT_EQ(args2.sample_.src_ts, timestamp);
}

TEST_F(database_queue_tests, push_resent_datas)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::RESENT_DATAS);
    InsertArgs args;
    EXPECT_CALL(database, insert(_, _)).Times(1)
        .WillOnce(Invoke(&args, &InsertArgs::insert));
    data_queue.push(timestamp, data);
    data_queue.flush();

    EXPECT_EQ(args.id_, 1);
    EXPECT_EQ(args.sample_.kind, DataKind::RESENT_DATA);
    EXPECT_EQ(args.sample_.src_ts, timestamp);
}

TEST_F(database_queue_tests, push_heartbeat_count)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::HEARTBEAT_COUNT);
    InsertArgs args;
    EXPECT_CALL(database, insert(_, _)).Times(1)
        .WillOnce(Invoke(&args, &InsertArgs::insert));
    data_queue.push(timestamp, data);
    data_queue.flush();

    EXPECT_EQ(args.id_, 1);
    EXPECT_EQ(args.sample_.kind, DataKind::HEARTBEAT_COUNT);
    EXPECT_EQ(args.sample_.src_ts, timestamp);
}

TEST_F(database_queue_tests, push_acknack_count)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(reader_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::ACKNACK_COUNT);
    InsertArgs args;
    EXPECT_CALL(database, insert(_, _)).Times(1)
        .WillOnce(Invoke(&args, &InsertArgs::insert));
    data_queue.push(timestamp, data);
    data_queue.flush();

    EXPECT_EQ(args.id_, 1);
    EXPECT_EQ(args.sample_.kind, DataKind::ACKNACK_COUNT);
    EXPECT_EQ(args.sample_.src_ts, timestamp);
}

TEST_F(database_queue_tests, push_nackfrag_count)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(reader_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::NACKFRAG_COUNT);
    InsertArgs args;
    EXPECT_CALL(database, insert(_, _)).Times(1)
        .WillOnce(Invoke(&args, &InsertArgs::insert));
    data_queue.push(timestamp, data);
    data_queue.flush();

    EXPECT_EQ(args.id_, 1);
    EXPECT_EQ(args.sample_.kind, DataKind::NACKFRAG_COUNT);
    EXPECT_EQ(args.sample_.src_ts, timestamp);
}

TEST_F(database_queue_tests, push_gap_count)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::GAP_COUNT);
    InsertArgs args;
    EXPECT_CALL(database, insert(_, _)).Times(1)
        .WillOnce(Invoke(&args, &InsertArgs::insert));
    data_queue.push(timestamp, data);
    data_queue.flush();

    EXPECT_EQ(args.id_, 1);
    EXPECT_EQ(args.sample_.kind, DataKind::GAP_COUNT);
    EXPECT_EQ(args.sample_.src_ts, timestamp);
}

TEST_F(database_queue_tests, push_data_count)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::DATA_COUNT);
    InsertArgs args;
    EXPECT_CALL(database, insert(_, _)).Times(1)
        .WillOnce(Invoke(&args, &InsertArgs::insert));
    data_queue.push(timestamp, data);
    data_queue.flush();

    EXPECT_EQ(args.id_, 1);
    EXPECT_EQ(args.sample_.kind, DataKind::DATA_COUNT);
    EXPECT_EQ(args.sample_.src_ts, timestamp);
}

TEST_F(database_queue_tests, push_pdp_count)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(participant_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::PDP_PACKETS);
    InsertArgs args;
    EXPECT_CALL(database, insert(_, _)).Times(1)
        .WillOnce(Invoke(&args, &InsertArgs::insert));
    data_queue.push(timestamp, data);
    data_queue.flush();

    EXPECT_EQ(args.id_, 1);
    EXPECT_EQ(args.sample_.kind, DataKind::PDP_PACKETS);
    EXPECT_EQ(args.sample_.src_ts, timestamp);
}

TEST_F(database_queue_tests, push_edp_count)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(participant_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::EDP_PACKETS);
    InsertArgs args;
    EXPECT_CALL(database, insert(_, _)).Times(1)
        .WillOnce(Invoke(&args, &InsertArgs::insert));
    data_queue.push(timestamp, data);
    data_queue.flush();

    EXPECT_EQ(args.id_, 1);
    EXPECT_EQ(args.sample_.kind, DataKind::EDP_PACKETS);
    EXPECT_EQ(args.sample_.src_ts, timestamp);
}

TEST_F(database_queue_tests, push_discovery_times)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::array<uint8_t, 4> entity_id = {0, 0, 0, 1};

    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    DatabaseDataQueue::StatisticsGuidPrefix remote_prefix;
    remote_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId remote_entity_id;
    remote_entity_id.value(entity_id);
    DatabaseDataQueue::StatisticsGuid remote_guid;
    remote_guid.guidPrefix(remote_prefix);
    remote_guid.entityId(remote_entity_id);

    DatabaseDataQueue::StatisticsDiscoveryTime inner_data;
    inner_data.local_participant_guid(participant_guid);
    inner_data.remote_entity_guid(remote_guid);
    inner_data.time(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->discovery_time(inner_data);
    data->_d(EventKind::DISCOVERED_ENTITY);
    InsertArgs args;
    EXPECT_CALL(database, insert(_, _)).Times(1)
        .WillOnce(Invoke(&args, &InsertArgs::insert));
    data_queue.push(timestamp, data);
    data_queue.flush();

    EXPECT_EQ(args.id_, 1);
    EXPECT_EQ(args.sample_.kind, DataKind::DISCOVERY_TIME);
    EXPECT_EQ(args.sample_.src_ts, timestamp);
}

TEST_F(database_queue_tests, push_sample_datas)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    DatabaseDataQueue::StatisticsSequenceNumber sequence_number;
    sequence_number.high(1024);
    sequence_number.low(2048);

    DatabaseDataQueue::StatisticsSampleIdentity sample_identity;
    sample_identity.writer_guid(writer_guid);
    sample_identity.sequence_number(sequence_number);

    DatabaseDataQueue::StatisticsSampleIdentityCount inner_data;
    inner_data.count(1024);
    inner_data.sample_id(sample_identity);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->sample_identity_count(inner_data);
    data->_d(EventKind::SAMPLE_DATAS);
    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entities_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(1)
        .WillOnce(Return(std::vector<EntityId>(1, EntityId(1))));

     // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args;
    EXPECT_CALL(database, insert(_, _)).Times(1)
        .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();

    // Check expected arguments
    EXPECT_EQ(args.entities_[0], 1);
    EXPECT_EQ(args.samples_[0].get().src_ts, timestamp);
    EXPECT_EQ(args.samples_[0].get().kind, DataKind::SAMPLE_DATAS);
    EXPECT_EQ(dynamic_cast<const SampleDatasCountSample&>(args.samples_[0].get()).count, 1024);
    EXPECT_EQ(dynamic_cast<const SampleDatasCountSample&>(args.samples_[0].get()).sequence_number, sn.to64long());
}

TEST_F(database_queue_tests, push_physical_data_no_participant_exists)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
    std::string processname = "command";
    std::string pid = "1234";
    std::string username = "user";
    std::string hostname = "host";
    std::string participant_guid_str = "1.2.3.4.5.6.7.8.9.10.11.12.0.0.0.0";

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

    // Precondition: The participant does not exist
    EXPECT_CALL(database, get_entities_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
        .WillOnce(Return(std::vector<EntityId>()));

    // Add to the queue and wait to be processed
    data_queue.stop_consumer();
    data_queue.push(timestamp, data);
    data_queue.do_swap();
    ASSERT_THROW(data_queue.consume_sample(), eprosima::statistics_backend::BadParameter);
}

TEST_F(database_queue_tests, push_physical_data_process_exists)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
    std::string processname = "command";
    std::string pid = "1234";
    std::string username = "user";
    std::string hostname = "host";
    std::string participant_guid_str = "1.2.3.4.5.6.7.8.9.10.11.12.0.0.0.0";

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

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entities_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
        .WillOnce(Return(std::vector<EntityId>(1, EntityId(1))));

    // Precondition: The process exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::PROCESS, processname)).Times(1)
        .WillOnce(Return(std::vector<EntityId>(1, EntityId(2))));

    // Expectation: methods that may be called but we do not care at this moment
    EXPECT_CALL(database, get_entity(_)).Times(AnyNumber());

    // Expectation: The link method is called with appropriate arguments
    EXPECT_CALL(database, link_participant_with_process(EntityId(1), EntityId(2))).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_physical_data_no_process_exists)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
    std::string processname = "command";
    std::string pid = "1234";
    std::string username = "user";
    std::string hostname = "host";
    std::string participant_guid_str = "1.2.3.4.5.6.7.8.9.10.11.12.0.0.0.0";

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

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entities_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
        .WillOnce(Return(std::vector<EntityId>(1, EntityId(1))));

    // Precondition: The process does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::PROCESS, processname)).Times(1)
        .WillOnce(Return(std::vector<EntityId>()));

    // Precondition: The user exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::USER, username)).Times(1)
        .WillOnce(Return(std::vector<EntityId>(1, EntityId(2))));

    // Expectation: The process is created and given ID 3
    InsertEntityArgs insert_args(EntityId(3));
    EXPECT_CALL(database, insert(_)).Times(1)
        .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert));

    // Expectation: methods that may be called but we do not care at this moment
    EXPECT_CALL(database, get_entity(_)).Times(AnyNumber());

    // Expectation: The link method is called with appropriate arguments
    EXPECT_CALL(database, link_participant_with_process(EntityId(1), EntityId(3))).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();

    // Check expected arguments
    EXPECT_EQ(insert_args.entities_[0]->kind, EntityKind::PROCESS);
    EXPECT_EQ(insert_args.entities_[0]->name, processname);
}

TEST_F(database_queue_tests, push_physical_data_no_process_no_user_exists)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
    std::string processname = "command";
    std::string pid = "1234";
    std::string username = "user";
    std::string hostname = "host";
    std::string participant_guid_str = "1.2.3.4.5.6.7.8.9.10.11.12.0.0.0.0";

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

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entities_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
        .WillOnce(Return(std::vector<EntityId>(1, EntityId(1))));

    // Precondition: The process does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::PROCESS, processname)).Times(1)
        .WillOnce(Return(std::vector<EntityId>()));

    // Precondition: The user does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::USER, username)).Times(1)
        .WillOnce(Return(std::vector<EntityId>()));

    // Precondition: The host exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::HOST, hostname)).Times(1)
        .WillOnce(Return(std::vector<EntityId>(1, EntityId(2))));

    // Expectation: The user is created and given ID 3
    InsertEntityArgs insert_args(EntityId(3));

    // Expectation: The process is created and given ID 4
    insert_args.add_expected_id(EntityId(4));

    EXPECT_CALL(database, insert(_)).Times(2)
        .WillRepeatedly(Invoke(&insert_args, &InsertEntityArgs::insert));

    // Expectation: methods that may be called but we do not care at this moment
    EXPECT_CALL(database, get_entity(_)).Times(AnyNumber());

    // Expectation: The link method is called with appropriate arguments
    EXPECT_CALL(database, link_participant_with_process(EntityId(1), EntityId(4))).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();

    // Check expected arguments
    EXPECT_EQ(insert_args.entities_[0]->kind, EntityKind::USER);
    EXPECT_EQ(insert_args.entities_[0]->name, username);

    EXPECT_EQ(insert_args.entities_[1]->kind, EntityKind::PROCESS);
    EXPECT_EQ(insert_args.entities_[1]->name, processname);
}


TEST_F(database_queue_tests, push_physical_data_no_process_no_user_no_host_exists)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
    std::string processname = "command";
    std::string pid = "1234";
    std::string username = "user";
    std::string hostname = "host";
    std::string participant_guid_str = "1.2.3.4.5.6.7.8.9.10.11.12.0.0.0.0";

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

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entities_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
        .WillOnce(Return(std::vector<EntityId>(1, EntityId(1))));

    // Precondition: The process does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::PROCESS, processname)).Times(1)
        .WillOnce(Return(std::vector<EntityId>()));

    // Precondition: The user does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::USER, username)).Times(1)
        .WillOnce(Return(std::vector<EntityId>()));

    // Precondition: The host does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::HOST, hostname)).Times(1)
        .WillOnce(Return(std::vector<EntityId>()));

    // Expectation: The host is created and given ID 3
    InsertEntityArgs insert_args(EntityId(3));

    // Expectation: The user is created and given ID 4
    insert_args.add_expected_id(EntityId(4));

    // Expectation: The process is created and given ID 5
    insert_args.add_expected_id(EntityId(5));

    EXPECT_CALL(database, insert(_)).Times(3)
        .WillRepeatedly(Invoke(&insert_args, &InsertEntityArgs::insert));

    // Expectation: methods that may be called but we do not care at this moment
    EXPECT_CALL(database, get_entity(_)).Times(AnyNumber());

    // Expectation: The link method is called with appropriate arguments
    EXPECT_CALL(database, link_participant_with_process(EntityId(1), EntityId(5))).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();

    // Check expected arguments
    EXPECT_EQ(insert_args.entities_[0]->kind, EntityKind::HOST);
    EXPECT_EQ(insert_args.entities_[0]->name, hostname);

    EXPECT_EQ(insert_args.entities_[1]->kind, EntityKind::USER);
    EXPECT_EQ(insert_args.entities_[1]->name, username);

    EXPECT_EQ(insert_args.entities_[2]->kind, EntityKind::PROCESS);
    EXPECT_EQ(insert_args.entities_[2]->name, processname);
}

TEST_F(database_queue_tests, push_physical_data_wrong_processname_format)
{
    std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
    std::string processname = "command";
    std::string pid = "1234";
    std::string username = "user";
    std::string hostname = "host";
    std::string participant_guid_str = "1.2.3.4.5.6.7.8.9.10.11.12.0.0.0.0";

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

    // Build the process name with the wrong format
    std::stringstream ss;
    ss << processname << pid;
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

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entities_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
        .WillOnce(Return(std::vector<EntityId>(1, EntityId(1))));

    // Add to the queue and wait to be processed
    data_queue.stop_consumer();
    data_queue.push(timestamp, data);
    data_queue.do_swap();
    ASSERT_THROW(data_queue.consume_sample(), eprosima::statistics_backend::Error);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
