/* Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file database.hpp
 */

#ifndef FASTDDS_STATISTICS_BACKEND_SRC_CPP_DATABASE__DATABASE_HPP
#define FASTDDS_STATISTICS_BACKEND_SRC_CPP_DATABASE__DATABASE_HPP

#include <atomic>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <type_traits> // enable_if, is_integral

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/Alerts.hpp>
#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/types/Notifiers.hpp>

#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/RemoteLocators.hpp>

#include <database/entities.hpp>
#include <types/DataContainer.hpp>

namespace eprosima {
namespace statistics_backend {

/**
 * std::chrono::duration uses sfinae to prevent information losses on construction. Some OS system_clock implemenations
 * are not accurate enough to handle a nanosecond resolution. For example on windows the system_clock resolution is 100
 * nanoseconds. Truncation must be enforced in those cases.
 * As reference on a linux distro over an i5 processor the actual system_clock tested resolution is in average 30 ns
 * with a standard deviation of 40 ns. The worse case scenario is about 70 ns ~ 100 ns used by windows.
 *
 * @param nanosecs from posix epoch 1970-01-01
 * @return argument date on local system_clock time point
 */

template<class T,
        typename std::enable_if<
            std::is_integral<T>::value,
            bool>::type = true>
std::chrono::system_clock::duration nanoseconds_to_systemclock_duration(
        T nanosecs) noexcept
{
    using namespace std;
    using namespace std::chrono;

    auto system_clock_resolution = system_clock::duration(1);
    return system_clock::duration(
        nanosecs / duration_cast<nanoseconds>(system_clock_resolution).count());
}

template<class T,
        typename std::enable_if<
            std::is_integral<T>::value,
            bool>::type = true>
std::chrono::system_clock::time_point nanoseconds_to_systemclock(
        T nanosecs) noexcept
{
    return std::chrono::system_clock::time_point(
        nanoseconds_to_systemclock_duration(nanosecs));
}

namespace database {

class Database
{
public:

    /**
     * @brief Destructor of Database.
     *
     * @note It requires a virtual dtor for test sake.
     */
    virtual ~Database() = default;

    /**
     * @brief Create new DomainParticipant and insert it in database.
     * @param name The name of the DomainParticipant.
     * @param qos The QoS of the DomainParticipant.
     * @param guid The GUID of the DomainParticipant.
     * @param domain_id The EntityId of the domain to which the DomainParticipant corresponds.
     * @param status The status of the DomainParticipant.
     * @param app_id The AppId of the DomainParticipant.
     * @param app_metadata The App metadata of the DomainParticipant.
     * @param discovery_source The DiscoverySource of the DomainParticipant.
     * @param original_domain The original DomainId of the DomainParticipant, UNKNOWN_DOMAIN_ID if not specified.
     *
     * @return EntityId of the DomainParticipant once inserted.
     */
    EntityId insert_new_participant(
            const std::string& name,
            const Qos& qos,
            const std::string& guid,
            const EntityId& domain_id,
            const StatusLevel& status,
            const AppId& app_id,
            const std::string& app_metadata,
            DiscoverySource discovery_source,
            DomainId original_domain = UNKNOWN_DOMAIN_ID);

    /**
     * @brief Process Host-User-Process entities and insert them in database.
     * @param host_name The name of the host.
     * @param user_name The name of the user.
     * @param process_name The name of the process.
     * @param process_pid The pid of the process.
     * @param discovery_source The discovery source of the physical entities.
     * @param should_link_process_participant If true, try to link process to participant.
     * @param participant_id The EntityId of the participant that the process might be linked to.
     * @param physical_entities_ids Map where host-user-process EntityIds are stored when inserted in database.
     */
    void process_physical_entities(
            const std::string& host_name,
            const std::string& user_name,
            const std::string& process_name,
            const std::string& process_pid,
            DiscoverySource discovery_source,
            bool& should_link_process_participant,
            const EntityId& participant_id,
            std::map<std::string, EntityId>& physical_entities_ids);

    /**
     * @brief Check if the topic is already in the database. Topic type must match.
     * @param topic_type The type of the topic.
     * @param topic_id The EntityId of the topic.
     *
     * @return True if the topic is in the database.
     */
    bool is_topic_in_database(
            const std::string& topic_type,
            const EntityId& topic_id);

    /**
     * @brief Create new Topic and insert it in database.
     * @param name The name of the Topic.
     * @param type_name The type name of the Topic.
     * @param alias The alias of the Topic.
     * @param domain_id The EntityId of the domain to which the Topic corresponds.
     *
     * @return EntityId of the Topic once inserted.
     */
    EntityId insert_new_topic(
            const std::string& name,
            const std::string& type_name,
            const std::string& alias,
            const EntityId& domain_id);

    /**
     * @brief Check if a Topic data type is already in the database
     * @param type_name The type name of the Topic.
     *
     * @return True if the Topic data type is in the database.
     */
    bool is_type_in_database(
            const std::string& type_name);

    /**
     * @brief Insert a new type IDL into the database or update it, and perform ROS 2 demangling if needed.
     * If demangled, insert the demangled type IDL and separately, the original one in a different map.
     * @param topic_type The type of the topic.
     * @param topic_idl The IDL representation of the type.
     */
    void insert_new_type_idl(
            const std::string& topic_type,
            const std::string& topic_idl);

    /**
     * @brief Create new Endpoint and corresponding Locator, and insert them in database.
     * @param endpoint_guid The GUID of the Endpoint.
     * @param name The name of the Endpoint.
     * @param alias The alias of the Endpoint.
     * @param qos The QoS of the Endpoint.
     * @param is_virtual_metatraffic Flag for metatraffic endpoints.
     * @param locators Locators related to endpoint.
     * @param kind EntityKind of the endpoint.
     * @param participant_id The EntityId of the Participant related to the Endpoint.
     * @param topic_id The EntityId of the Topic related to the Endpoint.
     * @param app_data The AppId and app metadata related to the Endpoint.
     * @param discovery_source The DiscoverySource of the Endpoint.
     * @param original_domain The original DomainId of the Endpoint, UNKNOWN_DOMAIN_ID if not specified.
     *
     * @return EntityId of the Endpoint once inserted.
     */
    EntityId insert_new_endpoint(
            const std::string& endpoint_guid,
            const std::string& name,
            const std::string& alias,
            const Qos& qos,
            const bool& is_virtual_metatraffic,
            const fastdds::rtps::RemoteLocatorList& locators,
            const EntityKind& kind,
            const EntityId& participant_id,
            const EntityId& topic_id,
            const std::pair<AppId, std::string>& app_data,
            DiscoverySource discovery_source,
            DomainId original_domain);

    /**
     * @brief Insert a new entity into the database.
     * @param entity The entity object to be inserted.
     * @throws eprosima::statistics_backend::BadParameter in the following cases:
     *             * If the entity already exists in the database.
     *             * If the parent entity does not exist in the database (except for the cases of
     *               a DomainParticipant entity, for which an unregistered parent process is allowed;
     *               and Locator entities, which can be registered without a parent endpoint).
     *             * If the entity name is empty.
     *             * Depending on the type of entity, if some other identifier is empty.
     *             * For entities with GUID, if the GUID is not unique.
     *             * For entities with QoS, if the QoS is empty.
     *             * For entities with locators, if the locators' collection is empty.
     * @return The EntityId of the inserted entity.
     */
    EntityId insert(
            const std::shared_ptr<Entity>& entity);

    /**
     * @brief Insert a new statistics sample into the database.
     * @param domain_id The EntityId of the domain that contains the entity.
     * @param entity_id The EntityId to which the sample relates.
     * @param sample The sample to be inserted.
     * @throws eprosima::statistics_backend::BadParameter in the following cases:
     *             * If the \c domain_id does not refer to a known domain.
     *             * If the \c entity_id does not refer to a known entity.
     *             * If the \c sample kind is DataKind::INVALID.
     */
    void insert(
            const EntityId& domain_id,
            const EntityId& entity_id,
            const StatisticsSample& sample);

    /**
     * @brief Insert a new monitor service sample into the database.
     * @param domain_id The EntityId of the domain that contains the entity.
     * @param entity_id The EntityId to which the sample relates.
     * @param sample The sample to be inserted.
     * @throws eprosima::statistics_backend::BadParameter in the following cases:
     *             * If the \c domain_id does not refer to a known domain.
     *             * If the \c entity_id does not refer to a known entity.
     *             * If the \c sample kind is StatusKind::INVALID.
     * @return True if the entity status has been updated.
     */
    bool insert(
            const EntityId& domain_id,
            const EntityId& entity_id,
            const MonitorServiceSample& sample);

