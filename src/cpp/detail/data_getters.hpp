// Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

/**
 * @file data_getters.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DETAIL_DATA_GETTERS_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DETAIL_DATA_GETTERS_HPP_

#include <algorithm>  // std::max
#include <memory>     // std::unique_ptr
#include <vector>     // std::vector
#include <utility>    // std::pair

#include <database/samples.hpp>

namespace eprosima {
namespace statistics_backend {
namespace detail {

using base_iterator_type = typename std::vector<const database::StatisticsSample*>::const_iterator;

/**
 * @brief A const_iterator for statistics samples obtained from the database select operation.
 *
 * Provides adapter interfaces to get the timestamp and statistic (double) value of the specific database
 * sample.
 */
struct GenericIterator : public base_iterator_type
{
    explicit GenericIterator(
            const base_iterator_type& plain_iterator) noexcept
        : base_iterator_type(plain_iterator)
    {
    }

    /**
     * @brief Get the timestamp of the sample currently pointed to by this iterator.
     * @return Timestamp of sample.
     */
    Timestamp get_timestamp() const noexcept
    {
        return (**this)->src_ts;
    }

    /**
     * @brief Get the statistic value of the sample currently pointed to by this iterator.
     * @return Statistic value of sample.
     */
    virtual double get_value() const noexcept = 0;

protected:

    /// An utility method to get a reference to a specific child of the sample
    template<typename T>
    const T& sample() const noexcept
    {
        const database::StatisticsSample* ptr = **this;
        return *static_cast<const T*>(ptr);
    }

};

/// Basic template which provides a concrete specialization of the reference getter.
template<typename T>
struct BasicStatisticsIterator : public GenericIterator
{
    explicit BasicStatisticsIterator(
            const base_iterator_type& plain_iterator) noexcept
        : GenericIterator(plain_iterator)
    {
    }

protected:

    const T& sample() const noexcept
    {
        return GenericIterator::sample<T>();
    }

};

/// Final template that should be specialized for every kind of database sample
template<typename T>
struct StatisticsIterator final : public BasicStatisticsIterator<T>
{
    explicit StatisticsIterator(
            const base_iterator_type& plain_iterator) noexcept
        : BasicStatisticsIterator<T>(plain_iterator)
    {
    }

};

/// Iterator returned for EntityDataSample
template<>
struct StatisticsIterator<database::EntityDataSample> final
    : public BasicStatisticsIterator<database::EntityDataSample>
{
    explicit StatisticsIterator(
            const base_iterator_type& plain_iterator) noexcept
        : BasicStatisticsIterator<database::EntityDataSample>(plain_iterator)
    {
    }

    double get_value() const noexcept override
    {
        return sample().data;
    }

};

/// Iterator returned for EntityCountSample
template<>
struct StatisticsIterator<database::EntityCountSample> final
    : public BasicStatisticsIterator<database::EntityCountSample>
{
    explicit StatisticsIterator(
            const base_iterator_type& plain_iterator) noexcept
        : BasicStatisticsIterator<database::EntityCountSample>(plain_iterator)
    {
    }

    double get_value() const noexcept override
    {
        return sample().count;
    }

};

/// Iterator returned for ByteCountSample
template<>
struct StatisticsIterator<database::ByteCountSample> final
    : public BasicStatisticsIterator<database::ByteCountSample>
{
    explicit StatisticsIterator(
            const base_iterator_type& plain_iterator) noexcept
        : BasicStatisticsIterator<database::ByteCountSample>(plain_iterator)
    {
    }

    double get_value() const noexcept override
    {
        double order = (std::max)(sample().magnitude_order, static_cast<int16_t>(0));
        return static_cast<double>(sample().count) + pow(2, 64) * order;
    }

};

/// Iterator returned for TimepointSample
template<>
struct StatisticsIterator<database::TimepointSample> final
    : public BasicStatisticsIterator<database::TimepointSample>
{
    explicit StatisticsIterator(
            const base_iterator_type& plain_iterator) noexcept
        : BasicStatisticsIterator<database::TimepointSample>(plain_iterator)
    {
    }

    double get_value() const noexcept override
    {
        // TODO(Miguel C): How should we treat DISCOVERY_TIME?
        return 0;
    }

};

/// Iterator returned for PublicationThroughputSample
template<>
struct StatisticsIterator<database::PublicationThroughputSample> final
    : public BasicStatisticsIterator<database::PublicationThroughputSample>
{
    explicit StatisticsIterator(
            const base_iterator_type& plain_iterator) noexcept
        : BasicStatisticsIterator<database::PublicationThroughputSample>(plain_iterator)
    {
    }

    double get_value() const noexcept override
    {
        auto aux_prev = *this - 1;
        auto& prev = reinterpret_cast<decltype(*this)>(aux_prev);
        double diff = sample().data - prev.sample().data;
        auto diff_time = get_timestamp() - prev.get_timestamp();
        return diff / diff_time.count();
    }

};

