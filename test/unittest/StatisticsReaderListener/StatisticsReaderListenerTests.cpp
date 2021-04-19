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
#include <subscriber/StatisticsReaderListener.hpp>
#include <topic_types/types.h>

#include "gtest/gtest.h"
#include <gmock/gmock.h>

using namespace eprosima::statistics_backend::database;
using StatisticsData = eprosima::fastdds::statistics::Data;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::AnyNumber;

using namespace eprosima::fastdds::statistics;
using namespace eprosima::statistics_backend::database;
using namespace eprosima::statistics_backend::subscriber;


class statistics_reader_listener_tests : public ::testing::Test
{

public:

    Database database;
    DatabaseEntityQueue entity_queue;
    DatabaseDataQueue data_queue;
    StatisticsReaderListener reader_listener;

    statistics_reader_listener_tests()
        : entity_queue(&database)
        , data_queue(&database)
        , reader_listener(&entity_queue, &data_queue)
    {
    }

};

TEST_F(statistics_reader_listener_tests, new_history_latency_received)
{
    reader_listener.on_data_available(nullptr);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
