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
 * @file data.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_DATA_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_DATA_HPP_

#include "samples.hpp"

#include <fastdds-statistics-backend/nlohmann-json/json.hpp>
#include <fastdds-statistics-backend/types/EntityId.hpp>

#include <chrono>
#include <vector>

namespace eprosima {
namespace statistics_backend {
namespace database {

/*
 * JSON tree structure for keeping DDS QoS
 */
using Qos = nlohmann::json;

/*
 * Base struct for all data strutures related to DDS entities
 */
struct DDSEntityData
{
    /**
     * Clear all the internal data structures.
     */
    virtual void clear() = 0;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::DISCOVERY_TOPIC
     *
     * Collection of entities that the entity has discovered. The key of the map is the EntityId
     * of the discovered entity. The value is a pair of (time_point, bool), where the bool
     * represents whether the timepoint corresponds to a discovery/update (represented as ALIVE
     * with value 1), or to a un-discovery (represented as DISPOSED with value 0).
     */
    std::map<EntityId, std::vector<std::pair<std::chrono::steady_clock::time_point, bool>>> discovered_entity;
};

/*
 * Data related to a DomainParticipant
 */
struct DomainParticipantData : DDSEntityData
{
    /**
     * Clear the vectors and maps, and set the counts to zero
     */
    void clear() final;


    /*
     * Data reported by topic: eprosima::fastdds::statistics::PDP_PACKETS_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    std::vector<EntityCountSample> pdp_packets;

    /*
     * Store the last reported sample form topic: eprosima::fastdds::statistics::PDP_PACKETS_TOPIC
     * This is done to speed up the calculation of the entries in pdp_packets
     */
    EntityCountSample last_reported_pdp_packets;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::EDP_PACKETS_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    std::vector<EntityCountSample> edp_packets;

    /*
     * Store the last reported sample form topic: eprosima::fastdds::statistics::EDP_PACKETS_TOPIC
     * This is done to speed up the calculation of the entries in edp_packets
     */
    EntityCountSample last_reported_edp_packets;
};

/*
 * Data related to a DataReader
 */
struct DataReaderData : DDSEntityData
{
    /**
     * Clear the vectors and maps, and set the counts to zero
     */
    void clear() final;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::SUBSCRIPTION_THROUGHPUT_TOPIC
     */
    std::vector<EntityDataSample> subscription_throughput;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::ACKNACK_COUNT_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    std::vector<EntityCountSample> acknack_count;

    /*
     * Store the last reported sample form topic: eprosima::fastdds::statistics::ACKNACK_COUNT_TOPIC
     * This is done to speed up the calculation of the entries in acknack_count
     */
    EntityCountSample last_reported_acknack_count;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::NACKFRAG_COUNT_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    std::vector<EntityCountSample> nackfrag_count;

    /*
     * Store the last reported sample form topic: eprosima::fastdds::statistics::NACKFRAG_COUNT_TOPIC
     * This is done to speed up the calculation of the entries in nackfrag_count
     */
    EntityCountSample last_reported_nackfrag_count;
};

/*
 * Base struct for data related to RTPS
 */
struct RTPSData
{
    /**
     * Clear the vectors and maps, and set the counts to zero
     */
    virtual void clear();

    /*
     * Packet count data reported by topic: eprosima::fastdds::statistics::RTPS_SENT_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    std::map<EntityId, std::vector<EntityCountSample>> rtps_packets_sent;

    /*
     * Store the last packet count reported form topic: eprosima::fastdds::statistics::RTPS_SENT_TOPIC
     * This is done to speed up the calculation of the entries in rtps_packets_sent
     */
    EntityCountSample last_reported_rtps_packets_sent_count;

    /*
     * Byte count data reported by topic: eprosima::fastdds::statistics::RTPS_SENT_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    std::map<EntityId, std::vector<ByteCountSample>> rtps_bytes_sent;

    /*
     * Store the last byte count reported form topic: eprosima::fastdds::statistics::RTPS_SENT_TOPIC
     * This is done to speed up the calculation of the entries in rtps_bytes_sent
     */
    ByteCountSample last_reported_rtps_bytes_sent_count;

    /*
     * Packet count data reported by topic: eprosima::fastdds::statistics::RTPS_LOST_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    std::map<EntityId, std::vector<EntityCountSample>> rtps_packets_lost;

    /*
     * Store the last packet count reported form topic: eprosima::fastdds::statistics::RTPS_LOST_TOPIC
     * This is done to speed up the calculation of the entries in rtps_packets_lost
     */
    EntityCountSample last_reported_rtps_packets_lost_count;

    /*
     * Byte count data reported by topic: eprosima::fastdds::statistics::RTPS_SENT_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    std::map<EntityId, std::vector<ByteCountSample>> rtps_bytes_lost;

    /*
     * Store the last byte count reported form topic: eprosima::fastdds::statistics::RTPS_SENT_TOPIC
     * This is done to speed up the calculation of the entries in rtps_bytes_lost
     */
    ByteCountSample last_reported_rtps_bytes_lost_count;

protected:

    /**
     * Empty protected constructor so the struct cannot be instantiated
     */
    RTPSData(){};
};

/*
 * Data related to a DataWriter
 */
struct DataWriterData : DDSEntityData, RTPSData
{
    /**
     * Clear the vectors and maps, and set the counts to zero
     */
    void clear() final;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::PUBLICATION_THROUGHPUT_TOPIC
     */
    std::vector<EntityDataSample> publication_throughput;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::RESENT_DATAS_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    std::vector<EntityCountSample> resent_datas;

    /*
     * Store the last byte count reported form topic: eprosima::fastdds::statistics::RESENT_DATAS_TOPIC
     * This is done to speed up the calculation of the entries in resent_datas
     */
    EntityCountSample last_reported_resent_datas;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::HEARTBEAT_COUNT_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    std::vector<EntityCountSample> heartbeat_count;

    /*
     * Store the last byte count reported form topic: eprosima::fastdds::statistics::HEARTBEAT_COUNT_TOPIC
     * This is done to speed up the calculation of the entries in heartbeat_count
     */
    EntityCountSample last_reported_heartbeat_count;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::GAP_COUNT_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    std::vector<EntityCountSample> gap_count;

    /*
     * Store the last byte count reported form topic: eprosima::fastdds::statistics::GAP_COUNT_TOPIC
     * This is done to speed up the calculation of the entries in gap_count
     */
    EntityCountSample last_reported_gap_count;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::DATA_COUNT_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    std::vector<EntityCountSample> data_count;

    /*
     * Store the last byte count reported form topic: eprosima::fastdds::statistics::DATA_COUNT_TOPIC
     * This is done to speed up the calculation of the entries in data_count
     */
    EntityCountSample last_reported_data_count;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::SAMPLE_DATAS_TOPIC
     *
     * The key of the map corresponds to the sequence number of the change.
     * The second one is the number of DATA/DATAFRAG sub-messages sent for sending that change.
     */
    std::map<uint64_t, uint64_t> sample_datas;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::HISTORY_LATENCY_TOPIC
     *
     * Store the reported latencies between the local and remote endpoint (identified by its
     * EntityId) histories.
     */
    std::map<EntityId, std::vector<EntityDataSample>> history2history_latency;
};

/*
 * Data related to a locator
 */
struct LocatorData
{
    /**
     * Clear the vectors and maps, and set the counts to zero
     */
    void clear() final;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::NETWORK_LATENCY_TOPIC
     *
     * Store the reported latencies between the local and remote locator, identified by its
     * EntityId.
     */
    std::map<EntityId, std::vector<EntityDataSample>> network_latency_per_locator;
};

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_DATA_HPP_
