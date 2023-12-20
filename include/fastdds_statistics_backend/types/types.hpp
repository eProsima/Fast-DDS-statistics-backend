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
 * @file types.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_TYPES_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_TYPES_HPP_

#include <fastdds_statistics_backend/nlohmann-json/json.hpp>
#include <fastdds_statistics_backend/types/Bitmask.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/topic_types/monitorservice_types.h>

#include <chrono>

namespace eprosima {
namespace statistics_backend {

/**
 * Info tree structure. Please refer to https://nlohmann.github.io/json/doxygen/index.html
 */
using Info = nlohmann::json;

/**
 * Topology graph tree structure.  Please refer to https://nlohmann.github.io/json/doxygen/index.html
 */
using Graph = nlohmann::json;

/**
 * Type DDS Domain IDs
 */
using DomainId = uint32_t;

/**
 * Type used to represent time points
 */
using Timestamp = std::chrono::time_point<std::chrono::system_clock>;

/**
 * Type representing the database in a tree structure
 */
using DatabaseDump = nlohmann::json;

/**
 * @brief Type of the data returned by the backend.
 *
 * The first field represents the time at which the data was recorded.
 * This can be the time of the raw data point if no bins are being used,
 * or the starting time of the bin (see get_data()).
 *
 * The second field represents the data value itself.
 * This will be the value of the calculated statistic, or the raw data
 * if no statistic has been requested (see get_data()).
 *
 * \sa get_data()
 *
 */
using StatisticsData = std::pair<Timestamp, double>;

/*
 * Schema for Entities kinds store in Backend
 *                  *
 *    PHYSICAL      *           LOGICAL
 *   ──────────     *          ─────────
 *                  *
 * ┌───────────┐    *     ┌───────────┐
 * │   HOST    │    *     │  DOMAIN   │1
 * └──┬────────┘    *     └─┬─────────┴─────┐
 *    │1            *       │1              │
 *    │             *       │               │
 *    │N            *       │               │N
 * ┌──▼────────┐    *       │            ┌──▼────────┐
 * │   USER    │    *       │            │   TOPIC   │
 * └──┬────────┘    *       │            ├──────────┬┘
 *    │1            *       │            │1         │1
 *    │             *       │            │          │
 *    │N            *       │N           │          │
 * ┌──▼────────┐1   *    N┌─▼─────────┐  │          │
 * │  PROCESS  ├──────────►PARTICIPANT│1 │          │
 * └───────────┘    *     └─────┬─────┴──┼───────┐  │
 *                  *           │1       │       │  │
 *                  *           │        │       │  │
 *                  *           │N       │N      │N │N
 *                  *         ┌─▼────────▼┐    ┌─▼──▼──────┐
 *                  *         │DATAREADER │    │DATAWRITER │
 *                  *         └─┬─────────┘    └┬──────────┘
 *                  *           │M              │M
 * ┌───────────┐N   *           │               │
 * │  LOCATOR  ◄────────────────┴───────────────┘
 * └───────────┘    *
 */

/**
 * Indicates the Type of an Entity in Statistics Backend structure
 */
enum class EntityKind
{
    /// Invalid entity kind
    INVALID,

    /// Host/Machine where a participant is allocated
    HOST,

    /// User that has executed a participant
    USER,

    /// Process where a participant is running
    PROCESS,

    /// Abstract DDS network by Domain or by Discovery Server
    DOMAIN,

    /// DDS Topic
    TOPIC,

    /// DDS Domain Participant
    PARTICIPANT,

    /// DDS DataWriter
    DATAWRITER,

    /// DDS DataReader
    DATAREADER,

    /// Physical locator that a communication is using (IP + port || SHM + port)
    /// Store the Locator Statistic data
    LOCATOR,
};

/**
 * Indicates the Status level in Statistics Backend structure
 */
enum class StatusLevel
{
    /// Ok entity status
    OK_STATUS,

    /// Warning entity status
    WARNING_STATUS,

    /// Error entity status
    ERROR_STATUS,
};

/**
 * Indicates the AppId of a participant in Statistics Backend structure
 */
enum class AppId
{
    /// Unknown App
    UNKNOWN,

    /// Fast DDS Statistics Backend App
    FASTDDS_MONITOR,

    /// DDS Router App
    DDS_ROUTER,

    /// Shapes Demo App
    SHAPES_DEMO,

    /// Integration Service App
    INTEGRATION_SERVICE,

    /// Fast DDS Visualizer App
    FASTDDS_VISUALIZER,

    /// Fast DDS Spy App
    FASTDDS_SPY,

    /// DDS Recorder App
    DDS_RECORDER,

    /// DDS Replayer App
    DDS_REPLAYER,

