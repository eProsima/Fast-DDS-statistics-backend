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

#ifndef FASTDDS_STATISTICS_BACKEND__STATISTICS_BACKEND_HPP
#define FASTDDS_STATISTICS_BACKEND__STATISTICS_BACKEND_HPP

#include <chrono>
#include <string>

#include <fastdds_statistics_backend/fastdds_statistics_backend_dll.h>
#include <fastdds_statistics_backend/listener/DomainListener.hpp>
#include <fastdds_statistics_backend/listener/PhysicalListener.hpp>
#include <fastdds_statistics_backend/listener/CallbackMask.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/types.hpp>
#include <fastdds_statistics_backend/types/utils.hpp>
#include <fastdds_statistics_backend/types/app_names.h>

namespace eprosima {
namespace statistics_backend {

class StatisticsBackend
{

public:

    /**
     * @brief Deleted constructor, since the whole interface is static.
     */
    StatisticsBackend() = delete;

    /**
     * @brief Set the listener for the physical domain events.
     *
     * Any physical listener already configured will be replaced by the new one.
     * The provided pointer to the listener can be null, in which case,
     * any physical listener already configured will be removed.
     *
     * @param listener The listener with the callback implementations.
     * @param callback_mask Mask of the callbacks. Only the events that have the mask bit set will be informed.
     * @param data_mask Mask of the data types that will be monitored.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static void set_physical_listener(
            PhysicalListener* listener,
            CallbackMask callback_mask = CallbackMask::all(),
            DataKindMask data_mask = DataKindMask::none());

    /**
     * @brief Starts monitoring on a given domain.
     *
     * This function creates a new statistics DomainParticipant that starts monitoring
     * the requested domain ID.
     *
     * @param domain The domain ID of the DDS domain to monitor.
     * @param domain_listener Listener with the callback to use to inform of events.
     * @param callback_mask Mask of the callbacks. Only the events that have the mask bit set will be informed.
     * @param data_mask Mask of the data types that will be monitored.
     * @param app_id App id of the monitor participant.
     * @param app_metadata Metadata of the monitor participant.
     * @return The ID of the created statistics Domain.
     * @throws eprosima::statistics_backend::BadParameter if a monitor is already created for the given domain.
     * @throws eprosima::statistics_backend::Error if the creation of the monitor fails.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static EntityId init_monitor(
            DomainId domain,
            DomainListener* domain_listener = nullptr,
            CallbackMask callback_mask = CallbackMask::all(),
            DataKindMask data_mask = DataKindMask::none(),
            std::string app_id = app_id_str[(int)AppId::UNKNOWN],
            std::string app_metadata = "");

    /**
     * @brief Starts monitoring the network corresponding to a server.
     *
     * This function creates a new statistics DomainParticipant that starts monitoring
     * the network of the server with the given locators.
     *
     * The format to specify a locator is: <tt>kind:[IP]:port</tt>, where:
     *  * \b kind is one of { \c UDPv4, \c TCPv4, \c UDPv6, \c TCPv4 }
     *  * \b IP is the IP address
     *  * \b port is the IP port
     * Note that \c SHM locators are not supported. For any server configured with shared memory locators,
     * initialize the monitor using only the non shared memory locators.
     *
     * @param discovery_server_locators The locator list of the server whose network is to be monitored,
     *                                  formatted as a semicolon separated list of locators.
     * @param domain_listener Listener with the callback to use to inform of events.
     * @param callback_mask Mask of the callbacks. Only the events that have the mask bit set will be informed.
     * @param data_mask Mask of the data types that will be monitored.
     * @param app_id App id of the monitor participant.
     * @param app_metadata Metadata of the monitor participant.
     * @return The ID of the created statistics Domain.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static EntityId init_monitor(
            std::string discovery_server_locators,
            DomainListener* domain_listener = nullptr,
            CallbackMask callback_mask = CallbackMask::all(),
            DataKindMask data_mask = DataKindMask::none(),
            std::string app_id = app_id_str[(int)AppId::UNKNOWN],
            std::string app_metadata = "");

    /**
     * @brief Restarts a given monitor.
     *
     * This function restarts a domain monitor.
     * If the monitor is still active (meaning it has not being stopped), this function takes no effect.
     *
     * @param monitor_id The entity ID of the monitor to restart.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static void restart_monitor(
            EntityId monitor_id);

    /**
     * @brief Stops a given monitor.
     *
     * This function stops a domain monitor.
     * After stopping, the statistical data related to the domain is still accessible.
     *
     * @param monitor_id The entity ID of the monitor to stop.
     * @throws eprosima::statistics_backend::BadParameter if the given monitor ID is not yet registered.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static void stop_monitor(
            EntityId monitor_id);

    /**
     * @brief Clear the data of a domain given its monitor.
     *
     * This function clears all the data related to a domain given its monitor ID.
     * If the monitor is still active (meaning it has not being stopped), this functions takes no effect.
     * After clearing, the statistical data related to the domain is deleted and therefore
     * no longer accessible.
     *
     * @param monitor_id The entity ID of the monitor to stop.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
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
     * @param listener The listener with the callback implementations.
     * @param callback_mask Mask of the callbacks. Only the events that have the mask bit set will be informed.
     * @param data_mask Mask of the data types that will be monitored.
     * @throws eprosima::statistics_backend::BadParameter if the given monitor ID is not yet registered.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static void set_domain_listener(
            EntityId monitor_id,
            DomainListener* listener = nullptr,
            CallbackMask callback_mask = CallbackMask::all(),
            DataKindMask data_mask = DataKindMask::none());

    /**
     * @brief Get all the entities of a given type related to another entity.
     *
     * Get all the entity ids for every entity of kind \c entity_type that is connected with entity \c entity_id.
     * Connection between entities means they are directly connected by a contained/connect relation
     * (i.e. Host - User | Domain - Topic) or that connected entities are connected to it.
     *
     * Use case: To get all host in the system, use arguments HOST and EntityId::all().
     *
     * Use case: To get all locators from a participant with id X, use arguments LOCATOR and X,
     * this will get all the locators that are connected with the endpoints this participant has.
     *
     * In case the \c entity_id is not specified, all entities of type \c entity_type are returned.
     *
     * @param entity_type The type of entities for which the search is performed.
     * @param entity_id The ID of the entity to which the resulting entities are related.
     * @throws eprosima::statistics_backend::BadParameter in the following cases:
     *     * if the \c entity_kind is \c INVALID.
     *     * if the \c entity_id does not reference a entity contained in the database or is not EntityId::all().
     *     * if the EntityKind of the Entity with \c entity_id is \c INVALID.
     * @return All entities of type \c entity_type that are related to \c entity_id.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static std::vector<EntityId> get_entities(
            EntityKind entity_type,
            EntityId entity_id = EntityId::all());

    /**
     * @brief Get the EntityId for a given GUID in string format.
     *
     * @param guid The GUID in string format.
     * @return The EntityId corresponding to the given GUID.
     * @throws eprosima::statistics_backend::BadParameter if the GUID is not found.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static EntityId get_entity_by_guid(
            const std::string& guid);

    /**
     * @brief Returns whether the entity is active.
     *
     * For monitors, active means that no call to stop_monitor() has been performed since the last
     * time the monitor was activated.
     * For the rest of entities, active means that there is statistical data being reported within the entity.
     *
     * @param entity_id The ID of the entity whose activeness is requested.
     * @return true if active, false otherwise.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static bool is_active(
            EntityId entity_id);

    /**
     * @brief Returns whether the entity is related to a metatraffic topic.
     *
     * For Topics, it is true when they are used for sharing metatraffic data.
     * For DDSEndpoints, it is true when their associated to a metatraffic Topic.
     * For the rest of entities, metatraffic is always false.
     *
     * @param entity_id The ID of the entity whose metatraffic attribute is requested.
     * @return true if metatraffic, false otherwise.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static bool is_metatraffic(
            EntityId entity_id);

    /**
     * @brief Returns the entity kind of a given id.
     *
     * @param entity_id The ID of the entity whose type is requested.
     * @throws eprosima::statistics_backend::BadParameter if there is no entity with the given ID.
     * @return EntityKind of \c entity_id.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static EntityKind get_type(
            EntityId entity_id);

    /**
     * @brief Returns the entity status of a given id.
     *
     * @param entity_id The ID of the entity whose status is requested.
     * @throws eprosima::statistics_backend::BadParameter if there is no entity with the given ID.
     * @return StatusLevel of \c entity_id.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static StatusLevel get_status(
            EntityId entity_id);

    /**
     * @brief Get the meta information of a given entity.
     *
     * @param entity_id The entity for which the meta information is retrieved.
     * @return Info object describing the entity's meta information.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static Info get_info(
            EntityId entity_id);

    /**
     * @brief Get the IDL representation of a data type in string format for a given topic entity
     *
     * @param entity_id The entity for which the data type IDL is retrieved.
     * @return String object describing the entity's data type IDL.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static std::string get_type_idl(
            EntityId entity_id);

    /**
     * @brief Returns the id of the topic associated to an endpoint.
     *
     * @param endpoint_id The ID of a given endpoint.
     *
     * @return EntityId of the topic on which the endpoint publishes/receives messages.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static EntityId get_endpoint_topic_id(
            EntityId endpoint_id);

    /**
     * @brief Returns the id of the domain to which a given entity (Domain, DomainParticipant, Topic, endpoints) belongs.
     *
     * @param entity_id The ID of a given entity.
     *
     * @return EntityId of the domain.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static EntityId get_domain_id(
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
     * This time interval is further divided into \c bin segments of equal length,
     * and a measurement is returned for each segment.
     * Consequently, \c t_to should be greater than \c t_from by at least \c bin nanoseconds.
     *
     * If \c bin is zero, no statistic is calculated and the raw data values in the requested
     * time interval are returned.
     *
     * \par Statistics
     *
     * The kind of statistic calculated for each \c bin segment is indicated by \c statistic.
     * In this implementation, if \c statistic is \c NONE, the first raw data point in the segment is returned.
     *
     * \sa StatisticsBackend
     *
     * @param data_type The type of the measurement being requested.
     * @param entity_ids_source Ids of the source entities of the requested data. These IDs must correspond to
     *                          entities of specific kinds depending on the data_type.
     * @param entity_ids_target Ids of the target entities of the requested data. These IDs must correspond to
     *                          entities of specific kinds depending on the data_type.
     * @param bins Number of time intervals in which the measurement time is divided.
     * @param t_from Starting time of the returned measures.
     * @param t_to Ending time of the returned measures.
     * @param statistic Statistic to calculate for each of the bins.
     * @throws eprosima::statistics_backend::BadParameter if the above preconditions are not met.
     * @return a vector of \c bin elements with the values of the requested statistic.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static std::vector<StatisticsData> get_data(
            DataKind data_type,
            const std::vector<EntityId>& entity_ids_source,
            const std::vector<EntityId>& entity_ids_target,
            uint16_t bins = 0,
            Timestamp t_from = the_initial_time(),
            Timestamp t_to = now(),
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
     * This time interval is further divided into \c bin segments of equal length,
     * and a measurement is returned for each segment.
     * Consequently, \c t_to should be greater than \c t_from by at least \c bin nanoseconds.
     *
     * If \c bin is zero, no statistic is calculated and the raw data values in the requested
     * time interval are returned.
     *
     * \par Statistics
     *
     * The kind of statistic calculated for each \c bin segment is indicated by \c statistic.
     * In this implementation, if \c statistic is \c NONE, the first raw data point in the segment is returned.
     *
     * \sa StatisticsBackend
     *
     * @param data_type The type of the measurement being requested.
     * @param entity_ids Ids of the entities of the requested data. These IDs must correspond to
     *                   entities of specific kinds depending on the data_type.
     * @param bins Number of time intervals in which the measurement time is divided.
     * @param t_from Starting time of the returned measures.
     * @param t_to Ending time of the returned measures.
     * @param statistic Statistic to calculate for each of the bins.
     * @throws eprosima::statistics_backend::BadParameter if the above preconditions are not met.
     * @return a vector of \c bin elements with the values of the requested statistic.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static std::vector<StatisticsData> get_data(
            DataKind data_type,
            const std::vector<EntityId>& entity_ids,
            uint16_t bins = 0,
            Timestamp t_from = the_initial_time(),
            Timestamp t_to = now(),
            StatisticKind statistic = StatisticKind::NONE);

    /**
     * @brief Overload of get_data method without time arguments.
     *
     * It calls the get_data method with the default time arguments.
     * It is used to set the \c statistic argument with default time values.
     *
     * @param data_type The type of the measurement being requested.
     * @param entity_ids_source Ids of the source entities of the requested data. These IDs must correspond to
     *                          entities of specific kinds depending on the data_type.
     * @param entity_ids_target Ids of the target entities of the requested data. These IDs must correspond to
     *                          entities of specific kinds depending on the data_type.
     * @param bins Number of time intervals in which the measurement time is divided.
     * @param statistic Statistic to calculate for each of the bins.
     * @throws eprosima::statistics_backend::BadParameter if the above preconditions are not met.
     * @return a vector of \c bin elements with the values of the requested statistic.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static std::vector<StatisticsData> get_data(
            DataKind data_type,
            const std::vector<EntityId>& entity_ids_source,
            const std::vector<EntityId>& entity_ids_target,
            uint16_t bins,
            StatisticKind statistic);

    /**
     * @brief Overload of get_data method without time arguments.
     *
     * It calls the get_data method with the default time arguments.
     * It is used to set the \c statistic argument with default time values.
     *
     * @param data_type The type of the measurement being requested.
     * @param entity_ids Ids of the entities of the requested data. These IDs must correspond to
     *                   entities of specific kinds depending on the data_type.
     * @param bins Number of time intervals in which the measurement time is divided.
     * @param statistic Statistic to calculate for each of the bins.
     * @throws eprosima::statistics_backend::BadParameter if the above preconditions are not met.
     * @return a vector of \c bin elements with the values of the requested statistic.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static std::vector<StatisticsData> get_data(
            DataKind data_type,
            const std::vector<EntityId>& entity_ids,
            uint16_t bins,
            StatisticKind statistic);

    /**
     * @brief Get monitor service status data.
     *
     * Default method is called if StatusKind is invalid.
     *
     * @param entity_id The id of the Entity whose status info is requested.
     * @param status_data Status data to be filled.
     * @throws eprosima::statistics_backend::BadParameter in the following cases:
     *     * if the \c entity_id does not reference a entity contained in the database.
     *     * if there is no specialization template for the requested StatusKind.
     *     * if the EntityKind of the Entity with \c entity_id doesn't have the associated \c status_data.
     */
    template <typename T>
    static void get_status_data(
            const EntityId& entity_id,
            T& status_data);

