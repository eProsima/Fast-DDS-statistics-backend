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

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_DATABASE_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_DATABASE_HPP_

#include <atomic>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <type_traits> // enable_if, is_integral

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/exception/Exception.hpp>

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
     * Get an entity given its EntityId
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
     * @brief Get EntityKind given an EntityId.
     *
     * @param entity_id The EntityId of the entity.
     * @throws eprosima::statistics_backend::BadParameter if there is no entity with the given ID.
     * @return The EntityKind of the given entity.
     */
    EntityKind get_entity_kind(
            EntityId entity_id) const;

    /**
     * @brief Get EntityStatus given an EntityId.
     *
     * @param entity_id The EntityId of the entity.
     * @throws eprosima::statistics_backend::BadParameter if there is no entity with the given ID.
     * @return The EntityStatus of the given entity.
     */
    EntityStatus get_entity_status(
            EntityId entity_id) const;

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
     * @param active The entity status value to set
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
     * a.k.a DataReader or DataWriter.
     *
     * @tparam T The DDSEndpoint-derived class name. Only DataReader and DataWriter are allowed.
     * @return The corresponding internal collection, a.k.a datareaders_ or datawriters_.
     */
    template<typename T>
    std::map<EntityId, std::map<EntityId, std::shared_ptr<T>>>& dds_endpoints();

    /**
     * Get all entities of a given EntityKind related to another entity.
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
    const std::vector<std::shared_ptr<const Entity>> get_entities(
            EntityKind entity_kind,
            const std::shared_ptr<const Entity>& entity) const;

    /**
     * @brief Auxiliar function for boilerplate code to update a Locator with either a DataReader or a DataWriter using it.
     *
     * @tparam T The DDSEndpoint to add to the Locator list. Only DDSEndpoint and its derived classes are allowed.
     * @param endpoint The endpoint of type T to add to the list of the locator.
     * @param locator The locator that will be updated with endpoint.
     * @return The EntityId of the inserted DDSEndpoint.
     */
    template<typename T>
    void insert_ddsendpoint_to_locator(
            std::shared_ptr<T>& endpoint,
            std::shared_ptr<Locator>& locator);

    /**
     * @brief Auxiliar function for boilerplate code to insert either a DataReader or a DataWriter.
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
    void insert_ddsendpoint(
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
        for (const auto& endpoints_it: dds_endpoints<T>())
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
            auto locators_with_same_name = get_entities_by_name(EntityKind::LOCATOR, locator_it.second->name);
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
            insert_ddsendpoint_to_locator(endpoint, locator_it.second);

            // Add this locator to the locators map
            endpoint->locators[locator_it.first] = locator_it.second;
        }

        /* Add endpoint to topics's collection */
        (*(endpoint->topic)).template ddsendpoints<T>()[endpoint->id] = endpoint;

        /* Insert endpoint in the database */
        dds_endpoints<T>()[endpoint->participant->domain->id][endpoint->id] = endpoint;
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
     * @brief Remove the statistics data of the database. This not include the info or discovery data.
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
     * Get an entity given its EntityId. This method is not thread safe.
     *
     * @param entity_id constant reference to the EntityId of the retrieved entity.
     * @throws eprosima::statistics_backend::BadParameter if there is no entity with the given ID.
     * @return A constant shared pointer to the Entity.
     */
    const std::shared_ptr<const Entity> get_entity_nts(
            const EntityId& entity_id) const;

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
     * The ID that will be assigned to the next entity.
     * Used to guarantee a unique EntityId within the database instance
     */
    std::atomic<int64_t> next_id_{0};

    //! Read-write synchronization mutex
    mutable std::shared_timed_mutex mutex_;
};

template<>
void Database::insert_ddsendpoint_to_locator(
        std::shared_ptr<DataWriter>& endpoint,
        std::shared_ptr<Locator>& locator);

template<>
void Database::insert_ddsendpoint_to_locator(
        std::shared_ptr<DataReader>& endpoint,
        std::shared_ptr<Locator>& locator);

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_DATABASE_HPP_