    /// AML-IP App
    AML_IP,
};

/**
 * Indicates the Type of Data stored by the Backend
 *
 *     | Signature               | Entities source   | Entity target | No. entities |
 *     |-------------------------|-------------------|---------------|--------------|
 *     | FASTDDS_LATENCY         | DataWriter        | DataReader    | 2            |
 *     | NETWORK_LATENCY         | DomainParticipant | Locator       | 2            |
 *     | PUBLICATION_THROUGHPUT  | DataWriter        |               | 1            |
 *     | SUBSCRIPTION_THROUGHPUT | DataReader        |               | 1            |
 *     | RTPS_PACKETS_SENT       | DomainParticipant | Locator       | 2            |
 *     | RTPS_BYTES_SENT         | DomainParticipant | Locator       | 2            |
 *     | RTPS_PACKETS_LOST       | DomainParticipant | Locator       | 2            |
 *     | RTPS_BYTES_LOST         | DomainParticipant | Locator       | 2            |
 *     | RESENT_DATA             | DataWriter        |               | 1            |
 *     | HEARTBEAT_COUNT         | DataWriter        |               | 1            |
 *     | ACKNACK_COUNT           | DataReader        |               | 1            |
 *     | NACKFRAG_COUNT          | DataReader        |               | 1            |
 *     | GAP_COUNT               | DataWriter        |               | 1            |
 *     | DATA_COUNT              | DataWriter        |               | 1            |
 *     | PDP_PACKETS             | DomainParticipant |               | 1            |
 *     | EDP_PACKETS             | DomainParticipant |               | 1            |
 *     | DISCOVERY_TIME          | DomainParticipant | DDSEntity     | 2            |
 *     | SAMPLE_DATAS            | DataWriter        |               | 1            |
 */
enum class DataKind : int32_t
{
    /// Represents no valid data kind
    INVALID                     = 0,

    /// Latency between a write operation (writer side) and data available
    /// (notification to user in reader side)
    FASTDDS_LATENCY             = 1 << 0,

    /// Latency between Locators pair
    NETWORK_LATENCY             = 1 << 1,

    /// Amount of data [Mb/s] sent by a DataWriter
    PUBLICATION_THROUGHPUT      = 1 << 2,

    /// Amount of data [Mb/s] received by a DataReader
    SUBSCRIPTION_THROUGHPUT     = 1 << 3,

    /// Amount of packets sent from a DDS Entity to a Locator
    RTPS_PACKETS_SENT           = 1 << 4,

    /// Amount of bytes sent from a DDS Entity to a Locator
    RTPS_BYTES_SENT             = 1 << 5,

    /// Amount of packets lost from a DDS Entity to a Locator
    RTPS_PACKETS_LOST           = 1 << 6,

    /// Amount of bytes lost from a DDS Entity to a Locator
    RTPS_BYTES_LOST             = 1 << 7,

    /// Amount of DATA/DATAFRAG sub-messages resent from a DataWriter/DomainParticipant
    RESENT_DATA                 = 1 << 8,

    /// Amount of HEARTBEATs that each non discovery DataWriter/DomainParticipant sends
    HEARTBEAT_COUNT             = 1 << 9,

    /// Amount of ACKNACKs that each non discovery DataReader/DomainParticipant sends
    ACKNACK_COUNT               = 1 << 10,

    /// Amount of NACKFRAGs that each non discovery DataReader/DomainParticipant sends
    NACKFRAG_COUNT              = 1 << 11,

    /// Amount of GAPs sub-messages sent from a DataWriter/DomainParticipant
    GAP_COUNT                   = 1 << 12,

    /// Amount of DATA/DATAFRAG sub-messages that each non discovery DataWriter sends
    DATA_COUNT                  = 1 << 13,

    /// Amount of PDP packets sent by Participant
    PDP_PACKETS                 = 1 << 14,

    /// Amount of EDP packets sent by Participant
    EDP_PACKETS                 = 1 << 15,

    /// Time when a DDS Entity discovers another DDS entity
    DISCOVERY_TIME              = 1 << 16,

    /// Amount of DATA/DATAFRAG sub-messages needed to send a single sample
    SAMPLE_DATAS                = 1 << 17,
};

/**
 * Indicates the Type of Monitor Service Status Data stored by the Backend
 */
enum class StatusKind : int32_t
{
    /// Represents no valid status data kind
    INVALID                     = 0,

    /// Collection of Parameters describing the Proxy Data of that entity.
    PROXY                       = 1 << 0,

    /// List of connections that this entity is using. Described here in more detail
    CONNECTION_LIST             = 1 << 1,

    /// Status of the Incompatible QoS of that entity.
    INCOMPATIBLE_QOS            = 1 << 2,

    /// Status of Inconsistent topics of the topic of that entity.
    INCONSISTENT_TOPIC          = 1 << 3,

