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
#include <cassert>    // assert
#include <cmath>      // std::isnan, std::sqrt
#include <memory>     // std::unique_ptr
#include <vector>     // std::vector

#include <fastdds_statistics_backend/types/types.hpp>

#include <database/samples.hpp>
#include <detail/data_getters.hpp>

namespace eprosima {
namespace statistics_backend {
namespace detail {

/**
 * Generic interface for aggregating raw statistics data into several bins.
 *
 * It is constructed with the bins configuration and a reference to the resulting
 * data collection.
 *
 * The data returned by the database select() method is processed by the @ref add_data
 * method.
 *
 * To perform final aggregation calculations, method @ref finish should be called.
 */
struct IDataAggregator
{
    virtual ~IDataAggregator() = default;

    /**
     * @brief Construct an IDataAggregator.
     * @param bins           Number of time intervals in which the measurement time is divided
     * @param t_from         Starting time of the returned measures
     * @param t_to           Ending time of the returned measures
     * @param returned_data  Reference to the collection to be returned by @ref StatisticsBackend::get_data
     */
    IDataAggregator(
            uint16_t bins,
            Timestamp t_from,
            Timestamp t_to,
            std::vector<StatisticsData>& returned_data)
        : data_(returned_data)
    {
        assert(bins > 0);
        prepare_bins(bins, t_from, t_to);
    }

    /**
     * @brief Process a collection of data returned by the database.
     * @param iterators Reference to the iterator pair returned by @ref get_iterators
     */
    void add_data(
            IteratorPair& iterators)
    {
        for (auto& it = *iterators.first; it != *iterators.second; ++it)
        {
            // Find and check the bin corresponding to the sample timestamp
            Timestamp ts = it.get_timestamp();
            auto index = (ts - data_[0].first) / interval_;
            assert((index >= 0) && static_cast<size_t>(index) < data_.size());

            // Add sample value to the corresponding bin
            add_sample(static_cast<size_t>(index), it.get_value());
        }
    }

    /**
     * @brief Performs final aggregation of data.
     */
    virtual void finish()
    {
    }

protected:

    /**
     * @brief Add a sample to a specific bin.
     * @param index Index of the bin where the sample should be added.
     * @param value Statistical value to add.
     */
    virtual void add_sample(
            size_t index,
            double value) = 0;

    /**
     * @brief Get the initial aggregation value.
     * @return Initial aggregation value.
     */
    virtual double initial_value() const noexcept
    {
        // All aggregators except @ref CountAggregator will be initialized to quiet_NaN
        return std::numeric_limits<double>::quiet_NaN();
    }

    /**
     * @brief Assign a value to a bin if it was not given a previous value.
     *
     * This method is used on aggregators that need to take special actions the first time
     * a value is added to a bin.
     * For instance, the @ref MaximumAggregator will assign the value the first time, and
     * will perform a comparison with the current value on the following ones.
     *
     * @param index Index of the bin where the sample should be added.
     * @param value Statistical value to assign.
     * @return whether the value has been assigned to the bin.
     */
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

    //! Reference to the collection to be returned by @ref StatisticsBackend::get_data
    std::vector<StatisticsData>& data_;

private:

    void prepare_bins(
            uint16_t bins,
            Timestamp t_from,
            Timestamp t_to)
    {
        // Perform a single allocation for all the returned bins
        data_.reserve(bins);

        // Calculate the duration of each bin
        interval_ = t_to - t_from;
        interval_ /= bins;

        // Fill each bin with the initial value and the initial timestamp of the bin
        double value = initial_value();
        do
        {
            data_.emplace_back(t_from, value);
            t_from += interval_;
            bins--;
        } while (bins > 0);
    }

