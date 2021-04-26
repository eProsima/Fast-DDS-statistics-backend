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
        DataKindMask mask,
        database::DatabaseDataQueue* data_queue) noexcept
    : DataReaderListener()
    , data_queue_(data_queue)
    , data_mask_(mask)
{
}

DataKindMask StatisticsReaderListener::set_mask(
        DataKindMask mask)
{
    std::swap(data_mask_, mask);
    return mask;
}

/**
 * @brief Retrieves the mask for the data kind.
 * @return mask The mask for the data kind
 * @return The old mask
 */
DataKindMask StatisticsReaderListener::get_mask()
{
    return data_mask_;
}


void StatisticsReaderListener::on_data_available(
        eprosima::fastdds::dds::DataReader* reader)
{
    std::shared_ptr<Data> data = std::make_shared<Data>();
    eprosima::fastdds::dds::SampleInfo info;
    if (reader->take_next_sample(data.get(), &info) != ReturnCode_t::RETCODE_OK)
    {
        return;
    }

    bool process = false;
    switch(data->_d())
    {
        case EventKind::HISTORY2HISTORY_LATENCY:
            if (data_mask_.is_set(DataKind::FASTDDS_LATENCY))
            {
                process = true;
            }
            break;
        case EventKind::NETWORK_LATENCY:
            if (data_mask_.is_set(DataKind::NETWORK_LATENCY))
            {
                process = true;
            }
            break;
        case EventKind::PUBLICATION_THROUGHPUT:
            if (data_mask_.is_set(DataKind::PUBLICATION_THROUGHPUT))
            {
                process = true;
            }
            break;
        case EventKind::SUBSCRIPTION_THROUGHPUT:
            if (data_mask_.is_set(DataKind::SUBSCRIPTION_THROUGHPUT))
            {
                process = true;
            }
            break;
        case EventKind::RTPS_SENT:
            if (data_mask_.is_set(DataKind::RTPS_PACKETS_SENT)  ||
                data_mask_.is_set(DataKind::RTPS_BYTES_SENT))
            {
                process = true;
            }
            break;
        case EventKind::RTPS_LOST:
            if (data_mask_.is_set(DataKind::RTPS_PACKETS_LOST)  ||
                data_mask_.is_set(DataKind::RTPS_BYTES_LOST))
            {
                process = true;
            }
            break;
        case EventKind::RESENT_DATAS:
            if (data_mask_.is_set(DataKind::RESENT_DATA))
            {
                process = true;
            }
            break;
        case EventKind::HEARTBEAT_COUNT:
            if (data_mask_.is_set(DataKind::HEARTBEAT_COUNT))
            {
                process = true;
            }
            break;
        case EventKind::ACKNACK_COUNT:
            if (data_mask_.is_set(DataKind::ACKNACK_COUNT))
            {
                process = true;
            }
            break;
        case EventKind::NACKFRAG_COUNT:
            if (data_mask_.is_set(DataKind::NACKFRAG_COUNT))
            {
                process = true;
            }
            break;
        case EventKind::GAP_COUNT:
            if (data_mask_.is_set(DataKind::GAP_COUNT))
            {
                process = true;
            }
            break;
        case EventKind::DATA_COUNT:
            if (data_mask_.is_set(DataKind::DATA_COUNT))
            {
                process = true;
            }
            break;
        case EventKind::PDP_PACKETS:
            if (data_mask_.is_set(DataKind::PDP_PACKETS))
            {
                process = true;
            }
            break;
        case EventKind::EDP_PACKETS:
            if (data_mask_.is_set(DataKind::EDP_PACKETS))
            {
                process = true;
            }
            break;
        case EventKind::DISCOVERED_ENTITY:
            if (data_mask_.is_set(DataKind::DISCOVERY_TIME))
            {
                process = true;
            }
            break;
        case EventKind::SAMPLE_DATAS:
            if (data_mask_.is_set(DataKind::SAMPLE_DATAS))
            {
                process = true;
            }
            break;
        case EventKind::PHYSICAL_DATA:
            process = true;
            break;
    }

    if (process)
    {
        std::chrono::steady_clock::time_point timestamp =
                std::chrono::steady_clock::time_point (std::chrono::nanoseconds(info.source_timestamp.to_ns()));
        data_queue_->push(timestamp, data);
    }
}


} //namespace database
} //namespace statistics_backend
} //namespace eprosima