    /**
     * @brief Triggers all the alerts of a specific kind if the entity and the data
     * meet the conditions
     *
     * @param domain_id The EntityId of the domain that contains the triggerer entity.
     * @param entity_id The EntityId of the entity for which to trigger the alerts.
     * @param endpoint The DDSEndpoint for which to trigger the alerts
     * @param alert_kind The kind of alert to trigger.
     * @param data The value that might trigger the alert
     */
    void trigger_alerts_of_kind(
            const EntityId& domain_id,
            const EntityId& entity_id,
            const std::shared_ptr<DDSEndpoint>& endpoint,
            const AlertKind alert_kind,
            const EntityCountSample& data);

    /**
     * @brief Triggers all the alerts of a specific kind if the entity and the data
     * meet the conditions
     *
     * @param domain_id The EntityId of the domain that contains the triggerer entity.
     * @param entity_id The EntityId of the entity for which to trigger the alerts.
     * @param endpoint The DDSEndpoint for which to trigger the alerts
     * @param alert_kind The kind of alert to trigger.
     * @param data The value that might trigger the alert
     */
    void trigger_alerts_of_kind(
            const EntityId& domain_id,
            const EntityId& entity_id,
            const std::shared_ptr<DDSEndpoint>& endpoint,
            const AlertKind alert_kind,
            const EntityDataSample& data);

    /**
     * For all alerts in the database, check if they have timed out.
     * If they have, they are triggered with an appropriate message
     */
    void check_alerts_timeouts();

    /**
     * @brief Create the link between a participant and a process.
     *
     * This operation entails:
     *     1. Adding reference to process to the participant.
     *     2. Adding the participant to the process' list of participants.
     *
     * @param participant_id The EntityId of the participant.
     * @param process_id The EntityId of the process.
     * @throw eprosima::statistics_backend::BadParameter in the following cases:
     *            * The participant is already linked with a process.
     *            * The participant does not exist in the database.
     *            * The process does not exist in the database.
     */
    void link_participant_with_process(
            const EntityId& participant_id,
            const EntityId& process_id);

    /**
     * @brief Erase all the data related to a domain.
     *
     * After the operation, the domain_id becomes invalid.
     * @pre
     *            * the domain must exist.
     *            * the monitor attached to the domain must be stopped.
     *
     * @param domain_id The EntityId of the domain to be erased.
     * @throws eprosima::statistics_backend::BadParameter in the following cases:
     *            * if the \c EntityId is not of \c DataKind::DOMAIN.
     *            * if the \c domain_id does not refer to a known domain.
     */
    void erase(
            EntityId& domain_id);

    /**
     * @brief Select data from the database.
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
     *
     * \sa Database
     *
     * @param data_type The type of the measurement being requested.
     * @param entity_id_source Id of the source entity of the requested data.
     * @param entity_id_target Id of the target entity of the requested data.
     * @param t_from Starting time of the returned measures.
     * @param t_to Ending time of the returned measures.
     * @throws eprosima::statistics_backend::BadParameter when the parameters are not consistent:
     *            * \c t_from must be less than \c t_to.
     *            * \c data_type must be of a type that relates to two entities.
     *            * Both EntityIds must be known in the database.
     * @return A vector of pointers to StatisticSamples.
     */
    std::vector<const StatisticsSample*> select(
            DataKind data_type,
            EntityId entity_id_source,
            EntityId entity_id_target,
            Timestamp t_from,
            Timestamp t_to);

    /**
     * @brief Select data from the database.
     *
     * Use this function for data types that relate to two entities,
     * as described in DataType.
     *
     * For data types that relate to two entities,
     * use the overloaded function that takes two entities as arguments.
     *
     * \par Measurement time and intervals
     *
     * \c t_from and \c t_to define the time interval for which the measurements will be returned.
     *
     * \sa Database
     *
     * @param data_type The type of the measurement being requested.
     * @param entity_id Id of entity of the requested data.
     * @param t_from Starting time of the returned measures.
     * @param t_to Ending time of the returned measures.
     * @throws eprosima::statistics_backend::BadParameter when the parameters are not consistent:
     *            * \c t_from must be less than \c t_to.
     *            * \c data_type must be of a type that relates to a single entity.
     *            * Both EntityIds must be known in the database.
     * @return A vector of pointers to StatisticSamples.
     */
    std::vector<const StatisticsSample*> select(
            DataKind data_type,
            EntityId entity_id,
            Timestamp t_from,
            Timestamp t_to);

    /**
     * @brief Get service status data.
     *
     * Default method is called if StatusKind is invalid.
     *
     * @param entity_id The id of the Entity whose status info is requested.
     * @param status_data Status data to be filled.
     *
     * @throws eprosima::statistics_backend::BadParameter if there is no specialization template for the requested StatusKind.
     */
    template <typename T>
    void get_status_data(
            const EntityId& entity_id,
            T& status_data)
    {
        static_cast<void>(entity_id);
        static_cast<void>(status_data);

        throw BadParameter("Unsupported StatusKind");
    }

    /**
     * @brief Whether an entity id is present/exists in the database.
     *
     * \c entity_kind is accelerates the search, but it is not mandatory.
     *
     * @param entity_id ID of the Entity to look for.
     *
     * @return true if exists an entity with such id
     * @return false otherwise
     *
     * @todo implement fast search in get_entity
     */
    bool is_entity_present(
            const EntityId& entity_id) const noexcept;

    /**
     * @brief Auxiliar function to get the internal collection of DDSEndpoints of a specific type,
     * a.k.a DataReader or DataWriter.
     *
     * @tparam T The DDSEndpoint-derived class name. Only DataReader and DataWriter are allowed.
     * @return The corresponding internal collection, a.k.a datareaders_ or datawriters_.
     */
    template<typename T>
    std::map<EntityId, std::map<EntityId, std::shared_ptr<T>>>& dds_endpoints();

    /**
     * Get an entity given its EntityId.
     *
     * @param entity_id constant reference to the EntityId of the retrieved entity.
     * @throws eprosima::statistics_backend::BadParameter if there is no entity with the given ID.
     * @return A constant shared pointer to the Entity.
     */
    const std::shared_ptr<const Entity> get_entity(
            const EntityId& entity_id) const;

    /**
     * Get all entities of a given EntityKind related to another entity.
     *
     * In case the \c entity_id is EntityId::all(), all entities of type \c entity_type are returned.
     *
     * @param entity_id constant reference to the EntityId of the entity to which the returned
     *                  entities are related.
     * @param entity_kind The EntityKind of the fetched entities.
     * @throws eprosima::statistics_backend::BadParameter in the following case:
     *            * if the \c entity_kind is \c INVALID.
     *            * if the \c entity_id does not reference a entity contained in the database or is not EntityId::all().
     *            * if the EntityKind of the Entity with \c entity_id is \c INVALID.
     * @return A constant vector of shared pointers to the entities
     */
    const std::vector<std::shared_ptr<const Entity>> get_entities(
            EntityKind entity_kind,
            const EntityId& entity_id) const;

    /**
     * Get all EntityIds of a given EntityKind related to another entity.
     *
     * In case the \c entity_id is EntityId::all(), all EntityIds of type \c entity_type are returned.
     *
     * @param entity_id constant reference to the EntityId of the entity to which the returned
     *                  entities are related.
     * @param entity_kind The EntityKind of the fetched entities.
     * @throws eprosima::statistics_backend::BadParameter in the following case:
     *            * if the \c entity_kind is \c INVALID.
     *            * if the \c entity_id does not reference a entity contained in the database or is not EntityId::all().
     *            * if the EntityKind of the Entity with \c entity_id is \c INVALID.
     * @return A vector containing the EntityIds of the entities.
     */
    std::vector<EntityId> get_entity_ids(
            EntityKind entity_type,
            const EntityId& entity_id) const;

    /**
     *  @brief Generate an EntityId that is unique for the database.
     *
     * @return The unique EntityId.
     */
    EntityId generate_entity_id() noexcept;

