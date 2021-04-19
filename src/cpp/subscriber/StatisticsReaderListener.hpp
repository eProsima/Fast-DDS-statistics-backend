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
 * @file StatisticsDataListener.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_SUBSCRIBER_STATISTICSDATALISTENER_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_SUBSCRIBER_STATISTICSDATALISTENER_HPP_

#include <string>

#include "fastdds/dds/subscriber/DataReaderListener.hpp"
#include "fastdds/dds/core/status/StatusMask.hpp"


class eprosima::fastdds::dds::DataReader;

namespace eprosima {
namespace statistics_backend {

namespace database {

class DatabaseDataQueue;
class DatabaseEntityQueue;

} // namespace database

namespace subscriber {

/**
 * @brief Mask to be used by the subscribers that receive the statistics data updates
 */
static const eprosima::fastdds::dds::StatusMask StatisticsDataMask =
        eprosima::fastdds::dds::StatusMask::data_available();

/**
 * @brief Listener of the internal backend subscribers that receive the statistics data updates.
 */
class StatisticsReaderListener : public eprosima::fastdds::dds::DataReaderListener
{

public:

    /**
     * @brief Constructor
     */
    StatisticsReaderListener(
            database::DatabaseEntityQueue* entity_queue,
            database::DatabaseDataQueue* data_queue) noexcept
        : DataReaderListener()
        , entity_queue_(entity_queue)
        , data_queue_(data_queue)
    {
    }

    /**
     * @brief Actions to be performed when a new Data Message is received.
     * @param reader DataReader that received the data
     */
    void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override
    {
        (void)reader;
    }

protected:

    //! Reference to the database queues
    database::DatabaseEntityQueue* entity_queue_;
    database::DatabaseDataQueue* data_queue_;
};

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_SUBSCRIBER_STATISTICSDATALISTENER_HPP_
