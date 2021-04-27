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
#include <shared_mutex>
#include <memory>

#include <fastdds-statistics-backend/exception/Exception.hpp>
#include <fastdds-statistics-backend/types/EntityId.hpp>
#include <fastdds-statistics-backend/exception/Exception.hpp>

#include "entities.hpp"

namespace eprosima {
namespace statistics_backend {
namespace database {

class Database
{
public:

    /**
     * @brief Insert a new entity into the database.
     * @param entity The entity object to be inserted.
     * @throws eprosima::statistics_backend::BadParameter in the following case:
     *             * If the entity already exists in the database
     *             * If the parent entity does not exist in the database (expect for the case of
     *               a Domainparticipant entity, for which an unregistered parent process is allowed)
     *             * If the entity name is empty
     *             * Depending on the type of entity, if some other identifier is empty
     *             * For entities with GUID, if the GUID is not unique
     *             * For entities with QoS, if the QoS is empty
     *             * For entities with locators, if the locators' collection is empty
     * @return The EntityId of the inserted entity
     */
    EntityId insert(
            const std::shared_ptr<Entity>& entity);

    /**
     * @brief Insert a new statistics sample into the database.
     * @param domain_id The EntityId to the domain that contains the entity
     * @param entity_id The EntityId to which the sample relates.
     * @param sample The sample to be inserted.
     */
    void insert(
            const EntityId& domain_id,
            const EntityId& entity_id,
            const StatisticsSample& sample);

    /**
     * @brief Create the link between a participant and a process
     *
     * This operation entails:
     *     1. Adding reference to process to the participant
     *     2. Adding the participant to the process' list of participants
     *     3. Adding entry to domains_by_process_
     *     4. Adding entry to processes_by_domain_
     *
     * @param participant_id The EntityId of the participant
     * @param process_id The EntityId of the process
     * @throw eprosima::statistics_backend::BadParameter in the following cases:
     *            * The participant is already linked with a process
     *            * The participant does not exist in the database
     *            * The process does not exist in the database
     */
    void link_participant_with_process(
            const EntityId& participant_id,
            const EntityId& process_id);

    /**
     * @brief Erase all the data related to a domain
     *
     * After the operation, the domain_id becomes invalid.
     *
     * @param domain_id The EntityId of the domain to be erased.
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
     * @param data_type The type of the measurement being requested
     * @param entity_id_source Id of the source entity of the requested data
     * @param entity_id_target Id of the target entity of the requested data
     * @param t_from Starting time of the returned measures.
     * @param t_to Ending time of the returned measures.
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
     * For data types that relate to a single entity,
     * use the overloaded function that takes a single entity as argument.
     *
     * \par Measurement time and intervals
     *
     * \c t_from and \c t_to define the time interval for which the measurements will be returned.
     *
     * \sa Database
     *
     * @param data_type The type of the measurement being requested
     * @param entity_id Id of entity of the requested data
     * @param t_from Starting time of the returned measures.
     * @param t_to Ending time of the returned measures.
     * @return A vector of pointers to StatisticSamples.
     */
    std::vector<const StatisticsSample*> select(
            DataKind data_type,
            EntityId entity_id,
            Timestamp t_from,
            Timestamp t_to);

    /**
     * Get an entity given its EntityId
     *
     * @param entity_id constant reference to the EntityId of the retrieved entity
     * @throws eprosima::statistics_backend::BadParameter if there is not entity with the given ID.
     * @return A constant shared pointer to the Entity
     */
    const std::shared_ptr<const Entity> get_entity(
            const EntityId& entity_id) const;

    /**
     * Get all entities of a given EntityKind related to another entity
     *
     * @param entity_id constant reference to the EntityId of the entity to which the returned
     *                  entities are related
     * @param entity_kind The EntityKind of the fetched entities
     * @return A constant vector of shared pointers to the entities
     */
    const std::vector<std::shared_ptr<const Entity>> get_entities(
            EntityKind entity_kind,
            const EntityId& entity_id) const;

    /**
     * Get all entities of a given EntityKind related to another entity
     *
     * @param entity_id constant reference to the EntityId of the entity to which the returned
     *                  entities are related
     * @param entity_kind The EntityKind of the fetched entities
     * @return A vector containing the EntityIds of the entities
     */
    std::vector<EntityId> get_entity_ids(
            EntityKind entity_type,
            const EntityId& entity_id) const;

    /**
     *  @brief Generate an EntityId that is unique for the database
     *
     * @return The unique EntityId
     */
    EntityId generate_entity_id() noexcept;

