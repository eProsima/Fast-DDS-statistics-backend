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

#include <ostream>

namespace eprosima {
namespace statistics_backend {

class EntityId
{
public:

    /**
     * @brief Instantiate an EntityId. The internal value is set to EntityId::invalid
     */
    EntityId();

    /**
     * @brief Instantiate an EntityId from an integer.
     * @param value The value to use as internal value on the EntityId
     */
    EntityId(
            int64_t value);

    /**
     * @brief Get the EntityId to refer all entities at once
     * @return An ID that refers all entities.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI static EntityId all();

    /**
     * @brief Get an invalid EntityId
     * @return An ID that is invalid
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI static EntityId invalid();

    /**
     * @brief Invalidate an EntityId
     *
     * @post
     *     is_valid() returns false
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI void invalidate();

    /**
     * @brief Check whether an EntityId is valid
     * @return True if valid, false otherwise
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI bool is_valid();

    /**
     * @brief Get the internal value of the EntityId
     * @return An int64_t with the representing internal value
     */
    int64_t value() const;

protected:

    //! The internal value of the EntityId
    int64_t value_;

    //! Invalid EntityID
    static constexpr int64_t ENTITY_ID_INVALID = -1;

    //! EntityId representing all entities
    static constexpr int64_t ENTITY_ID_ALL = 0;
};

/**
 * @brief Serialize an EntityId to std::ostream
 * @param output The output std::ostream
 * @param entity_id The EntityId to serialize
 */
inline std::ostream& operator <<(
        std::ostream& output,
        const EntityId& entity_id)
{
    return output << entity_id.value();
}

/**
 * @brief Check whether an EntityId is smaller than another one
 * @param entity_id_1 The left-side of the operation
 * @param entity_id_2 The right-side of the operation
 */
inline bool operator <(
        const EntityId& entity_id_1,
        const EntityId& entity_id_2)
{
    return entity_id_1.value() < entity_id_2.value();
}

/**
 * @brief Check whether an EntityId is smaller or equal than another one
 * @param entity_id_1 The left-side of the operation
 * @param entity_id_2 The right-side of the operation
 */
inline bool operator <=(
        const EntityId& entity_id_1,
        const EntityId& entity_id_2)
{
    return entity_id_1.value() <= entity_id_2.value();
}

/**
 * @brief Check whether an EntityId is greater than another one
 * @param entity_id_1 The left-side of the operation
 * @param entity_id_2 The right-side of the operation
 */
inline bool operator >(
        const EntityId& entity_id_1,
        const EntityId& entity_id_2)
{
    return entity_id_1.value() > entity_id_2.value();
}

/**
 * @brief Check whether an EntityId is greater or equal than another one
 * @param entity_id_1 The left-side of the operation
 * @param entity_id_2 The right-side of the operation
 */
inline bool operator >=(
        const EntityId& entity_id_1,
        const EntityId& entity_id_2)
{
    return entity_id_1.value() >= entity_id_2.value();
}

/**
 * @brief Check whether an EntityId is equal to another one
 * @param entity_id_1 The left-side of the operation
 * @param entity_id_2 The right-side of the operation
 */
inline bool operator ==(
        const EntityId& entity_id_1,
        const EntityId& entity_id_2)
{
    return entity_id_1.value() == entity_id_2.value();
}

/**
 * @brief Check whether an EntityId is different than another one
 * @param entity_id_1 The left-side of the operation
 * @param entity_id_2 The right-side of the operation
 */
inline bool operator !=(
        const EntityId& entity_id_1,
        const EntityId& entity_id_2)
{
    return entity_id_1.value() != entity_id_2.value();
}

} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_ENTITYID_HPP_
