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
 * @file EntityId.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_ENTITYID_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_ENTITYID_HPP_

#include <fastdds-statistics-backend/fastdds_statistics_backend_dll.h>

#include <string>

namespace eprosima {
namespace statistics_backend {

class FASTDDS_STATISTICS_BACKEND_DllAPI EntityId : public std::string
{
public:

    // Inherit std::string constructors
    using std::string::string;

    /**
     * @brief Return the EntityId to refer all entities at once
     * @return An ID that refers all entities.
     */
    static EntityId all()
    {
        return EntityId(ENTITY_ID_ALL);
    }

private:
    static constexpr const char * ENTITY_ID_ALL = "*";
};

} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_ENTITYID_HPP_
