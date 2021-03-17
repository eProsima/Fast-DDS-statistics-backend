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

#include <types/types.hpp>

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATICMOCKCOMPLEX_ENTITY_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATICMOCKCOMPLEX_ENTITY_HPP_

namespace eprosima {
namespace statistics_backend {

class EntityCollector
{
public:

    static EntityCollector* get_instance()
    {
        static EntityCollector instance;
        return &instance;
    }

protected:

    EntityCollector();

    ~EntityCollector();
};

class Entity
{
public:
    Entity(EntityId id, std::string name)
        : id_(id)
        , name_(name)
    {}

    virtual std::vector<EntityId> get_entities(
        EntityId entity_id,
        EntityKind entity_type);

    virtual Qos get_qos(
        EntityId entity_id);

    std::string get_name(
            EntityId entity_id)
    {
        return name_;
    }

    // Variables
    EntityId id_;
    std::string name_;
};

class Host : public Entity
{};

class User : public Entity
{};

class Process : public Entity
{};

} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATICMOCKCOMPLEX_ENTITY_HPP_
