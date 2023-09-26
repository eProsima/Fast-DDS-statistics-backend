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
 * Class that contains a list of elements (StatisticsSample) sorted by timestamp.
 *
 * This class helps to iterate efficiently over a sorted list of elements.
 * It also helps to efficiently remove elements in the front.
 *
 * @note list used instead of vector as \c clear method for front elements is much more efficient.
 *
 * @attention the data must be inserted sorted. This class does not manage the sort of the data.
 *
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
    static_assert(std::is_base_of<database::StatisticsSample, T>::value || std::is_base_of<MonitorServiceSample, T>::value,
            "Type of DataContainer not derived from database::StatisticsSample or database::MonitorServiceSample");

public:

    /**
     * @brief Access operator to the list
     *
     * @param index index of the element to look for
     * @return reference to the element
     *
     * @throws \c out_of_range if index higher than list size.
     *
     * @note thanks to chat gpt
     */
    T& operator [](
            std::size_t index)
    {
        // Check if the index is valid
        if (index >= this->size())
        {
            throw std::out_of_range("Index out of range in DataContainer list.");
        }

        // Traverse the list to the desired index
        auto it = this->begin();
        std::advance(it, index);

        // Return a reference to the element at the index
        return *it;
    }

    /**
     * @brief const access operator to the list
     *
     * @param index index of the element to look for
     * @return const reference to the element
     *
     * @throws \c out_of_range if index higher than list size.
     *
     * @note thanks to chat gpt
     */
    const T& operator [](
            std::size_t index) const
    {
        // Check if the index is valid
        if (index >= this->size())
        {
            throw std::out_of_range("Index out of range in DataContainer list.");
        }

        // Traverse the list to the desired index
        auto it = this->begin();
        std::advance(it, index);

        // Return a reference to the element at the index
        return *it;
    }

    //! Use map clear function if no arguments given
    using std::list<T>::clear;

    /**
     * @brief Clear internal data that are previous to the time given.
     *
     * @attention data that has timestamp equal to \c t_to will not be removed.
     *
     * @param t_to Minimum time that will be kept in internal containers.
     *
     * @note use \c the_end_of_time as argument to efficiently erase all the internal data.
     */
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

    /**
     * @brief Get the interval limits iterators pointing to the internal data that are between the time limits given.
     *
     * @param t_from Minimum time included in the interval
     * @param t_to Minimum time NOT included in the interval
     * @return Pair with first data inside the limit first, and second the first data higher to the limit.
     */
    std::pair<typename std::list<T>::iterator, typename std::list<T>::iterator> get_interval_limits(
            const Timestamp& t_from,
            const Timestamp& t_to)
    {
        return {find_by_timestamp_(t_from), find_by_timestamp_(t_to)};
    }

protected:

    /**
     * @brief Find the first internal data that is equal or higher than the time given.
     *
     * @param value_to_find timestamp to find
     * @return Iterator to the first data with timestamp equal or higher than \c value_to_find
     */
    typename std::list<T>::iterator find_by_timestamp_(
            const Timestamp& value_to_find)
    {
        return std::find_if(
            this->begin(),
            this->end(),
            [&value_to_find](const T& value)
            {
                return value.src_ts >= value_to_find;
            }
            );
    }

};

} // namespace details
} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_DATACONTAINER_HPP_