    /**
     * @brief Get the domain view graph.
     *
     * @param domain_id EntityId from domain whose graph is delivered.
     * @throws eprosima::statistics_backend::BadParameter if there is no graph for the specified domain id.
     * @return Graph object describing per domain topology of the entities.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static Graph get_domain_view_graph(
            const EntityId& domain_id);

    /**
     * @brief Regenerate graph from data stored in database.
     *
     * @param domain_id EntityId from domain whose graph is regenerated.
     *
     * @return True if the graph has been regenerated
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static bool regenerate_domain_graph(
            const EntityId& domain_id);

    /**
     * @brief Get a dump of the database.
     *
     * @param clear If true, clear all the statistics data of all entities.
     *
     * @return DatabaseDump object representing the backend database.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static DatabaseDump dump_database(
            bool clear);

    /**
     * @brief Dump Fast DDS Statistics Backend's database to a file.
     *
     * @param filename The name of the file where the database is dumped.
     * @param clear If true, clear all the statistics data of all entities.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static void dump_database(
            const std::string& filename,
            bool clear);

    /**
     * @brief Load Fast DDS Statistics Backend's database from a file.
     *
     * @pre The Backend's database has no data. This means that no monitors were initialized
     *      since the Backend started, or that the Backend has been reset().
     *
     * @param filename The name of the file from which where the database is loaded.
     * @throws eprosima::statistics_backend::BadParameter if the file does not exist.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static void load_database(
            const std::string& filename);

    /**
     * @brief Clear statistics data of all entities received previous to the time given.
     *
     * @param t_to Timestamp regarding the maximum time to stop removing data.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static void clear_statistics_data(
            const Timestamp& t_to = the_end_of_time());

    //! Remove all inactive entities from database.
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static void clear_inactive_entities();

    /**
     * @brief Resets the Fast DDS Statistics Backend.
     *
     * After calling this method, the Fast DDS Statistics Backend
     * reverts to its default state, as it was freshly started:
     * - All the data in the database is erased.
     * - All monitors are removed and cannot be restarted afterwards.
     * - The physical listener is removed.
     * - The physical listener callback mask is set to CallbackMask::none().
     * - The physical listener data mask is set to DataMask::none().
     *
     * @pre There are no active monitors. There can be inactive monitors.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static void reset();

    /**
     * @brief Return the EntityKind of the entities to which a DataKind refers.
     *
     * Some DataKind relate to a single Entity of a given EntityKind.
     * This is the case of @c SUBSCRIPTION_THROUGHPUT, that always relates to a @c DATAREADER.
     * Other DataKind relate to two different Entity, each one of a given EntityKind.
     * For example, @c FASTDDS_LATENCY relates to a @c DATAWRITER as source
     * and a @c DATAREADER as target of the data flow.
     * In the specific case of @c DISCOVERY_TIME, the DataKind relates to a @c PARTICIPANT as the discoverer,
     * but can relate to a @c DATAWRITER, @c DATAREADER or another @c PARTICIPANT as the discovered entity.
     *
     * Given a DataKind, this method provides a collection of all pairs of EntityKind to which
     * this DataKind relates.
     *
     * - For a @c DataKind that only relates to one Entity, the first element of the pair is the EntityKind
     * of such Entity, while the second element is @ref EntityKind::INVALID.
     * - For a DataKind that relates to two Entity, the first element of the pair is the EntityKind
     * of the source Entity, while the second element is the EntityKind of the target Entity.
     *
     * The source and target pairs returned by this method are exactly the accepted source and target EntityKind
     * accepted by @ref get_data for the given DataKind.
     * This is convenient to prepare a call to @ref get_data from an EntityKind.
     * First, call @ref get_data_supported_entity_kinds with the EntityKind to get the EntityKinds
     * of the related entities.
     * Then, call @ref get_entities to get the available entities for that kind.
     * Finally, call @ref get_data with the pairs that @ref get_entities returns.
     *
     * i.e. Get the DISCOVERY_TIME of all entities on Host2 discovered by Host1:
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
     * @param data_kind Data kind.
     * @return list of @c EntityKind pairs with the entity kinds to which a @c DataKind refers.
     *
     * @sa DataKind
     * @sa get_data
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static std::vector<std::pair<EntityKind, EntityKind>> get_data_supported_entity_kinds(
            DataKind data_kind);

    /**
     * @brief Set a new alias for the entity.
     *
     * @param entity_id The EntityId of the entity.
     * @param alias New alias that will replace the old one.
     * @throws eprosima::statistics_backend::BadParameter if there is no entity with the given ID.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static void set_alias(
            EntityId entity_id,
            const std::string& alias);

    /**
     * @brief Deserialize entity guid to string format.
     * @param data Entity guid.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static std::string deserialize_guid(
            fastdds::statistics::detail::GUID_s data);

    /**
     * @brief Serialize entity guid from string.
     * @param guid_str Entity guid.
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI
    static fastdds::statistics::detail::GUID_s serialize_guid(
            const std::string& guid_str);
};

} // namespace statistics_backend
} // namespace eprosima

#endif //FASTDDS_STATISTICS_BACKEND__STATISTICS_BACKEND_HPP
