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
 * @file StatisticsReaderListener.hpp
 */

#ifndef FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__STATISTICS_READER_LISTENER_HPP
#define FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__STATISTICS_READER_LISTENER_HPP

#include "fastdds/dds/subscriber/DataReaderListener.hpp"
#include "fastdds/dds/core/status/StatusMask.hpp"
#include <fastdds/statistics/dds/domain/DomainParticipant.hpp>

#include <fastdds_statistics_backend/topic_types/monitorservice_types.hpp>
#include <fastdds_statistics_backend/topic_types/types.hpp>
#include <database/data.hpp>

namespace eprosima {
namespace statistics_backend {

namespace database {

template <typename T>
class DatabaseDataQueue;
class Database;
class DatabaseEntityQueue;
struct ExtendedMonitorServiceStatusData;

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
            EntityId domain_id,
            database::DatabaseDataQueue<eprosima::fastdds::statistics::Data>* data_queue,
            database::DatabaseDataQueue<database::ExtendedMonitorServiceStatusData>* monitor_service_status_data_queue,
            const database::Database* db)
    noexcept;

    /**
     * @brief Actions to be performed when a new Data Message is received.
     * @param reader DataReader that received the data
     */
    void on_data_available(
            eprosima::fastdds::dds::DataReader* reader) override;

protected:

    template<typename T>
    bool get_available_data(
            eprosima::fastdds::dds::DataReader* reader,
            T& inner_data,
            std::chrono::system_clock::time_point& timestamp);

    /**
     * @brief Deserializes the proxy sample and extracts the optional QoS information (in database format) from
     * a MonitorService proxy sample storing everything in the ExtendedMonitorServiceStatusData object.
     *
     * @param participant Reference to the DomainParticipant used to deserialize the proxy data.
     * @param data The monitor service status data containing the proxy sample.
     * @param extended_data The ExtendedMonitorServiceStatusData object where the deserialized QoS information
     * will be stored.
     *
     * @return true if the QoS object was successfully filled, false otherwise.
     */
    bool deserialize_proxy_data(
            eprosima::fastdds::statistics::dds::DomainParticipant* participant,
            const eprosima::fastdds::statistics::MonitorServiceStatusData& data,
            database::ExtendedMonitorServiceStatusData& extended_data);

    //! Reference to the database queues
    database::DatabaseDataQueue<eprosima::fastdds::statistics::Data>* data_queue_;
    database::DatabaseDataQueue<database::ExtendedMonitorServiceStatusData>*
            monitor_service_status_data_queue_;

    //! Const reference to the database
    const database::Database* db_;

    // Database Domain ID to which this listener belongs
    const EntityId domain_id_;
};

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif // FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__STATISTICS_READER_LISTENER_HPP