/// Iterator returned for SubscriptionThroughputSample
template<>
struct StatisticsIterator<database::SubscriptionThroughputSample> final
    : public BasicStatisticsIterator<database::SubscriptionThroughputSample>
{
    explicit StatisticsIterator(
            const base_iterator_type& plain_iterator) noexcept
        : BasicStatisticsIterator<database::SubscriptionThroughputSample>(plain_iterator)
    {
    }

    double get_value() const noexcept override
    {
        auto aux_prev = *this - 1;
        auto& prev = reinterpret_cast<decltype(*this)>(aux_prev);
        double diff = sample().data - prev.sample().data;
        auto diff_time = get_timestamp() - prev.get_timestamp();
        return diff / diff_time.count();
    }

};

using IteratorPtr = std::unique_ptr<GenericIterator>;
using IteratorPair = std::pair<IteratorPtr, IteratorPtr>;

/**
 * @brief Get the iterators delimiting the traversal of a database select() result.
 * @param data The collection returned by the database select() call.
 * @return A pair of unique pointers to GenericIterator, with the begin and end iterators.
 */
template<typename T>
IteratorPair get_iterators(
        const std::vector<const database::StatisticsSample*>& data)
{
    return IteratorPair
           {
               new StatisticsIterator<T>(data.cbegin()),
               new StatisticsIterator<T>(data.cend())
           };
}

/**
 * @brief Get the iterators delimiting the traversal of a database select() result for throughput types.
 * @param data The collection returned by the database select() call.
 * @return A pair of unique pointers to GenericIterator, with the begin and end iterators.
 */
template<typename T>
IteratorPair get_throughput_iterators(
        const std::vector<const database::StatisticsSample*>& data)
{
    // As throughput iterators need access to the previous value, the first sample is ignored
    auto begin = data.cbegin();
    if (data.size() > 0)
    {
        ++begin;
    }

    return IteratorPair
           {
               new StatisticsIterator<T>(data.cbegin()),
               new StatisticsIterator<T>(data.cend())
           };
}

// Specialization of get_iterators for PublicationThroughputSample
template<>
IteratorPair get_iterators<database::PublicationThroughputSample>(
        const std::vector<const database::StatisticsSample*>& data)
{
    return get_throughput_iterators<database::PublicationThroughputSample>(data);
}

// Specialization of get_iterators for SubscriptionThroughputSample
template<>
IteratorPair get_iterators<database::SubscriptionThroughputSample>(
        const std::vector<const database::StatisticsSample*>& data)
{
    return get_throughput_iterators<database::SubscriptionThroughputSample>(data);
}

} // namespace detail

/**
 * @brief Get the iterators delimiting the traversal of a database select() result.
 * @param data_type The type of measurement requested to the database select() call.
 * @param data The collection returned by the database select() call.
 * @return A pair of unique pointers to GenericIterator, with the begin and end iterators.
 */
detail::IteratorPair get_iterators(
        DataKind data_type,
        const std::vector<const database::StatisticsSample*>& data)
{
    switch (data_type)
    {
        case DataKind::FASTDDS_LATENCY:
        case DataKind::NETWORK_LATENCY:
            return detail::get_iterators<database::EntityDataSample>(data);

        case DataKind::PUBLICATION_THROUGHPUT:
            return detail::get_iterators<database::PublicationThroughputSample>(data);

        case DataKind::SUBSCRIPTION_THROUGHPUT:
            return detail::get_iterators<database::SubscriptionThroughputSample>(data);

        case DataKind::RTPS_PACKETS_SENT:
        case DataKind::RTPS_PACKETS_LOST:
        case DataKind::RESENT_DATA:
        case DataKind::HEARTBEAT_COUNT:
        case DataKind::ACKNACK_COUNT:
        case DataKind::NACKFRAG_COUNT:
        case DataKind::GAP_COUNT:
        case DataKind::DATA_COUNT:
        case DataKind::PDP_PACKETS:
        case DataKind::EDP_PACKETS:
            return detail::get_iterators<database::EntityCountSample>(data);

        case DataKind::RTPS_BYTES_SENT:
        case DataKind::RTPS_BYTES_LOST:
            return detail::get_iterators<database::ByteCountSample>(data);

        case DataKind::DISCOVERY_TIME:
            return detail::get_iterators<database::TimepointSample>(data);

        case DataKind::SAMPLE_DATAS:
            // TODO(Miguel C): Should be treated different?
            return detail::get_iterators<database::EntityDataSample>(data);

        default:
            throw BadParameter("Unsupported data kind");
    }
}

} // namespace statistics_backend
} // namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DETAIL_DATA_GETTERS_HPP_
