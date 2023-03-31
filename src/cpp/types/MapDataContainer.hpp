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
 * @file MapDataContainer.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_MAPDATACONTAINER_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_MAPDATACONTAINER_HPP_

#include <list>

#include <fastdds_statistics_backend/exception/Exception.hpp>

#include <database/samples.hpp>
#include <types/DataContainer.hpp>

namespace eprosima {
namespace statistics_backend {
namespace details {

/**
 * TODO
 * @todo comment
 */
template <typename K, typename T>
class MapDataContainer : public std::map<K, DataContainer<T>>
{
    // This class only could be used with T types derived from \c StatisticsSample
    static_assert(std::is_base_of<database::StatisticsSample, T>::value, "Type of MapDataContainer not derived from database::StatisticsSample");

public:

    using std::map<K, DataContainer<T>>::clear;
    void clear(
            const Timestamp& t_to)
    {
        if (t_to == the_end_of_time())
        {
            this->clear();
        }
        else
        {
            // Remove internal data and in same loop
            // Remove those internals lists that become empty
            // NOTE: unable to use remove_if
            auto end = this->end();
            for(auto iter = this->begin(); iter != end;)
            {
                iter->second.clear(t_to);
                if(iter->second.empty())
                {
                    iter = this->erase(iter);
                }
                else
                {
                    ++iter;
                }
            }
        }
    }
};

} // namespace details
} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_MAPDATACONTAINER_HPP_
