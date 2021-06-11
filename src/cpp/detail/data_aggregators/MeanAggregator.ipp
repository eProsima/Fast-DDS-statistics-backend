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
 * @file MeanAggregator.ipp
 */

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
