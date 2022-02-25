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

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_SUBSCRIBER_STATISTICSPARTICIPANTLISTENER_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_SUBSCRIBER_STATISTICSPARTICIPANTLISTENER_HPP_

#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/Locator.h>

#include <database/entities.hpp>


namespace eprosima {
namespace statistics_backend {
namespace database {

class Database;
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
            database::DatabaseDataQueue* data_queue) noexcept;

    /*!
     * This method is called when a new Participant is discovered, or a previously discovered participant changes
     * its QOS or is removed.
     * @param participant Pointer to the Participant which discovered the remote participant.
     * @param info Remote participant information. User can take ownership of the object.
     */
    virtual void on_participant_discovery(
            eprosima::fastdds::dds::DomainParticipant* participant,
            fastrtps::rtps::ParticipantDiscoveryInfo&& info) override;

    /*!
     * This method is called when a new Subscriber is discovered, or a previously discovered subscriber changes
     * its QOS or is removed.
     * @param participant Pointer to the Participant which discovered the remote subscriber.
     * @param info Remote subscriber information. User can take ownership of the object.
     */
    virtual void on_subscriber_discovery(
            eprosima::fastdds::dds::DomainParticipant* participant,
            fastrtps::rtps::ReaderDiscoveryInfo&& info) override;

    /*!
     * This method is called when a new Publisher is discovered, or a previously discovered publisher changes
     * its QOS or is removed.
     * @param participant Pointer to the Participant which discovered the remote publisher.
     * @param info Remote publisher information. User can take ownership of the object.
     */
    virtual void on_publisher_discovery(
            eprosima::fastdds::dds::DomainParticipant* participant,
            fastrtps::rtps::WriterDiscoveryInfo&& info) override;

protected:

    EntityId domain_id_;                            ///< The DomainId this listener is monitoring
    database::Database* database_;                  ///< Reference to the statistics database. Injected on construction
    database::DatabaseEntityQueue* entity_queue_;   ///< Reference to the statistics entity queue. Injected on construction
    database::DatabaseDataQueue* data_queue_;       ///< Reference to the statistics data queue. Injected on construction
};


} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_SUBSCRIBER_STATISTICSPARTICIPANTLISTENER_HPP_