    /**
     * @brief Get all entities of a given EntityKind that match with the requested name.
     *
     * @param entity_kind The EntityKind of the fetched entities.
     * @param name The name of the entities for which to search.
     * @throws eprosima::statistics_backend::BadParameter if \c entity_kind is not valid.
     * @return A vector of pairs, where the first field is the EntityId of the Domain of the matching entities,
     *         and the second is the EntityId of the matching entities. For physical entities (Host, User, Process,
     *         Locator) the returned Domain EntityId is EntityId::INVALID, as it has no meaning since these entities
     *         do not belong to a Domain.
     */
    std::vector<std::pair<EntityId, EntityId>> get_entities_by_name(
            EntityKind entity_kind,
            const std::string& name) const;

    /**
     * @brief Get the type IDL of a given type name, if it exists.
     *
     * @param type_name The name of the data type for which to search.
     * @throws eprosima::statistics_backend::BadParameter if \c type_name does not exist in the database.
     * @return The IDL representation of the type in std::string format.
     */
    std::string get_type_idl(
            const std::string& type_name) const;

    /**
     * @brief Get the demangled type name of a given type, if it exists, for display purposes.
     *
     * @param type_name The name of the data type for which to search.
     * @throws eprosima::statistics_backend::BadParameter if \c type_name does not exist in the database.
     * @return The name type in std::string format.
     */
    std::string get_ros2_type_name(
            const std::string& type_name) const;

    /**
     * @brief Get the original ROS 2 type IDL of a given type name, if it exists.
     *
     * @param type_name The name of the data type for which to search.
     * @throws eprosima::statistics_backend::BadParameter if \c type_name does not exist in the database.
     * @return The original ROS 2 IDL representation of the type in std::string format.
     */
    std::string get_ros2_type_idl(
            const std::string& type_name) const;

    /**
     * @brief Gets the lists of active alerts
     */
    std::vector<AlertId> get_alerts_ids() const;

    /**
     * @brief Get alert information given its AlertId.
     */
    const std::shared_ptr<const AlertInfo> get_alert(
            const AlertId& alert_id) const;

    /**
     * @brief Get notifier information given its NotifierId.
     */
    const std::shared_ptr<const Notifier> get_notifier(
            const NotifierId& notifier_id) const;

    /**
     * @brief Get the entity of a given EntityKind that matches with the requested GUID.
     *
     * @param entity_kind The EntityKind of the fetched entities.
     * @param guid The GUID of the entities to search for.
     * @throws eprosima::statistics_backend::BadParameter in the following cases:
     *             * if the given EntityKind does not contain a GUID.
     *             * if there is no entity with the given parameters.
     * @return A pair, where the first field is the EntityId of the Domain of the matching entities,
     *         and the second is the EntityId of the matching entity.
     */
    std::pair<EntityId, EntityId> get_entity_by_guid(
            EntityKind entity_kind,
            const std::string& guid) const;

    /**
     * @brief Get the entity kind of an entity that matches with the requested GUID.
     *
     * @param guid The GUID of the entities to search for.
     * @throws eprosima::statistics_backend::BadParameter if there is no entity with the given parameters.
     * @return The EntityKind of the matching entity.
     */
    EntityKind get_entity_kind_by_guid(
            const eprosima::fastdds::statistics::detail::GUID_s& guid) const;

    /**
     * @brief Get EntityKind given an EntityId.
     *
     * @param entity_id The EntityId of the entity.
     * @throws eprosima::statistics_backend::BadParameter if there is no entity with the given ID.
     * @return The EntityKind of the given entity.
     */
    EntityKind get_entity_kind(
            EntityId entity_id) const;

    /**
     * @brief Get StatusLevel given an EntityId.
     *
     * @param entity_id The EntityId of the entity.
     * @throws eprosima::statistics_backend::BadParameter if there is no entity with the given ID.
     * @return The StatusLevel of the given entity.
     */
    StatusLevel get_entity_status(
            EntityId entity_id) const;

    /**
     * @brief Get GUID string given an EntityId.
     *
     * @param entity_id The EntityId of the entity.
     * @throws eprosima::statistics_backend::BadParameter if there is no entity with the given ID.
     * @return The GUID string of the given entity.
     */
    std::string get_entity_guid(
            EntityId entity_id) const;

    /**
     * Update entity status according to warnings and errors.
     *
     * @param entity_error Flag showing if there is any error status data in the entity.
     * @param entity_warning Flag showing if there is any warning status data in the entity.
     * @param entity_status Reference to the status of the entity.
     * @return True if the status has changed.
     */
    bool entity_status_logic(
            const bool& entity_error,
            const bool& entity_warning,
            StatusLevel& entity_status);

    /**
     * @brief Update the current QoS of a existing entity according to the received QoS.
     *
     * @note When entity QoS is updated, current and received QoS are merged:
     *       new keys are added to the entity QoS JSON, and existing values are preserved
     *       unless explicitly overwritten.
     * @param entity The ID of the entity to be updated.
     * @param received_qos The new QoS information used to update the entity.
     * @throws eprosima::statistics_backend::BadParameter in the following cases:
     *            * If an entity with the given ID does not exist in the database.
     *            * If the entity exists in the database, but it is not a valid DDS Entity (i.e: It does not store QoS policies).
     * @return True if entity QoS has been updated,
     *         false if there is no changes.
     */
    bool update_entity_qos(
            const EntityId& entity,
            const Qos& received_qos);

    /**
     * @brief Update participant discovery information.
     * @param participant_id The EntityId of the participant to be updated.
     * @param host The name of the host.
     * @param user The name of the user.
     * @param process The name of the process.
     * @param name The name of the participant.
     * @param qos The QoS of the participant.
     * @param guid The GUID of the participant.
     * @param domain_id The EntityId of the domain to which the participant corresponds.
     * @param status The status of the participant.
     * @param app_id The AppId of the participant.
     * @param app_metadata The App metadata of the participant.
     * @param discovery_source The DiscoverySource of the participant.
     * @param original_domain The original DomainId of the participant, UNKNOWN_DOMAIN_ID if not
     */
    bool update_participant_discovery_info(
            const EntityId& participant_id,
            const std::string& host,
            const std::string& user,
            const std::string& process,
            const std::string& name,
            const Qos& qos,
            const std::string& guid,
            const EntityId& domain_id,
            const StatusLevel& status,
            const AppId& app_id,
            const std::string& app_metadata,
            DiscoverySource discovery_source,
            DomainId original_domain);

    /**
     * @brief Get the specified domain view graph from database.
     *
     * @param domain Domain from which graph is delivered.
     * @throws eprosima::statistics_backend::BadParameter if there is no graph for the specified domain id.
     * @return Graph object describing topology of the entities in the domain.
     */
    Graph get_domain_view_graph(
            const EntityId& domain_id) const;

    /**
     * @brief Init domain view graph with specified domain.
     *
     * @param domain_name The name of the domain where monitoring.
     * @param domain_id The DomainId of the domain where monitoring.
     * @param domain_entity_id The EntityId of the domain.
     */
    void init_domain_view_graph(
            const std::string& domain_name,
            const DomainId domain_id,
            const EntityId& domain_entity_id);

    /**
     * @brief Update host-user-process-participant in domain view graph after participant entity discovery.
     *
     * @param domain_entity_id The EntityId of the domain.
     * @param host_entity_id The EntityId of the host.
     * @param user_entity_id The EntityId of the user.
     * @param process_entity_id The EntityId of the process.
     * @param participant_entity_id The EntityId of the participant.
     *
     * @return True if graph has been updated
     */
    bool update_participant_in_graph(
            const EntityId& domain_entity_id,
            const EntityId& host_entity_id,
            const EntityId& user_entity_id,
            const EntityId& process_entity_id,
            const EntityId& participant_entity_id);

    /**
     * @brief Update topic-endpoint in domain view graph after endpoint entity discovery.
     *
     * @param domain_entity_id The EntityId of the domain.
     * @param participant_entity_id The EntityId of the participant.
     * @param topic_entity_id The EntityId of the topic.
     * @param endpoint_entity_id The EntityId of the endpoint.
     *
     * @return True if graph has been updated
     */
    bool update_endpoint_in_graph(
            const EntityId& domain_entity_id,
            const EntityId& participant_entity_id,
            const EntityId& topic_entity_id,
            const EntityId& endpoint_entity_id);

    /**
     * @brief Regenerate graph from data stored in database.
     *
     * @param domain_entity_id The EntityId of the domain.
     *
     * @return True if the graph has been regenerated
     */
    bool regenerate_domain_graph(
            const EntityId& domain_entity_id);

    /**
     * @brief Update graph according to the updated entity.
     *
     * @param domain_id The EntityId of the domain containing the entity.
     * @param entity_id The EntityId of the entity updated.
     *
     * @return True if the graph has been updated
     */
    bool update_graph_on_updated_entity(
            const EntityId& domain_id,
            const EntityId& entity_id);

