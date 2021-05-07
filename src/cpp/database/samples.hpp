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

    virtual ~StatisticsSample() = default;

    virtual void clear();

    inline bool operator ==(
            const StatisticsSample& other) const noexcept
    {
        return (kind == other.kind && src_ts == other.src_ts);
    }

    inline bool operator !=(
            const StatisticsSample& other) const noexcept
    {
        return !(*this == other);
    }

    DataKind kind;
    std::chrono::system_clock::time_point src_ts;
};

/*
 * Generic data reported by an entity
 */
struct EntityDataSample : StatisticsSample
{
    EntityDataSample(
            DataKind sample_kind = DataKind::INVALID)
        : StatisticsSample(sample_kind)
        , data(0)
    {
    }

    virtual ~EntityDataSample() = default;

    void clear() final;

    inline bool operator ==(
            const EntityDataSample& other) const noexcept
    {
        return (StatisticsSample::operator ==(other) && data == other.data);
    }

    inline bool operator !=(
            const EntityDataSample& other) const noexcept
    {
        return !(*this == other);
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
        , count(0)
    {
    }

    virtual ~EntityCountSample() = default;

    void clear() final;

    inline bool operator ==(
            const EntityCountSample& other) const noexcept
    {
        return (StatisticsSample::operator ==(other) && count == other.count);
    }

    inline bool operator !=(
            const EntityCountSample& other) const noexcept
    {
        return !(*this == other);
    }

    inline EntityCountSample operator -(
            const EntityCountSample& other) const noexcept
    {
        assert(count >= other.count);
        EntityCountSample ret(kind);
        ret.src_ts = src_ts;
        ret.count = count - other.count;
        return ret;
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
        , count(0)
        , magnitude_order(0)
    {
    }

    virtual ~ByteCountSample() = default;

    void clear() final;

    inline bool operator ==(
            const ByteCountSample& other) const noexcept
    {
        return (StatisticsSample::operator ==(other) && count == other.count &&
               magnitude_order == other.magnitude_order);
    }

    inline bool operator !=(
            const ByteCountSample& other) const noexcept
    {
        return !(*this == other);
    }


    /**
     * ByteCountSample is, in a way, a number expressed in base 2^64, where count is the first digit from the right
     * (LSD), and magnitude_order is the second digit from the right (MSD). However, it has 2 peculiarities:
     *     1. MSD range is [-2^15; 2^15-1]
     *     2. LSD range is [0; 2^64-1]
     *
     * Because LSD CANNOT be negative, a negative number represented as ByteCountSample is a bit counter intuitive
     * at first. Following, and example of the representation of the same number in base 10 and as a ByteCountSample
     * is given (assume that a number can expressed as (MSD, LSD) in both representations).
     *     BASE 10:
     *         -9 = (0, -9)
     *     ByteCountSample
     *         -9 = (-1, 2^64 - 9) -> This is interpreted as (-(1 * 2^64) + (2^64 - 9)) = -2^64 + 2^64 -9 = -9
     */
    inline ByteCountSample operator -(
            const ByteCountSample& other) const noexcept
    {
        ByteCountSample ret(kind);
        ret.src_ts = src_ts;
        ret.count = count - other.count;
        ret.magnitude_order = magnitude_order - other.magnitude_order;
        if (ret.count > count)
        {
            ret.magnitude_order -=  1;
        }
        return ret;
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

    virtual ~TimepointSample() = default;

    inline bool operator ==(
            const TimepointSample& other) const noexcept
    {
        return (StatisticsSample::operator ==(other) && time == other.time);
    }

    inline bool operator !=(
            const TimepointSample& other) const noexcept
    {
        return !(*this == other);
    }

    std::chrono::system_clock::time_point time;
};

/*
 * Count reported by an entity towards a Locator
 */
struct EntityToLocatorCountSample : EntityCountSample
{
    EntityToLocatorCountSample(
            DataKind sample_kind = DataKind::INVALID)
        : EntityCountSample(sample_kind)
        , remote_locator(EntityId::invalid())
    {
    }

    virtual ~EntityToLocatorCountSample() = default;

    inline bool operator ==(
            const EntityToLocatorCountSample& other) const noexcept
    {
        return (EntityCountSample::operator ==(other) && remote_locator == other.remote_locator);
    }

    inline bool operator !=(
            const EntityToLocatorCountSample& other) const noexcept
    {
        return !(*this == other);
    }

    EntityId remote_locator;

};

/*
 * Bytes reported by an entity towards a Locator
 */
struct ByteToLocatorCountSample : ByteCountSample
{
    ByteToLocatorCountSample(
            DataKind sample_kind = DataKind::INVALID)
        : ByteCountSample(sample_kind)
        , remote_locator(EntityId::invalid())
    {
    }

    virtual ~ByteToLocatorCountSample() = default;

    inline bool operator ==(
            const ByteToLocatorCountSample& other) const noexcept
    {
        return (ByteCountSample::operator ==(other) && remote_locator == other.remote_locator);
    }

    inline bool operator !=(
            const ByteToLocatorCountSample& other) const noexcept
    {
        return !(*this == other);
    }

    EntityId remote_locator;

};

/*
 * Latency between a write operation and data available
 */
struct HistoryLatencySample : EntityDataSample
{
    HistoryLatencySample()
        : EntityDataSample(DataKind::FASTDDS_LATENCY)
        , reader(EntityId::invalid())
    {
    }

    inline bool operator ==(
            const HistoryLatencySample& other) const noexcept
    {
        return (EntityDataSample::operator ==(other) && reader == other.reader);
    }

    inline bool operator !=(
            const HistoryLatencySample& other) const noexcept
    {
        return !(*this == other);
    }

    EntityId reader;
};

/*
 * Latency between Locators pair
 */
struct NetworkLatencySample : EntityDataSample
{
    NetworkLatencySample()
        : EntityDataSample(DataKind::NETWORK_LATENCY)
        , remote_locator(EntityId::invalid())
    {
    }

    inline bool operator ==(
            const NetworkLatencySample& other) const noexcept
    {
        return (EntityDataSample::operator ==(other) && remote_locator == other.remote_locator);
    }

    inline bool operator !=(
            const NetworkLatencySample& other) const noexcept
    {
        return !(*this == other);
    }

    EntityId remote_locator;
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

    inline bool operator ==(
            const PublicationThroughputSample& other) const noexcept
    {
        return (EntityDataSample::operator ==(other));
    }

    inline bool operator !=(
            const PublicationThroughputSample& other) const noexcept
    {
        return !(*this == other);
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

    inline bool operator ==(
            const SubscriptionThroughputSample& other) const noexcept
    {
        return (EntityDataSample::operator ==(other));
    }

    inline bool operator !=(
            const SubscriptionThroughputSample& other) const noexcept
    {
        return !(*this == other);
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

    inline bool operator ==(
            const RtpsPacketsSentSample& other) const noexcept
    {
        return (EntityToLocatorCountSample::operator ==(other));
    }

    inline bool operator !=(
            const RtpsPacketsSentSample& other) const noexcept
    {
        return !(*this == other);
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

    inline bool operator ==(
            const RtpsBytesSentSample& other) const noexcept
    {
        return (ByteToLocatorCountSample::operator ==(other));
    }

    inline bool operator !=(
            const RtpsBytesSentSample& other) const noexcept
    {
        return !(*this == other);
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

    inline bool operator ==(
            const RtpsPacketsLostSample& other) const noexcept
    {
        return (EntityToLocatorCountSample::operator ==(other) && remote_locator == other.remote_locator);
    }

    inline bool operator !=(
            const RtpsPacketsLostSample& other) const noexcept
    {
        return !(*this == other);
    }

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

    inline bool operator ==(
            const RtpsBytesLostSample& other) const noexcept
    {
        return (ByteToLocatorCountSample::operator ==(other));
    }

    inline bool operator !=(
            const RtpsBytesLostSample& other) const noexcept
    {
        return !(*this == other);
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

    inline bool operator ==(
            const ResentDataSample& other) const noexcept
    {
        return (EntityCountSample::operator ==(other));
    }

    inline bool operator !=(
            const ResentDataSample& other) const noexcept
    {
        return !(*this == other);
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

    inline bool operator ==(
            const HeartbeatCountSample& other) const noexcept
    {
        return (EntityCountSample::operator ==(other));
    }

    inline bool operator !=(
            const HeartbeatCountSample& other) const noexcept
    {
        return !(*this == other);
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

    inline bool operator ==(
            const AcknackCountSample& other) const noexcept
    {
        return (EntityCountSample::operator ==(other));
    }

    inline bool operator !=(
            const AcknackCountSample& other) const noexcept
    {
        return !(*this == other);
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

    inline bool operator ==(
            const NackfragCountSample& other) const noexcept
    {
        return (EntityCountSample::operator ==(other));
    }

    inline bool operator !=(
            const NackfragCountSample& other) const noexcept
    {
        return !(*this == other);
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

    inline bool operator ==(
            const GapCountSample& other) const noexcept
    {
        return (EntityCountSample::operator ==(other));
    }

    inline bool operator !=(
            const GapCountSample& other) const noexcept
    {
        return !(*this == other);
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

    inline bool operator ==(
            const DataCountSample& other) const noexcept
    {
        return (EntityCountSample::operator ==(other));
    }

    inline bool operator !=(
            const DataCountSample& other) const noexcept
    {
        return !(*this == other);
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

    inline bool operator ==(
            const PdpCountSample& other) const noexcept
    {
        return (EntityCountSample::operator ==(other));
    }

    inline bool operator !=(
            const PdpCountSample& other) const noexcept
    {
        return !(*this == other);
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

    inline bool operator ==(
            const EdpCountSample& other) const noexcept
    {
        return (EntityCountSample::operator ==(other));
    }

    inline bool operator !=(
            const EdpCountSample& other) const noexcept
    {
        return !(*this == other);
    }

};

/*
 * Time when a DDS Entity discovers another DDS entity
 */
struct DiscoveryTimeSample : TimepointSample
{
    DiscoveryTimeSample()
        : TimepointSample(DataKind::DISCOVERY_TIME)
        , remote_entity(EntityId::invalid())
        , discovered(false)
    {
    }

    inline bool operator ==(
            const DiscoveryTimeSample& other) const noexcept
    {
        return (TimepointSample::operator ==(other) && remote_entity == other.remote_entity &&
               discovered == other.discovered);
    }

    inline bool operator !=(
            const DiscoveryTimeSample& other) const noexcept
    {
        return !(*this == other);
    }

    EntityId remote_entity;

    bool discovered;

};

/*
 * Time when a DDS Entity discovers another DDS entity
 */
struct SampleDatasCountSample : EntityCountSample
{
    SampleDatasCountSample()
        : EntityCountSample(DataKind::SAMPLE_DATAS)
        , sequence_number(0)
    {
    }

    inline bool operator ==(
            const SampleDatasCountSample& other) const noexcept
    {
        return (EntityCountSample::operator ==(other) && sequence_number == other.sequence_number);
    }

    inline bool operator !=(
            const SampleDatasCountSample& other) const noexcept
    {
        return !(*this == other);
    }

    uint64_t sequence_number;

};

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_SAMPLES_HPP_
