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
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/statistics/topic_names.hpp>
#include <fastrtps/types/TypesBase.h>

#include <database/database_queue.hpp>
#include <topic_types/types.h>

namespace eprosima {
namespace statistics_backend {
namespace subscriber {

using namespace eprosima::fastrtps::types;
using namespace eprosima::fastdds::statistics;
using namespace eprosima::fastdds::dds;


static const std::map<std::string, EventKind> topics =
{
    {HISTORY_LATENCY_TOPIC,         HISTORY2HISTORY_LATENCY},
    {NETWORK_LATENCY_TOPIC,         NETWORK_LATENCY},
    {PUBLICATION_THROUGHPUT_TOPIC,  PUBLICATION_THROUGHPUT},
    {SUBSCRIPTION_THROUGHPUT_TOPIC, SUBSCRIPTION_THROUGHPUT},
    {RTPS_SENT_TOPIC,               RTPS_SENT},
    {RTPS_LOST_TOPIC,               RTPS_LOST},
    {RESENT_DATAS_TOPIC,            RESENT_DATAS},
    {HEARTBEAT_COUNT_TOPIC,         HEARTBEAT_COUNT},
    {ACKNACK_COUNT_TOPIC,           ACKNACK_COUNT},
    {NACKFRAG_COUNT_TOPIC,          NACKFRAG_COUNT},
    {GAP_COUNT_TOPIC,               GAP_COUNT},
    {DATA_COUNT_TOPIC,              DATA_COUNT},
    {PDP_PACKETS_TOPIC,             PDP_PACKETS},
    {EDP_PACKETS_TOPIC,             EDP_PACKETS},
    {DISCOVERY_TOPIC,               DISCOVERED_ENTITY},
    {SAMPLE_DATAS_TOPIC,            SAMPLE_DATAS},
    {PHYSICAL_DATA_TOPIC,           PHYSICAL_DATA}
};

StatisticsReaderListener::StatisticsReaderListener(
        database::DatabaseDataQueue* data_queue) noexcept
    : DataReaderListener()
    , data_queue_(data_queue)
{
}

template<typename T>
bool StatisticsReaderListener::get_available_data(
        eprosima::fastdds::dds::DataReader* reader,
        T& inner_data,
        std::chrono::system_clock::time_point& timestamp)
{
    SampleInfo info;
    if (reader->take_next_sample(&inner_data, &info) == ReturnCode_t::RETCODE_OK)
    {
        if (!info.valid_data)
        {
            // Received data not valid
            return false;
        }
        timestamp =
                std::chrono::system_clock::time_point (std::chrono::nanoseconds(info.source_timestamp.to_ns()));
        return true;
    }
    return false;
}

void StatisticsReaderListener::on_data_available(
        eprosima::fastdds::dds::DataReader* reader)
{
    std::shared_ptr<Data> data = std::make_shared<Data>();
    std::chrono::system_clock::time_point timestamp;

    const std::string& topic_name = reader->get_topicdescription()->get_name();

    bool enqueue = false;
    if (HISTORY_LATENCY_TOPIC == topic_name)
    {
        WriterReaderData inner_data;
        if (get_available_data(reader, inner_data, timestamp))
        {
            data->writer_reader_data(inner_data);
            enqueue = true;
        }
    }
    else if (NETWORK_LATENCY_TOPIC == topic_name)
    {
        Locator2LocatorData inner_data;
        if (get_available_data(reader, inner_data, timestamp))
        {
            data->locator2locator_data(inner_data);
            enqueue = true;
        }
    }
    else if (PUBLICATION_THROUGHPUT_TOPIC == topic_name || SUBSCRIPTION_THROUGHPUT_TOPIC == topic_name)
    {
        EntityData inner_data;
        if (get_available_data(reader, inner_data, timestamp))
        {
            data->entity_data(inner_data);
            enqueue = true;
        }
    }
    else if (RTPS_SENT_TOPIC == topic_name || RTPS_LOST_TOPIC == topic_name)
    {
        Entity2LocatorTraffic inner_data;
        if (get_available_data(reader, inner_data, timestamp))
        {
            data->entity2locator_traffic(inner_data);
            enqueue = true;
        }
    }
    else if (RESENT_DATAS_TOPIC == topic_name || HEARTBEAT_COUNT_TOPIC == topic_name ||
            ACKNACK_COUNT_TOPIC == topic_name || NACKFRAG_COUNT_TOPIC == topic_name || GAP_COUNT_TOPIC == topic_name ||
            DATA_COUNT_TOPIC == topic_name || PDP_PACKETS_TOPIC == topic_name || EDP_PACKETS_TOPIC == topic_name)
    {
        EntityCount inner_data;
        if (get_available_data(reader, inner_data, timestamp))
        {
            data->entity_count(inner_data);
            enqueue = true;
        }
    }
    else if (DISCOVERY_TOPIC == topic_name)
    {
        DiscoveryTime inner_data;
        if (get_available_data(reader, inner_data, timestamp))
        {
            data->discovery_time(inner_data);
            enqueue = true;
        }
    }
    else if (SAMPLE_DATAS_TOPIC == topic_name)
    {
        SampleIdentityCount inner_data;
        if (get_available_data(reader, inner_data, timestamp))
        {
            data->sample_identity_count(inner_data);
            enqueue = true;
        }
    }
    else if (PHYSICAL_DATA_TOPIC == topic_name)
    {
        PhysicalData inner_data;
        if (get_available_data(reader, inner_data, timestamp))
        {
            data->physical_data(inner_data);
            enqueue = true;
        }
    }

    if (!enqueue)
    {
        // Nothing to push to queue
        return;
    }

    data->_d(topics.at(topic_name));
    data_queue_->push(timestamp, data);
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
