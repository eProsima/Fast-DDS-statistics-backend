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
 * @file StatisticsBackend.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATISTICSBACKEND_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATISTICSBACKEND_HPP_

#include <chrono>
#include <string>

#include <fastdds-statistics-backend/fastdds_statistics_backend_dll.h>
#include <fastdds-statistics-backend/listener/DomainListener.hpp>
#include <fastdds-statistics-backend/listener/PhysicalListener.hpp>
#include <fastdds-statistics-backend/listener/CallbackMask.hpp>
#include <fastdds-statistics-backend/types/types.hpp>
#include <fastdds-statistics-backend/types/EntityId.hpp>

namespace eprosima {
namespace statistics_backend {

class FASTDDS_STATISTICS_BACKEND_DllAPI StatisticsBackend
{

public:

    /**
     * @brief Deleted constructor, since the whole interface is static
     */
    StatisticsBackend() = delete;

    /**
     * @brief Set the listener for the physical domain events.
     *
     * Any physical listener already configured will be replaced by the new one.
     * The provided pointer to the listener can be null, in which case,
     * any physical listener already configured will be removed.
     *
     * @param listener the listener with the callback implementations.
     * @param callback_mask Mask of the callbacks. Only the events that have the mask bit set will be informed.
     * @param data_mask Mask of the data types that will be monitored
     */
    static void set_physical_listener(
            PhysicalListener* listener,
            CallbackMask callback_mask = CallbackMask::all(),
            DataKindMask data_mask = DataKindMask::none());

    /**
     * @brief Starts monitoring on a given domain
     *
     * This function creates a new statistics DomainParticipant that starts monitoring
     * the requested domain ID.
     *
     * @param domain The domain ID of the DDS domain to monitor
     * @param domain_listener Listener with the callback to use to inform of events
     * @param callback_mask Mask of the callbacks. Only the events that have the mask bit set will be informed
     * @param data_mask Mask of the data types that will be monitored
     * @return The ID of the created statistics DomainParticipant.
     */
    static EntityId init_monitor(
            DomainId domain,
            DomainListener* domain_listener = nullptr,
            CallbackMask callback_mask = CallbackMask::all(),
            DataKindMask data_mask = DataKindMask::none());

    /**
     * @brief Starts monitoring the domain corresponding to a server
     *
     * This function creates a new statistics DomainParticipant that starts monitoring
     * the domain of the server with the given locator.
     *
     * @param discovery_server_locators The locator of the server whose domain is to be monitored, formatted as "IPV4address:port"
     * @param domain_listener Listener with the callback to use to inform of events
     * @param callback_mask Mask of the callbacks. Only the events that have the mask bit set will be informed
     * @param data_mask Mask of the data types that will be monitored
     * @return The ID of the created statistics DomainParticipant.
     */
    static EntityId init_monitor(
            std::string discovery_server_locators,
            DomainListener* domain_listener = nullptr,
            CallbackMask callback_mask = CallbackMask::all(),
            DataKindMask data_mask = DataKindMask::none());

    /**
     * @brief Restarts a given monitor
     *
     * This function restarts a domain monitor. If the monitor is still active (meaning it has not
     * being stopped), this function takes no effect.
     *
     * @param monitor_id The entity ID of the monitor to restart.
     */
    static void restart_monitor(
            EntityId monitor_id);

    /**
     * @brief Stops a given monitor
     *
     * This function stops a domain monitor. After stopping, the statistical data related to the
     * domain is still accessible.
     *
     * @param monitor_id The entity ID of the monitor to stop.
     */
    static void stop_monitor(
            EntityId monitor_id);

    /**
     * @brief Clear the data of a domain given its monitor
     *
     * This function clears all the data related to a domain given its monitor ID.
     * If the monitor is still active (meaning it has not being stopped), this functions takes no
     * effect. After clearing, the statistical data related to the domain is deleted and therefore
     * no longer accessible.
     *
     * @param monitor_id The entity ID of the monitor to stop.
     */
    static void clear_monitor(
            EntityId monitor_id);

