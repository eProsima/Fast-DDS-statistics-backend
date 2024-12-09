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

#ifndef FASTDDS_STATISTICS_BACKEND_SRC_CPP_DATABASE__DATA_HPP
#define FASTDDS_STATISTICS_BACKEND_SRC_CPP_DATABASE__DATA_HPP

#include <chrono>
#include <vector>

#include <fastdds_statistics_backend/nlohmann-json/json.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/utils.hpp>

#include <types/DataContainer.hpp>
#include <types/MapDataContainer.hpp>
#include <database/samples.hpp>

namespace eprosima {
namespace statistics_backend {
namespace database {

/*
 * JSON tree structure for keeping DDS QoS
 */
using Qos = nlohmann::json;

/**
 * @brief Abstract common interface for all Statistical Data structures of every kind of entity.
 *
 * Every Statistical Data Structure must have a method to clear internal data depending on a Timestamp.
 * This class implements the \c clear methods for default arguments.
 */
struct Data
{

    /**
     * @brief Removes those internal data from the substructures that are previous to the time given.
     *
     * @param t_to last time to be removed from internal structures
     * @param clear_last_reported whether to remove the last reported data for each internal structure.
     *
     * @note if \c t_to is \c the_end_of_time (max time point) the structures are cleared completely,
     * making this call more efficient.
     */
    virtual void clear(
            const Timestamp& t_to,
            bool clear_last_reported) = 0;

    /**
     * @brief \c clear method with default parameter \c clear_last_reported to \c true .
     *
     * @param t_to last time to be removed from internal structures
     *
     * @note clear_last_reported is default to \c true to maintain old API.
     * However I think this should not be the default value.
     */
    virtual void clear(
            const Timestamp& t_to)
    {
        clear(t_to, true);
    }

    /**
     * @brief \c clear method with default parameter \c t_to to \c the_end_of_time .
     */
    virtual void clear()
    {
        clear(the_end_of_time());
    }

};

/**
 * Base struct for data related to Domain Participant
 *
 * @note I do not know why this struct exist, as it is only inherited by one class. ¯\_(ツ)_/¯
 */
struct RTPSData : public Data
{
    // Implement Data::clear virtual method
    using Data::clear;
    virtual void clear(
            const Timestamp& t_to,
            bool clear_last_reported) override;

    /*
     * Packet count data reported by topic: eprosima::fastdds::statistics::RTPS_SENT_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    details::MapDataContainer<EntityId, EntityCountSample> rtps_packets_sent;

    /*
     * Store the last packet count reported form topic: eprosima::fastdds::statistics::RTPS_SENT_TOPIC
     * by locator id.
     * This is done to speed up the calculation of the entries in rtps_packets_sent
     */
    std::map<EntityId, EntityCountSample> last_reported_rtps_packets_sent_count;

    /*
     * Byte count data reported by topic: eprosima::fastdds::statistics::RTPS_SENT_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    details::MapDataContainer<EntityId, ByteCountSample> rtps_bytes_sent;

    /*
     * Store the last byte count reported form topic: eprosima::fastdds::statistics::RTPS_SENT_TOPIC
     * by locator id.
     * This is done to speed up the calculation of the entries in rtps_bytes_sent
     */
    std::map<EntityId, ByteCountSample> last_reported_rtps_bytes_sent_count;

    /*
     * Packet count data reported by topic: eprosima::fastdds::statistics::RTPS_LOST_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    details::MapDataContainer<EntityId, EntityCountSample> rtps_packets_lost;

    /*
     * Store the last packet count reported form topic: eprosima::fastdds::statistics::RTPS_LOST_TOPIC
     * by locator id.
     * This is done to speed up the calculation of the entries in rtps_packets_lost
     */
    std::map<EntityId, EntityCountSample> last_reported_rtps_packets_lost_count;

    /*
     * Byte count data reported by topic: eprosima::fastdds::statistics::RTPS_SENT_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    details::MapDataContainer<EntityId, ByteCountSample> rtps_bytes_lost;

    /*
     * Store the last byte count reported form topic: eprosima::fastdds::statistics::RTPS_SENT_TOPIC
     * by locator id.
     * This is done to speed up the calculation of the entries in rtps_bytes_lost
     */
    std::map<EntityId, ByteCountSample> last_reported_rtps_bytes_lost_count;
};

/*
 * Data related to a DomainParticipant
 */
struct DomainParticipantStatisticsData : public RTPSData
{
    // Implement Data::clear virtual method
    using RTPSData::clear;
    virtual void clear(
            const Timestamp& t_to,
            bool clear_last_reported) override;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::DISCOVERY_TOPIC
     *
     * Collection of entities that the entity has discovered. The key of the map is the EntityId
     * of the discovered entity. The value is a pair of (time_point, bool), where the bool
     * represents whether the timepoint corresponds to a discovery/update (represented as ALIVE
     * with value 1), or to a un-discovery (represented as DISPOSED with value 0).
     */
    details::MapDataContainer<EntityId, DiscoveryTimeSample> discovered_entity;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::PDP_PACKETS_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    details::DataContainer<EntityCountSample> pdp_packets;

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
    details::DataContainer<EntityCountSample> edp_packets;

    /*
     * Store the last reported sample form topic: eprosima::fastdds::statistics::EDP_PACKETS_TOPIC
     * This is done to speed up the calculation of the entries in edp_packets
     */
    EntityCountSample last_reported_edp_packets;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::NETWORK_LATENCY_TOPIC
     *
     * Store the reported latencies between the participant and remote locator, identified by its
     * EntityId.
     */
    details::MapDataContainer<EntityId, EntityDataSample> network_latency_per_locator;
};

/*
 * Data related to a DataReader
 */
struct DataReaderStatisticsData : public Data
{
    // Implement Data::clear virtual method
    using Data::clear;
    virtual void clear(
            const Timestamp& t_to,
            bool clear_last_reported) override;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::SUBSCRIPTION_THROUGHPUT_TOPIC
     */
    details::DataContainer<EntityDataSample> subscription_throughput;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::ACKNACK_COUNT_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    details::DataContainer<EntityCountSample> acknack_count;

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
    details::DataContainer<EntityCountSample> nackfrag_count;

