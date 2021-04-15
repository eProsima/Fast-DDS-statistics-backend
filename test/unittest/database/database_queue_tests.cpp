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
};


class database_queue_tests: public ::testing::Test
{

public:

    Database database;
    DatabaseEntityQueueWrapper entity_queue;

    database_queue_tests()
        : entity_queue(&database)
    {
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

