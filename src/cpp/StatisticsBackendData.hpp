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
 * @file StatisticsBackendData.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATA_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATA_HPP_

#include <map>
#include <string>

#include <fastdds-statistics-backend/listener/DomainListener.hpp>
#include <fastdds-statistics-backend/listener/PhysicalListener.hpp>
#include <fastdds-statistics-backend/listener/CallbackMask.hpp>
#include <fastdds-statistics-backend/types/types.hpp>
#include <fastdds-statistics-backend/types/EntityId.hpp>

#include <database/database.hpp>
#include "Monitor.hpp"

namespace eprosima {
namespace statistics_backend {
namespace details {


/**
 * @brief Structure holding all the detailed state of the backend.
 */
class StatisticsBackendData
{
public:

    //! Reference to the Database
    std::unique_ptr<database::Database> database_;

    //! Collection of monitors
    std::map<EntityId, std::unique_ptr<Monitor>> monitors_;

    //! Physical  listener
    PhysicalListener* physical_listener_;

    //! Mask for the physical listener
    CallbackMask physical_callback_mask_;

    //! Data mask for the physical listener
    DataKindMask physical_data_mask_;

    //! Status for the Hosts
    DomainListener::Status host_status_;

    //! Status for the Users
    DomainListener::Status user_status_;

    //! Status for the Processes
    DomainListener::Status process_status_;

    //! Status for the Locators
    DomainListener::Status locator_status_;

    /**
     * @brief Get the singleton instance object
     *
     * @return Raw pointer to the singleton instance
     */
    static StatisticsBackendData* get_instance()
    {
        static StatisticsBackendData instance;
        return &instance;
    }

    /**
     * @brief Specifies the reason of calling the entity discovery methods
     *
     */
    enum DiscoveryStatus
    {
        DISCOVERY,      ///< The entity was discovered
        UNDISCOVERY,    ///< The entity was undiscovered
        UPDATE          ///< The entity was updated
    };

    /**
     * @brief Notify the user about a new discovered entity
     *
     * @param domain_id The domain where the entity was discovered
     * @param entity_id The entity_id of the discovered entity
     * @param entity_kind EntityKind of the discovery event
     * @param discovery_status The reason why the method is being called
     */
    void on_domain_entity_discovery(
            EntityId domain_id,
            EntityId entity_id,
            EntityKind entity_kind,
            DiscoveryStatus discovery_status);

    /**
     * @brief Notify the user about a new discovered entity
     *
     * There is no DiscoveryStatus parameter because physical entities are never undiscovered nor updated
     *
     * @param participant_id Entity ID of the participant that discovered the entity.
     * @param entity_id The entity_id of the discovered entity
     * @param entity_kind EntityKind of the discovery event
     */
    void on_physical_entity_discovery(
            EntityId participant_id,
            EntityId entity_id,
            EntityKind entity_kind);

    /**
     * @brief Notify the user about a new available data
     *
     * @param domain_id The domain where the data is available
     * @param entity_id The entity for which the new data is available
     * @param data_kind The DataKind of the new available data
     */
    void on_data_available(
            EntityId domain_id,
            EntityId entity_id,
            DataKind data_kind);

protected:

    /**
     * @brief Protected constructor of the singleton
     */
    StatisticsBackendData()
        : database_(new database::Database)
        , physical_listener_(nullptr)
    {
    }

    /**
     * @brief Check whether the domain listener should be called given the arguments
     *
     * The result will be true if all of the following conditions are met:
     * - The \c monitor has a non-null listener
     * - The callback mask of the \c monitor has the \c callback_kind bit set
     * - The \c data_kind is not INVALID or the data mask of the \c monitor has the \c data_kind bit set
     *
     * When the method is called on the discovery of an entity, \c data_kind = INVALID
     * should be used, signalling that the value of the data mask set by the user is irrelevant in this case.
     *
     * @param monitor The monitor whose domain listener we are testing
     * @param callback_kind The callback kind to check against the callback kind mask
     * @param data_kind The data kind to check against the data kind mask
     * @return true if the listener should be called. False otherwise.
     */
    bool should_call_domain_listener(
            std::unique_ptr<Monitor>& monitor,
            CallbackKind callback_kind,
            DataKind data_kind = DataKind::INVALID);

    /**
     * @brief Check whether the physical listener should be called given the arguments
     *
     * The result will be true if all of the following conditions are met:
     * - There is a non-null physical listener
     * - The callback mask of the physical listener has the \c callback_kind bit set
     * - The \c data_kind is not INVALID or the data mask of the physical listener has the \c data_kind bit set
     *
     * When the method is called on the discovery of an entity, \c data_kind = INVALID
     * should be used, signalling that the value of the data mask set by the user is irrelevant in this case.
     *
     * @param callback_kind The callback kind to check against the callback kind mask
     * @param data_kind The data kind to check against the data kind mask
     * @return true if the listener should be called. False otherwise.
     */
    bool should_call_physical_listener(
            CallbackKind callback_kind,
            DataKind data_kind = DataKind::INVALID);

    /**
     * @brief Updates the given status before calling a user listener
     *
     * @param discovery_status Either DISCOVERY or UNDISCOVERY or UPDATE
     * @param status The status to update
     */
    void prepare_entity_discovery_status(
            DiscoveryStatus discovery_status,
            DomainListener::Status& status);
};

} // namespace details
} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATA_HPP_
