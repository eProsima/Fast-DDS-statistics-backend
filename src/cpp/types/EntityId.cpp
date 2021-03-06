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

#include <fastdds_statistics_backend/types/EntityId.hpp>

namespace eprosima {
namespace statistics_backend {

EntityId::EntityId() noexcept
    : value_(ENTITY_ID_INVALID)
{
}

EntityId::EntityId(
        int64_t value) noexcept
    : value_(value)
{
}

EntityId EntityId::all() noexcept
{
    return EntityId(ENTITY_ID_ALL);
}

EntityId EntityId::invalid() noexcept
{
    return EntityId(ENTITY_ID_INVALID);
}

bool EntityId::is_valid() const noexcept
{
    return (value_ >= 0 || is_all());
}

bool EntityId::is_all() const noexcept
{
    return (value_ == ENTITY_ID_ALL);
}

bool EntityId::is_valid_and_unique() const noexcept
{
    return is_valid() && !is_all();
}

void EntityId::invalidate() noexcept
{
    value_ = ENTITY_ID_INVALID;
}

int64_t EntityId::value() const noexcept
{
    return value_;
}

} // namespace statistics_backend
} // namespace eprosima
