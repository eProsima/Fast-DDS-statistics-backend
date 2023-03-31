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
     * @param initial_value  Value with which to initialize the contents of returned_data
     */
    IDataAggregator(
            uint16_t bins,
            Timestamp t_from,
            Timestamp t_to,
            std::vector<StatisticsData>& returned_data,
            double initial_value = std::numeric_limits<double>::quiet_NaN())
        : data_(returned_data)
    {
        assert(bins > 0);
        prepare_bins(bins, t_from, t_to, initial_value);
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
            Timestamp ts = it.get_the_beginning_of_time() + interval_;
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
            Timestamp t_to,
            double initial_value)
    {
        // Perform a single allocation for all the returned bins
        data_.reserve(bins);

        // Calculate the duration of each bin
        interval_ = t_to - t_from;
        interval_ /= bins;

        // Fill each bin with the initial value and the initial timestamp of the bin
        do
        {
            t_from += interval_;
            data_.emplace_back(t_from, initial_value);
            bins--;
        } while (bins > 0);
    }

    //! Duration of each bin
    Timestamp::duration interval_;
};

#include "data_aggregators/CountAggregator.ipp"
#include "data_aggregators/MaximumAggregator.ipp"
#include "data_aggregators/MeanAggregator.ipp"
#include "data_aggregators/MedianAggregator.ipp"
#include "data_aggregators/MinimumAggregator.ipp"
#include "data_aggregators/NoneAggregator.ipp"
#include "data_aggregators/StdDevAggregator.ipp"
#include "data_aggregators/SumAggregator.ipp"

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

        case StatisticKind::STANDARD_DEVIATION:
            ret_val = new detail::StdDevAggregator(bins, t_from, t_to, returned_data);
            break;

        case StatisticKind::COUNT:
        default:
            ret_val = new detail::CountAggregator(bins, t_from, t_to, returned_data);
            break;
    }

    return std::unique_ptr<detail::IDataAggregator>(ret_val);
}

} // namespace statistics_backend
} // namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DETAIL_DATA_AGGREGATION_HPP_