    /**
     * @brief Setter for entity alias.
     *
     * @param entity_id The EntityId of the entity updated.
     * @param alias The new alias of the entity.
     *
     */
    void set_alias(
            const EntityId& entity_id,
            const std::string& alias);

    /**
     * @brief Setter for entity alert.
     *
     * @param alert_info The new alert information.
     * @return The AlertId of the alert.
     */
    AlertId insert_alert(
            AlertInfo& alert_info);

    /**
     * @brief Remove an alert from the database.
     *
     * @param alert_info The alert id
     */
    void remove_alert(
            const AlertId& alert_id);

    /**
     * @brief Insert a notifier into the database.
     *
     * @param notifier The new notifier.
     * @return The NotifierId of the added notifier.
     */
    NotifierId insert_notifier(
            Notifier& notifier);

    /**
     * @brief Trigger a notifier.
     *
     * @param notifier_id The NotifierId of the notifier to trigger.
     * @param message The message to send with the notifier.
     */
    void trigger_notifier(
            const NotifierId& notifier_id,
            std::string message);

    /**
     * @brief Remove a notifier from the database.
     *
     * @param notifier_id The NotifierId of the notifier to remove.
     */
    void remove_notifier(
            const NotifierId& notifier_id);

    /**
     * @brief Get a dump of the database.
     *
     * @param clear If true, remove the statistics data of the database. This not include the info or discovery data.
     *
     * @return DatabaseDump object representing the backend database.
     *
     * @note This method includes clear possibility so dump and clear could be under the same mutex lock.
     */
    DatabaseDump dump_database(
            const bool clear = false);

    /**
     * @brief Load Entities and their data from dump (json) object.
     *
     * @param dump Object with the object with the dump to load.
     * @throws eprosima::statistics_backend::Error if there are already entities contained within the database.
     */
    void load_database(
            const DatabaseDump& dump);

    /**
     * Change the status (active/inactive) of an entity given an EntityId.
     * Also check if the references of the entity must also be changed and change the status in that case.
     * A call to the user listener will be performed if the status of the entity changes.
     * @param entity_id The EntityId of the entity
     * @param active The active/inactive value to set
     * @throws eprosima::statistics_backend::BadParameter if entity_kind is not valid.
     */
    void change_entity_status(
            const EntityId& entity_id,
            bool active);

    /**
     * @brief Remove the statistics data of the database. This does not include the info or discovery data.
     *
     * @param t_to Timestamp regarding the maximum time to stop removing data.
     */
    void clear_statistics_data(
            const Timestamp& t_to);

    /**
     * @brief Remove all inactive entities of the database. This does not include domains.
     */
    void clear_inactive_entities();

    /**
     * @brief Returns whether the entity is active.
     *
     * @param entity_id The ID of the entity whose active attribute is requested.
     *
     * @return True if active, false otherwise.
     */
    bool is_active(
            const EntityId& entity_id);

    /**
     * @brief Returns whether the entity is metatraffic.
     *
     * @param entity_id The ID of the entity whose metatraffic attribute is requested.
     *
     * @return True if metatraffic, false otherwise.
     */
    bool is_metatraffic(
            const EntityId& entity_id);

    /**
     * @brief Returns whether the entity was discovered using a proxy message
     *
     * @param entity_id The ID of the entity whose proxy attribute is requested.
     *
     * @return True if proxy, false otherwise.
     */
    bool is_proxy(
            const EntityId& entity_id);

    /**
     * @brief Get the meta information of a given entity.
     *
     * @param entity_id The entity for which the meta information is retrieved.
     *
     * @return Info object describing the entity's meta information.
     */
    Info get_info(
            const EntityId& entity_id);

    /**
     * @brief Get the meta information of a given alert.
     *
     * @param alert_id The alert for which the meta information is retrieved.
     *
     * @return Info object describing the alert's meta information.
     */
    Info get_info(
            const AlertId& alert_id);

    /**
     * @brief Returns the id of the topic associated to an endpoint.
     *
     * @param endpoint_id The ID of a given endpoint.
     *
     * @return EntityId of the topic on which the endpoint publishes/receives messages.
     */
    EntityId get_endpoint_topic_id(
            const EntityId& endpoint_id);

    /**
     * @brief Returns the id of the domain associated to a given entity (Domain, Participant, topic and endpoint).
     *
     * @param entity_id The ID of an entity.
     *
     * @return EntityId of the domain which the entity belongs.
     */
    EntityId get_domain_id(
            const EntityId& entity_id);

    /**
     * @brief Check if the entities passed correspond to the specified entity kind.
     *
     * @param kind The expected EntityKind of the entities provided.
     * @param entity_ids Vector of entities whose kind is going to be checked
     * @param message Message to throw if the Entity does not exist in the database or if the kind does not correspond.
     *
     * @throws eprosima::statistics_backend::BadParameter If the Entity does not exist in the database or if the kind does not correspond.
     */
    void check_entity_kinds(
            EntityKind kind,
            const std::vector<EntityId>& entity_ids,
            const char* message);

    /**
     * @brief Check if the entities passed correspond to the specified entity kind (DISCOVERY_TIME case).
     *
     * @param kind The expected EntityKind of the entities provided.
     * @param entity_ids Vector of entities whose kind is going to be checked
     * @param message Message to throw if the Entity does not exist in the database or if the kind does not correspond.
     *
     * @throws eprosima::statistics_backend::BadParameter If the Entity does not exist in the database or if the kind does not correspond.
     */
    void check_entity_kinds(
            EntityKind kinds[3],
            const std::vector<EntityId>& entity_ids,
            const char* message);

protected:

    inline std::string id_to_string(
            EntityId id)
    {
        return std::to_string(id.value());
    }

    inline std::string time_to_string(
            std::chrono::system_clock::time_point time)
    {
        std::string s = std::to_string(
            std::chrono::duration_cast<std::chrono::nanoseconds>(time.time_since_epoch()).count());
        return s;
    }

    inline long long string_to_int(
            std::string const& str) const
    {
        if (str.find_first_not_of("-1234567890") != std::string::npos)
        {
            throw CorruptedFile("string must be an integer: " + str);
        }
        return stoll(str);
    }

    inline unsigned long long string_to_uint(
            std::string const& str) const
    {
        if (str.find_first_not_of("1234567890") != std::string::npos)
        {
            throw CorruptedFile("string must be an unsigned integer: " + str);
        }
        return stoull(str);
    }

    /**
     * @brief Auxiliar function to get the internal collection of DDSEndpoints of a specific type,
     * a.k.a DataReader or DataWriter.  This method is not thread safe.
     *
     * @tparam T The DDSEndpoint-derived class name. Only DataReader and DataWriter are allowed.
     * @return The corresponding internal collection, a.k.a datareaders_ or datawriters_.
     */
    template<typename T>
    std::map<EntityId, std::map<EntityId, std::shared_ptr<T>>>& dds_endpoints_nts();

    /**
     * Get all entities of a given EntityKind related to another entity. This method is not thread safe.
     *
     * In case the \c entity_id is EntityId::all(), all entities of type \c entity_type are returned.
     *
     * @param entity_id constant reference to the EntityId of the entity to which the returned
     *                  entities are related.
     * @param entity_kind The EntityKind of the fetched entities.
     * @throws eprosima::statistics_backend::BadParameter in the following case:
     *            * if the \c entity_kind is \c INVALID.
     *            * if the \c entity_id does not reference a entity contained in the database or is not EntityId::all().
     *            * if the EntityKind of the Entity with \c entity_id is \c INVALID.
     * @return A constant vector of shared pointers to the entities
     */
    const std::vector<std::shared_ptr<const Entity>> get_entities_nts(
            EntityKind entity_kind,
            const EntityId& entity_id) const;

    /**
     * @brief Get all entities of a given EntityKind related to another entity. This method is not thread safe.
     *
     * In case the \c entity is nullptr, all EntityIds of type \c entity_type are returned.
     *
     * @param entity constant reference to the entity to which the returned
     *               entities are related.
     * @param entity_kind The EntityKind of the fetched entities.
     * @throws eprosima::statistics_backend::BadParameter in the following case:
     *             * if the \c entity_kind is \c INVALID.
     *             * if the kind of the \c entity is \c INVALID.
     * @return A constant vector of shared pointers to the entities.
     */
    const std::vector<std::shared_ptr<const Entity>> get_entities_nts(
            EntityKind entity_kind,
            const std::shared_ptr<const Entity>& entity) const;