    /**
     * @brief Set the listener of a monitor for the domain events.
     *
     * Any domain listener already configured will be replaced by the new one.
     * The provided pointer to the listener can be null, in which case,
     * any domain listener already configured will be removed.
     *
     * @param monitor_id The entity ID of the monitor.
     * @param listener the listener with the callback implementations.
     * @param callback_mask Mask of the callbacks. Only the events that have the mask bit set will be informed.
     * @param data_mask Mask of the data types that will be monitored
     */
    static void set_domain_listener(
            EntityId monitor_id,
            DomainListener* listener = nullptr,
            CallbackMask callback_mask = CallbackMask::all(),
            DataKindMask data_mask = DataKindMask::none());

    /**
     * @brief Get all the entities of a given type related to another entity
     *
     * Get all the entity ids for every entity of kind \c entity_type that is connected with entity \c entity_id
     * Connection between entities means they are directly connected by a contained/connect relation
     * (i.e. Host - User | Domain - Topic) or that connected entities are connected to it
     *
     * Use case: To get all host in the system, use arguments HOST and EntityId::all()
     * Use case: To get all locators from a participant with id X, use arguments LOCATOR and X, this will
     *  get all the locators that are connected with the endpoints this participant has.
     *
     * In case the \c entity_id is not specified, all entities of type \c entity_type are returned
     *
     * @param entity_type The type of entities for which the search is performed
     * @param entity_id The ID of the entity to which the resulting entities are related
     * @throws eprosima::statistics_backend::BadParameter in the following case:
     *                  * if the \c entity_kind is \c INVALID
     *                  * if the \c entity_id does not reference a entity contained in the database or is not EntityId::all().
     *                  * if the EntityKind of the Entity with \c entity_id is \c INVALID
     * @return All entities of type \c entity_type that are related to \c entity_id
     */
    static std::vector<EntityId> get_entities(
            EntityKind entity_type,
            EntityId entity_id = EntityId::all());

    /**
     * @brief Returns whether the entity is active.
     *
     * For monitors, active means that no call to stop_monitor() has been performed since the last
     * time the monitor was activated. For the rest of entities, active means that there is
     * statistical data being reported within the entity.
     *
     * @param entity_id The ID of the entity whose activeness is requested
     * @return true if active, false otherwise.
     */
    static bool is_active(
            EntityId entity_id);

    /**
     * @brief Returns the entity kind of a given id.
     *
     * @param entity_id The ID of the entity whose type is requested
     * @return EntityKind of \c entity_id.
     */
    static EntityKind get_type(
            EntityId entity_id);

    /**
     * @brief Get the meta information of a given entity
     *
     * @param entity_id The entity for which the meta information is retrieved
     * @return Info object describing the entity's meta information
     */
    static Info get_info(
            EntityId entity_id);

    /**
     * @brief Provides access to the data measured during the monitoring.
     *
     * Use this function for data types that relate to two entities,
     * as described in DataType.
     *
     * For data types that relate to a single entity,
     * use the overloaded function that takes a single entity as argument.
     *
     * \par Measurement time and intervals
     *
     * \c t_from and \c t_to define the time interval for which the measurements will be returned.
     * This time interval is further divided int \c bin segments of equal length,
     * and a measurement is returned for each segment.
     *
     * If \c bin is zero, no statistic is calculated and the raw data values in the requested
     * time interval are returned
     *
     * \par Statistics
     *
     * The kind of statistic calculated for each \c bin segment is indicated by \c statistic.
     * In this implementation, if \c statistic is \c NONE, the first raw data point in the segment is returned.
     *
     * \sa StatisticsBackend
     *
     * @param data_type The type of the measurement being requested
     * @param entity_ids_source Ids of the source entities of the requested data. These IDs must correspond to
     *                          entities of specific kinds depending on the data_type.
     * @param entity_ids_target Ids of the target entities of the requested data. These IDs must correspond to
     *                          entities of specific kinds depending on the data_type.
     * @param bins Number of time intervals in which the measurement time is divided
     * @param t_from Starting time of the returned measures.
     * @param t_to Ending time of the returned measures.
     * @param statistic Statistic to calculate for each of the bins
     * @return a vector of \c bin elements with the values of the requested statistic
     */
    static std::vector<StatisticsData> get_data(
            DataKind data_type,
            const std::vector<EntityId> entity_ids_source,
            const std::vector<EntityId> entity_ids_target,
            uint16_t bins = 0,
            Timestamp t_from = Timestamp(),
            Timestamp t_to = std::chrono::system_clock::now(),
            StatisticKind statistic = StatisticKind::NONE);

