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
 * @file StatisticsParticipantListener.hpp
 */

#ifndef FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__STATISTICS_PARTICIPANT_LISTENER_HPP
#define FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__STATISTICS_PARTICIPANT_LISTENER_HPP

#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/Locator.hpp>

#include <database/entities.hpp>

#include <fastdds_statistics_backend/topic_types/monitorservice_types.hpp>

#include <fastdds_statistics_backend/types/app_names.h>

namespace eprosima {
namespace statistics_backend {
namespace database {

class Database;
template <typename T>
class DatabaseDataQueue;
class DatabaseEntityQueue;

} // namespace database

namespace subscriber {

/**
 * @brief Listener of the internal backend statistics participants.
 */
class StatisticsParticipantListener : public eprosima::fastdds::dds::DomainParticipantListener
{

public:

    /**
     * @brief Constructor
     */
    StatisticsParticipantListener(
            EntityId domain_id,
            database::Database* database,
            database::DatabaseEntityQueue* entity_queue,
            database::DatabaseDataQueue<eprosima::fastdds::statistics::Data>* data_queue,
            database::DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>* monitor_service_data_queue)
    noexcept;

    /*!
     * This method is called when a new Participant is discovered, or a previously discovered participant changes
     * its QOS or is removed.
     * @param participant Pointer to the Participant which discovered the remote participant.
     * @param info Remote participant information. User can take ownership of the object.
     */
    void on_participant_discovery(
            eprosima::fastdds::dds::DomainParticipant* participant,
            fastdds::rtps::ParticipantDiscoveryInfo&& info,
            bool &should_be_ignored) override;

    /*!
     * This method is called when a new Subscriber is discovered, or a previously discovered subscriber changes
     * its QOS or is removed.
     * @param participant Pointer to the Participant which discovered the remote subscriber.
     * @param info Remote subscriber information. User can take ownership of the object.
     */
    void on_data_reader_discovery(
            fastdds::dds::DomainParticipant* participant,
            fastdds::rtps::ReaderDiscoveryInfo&& info,
            bool &should_be_ignored) override;

    /*!
     * This method is called when a new DataWriter is discovered, or a previously discovered DataWriter changes
     * its QOS or is removed.
     *
     * @param [in]  participant        Pointer to the Participant which discovered the remote writer.
     * @param [in]  reason             The reason motivating this method to be called.
     * @param [in]  info               Remote writer information.
     * @param [out] should_be_ignored  Flag to indicate the library to automatically ignore the discovered writer.
     */
    void on_data_writer_discovery(
            fastdds::dds::DomainParticipant* participant,
            fastdds::rtps::WriterDiscoveryStatus reason,
            const fastdds::rtps::PublicationBuiltinTopicData& info,
            bool& should_be_ignored) override;

protected:

    EntityId domain_id_;                                                                                                ///< The DomainId this listener is monitoring
    database::Database* database_;                                                                                      ///< Reference to the statistics database. Injected on construction
    database::DatabaseEntityQueue* entity_queue_;                                                                       ///< Reference to the statistics entity queue. Injected on construction
    database::DatabaseDataQueue<eprosima::fastdds::statistics::Data>* data_queue_;                                      ///< Reference to the statistics data queue. Injected on construction
    database::DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>*
            monitor_service_status_data_queue_;                                                                                      ///< Reference to the monitor service status data queue. Injected on construction
};


} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif // FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__STATISTICS_PARTICIPANT_LISTENER_HPP
