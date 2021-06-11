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
 * @file StdDevAggregator.ipp
 */

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