    /**
     * @brief Provides access to the data measured during the monitoring.
     *
     * Use this function for data types that relate to a single entity,
     * as described in DataType.
     *
     * For data types that relate to two entities,
     * use the overloaded function that takes a source and a target entity as arguments.
     *
     * \par Measurement time and intervals
     *
     * \c t_from and \c t_to define the time interval for which the measurements will be returned.
     * This time interval is further divided int \c bin segments of equal length,
     * and a measurement is returned for each segment.
     *
     * If \c bin is zero, no statistic is calculated and the raw data values in the requested
     * time interval are returned
     *
     * \par Statistics
     *
     * The kind of statistic calculated for each \c bin segment is indicated by \c statistic.
     * In this implementation, if \c statistic is \c NONE, the first raw data point in the segment is returned.
     *
     * \sa StatisticsBackend
     *
     * @param data_type The type of the measurement being requested
     * @param entity_ids Ids of the entities of the requested data. These IDs must correspond to
     *                   entities of specific kinds depending on the data_type.
     * @param bins Number of time intervals in which the measurement time is divided
     * @param t_from Starting time of the returned measures.
     * @param t_to Ending time of the returned measures.
     * @param statistic Statistic to calculate for each of the bins
     * @return a vector of \c bin elements with the values of the requested statistic
     */
    static std::vector<StatisticsData> get_data(
            DataKind data_type,
            const std::vector<EntityId> entity_ids,
            uint16_t bins = 0,
            Timestamp t_from = Timestamp(),
            Timestamp t_to = std::chrono::system_clock::now(),
            StatisticKind statistic = StatisticKind::NONE);

    /**
     * @brief Overload of get_data method without time arguments
     *
     * It calls the get_data method with the default time arguments.
     * It is used to set the \c statistic argument with default time values.
     *
     * @param data_type The type of the measurement being requested
     * @param entity_ids_source Ids of the source entities of the requested data. These IDs must correspond to
     *                          entities of specific kinds depending on the data_type.
     * @param entity_ids_target Ids of the target entities of the requested data. These IDs must correspond to
     *                          entities of specific kinds depending on the data_type.
     * @param bins Number of time intervals in which the measurement time is divided
     * @param statistic Statistic to calculate for each of the bins
     * @return a vector of \c bin elements with the values of the requested statistic
     */
    static std::vector<StatisticsData> get_data(
            DataKind data_type,
            const std::vector<EntityId> entity_ids_source,
            const std::vector<EntityId> entity_ids_target,
            uint16_t bins,
            StatisticKind statistic);

    /**
     * @brief Overload of get_data method without time arguments
     *
     * It calls the get_data method with the default time arguments.
     * It is used to set the \c statistic argument with default time values.
     *
     * @param data_type The type of the measurement being requested
     * @param entity_ids Ids of the entities of the requested data. These IDs must correspond to
     *                   entities of specific kinds depending on the data_type.
     * @param bins Number of time intervals in which the measurement time is divided
     * @param statistic Statistic to calculate for each of the bins
     * @return a vector of \c bin elements with the values of the requested statistic
     */
    static std::vector<StatisticsData> get_data(
            DataKind data_type,
            const std::vector<EntityId> entity_ids,
            uint16_t bins,
            StatisticKind statistic);

