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
 * @file MaximumAggregator.ipp
 */

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
