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
 * @file MedianAggregator.ipp
 */

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
