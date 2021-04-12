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

#include "data.hpp"

namespace eprosima {
namespace statistics_backend {
namespace database {

void DomainParticipantData::clear()
{
    discovered_entity.clear();
    pdp_packets.clear();
    last_reported_pdp_packets.clear();
    edp_packets.clear();
    last_reported_edp_packets.clear();
}

void DataReaderData::clear()
{
    discovered_entity.clear();
    history2history_latency.clear();
    subscription_throughput.clear();
    acknack_count.clear();
    last_reported_acknack_count.clear();
    nackfrag_count.clear();
    last_reported_nackfrag_count.clear();
}

void RTPSData::clear()
{
    rtps_packets_sent.clear();
    last_reported_rtps_packets_sent_count.clear();
    rtps_bytes_sent.clear();
    last_reported_rtps_bytes_sent_count.clear();
    rtps_packets_lost.clear();
    last_reported_rtps_packets_lost_count.clear();
    rtps_bytes_lost.clear();
    last_reported_rtps_bytes_lost_count.clear();
}

void DataWriterData::clear()
{
    discovered_entity.clear();
    history2history_latency.clear();
    RTPSData::clear();
    publication_throughput.clear();
    resent_datas.clear();
    last_reported_resent_datas.clear();
    heartbeat_count.clear();
    last_reported_heartbeat_count.clear();
    gap_count.clear();
    last_reported_gap_count.clear();
    data_count.clear();
    last_reported_data_count.clear();
    sample_datas.clear();
}

void LocatorData::clear()
{
    RTPSData::clear();
    network_latency_per_locator.clear();
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
