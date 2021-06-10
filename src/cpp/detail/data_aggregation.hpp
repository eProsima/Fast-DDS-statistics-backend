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

#include <algorithm>  // std::min, std::max, std::nth_element
#include <cmath>      // std::isnan
#include <memory>     // std::unique_ptr
#include <vector>     // std::vector

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

    bool assign_if_nan(
            size_t index,
            double value)
    {
        if (std::isnan(data_[index].second))
        {
            data_[index].second = value;
            return true;
        }

        return false;
    }

    std::vector<StatisticsData>& data_;

private:

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

    Timestamp::duration interval_;
};

struct NoneAggregator final : public IDataAggregator
{
    NoneAggregator(
            uint16_t bins,
            Timestamp t_from,
            Timestamp t_to,
            std::vector<StatisticsData>& returned_data)
        : IDataAggregator(bins, t_from, t_to, returned_data)
    {
    }

protected:

    void add_sample(
            size_t index,
            double value) override
    {
        assign_if_nan(index, value);
    }

};

struct SumAggregator final : public IDataAggregator
{
    SumAggregator(
            uint16_t bins,
            Timestamp t_from,
            Timestamp t_to,
            std::vector<StatisticsData>& returned_data)
        : IDataAggregator(bins, t_from, t_to, returned_data)
    {
    }

protected:

    void add_sample(
            size_t index,
            double value) override
    {
        if (!assign_if_nan(index, value))
        {
            data_[index].second += value;
        }
    }

};

struct MeanAggregator final : public IDataAggregator
{
    MeanAggregator(
            uint16_t bins,
            Timestamp t_from,
            Timestamp t_to,
            std::vector<StatisticsData>& returned_data)
        : IDataAggregator(bins, t_from, t_to, returned_data)
    {
        num_samples_.assign(data_.size(), 0);
    }

    void finish() override
    {
        for (size_t n = 0; n < data_.size(); ++n)
        {
            if (0 != num_samples_[n])
            {
                data_[n].second /= num_samples_[n];
            }
        }
    }

protected:

    void add_sample(
            size_t index,
            double value) override
    {
        if (!assign_if_nan(index, value))
        {
            data_[index].second += value;
        }
    }

private:

    std::vector<size_t> num_samples_;

};

struct MedianAggregator final : public IDataAggregator
{
    MedianAggregator(
            uint16_t bins,
            Timestamp t_from,
            Timestamp t_to,
            std::vector<StatisticsData>& returned_data)
        : IDataAggregator(bins, t_from, t_to, returned_data)
    {
        samples_.resize(data_.size());
    }

    void finish() override
    {
        for (size_t n = 0; n < data_.size(); ++n)
        {
            size_t n_samples = samples_[n].size();
            if (n_samples > 0)
            {
                size_t index = (n_samples - 1) / 2;
                auto begin = samples_[n].begin();
                auto end = samples_[n].end();
                auto med = begin;
                std::advance(med, index);
                std::nth_element(begin, med, end);
                double median = *med;
                if (0 == (n_samples % 2))
                {
                    ++med;
                    std::nth_element(med, med, end);
                    median = (median + *med) / 2;
                }

                data_[n].second = median;
            }
        }
    }

protected:

    void add_sample(
            size_t index,
            double value) override
    {
        samples_[index].push_back(value);
    }

private:

    std::vector<std::vector<double>> samples_;

};

struct MaximumAggregator final : public IDataAggregator
{
    MaximumAggregator(
            uint16_t bins,
            Timestamp t_from,
            Timestamp t_to,
            std::vector<StatisticsData>& returned_data)
        : IDataAggregator(bins, t_from, t_to, returned_data)
    {
    }

protected:

    void add_sample(
            size_t index,
            double value) override
    {
        if (!assign_if_nan(index, value))
        {
            data_[index].second = (std::max)(value, data_[index].second);
        }
    }

};

struct MinimumAggregator final : public IDataAggregator
{
    MinimumAggregator(
            uint16_t bins,
            Timestamp t_from,
            Timestamp t_to,
            std::vector<StatisticsData>& returned_data)
        : IDataAggregator(bins, t_from, t_to, returned_data)
    {
    }

protected:

    void add_sample(
            size_t index,
            double value) override
    {
        if (!assign_if_nan(index, value))
        {
            data_[index].second = (std::min)(value, data_[index].second);
        }
    }

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
            double value) override
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
    detail::IDataAggregator* ret_val = nullptr;

    switch (statistic)
    {
        case StatisticKind::NONE:
            ret_val = new detail::NoneAggregator(bins, t_from, t_to, returned_data);
            break;

        case StatisticKind::SUM:
            ret_val = new detail::SumAggregator(bins, t_from, t_to, returned_data);
            break;

        case StatisticKind::MEAN:
            ret_val = new detail::MeanAggregator(bins, t_from, t_to, returned_data);
            break;

        case StatisticKind::MEDIAN:
            ret_val = new detail::MedianAggregator(bins, t_from, t_to, returned_data);
            break;

        case StatisticKind::MAX:
            ret_val = new detail::MaximumAggregator(bins, t_from, t_to, returned_data);
            break;

        case StatisticKind::MIN:
            ret_val = new detail::MinimumAggregator(bins, t_from, t_to, returned_data);
            break;

        case StatisticKind::COUNT:
            ret_val = new detail::CountAggregator(bins, t_from, t_to, returned_data);
            break;

        default:
            ret_val = new detail::CountAggregator(bins, t_from, t_to, returned_data);
            break;
    }

    return std::unique_ptr<detail::IDataAggregator>(ret_val);
}

} // namespace statistics_backend
} // namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DETAIL_DATA_AGGREGATION_HPP_
