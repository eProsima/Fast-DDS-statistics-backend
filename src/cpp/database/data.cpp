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

void RTPSData::clear(
        const Timestamp& t_to,
        bool clear_last_reported)
{
    rtps_packets_sent.clear(t_to);
    rtps_bytes_sent.clear(t_to);
    rtps_packets_lost.clear(t_to);
    rtps_bytes_lost.clear(t_to);
    if (clear_last_reported)
    {
        last_reported_rtps_packets_sent_count.clear();
        last_reported_rtps_bytes_sent_count.clear();
        last_reported_rtps_packets_lost_count.clear();
        last_reported_rtps_bytes_lost_count.clear();
    }
}

void DomainParticipantStatisticsData::clear(
        const Timestamp& t_to,
        bool clear_last_reported)
{
    RTPSData::clear(t_to, clear_last_reported);
    discovered_entity.clear(t_to);
    pdp_packets.clear(t_to);
    edp_packets.clear(t_to);
    network_latency_per_locator.clear(t_to);
    if (clear_last_reported)
    {
        last_reported_pdp_packets.clear();
        last_reported_edp_packets.clear();
    }
}

void DataReaderStatisticsData::clear(
        const Timestamp& t_to,
        bool clear_last_reported)
{
    subscription_throughput.clear(t_to);
    acknack_count.clear(t_to);
    nackfrag_count.clear(t_to);
    if (clear_last_reported)
    {
        last_reported_acknack_count.clear();
        last_reported_nackfrag_count.clear();
    }
}

void DataWriterStatisticsData::clear(
        const Timestamp& t_to,
        bool clear_last_reported)
{
    history2history_latency.clear(t_to);
    publication_throughput.clear(t_to);
    resent_datas.clear(t_to);
    heartbeat_count.clear(t_to);
    gap_count.clear(t_to);
    data_count.clear(t_to);
    sample_datas.clear(t_to);
    if (clear_last_reported)
    {
        last_reported_resent_datas.clear();
        last_reported_heartbeat_count.clear();
        last_reported_gap_count.clear();
        last_reported_data_count.clear();
    }
}

void DataReaderMonitorServiceData::clear(
        const Timestamp& t_to,
        bool clear_last_reported)
{
    proxy.clear(t_to);
    connection_list.clear(t_to);
    incompatible_qos.clear(t_to);
    liveliness_changed.clear(t_to);
    inconsistent_topic.clear(t_to);
    deadline_missed.clear(t_to);
    sample_lost.clear(t_to);
    extended_incompatible_qos.clear(t_to);
    static_cast<void>(clear_last_reported);
}

void DataWriterMonitorServiceData::clear(
        const Timestamp& t_to,
        bool clear_last_reported)
{
    proxy.clear(t_to);
    connection_list.clear(t_to);
    incompatible_qos.clear(t_to);
    liveliness_lost.clear(t_to);
    inconsistent_topic.clear(t_to);
    deadline_missed.clear(t_to);
    extended_incompatible_qos.clear(t_to);
    static_cast<void>(clear_last_reported);
}

void DomainParticipantMonitorServiceData::clear(
        const Timestamp& t_to,
        bool clear_last_reported)
{
    proxy.clear(t_to);
    connection_list.clear(t_to);
    static_cast<void>(clear_last_reported);
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