    /**
     * @brief Get GUID string given an EntityId. This method is not thread safe.
     *
     * @param entity_id The EntityId of the entity.
     * @throws eprosima::statistics_backend::BadParameter if there is no entity with
     * the given ID.
     * @return The GUID string of the given entity.
     */
    std::string get_entity_guid_nts(
            EntityId entity_id) const;

    /**
     * @brief Auxiliar function for boilerplate code to update a Locator with either a DataReader or a DataWriter using it. This method is not thread safe.
     *
     * @tparam T The DDSEndpoint to add to the Locator list. Only DDSEndpoint and its derived classes are allowed.
     * @param endpoint The endpoint of type T to add to the list of the locator.
     * @param locator The locator that will be updated with endpoint.
     * @return The EntityId of the inserted DDSEndpoint.
     */
    template<typename T>
    void insert_ddsendpoint_to_locator_nts(
            std::shared_ptr<T>& endpoint,
            std::shared_ptr<Locator>& locator);

    /**
     * @brief Auxiliar function for boilerplate code to insert either a DataReader or a DataWriter. This method is not thread safe.
     *
     * @tparam T The DDSEndpoint to insert. Only DDSEndpoint and its derived classes are allowed.
     * @param endpoint The endpoint of type T to add to the database.
     * @param entity_id The ID of the entity, passing an entity with EntityId::invalid() will generate a new one.
     * @throws eprosima::statistics_backend::BadParameter in the followng cases:
     *             * if the endpoint information is incomplete (name, QoS, GUID, and locators).
     *             * if the parent participant and/or topic does not exist.
     *             * if an endpoint with the same name and/or GUID already exists.
     */
    template<typename T>
    void insert_ddsendpoint_nts(
            std::shared_ptr<T>& endpoint,
            EntityId& entity_id)
    {
        /* Check that name is not empty */
        if (endpoint->name.empty())
        {
            throw BadParameter("Endpoint name cannot be empty");
        }

        /* Check that qos is not empty */
        if (endpoint->qos.empty())
        {
            throw BadParameter("Endpoint QoS cannot be empty");
        }

        /* Check that GUID is not empty */
        if (endpoint->guid.empty())
        {
            throw BadParameter("Endpoint GUID cannot be empty");
        }

        /* Check that locators is not empty */
        if (endpoint->locators.empty())
        {
            throw BadParameter("Endpoint locators cannot be empty");
        }

        /* Check that participant exists */
        bool participant_exists = false;
        auto domain_participants = participants_.find(endpoint->participant->domain->id);
        if (domain_participants != participants_.end())
        {
            for (const auto& participant_it : domain_participants->second)
            {
                if (endpoint->participant.get() == participant_it.second.get())
                {
                    participant_exists = true;
                    break;
                }
            }
        }

        if (!participant_exists)
        {
            throw BadParameter("Parent participant does not exist in the database");
        }

        /* Check that topic exists */
        bool topic_exists = false;
        auto domain_topics = topics_.find(endpoint->topic->domain->id);
        if (domain_topics != topics_.end())
        {
            for (const auto& topic_it : domain_topics->second)
            {
                if (endpoint->topic.get() == topic_it.second.get())
                {
                    topic_exists = true;
                    break;
                }
            }
        }

        if (!topic_exists)
        {
            throw BadParameter("Parent topic does not exist in the database");
        }

        /* Check that this is indeed a new endpoint and that its GUID is unique */
        for (const auto& endpoints_it: dds_endpoints_nts<T>())
        {
            for (const auto& endpoint_it : endpoints_it.second)
            {
                if (endpoint.get() == endpoint_it.second.get())
                {
                    throw BadParameter("Endpoint already exists in the database");
                }
                if (endpoint->guid == endpoint_it.second->guid)
                {
                    throw BadParameter(
                              "An endpoint with GUID '" + endpoint->guid + "' already exists in the database");
                }
            }
        }

        // Add id to the entity
        if (!entity_id.is_valid_and_unique())
        {
            entity_id = generate_entity_id();
        }
        else if (entity_id.value() >= next_id_)
        {
            next_id_ = entity_id.value() + 1;
        }
        endpoint->id = entity_id;

        /* Add endpoint to participant' collection */
        (*(endpoint->participant)).template ddsendpoints<T>()[endpoint->id] = endpoint;
        if (endpoint->is_virtual_metatraffic)
        {
            assert(nullptr == endpoint->participant->meta_traffic_endpoint);
            endpoint->participant->meta_traffic_endpoint = endpoint;
        }

        // Check if every locator is not repeated already in the database
        // This is a workaround for an error ocurred when two same locators arrive to the queue inside different
        // endpoints, and so both are inserted in the database when they are the same

        // First copy the map (there is no way to change key in a map inside a loop)
        std::map<EntityId, std::shared_ptr<Locator>> actual_locators_map;

        // Iterate over every locator in the current map (that may already exist in the database)
        for (auto& locator_it : endpoint->locators)
        {
            // See if we already know the locator
            auto locators_with_same_name = get_entities_by_name_nts(EntityKind::LOCATOR, locator_it.second->name);
            if (!locators_with_same_name.empty())
            {
                // It could be only one in the database with the same name
                assert(locators_with_same_name.size() == 1);

                // As the locator already exists, take it and use it for this endpoint
                std::shared_ptr<database::Locator> locator = std::const_pointer_cast<database::Locator>(
                    std::static_pointer_cast<const database::Locator>(
                        get_entity_nts(locators_with_same_name.front().second)));
                actual_locators_map[locator->id] = locator;
            }
            else
            {
                // Check whether this locator is repeated inside this same endpoint
                bool repeated = false;
                for (auto new_locators : actual_locators_map)
                {
                    if (new_locators.second->name == locator_it.second->name)
                    {
                        repeated = true;
                        break;
                    }
                }

                if (!repeated)
                {
                    // It is actually a new locator, so it is added to the new map
                    actual_locators_map[locator_it.first] = locator_it.second;
                }
            }
        }

        // Remove the current locator map as it will be filled in the following loop
        endpoint->locators.clear();

        /* Add to x_by_y_ collections and to locators_ */
        for (auto& locator_it : actual_locators_map)
        {
            /* Check that locator exists */
            if (locators_.find(locator_it.first) == locators_.end())
            {
                throw BadParameter("Locator does not exist in the database");
            }

            // Add endpoint to locator's collection
            insert_ddsendpoint_to_locator_nts(endpoint, locator_it.second);

            // Add this locator to the locators map
            endpoint->locators[locator_it.first] = locator_it.second;
        }

        /* Add endpoint to topics's collection */
        (*(endpoint->topic)).template ddsendpoints<T>()[endpoint->id] = endpoint;

        /* Insert endpoint in the database */
        dds_endpoints_nts<T>()[endpoint->participant->domain->id][endpoint->id] = endpoint;
    }

    /**
     * @brief Notifies the user about a new discovered locator.
     *
     * @param locator_id The ID of the discovered locator.
     */
    void notify_locator_discovery (
            const EntityId& locator_id);

    /**
     * @brief Get a dump of an Entity stored in the database.

     * @param entity Pointer to Entity to dump.
     * @return \c DatabaseDump Object representing the entity.
     */
    DatabaseDump dump_entity_(
            const std::shared_ptr<Host>& entity);
    DatabaseDump dump_entity_(
            const std::shared_ptr<User>& entity);
    DatabaseDump dump_entity_(
            const std::shared_ptr<Process>& entity);
    DatabaseDump dump_entity_(
            const std::shared_ptr<Domain>& entity);
    DatabaseDump dump_entity_(
            const std::shared_ptr<Topic>& entity);
    DatabaseDump dump_entity_(
            const std::shared_ptr<DomainParticipant>& entity);
    DatabaseDump dump_entity_(
            const std::shared_ptr<DataWriter>& entity);
    DatabaseDump dump_entity_(
            const std::shared_ptr<DataReader>& entity);
    DatabaseDump dump_entity_(
            const std::shared_ptr<Locator>& entity);

