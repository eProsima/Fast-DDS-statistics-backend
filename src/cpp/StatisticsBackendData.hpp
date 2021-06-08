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

    StatisticsBackendData()
        : database_(new database::Database)
        , physical_listener_(nullptr)
    {
    }

    bool should_call_domain_listener(
            std::unique_ptr<Monitor>& monitor,
            CallbackKind callback_kind,
            DataKind data_kind = DataKind::INVALID);

    bool should_call_physical_listener(
            CallbackKind callback_kind,
            DataKind data_kind = DataKind::INVALID);

    bool prepare_entity_discovery_status(
            DiscoveryStatus discovery_status,
            DomainListener::Status& status);
};

} // namespace details
} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATA_HPP_
