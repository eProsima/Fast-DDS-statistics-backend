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
 * @file Entity.hpp
 */

#include <fastdds-statistics-backend/types/types.hpp>

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATICMOCKCOMPLEX_ENTITY_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATICMOCKCOMPLEX_ENTITY_HPP_

namespace eprosima {
namespace statistics_backend {

class Entity
{
public:
    Entity(EntityId id, std::string name)
        : id_(id)
        , name_(name)
    {}

    virtual std::vector<EntityId> get_entities(
        const EntityKind entity_type) const;

    virtual Info get_info() const;

    std::string name() const
    {
        return name_;
    }

    EntityId id() const
    {
        return id_;
    }

private:

    const EntityId id_;
    std::string name_;
};

} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATICMOCKCOMPLEX_ENTITY_HPP_
