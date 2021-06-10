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
 * @file data_aggregation.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DETAIL_DATA_AGGREGATION_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DETAIL_DATA_AGGREGATION_HPP_

#include <memory>

#include <fastdds_statistics_backend/types/types.hpp>

#include <database/samples.hpp>
#include <detail/data_getters.hpp>

namespace eprosima {
namespace statistics_backend {
namespace detail {

struct IDataAggregator
{
    virtual ~IDataAggregator() = default;

    IDataAggregator(
            uint16_t bins,
            Timestamp t_from,
            Timestamp t_to,
            std::vector<StatisticsData>& returned_data)
        : data_(returned_data)
    {
        prepare_bins(bins, t_from, t_to);
    }

    void add_data(
            IteratorPair& iterators)
    {
        for (auto& it = *iterators.first; it != *iterators.second; ++it)
        {
            Timestamp ts = it.get_timestamp();
            auto index = (ts - data_[0].first) / interval_;
            assert((index >= 0) && static_cast<size_t>(index) < data_.size());
            add_sample(static_cast<size_t>(index), it.get_value());
        }
    }

    virtual void finish()
    {
    }

protected:

    virtual void add_sample(
            size_t index,
            double value) = 0;

    virtual double initial_value() const noexcept
    {
        return std::numeric_limits<double>::quiet_NaN();
    }

    void prepare_bins(
            uint16_t bins,
            Timestamp t_from,
            Timestamp t_to)
    {
        interval_ = t_to - t_from;
        interval_ /= bins;

        do
        {
            data_.emplace_back(t_from, initial_value());
            t_from += interval_;
            bins--;
        } while (bins > 0);
    }

    std::vector<StatisticsData>& data_;
    Timestamp::duration interval_;
};

struct CountAggregator final : public IDataAggregator
{
    CountAggregator(
            uint16_t bins,
            Timestamp t_from,
            Timestamp t_to,
            std::vector<StatisticsData>& returned_data)
        : IDataAggregator(bins, t_from, t_to, returned_data)
    {
    }

protected:

    double initial_value() const noexcept
    {
        return 0;
    }

    void add_sample(
            size_t index,
            double value)
    {
        static_cast<void>(value);
        data_[index].second++;
    }

};

} // namespace detail

std::unique_ptr<detail::IDataAggregator> get_data_aggregator(
        uint16_t bins,
        Timestamp t_from,
        Timestamp t_to,
        StatisticKind statistic,
        std::vector<StatisticsData>& returned_data)
{
    // TODO(Miguel C): Return aggregator depending on statistic
    static_cast<void>(statistic);

    detail::IDataAggregator* ret_val = new detail::CountAggregator(bins, t_from, t_to, returned_data);
    return std::unique_ptr<detail::IDataAggregator>(ret_val);
}

} // namespace statistics_backend
} // namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DETAIL_DATA_AGGREGATION_HPP_