    /**
     * @brief Get the topology graph
     *
     * @return Graph object describing the complete topology of the entities
     */
    static Graph get_graph();

    /**
     * @brief Get a dump of the database
     *
     * @return DatabaseDump object representing the backend database
     */
    static DatabaseDump dump_database();

    /**
     * @brief Dump Fast DDS Statistics Backend's database to a file
     *
     * @param filename The name of the file where the database is dumped
     */
    static void dump_database(
            std::string filename);

    /**
     * @brief Load Fast DDS Statistics Backend's database from a file
     *
     * @param filename The name of the file from which where the database is loaded
     */
    static void load_database(
            std::string filename);

    /**
     * @brief Return the EntityKind of the entities to which a DataKind refers
     *
     * Some DataKind relate to a single Entity of a given EntityKind.
     * This is the case of @c SUBSCRIPTION_THROUGHPUT, that always relates to a @c DATAREADER.
     * Other DataKind relate to two different Entity, each one of a given EntityKind.
     * For example, @c FASTDDS_LATENCY relates to a @c DATAWRITER as source
     * and a @c DATAREADER as target of the data flow.
     * In the specific case of @c DISCOVERED_ENTITY, the DataKind relates to a @c PARTICIPANT as the discoverer,
     * but can relate to a @c DATAWRITER, @c DATAREADER or another @c PARTICIPANT as the discovered entity.
     *
     * Given a DataKind, this method provides a collection of all pairs of EntityKind to which
     * this DataKind relates.
     *
     * - For a @c DataKind that only relates to one Entity, the first element of the pair is the EntityKind
     * of such Entity, while the second element is @ref EntityKind::INVALID
     * - For a DataKind that relates to two Entity, the first element of the pair is the EntityKind
     * of the source Entity, while the second element is the EntityKind of the target Entity
     *
     * The source and target pairs returned by this method are exactly the accepted source and target EntityKind
     * accepted by @ref get_data for the given DataKind.
     * This is convenient to prepare a call to @ref get_data from an EntityKind.
     * First, call @ref get_data_supported_entity_kinds with the EntityKind to get the EntityKinds of the related entities.
     * Then, call @ref get_entities to get the available entities for that kind.
     * Finally, call @ref get_data with the pairs that @ref get_entities returns.
     *
     * i.e. Get the DISCOVERY_TIME of all entities on Host2 discovered by Host1
     * @code
     * // Get all the EntityKind pairs related to DISCOVERY_TIME.
     * std::vector<std::pair<EntityKind, EntityKind>> types_list =
     *         StatisticsBackend::get_data_supported_entity_kinds(DataKind::DISCOVERY_TIME);
     *
     * // Iterate over all the valid pairs composing the final result
     * std::vector<StatisticsData> discovery_times;
     * for (std::pair<EntityKind, EntityKind> type_pair : types_list)
     * {
     *     // Take the data for this pair and append it to the existing data
     *     std::vector<StatisticsData> tmp = StatisticsBackend::get_data(
     *             DataKind::DISCOVERY_TIME,
     *             StatisticsBackend::get_entities(type_pair.first, host1_id),
     *             StatisticsBackend::get_entities(type_pair.second, host2_id));
     *
     *     discovery_times.insert(discovery_times.end(), tmp.begin(), tmp.end());
     * }
     * @endcode
     *
     * @param data_kind Data kind
     * @return list of @c EntityKind pairs with the entity kinds to which a @c DataKind refers
     *
     * @sa DataKind
     * @sa get_data
     */
    static std::vector<std::pair<EntityKind, EntityKind>> get_data_supported_entity_kinds(
            DataKind data_kind);
};

} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATISTICSBACKEND_HPP_