    /**
     * @brief Get a dump of data stored in the database.

     * @param data Reference to a data container.
     * @return \c DatabaseDump Object representing the data.
     */
    DatabaseDump dump_data_(
            const std::map<EntityId, details::DataContainer<ByteCountSample>>& data);
    DatabaseDump dump_data_(
            const std::map<EntityId, details::DataContainer<EntityCountSample>>& data);
    DatabaseDump dump_data_(
            const std::map<EntityId, details::DataContainer<EntityDataSample>>& data);
    DatabaseDump dump_data_(
            const std::map<EntityId, details::DataContainer<DiscoveryTimeSample>>& data);
    DatabaseDump dump_data_(
            const std::map<uint64_t, details::DataContainer<EntityCountSample>>& data);
    DatabaseDump dump_data_(
            const details::DataContainer<EntityCountSample>& data);
    DatabaseDump dump_data_(
            const details::DataContainer<EntityDataSample>& data);
    DatabaseDump dump_data_(
            const EntityCountSample& data);
    DatabaseDump dump_data_(
            const ByteCountSample& data);
    DatabaseDump dump_data_(
            const std::map<EntityId, EntityCountSample>& data);
    DatabaseDump dump_data_(
            const std::map<EntityId, ByteCountSample>& data);

    /**
     * @brief Remove the statistics data of the database. This not include the info or discovery data. This method is not thread safe.
     *
     * @param t_to Timestamp regarding the maximum time to stop removing data.
     *
     * @warning This method does not guard a mutex, as it is expected to be called with mutex already taken.
     */
    void clear_statistics_data_nts_(
            const Timestamp& t_to);

    /**
     * @brief Remove all inactive entities of the database. This does not include domains.
     *
     * This deletion of entities is done in an atomical way, so no other interaction with the
     * database may occur while deleting or coherence cannot be assured.
     *
     * @warning This method does not guard a mutex, as it is expected to be called with mutex already taken.
     */
    void clear_inactive_entities_nts_();

    /**
     * @brief Insert a new entity into the database. This method is not thread safe.
     * @param entity The entity object to be inserted.
     * @param entity_id The ID of the entity, passing an entity with EntityId::invalid() will generate a new one.
     * @throws eprosima::statistics_backend::BadParameter in the following case:
     *             * If the entity already exists in the database.
     *             * If the parent entity does not exist in the database (expect for the case of
     *               a Domainparticipant entity, for which an unregistered parent process is allowed).
     *             * If the entity name is empty.
     *             * Depending on the type of entity, if some other identifier is empty.
     *             * For entities with GUID, if the GUID is not unique.
     *             * For entities with QoS, if the QoS is empty.
     *             * For entities with locators, if the locators' collection is empty.
     */
    void insert_nts(
            const std::shared_ptr<Entity>& entity,
            EntityId& entity_id);


    /**
     * @brief Process physical entities (Host, User, Process) related to a newly discovered participant. This method is not thread safe.
     * @param host_name The name of the host.
     * @param user_name The name of the user.
     * @param process_name The name of the process.
     * @param process_pid The PID of the process.
     * @param discovery_source The DiscoverySource of the participant.
     * @param should_link_process_participant Set to true if the process must be linked to the participant.
     * @param participant_id The EntityId of the participant.
     * @param physical_entities_ids Map where host-user-process EntityIds are stored when inserted in database.
     */
    void process_physical_entities_nts(
            const std::string& host_name,
            const std::string& user_name,
            const std::string& process_name,
            const std::string& process_pid,
            DiscoverySource discovery_source,
            bool& should_link_process_participant,
            const EntityId& participant_id,
            std::map<std::string, EntityId>& physical_entities_ids);

    /**
     * @brief Create new Endpoint. This method is not thread safe.
     * @param endpoint_guid The GUID of the Endpoint.
     * @param name The name of the Endpoint.
     * @param qos The QoS of the Endpoint.
     * @param participant The DomainParticipant related to the Endpoint.
     * @param topic The Topic related to the Endpoint.
     * @param app_id The AppId related to the Endpoint.
     * @param app_metadata The app metadata related to the Endpoint.
     * @param discovery_source The DiscoverySource related to the Endpoint.
     * @param original_domain The DomainId of the original domain where the Endpoint has been discovered.
     *
     * @return EntityId of the Endpoint once inserted.
     */
    template <typename T>
    std::shared_ptr<database::DDSEndpoint> create_endpoint_nts(
            const std::string& endpoint_guid,
            const std::string& name,
            const Qos& qos,
            const std::shared_ptr<DomainParticipant>& participant,
            const std::shared_ptr<Topic>& topic,
            const AppId& app_id,
            const std::string& app_metadata,
            DiscoverySource discovery_source,
            DomainId original_domain);

    /**
     * @brief Get the locator with id \c entity_id. This method is not thread safe.
     *
     * If the locator does not exist yet, create and insert it into the database.
     *
     * @param entity_id The EntityId to the locator to be found or inserted.
     * @throws eprosima::statistics_backend::BadParameter in the following case:
     *             * If the entity already exists in the database.
     *             * If the parent entity does not exist in the database (expect for the case of
     *               a Domainparticipant entity, for which an unregistered parent process is allowed).
     *             * If the entity name is empty.
     *             * Depending on the type of entity, if some other identifier is empty.
     *             * For entities with GUID, if the GUID is not unique.
     *             * For entities with QoS, if the QoS is empty.
     *             * For entities with locators, if the locators' collection is empty.
     */
    std::shared_ptr<Locator>  get_locator_nts(
            EntityId const& entity_id);

    /**
     * @brief Insert a new statistics sample into the database. This method is not thread safe.
     *
     * @param domain_id The EntityId of the domain that contains the entity.
     * @param entity_id The EntityId to which the sample relates.
     * @param sample The sample to be inserted.
     * @param loading Is a insert coming from loading a database dump.
     * @param last_reported Is a insert of a last_reported data.
     * @throws eprosima::statistics_backend::BadParameter in the following cases:
     *             * If the \c domain_id does not refer to a known domain.
     *             * If the \c entity_id does not refer to a known entity.
     *             * If the \c sample kind is DataKind::INVALID.
     */
    void insert_nts(
            const EntityId& domain_id,
            const EntityId& entity_id,
            const StatisticsSample& sample,
            const bool loading = false,
            const bool last_reported = false);

    /**
     * @brief Triggers all the alerts of a specific kind if the entity and the data
     * meet the conditions. This method is not thread safe.
     *
     * @param domain_id The EntityId of the domain that contains the triggerer entity.
     * @param entity_id The EntityId of the entity for which to trigger the alerts.
     * @param endpoint The DDSEndpoint for which to trigger the alerts
     * @param alert_kind The kind of alert to trigger.
     * @param data The value that might trigger the alert
     */
    void trigger_alerts_of_kind_nts(
            const EntityId& domain_id,
            const EntityId& entity_id,
            const std::shared_ptr<DDSEndpoint>& endpoint,
            const AlertKind alert_kind,
            const EntityCountSample& data);

    /**
     * @brief Triggers all the alerts of a specific kind if the entity and the data
     * meet the conditions. This method is not thread safe.
     *
     * @param domain_id The EntityId of the domain that contains the triggerer entity.
     * @param entity_id The EntityId of the entity for which to trigger the alerts.
     * @param endpoint The DDSEndpoint for which to trigger the alerts
     * @param alert_kind The kind of alert to trigger.
     * @param data The value that might trigger the alert
     */
    void trigger_alerts_of_kind_nts(
            const EntityId& domain_id,
            const EntityId& entity_id,
            const std::shared_ptr<DDSEndpoint>& endpoint,
            const AlertKind alert_kind,
            const EntityDataSample& data);

    /**
     * @brief Insert a new monitor service sample into the database. This method is not thread safe.
     *
     * @param domain_id The EntityId of the domain that contains the entity.
     * @param entity_id The EntityId to which the sample relates.
     * @param sample The sample to be inserted.
     * @throws eprosima::statistics_backend::BadParameter in the following cases:
     *             * If the \c domain_id does not refer to a known domain.
     *             * If the \c entity_id does not refer to a known entity.
     *             * If the \c sample kind is StatusKind::INVALID.
     * @return True if the entity status has been modified.
     */
    bool insert_nts(
            const EntityId& domain_id,
            const EntityId& entity_id,
            const MonitorServiceSample& sample);

    /**
     * @brief Get all entities of a given EntityKind that match with the requested name. This method is not thread safe.
     *
     * @param entity_kind The EntityKind of the fetched entities.
     * @param name The name of the entities for which to search.
     * @throws eprosima::statistics_backend::BadParameter if \c entity_kind is not valid.
     * @return A vector of pairs, where the first field is the EntityId of the Domain of the matching entities,
     *         and the second is the EntityId of the matching entities. For physical entities (Host, User, Process,
     *         Locator) the returned Domain EntityId is EntityId::INVALID, as it has no meaning since these entities
     *         do not belong to a Domain.
     */
    std::vector<std::pair<EntityId, EntityId>> get_entities_by_name_nts(
            EntityKind entity_kind,
            const std::string& name) const;