    /// Tracks the status of the number of times that liveliness was lost (writer side).
    LIVELINESS_LOST             = 1 << 4,

    /// Tracks the status of the number of times that liveliness status changed (reader side).
    LIVELINESS_CHANGED          = 1 << 5,

    /// The Status of the number of deadlines missed that were registered in that entity.
    DEADLINE_MISSED             = 1 << 6,

    /// Tracks the number of times that this entity lost samples.
    SAMPLE_LOST                 = 1 << 7,

    ///
    STATUSES_SIZE               = 1 << 8,
};

/**
 * @brief Bitmask of data kinds
 *
 * values of DataKind can be combined with the '|' operator to build the mask:
 *
 * \code{.cpp}
 *     DataKindMask mask = DataKind::PUBLICATION_THROUGHPUT | DataKind::SUBSCRIPTION_THROUGHPUT;
 * \endcode
 *
 * \sa Bitmask
 */
using DataKindMask = Bitmask<DataKind>;

/*
 * Available statistics operations to be performed on the raw data.
 */
enum class StatisticKind
{
    /// Non accumulative kind, it chooses a data point between the set given.
    /// Implemented to take the first data in set : [0]
    NONE,

    /// Numerical mean of values in the set
    MEAN,

    /// Standard Deviation of the values in the set
    STANDARD_DEVIATION,

    /// Maximum value in the set
    MAX,

    /// Minimum value in the set
    MIN,

    /// Median value of the set
    MEDIAN,

    /// Amount of values in the set
    COUNT,

    /// Summation of the values in the set
    SUM
};


/** @struct MonitorServiceSample
 * Base class for all monitor service status samples. It adds the timepoint and status level to the sample
 *
 * \sa get_status_data()
 */
struct MonitorServiceSample
{
    MonitorServiceSample(
            StatusKind sample_kind = StatusKind::INVALID,
            StatusLevel sample_status = StatusLevel::OK_STATUS)
        : kind(sample_kind)
        , status(sample_status)
    {
    }

    virtual ~MonitorServiceSample() = default;

    virtual FASTDDS_STATISTICS_BACKEND_DllAPI
    void clear();

    inline bool operator ==(
            const MonitorServiceSample& other) const noexcept
    {
        return (kind == other.kind && status == other.status && src_ts == other.src_ts);
    }

    inline bool operator !=(
            const MonitorServiceSample& other) const noexcept
    {
        return !(*this == other);
    }

    StatusKind kind;
    StatusLevel status;
    std::chrono::system_clock::time_point src_ts;
};


/** @struct ProxySample
 * @brief Proxy data sample of an entity.
 */
struct ProxySample : MonitorServiceSample
{
    ProxySample()
        : MonitorServiceSample(StatusKind::PROXY)
    {
    }

    virtual ~ProxySample() = default;

    FASTDDS_STATISTICS_BACKEND_DllAPI
    void clear() final;

    inline bool operator ==(
            const ProxySample& other) const noexcept
    {
        return (MonitorServiceSample::operator ==(other) && entity_proxy == other.entity_proxy);
    }

    inline bool operator !=(
            const ProxySample& other) const noexcept
    {
        return !(*this == other);
    }

    std::vector<uint8_t> entity_proxy;
};

/** @struct ConnectionListSample
 * Connection list sample of an entity. Each of the elements is a Connection in which the possible values for the ConnectionMode are: intraprocess, data sharing, transport.
 */
struct ConnectionListSample : MonitorServiceSample
{
    ConnectionListSample()
        : MonitorServiceSample(StatusKind::CONNECTION_LIST)
    {
    }

    virtual ~ConnectionListSample() = default;

    FASTDDS_STATISTICS_BACKEND_DllAPI
    void clear() final;

    inline bool operator ==(
            const ConnectionListSample& other) const noexcept
    {
        return (MonitorServiceSample::operator ==(other) && connection_list == other.connection_list);
    }

    inline bool operator !=(
            const ConnectionListSample& other) const noexcept
    {
        return !(*this == other);
    }

    std::vector<eprosima::fastdds::statistics::Connection> connection_list;
};

/** @struct IncompatibleQosSample
 * Incompatible Qos sample of an entity:
 *  - DataWriter Incompatible QoS Offered
 *  - DataReader Incompatible QoS Requested.
 */
struct IncompatibleQosSample : MonitorServiceSample
{
    IncompatibleQosSample()
        : MonitorServiceSample(StatusKind::INCOMPATIBLE_QOS)
    {
    }

    virtual ~IncompatibleQosSample() = default;

    FASTDDS_STATISTICS_BACKEND_DllAPI
    void clear() final;