    /*
     * Store the last reported sample form topic: eprosima::fastdds::statistics::NACKFRAG_COUNT_TOPIC
     * This is done to speed up the calculation of the entries in nackfrag_count
     */
    EntityCountSample last_reported_nackfrag_count;
};

/*
 * Data related to a DataWriter
 */
struct DataWriterStatisticsData : public Data
{
    // Implement Data::clear virtual method
    using Data::clear;
    virtual void clear(
            const Timestamp& t_to,
            bool clear_last_reported) override;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::PUBLICATION_THROUGHPUT_TOPIC
     */
    details::DataContainer<EntityDataSample> publication_throughput;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::RESENT_DATAS_TOPIC
     *
     * Fast DDS reports the accumulated count. In opposition, the backend stores the count as the
     * difference between each accumulated report and the previous one.
     */
    details::DataContainer<EntityCountSample> resent_datas;

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
    details::DataContainer<EntityCountSample> heartbeat_count;

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
    details::DataContainer<EntityCountSample> gap_count;

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
    details::DataContainer<EntityCountSample> data_count;

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
    details::MapDataContainer<uint64_t, EntityCountSample> sample_datas;

    /*
     * Data reported by topic: eprosima::fastdds::statistics::HISTORY_LATENCY_TOPIC
     *
     * Store the reported latencies between the local and remote endpoint (identified by its
     * EntityId) histories.
     */
    details::MapDataContainer<EntityId, EntityDataSample> history2history_latency;
};

struct DataWriterMonitorServiceData : public Data
{
    // Implement Data::clear virtual method
    using Data::clear;
    virtual void clear(
            const Timestamp& t_to,
            bool clear_last_reported) override;

    /*
     * Proxy Data reported by topic: eprosima::fastdds::statistics::MONITOR_SERVICE_TOPIC
     */
    details::DataContainer<ProxySample> proxy;
    /*
     * Connection List Data reported by topic: eprosima::fastdds::statistics::MONITOR_SERVICE_TOPIC
     */
    details::DataContainer<ConnectionListSample> connection_list;
    /*
     * Incompatible QoS Data reported by topic: eprosima::fastdds::statistics::MONITOR_SERVICE_TOPIC
     */
    details::DataContainer<IncompatibleQosSample> incompatible_qos;
    /*
     * Inconsistent Topic Data reported by topic: eprosima::fastdds::statistics::MONITOR_SERVICE_TOPIC
     */
    details::DataContainer<InconsistentTopicSample> inconsistent_topic;
    /*
     * Liveliness Lost Data reported by topic: eprosima::fastdds::statistics::MONITOR_SERVICE_TOPIC
     */
    details::DataContainer<LivelinessLostSample> liveliness_lost;
    /*
     * Deadline Missed Data reported by topic: eprosima::fastdds::statistics::MONITOR_SERVICE_TOPIC
     */
    details::DataContainer<DeadlineMissedSample> deadline_missed;
};

struct DataReaderMonitorServiceData : public Data
{
    // Implement Data::clear virtual method
    using Data::clear;
    virtual void clear(
            const Timestamp& t_to,
            bool clear_last_reported) override;

    /*
     * Proxy Data reported by topic: eprosima::fastdds::statistics::MONITOR_SERVICE_TOPIC
     */
    details::DataContainer<ProxySample> proxy;
    /*
     * Connection List Data reported by topic: eprosima::fastdds::statistics::MONITOR_SERVICE_TOPIC
     */
    details::DataContainer<ConnectionListSample> connection_list;
    /*
     * Incompatible QoS Data reported by topic: eprosima::fastdds::statistics::MONITOR_SERVICE_TOPIC
     */
    details::DataContainer<IncompatibleQosSample> incompatible_qos;
    /*
     * Inconsistent Topic Data reported by topic: eprosima::fastdds::statistics::MONITOR_SERVICE_TOPIC
     */
    details::DataContainer<InconsistentTopicSample> inconsistent_topic;
    /*
     * Liveliness Changed Data reported by topic: eprosima::fastdds::statistics::MONITOR_SERVICE_TOPIC
     */
    details::DataContainer<LivelinessChangedSample> liveliness_changed;
    /*
     * Deadline Missed Data reported by topic: eprosima::fastdds::statistics::MONITOR_SERVICE_TOPIC
     */
    details::DataContainer<DeadlineMissedSample> deadline_missed;
    /*
     * Sample Lost Data reported by topic: eprosima::fastdds::statistics::MONITOR_SERVICE_TOPIC
     */
    details::DataContainer<SampleLostSample> sample_lost;
};

struct DomainParticipantMonitorServiceData : public Data
{
    // Implement Data::clear virtual method
    using Data::clear;
    virtual void clear(
            const Timestamp& t_to,
            bool clear_last_reported) override;

    /*
     * Proxy Data reported by topic: eprosima::fastdds::statistics::MONITOR_SERVICE_TOPIC
     */
    details::DataContainer<ProxySample> proxy;
    /*
     * Connection List Data reported by topic: eprosima::fastdds::statistics::MONITOR_SERVICE_TOPIC
     */
    details::DataContainer<ConnectionListSample> connection_list;

};

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif // FASTDDS_STATISTICS_BACKEND_SRC_CPP_DATABASE__DATA_HPP