    /**
     * @brief Get the alert entity with the given ID. This method is not thread safe.
     */
    const std::shared_ptr<const AlertInfo> get_alert_nts(
            const AlertId& alert_id) const;

    /**
     * @brief Get the notifier with the given ID. This method is not thread safe.
     */
    const std::shared_ptr<const Notifier> get_notifier_nts(
            const NotifierId& notifier_id) const;

    /**
     * @brief Get the type IDL of a given type name, if it exists. This method is not thread safe.
     *
     * @param type_name The name of the data type for which to search.
     * @throws eprosima::statistics_backend::BadParameter if \c type_name does not exist in the database.
     * @return The IDL representation of the type in std::string format.
     */
    std::string get_type_idl_nts(
            const std::string& type_name) const;

    /**
     * @brief Get the demangled type name of a given type, if it exists, for display purposes. This method is not thread safe.
     *
     * @param type_name The name of the data type for which to search.
     * @throws eprosima::statistics_backend::BadParameter if \c type_name does not exist in the database.
     * @return The name type in std::string format.
     */
    std::string get_ros2_type_name_nts(
            const std::string& type_name) const;

    /**
     * @brief Get the original ROS 2 type IDL of a given type name, if it exists.  This method is not thread safe.
     *
     * @param type_name The name of the data type for which to search.
     * @throws eprosima::statistics_backend::BadParameter if \c type_name does not exist in the database.
     * @return The original ROS 2 IDL representation of the type in std::string format.
     */
    std::string get_ros2_type_idl_nts(
            const std::string& type_name) const;

    /**
     * @brief Get the entity of a given EntityKind that matches with the requested GUID. This method is not thread safe.
     *
     * @param entity_kind The EntityKind of the fetched entities.
     * @param guid The GUID of the entities to search for.
     * @throws eprosima::statistics_backend::BadParameter in the following cases:
     *             * if the given EntityKind does not contain a GUID.
     *             * if there is no entity with the given parameters.
     * @return A pair, where the first field is the EntityId of the Domain of the matching entities,
     *         and the second is the EntityId of the matching entity.
     */
    std::pair<EntityId, EntityId> get_entity_by_guid_nts(
            EntityKind entity_kind,
            const std::string& guid) const;

    /**
     * @brief Check if the entity status should be updated depending on its monitor service status data
     *
     * @param entity The Entity that might be updated.
     * @throws eprosima::statistics_backend::BadParameter if there is no specialization template for the requested EntityKind.
     * @return True if entity has been updated
     */
    template <typename T>
    bool update_entity_status_nts(
            std::shared_ptr<T>& entity)
    {
        static_cast<void>(entity);
        throw BadParameter("Unsupported EntityKind");
    }

    /**
     * @brief Update entity status according to warnings and errors. This method is not thread safe.
     *
     * @param entity_error Flag showing if there is any error status data in the entity.
     * @param entity_warning Flag showing if there is any warning status data in the entity.
     * @param entity_status Reference to the status of the entity.
     * @return True if the status has changed.
     */
    bool entity_status_logic_nts(
            const bool& entity_error,
            const bool& entity_warning,
            StatusLevel& entity_status);

    /**
     * @brief Update the current QoS of a existing entity according to the received QoS.
     *
     * @note When entity QoS is updated, current and received QoS are merged:
     *       new keys are added to the entity QoS JSON, and existing values are preserved
     *       unless explicitly overwritten.
     * @param entity The ID of the entity to be updated.
     * @param received_qos The new QoS information used to update the entity.
     * @throws eprosima::statistics_backend::BadParameter in the following cases:
     *            * If an entity with the given ID does not exist in the database.
     *            * If the entity exists in the database, but it is not a valid DDS Endpoint (i.e: It does not store QoS policies).
     * @return True if entity QoS has been updated,
     *         false if not (i.e: if optional QoS information was already received).
     */
    bool update_entity_qos_nts(
            const EntityId& entity,
            const Qos& received_qos);


    bool update_participant_discovery_info_nts(
            const EntityId& participant_id,
            const std::string& host,
            const std::string& user,
            const std::string& process,
            const std::string& name,
            const Qos& qos,
            const std::string& guid,
            const EntityId& domain_id,
            const StatusLevel& status,
            const AppId& app_id,
            const std::string& app_metadata,
            DiscoverySource discovery_source,
            DomainId original_domain);

    /**
     * Get an entity given its EntityId. This method is not thread safe.
     *
     * @param entity_id constant reference to the EntityId of the retrieved entity.
     * @throws eprosima::statistics_backend::BadParameter if there is no entity with the given ID.
     * @return A constant shared pointer to the Entity.
     */
    const std::shared_ptr<const Entity> get_entity_nts(
            const EntityId& entity_id) const;

    /**
     * @brief Get a mutable entity given its EntityId. This method is not thread safe.
     * @param entity_id constant reference to the EntityId of the retrieved entity.
     * @throws eprosima::statistics_backend::BadParameter if there is no entity with the given ID.
     * @return A shared pointer to the Entity.
     */
    const std::shared_ptr<Entity> get_mutable_entity_nts(
            const EntityId& entity_id);

    /**
     * @brief Get the specified domain view graph from database. This method is not thread safe.
     *
     * @param domain Domain from which graph is delivered.
     * @throws eprosima::statistics_backend::BadParameter if there is no graph for the specified domain id.
     * @return Graph object describing topology of the entities in the domain.
     */
    Graph get_domain_view_graph_nts(
            const EntityId& domain_id) const;

    /**
     * @brief Init domain view graph with specified domain. This method is not thread safe.
     *
     * @param domain_name Domain where monitoring.
     * @param domain_entity_id The EntityId of the domain.
     */
    void init_domain_view_graph_nts(
            const std::string& domain_name,
            const DomainId domain_id,
            const EntityId& domain_entity_id);

    /**
     * @brief Update host-user-process-participant in domain view graph after participant entity discovery. This method is not thread safe.
     *
     * @param domain_entity_id The EntityId of the domain.
     * @param host_entity_id The EntityId of the host.
     * @param user_entity_id The EntityId of the user.
     * @param process_entity_id The EntityId of the process.
     * @param participant_entity_id The EntityId of the participant.
     *
     * @return True if graph has been updated
     */
    bool update_participant_in_graph_nts(
            const EntityId& domain_entity_id,
            const EntityId& host_entity_id,
            const EntityId& user_entity_id,
            const EntityId& process_entity_id,
            const EntityId& participant_entity_id);

    /**
     * @brief Update topic-endpoint in domain view graph after endpoint entity discovery. This method is not thread safe.
     *
     * @param domain_entity_id The EntityId of the domain.
     * @param participant_entity_id The EntityId of the participant.
     * @param topic_entity_id The EntityId of the topic.
     * @param endpoint_entity_id The EntityId of the endpoint.
     *
     * @return True if graph has been updated
     */
    bool update_endpoint_in_graph_nts(
            const EntityId& domain_entity_id,
            const EntityId& participant_entity_id,
            const EntityId& topic_entity_id,
            const EntityId& endpoint_entity_id);

    /**
     * @brief Regenerate graph from data stored in database. This method is not thread safe.
     *
     * @param domain_entity_id The EntityId of the domain.
     *
     * @return True if the graph has been regenerated
     */
    bool regenerate_domain_graph_nts(
            const EntityId& domain_entity_id);

    /**
     * @brief Update graph according to the updated entity.This method is not thread safe.
     *
     * @param domain_id The EntityId of the domain containing the entity.
     * @param entity_id The EntityId of the entity updated.
     *
     * @return True if the graph has been updated
     */
    bool update_graph_on_updated_entity_nts(
            const EntityId& domain_id,
            const EntityId& entity_id);

    /**
     * @brief Get entity subgraph. This method is not thread safe.
     *
     * @param entity_id The EntityId from the entity which asks for its subgraph.
     * @param entity_graph Subgraph where to add entity data.
     *
     */
    Graph get_entity_subgraph_nts(
            const EntityId& entity_id,
            Graph& entity_graph);

    /**
     * @brief Setter for entity alias. This method is not thread safe.
     *
     * @param entity_id The EntityId of the entity updated.
     * @param alias The new alias of the entity.
     *
     */
    void set_alias_nts(
            const EntityId& entity_id,
            const std::string& alias);

