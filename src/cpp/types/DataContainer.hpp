// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file DataContainer.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_DATACONTAINER_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_DATACONTAINER_HPP_

#include <list>

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/types/utils.hpp>

#include <database/samples.hpp>

namespace eprosima {
namespace statistics_backend {
namespace details {

/**
 * TODO
 * @todo comment
 *
 * TODO
 * @todo this functions could be implemented much more efficiency:
 *  - binary search
 *  - add find_from_back option to accelerate search
 *  - add a helper iterator to no start from the beginning
 * For now we want to reduce implementation time, so lineal search it is
 */
template <typename T>
class DataContainer : public std::list<T>
{
    // This class only could be used with T types derived from \c StatisticsSample
    static_assert(std::is_base_of<database::StatisticsSample, T>::value, "Type of DataContainer not derived from database::StatisticsSample");

public:

    /**
     * @brief
     *
     * @param index
     * @return T&
     *
     * @note thanks to chat gpt
     */
    T& operator[](std::size_t index)
    {
        // Check if the index is valid
        if (index >= this->size()) {
            throw std::out_of_range("Index out of range in DataContainer list.");
        }

        // Traverse the list to the desired index
        auto it = this->begin();
        std::advance(it, index);

        // Return a reference to the element at the index
        return *it;
    }

    const T& operator[](std::size_t index) const
    {
        // Check if the index is valid
        if (index >= this->size()) {
            throw std::out_of_range("Index out of range in DataContainer list.");
        }

        // Traverse the list to the desired index
        auto it = this->begin();
        std::advance(it, index);

        // Return a reference to the element at the index
        return *it;
    }

    using std::list<T>::clear;
    void clear(
            const Timestamp& t_to)
    {
        if (t_to == the_end_of_time())
        {
            this->clear();
        }
        else
        {
            this->erase(this->begin(), find_by_timestamp_(t_to));
        }
    }

    std::pair<typename std::list<T>::iterator, typename std::list<T>::iterator> get_interval_limits(
        const Timestamp& t_from,
        const Timestamp& t_to)
    {
        return {find_by_timestamp_(t_from), find_by_timestamp_(t_to)};
    }

protected:

    typename std::list<T>::iterator find_by_timestamp_(
        const Timestamp& value_to_find)
    {
        return std::find_if(
            this->begin(),
            this->end(),
            [&value_to_find](const T& value)
                { return value.src_ts > value_to_find; }
        );
    }
};

} // namespace details
} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_DATACONTAINER_HPP_