    //! Duration of each bin
    Timestamp::duration interval_;
};

/// An @ref IDataAggregator that returns the first sample received for each bin
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

/// An @ref IDataAggregator that returns the addition of all the samples added to each bin
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

/**
 * @brief An @ref IDataAggregator that returns the mean value of all the samples added to each bin
 *
 * It behaves as the @ref SumAggregator, but it also keeps track of the number of samples added to each bin.
 * It will then perform the final division inside @ref MeanAggregator::finish.
 */
struct MeanAggregator final : public IDataAggregator
{
    MeanAggregator(
            uint16_t bins,
            Timestamp t_from,
            Timestamp t_to,
            std::vector<StatisticsData>& returned_data)
        : IDataAggregator(bins, t_from, t_to, returned_data)
    {
        // Initialize all sample counts to 0
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
        // Accumulate value
        if (!assign_if_nan(index, value))
        {
            data_[index].second += value;
        }

        // Increase number of samples
        ++num_samples_[index];
    }

private:

    /// Keeps the number of samples added to each bin
    std::vector<size_t> num_samples_;

};

/**
 * @brief An @ref IDataAggregator that returns the median value of all the samples added to each bin
 *
 * It keeps the collection of all the samples received on each bin, and finds the median on
 * @ref MedianAggregator::finish.
 */
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
                // For an odd number of samples, this will give us the index of the middle sample.
                // For an even number of samples, this will give us the first index of the two middle samples.
                size_t index = (n_samples - 1) / 2;

                // We use nth_element to perform a partial ordering of the collection
                auto begin = samples_[n].begin();
                auto end = samples_[n].end();
                auto med = begin;
                std::advance(med, index);
                std::nth_element(begin, med, end);

                // The sample at index is now the median.
                // All samples before index are lower. All samples after index are greater.
                double median = *med;
                if (0 == (n_samples % 2))
                {
                    // Perform partial ordering of the items after index so the first one is the lowest of them.
                    ++med;
                    std::nth_element(med, med, end);

                    // Median is the mean of the two center values
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

    /// Keeps all the values added to each bin
    std::vector<std::vector<double>> samples_;

};

/// An @ref IDataAggregator that returns the maximum of all the samples added to each bin
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

/// An @ref IDataAggregator that returns the minimum of all the samples added to each bin
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

/// An @ref IDataAggregator that returns the number of samples added to each bin
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

/**
 * @brief An @ref IDataAggregator that returns the maximum of all the samples added to each bin.
 *
 * Implements the <a href="https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Na%C3%AFve_algorithm">naïve algorithm</a>
 * keeping the sum of the values, the sum of the squares, and the number of samples.
 */
struct StdDevAggregator final : public IDataAggregator
{
    StdDevAggregator(
            uint16_t bins,
            Timestamp t_from,
            Timestamp t_to,
            std::vector<StatisticsData>& returned_data)
        : IDataAggregator(bins, t_from, t_to, returned_data)
    {
        bin_data_.resize(data_.size());
    }

    void finish() override
    {
        for (size_t n = 0; n < data_.size(); ++n)
        {
            BinData& data = bin_data_[n];
            auto n_samples = data.num_samples;
            if (n_samples >= 2)
            {
                double variance = (data.sum_sq - (data.sum * data.sum) / n_samples) / (n_samples - 1);
                data_[n].second = std::sqrt(variance);
            }
        }
    }

protected:

    void add_sample(
            size_t index,
            double value) override
    {
        BinData& data = bin_data_[index];
        data.sum += value;
        data.sum_sq += (value * value);
        data.num_samples += 1;
    }

private:

    /// Holds the calculation data required for one bin
    struct BinData
    {
        double sum = 0;
        double sum_sq = 0;
        uint64_t num_samples = 0;
    };

    /// Holds the calculation data required for each bin
    std::vector<BinData> bin_data_;

};

} // namespace detail

/**
 * @brief Get the aggregator required for processing a @ref StatisticsBackend::get_data call.
 * @param bins           Number of time intervals in which the measurement time is divided
 * @param t_from         Starting time of the returned measures
 * @param t_to           Ending time of the returned measures
 * @param statistic      Kind of aggregation to perform
 * @param returned_data  Reference to the collection to be returned by @ref StatisticsBackend::get_data
 *
 */
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

        case StatisticKind::STANDARD_DEVIATION:
            ret_val = new detail::StdDevAggregator(bins, t_from, t_to, returned_data);
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