    /**
     * Get all entities of a given EntityKind that match with the requested name
     *
     * @param entity_kind The EntityKind of the fetched entities
     * @param name The name of the entities for which to search
     * @throws eprosima::statistics_backend::BadParameter if entity_kind is not valid
     * @return A vector of pairs, where the first field is the EntityId of the Domain of the matching entities,
     *         and the second is the EntityId of the matching entities. For physical entities (Host, User, Process,
     *         Locator) the returned Domain EntityId is EntityId::INVALID, as it has no meaning since these entities
     *         do not belong to a Domain.
     */
    std::vector<std::pair<EntityId, EntityId>> get_entities_by_name(
            EntityKind entity_kind,
            const std::string& name) const;

    /**
     * Get all entities of a given EntityKind that match with the requested GUID
     *
     * If the given EntityKind does not contain a GUID,
     * BadParameter is thrown.
     *
     * @param entity_kind The EntityKind of the fetched entities
     * @param guid The GUID of the entities to search for
     * @return A vector of pairs, where the first field is the EntityId of the Domain of the matching entities,
     *         and the second is the EntityId of the matching entities. For physical entities (Host, User, Process,
     *         Locator) the returned Domain EntityId is EntityId::INVALID, as it has no meaning since these entities
     *         do not belong to a Domain.
     */
    std::vector<std::pair<EntityId, EntityId>> get_entities_by_guid(
            EntityKind entity_kind,
            const std::string& guid) const;

protected:

    /**
     * @brief Auxiliar function to get the internal collection of DDSEndpoints of a specific type,
     * a.k.a DataReader or DataWriter.
     *
     * @tparam T The DDSEndpoint-derived class name. Only DataReader and DataWriter are allowed.
     * @return The corresponding internal collection, a.k.a datareaders_ or datawriters_
     */
    template<typename T>
    std::map<EntityId, std::map<EntityId, std::shared_ptr<T>>>& dds_endpoints();

    /**
     * @brief Auxiliar function for boilerplate code to insert either a DataReader or a DataWriter
     *
     * @tparam T The DDSEndpoint to insert. Only DDSEndpoint and its derived classes are allowed.
     * @param endpoint
     * @return The EntityId of the inserted DDSEndpoint
     */
    template<typename T>
    EntityId insert_ddsendpoint(
            std::shared_ptr<T>& endpoint)
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

        /* Check that participant exits */
        bool participant_exists = false;
        for (auto participant_it : participants_[endpoint->participant->domain->id])
        {
            if (endpoint->participant.get() == participant_it.second.get())
            {
                participant_exists = true;
                break;
            }
        }

        if (!participant_exists)
        {
            throw BadParameter("Parent participant does not exist in the database");
        }

        /* Check that topic exits */
        bool topic_exists = false;
        for (auto topic_it : topics_[endpoint->topic->domain->id])
        {
            if (endpoint->topic.get() == topic_it.second.get())
            {
                topic_exists = true;
                break;
            }
        }

        if (!topic_exists)
        {
            throw BadParameter("Parent topic does not exist in the database");
        }

        /* Check that this is indeed a new endpoint and that its GUID is unique */
        for (auto endpoints_it: dds_endpoints<T>())
        {
            for (auto endpoint_it : endpoints_it.second)
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

        /* Add endpoint to participant' collection */
        endpoint->id = generate_entity_id();
        (*(endpoint->participant)).template ddsendpoints<T>()[endpoint->id] = endpoint;

        /* Add to x_by_y_ collections and to locators_ */
        for (auto locator_it : endpoint->locators)
        {
            // Add locator to locators_
            locators_[locator_it.first] = locator_it.second;
            // Add reader's locators to locators_by_participant_
            locators_by_participant_[endpoint->participant->id][locator_it.first] = locator_it.second;
            // Add reader's participant to participants_by_locator_
            participants_by_locator_[locator_it.first][endpoint->participant->id] = endpoint->participant;
        }

        /* Add endpoint to topics's collection */
        endpoint->data.clear();
        (*(endpoint->topic)).template ddsendpoints<T>()[endpoint->id] = endpoint;

        /* Insert endpoint in the database */
        dds_endpoints<T>()[endpoint->participant->domain->id][endpoint->id] = endpoint;
        return endpoint->id;
    }

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

    /* Collections with duplicated information used to speed up searches */
    //! Domains sorted by process EntityId
    std::map<EntityId, std::map<EntityId, std::shared_ptr<Domain>>> domains_by_process_;

    //! Processes sorted by domain EntityId
    std::map<EntityId, std::map<EntityId, std::shared_ptr<Process>>> processes_by_domain_;

    //! DomainParticipants sorted by locator EntityId
    std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>> participants_by_locator_;

    //! Locators sorted by participant EntityId
    std::map<EntityId, std::map<EntityId, std::shared_ptr<Locator>>> locators_by_participant_;

    /**
     * The ID that will be assigned to the next entity.
     * Used to guarantee a unique EntityId within the database instance
     */
    std::atomic<int64_t> next_id_{0};

    //! Read-write synchronization mutex
    mutable std::shared_timed_mutex mutex_;
};

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_DATABASE_HPP_
