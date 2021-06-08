/* Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file StatisticsReaderListener.cpp
 */

#include "StatisticsReaderListener.hpp"

#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastrtps/types/TypesBase.h>

#include <database/database_queue.hpp>

namespace eprosima {
namespace statistics_backend {
namespace subscriber {

using namespace eprosima::fastrtps::types;
using namespace eprosima::fastdds::statistics;

StatisticsReaderListener::StatisticsReaderListener(
        database::DatabaseDataQueue* data_queue) noexcept
    : DataReaderListener()
    , data_queue_(data_queue)
{
}

void StatisticsReaderListener::on_data_available(
        eprosima::fastdds::dds::DataReader* reader)
{
    std::shared_ptr<Data> data = std::make_shared<Data>();
    eprosima::fastdds::dds::SampleInfo info;
    while (reader->take_next_sample(data.get(), &info) == ReturnCode_t::RETCODE_OK)
    {
        if (!info.valid_data)
        {
            return;
        }

        std::chrono::system_clock::time_point timestamp =
                std::chrono::system_clock::time_point (std::chrono::nanoseconds(info.source_timestamp.to_ns()));
        data_queue_->push(timestamp, data);
    }
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
