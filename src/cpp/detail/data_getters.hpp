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

#include "database/samples.hpp"

namespace eprosima {
namespace statistics_backend {
namespace detail {

struct GenericIterator : public std::vector<const database::StatisticsSample*>::const_iterator
{
    using base_iterator_type = typename std::vector<const database::StatisticsSample*>::const_iterator;

    explicit GenericIterator(
            const base_iterator_type& plain_iterator) noexcept
        : base_iterator_type(plain_iterator)
    {
    }

    virtual double get_value() const noexcept = 0;

    template<typename T>
    const T& sample() const noexcept
    {
        const database::StatisticsSample* ptr = **this;
        return *static_cast<const T*>(ptr);
    }

};

template<typename T>
struct BasicStatisticsIterator
    : public GenericIterator
{
    using base_iterator_type = typename std::vector<const T*>::const_iterator;

    explicit BasicStatisticsIterator(
            const base_iterator_type& plain_iterator) noexcept
        : GenericIterator(reinterpret_cast<const GenericIterator::base_iterator_type&>(plain_iterator))
    {
    }

    const T& sample() const noexcept
    {
        return GenericIterator::sample<T>();
    }
};

template<typename T>
struct StatisticsIterator : public BasicStatisticsIterator<T>
{
    using base_iterator_type = typename BasicStatisticsIterator<T>::base_iterator_type;

    explicit StatisticsIterator(
            const base_iterator_type& plain_iterator) noexcept
        : BasicStatisticsIterator<T>(plain_iterator)
    {
    }
};

template<>
struct StatisticsIterator<database::EntityDataSample>
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

template<>
struct StatisticsIterator<database::EntityCountSample>
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

template<>
struct StatisticsIterator<database::ByteCountSample>
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

template<>
struct StatisticsIterator<database::TimepointSample>
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

using IteratorPtr = std::unique_ptr<GenericIterator>;
using IteratorPair = std::pair<IteratorPtr, IteratorPtr>;

template<typename T>
IteratorPair get_iterators(
        const std::vector<const T*>& data)
{
    return IteratorPair
    {
        new StatisticsIterator<T>(data.cbegin()),
        new StatisticsIterator<T>(data.cend())
    };
}

using EntityDataVector = std::vector<const database::EntityDataSample*>;
using EntityCountVector = std::vector<const database::EntityCountSample*>;
using ByteCountVector = std::vector<const database::ByteCountSample*>;
using TimePointVector = std::vector<const database::TimepointSample*>;

} // namespace detail

detail::IteratorPair get_iterators(
        DataKind kind,
        const std::vector<const database::StatisticsSample*>& data)
{
    switch(kind)
    {
        case DataKind::FASTDDS_LATENCY:
        case DataKind::NETWORK_LATENCY:
        case DataKind::PUBLICATION_THROUGHPUT:
        case DataKind::SUBSCRIPTION_THROUGHPUT:
            return detail::get_iterators(reinterpret_cast<const detail::EntityDataVector&>(data));

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
            return detail::get_iterators(reinterpret_cast<const detail::EntityCountVector&>(data));

        case DataKind::RTPS_BYTES_SENT:
        case DataKind::RTPS_BYTES_LOST:
            return detail::get_iterators(reinterpret_cast<const detail::ByteCountVector&>(data));

        case DataKind::DISCOVERY_TIME:
            return detail::get_iterators(reinterpret_cast<const detail::TimePointVector&>(data));

        case DataKind::SAMPLE_DATAS:
            // TODO(Miguel C): Should be treated different?
            return detail::get_iterators(reinterpret_cast<const detail::EntityDataVector&>(data));

        default:
            throw BadParameter("Unsupported data kind");
    }
}


} // namespace statistics_backend
} // namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DETAIL_DATA_GETTERS_HPP_
