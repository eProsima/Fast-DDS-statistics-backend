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

#include <ostream>

#include <fastdds_statistics_backend/fastdds_statistics_backend_dll.h>

namespace eprosima {
namespace statistics_backend {

class FASTDDS_STATISTICS_BACKEND_DllAPI EntityId
{
public:

    /**
     * @brief Instantiate an EntityId. The internal value is set to EntityId::invalid
     */
    EntityId() noexcept;

    /**
     * @brief Instantiate an EntityId from an integer.
     * @param value The value to use as internal value on the EntityId
     */
    EntityId(
            int64_t value) noexcept;

    /**
     * @brief Move constructor
     * @param entity_id The moved EntityId
     */
    EntityId(
            EntityId&& entity_id) noexcept = default;

    /**
     * @brief Copy constructor
     * @param entity_id The copied EntityId
     */
    EntityId(
            const EntityId& entity_id) noexcept = default;

    /**
     * @brief Copy assignment operator
     * @param entity_id The assigned EntityId
     */
    EntityId& operator =(
            const EntityId& entity_id) noexcept = default;

    /**
     * @brief Move assignment operator
     * @param entity_id The assigned EntityId
     */
    EntityId& operator =(
            EntityId&& entity_id) noexcept = default;

    /**
     * @brief Get the EntityId to refer all entities at once
     * @return An ID that refers all entities.
     */
    static EntityId all() noexcept;

    /**
     * @brief Get an invalid EntityId
     * @return An ID that is invalid
     */
    static EntityId invalid() noexcept;

    /**
     * @brief Invalidate an EntityId
     *
     * @post
     *     is_valid() returns false
     */
    void invalidate() noexcept;

    /**
     * @brief Check whether an EntityId is valid
     * @return True if valid, false otherwise
     */
    bool is_valid() const noexcept;

    /**
     * @brief Check whether an EntityId is the ID representing all entities
     * @return True if is ENTITY_ID_ALL, false otherwise
     */
    bool is_all() const noexcept;

    /**
     * @brief Check whether an EntityId is an ID representing one specific entity
     * @return True if it is valid and not ENTITY_ID_ALL, false otherwise
     */
    bool is_valid_and_unique() const noexcept;

    /**
     * @brief Get the internal value of the EntityId
     * @return An int64_t with the representing internal value
     */
    int64_t value() const noexcept;

protected:

    //! The internal value of the EntityId
    int64_t value_;

    //! Invalid EntityID
    static constexpr int64_t ENTITY_ID_INVALID = -1;

    //! EntityId representing all entities
    static constexpr int64_t ENTITY_ID_ALL = -2;
};

/**
 * @brief Serialize an EntityId to std::ostream
 * @param output The output std::ostream
 * @param entity_id The EntityId to serialize
 */
FASTDDS_STATISTICS_BACKEND_DllAPI inline std::ostream& operator <<(
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
FASTDDS_STATISTICS_BACKEND_DllAPI inline bool operator <(
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
FASTDDS_STATISTICS_BACKEND_DllAPI inline bool operator <=(
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
FASTDDS_STATISTICS_BACKEND_DllAPI inline bool operator >(
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
FASTDDS_STATISTICS_BACKEND_DllAPI inline bool operator >=(
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
FASTDDS_STATISTICS_BACKEND_DllAPI inline bool operator ==(
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
FASTDDS_STATISTICS_BACKEND_DllAPI inline bool operator !=(
        const EntityId& entity_id_1,
        const EntityId& entity_id_2)
{
    return entity_id_1.value() != entity_id_2.value();
}

} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_ENTITYID_HPP_
