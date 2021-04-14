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
 * @file samples.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_SAMPLES_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_SAMPLES_HPP_

#include <chrono>

#include <fastdds-statistics-backend/types/types.hpp>


namespace eprosima {
namespace statistics_backend {
namespace database {

/*
 * Base class for all statistics samples. It adds the timepoint for the sample
 */
struct StatisticsSample
{
    StatisticsSample(
            DataKind sample_kind = DataKind::INVALID)
        : kind(sample_kind)
    {
    }

    DataKind kind;
    std::chrono::steady_clock::time_point src_ts;
};

/*
 * Generic data reported by an entity
 */
struct EntityDataSample : StatisticsSample
{
    EntityDataSample(
            DataKind sample_kind = DataKind::INVALID)
        : StatisticsSample(sample_kind)
    {
    }

    double data;
};

/*
 * Count reported by an entity
 */
struct EntityCountSample : StatisticsSample
{
    EntityCountSample(
            DataKind sample_kind = DataKind::INVALID)
        : StatisticsSample(sample_kind)
    {
    }

    uint64_t count;
};

/*
 * Count reported by an entity. The magnitude is a multiplyer in base 10
 */
struct ByteCountSample : StatisticsSample
{
    ByteCountSample(
            DataKind sample_kind = DataKind::INVALID)
        : StatisticsSample(sample_kind)
    {
    }

    uint64_t count;
    int16_t magnitude_order;
};

/*
 * Time reported by an entity.
 */
struct TimepointSample : StatisticsSample
{
    TimepointSample(
            DataKind sample_kind = DataKind::INVALID)
        : StatisticsSample(sample_kind)
    {
    }

    std::chrono::steady_clock::time_point time;
};

/*
 * Count reported by an entity towards a Locator
 */
struct EntityToLocatorCountSample : EntityCountSample
{
    EntityToLocatorCountSample(
            DataKind sample_kind = DataKind::INVALID)
        : EntityCountSample(sample_kind)
    {
    }

    std::string remote_locator;

};

/*
 * Bytes reported by an entity towards a Locator
 */
struct ByteToLocatorCountSample : ByteCountSample
{
    ByteToLocatorCountSample(
            DataKind sample_kind = DataKind::INVALID)
        : ByteCountSample(sample_kind)
    {
    }

    std::string remote_locator;

};

/*
 * Latency between a write operation and data available
 */
struct HistoryLatencySample : EntityDataSample
{
    HistoryLatencySample()
        : EntityDataSample(DataKind::FASTDDS_LATENCY)
    {
    }

    std::string reader_guid;
};

/*
 * Latency between Locators pair
 */
struct NetworkLatencySample : EntityDataSample
{
    NetworkLatencySample()
        : EntityDataSample(DataKind::NETWORK_LATENCY)
    {
    }

    std::string remote_locator;
};

/*
 * Amount of data sent by a DataWriter
 */
struct PublicationThroughputSample : EntityDataSample
{
    PublicationThroughputSample()
        : EntityDataSample(DataKind::PUBLICATION_THROUGHPUT)
    {
    }
};

/*
 * Amount of data received by a DataReader
 */
struct SubscriptionThroughputSample : EntityDataSample
{
    SubscriptionThroughputSample()
        : EntityDataSample(DataKind::SUBSCRIPTION_THROUGHPUT)
    {
    }
};

/*
 * Amount of packets sent from a DDS Entity to a Locator
 */
struct RtpsPacketsSentSample : EntityToLocatorCountSample
{
    RtpsPacketsSentSample()
        : EntityToLocatorCountSample(DataKind::RTPS_PACKETS_SENT)
    {
    }

};

/*
 * Amount of bytes sent from a DDS Entity to a Locator
 */
struct RtpsBytesSentSample : ByteToLocatorCountSample
{
    RtpsBytesSentSample()
        : ByteToLocatorCountSample(DataKind::RTPS_BYTES_SENT)
    {
    }

};

/*
 * Amount of packets lost from a DDS Entity to a Locator
 */
struct RtpsPacketsLostSample : EntityToLocatorCountSample
{
    RtpsPacketsLostSample()
        : EntityToLocatorCountSample(DataKind::RTPS_PACKETS_LOST)
    {
    }

    std::string remote_locator;

};

/*
 * Amount of bytes lost from a DDS Entity to a Locator
 */
struct RtpsBytesLostSample : ByteToLocatorCountSample
{
    RtpsBytesLostSample()
        : ByteToLocatorCountSample(DataKind::RTPS_BYTES_LOST)
    {
    }

};

/*
 * Amount of DATA/DATAFRAG sub-messages resent from a DataWriter
 */
struct ResentDataSample : EntityCountSample
{
    ResentDataSample()
        : EntityCountSample(DataKind::RESENT_DATA)
    {
    }
};

/*
 * Amount of HEARTBEATs that each non discovery DataWriter sends
 */
struct HeartbeatCountSample : EntityCountSample
{
    HeartbeatCountSample()
        : EntityCountSample(DataKind::HEARTBEAT_COUNT)
    {
    }
};

/*
 * Amount of ACKNACKs that each non discovery DataWriter sends
 */
struct AcknackCountSample : EntityCountSample
{
    AcknackCountSample()
        : EntityCountSample(DataKind::ACKNACK_COUNT)
    {
    }
};

/*
 * Amount of NACKFRAGs that each non discovery DataWriter sends
 */
struct NackfragCountSample : EntityCountSample
{
    NackfragCountSample()
        : EntityCountSample(DataKind::NACKFRAG_COUNT)
    {
    }
};

/*
 * Amount of GAPs sub-messages sent from a DataWriter
 */
struct GapCountSample : EntityCountSample
{
    GapCountSample()
        : EntityCountSample(DataKind::GAP_COUNT)
    {
    }
};

/*
 * Amount of DATA/DATAFRAG sub-messages that each non discovery DataWriter sends
 */
struct DataCountSample : EntityCountSample
{
    DataCountSample()
        : EntityCountSample(DataKind::DATA_COUNT)
    {
    }
};

/*
 * Amount of PDP packets sent by Participant
 */
struct PdpCountSample : EntityCountSample
{
    PdpCountSample()
        : EntityCountSample(DataKind::PDP_PACKETS)
    {
    }
};

/*
 * Amount of EDP packets sent by Participant
 */
struct EdpCountSample : EntityCountSample
{
    EdpCountSample()
        : EntityCountSample(DataKind::EDP_PACKETS)
    {
    }
};

/*
 * Time when a DDS Entity discovers another DDS entity
 */
struct DiscoveryTimeSample : TimepointSample
{
    DiscoveryTimeSample()
        : TimepointSample(DataKind::DISCOVERY_TIME)
    {
    }

    std::string remote_entity_guid;

};

/*
 * Time when a DDS Entity discovers another DDS entity
 */
struct SampleDatasCountSample : EntityCountSample
{
    SampleDatasCountSample()
        : EntityCountSample(DataKind::SAMPLE_DATAS)
    {
    }

    uint64_t sequence_number;

};

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_SAMPLES_HPP_