    /**
     * @brief Setter for entity alert.
     *
     * @param alert_info The new alert information.
     * @return The AlertId of the alert.
     */
    AlertId insert_alert_nts(
            AlertInfo& alert_info);

    /**
     * @brief Create the link between a participant and a process. This method is not thread safe.
     *
     * This operation entails:
     *     1. Referencing the process from the participant.
     *     2. Adding the participant to the process' list of participants.
     *
     * @param participant_id The EntityId of the participant.
     * @param process_id The EntityId of the process.
     * @throw eprosima::statistics_backend::BadParameter in the following cases:
     *            * The participant is already linked with a process.
     *            * The participant does not exist in the database.
     *            * The process does not exist in the database.
     */
    void link_participant_with_process_nts(
            const EntityId& participant_id,
            const EntityId& process_id);

    /**
     * @brief Load data from a dump.

     * @param dump Reference to the .json with the info of the data.
     * @param entity Reference to the entity where the info will be inserted.
     * @throws eprosima::statistics_backend::BadParameter in the following cases:
     *             * If the \c domain_id does not refer to a known domain.
     *             * If the \c entity_id does not refer to a known entity.
     *             * If the \c sample kind is DataKind::INVALID.
     */
    void load_data(
            const DatabaseDump& dump,
            const std::shared_ptr<DomainParticipant>& entity);
    void load_data(
            const DatabaseDump& dump,
            const std::shared_ptr<DataWriter>& entity);
    void load_data(
            const DatabaseDump& dump,
            const std::shared_ptr<DataReader>& entity);

    /**
     * Change the status (active/inactive) of an entity given an EntityId.
     * Also check if the references of the entity must also be changed and change the status in that case.
     * A call to the user listeners will be performed if the status of the entity changes.
     * @param entity_id The EntityId of the entity
     * @param active The entity status value to set
     * @param entity_kind The EntityKind of the entity
     * @param domain_id The entityId of the domain
     */
    void change_entity_status_of_kind(
            const EntityId& entity_id,
            bool active,
            const EntityKind& entity_kind,
            const EntityId& domain_id = EntityId::invalid()) noexcept;

    /**
     * @brief Run a function without the database mutex taken.
     *
     * If the mutex is taken, unlock it previous to execution, and lock it again afterwards.
     * If the mutex is not taken, execute the function and do not lock afterwards.
     *
     * @param lambda The function to be run without protection.
     */
    template<typename Functor>
    void execute_without_lock(
            const Functor& lambda) noexcept;

    /**
     * @brief Clear the internal references of each element inside the database to those referenced elements that no
     * longer exist.
     *
     * This is done after clearing entities in the database.
     * It could happen that an still existing entity in the database references (with a fragile ptr) an erased one,
     * thus this method checks every still active entity and removes these references.
     *
     * The entities that could keep a non valid reference are:
     * - process -> participant
     * - user -> process
     * - host -> user
     * - participant -> endpoint
     * - topic -> endpoint
     * - domain -> topic & participants
     *
     * @warning This method does not guard a mutex, as it is expected to be called with mutex already taken.
     */
    void clear_internal_references_nts_();

    /**
     * @brief Convert a StatisticsSample to a string representation.
     *
     * @param sample The StatisticsSample to convert.
     * @return The string representation of the sample.
     */
    std::string convert_stat_to_string(
            const StatisticsSample& sample) const;

    //! Collection of Hosts sorted by EntityId
    std::map<EntityId, std::shared_ptr<Host>> hosts_;

    //! Collection of Users sorted by EntityId
    std::map<EntityId, std::shared_ptr<User>> users_;

    //! Collection of Processes sorted by EntityId
    std::map<EntityId, std::shared_ptr<Process>> processes_;

    //! Collection of Locators sorted by EntityId
    std::map<EntityId, std::shared_ptr<Locator>> locators_;

    //! Collection of Domains sorted by EntityId
    std::map<EntityId, std::shared_ptr<Domain>> domains_;

    /**
     * Collection of DomainParticipants sorted by EntityId of the domain to which they belong
     *
     * Each value in the collection is in turn a map of the actual DomainParticipants sorted by EntityId
     */
    std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>> participants_;

    /**
     * Collection of DataWriters sorted by EntityId of the domain to which they belong
     *
     * Each value in the collection is in turn a map of the actual DataWriters sorted by EntityId
     */
    std::map<EntityId, std::map<EntityId, std::shared_ptr<DataWriter>>> datawriters_;

    /**
     * Collection of DataReaders sorted by EntityId of the domain to which they belong
     *
     * Each value in the collection is in turn a map of the actual DataReaders sorted by EntityId
     */
    std::map<EntityId, std::map<EntityId, std::shared_ptr<DataReader>>> datareaders_;

    /**
     * Collection of Topics sorted by EntityId of the domain to which they belong
     *
     * Each value in the collection is in turn a map of the actual Topics sorted by EntityId
     */
    std::map<EntityId, std::map<EntityId, std::shared_ptr<Topic>>> topics_;

    /**
     * Collection of Alerts, cannot be indexed by EntityId as they correlated entities
     * may not exist yet in the time of creation
     */
    std::map<EntityId, std::map<AlertId, std::shared_ptr<AlertInfo>>> alerts_;

    /**
     * Collection of Notifiers sorted by NotifierId
     */
    NotifierManager notifiers_;

    /**
     * Collection of topic IDLs sorted by topic data types, with which they are biunivocally identified.
     * This is used to store the IDLs of the discovered topics
     *
     * Each value in the collection is in turn a map of the actual Topic IDLs sorted by data type
     */
    std::map<std::string, std::string> type_idls_;

    /**
     * Collection of type names relating the original name and the ROS 2 demangled name.
     * Only those types that have been modified are stored.
     */
    std::map<std::string, std::string> type_ros2_modified_name_;

    /**
     * Collection of type idls relating the original idl and the original name.
     * Note that demangling is done by default, so the demangled IDL is stored in the main map.
     * Only those types that have been modified are stored.
     */
    std::map<std::string, std::string> type_ros2_unmodified_idl_;

    //! Graph map describing per domain complete topology of the entities.
    std::map<EntityId, Graph> domain_view_graph;

    /**
     * The ID that will be assigned to the next entity.
     * Used to guarantee a unique EntityId within the database instance
     */
    std::atomic<int64_t> next_id_{0};

    /**
     * The ID that will be assigned to the next alert.
     * Used to guarantee a unique AlertId within the database instance
     */
    std::atomic<uint32_t> next_alert_id_{0};

    //! Read-write synchronization mutex
    mutable std::shared_timed_mutex mutex_;
};

template<>
void Database::insert_ddsendpoint_to_locator_nts(
        std::shared_ptr<DataWriter>& endpoint,
        std::shared_ptr<Locator>& locator);

template<>
void Database::insert_ddsendpoint_to_locator_nts(
        std::shared_ptr<DataReader>& endpoint,
        std::shared_ptr<Locator>& locator);

template <>
bool Database::update_entity_status_nts(
        std::shared_ptr<DomainParticipant>& entity);

template <>
bool Database::update_entity_status_nts(
        std::shared_ptr<DataReader>& entity);

template <>
bool Database::update_entity_status_nts(
        std::shared_ptr<DataWriter>& entity);

template <>
void Database::get_status_data(
        const EntityId& entity_id,
        ProxySample& status_data);
template <>
void Database::get_status_data(
        const EntityId& entity_id,
        ConnectionListSample& status_data);
template <>
void Database::get_status_data(
        const EntityId& entity_id,
        IncompatibleQosSample& status_data);
template <>
void Database::get_status_data(
        const EntityId& entity_id,
        InconsistentTopicSample& status_data);
template <>
void Database::get_status_data(
        const EntityId& entity_id,
        LivelinessLostSample& status_data);
template <>
void Database::get_status_data(
        const EntityId& entity_id,
        LivelinessChangedSample& status_data);
template <>
void Database::get_status_data(
        const EntityId& entity_id,
        DeadlineMissedSample& status_data);
template <>
void Database::get_status_data(
        const EntityId& entity_id,
        SampleLostSample& status_data);
template <>
void Database::get_status_data(
        const EntityId& entity_id,
        ExtendedIncompatibleQosSample& status_data);

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif // FASTDDS_STATISTICS_BACKEND_SRC_CPP_DATABASE__DATABASE_HPP