    inline bool operator ==(
            const IncompatibleQosSample& other) const noexcept
    {
        return (MonitorServiceSample::operator ==(other) && incompatible_qos_status == other.incompatible_qos_status);
    }

    inline bool operator !=(
            const IncompatibleQosSample& other) const noexcept
    {
        return !(*this == other);
    }

    eprosima::fastdds::statistics::IncompatibleQoSStatus_s incompatible_qos_status;
};

/** @struct InconsistentTopicSample
 * Inconsistent topic sample of the topic of that entity. Asked to the topic of the requested entity.
 */
struct InconsistentTopicSample : MonitorServiceSample
{
    InconsistentTopicSample()
        : MonitorServiceSample(StatusKind::INCONSISTENT_TOPIC)
    {
    }

    virtual ~InconsistentTopicSample() = default;

    FASTDDS_STATISTICS_BACKEND_DllAPI
    void clear() final;

    inline bool operator ==(
            const InconsistentTopicSample& other) const noexcept
    {
        return (MonitorServiceSample::operator ==(other) &&
               inconsistent_topic_status == other.inconsistent_topic_status);
    }

    inline bool operator !=(
            const InconsistentTopicSample& other) const noexcept
    {
        return !(*this == other);
    }

    eprosima::fastdds::statistics::InconsistentTopicStatus_s inconsistent_topic_status;
};

/** @struct LivelinessLostSample
 * Liveliness lost sample containing the number of times that liveliness was lost by a DataWriter.
 */
struct LivelinessLostSample : MonitorServiceSample
{
    LivelinessLostSample()
        : MonitorServiceSample(StatusKind::LIVELINESS_LOST)
    {
    }

    virtual ~LivelinessLostSample() = default;

    FASTDDS_STATISTICS_BACKEND_DllAPI
    void clear() final;

    inline bool operator ==(
            const LivelinessLostSample& other) const noexcept
    {
        return (MonitorServiceSample::operator ==(other) && liveliness_lost_status == other.liveliness_lost_status);
    }

    inline bool operator !=(
            const LivelinessLostSample& other) const noexcept
    {
        return !(*this == other);
    }

    eprosima::fastdds::statistics::LivelinessLostStatus_s liveliness_lost_status;
};

/** @struct LivelinessChangedSample
 * Liveliness changed sample containing the number of times that liveliness status changed in a DataReader.
 */
struct LivelinessChangedSample : MonitorServiceSample
{
    LivelinessChangedSample()
        : MonitorServiceSample(StatusKind::LIVELINESS_CHANGED)
    {
    }

    virtual ~LivelinessChangedSample() = default;

    FASTDDS_STATISTICS_BACKEND_DllAPI
    void clear() final;

    inline bool operator ==(
            const LivelinessChangedSample& other) const noexcept
    {
        return (MonitorServiceSample::operator ==(other) &&
               liveliness_changed_status == other.liveliness_changed_status);
    }

    inline bool operator !=(
            const LivelinessChangedSample& other) const noexcept
    {
        return !(*this == other);
    }

    eprosima::fastdds::statistics::LivelinessChangedStatus_s liveliness_changed_status;
};

/** @struct DeadlineMissedSample
 * Deadline missed sample containing the number of deadlines missed that were registered in that entity.
 */
struct DeadlineMissedSample : MonitorServiceSample
{
    DeadlineMissedSample()
        : MonitorServiceSample(StatusKind::DEADLINE_MISSED)
    {
    }

    virtual ~DeadlineMissedSample() = default;

    FASTDDS_STATISTICS_BACKEND_DllAPI
    void clear() final;

    inline bool operator ==(
            const DeadlineMissedSample& other) const noexcept
    {
        return (MonitorServiceSample::operator ==(other) && deadline_missed_status == other.deadline_missed_status);
    }

    inline bool operator !=(
            const DeadlineMissedSample& other) const noexcept
    {
        return !(*this == other);
    }

    eprosima::fastdds::statistics::DeadlineMissedStatus_s deadline_missed_status;
};

/** @struct SampleLostSample
 * Sample lost sample containing the number of times that this entity lost samples.
 */
struct SampleLostSample : MonitorServiceSample
{
    SampleLostSample()
        : MonitorServiceSample(StatusKind::SAMPLE_LOST)
    {
    }

    virtual ~SampleLostSample() = default;

    FASTDDS_STATISTICS_BACKEND_DllAPI
    void clear() final;

    inline bool operator ==(
            const SampleLostSample& other) const noexcept
    {
        return (MonitorServiceSample::operator ==(other) && sample_lost_status == other.sample_lost_status);
    }

    inline bool operator !=(
            const SampleLostSample& other) const noexcept
    {
        return !(*this == other);
    }

    eprosima::fastdds::statistics::SampleLostStatus_s sample_lost_status;
};

} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_TYPES_HPP_
