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

#include <fastdds-statistics-backend/types/EntityId.hpp>

namespace eprosima {
namespace statistics_backend {

EntityId::EntityId()
    : entity_id_("uuid")
{
}

EntityId::EntityId(
        const char* const entity_id)
    : entity_id_(entity_id)
{
}

EntityId EntityId::all()
{
    return EntityId(ENTITY_ID_ALL);
}

EntityId EntityId::invalid()
{
    return EntityId(ENTITY_ID_INVALID);
}

bool EntityId::valid()
{
    return (entity_id_ == ENTITY_ID_INVALID);
}

void EntityId::invalidate()
{
    entity_id_ = ENTITY_ID_INVALID;
}

const std::string& EntityId::str() const
{
    return entity_id_;
}

const char* EntityId::c_str() const
{
    return entity_id_.c_str();
}

bool EntityId::operator <(
        EntityId& entity_id)
{
    return entity_id_ < entity_id.str();
}

bool EntityId::operator <=(
        EntityId& entity_id)
{
    return entity_id_ <= entity_id.str();
}

bool EntityId::operator >(
        EntityId& entity_id)
{
    return entity_id_ > entity_id.str();
}

bool EntityId::operator >=(
        EntityId& entity_id)
{
    return entity_id_ >= entity_id.str();
}

bool EntityId::operator ==(
        EntityId& entity_id)
{
    return entity_id_ == entity_id.str();
}

bool EntityId::operator !=(
        EntityId& entity_id)
{
    return entity_id_ != entity_id.str();
}

} // namespace statistics_backend
} // namespace eprosima
