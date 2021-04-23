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
 * @file StatisticsParticipantListener.cpp
 */

#include "subscriber/StatisticsParticipantListener.hpp"

#include "fastdds/dds/domain/DomainParticipant.hpp"
#include "fastdds/dds/domain/DomainParticipantListener.hpp"
#include "fastdds/dds/core/status/StatusMask.hpp"

#include "database/database_queue.hpp"
#include "QosSerializer.hpp"


namespace eprosima {
namespace statistics_backend {
namespace subscriber {

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

const StatusMask StatisticsParticipantListener::StatisticsParticipantMask =
    StatusMask::none();

StatisticsParticipantListener::StatisticsParticipantListener(
        database::Database* database,
        database::DatabaseEntityQueue* entity_queue,
        database::DatabaseDataQueue* data_queue) noexcept
    : DomainParticipantListener()
    , database_(database)
    , entity_queue_(entity_queue)
    , data_queue_(data_queue)
{
}

void StatisticsParticipantListener::on_participant_discovery(
        DomainParticipant* participant,
        ParticipantDiscoveryInfo&& info)
{
    (void)participant, (void)info;
}

void StatisticsParticipantListener::on_subscriber_discovery(
        DomainParticipant* participant,
        ReaderDiscoveryInfo&& info)
{
    (void)participant, (void)info;
}

void StatisticsParticipantListener::on_publisher_discovery(
        eprosima::fastdds::dds::DomainParticipant* participant,
        fastrtps::rtps::WriterDiscoveryInfo&& info)
{
    (void)participant, (void)info;
}


} //namespace database
} //namespace statistics_backend
} //namespace eprosima
