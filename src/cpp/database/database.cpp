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

#include "database.hpp"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <mutex>  // For std::unique_lock
#include <shared_mutex>
#include <string>
#include <vector>

#include <fastdds/rtps/builtin/data/ParticipantBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/types/types.hpp>
#include <fastdds_statistics_backend/types/JSONTags.h>
#include <fastdds_statistics_backend/types/app_names.h>
#include <fastdds_statistics_backend/types/Alerts.hpp>
#include <StatisticsBackendData.hpp>

#include "database_queue.hpp"
#include "samples.hpp"

namespace eprosima {
namespace statistics_backend {
namespace database {

template<typename T>
std::string to_string(
        T data)
{
    std::stringstream ss;
    ss << data;
    return ss.str();
}

template<typename E>
void clear_inactive_entities_from_map_(
        std::map<EntityId, std::shared_ptr<E>>& map)
{
    static_assert(std::is_base_of<Entity, E>::value, "Class does not inherit from Entity.");

    // Iterate over whole loop and remove those entities that are not alive
    for (auto it = map.cbegin(); it != map.cend(); /* no increment */)
    {
        if (!it->second->active)
        {
            // Remove it and have reference to next element
            it = map.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

template<typename E>
void clear_inactive_entities_from_map_(
        std::map<EntityId, std::map<EntityId, std::shared_ptr<E>>>& map)
{
    // The higher map will not be removed because it holds the domain, so
    // we should just iterate over the internal map.
    for (auto& it : map)
    {
        clear_inactive_entities_from_map_(it.second);
    }
}

template<typename E>
void clear_inactive_entities_from_map_(
        std::map<EntityId, details::fragile_ptr<E>>& map)
{
    static_assert(std::is_base_of<Entity, E>::value, "Class does not inherit from Entity.");

    // Iterate over whole loop and remove those entities that are not alive
    for (auto it = map.cbegin(); it != map.cend(); /* no increment */)
    {
        if (it->second.expired())
        {
            // Remove it and have reference to next element
            it = map.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

EntityId Database::insert_new_participant(
        const std::string& name,
        const Qos& qos,
        const std::string& guid,
        const EntityId& domain_id,
        const StatusLevel& status,
        const AppId& app_id,
        const std::string& app_metadata,
        DiscoverySource discovery_source,
        DomainId original_domain /*= UNKNOWN_DOMAIN_ID*/)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);

    // Get the domain from the database
    // This may throw if the domain does not exist
    // The database MUST contain the domain, or something went wrong upstream
    std::shared_ptr<Domain> domain = std::const_pointer_cast<Domain>(
        std::static_pointer_cast<const Domain>(get_entity_nts(domain_id)));

    auto participant = std::make_shared<DomainParticipant>(
        name,
        qos,
        guid,
        std::shared_ptr<Process>(),
        domain,
        status,
        app_id,
        app_metadata,
        discovery_source,
        original_domain);

    EntityId entity_id;
    insert_nts(participant, entity_id);
    return entity_id;
}

void Database::process_physical_entities(
        const std::string& host_name,
        const std::string& user_name,
        const std::string& process_name,
        const std::string& process_pid,
        DiscoverySource discovery_source,
        bool& should_link_process_participant,
        const EntityId& participant_id,
        std::map<std::string, EntityId>& physical_entities_ids)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);
    process_physical_entities_nts(host_name, user_name, process_name, process_pid,
            discovery_source, should_link_process_participant, participant_id, physical_entities_ids);
}

void Database::process_physical_entities_nts(
        const std::string& host_name,
        const std::string& user_name,
        const std::string& process_name,
        const std::string& process_pid,
        DiscoverySource discovery_source,
        bool& should_link_process_participant,
        const EntityId& participant_id,
        std::map<std::string, EntityId>& physical_entities_ids)
{
    std::shared_ptr<Host> host;
    std::shared_ptr<User> user;
    std::shared_ptr<Process> process;


    // Get host entity
    auto hosts = get_entities_by_name_nts(EntityKind::HOST, host_name);
    if (hosts.empty())
    {
        host.reset(new Host(host_name));
        host->discovery_source = discovery_source;
        EntityId entity_id;
        insert_nts(host, entity_id);
        host->id = entity_id;
    }
    else
    {
        // Host name reported by Fast DDS are considered unique
        std::shared_ptr<const Host> const_host = std::dynamic_pointer_cast<const Host>(get_entity_nts(
                            hosts.front().second));
        host = std::const_pointer_cast<Host>(const_host);
        if (discovery_source == DiscoverySource::DISCOVERY && host->discovery_source != DiscoverySource::DISCOVERY)
        {
            // If the host already exists but it has now been discovered thorugh the standard procedure, its state
            // must be changed
            host->discovery_source = discovery_source;
        }
    }

    physical_entities_ids[HOST_ENTITY_TAG] = host->id;

    // Get user entity
    auto users = get_entities_by_name_nts(EntityKind::USER, user_name);
    for (const auto& it : users)
    {
        std::shared_ptr<const User> const_user =
                std::dynamic_pointer_cast<const User>(get_entity_nts(it.second));

        // The user name is unique within the host
        if (const_user->host == host)
        {
            user = std::const_pointer_cast<User>(const_user);
            if (discovery_source == DiscoverySource::DISCOVERY && user->discovery_source != DiscoverySource::DISCOVERY)
            {
                // If the user already exists but it has now been discovered thorugh the standard procedure, its state
                // must be changed
                user->discovery_source = discovery_source;
            }
            break;
        }
    }
    if (!user)
    {
        user.reset(new User(user_name, host));
        user->discovery_source = discovery_source;
        EntityId entity_id;
        insert_nts(user, entity_id);
        user->id = entity_id;
    }

    physical_entities_ids[USER_ENTITY_TAG] = user->id;

    auto processes = get_entities_by_name_nts(EntityKind::PROCESS, process_name);
    for (const auto& it : processes)
    {
        std::shared_ptr<const Process> const_process =
                std::dynamic_pointer_cast<const Process>(get_entity_nts(it.second));

        // There is only one process with the same name for a given user
        if (const_process->user == user)
        {
            process = std::const_pointer_cast<Process>(const_process);
            if (discovery_source == DiscoverySource::DISCOVERY &&
                    process->discovery_source != DiscoverySource::DISCOVERY)
            {
                // If the process already exists but it has now been discovered thorugh the standard procedure, its state
                // must be changed
                process->discovery_source = discovery_source;
            }
            break;
        }
    }
    if (!process)
    {
        process.reset(new Process(process_name, process_pid, user));
        process->discovery_source = discovery_source;
        EntityId entity_id;
        insert_nts(process, entity_id);
        process->id = entity_id;
        should_link_process_participant = true;
    }

    physical_entities_ids[PROCESS_ENTITY_TAG] = process->id;

    if (should_link_process_participant && participant_id != EntityId::invalid())
    {
        link_participant_with_process_nts(participant_id, process->id);
    }
}

bool Database::is_topic_in_database(
        const std::string& topic_type,
        const EntityId& topic_id)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);

    // Check whether the topic is already in the database
    std::shared_ptr<Topic> topic;

    try
    {
        topic = std::const_pointer_cast<Topic>(
            std::static_pointer_cast<const Topic>(get_entity_nts(topic_id)));

        if (topic->data_type == topic_type)
        {
            //Found the correct topic
            return true;
        }
        else
        {
            return false;
        }
    }
    catch (BadParameter&)
    {
        return false;
    }
}

EntityId Database::insert_new_topic(
        const std::string& name,
        const std::string& type_name,
        const std::string& alias,
        const EntityId& domain_id)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);

    // Get the domain from the database
    // This may throw if the domain does not exist
    // The database MUST contain the domain, or something went wrong upstream
    std::shared_ptr<Domain> domain = std::const_pointer_cast<Domain>(
        std::static_pointer_cast<const Domain>(get_entity_nts(domain_id)));

    // Create the topic to insert in the database
    auto topic = std::make_shared<Topic>(
        name,
        type_name,
        domain);

    if (!alias.empty())
    {
        topic->alias = alias;
    }

    EntityId entity_id;
    insert_nts(topic, entity_id);
    return entity_id;
}

bool Database::is_type_in_database(
        const std::string& type_name)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);
    return (type_idls_.find(type_name) != type_idls_.end());
}

void Database::insert_new_type_idl(
        const std::string& type_name,
        const std::string& type_idl)
{
    // Check that type name is not empty
    if (type_name.empty())
    {
        EPROSIMA_LOG_ERROR(BACKEND_DATABASE, "Type name cannot be empty");
        return;
    }

    // Check that type name is not already registered and we're trying to delete the type IDL
    if (is_type_in_database(type_name) && type_idl.empty())
    {
        return;
    }

    if (type_idl.find("module dds_\n") != std::string::npos
            || type_idl.find("::dds_::") != std::string::npos
            || type_name.find("dds_") != std::string::npos)
    {
        //Perform the demangling operations

        std::string type_name_demangled = type_name;
        std::string type_idl_demangled = type_idl;

        //Step 1: delete the module dds_

        while (type_idl_demangled.find("module dds_\n") != std::string::npos)
        {
            //First: delete the module dds_ identification, and the open brace
            size_t pos_start = type_idl_demangled.find("module dds_\n");
            size_t pos_open_brace = type_idl_demangled.find("{", pos_start);
            type_idl_demangled.erase(pos_start, pos_open_brace - pos_start + 2);

            //Second: find next line, and delete dangling whitespace
            size_t pos_line = type_idl_demangled.find_first_not_of(" ", pos_start);
            type_idl_demangled.erase(pos_start, pos_line - pos_start);

            //Third: unindent all the content
            pos_start = type_idl_demangled.find("   ", pos_start);
            while (type_idl_demangled[type_idl_demangled.find_first_not_of("   ", pos_start)] != '}')
            {
                type_idl_demangled.erase(pos_start, 4);
                size_t pos_new_line = type_idl_demangled.find_first_not_of(' ', pos_start);
                pos_start = type_idl_demangled.find("   ", pos_new_line);
            }

            //Fourth: delete the closing brace and whitespace
            size_t pos_end = type_idl_demangled.find("};", pos_start);
            type_idl_demangled.erase(pos_start - 1, pos_end - pos_start + 3);
        }

        //Step 2: delete the ::dds_:: namespace

        while (type_name_demangled.find("::dds_::") != std::string::npos)
        {
            size_t pos = type_name_demangled.find("::dds_::");
            type_name_demangled.erase(pos, 6);
        }

        while (type_idl_demangled.find("::dds_::") != std::string::npos)
        {
            size_t pos = type_idl_demangled.find("::dds_::");
            type_idl_demangled.erase(pos, 6);
        }

        //Step 3: delete the underscores

        while (type_name_demangled.back() == '_')
        {
            type_name_demangled.pop_back();
        }

        while (type_idl_demangled.find("__") != std::string::npos)
        {
            size_t pos = type_idl_demangled.find("__");
            type_idl_demangled.erase(pos, 2);
        }

        while (type_idl_demangled.find("_ ") != std::string::npos)
        {
            size_t pos = type_idl_demangled.find("_ ");
            type_idl_demangled.erase(pos, 1);
        }

        while (type_idl_demangled.find("_\n") != std::string::npos)
        {
            size_t pos = type_idl_demangled.find("_\n");
            type_idl_demangled.erase(pos, 1);
        }

        while (type_idl_demangled.find("_>") != std::string::npos)
        {
            size_t pos = type_idl_demangled.find("_>");
            type_idl_demangled.erase(pos, 1);
        }

        //Register the now demangled idl, the original as backup, and their relation
        std::lock_guard<std::shared_timed_mutex> guard(mutex_);
        type_idls_[type_name] = type_idl_demangled;
        type_ros2_unmodified_idl_[type_name] = type_idl;
        type_ros2_modified_name_[type_name] = type_name_demangled;
    }
    else
    {
        std::lock_guard<std::shared_timed_mutex> guard(mutex_);
        type_idls_[type_name] = type_idl;
    }
}

EntityId Database::insert_new_endpoint(
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
        DomainId original_domain = UNKNOWN_DOMAIN_ID)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);

    std::shared_ptr<DomainParticipant> participant =
            std::const_pointer_cast<DomainParticipant>(
        std::static_pointer_cast<const DomainParticipant>(get_entity_nts(
            participant_id)));

    std::shared_ptr<Topic> topic =
            std::const_pointer_cast<Topic>(
        std::static_pointer_cast<const Topic>(get_entity_nts(
            topic_id)));

    std::shared_ptr<DDSEndpoint> endpoint;

    if (kind == EntityKind::DATAREADER)
    {
        endpoint = create_endpoint_nts<DataReader>(
            endpoint_guid,
            name,
            qos,
            participant,
            topic,
            app_data.first,
            app_data.second,
            discovery_source,
            original_domain);
    }
    else
    {
        endpoint = create_endpoint_nts<DataWriter>(
            endpoint_guid,
            name,
            qos,
            participant,
            topic,
            app_data.first,
            app_data.second,
            discovery_source,
            original_domain);
    }


    // Mark it as the meta traffic one
    endpoint->is_virtual_metatraffic = is_virtual_metatraffic;
    if (!alias.empty())
    {
        endpoint->alias = alias;
    }

    /* Start processing the locator info */

    // Routine to process one locator from the locator list of the endpoint
    auto process_locators = [&](const eprosima::fastdds::rtps::Locator_t& dds_locator)
            {
                std::shared_ptr<Locator> locator;

                // Look for an existing locator
                // There can only be one
                auto locator_ids = get_entities_by_name_nts(EntityKind::LOCATOR, to_string(dds_locator));
                assert(locator_ids.empty() || locator_ids.size() == 1);

                if (!locator_ids.empty())
                {
                    // The locator exists. Add the existing one.
                    locator = std::const_pointer_cast<Locator>(
                        std::static_pointer_cast<const Locator>(get_entity_nts(locator_ids.front().
                                second)));
                }
                else
                {
                    // The locator is not in the database. Add the new one.
                    locator = std::make_shared<Locator>(to_string(dds_locator));
                    insert_nts(locator, locator->id);
                    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                        locator->id,
                        EntityKind::LOCATOR,
                        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
                }

                endpoint->locators[locator->id] = locator;
            };

    for (const auto& dds_locator : locators.unicast)
    {
        process_locators(dds_locator);
    }
    for (const auto& dds_locator : locators.multicast)
    {
        process_locators(dds_locator);
    }

    // insert the endpoint
    EntityId entity_id;
    insert_nts(endpoint, entity_id);
    return entity_id;
}

template <typename T>
std::shared_ptr<DDSEndpoint> Database::create_endpoint_nts(
        const std::string& endpoint_guid,
        const std::string& name,
        const Qos& qos,
        const std::shared_ptr<DomainParticipant>& participant,
        const std::shared_ptr<Topic>& topic,
        const AppId& app_id,
        const std::string& app_metadata,
        DiscoverySource discovery_source,
        DomainId original_domain)
{
    return std::make_shared<T>(
        name,
        qos,
        endpoint_guid,
        participant,
        topic,
        StatusLevel::OK_STATUS,
        app_id,
        app_metadata,
        discovery_source,
        original_domain);
}

EntityId Database::insert(
        const std::shared_ptr<Entity>& entity)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);

    // Insert in the database with a unique ID
    EntityId entity_id = EntityId::invalid();
    insert_nts(entity, entity_id);

    return entity_id;
}

void Database::insert_nts(
        const std::shared_ptr<Entity>& entity,
        EntityId& entity_id)
{
    // Clear the entity
    entity->clear();

    switch (entity->kind)
    {
        case EntityKind::HOST:
        {
            std::shared_ptr<Host> host = std::static_pointer_cast<Host>(entity);

            /* Check that host name is not empty */
            if (host->name.empty())
            {
                throw BadParameter("Host name cannot be empty");
            }

            /* Check that this is indeed a new host, and that its name is unique */
            for (const auto& host_it: hosts_)
            {
                if (host.get() == host_it.second.get())
                {
                    throw BadParameter("Host already exists in the database");
                }
                if (host->name == host_it.second->name)
                {
                    throw BadParameter("Host with name " + host->name + " already exists in the database");
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
            host->id = entity_id;

            /* Insert host in the database */
            hosts_[host->id] = host;
            break;
        }
        case EntityKind::USER:
        {
            std::shared_ptr<User> user = std::static_pointer_cast<User>(entity);

            /* Check that user name is not empty */
            if (user->name.empty())
            {
                throw BadParameter("User name cannot be empty");
            }

            /* Check that this is indeed a new user */
            for (const auto& user_it: users_)
            {
                if (user.get() == user_it.second.get())
                {
                    throw BadParameter("User already exists in the database");
                }
            }

            /* Check that host exits */
            bool host_exists = false;
            for (const auto& host_it : hosts_)
            {
                if (user->host.get() == host_it.second.get())
                {
                    host_exists = true;
                    break;
                }
            }

            if (!host_exists)
            {
                throw BadParameter("Parent host does not exist in the database");
            }

            /* Check that a user with the same name does not exist in the host collection */
            for (const auto& user_it : user->host->users)
            {
                if (user->name == user_it.second->name)
                {
                    throw BadParameter(
                              "Another User with name '" + user->name
                              + "' already exists in parent host collection");
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
            user->id = entity_id;

            /* Add user to users collection */
            users_[user->id] = user;

            /* Add user to host's users collection */
            user->host->users[user->id] = user;
            break;
        }
        case EntityKind::PROCESS:
        {
            std::shared_ptr<Process> process = std::static_pointer_cast<Process>(entity);

            /* Check that process name is not empty */
            if (process->name.empty())
            {
                throw BadParameter("Process name cannot be empty");
            }

            /* Check that PID is not empty */
            if (process->pid.empty())
            {
                throw BadParameter("Process PID cannot be empty");
            }

            /* Check that this is indeed a new process */
            for (const auto& process_it: processes_)
            {
                if (process.get() == process_it.second.get())
                {
                    throw BadParameter("Process already exists in the database");
                }
            }

            /* Check that user exits */
            bool user_exists = false;
            for (const auto& user_it : users_)
            {
                if (process->user.get() == user_it.second.get())
                {
                    user_exists = true;
                    break;
                }
            }

            if (!user_exists)
            {
                throw BadParameter("Parent user does not exist in the database");
            }

            /* Check that a process with the same pid does not exist in the same host */
            for (const auto& user_it : hosts_[process->user->host->id]->users)
            {
                for (const auto& process_it : user_it.second->processes)
                {
                    if (process->pid == process_it.second->pid)
                    {
                        throw BadParameter(
                                  "Another process with PID '" + process->pid
                                  + "' already exists in the same host");
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
            process->id = entity_id;

            /* Add process to processes collection */
            processes_[process->id] = process;

            /* Add process to user's processes collection */
            process->user->processes[process->id] = process;
            break;
        }
        case EntityKind::DOMAIN:
        {
            std::shared_ptr<Domain> domain = std::static_pointer_cast<Domain>(entity);

            /* Check that domain name is not empty */
            if (domain->name.empty())
            {
                throw BadParameter("Domain name cannot be empty");
            }

            /* Check that this is indeed a new domain and that its name is unique */
            for (const auto& domain_it: domains_)
            {
                if (domain.get() == domain_it.second.get())
                {
                    throw BadParameter("Domain already exists in the database");
                }
                if (domain->name == domain_it.second->name)
                {
                    throw BadParameter(
                              "A Domain with name '" + domain->name + "' already exists in the database");
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
            domain->id = entity_id;

            /* Insert domain in the database */
            domains_[domain->id] = domain;
            break;
        }
        case EntityKind::TOPIC:
        {
            std::shared_ptr<Topic> topic = std::static_pointer_cast<Topic>(entity);

            /* Check that topic name is not empty */
            if (topic->name.empty())
            {
                throw BadParameter("Topic name cannot be empty");
            }

            /* Check that topic data type is not empty */
            if (topic->data_type.empty())
            {
                throw BadParameter("Topic data type cannot be empty");
            }

            /* Check that domain exists */
            bool domain_exists = false;
            for (const auto& domain_it : domains_)
            {
                if (topic->domain.get() == domain_it.second.get())
                {
                    domain_exists = true;
                    break;
                }
            }

            if (!domain_exists)
            {
                throw BadParameter("Parent domain does not exist in the database");
            }

            /* Check that this is indeed a new topic and that its name and type combination is unique in the domain */
            auto domain_topics = topics_.find(topic->domain->id);
            if (domain_topics != topics_.end())
            {
                for (const auto& topic_it: domain_topics->second)
                {
                    if (topic.get() == topic_it.second.get())
                    {
                        throw BadParameter("Topic already exists in the database");
                    }
                    if (topic->name == topic_it.second->name &&
                            topic->data_type == topic_it.second->data_type)
                    {
                        throw BadParameter(
                                  "A topic with name '" + topic->name +
                                  "' and type '" + topic->data_type +
                                  "' already exists in the database for the same domain");
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
            topic->id = entity_id;

            /* Add topic to domain's collection */
            domains_[topic->domain->id]->topics[topic->id] = topic;

            /* Insert topic in the database */
            topics_[topic->domain->id][topic->id] = topic;
            break;
        }
        case EntityKind::PARTICIPANT:
        {
            std::shared_ptr<DomainParticipant> participant = std::static_pointer_cast<DomainParticipant>(entity);

            /* Check that name is not empty */
            if (participant->name.empty())
            {
                throw BadParameter("Participant name cannot be empty");
            }

            /* Check that qos is not empty */
            if (participant->qos.empty())
            {
                throw BadParameter("Participant QoS cannot be empty");
            }

            /* Check that GUID is not empty */
            if (participant->guid.empty())
            {
                throw BadParameter("Participant GUID cannot be empty");
            }

            /* Check that domain exits */
            bool domain_exists = false;
            for (const auto& domain_it : domains_)
            {
                if (participant->domain.get() == domain_it.second.get())
                {
                    domain_exists = true;
                    break;
                }
            }

            if (!domain_exists)
            {
                throw BadParameter("Parent domain does not exist in the database");
            }

            /* Check that this is indeed a new participant and that its GUID is unique */
            for (const auto& domain_it : participants_)
            {
                for (const auto& participant_it : domain_it.second)
                {
                    // Check that participant is new
                    if (participant.get() == participant_it.second.get())
                    {
                        throw BadParameter("Participant already exists in the database");
                    }
                    // Check GUID uniqueness
                    if (participant->guid == participant_it.second->guid)
                    {
                        throw BadParameter(
                                  "A participant with GUID '" + participant->guid +
                                  "' already exists in the database for the same domain");
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
            participant->id = entity_id;

            /* Add participant to domain's collection */
            participant->domain->participants[participant->id] = participant;

            /* Insert participant in the database */
            participants_[participant->domain->id][participant->id] = participant;
            break;
        }
        case EntityKind::DATAREADER:
        {
            auto data_reader = std::static_pointer_cast<DataReader>(entity);
            insert_ddsendpoint_nts<DataReader>(data_reader, entity_id);
            break;
        }
        case EntityKind::DATAWRITER:
        {
            auto data_writer = std::static_pointer_cast<DataWriter>(entity);
            insert_ddsendpoint_nts<DataWriter>(data_writer, entity_id);
            break;
        }
        case EntityKind::LOCATOR:
        {
            std::shared_ptr<Locator> locator = std::static_pointer_cast<Locator>(entity);

            /* Check that locator name is not empty */
            if (locator->name.empty())
            {
                throw BadParameter("Locator name cannot be empty");
            }

            /* Check that this is indeed a new locator, and that its name is unique */
            for (const auto& locator_it: locators_)
            {
                if (locator.get() == locator_it.second.get())
                {
                    throw BadParameter("Locator already exists in the database");
                }
                if (locator->name == locator_it.second->name)
                {
                    throw BadParameter("Locator with name " + locator->name + " already exists in the database");
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
            locator->id = entity_id;

            /* Insert locator in the database */
            locators_[locator->id] = locator;
            break;
        }
        default:
        {
            break;
        }
    }
}

void Database::insert(
        const EntityId& domain_id,
        const EntityId& entity_id,
        const StatisticsSample& sample)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);

    insert_nts(domain_id, entity_id, sample);
}

bool Database::insert(
        const EntityId& domain_id,
        const EntityId& entity_id,
        const MonitorServiceSample& sample)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);

    return insert_nts(domain_id, entity_id, sample);
}

std::shared_ptr<Locator> Database::get_locator_nts(
        EntityId const& entity_id)
{
    std::shared_ptr<Locator> locator;

    // Check if the entity is a known locator
    auto locator_it = locators_.find(entity_id);

    // If the locator has not been discovered, create it
    if (locator_it == locators_.end())
    {
        // Give him a unique name, including his id
        locator = std::make_shared<Locator>("locator_" + std::to_string(entity_id.value()));
        EntityId locator_id = entity_id;
        insert_nts(locator, locator_id);
        notify_locator_discovery(locator_id);
    }
    else
    {
        locator = locator_it->second;
    }
    return locator;
}

void Database::notify_locator_discovery (
        const EntityId& locator_id)
{
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        locator_id, EntityKind::LOCATOR,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
}

void Database::trigger_alerts_of_kind(
        const EntityId& domain_id,
        const EntityId& entity_id,
        const std::shared_ptr<DDSEndpoint>& endpoint,
        const AlertKind alert_kind,
        const double& data)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);

    // NOTE: At the moment, alerts are defined for all domains, so we ignore domain_id
    // There should be an option to get the active domains when setting the alert
    // and that is the value that should be used here. The static cast is just to avoid
    // the warning
    static_cast<void>(domain_id);

    trigger_alerts_of_kind_nts(
        domain_id,
        entity_id,
        endpoint,
        alert_kind,
        data);
}

void Database::trigger_alerts_of_kind_nts(
        const EntityId& domain_id,
        const EntityId& entity_id,
        const std::shared_ptr<DDSEndpoint>& endpoint,
        const AlertKind alert_kind,
        const double& data)
{
    // NOTE: At the moment, alerts are defined for all domains, so we ignore domain_id
    // There should be an option to get the active domains when setting the alert
    // and that is the value that should be used here. The static cast is just to avoid
    // the warning
    static_cast<void>(domain_id);

    for (auto& [alert_id, alert_info] : alerts_)
    {
        if (alert_info->get_alert_kind() == alert_kind)
        {
            // Get the metadata from the entity that sent the stats
            std::string topic_name = endpoint->topic->name;
            std::string user_name  = endpoint->participant->process->user->name;
            std::string host_name  = endpoint->participant->process->user->host->name;
            if (alert_info->check_trigger_conditions(host_name, user_name, topic_name, data))
            {
                // Update trigger info such as last trigger timestamp
                alert_info->trigger();
                // Notify the alert has been triggered
                details::StatisticsBackendData::get_instance()->on_alert_triggered(
                    domain_id,
                    entity_id,
                    *alert_info,
                    data);
            }
        }
    }
}

void Database::insert_nts(
        const EntityId& domain_id,
        const EntityId& entity_id,
        const StatisticsSample& sample,
        const bool loading,
        const bool last_reported)
{

    /* Check that domain_id refers to a known domain */
    if (sample.kind != DataKind::NETWORK_LATENCY)
    {
        if (domains_.find(domain_id) == domains_.end())
        {
            throw BadParameter(std::to_string(domain_id.value()) + " does not refer to a known domain");
        }
    }
    switch (sample.kind)
    {
        case DataKind::FASTDDS_LATENCY:
        {
            /* Check that the entity is a known writer */
            auto domain_writers = datawriters_.find(domain_id);
            if (domain_writers != datawriters_.end())
            {
                auto writer = domain_writers->second.find(entity_id);
                if (writer != domain_writers->second.end())
                {
                    const HistoryLatencySample& fastdds_latency = dynamic_cast<const HistoryLatencySample&>(sample);

                    // Reject samples with old timestamps
                    if (writer->second->data.history2history_latency.find(fastdds_latency.reader) !=
                            writer->second->data.history2history_latency.end() &&
                            fastdds_latency.src_ts <=
                            writer->second->data.history2history_latency[fastdds_latency.reader].back().
                                    src_ts)
                    {
                        break;
                    }

                    writer->second->data.history2history_latency[fastdds_latency.reader].push_back(fastdds_latency);
                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datawriter in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::NETWORK_LATENCY:
        {
            /* Check that the entity is a known participant */
            auto domain_participants = participants_.find(domain_id);
            if (domain_participants != participants_.end())
            {
                auto participant = domain_participants->second.find(entity_id);
                if (participant != domain_participants->second.end())
                {
                    const NetworkLatencySample& network_latency = dynamic_cast<const NetworkLatencySample&>(sample);

                    // Reject samples with old timestamps
                    if (participant->second->data.network_latency_per_locator.find(network_latency.remote_locator) !=
                            participant->second->data.network_latency_per_locator.end() &&
                            network_latency.src_ts <=
                            participant->second->data.network_latency_per_locator[network_latency.
                                    remote_locator].back().src_ts)
                    {
                        break;
                    }

                    participant->second->data.network_latency_per_locator[network_latency.remote_locator].push_back(
                        network_latency);
                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known participant in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::PUBLICATION_THROUGHPUT:
        {
            /* Check that the entity is a known writer */
            auto domain_writers = datawriters_.find(domain_id);
            if (domain_writers != datawriters_.end())
            {
                auto writer = domain_writers->second.find(entity_id);
                if (writer != domain_writers->second.end())
                {
                    const PublicationThroughputSample& publication_throughput =
                            dynamic_cast<const PublicationThroughputSample&>(sample);

                    // Reject samples with old timestamps
                    if (!writer->second->data.publication_throughput.empty() &&
                            publication_throughput.src_ts <= writer->second->data.publication_throughput.back().src_ts)
                    {
                        break;
                    }

                    writer->second->data.publication_throughput.push_back(publication_throughput);
                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datawriter in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::SUBSCRIPTION_THROUGHPUT:
        {
            /* Check that the entity is a known reader */
            auto domain_readers = datareaders_.find(domain_id);
            if (domain_readers != datareaders_.end())
            {
                auto reader = domain_readers->second.find(entity_id);
                if (reader != domain_readers->second.end())
                {
                    const SubscriptionThroughputSample& subscription_throughput =
                            dynamic_cast<const SubscriptionThroughputSample&>(sample);

                    // Reject samples with old timestamps
                    if (!reader->second->data.subscription_throughput.empty() &&
                            subscription_throughput.src_ts <=
                            reader->second->data.subscription_throughput.back().src_ts)
                    {
                        break;
                    }

                    reader->second->data.subscription_throughput.push_back(subscription_throughput);

                    // Trigger corresponding alerts
                    trigger_alerts_of_kind_nts(domain_id, entity_id, reader->second, AlertKind::NO_DATA,
                            subscription_throughput.data);
                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datareader in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::RTPS_PACKETS_SENT:
        {
            /* Check that the entity is a known participant */
            auto domain_participants = participants_.find(domain_id);
            if (domain_participants != participants_.end())
            {
                auto participant = domain_participants->second.find(entity_id);
                if (participant != domain_participants->second.end())
                {
                    const RtpsPacketsSentSample& rtps_packets_sent = dynamic_cast<const RtpsPacketsSentSample&>(sample);

                    // Reject samples with old timestamps (unless we are loading last reported)
                    auto it = participant->second->data.rtps_packets_sent.find(rtps_packets_sent.remote_locator);
                    if (it != participant->second->data.rtps_packets_sent.end() &&
                            rtps_packets_sent.src_ts <= it->second.back().src_ts &&
                            !(loading && last_reported && rtps_packets_sent.src_ts == it->second.back().src_ts))
                    {
                        break;
                    }

                    // Create remote_locator if it does not exist
                    get_locator_nts(rtps_packets_sent.remote_locator);

                    // Check if the insertion is from the load
                    if (loading)
                    {
                        if (last_reported)
                        {
                            // Store last reported
                            participant->second->data.last_reported_rtps_packets_sent_count[rtps_packets_sent.
                                            remote_locator] =
                                    rtps_packets_sent;
                        }
                        else
                        {
                            // Store data directly
                            participant->second->data.rtps_packets_sent[rtps_packets_sent.remote_locator].push_back(
                                rtps_packets_sent);
                        }
                    }
                    else
                    {
                        // Store the increment since the last report
                        participant->second->data.rtps_packets_sent[rtps_packets_sent.remote_locator].push_back(
                            rtps_packets_sent -
                            participant->second->data.last_reported_rtps_packets_sent_count[rtps_packets_sent.
                                    remote_locator]);
                        // Update last report
                        participant->second->data.last_reported_rtps_packets_sent_count[rtps_packets_sent.remote_locator
                        ] =
                                rtps_packets_sent;
                    }

                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known participant in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::RTPS_BYTES_SENT:
        {
            /* Check that the entity is a known participant */
            auto domain_participants = participants_.find(domain_id);
            if (domain_participants != participants_.end())
            {
                auto participant = domain_participants->second.find(entity_id);
                if (participant != domain_participants->second.end())
                {
                    const RtpsBytesSentSample& rtps_bytes_sent = dynamic_cast<const RtpsBytesSentSample&>(sample);

                    // Reject samples with old timestamps (unless we are loading last reported)
                    auto it = participant->second->data.rtps_bytes_sent.find(rtps_bytes_sent.remote_locator);
                    if (it != participant->second->data.rtps_bytes_sent.end() &&
                            rtps_bytes_sent.src_ts <= it->second.back().src_ts &&
                            !(loading && last_reported && rtps_bytes_sent.src_ts == it->second.back().src_ts))
                    {
                        break;
                    }

                    // Create remote_locator if it does not exist
                    get_locator_nts(rtps_bytes_sent.remote_locator);

                    // Check if the insertion is from the load
                    if (loading)
                    {
                        if (last_reported)
                        {
                            // Store last reported
                            participant->second->data.last_reported_rtps_bytes_sent_count[rtps_bytes_sent.remote_locator
                            ] =
                                    rtps_bytes_sent;
                        }
                        else
                        {
                            // Store data directly
                            participant->second->data.rtps_bytes_sent[rtps_bytes_sent.remote_locator].push_back(
                                rtps_bytes_sent);
                        }
                    }
                    else
                    {
                        // Store the increment since the last report
                        participant->second->data.rtps_bytes_sent[rtps_bytes_sent.remote_locator].push_back(
                            rtps_bytes_sent -
                            participant->second->data.last_reported_rtps_bytes_sent_count[rtps_bytes_sent.remote_locator]);
                        // Update last report
                        participant->second->data.last_reported_rtps_bytes_sent_count[rtps_bytes_sent.remote_locator] =
                                rtps_bytes_sent;
                    }

                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known participant in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::RTPS_PACKETS_LOST:
        {
            /* Check that the entity is a known participant */
            auto domain_participants = participants_.find(domain_id);
            if (domain_participants != participants_.end())
            {
                auto participant = domain_participants->second.find(entity_id);
                if (participant != domain_participants->second.end())
                {
                    const RtpsPacketsLostSample& rtps_packets_lost = dynamic_cast<const RtpsPacketsLostSample&>(sample);

                    // Reject samples with old timestamps (unless we are loading last reported)
                    auto it = participant->second->data.rtps_packets_lost.find(rtps_packets_lost.remote_locator);
                    if (it != participant->second->data.rtps_packets_lost.end() &&
                            rtps_packets_lost.src_ts <= it->second.back().src_ts &&
                            !(loading && last_reported && rtps_packets_lost.src_ts == it->second.back().src_ts))
                    {
                        break;
                    }

                    // Create remote_locator if it does not exist
                    get_locator_nts(rtps_packets_lost.remote_locator);

                    // Check if the insertion is from the load
                    if (loading)
                    {
                        if (last_reported)
                        {
                            // Store last reported
                            participant->second->data.last_reported_rtps_packets_lost_count[rtps_packets_lost.
                                            remote_locator] =
                                    rtps_packets_lost;
                        }
                        else
                        {
                            // Store data directly
                            participant->second->data.rtps_packets_lost[rtps_packets_lost.remote_locator].push_back(
                                rtps_packets_lost);
                        }
                    }
                    else
                    {
                        // Store the increment since the last report
                        participant->second->data.rtps_packets_lost[rtps_packets_lost.remote_locator].push_back(
                            rtps_packets_lost -
                            participant->second->data.last_reported_rtps_packets_lost_count[rtps_packets_lost.
                                    remote_locator]);
                        // Update last report
                        participant->second->data.last_reported_rtps_packets_lost_count[rtps_packets_lost.remote_locator
                        ] =
                                rtps_packets_lost;
                    }

                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known participant in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::RTPS_BYTES_LOST:
        {
            /* Check that the entity is a known participant */
            auto domain_participants = participants_.find(domain_id);
            if (domain_participants != participants_.end())
            {
                auto participant = domain_participants->second.find(entity_id);
                if (participant != domain_participants->second.end())
                {
                    const RtpsBytesLostSample& rtps_bytes_lost = dynamic_cast<const RtpsBytesLostSample&>(sample);

                    // Reject samples with old timestamps (unless we are loading last reported)
                    auto it = participant->second->data.rtps_bytes_lost.find(rtps_bytes_lost.remote_locator);
                    if (it != participant->second->data.rtps_bytes_lost.end() &&
                            rtps_bytes_lost.src_ts <= it->second.back().src_ts &&
                            !(loading && last_reported && rtps_bytes_lost.src_ts == it->second.back().src_ts))
                    {
                        break;
                    }

                    // Create remote_locator if it does not exist
                    get_locator_nts(rtps_bytes_lost.remote_locator);

                    // Check if the insertion is from the load
                    if (loading)
                    {
                        if (last_reported)
                        {
                            // Store last reported
                            participant->second->data.last_reported_rtps_bytes_lost_count[rtps_bytes_lost.remote_locator
                            ] =
                                    rtps_bytes_lost;
                        }
                        else
                        {
                            // Store data directly
                            participant->second->data.rtps_bytes_lost[rtps_bytes_lost.remote_locator].push_back(
                                rtps_bytes_lost);
                        }
                    }
                    else
                    {
                        // Store the increment since the last report
                        participant->second->data.rtps_bytes_lost[rtps_bytes_lost.remote_locator].push_back(
                            rtps_bytes_lost -
                            participant->second->data.last_reported_rtps_bytes_lost_count[rtps_bytes_lost.remote_locator]);
                        // Update last report
                        participant->second->data.last_reported_rtps_bytes_lost_count[rtps_bytes_lost.remote_locator] =
                                rtps_bytes_lost;
                    }

                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known participant in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::RESENT_DATA:
        {
            /* Check that the entity is a known writer */
            auto domain_writers = datawriters_.find(domain_id);
            if (domain_writers != datawriters_.end())
            {
                auto writer = domain_writers->second.find(entity_id);
                if (writer != domain_writers->second.end())
                {
                    const ResentDataSample& resent_datas = dynamic_cast<const ResentDataSample&>(sample);

                    // Reject samples with old timestamps (unless we are loading last reported)
                    if (!writer->second->data.resent_datas.empty() &&
                            resent_datas.src_ts <= writer->second->data.resent_datas.back().src_ts &&
                            !(loading && last_reported &&
                            resent_datas.src_ts == writer->second->data.resent_datas.back().src_ts))
                    {
                        break;
                    }

                    // Check if the insertion is from the load
                    if (loading)
                    {
                        if (last_reported)
                        {
                            // Store last reported
                            writer->second->data.last_reported_resent_datas = resent_datas;
                        }
                        else
                        {
                            // Store data directly
                            writer->second->data.resent_datas.push_back(resent_datas);
                        }
                    }
                    else
                    {
                        // Store the increment since the last report
                        writer->second->data.resent_datas.push_back(
                            resent_datas - writer->second->data.last_reported_resent_datas);
                        // Update last report
                        writer->second->data.last_reported_resent_datas = resent_datas;
                    }

                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datawriter in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::HEARTBEAT_COUNT:
        {
            /* Check that the entity is a known writer */
            auto domain_writers = datawriters_.find(domain_id);
            if (domain_writers != datawriters_.end())
            {
                auto writer = domain_writers->second.find(entity_id);
                if (writer != domain_writers->second.end())
                {
                    const HeartbeatCountSample& heartbeat_count = dynamic_cast<const HeartbeatCountSample&>(sample);

                    // Reject samples with old timestamps (unless we are loading last reported)
                    if (!writer->second->data.heartbeat_count.empty() &&
                            heartbeat_count.src_ts <= writer->second->data.heartbeat_count.back().src_ts &&
                            !(loading && last_reported &&
                            heartbeat_count.src_ts == writer->second->data.heartbeat_count.back().src_ts))
                    {
                        break;
                    }

                    // Check if the insertion is from the load
                    if (loading)
                    {
                        if (last_reported)
                        {
                            // Store last reported
                            writer->second->data.last_reported_heartbeat_count = heartbeat_count;
                        }
                        else
                        {
                            // Store data directly
                            writer->second->data.heartbeat_count.push_back(heartbeat_count);
                        }
                    }
                    else
                    {
                        // Store the increment since the last report
                        writer->second->data.heartbeat_count.push_back(heartbeat_count -
                                writer->second->data.last_reported_heartbeat_count);
                        // Update last report
                        writer->second->data.last_reported_heartbeat_count = heartbeat_count;
                    }

                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datawriter in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::ACKNACK_COUNT:
        {
            /* Check that the entity is a known reader */
            auto domain_readers = datareaders_.find(domain_id);
            if (domain_readers != datareaders_.end())
            {
                auto reader = domain_readers->second.find(entity_id);
                if (reader != domain_readers->second.end())
                {
                    const AcknackCountSample& acknack_count = dynamic_cast<const AcknackCountSample&>(sample);

                    // Reject samples with old timestamps (unless we are loading last reported)
                    if (!reader->second->data.acknack_count.empty() &&
                            acknack_count.src_ts <= reader->second->data.acknack_count.back().src_ts &&
                            !(loading && last_reported &&
                            acknack_count.src_ts == reader->second->data.acknack_count.back().src_ts))
                    {
                        break;
                    }

                    // Check if the insertion is from the load
                    if (loading)
                    {
                        if (last_reported)
                        {
                            // Store last reported
                            reader->second->data.last_reported_acknack_count = acknack_count;
                        }
                        else
                        {
                            // Store data directly
                            reader->second->data.acknack_count.push_back(acknack_count);
                        }
                    }
                    else
                    {
                        // Store the increment since the last report
                        reader->second->data.acknack_count.push_back(
                            acknack_count - reader->second->data.last_reported_acknack_count);
                        // Update last report
                        reader->second->data.last_reported_acknack_count = acknack_count;
                    }

                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datareader in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::NACKFRAG_COUNT:
        {
            /* Check that the entity is a known reader */
            auto domain_readers = datareaders_.find(domain_id);
            if (domain_readers != datareaders_.end())
            {
                auto reader = domain_readers->second.find(entity_id);
                if (reader != domain_readers->second.end())
                {
                    const NackfragCountSample& nackfrag_count = dynamic_cast<const NackfragCountSample&>(sample);

                    // Reject samples with old timestamps (unless we are loading last reported)
                    if (!reader->second->data.nackfrag_count.empty() &&
                            nackfrag_count.src_ts <= reader->second->data.nackfrag_count.back().src_ts &&
                            !(loading && last_reported &&
                            nackfrag_count.src_ts == reader->second->data.nackfrag_count.back().src_ts))
                    {
                        break;
                    }

                    // Check if the insertion is from the load
                    if (loading)
                    {
                        if (last_reported)
                        {
                            // Store last reported
                            reader->second->data.last_reported_nackfrag_count = nackfrag_count;
                        }
                        else
                        {
                            // Store data directly
                            reader->second->data.nackfrag_count.push_back(nackfrag_count);
                        }
                    }
                    else
                    {
                        // Store the increment since the last report
                        reader->second->data.nackfrag_count.push_back(
                            nackfrag_count - reader->second->data.last_reported_nackfrag_count);
                        // Update last report
                        reader->second->data.last_reported_nackfrag_count = nackfrag_count;
                    }

                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datareader in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::GAP_COUNT:
        {
            /* Check that the entity is a known writer */
            auto domain_writers = datawriters_.find(domain_id);
            if (domain_writers != datawriters_.end())
            {
                auto writer = domain_writers->second.find(entity_id);
                if (writer != domain_writers->second.end())
                {
                    const GapCountSample& gap_count = dynamic_cast<const GapCountSample&>(sample);

                    // Reject samples with old timestamps (unless we are loading last reported)
                    if (!writer->second->data.gap_count.empty() &&
                            gap_count.src_ts <= writer->second->data.gap_count.back().src_ts &&
                            !(loading && last_reported &&
                            gap_count.src_ts == writer->second->data.gap_count.back().src_ts))
                    {
                        break;
                    }

                    // Check if the insertion is from the load
                    if (loading)
                    {
                        if (last_reported)
                        {
                            // Store last reported
                            writer->second->data.last_reported_gap_count = gap_count;
                        }
                        else
                        {
                            // Store data directly
                            writer->second->data.gap_count.push_back(gap_count);
                        }
                    }
                    else
                    {
                        // Store the increment since the last report
                        writer->second->data.gap_count.push_back(
                            gap_count - writer->second->data.last_reported_gap_count);
                        // Update last report
                        writer->second->data.last_reported_gap_count = gap_count;
                    }

                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datawriter in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::DATA_COUNT:
        {
            /* Check that the entity is a known writer */
            auto domain_writers = datawriters_.find(domain_id);
            if (domain_writers != datawriters_.end())
            {
                auto writer = domain_writers->second.find(entity_id);
                if (writer != domain_writers->second.end())
                {
                    const DataCountSample& data_count = dynamic_cast<const DataCountSample&>(sample);

                    // Reject samples with old timestamps (unless we are loading last reported)
                    if (!writer->second->data.data_count.empty() &&
                            data_count.src_ts <= writer->second->data.data_count.back().src_ts &&
                            !(loading && last_reported &&
                            data_count.src_ts == writer->second->data.data_count.back().src_ts))
                    {
                        break;
                    }

                    // Check if the insertion is from the load
                    if (loading)
                    {
                        if (last_reported)
                        {
                            // Store last reported
                            writer->second->data.last_reported_data_count = data_count;
                        }
                        else
                        {
                            // Store data directly
                            writer->second->data.data_count.push_back(data_count);
                        }
                    }
                    else
                    {
                        // Store the increment since the last report
                        writer->second->data.data_count.push_back(
                            data_count - writer->second->data.last_reported_data_count);
                        // Update last report
                        writer->second->data.last_reported_data_count = data_count;
                    }

                    // Trigger corresponding alerts
                    trigger_alerts_of_kind_nts(domain_id, entity_id, writer->second, AlertKind::NEW_DATA,
                            writer->second->data.data_count.back().count);
                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datawriter in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::PDP_PACKETS:
        {
            /* Check that the entity is a known participant */
            auto domain_participants = participants_.find(domain_id);
            if (domain_participants != participants_.end())
            {
                auto participant = domain_participants->second.find(entity_id);
                if (participant != domain_participants->second.end())
                {
                    const PdpCountSample& pdp_packets = dynamic_cast<const PdpCountSample&>(sample);

                    // Reject samples with old timestamps (unless we are loading last reported)
                    if (!participant->second->data.pdp_packets.empty() &&
                            pdp_packets.src_ts <= participant->second->data.pdp_packets.back().src_ts &&
                            !(loading && last_reported &&
                            pdp_packets.src_ts == participant->second->data.pdp_packets.back().src_ts))
                    {
                        break;
                    }

                    // Check if the insertion is from the load
                    if (loading)
                    {
                        if (last_reported)
                        {
                            // Store last reported
                            participant->second->data.last_reported_pdp_packets = pdp_packets;
                        }
                        else
                        {
                            // Store data directly
                            participant->second->data.pdp_packets.push_back(pdp_packets);
                        }

                    }
                    else
                    {
                        // Store the increment since the last report
                        participant->second->data.pdp_packets.push_back(
                            pdp_packets - participant->second->data.last_reported_pdp_packets);
                        // Update last report
                        participant->second->data.last_reported_pdp_packets = pdp_packets;
                    }

                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known participant in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::EDP_PACKETS:
        {
            /* Check that the entity is a known participant */
            auto domain_participants = participants_.find(domain_id);
            if (domain_participants != participants_.end())
            {
                auto participant = domain_participants->second.find(entity_id);
                if (participant != domain_participants->second.end())
                {
                    const EdpCountSample& edp_packets = dynamic_cast<const EdpCountSample&>(sample);

                    // Reject samples with old timestamps
                    if (!participant->second->data.edp_packets.empty() &&
                            edp_packets.src_ts <= participant->second->data.edp_packets.back().src_ts &&
                            !(loading && last_reported &&
                            edp_packets.src_ts == participant->second->data.edp_packets.back().src_ts))
                    {
                        break;
                    }

                    // Check if the insertion is from the load
                    if (loading)
                    {
                        if (last_reported)
                        {
                            // Store last reported
                            participant->second->data.last_reported_edp_packets = edp_packets;
                        }
                        else
                        {
                            // Store data directly
                            participant->second->data.edp_packets.push_back(edp_packets);
                        }
                    }
                    else
                    {
                        // Store the increment since the last report
                        participant->second->data.edp_packets.push_back(
                            edp_packets - participant->second->data.last_reported_edp_packets);
                        // Update last report
                        participant->second->data.last_reported_edp_packets = edp_packets;
                    }

                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known participant in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::DISCOVERY_TIME:
        {
            /* Check that the entity is a known participant */
            auto domain_participants = participants_.find(domain_id);
            if (domain_participants != participants_.end())
            {
                auto participant = domain_participants->second.find(entity_id);
                if (participant != domain_participants->second.end())
                {
                    const DiscoveryTimeSample& discovery_time = dynamic_cast<const DiscoveryTimeSample&>(sample);

                    // Reject samples with old timestamps
                    if (participant->second->data.discovered_entity.find(discovery_time.remote_entity) !=
                            participant->second->data.discovered_entity.end() &&
                            discovery_time.src_ts <=
                            participant->second->data.discovered_entity[discovery_time.remote_entity].back()
                                    .src_ts)
                    {
                        break;
                    }

                    participant->second->data.discovered_entity[discovery_time.remote_entity].push_back(discovery_time);
                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known participant in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::SAMPLE_DATAS:
        {
            /* Check that the entity is a known writer */
            auto domain_writers = datawriters_.find(domain_id);
            if (domain_writers != datawriters_.end())
            {
                auto writer = domain_writers->second.find(entity_id);
                if (writer != domain_writers->second.end())
                {
                    const SampleDatasCountSample& sample_datas = dynamic_cast<const SampleDatasCountSample&>(sample);

                    // Reject samples with old timestamps
                    if (writer->second->data.sample_datas.find(sample_datas.sequence_number) !=
                            writer->second->data.sample_datas.end() &&
                            sample_datas.src_ts <=
                            writer->second->data.sample_datas[sample_datas.sequence_number].back().src_ts)
                    {
                        break;
                    }

                    // Only save the last received sample for each sequence number
                    writer->second->data.sample_datas[sample_datas.sequence_number].clear();
                    writer->second->data.sample_datas[sample_datas.sequence_number].push_back(sample_datas);
                    break;
                }
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datawriter in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::INVALID:
        {
            throw BadParameter("Invalid DataKind");
        }
    }
    static_cast<void>(entity_id);
    static_cast<void>(sample);
}

bool Database::insert_nts(
        const EntityId& domain_id,
        const EntityId& entity_id,
        const MonitorServiceSample& sample)
{
    bool entity_updated = false;
    /* Check that domain_id refers to a known domain */
    if (domains_.find(domain_id) == domains_.end())
    {
        throw BadParameter(std::to_string(domain_id.value()) + " does not refer to a known domain");
    }

    switch (sample.kind)
    {
        case StatusKind::PROXY:
        {
            std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);
            const ProxySample& proxy = dynamic_cast<const ProxySample&>(sample);
            switch (entity->kind)
            {
                case (EntityKind::PARTICIPANT):
                {
                    std::shared_ptr<const DomainParticipant> const_participant =
                            std::dynamic_pointer_cast<const DomainParticipant>(entity);
                    std::shared_ptr<DomainParticipant> participant = std::const_pointer_cast<DomainParticipant>(
                        const_participant);

                    // Reject samples with old timestamps
                    if (!participant->monitor_service_data.proxy.empty() &&
                            proxy.src_ts <= participant->monitor_service_data.proxy.back().src_ts)
                    {
                        break;
                    }

                    participant->monitor_service_data.proxy.push_back(proxy);
                    break;
                }
                case (EntityKind::DATAREADER):
                {
                    std::shared_ptr<const DataReader> const_datareader = std::dynamic_pointer_cast<const DataReader>(
                        entity);
                    std::shared_ptr<DataReader> datareader = std::const_pointer_cast<DataReader>(const_datareader);

                    // Reject samples with old timestamps
                    if (!datareader->monitor_service_data.proxy.empty() &&
                            proxy.src_ts <= datareader->monitor_service_data.proxy.back().src_ts)
                    {
                        break;
                    }

                    datareader->monitor_service_data.proxy.push_back(proxy);
                    break;
                }
                case (EntityKind::DATAWRITER):
                {
                    std::shared_ptr<const DataWriter> const_datawriter = std::dynamic_pointer_cast<const DataWriter>(
                        entity);
                    std::shared_ptr<DataWriter> datawriter = std::const_pointer_cast<DataWriter>(const_datawriter);

                    // Reject samples with old timestamps
                    if (!datawriter->monitor_service_data.proxy.empty() &&
                            proxy.src_ts <= datawriter->monitor_service_data.proxy.back().src_ts)
                    {
                        break;
                    }

                    datawriter->monitor_service_data.proxy.push_back(proxy);
                    break;
                }
                default:
                {
                    throw BadParameter("Unsupported PROXY Status type and EntityKind combination");
                }
            }
            break;
        }
        case StatusKind::CONNECTION_LIST:
        {
            std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);
            const ConnectionListSample& connection_list = dynamic_cast<const ConnectionListSample&>(sample);
            switch (entity->kind)
            {
                case (EntityKind::PARTICIPANT):
                {
                    std::shared_ptr<const DomainParticipant> const_participant =
                            std::dynamic_pointer_cast<const DomainParticipant>(entity);
                    std::shared_ptr<DomainParticipant> participant = std::const_pointer_cast<DomainParticipant>(
                        const_participant);

                    // Reject samples with old timestamps
                    if (!participant->monitor_service_data.connection_list.empty() &&
                            connection_list.src_ts <= participant->monitor_service_data.connection_list.back().src_ts)
                    {
                        break;
                    }

                    participant->monitor_service_data.connection_list.push_back(connection_list);
                    break;
                }
                case (EntityKind::DATAREADER):
                {
                    std::shared_ptr<const DataReader> const_datareader = std::dynamic_pointer_cast<const DataReader>(
                        entity);
                    std::shared_ptr<DataReader> datareader = std::const_pointer_cast<DataReader>(const_datareader);

                    // Reject samples with old timestamps
                    if (!datareader->monitor_service_data.connection_list.empty() &&
                            connection_list.src_ts <= datareader->monitor_service_data.connection_list.back().src_ts)
                    {
                        break;
                    }

                    datareader->monitor_service_data.connection_list.push_back(connection_list);
                    break;
                }
                case (EntityKind::DATAWRITER):
                {
                    std::shared_ptr<const DataWriter> const_datawriter = std::dynamic_pointer_cast<const DataWriter>(
                        entity);
                    std::shared_ptr<DataWriter> datawriter = std::const_pointer_cast<DataWriter>(const_datawriter);

                    // Reject samples with old timestamps
                    if (!datawriter->monitor_service_data.connection_list.empty() &&
                            connection_list.src_ts <= datawriter->monitor_service_data.connection_list.back().src_ts)
                    {
                        break;
                    }

                    datawriter->monitor_service_data.connection_list.push_back(connection_list);
                    break;
                }
                default:
                {
                    throw BadParameter("Unsupported CONNECTION_LIST Status type and EntityKind combination");
                }
            }
            break;
        }
        case StatusKind::INCOMPATIBLE_QOS:
        {
            std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);
            const IncompatibleQosSample& incompatible_qos = dynamic_cast<const IncompatibleQosSample&>(sample);
            switch (entity->kind)
            {
                case (EntityKind::DATAREADER):
                {
                    std::shared_ptr<const DataReader> const_datareader = std::dynamic_pointer_cast<const DataReader>(
                        entity);
                    std::shared_ptr<DataReader> datareader = std::const_pointer_cast<DataReader>(const_datareader);

                    // Reject samples with old timestamps
                    if (!datareader->monitor_service_data.incompatible_qos.empty() &&
                            incompatible_qos.src_ts <= datareader->monitor_service_data.incompatible_qos.back().src_ts)
                    {
                        break;
                    }

                    datareader->monitor_service_data.incompatible_qos.push_back(incompatible_qos);
                    entity_updated = update_entity_status_nts<DataReader>(datareader);
                    break;
                }
                case (EntityKind::DATAWRITER):
                {
                    std::shared_ptr<const DataWriter> const_datawriter = std::dynamic_pointer_cast<const DataWriter>(
                        entity);
                    std::shared_ptr<DataWriter> datawriter = std::const_pointer_cast<DataWriter>(const_datawriter);

                    // Reject samples with old timestamps
                    if (!datawriter->monitor_service_data.incompatible_qos.empty() &&
                            incompatible_qos.src_ts <= datawriter->monitor_service_data.incompatible_qos.back().src_ts)
                    {
                        break;
                    }

                    datawriter->monitor_service_data.incompatible_qos.push_back(incompatible_qos);
                    entity_updated = update_entity_status_nts<DataWriter>(datawriter);
                    break;
                }
                default:
                {
                    throw BadParameter("Unsupported INCOMPATIBLE_QOS Status type and EntityKind combination");
                }
            }
            break;
        }
        case StatusKind::INCONSISTENT_TOPIC:
        {
            std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);
            const InconsistentTopicSample& inconsistent_topic = dynamic_cast<const InconsistentTopicSample&>(sample);
            switch (entity->kind)
            {
                case (EntityKind::DATAREADER):
                {
                    std::shared_ptr<const DataReader> const_datareader = std::dynamic_pointer_cast<const DataReader>(
                        entity);
                    std::shared_ptr<DataReader> datareader = std::const_pointer_cast<DataReader>(const_datareader);

                    // Reject samples with old timestamps
                    if (!datareader->monitor_service_data.inconsistent_topic.empty() &&
                            inconsistent_topic.src_ts <=
                            datareader->monitor_service_data.inconsistent_topic.back().src_ts)
                    {
                        break;
                    }

                    datareader->monitor_service_data.inconsistent_topic.push_back(inconsistent_topic);
                    entity_updated = update_entity_status_nts<DataReader>(datareader);
                    break;
                }
                case (EntityKind::DATAWRITER):
                {
                    std::shared_ptr<const DataWriter> const_datawriter = std::dynamic_pointer_cast<const DataWriter>(
                        entity);
                    std::shared_ptr<DataWriter> datawriter = std::const_pointer_cast<DataWriter>(const_datawriter);

                    // Reject samples with old timestamps
                    if (!datawriter->monitor_service_data.inconsistent_topic.empty() &&
                            inconsistent_topic.src_ts <=
                            datawriter->monitor_service_data.inconsistent_topic.back().src_ts)
                    {
                        break;
                    }

                    datawriter->monitor_service_data.inconsistent_topic.push_back(inconsistent_topic);

                    entity_updated = update_entity_status_nts<DataWriter>(datawriter);
                    break;
                }
                default:
                {
                    throw BadParameter("Unsupported INCONSISTENT_TOPIC Status type and EntityKind combination");
                }
            }
            break;
        }
        case StatusKind::LIVELINESS_LOST:
        {
            std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);
            const LivelinessLostSample& liveliness_lost = dynamic_cast<const LivelinessLostSample&>(sample);
            if (entity->kind == EntityKind::DATAWRITER)
            {
                std::shared_ptr<const DataWriter> const_datawriter =
                        std::dynamic_pointer_cast<const DataWriter>(entity);
                std::shared_ptr<DataWriter> datawriter = std::const_pointer_cast<DataWriter>(const_datawriter);

                // Reject samples with old timestamps
                if (!datawriter->monitor_service_data.liveliness_lost.empty() &&
                        liveliness_lost.src_ts <= datawriter->monitor_service_data.liveliness_lost.back().src_ts)
                {
                    break;
                }

                datawriter->monitor_service_data.liveliness_lost.push_back(liveliness_lost);
                entity_updated = update_entity_status_nts<DataWriter>(datawriter);
                break;
            }
            else
            {
                throw BadParameter("Unsupported LIVELINESS_LOST Status type and EntityKind combination");
            }
            break;
        }
        case StatusKind::LIVELINESS_CHANGED:
        {
            std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);
            const LivelinessChangedSample& liveliness_changed = dynamic_cast<const LivelinessChangedSample&>(sample);
            if (entity->kind == EntityKind::DATAREADER)
            {
                std::shared_ptr<const DataReader> const_datareader =
                        std::dynamic_pointer_cast<const DataReader>(entity);
                std::shared_ptr<DataReader> datareader = std::const_pointer_cast<DataReader>(const_datareader);

                // Reject samples with old timestamps
                if (!datareader->monitor_service_data.liveliness_changed.empty() &&
                        liveliness_changed.src_ts <= datareader->monitor_service_data.liveliness_changed.back().src_ts)
                {
                    break;
                }

                datareader->monitor_service_data.liveliness_changed.push_back(liveliness_changed);
                break;
            }
            else
            {
                throw BadParameter("Unsupported LIVELINESS_CHANGED Status type and EntityKind combination");
            }
            break;
        }
        case StatusKind::DEADLINE_MISSED:
        {
            std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);
            const DeadlineMissedSample& deadline_missed = dynamic_cast<const DeadlineMissedSample&>(sample);
            switch (entity->kind)
            {
                case (EntityKind::DATAREADER):
                {
                    std::shared_ptr<const DataReader> const_datareader = std::dynamic_pointer_cast<const DataReader>(
                        entity);
                    std::shared_ptr<DataReader> datareader = std::const_pointer_cast<DataReader>(const_datareader);

                    // Reject samples with old timestamps
                    if (!datareader->monitor_service_data.deadline_missed.empty() &&
                            deadline_missed.src_ts <= datareader->monitor_service_data.deadline_missed.back().src_ts)
                    {
                        break;
                    }

                    datareader->monitor_service_data.deadline_missed.push_back(deadline_missed);
                    entity_updated = update_entity_status_nts<DataReader>(datareader);
                    break;
                }
                case (EntityKind::DATAWRITER):
                {
                    std::shared_ptr<const DataWriter> const_datawriter = std::dynamic_pointer_cast<const DataWriter>(
                        entity);
                    std::shared_ptr<DataWriter> datawriter = std::const_pointer_cast<DataWriter>(const_datawriter);

                    // Reject samples with old timestamps
                    if (!datawriter->monitor_service_data.deadline_missed.empty() &&
                            deadline_missed.src_ts <= datawriter->monitor_service_data.deadline_missed.back().src_ts)
                    {
                        break;
                    }

                    datawriter->monitor_service_data.deadline_missed.push_back(deadline_missed);
                    entity_updated = update_entity_status_nts<DataWriter>(datawriter);
                    break;
                }
                default:
                {
                    throw BadParameter("Unsupported DEADLINE_MISSED Status type and EntityKind combination");
                }
            }
            break;
        }
        case StatusKind::SAMPLE_LOST:
        {
            std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);
            const SampleLostSample& sample_lost = dynamic_cast<const SampleLostSample&>(sample);
            if (entity->kind == EntityKind::DATAREADER)
            {
                std::shared_ptr<const DataReader> const_datareader =
                        std::dynamic_pointer_cast<const DataReader>(entity);
                std::shared_ptr<DataReader> datareader = std::const_pointer_cast<DataReader>(const_datareader);

                // Reject samples with old timestamps
                if (!datareader->monitor_service_data.sample_lost.empty() &&
                        sample_lost.src_ts <= datareader->monitor_service_data.sample_lost.back().src_ts)
                {
                    break;
                }

                datareader->monitor_service_data.sample_lost.push_back(sample_lost);
                entity_updated = update_entity_status_nts<DataReader>(datareader);
                break;
            }
            else
            {
                throw BadParameter("Unsupported SAMPLE_LOST Status type and EntityKind combination");
            }
            break;
        }
        case StatusKind::EXTENDED_INCOMPATIBLE_QOS:
        {
            std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);
            const ExtendedIncompatibleQosSample& extended_incompatible_qos =
                    dynamic_cast<const ExtendedIncompatibleQosSample&>(sample);
            switch (entity->kind)
            {
                case (EntityKind::DATAREADER):
                {
                    std::shared_ptr<const DataReader> const_datareader = std::dynamic_pointer_cast<const DataReader>(
                        entity);
                    std::shared_ptr<DataReader> datareader = std::const_pointer_cast<DataReader>(const_datareader);

                    // Reject samples with old timestamps
                    if (!datareader->monitor_service_data.extended_incompatible_qos.empty() &&
                            extended_incompatible_qos.src_ts <=
                            datareader->monitor_service_data.extended_incompatible_qos.back().src_ts)
                    {
                        break;
                    }

                    datareader->monitor_service_data.extended_incompatible_qos.push_back(extended_incompatible_qos);
                    entity_updated = update_entity_status_nts<DataReader>(datareader);
                    break;
                }
                case (EntityKind::DATAWRITER):
                {
                    std::shared_ptr<const DataWriter> const_datawriter = std::dynamic_pointer_cast<const DataWriter>(
                        entity);
                    std::shared_ptr<DataWriter> datawriter = std::const_pointer_cast<DataWriter>(const_datawriter);

                    // Reject samples with old timestamps
                    if (!datawriter->monitor_service_data.extended_incompatible_qos.empty() &&
                            extended_incompatible_qos.src_ts <=
                            datawriter->monitor_service_data.extended_incompatible_qos.back().src_ts)
                    {
                        break;
                    }

                    datawriter->monitor_service_data.extended_incompatible_qos.push_back(extended_incompatible_qos);
                    entity_updated = update_entity_status_nts<DataWriter>(datawriter);
                    break;
                }
                default:
                {
                    throw BadParameter("Unsupported EXTENDED_INCOMPATIBLE_QOS Status type and EntityKind combination");
                }
            }
            break;
        }
        default:
        {
            throw BadParameter("Unsupported StatusKind");
        }
    }
    return entity_updated;
}

void Database::link_participant_with_process(
        const EntityId& participant_id,
        const EntityId& process_id)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);

    link_participant_with_process_nts(participant_id, process_id);
}

void Database::link_participant_with_process_nts(
        const EntityId& participant_id,
        const EntityId& process_id)
{
    /* Get the participant and the domain */
    EntityId domain_id;
    std::map<EntityId, std::shared_ptr<DomainParticipant>>::iterator participant_it;
    bool participant_exists = false;
    for (auto& domain_it : participants_)
    {
        participant_it = domain_it.second.find(participant_id);
        if (participant_it != domain_it.second.end())
        {
            participant_exists = true;
            domain_id = domain_it.first;
            break;
        }
    }

    /* Check that the participant exists */
    if (!participant_exists)
    {
        throw BadParameter("EntityId " + std::to_string(
                          participant_id.value()) + " does not identify a known participant");
    }
    /* Verify that participant does not have a link already */
    else if (participant_it->second->process)
    {
        throw BadParameter("Participant with ID " + std::to_string(
                          participant_id.value()) + " is already linked with a process");
    }

    /* Get the process */
    auto process_it = processes_.find(process_id);
    if (process_it == processes_.end())
    {
        throw BadParameter("EntityId " + std::to_string(process_id.value()) + " does not identify a known process");
    }

    /* Add reference to process to the participant */
    participant_it->second->process = process_it->second;

    /* Add the participant to the process' list of participants */
    // Not storing the std::shared_ptr<DomainParticipant> in a local variable results in the participant_it->second
    // being moved when inserting it into the map. This causes a SEGFAULT later on when accessing to it.
    auto participant = participant_it->second;
    process_it->second->participants[participant_it->first] = participant;

    // If the participant is active, the process must be active
    if (participant->active)
    {
        if (!participant->process->active)
        {
            change_entity_status_of_kind(participant->process->id, true, EntityKind::PROCESS, domain_id);
        }
    }
}

const std::shared_ptr<const Entity> Database::get_entity(
        const EntityId& entity_id) const
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return get_entity_nts(entity_id);
}

const std::shared_ptr<const Entity> Database::get_entity_nts(
        const EntityId& entity_id) const
{
    /* Iterate over all the collections looking for the entity */
    for (const auto& host_it : hosts_)
    {
        if (host_it.second->id == entity_id)
        {
            return host_it.second;
        }
    }
    for (const auto& process_it : processes_)
    {
        if (process_it.second->id == entity_id)
        {
            return process_it.second;
        }
    }
    for (const auto& user_it : users_)
    {
        if (user_it.second->id == entity_id)
        {
            return user_it.second;
        }
    }
    for (const auto& domain_it : domains_)
    {
        if (domain_it.second->id == entity_id)
        {
            return domain_it.second;
        }
    }
    for (const auto& domain_it : topics_)
    {
        for (const auto& topic_it : domain_it.second)
        {
            if (topic_it.second->id == entity_id)
            {
                return topic_it.second;
            }
        }
    }
    for (const auto& domain_it : participants_)
    {
        for (const auto& participant_it : domain_it.second)
        {
            if (participant_it.second->id == entity_id)
            {
                return participant_it.second;
            }
        }
    }
    for (const auto& domain_it : datareaders_)
    {
        for (const auto& datareader_it : domain_it.second)
        {
            if (datareader_it.second->id == entity_id)
            {
                return datareader_it.second;
            }
        }
    }
    for (const auto& domain_it : datawriters_)
    {
        for (const auto& datawriter_it : domain_it.second)
        {
            if (datawriter_it.second->id == entity_id)
            {
                return datawriter_it.second;
            }
        }
    }
    for (const auto& locator_it : locators_)
    {
        if (locator_it.second->id == entity_id)
        {
            return locator_it.second;
        }
    }
    /* The entity has not been found */
    throw BadParameter("Database does not contain an entity with ID " + std::to_string(entity_id.value()));
}

const std::shared_ptr<Entity> Database::get_mutable_entity_nts(
        const EntityId& entity_id)
{
    /* Iterate over all the collections looking for the entity */
    for (const auto& host_it : hosts_)
    {
        if (host_it.second->id == entity_id)
        {
            return host_it.second;
        }
    }
    for (const auto& process_it : processes_)
    {
        if (process_it.second->id == entity_id)
        {
            return process_it.second;
        }
    }
    for (const auto& user_it : users_)
    {
        if (user_it.second->id == entity_id)
        {
            return user_it.second;
        }
    }
    for (const auto& domain_it : domains_)
    {
        if (domain_it.second->id == entity_id)
        {
            return domain_it.second;
        }
    }
    for (const auto& domain_it : topics_)
    {
        for (const auto& topic_it : domain_it.second)
        {
            if (topic_it.second->id == entity_id)
            {
                return topic_it.second;
            }
        }
    }
    for (const auto& domain_it : participants_)
    {
        for (const auto& participant_it : domain_it.second)
        {
            if (participant_it.second->id == entity_id)
            {
                return participant_it.second;
            }
        }
    }
    for (const auto& domain_it : datareaders_)
    {
        for (const auto& datareader_it : domain_it.second)
        {
            if (datareader_it.second->id == entity_id)
            {
                return datareader_it.second;
            }
        }
    }
    for (const auto& domain_it : datawriters_)
    {
        for (const auto& datawriter_it : domain_it.second)
        {
            if (datawriter_it.second->id == entity_id)
            {
                return datawriter_it.second;
            }
        }
    }
    for (const auto& locator_it : locators_)
    {
        if (locator_it.second->id == entity_id)
        {
            return locator_it.second;
        }
    }
    /* The entity has not been found */
    throw BadParameter("Database does not contain an entity with ID " + std::to_string(entity_id.value()));
}

std::vector<std::pair<EntityId, EntityId>> Database::get_entities_by_name(
        EntityKind entity_kind,
        const std::string& name) const
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return get_entities_by_name_nts(entity_kind, name);
}

std::vector<std::pair<EntityId, EntityId>> Database::get_entities_by_name_nts(
        EntityKind entity_kind,
        const std::string& name) const
{
    std::vector<std::pair<EntityId, EntityId>> entities;
    switch (entity_kind)
    {
        case EntityKind::HOST:
        {
            for (const auto& host_it : hosts_)
            {
                if (host_it.second->name == name)
                {
                    entities.push_back(std::pair<EntityId, EntityId>(EntityId(), host_it.first));
                }
            }
            break;
        }
        case EntityKind::USER:
        {
            for (const auto& user_it : users_)
            {
                if (user_it.second->name == name)
                {
                    entities.push_back(std::pair<EntityId, EntityId>(EntityId(), user_it.first));
                }
            }
            break;
        }
        case EntityKind::PROCESS:
        {
            for (const auto& process_it : processes_)
            {
                if (process_it.second->name == name)
                {
                    entities.push_back(std::pair<EntityId, EntityId>(EntityId(), process_it.first));
                }
            }
            break;
        }
        case EntityKind::DOMAIN:
        {
            for (const auto& domain_it : domains_)
            {
                if (domain_it.second->name == name)
                {
                    entities.push_back(std::pair<EntityId, EntityId>(domain_it.first, domain_it.first));
                }
            }
            break;
        }
        case EntityKind::PARTICIPANT:
        {
            for (const auto& domain_it : participants_)
            {
                for (const auto& participant_it : domain_it.second)
                {
                    if (participant_it.second->name == name)
                    {
                        entities.push_back(std::pair<EntityId, EntityId>(domain_it.first, participant_it.first));
                    }
                }
            }
            break;
        }
        case EntityKind::TOPIC:
        {
            for (const auto& domain_it : topics_)
            {
                for (const auto& topic_it : domain_it.second)
                {
                    if (topic_it.second->name == name)
                    {
                        entities.push_back(std::pair<EntityId, EntityId>(domain_it.first, topic_it.first));
                    }
                }
            }
            break;
        }
        case EntityKind::DATAREADER:
        {
            for (const auto& domain_it : datareaders_)
            {
                for (const auto& datareader_it : domain_it.second)
                {
                    if (datareader_it.second->name == name)
                    {
                        entities.push_back(std::pair<EntityId, EntityId>(domain_it.first, datareader_it.first));
                    }
                }
            }
            break;
        }
        case EntityKind::DATAWRITER:
        {
            for (const auto& domain_it : datawriters_)
            {
                for (const auto& datawriter_it : domain_it.second)
                {
                    if (datawriter_it.second->name == name)
                    {
                        entities.push_back(std::pair<EntityId, EntityId>(domain_it.first, datawriter_it.first));
                    }
                }
            }
            break;
        }
        case EntityKind::LOCATOR:
        {
            for (const auto& locator_it : locators_)
            {
                if (locator_it.second->name == name)
                {
                    entities.push_back(std::pair<EntityId, EntityId>(EntityId(), locator_it.first));
                }
            }
            break;
        }
        default:
        {
            throw BadParameter("Incorrect EntityKind");
        }
    }
    return entities;
}

const std::shared_ptr<const AlertInfo> Database::get_alert_nts(
        const AlertId& alert_id) const
{
    /* Iterate over all the collections looking for the entity */
    for (const auto& alert_it : alerts_)
    {
        if (alert_it.second->get_alert_id() == alert_id)
        {
            return alert_it.second;
        }
    }

    return nullptr;
}

std::string Database::get_type_idl(
        const std::string& type_name) const
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return get_type_idl_nts(type_name);
}

std::string Database::get_type_idl_nts(
        const std::string& type_name) const
{
    auto it = type_idls_.find(type_name);
    if (it != type_idls_.end())
    {
        return it->second;
    }
    throw BadParameter("Type " + type_name + " not found in the database");
}

std::string Database::get_ros2_type_name(
        const std::string& type_name) const
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return get_ros2_type_name_nts(type_name);
}

std::string Database::get_ros2_type_name_nts(
        const std::string& type_name) const
{
    auto it = type_ros2_modified_name_.find(type_name);
    if (it != type_ros2_modified_name_.end())
    {
        // The type was demangled
        return it->second;
    }
    else
    {
        auto it_non_ros2 = type_idls_.find(type_name);
        if (it_non_ros2 != type_idls_.end())
        {
            return type_name;
        }
        throw BadParameter("Type " + type_name + " not found in the database");
    }
}

std::string Database::get_ros2_type_idl(
        const std::string& type_name) const
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return get_ros2_type_idl_nts(type_name);
}

std::string Database::get_ros2_type_idl_nts(
        const std::string& type_name) const
{
    auto it = type_ros2_unmodified_idl_.find(type_name);
    if (it != type_ros2_unmodified_idl_.end())
    {
        // The type was demangled
        return it->second;
    }
    else
    {
        return get_type_idl_nts(type_name);
    }
}

void Database::erase(
        EntityId& domain_id)
{
    // Check that the given domain_id corresponds to a known monitor.
    // Upper layer ensures that the monitor has been stopped.
    // Upper layer also ensures that the monitor_id is valid and corresponds to a known domain.
    if (EntityKind::DOMAIN != get_entity_kind(domain_id))
    {
        throw BadParameter("Incorrect EntityKind");
    }

    // The mutex should be taken only once. get_entity_kind already locks.
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);

    for (auto& reader : datareaders_[domain_id])
    {
        // Unlink related locators
        for (auto locator : reader.second->locators)
        {
            locators_[locator.first]->data_readers.erase(reader.first);
        }
    }
    // Erase datareaders map element
    datareaders_.erase(domain_id);

    // Remove datawriters and unlink related locators
    for (auto& writer : datawriters_[domain_id])
    {
        // Unlink related locators
        for (auto locator : writer.second->locators)
        {
            locators_[locator.first]->data_writers.erase(writer.first);
        }
    }
    // Erase datawriters map element
    datawriters_.erase(domain_id);

    // Erase topics map element
    topics_.erase(domain_id);

    // Remove participants and unlink related process
    for (auto& participant : participants_[domain_id])
    {
        // Unlink related process if it exists
        if (participant.second->process)
        {
            processes_[participant.second->process->id]->participants.erase(participant.first);
            bool change_status = true;
            if (!processes_[participant.second->process->id]->participants.empty())
            {
                for (const auto& entity_it : processes_[participant.second->process->id]->participants)
                {
                    if (entity_it.second->active)
                    {
                        change_status = false;
                        break;
                    }
                }
            }
            if (change_status)
            {
                change_entity_status_of_kind(processes_[participant.second->process->id]->id, false,
                        EntityKind::PROCESS, domain_id);
            }
        }
    }
    // Erase participants map element
    participants_.erase(domain_id);

    // Keep domain
    domains_[domain_id]->topics.clear();
    domains_[domain_id]->participants.clear();

    // Regenerate domain_view_graph
    if (regenerate_domain_graph_nts(domain_id))
    {
        // TODO (eProsima) Workaround to avoid deadlock if callback implementation requires taking the database
        // mutex (e.g. by calling get_info). A refactor for not calling on_domain_view_graph_update from within
        // this function would be required.
        execute_without_lock([&]()
                {
                    details::StatisticsBackendData::get_instance()->on_domain_view_graph_update(domain_id);
                });
    }
}

std::vector<const StatisticsSample*> Database::select(
        DataKind data_type,
        EntityId entity_id_source,
        EntityId entity_id_target,
        Timestamp t_from,
        Timestamp t_to)
{
    /* Check that the given timestamps are consistent */
    if (t_to <= t_from)
    {
        throw BadParameter("Final timestamp must be strictly greater than the origin timestamp");
    }

    std::shared_lock<std::shared_timed_mutex> lock(mutex_);

    auto source_entity = get_entity_nts(entity_id_source);
    auto target_entity = get_entity_nts(entity_id_target);

    std::vector<const StatisticsSample*> samples;
    switch (data_type)
    {
        case DataKind::FASTDDS_LATENCY:
        {
            assert(EntityKind::DATAWRITER == source_entity->kind);
            assert(EntityKind::DATAREADER == target_entity->kind);
            auto writer = std::static_pointer_cast<const DataWriter>(source_entity);
            /* Look if the writer has information about the required reader */
            auto reader = writer->data.history2history_latency.find(entity_id_target);
            if (reader != writer->data.history2history_latency.end())
            {
                /* Look for the samples between the given timestamps */
                // TODO(jlbueno) Knowing that the samples are ordered by timestamp it would be more efficient to
                // implement a binary search (PR#58 Originally posted by @IkerLuengo in
                // https://github.com/eProsima/Fast-DDS-statistics-backend/pull/58#discussion_r629383536)
                for (auto& sample : reader->second)
                {
                    if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                    {
                        samples.push_back(&sample);
                    }
                    else if (sample.src_ts > t_to)
                    {
                        break;
                    }
                }
            }
            break;
        }
        case DataKind::NETWORK_LATENCY:
        {
            assert(EntityKind::PARTICIPANT == source_entity->kind);
            assert(EntityKind::LOCATOR == target_entity->kind);
            auto participant = std::static_pointer_cast<const DomainParticipant>(source_entity);
            /* Look if the participant has information about the required locator */
            auto remote_locator = participant->data.network_latency_per_locator.find(entity_id_target);
            if (remote_locator != participant->data.network_latency_per_locator.end())
            {
                /* Look for the samples between the given timestamps */
                for (auto& sample : remote_locator->second)
                {
                    if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                    {
                        samples.push_back(&sample);
                    }
                    else if (sample.src_ts > t_to)
                    {
                        break;
                    }
                }
            }
            break;
        }
        case DataKind::RTPS_PACKETS_SENT:
        {
            assert(EntityKind::PARTICIPANT == source_entity->kind);
            assert(EntityKind::LOCATOR == target_entity->kind);
            auto participant = std::static_pointer_cast<const DomainParticipant>(source_entity);
            /* Look if the participant has information about the required locator */
            auto locator = participant->data.rtps_packets_sent.find(entity_id_target);
            if (locator != participant->data.rtps_packets_sent.end())
            {
                /* Look for the samples between the given timestamps */
                for (auto& sample : locator->second)
                {
                    if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                    {
                        samples.push_back(&sample);
                    }
                    else if (sample.src_ts > t_to)
                    {
                        break;
                    }
                }
            }
            break;
        }
        case DataKind::RTPS_BYTES_SENT:
        {
            assert(EntityKind::PARTICIPANT == source_entity->kind);
            assert(EntityKind::LOCATOR == target_entity->kind);
            auto participant = std::static_pointer_cast<const DomainParticipant>(source_entity);
            /* Look if the participant has information about the required locator */
            auto locator = participant->data.rtps_bytes_sent.find(entity_id_target);
            if (locator != participant->data.rtps_bytes_sent.end())
            {
                /* Look for the samples between the given timestamps */
                for (auto& sample : locator->second)
                {
                    if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                    {
                        samples.push_back(&sample);
                    }
                    else if (sample.src_ts > t_to)
                    {
                        break;
                    }
                }
            }
            break;
        }
        case DataKind::RTPS_PACKETS_LOST:
        {
            assert(EntityKind::PARTICIPANT == source_entity->kind);
            assert(EntityKind::LOCATOR == target_entity->kind);
            auto participant = std::static_pointer_cast<const DomainParticipant>(source_entity);
            /* Look if the participant has information about the required locator */
            auto locator = participant->data.rtps_packets_lost.find(entity_id_target);
            if (locator != participant->data.rtps_packets_lost.end())
            {
                /* Look for the samples between the given timestamps */
                for (auto& sample : locator->second)
                {
                    if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                    {
                        samples.push_back(&sample);
                    }
                    else if (sample.src_ts > t_to)
                    {
                        break;
                    }
                }
            }
            break;
        }
        case DataKind::RTPS_BYTES_LOST:
        {
            assert(EntityKind::PARTICIPANT == source_entity->kind);
            assert(EntityKind::LOCATOR == target_entity->kind);
            auto participant = std::static_pointer_cast<const DomainParticipant>(source_entity);
            /* Look if the participant has information about the required locator */
            auto locator = participant->data.rtps_bytes_lost.find(entity_id_target);
            if (locator != participant->data.rtps_bytes_lost.end())
            {
                /* Look for the samples between the given timestamps */
                for (auto& sample : locator->second)
                {
                    if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                    {
                        samples.push_back(&sample);
                    }
                    else if (sample.src_ts > t_to)
                    {
                        break;
                    }
                }
            }
            break;
        }
        case DataKind::DISCOVERY_TIME:
        {
            assert(EntityKind::PARTICIPANT == source_entity->kind);
            assert(EntityKind::PARTICIPANT == target_entity->kind || EntityKind::DATAREADER == target_entity->kind ||
                    EntityKind::DATAWRITER == target_entity->kind);
            auto participant = std::static_pointer_cast<const DomainParticipant>(source_entity);
            /* Look if the participant has information about the required dds entity */
            auto dds_entity = participant->data.discovered_entity.find(entity_id_target);
            if (dds_entity != participant->data.discovered_entity.end())
            {
                /* Look for the samples between the given timestamps */
                for (auto& sample : dds_entity->second)
                {
                    if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                    {
                        samples.push_back(&sample);
                    }
                    else if (sample.src_ts > t_to)
                    {
                        break;
                    }
                }
            }
            break;
        }
        default:
        {
            throw BadParameter("Incorrect DataKind");
        }
    }
    return samples;
}

std::vector<const StatisticsSample*> Database::select(
        DataKind data_type,
        EntityId entity_id,
        Timestamp t_from,
        Timestamp t_to)
{
    /* Check that the given timestamps are consistent */
    if (t_to <= t_from)
    {
        throw BadParameter("Final timestamp must be strictly greater than the origin timestamp");
    }

    std::shared_lock<std::shared_timed_mutex> lock(mutex_);

    auto entity = get_entity_nts(entity_id);

    std::vector<const StatisticsSample*> samples;
    switch (data_type)
    {
        case DataKind::PUBLICATION_THROUGHPUT:
        {
            assert(EntityKind::DATAWRITER == entity->kind);
            auto writer = std::static_pointer_cast<const DataWriter>(entity);
            /* Look for the samples between the given timestamps */
            // TODO(jlbueno) Knowing that the samples are ordered by timestamp it would be more efficient to
            // implement a binary search (PR#58 Originally posted by @IkerLuengo in
            // https://github.com/eProsima/Fast-DDS-statistics-backend/pull/58#discussion_r629383536)
            for (auto& sample : writer->data.publication_throughput)
            {
                /* The data is assumed to be ordered by timestamp */
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::SUBSCRIPTION_THROUGHPUT:
        {
            assert(EntityKind::DATAREADER == entity->kind);
            auto reader = std::static_pointer_cast<const DataReader>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : reader->data.subscription_throughput)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }

            break;
        }
        case DataKind::RESENT_DATA:
        {
            assert(EntityKind::DATAWRITER == entity->kind);
            auto writer = std::static_pointer_cast<const DataWriter>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : writer->data.resent_datas)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::HEARTBEAT_COUNT:
        {
            assert(EntityKind::DATAWRITER == entity->kind);
            auto writer = std::static_pointer_cast<const DataWriter>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : writer->data.heartbeat_count)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::ACKNACK_COUNT:
        {
            assert(EntityKind::DATAREADER == entity->kind);
            auto reader = std::static_pointer_cast<const DataReader>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : reader->data.acknack_count)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::NACKFRAG_COUNT:
        {
            assert(EntityKind::DATAREADER == entity->kind);
            auto reader = std::static_pointer_cast<const DataReader>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : reader->data.nackfrag_count)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::GAP_COUNT:
        {
            assert(EntityKind::DATAWRITER == entity->kind);
            auto writer = std::static_pointer_cast<const DataWriter>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : writer->data.gap_count)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::DATA_COUNT:
        {
            assert(EntityKind::DATAWRITER == entity->kind);
            auto writer = std::static_pointer_cast<const DataWriter>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : writer->data.data_count)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::PDP_PACKETS:
        {
            assert(EntityKind::PARTICIPANT == entity->kind);
            auto participant = std::static_pointer_cast<const DomainParticipant>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : participant->data.pdp_packets)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::EDP_PACKETS:
        {
            assert(EntityKind::PARTICIPANT == entity->kind);
            auto participant = std::static_pointer_cast<const DomainParticipant>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : participant->data.edp_packets)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::SAMPLE_DATAS:
        {
            assert(EntityKind::DATAWRITER == entity->kind);
            auto writer = std::static_pointer_cast<const DataWriter>(entity);
            /* This case is different from the above. Check all map keys and add sample if it is between the given
               timestamps. The samples do not need to be ordered by source timestamp so they should be sorted */
            for (auto& sample : writer->data.sample_datas)
            {
                // Vector only has one element
                if (sample.second[0].src_ts >= t_from && sample.second[0].src_ts <= t_to)
                {
                    samples.push_back(&sample.second[0]);
                }
            }
            std::sort(samples.begin(), samples.end(), [](
                        const StatisticsSample* first,
                        const StatisticsSample* second)
                    {
                        return first->src_ts < second->src_ts;
                    });
            break;
        }
        // Any other data_type corresponds to a sample which needs two entities or a DataKind::INVALID
        default:
        {
            throw BadParameter("Incorrect DataKind");
        }
    }
    return samples;
}

template <>
void Database::get_status_data(
        const EntityId& entity_id,
        ProxySample& status_data)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);

    std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);

    switch (entity->kind)
    {
        case EntityKind::PARTICIPANT:
        {
            std::shared_ptr<const DomainParticipant> participant = std::dynamic_pointer_cast<const DomainParticipant>(
                entity);
            if (!participant->monitor_service_data.proxy.empty())
            {
                status_data = participant->monitor_service_data.proxy.back();
            }
            break;
        }
        case EntityKind::DATAREADER:
        {
            std::shared_ptr<const DataReader> datareader = std::dynamic_pointer_cast<const DataReader>(entity);
            if (!datareader->monitor_service_data.proxy.empty())
            {
                status_data = datareader->monitor_service_data.proxy.back();
            }
            break;
        }
        case EntityKind::DATAWRITER:
        {
            std::shared_ptr<const DataWriter> datawriter = std::dynamic_pointer_cast<const DataWriter>(entity);
            if (!datawriter->monitor_service_data.proxy.empty())
            {
                status_data = datawriter->monitor_service_data.proxy.back();
            }
            break;
        }
        default:
        {
            throw BadParameter("Unsupported PROXY Status type and EntityKind combination");
        }
    }
}

template <>
void Database::get_status_data(
        const EntityId& entity_id,
        ConnectionListSample& status_data)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);

    std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);

    switch (entity->kind)
    {
        case EntityKind::PARTICIPANT:
        {
            std::shared_ptr<const DomainParticipant> participant = std::dynamic_pointer_cast<const DomainParticipant>(
                entity);
            if (!participant->monitor_service_data.connection_list.empty())
            {
                status_data = participant->monitor_service_data.connection_list.back();
            }
            break;
        }
        case EntityKind::DATAREADER:
        {
            std::shared_ptr<const DataReader> datareader = std::dynamic_pointer_cast<const DataReader>(entity);
            if (!datareader->monitor_service_data.connection_list.empty())
            {
                status_data = datareader->monitor_service_data.connection_list.back();
            }
            break;
        }
        case EntityKind::DATAWRITER:
        {
            std::shared_ptr<const DataWriter> datawriter = std::dynamic_pointer_cast<const DataWriter>(entity);
            if (!datawriter->monitor_service_data.connection_list.empty())
            {
                status_data = datawriter->monitor_service_data.connection_list.back();
            }
            break;
        }
        default:
        {
            throw BadParameter("Unsupported CONNECTION_LIST Status type and EntityKind combination");
        }
    }
}

template <>
void Database::get_status_data(
        const EntityId& entity_id,
        IncompatibleQosSample& status_data)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);

    std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);

    switch (entity->kind)
    {
        case EntityKind::DATAREADER:
        {
            std::shared_ptr<const DataReader> datareader = std::dynamic_pointer_cast<const DataReader>(entity);
            if (!datareader->monitor_service_data.incompatible_qos.empty())
            {
                status_data = datareader->monitor_service_data.incompatible_qos.back();
            }
            break;
        }
        case EntityKind::DATAWRITER:
        {
            std::shared_ptr<const DataWriter> datawriter = std::dynamic_pointer_cast<const DataWriter>(entity);
            if (!datawriter->monitor_service_data.incompatible_qos.empty())
            {
                status_data = datawriter->monitor_service_data.incompatible_qos.back();
            }
            break;
        }
        default:
        {
            throw BadParameter("Unsupported INCOMPATIBLE_QOS Status type and EntityKind combination");
        }
    }
}

template <>
void Database::get_status_data(
        const EntityId& entity_id,
        InconsistentTopicSample& status_data)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);

    std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);

    switch (entity->kind)
    {
        case EntityKind::DATAREADER:
        {
            std::shared_ptr<const DataReader> datareader = std::dynamic_pointer_cast<const DataReader>(entity);
            if (!datareader->monitor_service_data.inconsistent_topic.empty())
            {
                status_data = datareader->monitor_service_data.inconsistent_topic.back();
            }
            break;
        }
        case EntityKind::DATAWRITER:
        {
            std::shared_ptr<const DataWriter> datawriter = std::dynamic_pointer_cast<const DataWriter>(entity);
            if (!datawriter->monitor_service_data.inconsistent_topic.empty())
            {
                status_data = datawriter->monitor_service_data.inconsistent_topic.back();
            }
            break;
        }
        default:
        {
            throw BadParameter("Unsupported INCONSISTENT_TOPIC Status type and EntityKind combination");
        }
    }
}

template <>
void Database::get_status_data(
        const EntityId& entity_id,
        LivelinessLostSample& status_data)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);

    std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);

    if (entity->kind == EntityKind::DATAWRITER)
    {
        std::shared_ptr<const DataWriter> datawriter = std::dynamic_pointer_cast<const DataWriter>(entity);
        if (!datawriter->monitor_service_data.liveliness_lost.empty())
        {
            status_data = datawriter->monitor_service_data.liveliness_lost.back();
        }
    }
    else
    {
        throw BadParameter("Unsupported LIVELINES_LOST Status type and EntityKind combination");
    }
}

template <>
void Database::get_status_data(
        const EntityId& entity_id,
        LivelinessChangedSample& status_data)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);

    std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);

    if (entity->kind == EntityKind::DATAREADER)
    {
        std::shared_ptr<const DataReader> datareader = std::dynamic_pointer_cast<const DataReader>(entity);
        if (!datareader->monitor_service_data.liveliness_changed.empty())
        {
            status_data = datareader->monitor_service_data.liveliness_changed.back();
        }
    }
    else
    {
        throw BadParameter("Unsupported LIVELINESS_CHANGED Status type and EntityKind combination");
    }
}

template <>
void Database::get_status_data(
        const EntityId& entity_id,
        DeadlineMissedSample& status_data)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);

    std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);

    switch (entity->kind)
    {
        case EntityKind::DATAREADER:
        {
            std::shared_ptr<const DataReader> datareader = std::dynamic_pointer_cast<const DataReader>(entity);
            if (!datareader->monitor_service_data.deadline_missed.empty())
            {
                status_data = datareader->monitor_service_data.deadline_missed.back();
            }
            break;
        }
        case EntityKind::DATAWRITER:
        {
            std::shared_ptr<const DataWriter> datawriter = std::dynamic_pointer_cast<const DataWriter>(entity);
            if (!datawriter->monitor_service_data.deadline_missed.empty())
            {
                status_data = datawriter->monitor_service_data.deadline_missed.back();
            }
            break;
        }
        default:
        {
            throw BadParameter("Unsupported DEADLINE_MISSED Status type and EntityKind combination");
        }
    }
}

template <>
void Database::get_status_data(
        const EntityId& entity_id,
        SampleLostSample& status_data)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);

    std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);

    if (entity->kind == EntityKind::DATAREADER)
    {
        std::shared_ptr<const DataReader> datareader = std::dynamic_pointer_cast<const DataReader>(entity);
        if (!datareader->monitor_service_data.sample_lost.empty())
        {
            status_data = datareader->monitor_service_data.sample_lost.back();
        }
    }
    else
    {
        throw BadParameter("Unsupported SAMPLE_LOST Status type and EntityKind combination");
    }
}

template <>
void Database::get_status_data(
        const EntityId& entity_id,
        ExtendedIncompatibleQosSample& status_data)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);

    std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);

    switch (entity->kind)
    {
        case EntityKind::DATAREADER:
        {
            std::shared_ptr<const DataReader> datareader = std::dynamic_pointer_cast<const DataReader>(entity);
            if (!datareader->monitor_service_data.extended_incompatible_qos.empty())
            {
                status_data = datareader->monitor_service_data.extended_incompatible_qos.back();
            }
            break;
        }
        case EntityKind::DATAWRITER:
        {
            std::shared_ptr<const DataWriter> datawriter = std::dynamic_pointer_cast<const DataWriter>(entity);
            if (!datawriter->monitor_service_data.extended_incompatible_qos.empty())
            {
                status_data = datawriter->monitor_service_data.extended_incompatible_qos.back();
            }
            break;
        }
        default:
        {
            throw BadParameter("Unsupported EXTENDED_INCOMPATIBLE_QOS Status type and EntityKind combination");
        }
    }
}

bool Database::is_entity_present(
        const EntityId& entity_id) const noexcept
{
    // TODO: once the method get_entity allow calling it with an EntityKind, change this to allow it too.
    try
    {
        // Use get_entity search
        std::shared_lock<std::shared_timed_mutex> lock(mutex_);
        auto result = get_entity_nts(entity_id);
        return result.operator bool();
    }
    catch (const BadParameter&)
    {
        return false;
    }
}

std::pair<EntityId, EntityId> Database::get_entity_by_guid(
        EntityKind entity_kind,
        const std::string& guid) const
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return get_entity_by_guid_nts(entity_kind, guid);
}

std::pair<EntityId, EntityId> Database::get_entity_by_guid_nts(
        EntityKind entity_kind,
        const std::string& guid) const
{
    switch (entity_kind)
    {
        case EntityKind::PARTICIPANT:
        {
            for (const auto& domain_it : participants_)
            {
                for (const auto& participant_it : domain_it.second)
                {
                    if (participant_it.second->guid == guid)
                    {
                        return std::pair<EntityId, EntityId>(domain_it.first, participant_it.first);
                    }
                }
            }
            break;
        }
        case EntityKind::DATAREADER:
        {
            for (const auto& domain_it : datareaders_)
            {
                for (const auto& datareader_it : domain_it.second)
                {
                    if (datareader_it.second->guid == guid)
                    {
                        return std::pair<EntityId, EntityId>(domain_it.first, datareader_it.first);
                    }
                }
            }
            break;
        }
        case EntityKind::DATAWRITER:
        {
            for (const auto& domain_it : datawriters_)
            {
                for (const auto& datawriter_it : domain_it.second)
                {
                    if (datawriter_it.second->guid == guid)
                    {
                        return std::pair<EntityId, EntityId>(domain_it.first, datawriter_it.first);
                    }
                }
            }
            break;
        }
        default:
        {
            throw BadParameter("Incorrect EntityKind");
        }
    }

    throw BadParameter("No entity of type " + std::to_string(
                      static_cast<int>(entity_kind)) + " and GUID " + guid + " exists");
}

EntityKind Database::get_entity_kind_by_guid(
        const eprosima::fastdds::statistics::detail::GUID_s& guid_s) const
{

    eprosima::fastdds::rtps::EntityId_t entity_id_t;
    for (size_t i = 0; i < entity_id_t.size; ++i)
    {
        entity_id_t.value[i] = guid_s.entityId().value()[i];
    }

    if (entity_id_t == eprosima::fastdds::rtps::c_EntityId_RTPSParticipant)
    {
        return EntityKind::PARTICIPANT;
    }
    else if (entity_id_t.is_reader())
    {
        return EntityKind::DATAREADER;

    }
    else if (entity_id_t.is_writer())
    {
        return EntityKind::DATAWRITER;

    }
    else
    {
        throw BadParameter("No Participant, Datawriter or Datareader with provided GUID exists");
    }
}

EntityKind Database::get_entity_kind(
        EntityId entity_id) const
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return get_entity_nts(entity_id)->kind;
}

StatusLevel Database::get_entity_status(
        EntityId entity_id) const
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return get_entity_nts(entity_id)->status;
}

Graph Database::get_domain_view_graph(
        const EntityId& domain_id) const
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return get_domain_view_graph_nts(domain_id);
}

Graph Database::get_domain_view_graph_nts(
        const EntityId& domain_id) const
{
    try
    {
        return domain_view_graph.at(domain_id);
    }
    catch (const std::out_of_range& /*unused*/)
    {
        throw BadParameter("Invalid Domain EntityId");
    }
}

void Database::init_domain_view_graph(
        const std::string& domain_name,
        const DomainId domain_id,
        const EntityId& domain_entity_id)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);
    init_domain_view_graph_nts(domain_name, domain_id, domain_entity_id);
}

void Database::init_domain_view_graph_nts(
        const std::string& domain_name,
        const DomainId domain_id,
        const EntityId& domain_entity_id)
{
    domain_view_graph[domain_entity_id][KIND_TAG] = DOMAIN_ENTITY_TAG;
    domain_view_graph[domain_entity_id][DOMAIN_INFO_TAG][DOMAIN_NAME_TAG] = domain_name;
    domain_view_graph[domain_entity_id][DOMAIN_INFO_TAG][DOMAIN_ID_TAG] = domain_id;
    domain_view_graph[domain_entity_id][TOPIC_CONTAINER_TAG] = nlohmann::json::object();
    domain_view_graph[domain_entity_id][HOST_CONTAINER_TAG] = nlohmann::json::object();
}

bool Database::update_participant_in_graph(
        const EntityId& domain_entity_id,
        const EntityId& host_entity_id,
        const EntityId& user_entity_id,
        const EntityId& process_entity_id,
        const EntityId& participant_entity_id)

{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);
    return update_participant_in_graph_nts(domain_entity_id, host_entity_id, user_entity_id, process_entity_id,
                   participant_entity_id);
}

bool Database::update_participant_in_graph_nts(
        const EntityId& domain_entity_id,
        const EntityId& host_entity_id,
        const EntityId& user_entity_id,
        const EntityId& process_entity_id,
        const EntityId& participant_entity_id)

{
    bool graph_updated = false;

    // Check if the correspondent domain graph exists
    if (domain_view_graph.find(domain_entity_id) == domain_view_graph.end())
    {
        return graph_updated;
    }

    // Check if the correspondent host subgraph exists
    if (host_entity_id.value() == EntityId::invalid() || host_entity_id.value() == EntityId::all())
    {
        return graph_updated;
    }

    Graph* domain_graph = &domain_view_graph[domain_entity_id];

    std::shared_ptr<const Entity> host_entity = get_entity_nts(host_entity_id);
    std::string host_entity_id_value = std::to_string(host_entity_id.value());
    if (host_entity->active)
    {
        if ((*domain_graph)[HOST_CONTAINER_TAG][host_entity_id_value].empty())
        {
            (*domain_graph)[HOST_CONTAINER_TAG][host_entity_id_value][USER_CONTAINER_TAG] = nlohmann::json::object();
        }
        graph_updated =
                get_entity_subgraph_nts(host_entity_id,
                        (*domain_graph)[HOST_CONTAINER_TAG][host_entity_id_value]) || graph_updated;
    }
    else
    {
        if ((*domain_graph)[HOST_CONTAINER_TAG].find(host_entity_id_value) != (*domain_graph)[HOST_CONTAINER_TAG].end())
        {
            (*domain_graph)[HOST_CONTAINER_TAG].erase(host_entity_id_value);
            return true;
        }
        return graph_updated;
    }

    // Check if the correspondent user subgraph exists
    if (user_entity_id.value() == EntityId::invalid() || user_entity_id.value() == EntityId::all())
    {
        return graph_updated;
    }

    Graph* host_graph = &(*domain_graph)[HOST_CONTAINER_TAG][host_entity_id_value];

    std::shared_ptr<const Entity> user_entity = get_entity_nts(user_entity_id);
    std::string user_entity_id_value = std::to_string(user_entity_id.value());
    if (user_entity->active)
    {
        if ((*host_graph)[USER_CONTAINER_TAG][user_entity_id_value].empty())
        {
            (*host_graph)[USER_CONTAINER_TAG][user_entity_id_value][PROCESS_CONTAINER_TAG] = nlohmann::json::object();
        }
        graph_updated =
                get_entity_subgraph_nts(user_entity_id,
                        (*host_graph)[USER_CONTAINER_TAG][user_entity_id_value]) || graph_updated;
    }
    else
    {
        if ((*host_graph)[USER_CONTAINER_TAG].find(user_entity_id_value) != (*host_graph)[USER_CONTAINER_TAG].end())
        {
            (*host_graph)[USER_CONTAINER_TAG].erase(user_entity_id_value);
            return true;
        }
        return graph_updated;
    }

    // Check if the correspondent process subgraph exists
    if (process_entity_id.value() == EntityId::invalid() || process_entity_id.value() == EntityId::all())
    {
        return graph_updated;
    }

    Graph* user_graph = &(*host_graph)[USER_CONTAINER_TAG][user_entity_id_value];

    std::shared_ptr<const Entity> process_entity = get_entity_nts(process_entity_id);
    std::string process_entity_id_value = std::to_string(process_entity_id.value());
    if (process_entity->active)
    {
        if ((*user_graph)[PROCESS_CONTAINER_TAG][process_entity_id_value].empty())
        {
            (*user_graph)[PROCESS_CONTAINER_TAG][process_entity_id_value][PARTICIPANT_CONTAINER_TAG] =
                    nlohmann::json::object();
        }
        graph_updated =
                get_entity_subgraph_nts(process_entity_id,
                        (*user_graph)[PROCESS_CONTAINER_TAG][process_entity_id_value]) || graph_updated;
    }
    else
    {
        if ((*user_graph)[PROCESS_CONTAINER_TAG].find(process_entity_id_value) !=
                (*user_graph)[PROCESS_CONTAINER_TAG].end())
        {
            (*user_graph)[PROCESS_CONTAINER_TAG].erase(process_entity_id_value);
            return true;
        }
        return graph_updated;
    }

    // Check if the correspondent participant subgraph exists
    if (participant_entity_id.value() == EntityId::invalid()  || participant_entity_id.value() == EntityId::all())
    {
        return graph_updated;
    }

    Graph* process_graph = &(*user_graph)[PROCESS_CONTAINER_TAG][process_entity_id_value];

    std::shared_ptr<const Entity> participant_entity = get_entity_nts(participant_entity_id);
    std::string participant_entity_id_value = std::to_string(participant_entity_id.value());
    if (participant_entity->active)
    {
        if ((*process_graph)[PARTICIPANT_CONTAINER_TAG][participant_entity_id_value].empty())
        {
            (*process_graph)[PARTICIPANT_CONTAINER_TAG][participant_entity_id_value][ENDPOINT_CONTAINER_TAG] =
                    nlohmann::json::object();
        }
        graph_updated =
                get_entity_subgraph_nts(participant_entity_id,
                        (*process_graph)[PARTICIPANT_CONTAINER_TAG][participant_entity_id_value]) ||
                graph_updated;
    }
    else
    {
        if ((*process_graph)[PARTICIPANT_CONTAINER_TAG].find(participant_entity_id_value) !=
                (*process_graph)[PARTICIPANT_CONTAINER_TAG].end())
        {
            (*process_graph)[PARTICIPANT_CONTAINER_TAG].erase(participant_entity_id_value);
            return true;
        }
        return graph_updated;
    }

    return graph_updated;
}

bool Database::update_endpoint_in_graph(
        const EntityId& domain_entity_id,
        const EntityId& participant_entity_id,
        const EntityId& topic_entity_id,
        const EntityId& endpoint_entity_id)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);
    return update_endpoint_in_graph_nts(domain_entity_id, participant_entity_id, topic_entity_id, endpoint_entity_id);
}

bool Database::update_endpoint_in_graph_nts(
        const EntityId& domain_entity_id,
        const EntityId& participant_entity_id,
        const EntityId& topic_entity_id,
        const EntityId& endpoint_entity_id)
{
    bool graph_updated = false;

    // Check if the correspondent domain graph exists
    if (domain_view_graph.find(domain_entity_id) == domain_view_graph.end())
    {
        return graph_updated;
    }

    Graph* domain_graph = &domain_view_graph[domain_entity_id];

    // Check if the correspondent topic graph exists
    if (topic_entity_id.value() != EntityId::invalid() && topic_entity_id.value() != EntityId::all())
    {

        std::shared_ptr<const Entity> topic_entity = get_entity_nts(topic_entity_id);
        std::string topic_entity_id_value = std::to_string(topic_entity_id.value());
        if (topic_entity->active)
        {
            graph_updated =
                    get_entity_subgraph_nts(topic_entity_id,
                            (*domain_graph)[TOPIC_CONTAINER_TAG][topic_entity_id_value]) || graph_updated;
        }
        else
        {
            if ((*domain_graph)[TOPIC_CONTAINER_TAG].find(topic_entity_id_value) !=
                    (*domain_graph)[TOPIC_CONTAINER_TAG].end())
            {
                (*domain_graph)[TOPIC_CONTAINER_TAG].erase(topic_entity_id_value);
                graph_updated = true;
            }
        }
    }

    // Check if participant entityid is valid and unique
    if (participant_entity_id.value() == EntityId::invalid() || participant_entity_id.value() == EntityId::all())
    {
        return graph_updated;
    }

    // Get process->user->host ids
    std::shared_ptr<const Entity> participant_entity = get_entity_nts(participant_entity_id);
    std::shared_ptr<const DomainParticipant> participant =
            std::dynamic_pointer_cast<const DomainParticipant>(participant_entity);

    if (participant->process == nullptr)
    {
        return graph_updated;
    }
    std::shared_ptr<const Process> process = participant->process;
    if (process->user == nullptr)
    {
        return graph_updated;
    }
    std::shared_ptr<const User> user = process->user;
    if (user->host == nullptr)
    {
        return graph_updated;
    }
    std::shared_ptr<const Host> host = user->host;

    std::string participant_entity_id_value = std::to_string(participant_entity_id.value());
    std::string process_entity_id_value = std::to_string(process->id.value());
    std::string user_entity_id_value = std::to_string(user->id.value());
    std::string host_entity_id_value = std::to_string(host->id.value());

    // Check if the correspondent host-user-process-participant graph exists
    if ((*domain_graph)[HOST_CONTAINER_TAG].find(host_entity_id_value) == (*domain_graph)[HOST_CONTAINER_TAG].end())
    {
        return graph_updated;
    }

    Graph* host_graph = &(*domain_graph)[HOST_CONTAINER_TAG][host_entity_id_value];

    if ((*host_graph)[USER_CONTAINER_TAG].find(user_entity_id_value) == (*host_graph)[USER_CONTAINER_TAG].end())
    {
        return graph_updated;
    }

    Graph* user_graph = &(*host_graph)[USER_CONTAINER_TAG][user_entity_id_value];

    if ((*user_graph)[PROCESS_CONTAINER_TAG].find(process_entity_id_value) ==
            (*user_graph)[PROCESS_CONTAINER_TAG].end())
    {
        return graph_updated;
    }

    Graph* process_graph = &(*user_graph)[PROCESS_CONTAINER_TAG][process_entity_id_value];

    if ((*process_graph)[PARTICIPANT_CONTAINER_TAG].find(participant_entity_id_value) ==
            (*process_graph)[PARTICIPANT_CONTAINER_TAG].end())
    {
        return graph_updated;
    }

    // Check if the correspondent endpoint subgraph exists
    if (endpoint_entity_id.value() == EntityId::invalid() || endpoint_entity_id.value() == EntityId::all())
    {
        return graph_updated;
    }

    Graph* participant_graph = &(*process_graph)[PARTICIPANT_CONTAINER_TAG][participant_entity_id_value];

    std::shared_ptr<const Entity> endpoint_entity = get_entity_nts(endpoint_entity_id);
    std::string endpoint_entity_id_value = std::to_string(endpoint_entity_id.value());
    if (endpoint_entity->active)
    {
        graph_updated =
                get_entity_subgraph_nts(endpoint_entity_id,
                        (*participant_graph)[ENDPOINT_CONTAINER_TAG][endpoint_entity_id_value]) ||
                graph_updated;
        return graph_updated;
    }
    else
    {
        if ((*participant_graph)[ENDPOINT_CONTAINER_TAG].find(endpoint_entity_id_value) !=
                (*participant_graph)[ENDPOINT_CONTAINER_TAG].end())
        {
            (*participant_graph)[ENDPOINT_CONTAINER_TAG].erase(endpoint_entity_id_value);
            return true;
        }
        return graph_updated;
    }
}

bool Database::regenerate_domain_graph(
        const EntityId& domain_entity_id)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);
    return regenerate_domain_graph_nts(domain_entity_id);
}

bool Database::regenerate_domain_graph_nts(
        const EntityId& domain_entity_id)
{
    // Check if the correspondent domain graph exists
    if (domain_view_graph.find(domain_entity_id) != domain_view_graph.end())
    {
        domain_view_graph.erase(domain_entity_id);
    }
    else
    {
        EPROSIMA_LOG_WARNING(BACKEND_DATABASE,
                "Error regenerating graph. No previous graph was found");
        return false;
    }

    Graph* domain_graph = &domain_view_graph[domain_entity_id];

    std::shared_ptr<const Entity> db_entity = get_entity_nts(domain_entity_id);
    assert((db_entity != nullptr) && (db_entity->kind == EntityKind::DOMAIN));

    std::shared_ptr<const Domain> domain_entity = std::static_pointer_cast<const Domain>(db_entity);
    (*domain_graph)[KIND_TAG] = DOMAIN_ENTITY_TAG;
    (*domain_graph)[DOMAIN_INFO_TAG][DOMAIN_NAME_TAG] = domain_entity->name;
    (*domain_graph)[DOMAIN_INFO_TAG][DOMAIN_ID_TAG] = domain_entity->domain_id;
    (*domain_graph)[TOPIC_CONTAINER_TAG] = nlohmann::json::object();
    (*domain_graph)[HOST_CONTAINER_TAG] = nlohmann::json::object();

    // Add topics
    auto topics = get_entities_nts(EntityKind::TOPIC, domain_entity_id);
    for (auto topic : topics)
    {
        if (!topic->active)
        {
            continue;
        }
        std::string topic_entity_id_value = std::to_string(topic->id.value());
        get_entity_subgraph_nts(topic->id.value(), (*domain_graph)[TOPIC_CONTAINER_TAG][topic_entity_id_value]);
    }

    // Add hosts
    auto hosts = get_entities_nts(EntityKind::HOST, domain_entity_id);
    for (auto host : hosts)
    {
        if (!host->active)
        {
            continue;
        }

        std::string host_entity_id_value = std::to_string(host->id.value());
        get_entity_subgraph_nts(host->id.value(), (*domain_graph)[HOST_CONTAINER_TAG][host_entity_id_value]);

        Graph* host_graph = &(*domain_graph)[HOST_CONTAINER_TAG][host_entity_id_value];
        (*host_graph)[USER_CONTAINER_TAG] = nlohmann::json::object();
        auto users = get_entities_nts(EntityKind::USER, host);

        // Add users
        for (auto user : users)
        {
            if (!user->active)
            {
                continue;
            }

            std::string user_entity_id_value = std::to_string(user->id.value());
            get_entity_subgraph_nts(user->id.value(), (*host_graph)[USER_CONTAINER_TAG][user_entity_id_value]);

            Graph* user_graph = &(*host_graph)[USER_CONTAINER_TAG][user_entity_id_value];
            (*user_graph)[PROCESS_CONTAINER_TAG] = nlohmann::json::object();
            auto processes = get_entities_nts(EntityKind::PROCESS, user);

            //Add processes
            for (auto process : processes)
            {
                if (!process->active)
                {
                    continue;
                }

                std::string process_entity_id_value = std::to_string(process->id.value());
                get_entity_subgraph_nts(process->id.value(),
                        (*user_graph)[PROCESS_CONTAINER_TAG][process_entity_id_value]);

                Graph* process_graph = &(*user_graph)[PROCESS_CONTAINER_TAG][process_entity_id_value];
                (*process_graph)[PARTICIPANT_CONTAINER_TAG] = nlohmann::json::object();
                auto participants = get_entities_nts(EntityKind::PARTICIPANT, process);

                //Add prticipants
                for (auto participant : participants)
                {
                    if (!participant->active)
                    {
                        continue;
                    }

                    std::string participant_entity_id_value = std::to_string(participant->id.value());
                    get_entity_subgraph_nts(participant->id.value(),
                            (*process_graph)[PARTICIPANT_CONTAINER_TAG][participant_entity_id_value]);

                    Graph* participant_graph =
                            &(*process_graph)[PARTICIPANT_CONTAINER_TAG][participant_entity_id_value];
                    (*participant_graph)[ENDPOINT_CONTAINER_TAG] = nlohmann::json::object();
                    auto datareaders = get_entities_nts(EntityKind::DATAREADER, participant);

                    // Add endpoints
                    for (auto datareader : datareaders)
                    {
                        if (!datareader->active)
                        {
                            continue;
                        }

                        std::string datareader_entity_id_value = std::to_string(datareader->id.value());
                        get_entity_subgraph_nts(datareader->id.value(),
                                (*participant_graph)[ENDPOINT_CONTAINER_TAG][datareader_entity_id_value]);

                    }
                    auto datawriters = get_entities_nts(EntityKind::DATAWRITER, participant);
                    for (auto datawriter : datawriters)
                    {
                        if (!datawriter->active)
                        {
                            continue;
                        }

                        std::string datawriter_entity_id_value = std::to_string(datawriter->id.value());
                        get_entity_subgraph_nts(datawriter->id.value(),
                                (*participant_graph)[ENDPOINT_CONTAINER_TAG][datawriter_entity_id_value]);
                    }
                }
            }
        }
    }
    return true;
}

bool Database::update_graph_on_updated_entity(
        const EntityId& domain_id,
        const EntityId& entity_id)

{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);
    return update_graph_on_updated_entity_nts(domain_id, entity_id);
}

bool Database::update_graph_on_updated_entity_nts(
        const EntityId& domain_id,
        const EntityId& entity_id)
{
    std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);
    bool graph_updated = false;
    switch (entity->kind)
    {
        case EntityKind::HOST:
        {
            graph_updated = update_participant_in_graph_nts(domain_id, entity_id, EntityId(), EntityId(), EntityId());
            break;
        }
        case EntityKind::USER:
        {
            std::shared_ptr<const User> user = std::dynamic_pointer_cast<const User>(entity);
            if (user->host == nullptr)
            {
                return false;
            }
            std::shared_ptr<const Host> host = user->host;
            graph_updated = update_participant_in_graph_nts(domain_id, host->id, entity_id, EntityId(), EntityId());
            break;
        }
        case EntityKind::PROCESS:
        {
            std::shared_ptr<const Process> process =
                    std::dynamic_pointer_cast<const Process>(entity);
            if (process->user == nullptr)
            {
                return false;
            }
            std::shared_ptr<const User> user = process->user;
            if (user->host == nullptr)
            {
                return false;
            }
            std::shared_ptr<const Host> host = user->host;
            graph_updated = update_participant_in_graph_nts(domain_id, host->id, user->id, entity_id, EntityId());
            break;
        }
        case EntityKind::PARTICIPANT:
        {
            std::shared_ptr<const DomainParticipant> participant =
                    std::dynamic_pointer_cast<const DomainParticipant>(entity);
            if (participant->process == nullptr)
            {
                return false;
            }
            std::shared_ptr<const Process> process = participant->process;
            if (process->user == nullptr)
            {
                return false;
            }
            std::shared_ptr<const User> user = process->user;
            if (user->host == nullptr)
            {
                return false;
            }
            std::shared_ptr<const Host> host = user->host;
            graph_updated = update_participant_in_graph_nts(domain_id, host->id, user->id, process->id, entity_id);
            break;
        }
        case EntityKind::TOPIC:
        {
            graph_updated = update_endpoint_in_graph_nts(domain_id, EntityId(), entity_id, EntityId());
            break;
        }
        case EntityKind::DATAREADER:
        case EntityKind::DATAWRITER:
        {
            std::shared_ptr<const DDSEndpoint> endpoint =
                    std::dynamic_pointer_cast<const DDSEndpoint>(entity);
            if (endpoint->participant == nullptr)
            {
                return false;
            }
            std::shared_ptr<const DomainParticipant> participant = endpoint->participant;
            graph_updated = update_endpoint_in_graph_nts(domain_id, participant->id, EntityId(), entity_id);
            break;
        }
        default:
        {
            break;
        }
    }
    return graph_updated;
}

Graph Database::get_entity_subgraph_nts(
        const EntityId& entity_id,
        Graph& entity_graph)
{
    bool entity_graph_updated = false;

    std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);

    entity_graph[KIND_TAG] =  entity_kind_str[(int)entity->kind];
    entity_graph[DISCOVERY_SOURCE_TAG] =  discovery_source_str[(int)entity->discovery_source];

    if ( entity_graph[DISCOVERY_SOURCE_TAG] != entity->discovery_source)
    {
        entity_graph[DISCOVERY_SOURCE_TAG] =  discovery_source_str[(int)entity->discovery_source];
        entity_graph_updated = true;
    }

    if (entity_graph[ALIAS_TAG] != entity->alias)
    {
        entity_graph[ALIAS_TAG] = entity->alias;
        entity_graph_updated = true;
    }

    entity_graph[METATRAFFIC_TAG] =  entity->metatraffic;

    if (entity->kind != EntityKind::TOPIC && entity_graph[STATUS_TAG] != status_level_str[(int)entity->status])
    {
        entity_graph[STATUS_TAG] = status_level_str[(int)entity->status];
        entity_graph_updated = true;
    }

    switch (entity->kind)
    {
        case (EntityKind::PROCESS):
        {
            std::shared_ptr<const Process> process =
                    std::dynamic_pointer_cast<const Process>(entity);
            entity_graph[PID_TAG] =  process->pid;
            break;
        }
        case (EntityKind::PARTICIPANT):
        {
            std::shared_ptr<const DomainParticipant> participant =
                    std::dynamic_pointer_cast<const DomainParticipant>(entity);
            if (entity_graph[APP_ID_TAG] != app_id_str[(int)participant->app_id])
            {
                entity_graph[APP_ID_TAG] =  app_id_str[(int)participant->app_id];
                entity_graph_updated = true;
            }
            if (entity_graph[APP_METADATA_TAG] != participant->app_metadata)
            {
                entity_graph[APP_METADATA_TAG] =  participant->app_metadata;
                entity_graph_updated = true;
            }
            if (entity_graph[DDS_VENDOR_TAG] != dds_vendor_str[static_cast<int>(participant->dds_vendor)])
            {
                entity_graph[DDS_VENDOR_TAG] = dds_vendor_str[static_cast<int>(participant->dds_vendor)];
                entity_graph_updated = true;
            }
            break;
        }
        case (EntityKind::DATAWRITER):
        case (EntityKind::DATAREADER):
        {
            std::shared_ptr<const DDSEndpoint> endpoint =
                    std::dynamic_pointer_cast<const DDSEndpoint>(entity);
            entity_graph[TOPIC_ENTITY_TAG] =  std::to_string(endpoint->topic->id.value());
            if (entity_graph[APP_ID_TAG] != app_id_str[(int)endpoint->app_id])
            {
                entity_graph[APP_ID_TAG] =  app_id_str[(int)endpoint->app_id];
                entity_graph_updated = true;
            }
            if (entity_graph[APP_METADATA_TAG] != endpoint->app_metadata)
            {
                entity_graph[APP_METADATA_TAG] =  endpoint->app_metadata;
                entity_graph_updated = true;
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return entity_graph_updated;
}

template <>
bool Database::update_entity_status_nts(
        std::shared_ptr<DataReader>& entity)
{
    bool entity_error = false;
    bool entity_warning = false;

    // Check ExtendedIncompatibleQoS Status
    if (!entity->monitor_service_data.extended_incompatible_qos.empty() &&
            entity->monitor_service_data.extended_incompatible_qos.back().status == StatusLevel::ERROR_STATUS)
    {
        entity_error = true;
    }
    // Check DeadlineMissed Status
    if (!entity->monitor_service_data.deadline_missed.empty() &&
            entity->monitor_service_data.deadline_missed.back().status == StatusLevel::WARNING_STATUS)
    {
        entity_warning = true;
    }
    // Check SampleLost Status
    if (!entity->monitor_service_data.sample_lost.empty() &&
            entity->monitor_service_data.sample_lost.back().status == StatusLevel::WARNING_STATUS)
    {
        entity_warning = true;
    }

    // Set entity status
    return entity_status_logic_nts(entity_error, entity_warning, entity->status);
}

template <>
bool Database::update_entity_status_nts(
        std::shared_ptr<DataWriter>& entity)
{
    bool entity_error = false;
    bool entity_warning = false;

    // Check ExtendedIncompatibleQoS Status
    if (!entity->monitor_service_data.extended_incompatible_qos.empty() &&
            entity->monitor_service_data.extended_incompatible_qos.back().status == StatusLevel::ERROR_STATUS)
    {
        entity_error = true;
    }
    // Check LivelinessLost Status
    if (!entity->monitor_service_data.liveliness_lost.empty() &&
            entity->monitor_service_data.liveliness_lost.back().status == StatusLevel::WARNING_STATUS)
    {
        entity_warning = true;
    }
    // Check DeadlineMissed Status
    if (!entity->monitor_service_data.deadline_missed.empty() &&
            entity->monitor_service_data.deadline_missed.back().status == StatusLevel::WARNING_STATUS)
    {
        entity_warning = true;
    }

    // Set entity status
    return entity_status_logic_nts(entity_error, entity_warning, entity->status);
}

bool Database::update_entity_qos(
        const EntityId& entity,
        const Qos& received_qos)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);
    return update_entity_qos_nts(entity, received_qos);
}

bool Database::update_entity_qos_nts(
        const EntityId& entity,
        const Qos& received_qos)
{
    std::shared_ptr<const Entity> db_entity_const = get_entity_nts(entity);
    if (!db_entity_const->is_dds_entity())
    {
        throw BadParameter("Entity with id " + std::to_string(entity.value()) + " is not a DDS Entity");
    }

    std::shared_ptr<DDSEntity> db_entity =
            std::const_pointer_cast<DDSEntity>(std::static_pointer_cast<const DDSEntity>(db_entity_const));

    Qos old_qos = db_entity->qos;
    db_entity->qos.merge_patch(received_qos);

    return (db_entity->qos != old_qos);
}

bool Database::update_participant_discovery_info(
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
        DomainId original_domain)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);
    return update_participant_discovery_info_nts(participant_id, host, user, process, name, qos, guid, domain_id,
                   status, app_id, app_metadata, discovery_source, original_domain);
}

bool Database::update_participant_discovery_info_nts(
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
        DomainId original_domain)
{
    std::shared_ptr<Entity> db_entity = get_mutable_entity_nts(participant_id);
    if (db_entity->kind != EntityKind::PARTICIPANT)
    {
        throw BadParameter("Entity with id " + std::to_string(participant_id.value()) + " is not a Participant");
    }

    // Update of the participant inner information
    std::shared_ptr<DomainParticipant> db_participant = std::static_pointer_cast<DomainParticipant>(db_entity);
    db_participant->name = name;
    db_participant->qos = qos;
    db_participant->guid = guid;
    db_participant->status = status;
    db_participant->app_id = app_id;
    db_participant->app_metadata = app_metadata;
    db_participant->discovery_source = discovery_source;
    db_participant->original_domain = original_domain;
    db_participant->alias = db_participant->name;

    // Update of other entities that are linked to the participant
    std::map<std::string, EntityId> physical_entities_ids;
    physical_entities_ids[HOST_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[USER_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId::invalid();
    bool graph_updated = false;
    try
    {
        // Get process entity
        std::string process_name;
        std::string process_pid;
        size_t separator_pos = process.find_last_of(':');
        if (separator_pos == std::string::npos)
        {
            process_name = process;
            process_pid = process;
            EPROSIMA_LOG_INFO(BACKEND_DATABASE,
                    "Process name " + process_name + " does not follow the [command]:[PID] pattern");
        }
        else
        {
            process_name = process.substr(0, separator_pos);
            process_pid = process.substr(separator_pos + 1);
        }


        db_participant->process->name = process_name;
        db_participant->process->pid = process_pid;
        db_participant->process->user->name = user;
        db_participant->process->user->host->name = host;
        physical_entities_ids[PROCESS_ENTITY_TAG] = db_participant->process->id;
        physical_entities_ids[USER_ENTITY_TAG] = db_participant->process->user->id;
        physical_entities_ids[HOST_ENTITY_TAG] = db_participant->process->user->host->id;
    }
    catch (const std::exception& e)
    {
        EPROSIMA_LOG_ERROR(BACKEND_DATABASE_QUEUE, e.what());
    }

    graph_updated = update_participant_in_graph_nts(
        domain_id, physical_entities_ids[HOST_ENTITY_TAG], physical_entities_ids[USER_ENTITY_TAG],
        physical_entities_ids[PROCESS_ENTITY_TAG], participant_id);

    if (graph_updated)
    {
        details::StatisticsBackendData::get_instance()->on_domain_view_graph_update(domain_id);
    }

    return graph_updated;
}

bool Database::entity_status_logic(
        const bool& entity_error,
        const bool& entity_warning,
        StatusLevel& entity_status)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);
    return entity_status_logic_nts(entity_error, entity_warning, entity_status);
}

bool Database::entity_status_logic_nts(
        const bool& entity_error,
        const bool& entity_warning,
        StatusLevel& entity_status)
{
    // Set entity status
    if (entity_error)
    {
        if (entity_status != StatusLevel::ERROR_STATUS)
        {
            entity_status = StatusLevel::ERROR_STATUS;
            return true;
        }
        return false;
    }
    else if (entity_warning)
    {
        if (entity_status != StatusLevel::WARNING_STATUS)
        {
            entity_status = StatusLevel::WARNING_STATUS;
            return true;
        }
        return false;
    }
    else
    {
        if (entity_status != StatusLevel::OK_STATUS)
        {
            entity_status = StatusLevel::OK_STATUS;
            return true;
        }
        return false;
    }
}

void Database::set_alias(
        const EntityId& entity_id,
        const std::string& alias)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);
    set_alias_nts(entity_id, alias);
}

void Database::set_alias_nts(
        const EntityId& entity_id,
        const std::string& alias)
{
    std::shared_ptr<const Entity> const_entity = get_entity_nts(entity_id);
    std::shared_ptr<Entity> entity = std::const_pointer_cast<Entity>(const_entity);
    if (entity->alias != alias)
    {
        entity->alias = alias;
        auto domains = get_entities_nts(EntityKind::DOMAIN, entity_id);
        if (!domains.empty())
        {
            if (update_graph_on_updated_entity_nts(domains[0]->id, entity_id))
            {
                // TODO (eProsima) Workaround to avoid deadlock if callback implementation requires taking the database
                // mutex (e.g. by calling get_info). A refactor for not calling on_domain_view_graph_update from within
                // this function would be required.
                execute_without_lock([&]()
                        {
                            details::StatisticsBackendData::get_instance()->on_domain_view_graph_update(domains[0]->id);
                        });
            }
        }
    }
}

const std::vector<std::shared_ptr<const Entity>> Database::get_entities(
        EntityKind entity_kind,
        const EntityId& entity_id) const
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return get_entities_nts(entity_kind, entity_id);
}

const std::vector<std::shared_ptr<const Entity>> Database::get_entities_nts(
        EntityKind entity_kind,
        const EntityId& entity_id) const
{
    std::shared_ptr<const Entity> origin;

    // If entity_id is all, return all the entities of type entity_kind
    if (entity_id != EntityId::all())
    {
        // This call will throw BadParameter if there is no such entity.
        // We let this exception through, as it meets expectations
        origin = get_entity_nts(entity_id);
        assert(origin->kind != EntityKind::INVALID);
    }

    auto entities = get_entities_nts(entity_kind, origin);

    // Remove duplicates by sorting and unique-ing
    std::sort(entities.begin(), entities.end(), [](
                const std::shared_ptr<const Entity>& first,
                const std::shared_ptr<const Entity>& second)
            {
                return first.get() < second.get();
            });
    auto last = std::unique(entities.begin(), entities.end(), [](
                        const std::shared_ptr<const Entity>& first,
                        const std::shared_ptr<const Entity>& second)
                    {
                        return first.get() == second.get();
                    });
    entities.erase(last, entities.end());

    return entities;
}

std::vector<EntityId> Database::get_entity_ids(
        EntityKind entity_kind,
        const EntityId& entity_id) const
{
    std::vector<EntityId> entitiesIds;

    std::shared_lock<std::shared_timed_mutex> lock(mutex_);

    for (const auto& entity : get_entities_nts(entity_kind, entity_id))
    {
        entitiesIds.push_back(entity->id);
    }

    return entitiesIds;
}

std::vector<AlertId> Database::get_alerts_ids() const
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    std::vector<AlertId> alertsIds;

    for (const auto& alert_it : alerts_)
    {
        alertsIds.push_back(alert_it.first);
    }

    return alertsIds;
}

// Auxiliar function to convert a map to a vector
template<typename T>
void map_to_vector(
        const std::map<EntityId, std::shared_ptr<T>>& map,
        std::vector<std::shared_ptr<const Entity>>& vec)
{
    for (auto elem : map)
    {
        vec.push_back(elem.second);
    }
}

// Auxiliar function to convert a map of maps to a vector
template <typename T>
void map_of_maps_to_vector(
        const std::map<EntityId, std::map<EntityId, std::shared_ptr<T>>>& map,
        std::vector<std::shared_ptr<const Entity>>& vec)
{
    for (const auto& elem : map)
    {
        map_to_vector(elem.second, vec);
    }
}

const std::vector<std::shared_ptr<const Entity>> Database::get_entities_nts(
        EntityKind entity_kind,
        const std::shared_ptr<const Entity>& origin) const
{
    std::vector<std::shared_ptr<const Entity>> entities;

    // If entity_id is all, origin is nullptr. Return all the entities of type entity_kind
    if (origin == nullptr)
    {
        switch (entity_kind)
        {
            case EntityKind::HOST:
                map_to_vector(hosts_, entities);
                break;
            case EntityKind::USER:
                map_to_vector(users_, entities);
                break;
            case EntityKind::PROCESS:
                map_to_vector(processes_, entities);
                break;
            case EntityKind::DOMAIN:
                map_to_vector(domains_, entities);
                break;
            case EntityKind::TOPIC:
                map_of_maps_to_vector(topics_, entities);
                break;
            case EntityKind::PARTICIPANT:
                map_of_maps_to_vector(participants_, entities);
                break;
            case EntityKind::DATAWRITER:
                map_of_maps_to_vector(datawriters_, entities);
                break;
            case EntityKind::DATAREADER:
                map_of_maps_to_vector(datareaders_, entities);
                break;
            case EntityKind::LOCATOR:
                map_to_vector(locators_, entities);
                break;
            default:
                throw BadParameter("Invalid EntityKind");
        }
    }
    else
    {
        switch (origin->kind)
        {
            case EntityKind::HOST:
            {
                const std::shared_ptr<const Host>& host = std::dynamic_pointer_cast<const Host>(origin);
                switch (entity_kind)
                {
                    case EntityKind::HOST:
                        entities.push_back(host);
                        break;
                    case EntityKind::USER:
                        for (const auto& user : host->users)
                        {
                            if (!user.second.expired())
                            {
                                entities.push_back(user.second);
                            }
                        }
                        break;
                    case EntityKind::PROCESS:
                    case EntityKind::DOMAIN:
                    case EntityKind::PARTICIPANT:
                    case EntityKind::TOPIC:
                    case EntityKind::DATAREADER:
                    case EntityKind::DATAWRITER:
                    case EntityKind::LOCATOR:
                        for (const auto& user : host->users)
                        {
                            auto sub_entities = get_entities_nts(entity_kind, user.second);
                            entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                        }
                        break;
                    default:
                        throw BadParameter("Invalid EntityKind");
                }
                break;
            }
            case EntityKind::USER:
            {
                const std::shared_ptr<const User>& user = std::dynamic_pointer_cast<const User>(origin);
                switch (entity_kind)
                {
                    case EntityKind::HOST:
                        entities.push_back(user->host);
                        break;
                    case EntityKind::USER:
                        entities.push_back(user);
                        break;
                    case EntityKind::PROCESS:
                        for (const auto& process : user->processes)
                        {
                            if (!process.second.expired())
                            {
                                entities.push_back(process.second);
                            }
                        }
                        break;
                    case EntityKind::DOMAIN:
                    case EntityKind::PARTICIPANT:
                    case EntityKind::TOPIC:
                    case EntityKind::DATAREADER:
                    case EntityKind::DATAWRITER:
                    case EntityKind::LOCATOR:
                        for (const auto& process : user->processes)
                        {
                            auto sub_entities = get_entities_nts(entity_kind, process.second);
                            entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                        }
                        break;
                    default:
                        throw BadParameter("Invalid EntityKind");
                }
                break;
            }
            case EntityKind::PROCESS:
            {
                const std::shared_ptr<const Process>& process = std::dynamic_pointer_cast<const Process>(origin);
                switch (entity_kind)
                {
                    case EntityKind::HOST:
                    {
                        auto sub_entities = get_entities_nts(entity_kind, process->user);
                        entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                    }
                    break;
                    case EntityKind::USER:
                        if (!process->user.expired())
                        {
                            entities.push_back(process->user);
                        }
                        break;
                    case EntityKind::PROCESS:
                        entities.push_back(process);
                        break;
                    case EntityKind::PARTICIPANT:
                        for (const auto& participant : process->participants)
                        {
                            if (!participant.second.expired())
                            {
                                entities.push_back(participant.second);
                            }
                        }
                        break;
                    case EntityKind::DOMAIN:
                    case EntityKind::TOPIC:
                    case EntityKind::DATAREADER:
                    case EntityKind::DATAWRITER:
                    case EntityKind::LOCATOR:
                        for (const auto& participant : process->participants)
                        {
                            auto sub_entities = get_entities_nts(entity_kind, participant.second);
                            entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                        }
                        break;
                    default:
                        throw BadParameter("Invalid EntityKind");
                }
                break;
            }
            case EntityKind::DOMAIN:
            {
                const std::shared_ptr<const Domain>& domain = std::dynamic_pointer_cast<const Domain>(origin);
                switch (entity_kind)
                {
                    case EntityKind::DOMAIN:
                        entities.push_back(domain);
                        break;
                    case EntityKind::PARTICIPANT:
                        for (const auto& participant : domain->participants)
                        {
                            if (!participant.second.expired())
                            {
                                entities.push_back(participant.second);
                            }
                        }
                        break;
                    case EntityKind::TOPIC:
                        for (const auto& topic : domain->topics)
                        {
                            if (!topic.second.expired())
                            {
                                entities.push_back(topic.second);
                            }
                        }
                        break;
                    case EntityKind::HOST:
                    case EntityKind::USER:
                    case EntityKind::PROCESS:
                    case EntityKind::DATAREADER:
                    case EntityKind::DATAWRITER:
                    case EntityKind::LOCATOR:
                        for (const auto& participant : domain->participants)
                        {
                            auto sub_entities = get_entities_nts(entity_kind, participant.second);
                            entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                        }
                        break;
                    default:
                        throw BadParameter("Invalid EntityKind");
                }
                break;
            }
            case EntityKind::PARTICIPANT:
            {
                const std::shared_ptr<const DomainParticipant>& participant =
                        std::dynamic_pointer_cast<const DomainParticipant>(origin);
                switch (entity_kind)
                {
                    case EntityKind::HOST:
                    case EntityKind::USER:
                    {
                        // May not have the relation process - participant yet
                        if (participant->process)
                        {
                            auto sub_entities = get_entities_nts(entity_kind, participant->process);
                            entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                        }
                    }
                    break;
                    case EntityKind::PROCESS:
                        // May not have the relation process - participant yet
                        if (participant->process)
                        {
                            entities.push_back(participant->process);
                        }
                        break;
                    case EntityKind::DOMAIN:
                        if (!participant->domain.expired())
                        {
                            entities.push_back(participant->domain);
                        }
                        break;
                    case EntityKind::PARTICIPANT:
                        entities.push_back(participant);
                        break;
                    case EntityKind::DATAWRITER:
                        for (const auto& writer : participant->data_writers)
                        {
                            if (!writer.second.expired())
                            {
                                entities.push_back(writer.second);
                            }
                        }
                        break;
                    case EntityKind::DATAREADER:
                        for (const auto& reader : participant->data_readers)
                        {
                            if (!reader.second.expired())
                            {
                                entities.push_back(reader.second);
                            }
                        }
                        break;
                    case EntityKind::TOPIC:
                    case EntityKind::LOCATOR:
                        for (const auto& writer : participant->data_writers)
                        {
                            auto sub_entities = get_entities_nts(entity_kind, writer.second);
                            entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                        }
                        for (const auto& reader : participant->data_readers)
                        {
                            auto sub_entities = get_entities_nts(entity_kind, reader.second);
                            entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                        }
                        break;
                    default:
                        throw BadParameter("Invalid EntityKind");
                }
                break;
            }
            case EntityKind::TOPIC:
            {
                const std::shared_ptr<const Topic>& topic = std::dynamic_pointer_cast<const Topic>(origin);
                switch (entity_kind)
                {
                    case EntityKind::DOMAIN:
                        if (!topic->domain.expired())
                        {
                            entities.push_back(topic->domain);
                        }
                        break;
                    case EntityKind::TOPIC:
                        entities.push_back(topic);
                        break;
                    case EntityKind::DATAWRITER:
                        for (const auto& writer : topic->data_writers)
                        {
                            if (!writer.second.expired())
                            {
                                entities.push_back(writer.second);
                            }
                        }
                        break;
                    case EntityKind::DATAREADER:
                        for (const auto& reader : topic->data_readers)
                        {
                            if (!reader.second.expired())
                            {
                                entities.push_back(reader.second);
                            }
                        }
                        break;
                    case EntityKind::HOST:
                    case EntityKind::USER:
                    case EntityKind::PROCESS:
                    case EntityKind::PARTICIPANT:
                    case EntityKind::LOCATOR:
                        for (const auto& writer : topic->data_writers)
                        {
                            auto sub_entities = get_entities_nts(entity_kind, writer.second);
                            entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                        }
                        for (const auto& reader : topic->data_readers)
                        {
                            auto sub_entities = get_entities_nts(entity_kind, reader.second);
                            entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                        }
                        break;
                    default:
                        throw BadParameter("Invalid EntityKind");
                }
                break;
            }
            case EntityKind::DATAWRITER:
            {
                const std::shared_ptr<const DataWriter>& writer = std::dynamic_pointer_cast<const DataWriter>(origin);
                switch (entity_kind)
                {
                    case EntityKind::TOPIC:
                        if (!writer->topic.expired())
                        {
                            entities.push_back(writer->topic);
                        }
                        break;
                    case EntityKind::PARTICIPANT:
                        if (!writer->participant.expired())
                        {
                            entities.push_back(writer->participant);
                        }
                        break;
                    case EntityKind::DATAWRITER:
                        entities.push_back(writer);
                        break;
                    case EntityKind::LOCATOR:
                        for (const auto& locator : writer->locators)
                        {
                            if (!locator.second.expired())
                            {
                                entities.push_back(locator.second);
                            }
                        }
                        break;
                    case EntityKind::DATAREADER:
                    {
                        auto sub_entities = get_entities_nts(entity_kind, writer->topic);
                        entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                    }
                    break;
                    case EntityKind::HOST:
                    case EntityKind::USER:
                    case EntityKind::PROCESS:
                    case EntityKind::DOMAIN:
                    {
                        auto sub_entities = get_entities_nts(entity_kind, writer->participant);
                        entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                    }
                    break;
                    default:
                        throw BadParameter("Invalid EntityKind");
                }
                break;
            }
            case EntityKind::DATAREADER:
            {
                const std::shared_ptr<const DataReader>& reader = std::dynamic_pointer_cast<const DataReader>(origin);
                switch (entity_kind)
                {
                    case EntityKind::TOPIC:
                        if (!reader->topic.expired())
                        {
                            entities.push_back(reader->topic);
                        }
                        break;
                    case EntityKind::PARTICIPANT:
                        if (!reader->participant.expired())
                        {
                            entities.push_back(reader->participant);
                        }
                        break;
                    case EntityKind::DATAREADER:
                        entities.push_back(reader);
                        break;
                    case EntityKind::LOCATOR:
                        for (const auto& locator : reader->locators)
                        {
                            if (!locator.second.expired())
                            {
                                entities.push_back(locator.second);
                            }
                        }
                        break;
                    case EntityKind::DATAWRITER:
                    {
                        auto sub_entities = get_entities_nts(entity_kind, reader->topic);
                        entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                    }
                    break;
                    case EntityKind::HOST:
                    case EntityKind::USER:
                    case EntityKind::PROCESS:
                    case EntityKind::DOMAIN:
                    {
                        auto sub_entities = get_entities_nts(entity_kind, reader->participant);
                        entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                    }
                    break;
                    default:
                        throw BadParameter("Invalid EntityKind");
                }
                break;
            }
            case EntityKind::LOCATOR:
            {
                const std::shared_ptr<const Locator>& locator = std::dynamic_pointer_cast<const Locator>(origin);
                switch (entity_kind)
                {
                    case EntityKind::DATAREADER:
                        for (const auto& reader : locator->data_readers)
                        {
                            if (!reader.second.expired())
                            {
                                entities.push_back(reader.second);
                            }
                        }
                        break;
                    case EntityKind::DATAWRITER:
                        for (const auto& writer : locator->data_writers)
                        {
                            if (!writer.second.expired())
                            {
                                entities.push_back(writer.second);
                            }
                        }
                        break;
                    case EntityKind::LOCATOR:
                        entities.push_back(locator);
                        break;
                    case EntityKind::HOST:
                    case EntityKind::USER:
                    case EntityKind::PROCESS:
                    case EntityKind::PARTICIPANT:
                    case EntityKind::TOPIC:
                    case EntityKind::DOMAIN:
                        for (const auto& writer : locator->data_writers)
                        {
                            auto sub_entities = get_entities_nts(entity_kind, writer.second);
                            entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                        }
                        for (const auto& reader : locator->data_readers)
                        {
                            auto sub_entities = get_entities_nts(entity_kind, reader.second);
                            entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                        }
                        break;
                    default:
                        throw BadParameter("Invalid EntityKind");
                }
                break;
            }
            default:
                throw BadParameter("Invalid EntityKind");
        }
    }

    return entities;
}

EntityId Database::generate_entity_id() noexcept
{
    return EntityId(next_id_++);
}

template<>
std::map<EntityId, std::map<EntityId, std::shared_ptr<DataReader>>>& Database::dds_endpoints_nts<DataReader>()
{
    return datareaders_;
}

template<>
std::map<EntityId, std::map<EntityId, std::shared_ptr<DataReader>>>& Database::dds_endpoints<DataReader>()
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return dds_endpoints_nts<DataReader>();
}

template<>
std::map<EntityId, std::map<EntityId, std::shared_ptr<DataWriter>>>& Database::dds_endpoints_nts<DataWriter>()
{
    return datawriters_;
}

template<>
std::map<EntityId, std::map<EntityId, std::shared_ptr<DataWriter>>>& Database::dds_endpoints<DataWriter>()
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return dds_endpoints_nts<DataWriter>();
}

template<>
void Database::insert_ddsendpoint_to_locator_nts(
        std::shared_ptr<DataWriter>& endpoint,
        std::shared_ptr<Locator>& locator)
{
    locator->data_writers[endpoint->id] = endpoint;
}

template<>
void Database::insert_ddsendpoint_to_locator_nts(
        std::shared_ptr<DataReader>& endpoint,
        std::shared_ptr<Locator>& locator)
{
    locator->data_readers[endpoint->id] = endpoint;
}

DatabaseDump Database::dump_database(
        bool clear)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);

    DatabaseDump dump = DatabaseDump::object();

    // Add version
    dump[VERSION_TAG] = ACTUAL_DUMP_VERSION;

    // Hosts
    {
        DatabaseDump container = DatabaseDump::object();

        // For each entity of this kind in the database
        for (const auto& it : hosts_)
        {
            container[id_to_string(it.first.value())] = dump_entity_(it.second);
        }

        dump[HOST_CONTAINER_TAG] = container;
    }

    // Users
    {
        DatabaseDump container = DatabaseDump::object();

        // For each entity of this kind in the database
        for (const auto& it : users_)
        {
            container[id_to_string(it.first.value())] = dump_entity_(it.second);
        }

        dump[USER_CONTAINER_TAG] = container;
    }

    // Processes
    {
        DatabaseDump container = DatabaseDump::object();

        // For each entity of this kind in the database
        for (const auto& it : processes_)
        {
            container[id_to_string(it.first.value())] = dump_entity_(it.second);
        }

        dump[PROCESS_CONTAINER_TAG] = container;
    }

    // Domain
    {
        DatabaseDump container = DatabaseDump::object();

        // For each entity of this kind in the database
        for (const auto& it : domains_)
        {
            container[id_to_string(it.first.value())] = dump_entity_(it.second);
        }

        dump[DOMAIN_CONTAINER_TAG] = container;
    }

    // Topic
    {
        DatabaseDump container = DatabaseDump::object();
        // For each domain
        for (const auto& super_it : topics_)
        {
            // For each entity of this kind in the domain
            for (const auto& it : super_it.second)
            {
                container[id_to_string(it.first.value())] = dump_entity_(it.second);
            }
        }
        dump[TOPIC_CONTAINER_TAG] = container;
    }

    // Participant
    {
        DatabaseDump container = DatabaseDump::object();
        // For each domain
        for (const auto& super_it : participants_)
        {
            // For each entity of this kind in the domain
            for (const auto& it : super_it.second)
            {
                container[id_to_string(it.first.value())] = dump_entity_(it.second);
            }
        }
        dump[PARTICIPANT_CONTAINER_TAG] = container;
    }

    // DataWriter
    {
        DatabaseDump container = DatabaseDump::object();
        // For each domain
        for (const auto& super_it : datawriters_)
        {
            // For each entity of this kind in the domain
            for (const auto& it : super_it.second)
            {
                container[id_to_string(it.first.value())] = dump_entity_(it.second);
            }
        }
        dump[DATAWRITER_CONTAINER_TAG] = container;
    }

    // DataReader
    {
        DatabaseDump container = DatabaseDump::object();
        // For each domain
        for (const auto& super_it : datareaders_)
        {
            // For each entity of this kind in the domain
            for (const auto& it : super_it.second)
            {
                container[id_to_string(it.first.value())] = dump_entity_(it.second);
            }
        }
        dump[DATAREADER_CONTAINER_TAG] = container;
    }

    // Locator
    {
        DatabaseDump container = DatabaseDump::object();

        // For each entity of this kind in the database
        for (const auto& it : locators_)
        {
            container[id_to_string(it.first.value())] = dump_entity_(it.second);
        }

        dump[LOCATOR_CONTAINER_TAG] = container;
    }

    if (clear)
    {
        // Clear all
        clear_statistics_data_nts_(the_end_of_time());
    }

    return dump;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<Host>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_TAG] = entity->name;
    entity_info[ALIAS_TAG] = entity->alias;
    entity_info[STATUS_TAG] = entity->status;

    // metatraffic and active attributes are stored but ignored when loading
    entity_info[METATRAFFIC_TAG] = entity->metatraffic;
    entity_info[ALIVE_TAG] = entity->active;
    entity_info[DISCOVERY_SOURCE_TAG] = discovery_source_str[(int)entity->discovery_source];


    // Populate subentity array
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->users)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[USER_CONTAINER_TAG] = subentities;
    }

    return entity_info;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<User>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_TAG] = entity->name;
    entity_info[ALIAS_TAG] = entity->alias;
    entity_info[STATUS_TAG] = entity->status;

    entity_info[HOST_ENTITY_TAG] = id_to_string(entity->host->id);

    // metatraffic and active attributes are stored but ignored when loading
    entity_info[METATRAFFIC_TAG] = entity->metatraffic;
    entity_info[ALIVE_TAG] = entity->active;
    entity_info[DISCOVERY_SOURCE_TAG] = discovery_source_str[(int)entity->discovery_source];

    // Populate subentity array
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->processes)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[PROCESS_CONTAINER_TAG] = subentities;
    }

    return entity_info;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<Process>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_TAG] = entity->name;
    entity_info[ALIAS_TAG] = entity->alias;
    entity_info[PID_TAG] = entity->pid;
    entity_info[STATUS_TAG] = entity->status;

    entity_info[USER_ENTITY_TAG] = id_to_string(entity->user->id);

    // metatraffic and active attributes are stored but ignored when loading
    entity_info[METATRAFFIC_TAG] = entity->metatraffic;
    entity_info[ALIVE_TAG] = entity->active;
    entity_info[DISCOVERY_SOURCE_TAG] = discovery_source_str[(int)entity->discovery_source];

    // Populate subentity array
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->participants)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[PARTICIPANT_CONTAINER_TAG] = subentities;
    }

    return entity_info;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<Domain>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_TAG] = entity->name;
    entity_info[DOMAIN_ID_TAG] = entity->domain_id;
    entity_info[ALIAS_TAG] = entity->alias;
    entity_info[STATUS_TAG] = entity->status;

    // metatraffic and active attributes are stored but ignored when loading
    entity_info[METATRAFFIC_TAG] = entity->metatraffic;
    entity_info[ALIVE_TAG] = entity->active;
    entity_info[DISCOVERY_SOURCE_TAG] = discovery_source_str[(int)entity->discovery_source];

    // Populate subentity array for Topics
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->topics)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[TOPIC_CONTAINER_TAG] = subentities;
    }

    // Populate subentity array for Participants
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->participants)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[PARTICIPANT_CONTAINER_TAG] = subentities;
    }

    return entity_info;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<Topic>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_TAG] = entity->name;
    entity_info[ALIAS_TAG] = entity->alias;
    entity_info[DATA_TYPE_TAG] = entity->data_type;
    entity_info[STATUS_TAG] = entity->status;
    entity_info[DOMAIN_ENTITY_TAG] = id_to_string(entity->domain->id);

    // metatraffic and active attributes are stored but ignored when loading
    entity_info[METATRAFFIC_TAG] = entity->metatraffic;
    entity_info[ALIVE_TAG] = entity->active;
    entity_info[DISCOVERY_SOURCE_TAG] = discovery_source_str[(int)entity->discovery_source];

    // Populate subentity array for DataWriters
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->data_writers)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[DATAWRITER_CONTAINER_TAG] = subentities;
    }

    // Populate subentity array for DataReaders
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->data_readers)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[DATAREADER_CONTAINER_TAG] = subentities;
    }

    return entity_info;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<DomainParticipant>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_TAG] = entity->name;
    entity_info[ALIAS_TAG] = entity->alias;
    entity_info[GUID_TAG] = entity->guid;
    entity_info[QOS_TAG] = entity->qos;
    entity_info[STATUS_TAG] = entity->status;

    entity_info[DOMAIN_ENTITY_TAG] = id_to_string(entity->domain->id);

    // metatraffic and active attributes are stored but ignored when loading
    entity_info[METATRAFFIC_TAG] = entity->metatraffic;
    entity_info[ALIVE_TAG] = entity->active;
    entity_info[DISCOVERY_SOURCE_TAG] = discovery_source_str[(int)entity->discovery_source];

    if (entity->process)
    {
        entity_info[PROCESS_ENTITY_TAG] = id_to_string(entity->process->id);
    }
    else
    {
        entity_info[PROCESS_ENTITY_TAG] = id_to_string(EntityId::invalid());
    }

    // Populate subentity array for DataWriters
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->data_writers)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[DATAWRITER_CONTAINER_TAG] = subentities;
    }
    // Populate subentity array for DataReaders
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->data_readers)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[DATAREADER_CONTAINER_TAG] = subentities;
    }

    // Store data from the entity
    {
        DatabaseDump data = DatabaseDump::object();

        // discovered_entity
        data[DATA_KIND_DISCOVERY_TIME_TAG] = dump_data_(entity->data.discovered_entity);

        // pdp_packets
        data[DATA_KIND_PDP_PACKETS_TAG] = dump_data_(entity->data.pdp_packets);

        // edp_packets
        data[DATA_KIND_EDP_PACKETS_TAG] = dump_data_(entity->data.edp_packets);

        // rtps_packets_sent
        data[DATA_KIND_RTPS_PACKETS_SENT_TAG] = dump_data_(entity->data.rtps_packets_sent);

        // rtps_bytes_sent
        data[DATA_KIND_RTPS_BYTES_SENT_TAG] = dump_data_(entity->data.rtps_bytes_sent);

        // rtps_packets_lost
        data[DATA_KIND_RTPS_PACKETS_LOST_TAG] = dump_data_(entity->data.rtps_packets_lost);

        // rtps_bytes_lost
        data[DATA_KIND_RTPS_BYTES_LOST_TAG] = dump_data_(entity->data.rtps_bytes_lost);

        // network_latency_per_locator
        data[DATA_KIND_NETWORK_LATENCY_TAG] = dump_data_(entity->data.network_latency_per_locator);

        // pdp_packets last reported
        data[DATA_KIND_PDP_PACKETS_LAST_REPORTED_TAG] = dump_data_(entity->data.last_reported_pdp_packets);

        // edp_packets last reported
        data[DATA_KIND_EDP_PACKETS_LAST_REPORTED_TAG] = dump_data_(entity->data.last_reported_edp_packets);

        // rtps_packets_sent last reported
        data[DATA_KIND_RTPS_PACKETS_SENT_LAST_REPORTED_TAG] =
                dump_data_(entity->data.last_reported_rtps_packets_sent_count);

        // rtps_bytes_sent last reported
        data[DATA_KIND_RTPS_BYTES_SENT_LAST_REPORTED_TAG] =
                dump_data_(entity->data.last_reported_rtps_bytes_sent_count);

        // rtps_packets_lost last reported
        data[DATA_KIND_RTPS_PACKETS_LOST_LAST_REPORTED_TAG] =
                dump_data_(entity->data.last_reported_rtps_packets_lost_count);

        // rtps_bytes_lost last reported
        data[DATA_KIND_RTPS_BYTES_LOST_LAST_REPORTED_TAG] =
                dump_data_(entity->data.last_reported_rtps_bytes_lost_count);

        entity_info[DATA_CONTAINER_TAG] = data;
    }

    return entity_info;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<DataWriter>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_TAG] = entity->name;
    entity_info[ALIAS_TAG] = entity->alias;
    entity_info[GUID_TAG] = entity->guid;
    entity_info[QOS_TAG] = entity->qos;
    entity_info[STATUS_TAG] = entity->status;

    entity_info[PARTICIPANT_ENTITY_TAG] = id_to_string(entity->participant->id);
    entity_info[TOPIC_ENTITY_TAG] = id_to_string(entity->topic->id);
    entity_info[VIRTUAL_METATRAFFIC_TAG] = entity->is_virtual_metatraffic;

    // metatraffic and active attributes are stored but ignored when loading
    entity_info[METATRAFFIC_TAG] = entity->metatraffic;
    entity_info[ALIVE_TAG] = entity->active;
    entity_info[DISCOVERY_SOURCE_TAG] = discovery_source_str[(int)entity->discovery_source];

    // Populate subentity array for Locators
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->locators)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[LOCATOR_CONTAINER_TAG] = subentities;
    }

    // Store data from the entity
    {
        DatabaseDump data = DatabaseDump::object();

        // publication_throughput
        data[DATA_KIND_PUBLICATION_THROUGHPUT_TAG] = dump_data_(entity->data.publication_throughput);

        // resent_datas
        data[DATA_KIND_RESENT_DATA_TAG] = dump_data_(entity->data.resent_datas);

        // heartbeat_count
        data[DATA_KIND_HEARTBEAT_COUNT_TAG] = dump_data_(entity->data.heartbeat_count);

        // gap_count
        data[DATA_KIND_GAP_COUNT_TAG] = dump_data_(entity->data.gap_count);

        // data_count
        data[DATA_KIND_DATA_COUNT_TAG] = dump_data_(entity->data.data_count);

        // sample_datas
        data[DATA_KIND_SAMPLE_DATAS_TAG] = dump_data_(entity->data.sample_datas);

        // history2history_latency
        data[DATA_KIND_FASTDDS_LATENCY_TAG] = dump_data_(entity->data.history2history_latency);

        // resent_data last reported
        data[DATA_KIND_RESENT_DATA_LAST_REPORTED_TAG] = dump_data_(entity->data.last_reported_resent_datas);

        // heartbeat_count last reported
        data[DATA_KIND_HEARTBEAT_COUNT_LAST_REPORTED_TAG] = dump_data_(entity->data.last_reported_heartbeat_count);

        // gap_count last reported
        data[DATA_KIND_GAP_COUNT_LAST_REPORTED_TAG] = dump_data_(entity->data.last_reported_gap_count);

        // data_count last reported
        data[DATA_KIND_DATA_COUNT_LAST_REPORTED_TAG] = dump_data_(entity->data.last_reported_data_count);

        entity_info[DATA_CONTAINER_TAG] = data;
    }

    return entity_info;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<DataReader>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_TAG] = entity->name;
    entity_info[ALIAS_TAG] = entity->alias;
    entity_info[GUID_TAG] = entity->guid;
    entity_info[QOS_TAG] = entity->qos;
    entity_info[STATUS_TAG] = entity->status;

    entity_info[PARTICIPANT_ENTITY_TAG] = id_to_string(entity->participant->id);
    entity_info[TOPIC_ENTITY_TAG] = id_to_string(entity->topic->id);
    entity_info[VIRTUAL_METATRAFFIC_TAG] = entity->is_virtual_metatraffic;

    // metatraffic and active attributes are stored but ignored when loading
    entity_info[METATRAFFIC_TAG] = entity->metatraffic;
    entity_info[ALIVE_TAG] = entity->active;
    entity_info[DISCOVERY_SOURCE_TAG] = discovery_source_str[(int)entity->discovery_source];

    // Populate subentity array for Locators
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->locators)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[LOCATOR_CONTAINER_TAG] = subentities;
    }

    // Store data from the entity
    {
        DatabaseDump data = DatabaseDump::object();

        // subscription_throughput
        data[DATA_KIND_SUBSCRIPTION_THROUGHPUT_TAG] = dump_data_(entity->data.subscription_throughput);

        // acknack_count
        data[DATA_KIND_ACKNACK_COUNT_TAG] = dump_data_(entity->data.acknack_count);

        // nackfrag_count
        data[DATA_KIND_NACKFRAG_COUNT_TAG] = dump_data_(entity->data.nackfrag_count);

        // acknack_count last reported
        data[DATA_KIND_ACKNACK_COUNT_LAST_REPORTED_TAG] = dump_data_(entity->data.last_reported_acknack_count);

        // nackfrag_count last reported
        data[DATA_KIND_NACKFRAG_COUNT_LAST_REPORTED_TAG] = dump_data_(entity->data.last_reported_nackfrag_count);

        entity_info[DATA_CONTAINER_TAG] = data;
    }

    return entity_info;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<Locator>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_TAG] = entity->name;
    entity_info[ALIAS_TAG] = entity->alias;
    entity_info[STATUS_TAG] = entity->status;

    // metatraffic and active attributes are stored but ignored when loading
    entity_info[METATRAFFIC_TAG] = entity->metatraffic;
    entity_info[ALIVE_TAG] = entity->active;
    entity_info[DISCOVERY_SOURCE_TAG] = discovery_source_str[(int)entity->discovery_source];

    // Populate subentity array for DataWriters
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->data_writers)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[DATAWRITER_CONTAINER_TAG] = subentities;
    }

    // Populate subentity array for DataReaders
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->data_readers)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[DATAREADER_CONTAINER_TAG] = subentities;
    }

    return entity_info;
}

DatabaseDump Database::dump_data_(
        const std::map<EntityId, details::DataContainer<ByteCountSample>>& data)
{
    DatabaseDump data_dump = DatabaseDump::object();

    for (const auto& it : data)
    {
        DatabaseDump samples = DatabaseDump::array();

        for (const auto& sample : it.second)
        {
            DatabaseDump value = DatabaseDump::object();
            value[DATA_VALUE_SRC_TIME_TAG] = time_to_string(sample.src_ts);
            value[DATA_VALUE_COUNT_TAG] = sample.count;
            value[DATA_VALUE_MAGNITUDE_TAG] = sample.magnitude_order;

            samples.push_back(value);
        }

        data_dump[id_to_string(it.first.value())] = samples;
    }

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const std::map<EntityId, details::DataContainer<EntityCountSample>>& data)
{
    DatabaseDump data_dump = DatabaseDump::object();

    for (const auto& it : data)
    {
        DatabaseDump samples = DatabaseDump::array();

        for (const auto& sample : it.second)
        {
            DatabaseDump value = DatabaseDump::object();
            value[DATA_VALUE_SRC_TIME_TAG] = time_to_string(sample.src_ts);
            value[DATA_VALUE_COUNT_TAG] = sample.count;

            samples.push_back(value);
        }

        data_dump[id_to_string(it.first.value())] = samples;
    }

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const std::map<EntityId, details::DataContainer<EntityDataSample>>& data)
{
    DatabaseDump data_dump = DatabaseDump::object();

    for (const auto& it : data)
    {
        DatabaseDump samples = DatabaseDump::array();

        for (const auto& sample : it.second)
        {
            DatabaseDump value = DatabaseDump::object();
            value[DATA_VALUE_SRC_TIME_TAG] = time_to_string(sample.src_ts);
            value[DATA_VALUE_DATA_TAG] = sample.data;

            samples.push_back(value);
        }

        data_dump[id_to_string(it.first.value())] = samples;
    }

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const std::map<EntityId, details::DataContainer<DiscoveryTimeSample>>& data)
{
    DatabaseDump data_dump = DatabaseDump::object();

    for (const auto& it : data)
    {
        DatabaseDump samples = DatabaseDump::array();

        for (const auto& sample : it.second)
        {
            DatabaseDump value = DatabaseDump::object();
            value[DATA_VALUE_SRC_TIME_TAG] = time_to_string(sample.src_ts);
            value[DATA_VALUE_TIME_TAG] = time_to_string(sample.time);
            value[DATA_VALUE_REMOTE_ENTITY_TAG] = id_to_string(sample.remote_entity.value());
            value[DATA_VALUE_DISCOVERED_TAG] = sample.discovered;

            samples.push_back(value);
        }

        data_dump[id_to_string(it.first.value())] = samples;
    }

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const std::map<uint64_t, details::DataContainer<EntityCountSample>>& data)
{
    DatabaseDump data_dump = DatabaseDump::object();

    for (const auto& it : data)
    {
        DatabaseDump samples = DatabaseDump::array();

        for (const auto& sample : it.second)
        {
            DatabaseDump value = DatabaseDump::object();
            value[DATA_VALUE_SRC_TIME_TAG] = time_to_string(sample.src_ts);
            value[DATA_VALUE_COUNT_TAG] = sample.count;

            samples.push_back(value);
        }

        data_dump[id_to_string(it.first)] = samples;
    }

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const details::DataContainer<EntityCountSample>& data)
{
    DatabaseDump data_dump = DatabaseDump::array();

    for (const auto& it : data)
    {
        DatabaseDump value = DatabaseDump::object();
        value[DATA_VALUE_SRC_TIME_TAG] = time_to_string(it.src_ts);
        value[DATA_VALUE_COUNT_TAG] = it.count;

        data_dump.push_back(value);
    }

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const details::DataContainer<EntityDataSample>& data)
{
    DatabaseDump data_dump = DatabaseDump::array();

    for (const auto& it : data)
    {
        DatabaseDump value = DatabaseDump::object();
        value[DATA_VALUE_SRC_TIME_TAG] = time_to_string(it.src_ts);
        value[DATA_VALUE_DATA_TAG] = it.data;

        data_dump.push_back(value);
    }

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const EntityCountSample& data)
{
    DatabaseDump data_dump = DatabaseDump::object();
    data_dump[DATA_VALUE_SRC_TIME_TAG] = time_to_string(data.src_ts);
    data_dump[DATA_VALUE_COUNT_TAG] = data.count;

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const ByteCountSample& data)
{
    DatabaseDump data_dump = DatabaseDump::object();
    data_dump[DATA_VALUE_SRC_TIME_TAG] = time_to_string(data.src_ts);
    data_dump[DATA_VALUE_COUNT_TAG] = data.count;
    data_dump[DATA_VALUE_MAGNITUDE_TAG] = data.magnitude_order;

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const std::map<EntityId, EntityCountSample>& data)
{
    DatabaseDump data_dump = DatabaseDump::object();

    for (const auto& it : data)
    {
        data_dump[id_to_string(it.first.value())] = dump_data_(it.second);
    }

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const std::map<EntityId, ByteCountSample>& data)
{
    DatabaseDump data_dump = DatabaseDump::object();

    for (const auto& it : data)
    {
        data_dump[id_to_string(it.first.value())] = dump_data_(it.second);
    }

    return data_dump;
}

void Database::clear_statistics_data(
        const Timestamp& t_to)
{
    std::lock_guard<std::shared_timed_mutex> guard (mutex_);
    clear_statistics_data_nts_(t_to);
}

void Database::clear_statistics_data_nts_(
        const Timestamp& t_to)
{
    // Participants
    for (const auto& super_it : participants_)
    {
        // For each entity of this kind in the domain
        for (const auto& it : super_it.second)
        {
            it.second->data.clear(t_to, false);
            it.second->monitor_service_data.clear(t_to, false);
        }
    }
    // Datawriters
    for (const auto& super_it : datawriters_)
    {
        // For each entity of this kind in the domain
        for (const auto& it : super_it.second)
        {
            it.second->data.clear(t_to, false);
            it.second->monitor_service_data.clear(t_to, false);
        }
    }
    // Datareaders
    for (const auto& super_it : datareaders_)
    {
        // For each entity of this kind in the domain
        for (const auto& it : super_it.second)
        {
            it.second->data.clear(t_to, false);
            it.second->monitor_service_data.clear(t_to, false);
        }
    }
}

void Database::clear_inactive_entities()
{
    std::lock_guard<std::shared_timed_mutex> guard (mutex_);
    clear_inactive_entities_nts_();
    // Regenerate the entire graph
    for (auto it = domains_.cbegin(); it != domains_.cend(); ++it)
    {
        if (regenerate_domain_graph_nts(it->first))
        {
            // TODO (eProsima) Workaround to avoid deadlock if callback implementation requires taking the database
            // mutex (e.g. by calling get_info). A refactor for not calling on_domain_view_graph_update from within
            // this function would be required.
            execute_without_lock([&]()
                    {
                        details::StatisticsBackendData::get_instance()->on_domain_view_graph_update(it->first);
                    });
        }
    }
}

void Database::clear_inactive_entities_nts_()
{
    // Physical entities
    clear_inactive_entities_from_map_(hosts_);
    clear_inactive_entities_from_map_(users_);
    clear_inactive_entities_from_map_(processes_);

    // DDS entities
    clear_inactive_entities_from_map_(participants_);
    clear_inactive_entities_from_map_(datawriters_);
    clear_inactive_entities_from_map_(datareaders_);

    // Logic entities
    clear_inactive_entities_from_map_(topics_);

    // Domain and Locators are not affected

    // Remove internal references of entities to those that have been removed
    clear_internal_references_nts_();
}

bool Database::is_active(
        const EntityId& entity_id)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return get_entity_nts(entity_id)->active;
}

bool Database::is_metatraffic(
        const EntityId& entity_id)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return get_entity_nts(entity_id)->metatraffic;
}

bool Database::is_proxy(
        const EntityId& entity_id)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    return (get_entity_nts(entity_id)->discovery_source == DiscoverySource::PROXY);
}

Info Database::get_info(
        const EntityId& entity_id)
{
    Info info = Info::object();

    std::shared_lock<std::shared_timed_mutex> lock(mutex_);

    std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);

    info[ID_TAG] = entity_id.value();
    info[KIND_TAG] = entity_kind_str[(int)entity->kind];
    info[NAME_TAG] = entity->name;
    info[ALIAS_TAG] = entity->alias;
    info[ALIVE_TAG] = entity->active;
    info[METATRAFFIC_TAG] = entity->metatraffic;
    info[STATUS_TAG] = status_level_str[(int)entity->status];
    info[DISCOVERY_SOURCE_TAG] = discovery_source_str[(int)entity->discovery_source];

    switch (entity->kind)
    {
        case EntityKind::PROCESS:
        {
            std::shared_ptr<const Process> process =
                    std::dynamic_pointer_cast<const Process>(entity);
            info[PID_TAG] = process->pid;
            break;
        }
        case EntityKind::TOPIC:
        {
            std::shared_ptr<const Topic> topic =
                    std::dynamic_pointer_cast<const Topic>(entity);
            info[DATA_TYPE_TAG] = topic->data_type;
            break;
        }
        case EntityKind::PARTICIPANT:
        {
            std::shared_ptr<const DomainParticipant> participant =
                    std::dynamic_pointer_cast<const DomainParticipant>(entity);
            info[GUID_TAG] = participant->guid;
            info[QOS_TAG] = participant->qos;
            info[APP_ID_TAG] = app_id_str[(int)participant->app_id];
            info[APP_METADATA_TAG] = participant->app_metadata;
            info[DDS_VENDOR_TAG] = participant->dds_vendor;
            info[ORIGINAL_DOMAIN_TAG] = participant->original_domain;

            // Locators associated to endpoints
            std::set<std::string> locator_set;

            // Writers registered in the participant
            for (const auto& writer : participant->data_writers)
            {
                // Locators associated to each writer
                for (const auto& locator : writer.second.get()->locators)
                {
                    locator_set.insert(locator.second.get()->name);
                }
            }

            // Readers registered in the participant
            for (const auto& reader : participant->data_readers)
            {
                // Locators associated to each reader
                for (const auto& locator : reader.second.get()->locators)
                {
                    locator_set.insert(locator.second.get()->name);
                }
            }

            DatabaseDump locators = DatabaseDump::array();
            for (const auto& locator : locator_set)
            {
                locators.push_back(locator);
            }
            info[LOCATOR_CONTAINER_TAG] = locators;
            break;
        }
        case EntityKind::DOMAIN:
        {
            std::shared_ptr<const Domain> domain =
                    std::dynamic_pointer_cast<const Domain>(entity);
            info[DOMAIN_ID_TAG] = domain->domain_id;
            break;
        }
        case EntityKind::DATAWRITER:
        case EntityKind::DATAREADER:
        {
            std::shared_ptr<const DDSEntity> dds_entity =
                    std::dynamic_pointer_cast<const DDSEntity>(entity);
            info[GUID_TAG] = dds_entity->guid;
            info[QOS_TAG] = dds_entity->qos;
            info[APP_ID_TAG] = app_id_str[(int)dds_entity->app_id];
            info[APP_METADATA_TAG] = dds_entity->app_metadata;
            info[DDS_VENDOR_TAG] = dds_entity->dds_vendor;
            break;
        }
        default:
        {
            break;
        }
    }

    return info;
}

Info Database::get_info(
        const AlertId& alert_id)
{
    Info info = Info::object();

    std::shared_lock<std::shared_timed_mutex> lock(mutex_);

    std::shared_ptr<const AlertInfo> alert = get_alert_nts(alert_id);
    if (alert == nullptr)
    {
        throw BadParameter("Error: Alert ID does not exist");
    }

    info[ID_TAG]   = std::to_string(alert_id);
    info[ALERT_KIND_TAG] = alert_kind_str[(int)alert->get_alert_kind()];
    info[ALERT_NAME_TAG] = alert->get_alert_name();
    info[ALERT_HOST_TAG] = alert->get_host_name();
    info[ALERT_USER_TAG] = alert->get_user_name();
    info[ALERT_TOPIC_TAG] = alert->get_topic_name();

    if(alert->get_alert_kind() != AlertKind::NEW_DATA)
    {
        info[ALERT_THRESHOLD_TAG] = std::to_string(alert->get_trigger_threshold());
    }

    if (!alert->get_contact_info().empty())
    {
        info[ALERT_CONTACT_INFO_TAG] = alert->get_contact_info();
    }

    return info;
}

EntityId Database::get_endpoint_topic_id(
        const EntityId& endpoint_id)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    std::shared_ptr<const Entity> endpoint = get_entity_nts(endpoint_id);

    // Check if the entity is a valid endpoint
    if (endpoint->kind != EntityKind::DATAWRITER && endpoint->kind != EntityKind::DATAREADER)
    {
        throw BadParameter("Error: Entity is not a valid endpoint");
    }

    return std::dynamic_pointer_cast<const DDSEndpoint>(endpoint)->topic->id;
}

EntityId Database::get_domain_id(
        const EntityId& entity_id)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    std::shared_ptr<const Entity> entity = get_entity_nts(entity_id);

    switch (entity->kind)
    {
        case EntityKind::DOMAIN:
        {
            return entity_id;
        }
        case EntityKind::PARTICIPANT:
        {
            return std::dynamic_pointer_cast<const DomainParticipant>(entity)->domain->id;
        }
        case EntityKind::TOPIC:
        {
            return std::dynamic_pointer_cast<const Topic>(entity)->domain->id;
        }
        case EntityKind::DATAWRITER:
        case EntityKind::DATAREADER:
        {
            return std::dynamic_pointer_cast<const DDSEndpoint>(entity)->participant->domain->id;
        }
        default:
        {
            return EntityId::invalid();
        }
    }

}

void Database::check_entity_kinds(
        EntityKind kind,
        const std::vector<EntityId>& entity_ids,
        const char* message)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    for (EntityId id : entity_ids)
    {
        std::shared_ptr<const Entity> entity = get_entity_nts(id);
        if (!entity || kind != entity->kind)
        {
            throw BadParameter(message);
        }
    }
}

void Database::check_entity_kinds(
        EntityKind kinds[3],
        const std::vector<EntityId>& entity_ids,
        const char* message)
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    for (EntityId id : entity_ids)
    {
        std::shared_ptr<const Entity> entity = get_entity_nts(id);
        if (!entity || (kinds[0] != entity->kind && kinds[1] != entity->kind && kinds[2] != entity->kind))
        {
            throw BadParameter(message);
        }
    }
}

/**
 * @brief Check that in the 'dump', the references of the entity iterator 'it' of type 'entity_tag'
 * to entities of type 'reference_tag' are consistent and mutual. For this, the referenced entities must
 * have reference to 'it' of type 'entity_tag'.
 *
 * Example -> Check that each user, reference host[0]:
 *
 * \code
 * {
 *      host["0"]
 *      {
 *          users: ["1","5","9"]
 *      }
 *      user["1"]
 *      {
 *          host: "0"
 *      }
 * }
 * \endcode
 *
 * @param dump reference to the database dump.
 * @param it iterator to the dump of the entity.
 * @param entity_tag Type of the entity to check.
 * @param reference_tag Type of the referenced entity to check.
 * @throws eprosima::statistics_backend::FileCorrupted if the references are not consistent and mutual.
 */
void check_entity_contains_all_references(
        DatabaseDump const& dump,
        nlohmann::json::iterator const& it,
        std::string const& entity_tag,
        std::string const& reference_container_tag,
        std::string const& reference_tag)
{
    std::string entity_id = it.key();
    DatabaseDump references_id = (*it).at(reference_tag);
    DatabaseDump reference_container = dump.at(reference_container_tag);

    // Check all 'references_id' in the 'reference_container'
    for (auto refIt = references_id.begin(); refIt != references_id.end(); ++refIt)
    {
        std::string referenced_id = *refIt;

        // 1) Check that the 'referenced_id' entity exists.
        if (!reference_container.contains(referenced_id))
        {
            throw CorruptedFile(
                      "Entity container: " + reference_container.dump() + " do not have a Entity with ID: " +
                      referenced_id);
        }

        // 2) Check that referenced entity contains a reference to an 'entity_id' of type 'entity_tag'.
        std::vector<std::string> referenced_entities;
        auto referenced_entities_json = reference_container.at(referenced_id).at(entity_tag);
        if (referenced_entities_json.is_array())
        {
            referenced_entities.insert(referenced_entities.end(),
                    referenced_entities_json.begin(),
                    referenced_entities_json.end());
        }
        else
        {
            referenced_entities.push_back(referenced_entities_json.get<std::string>());
        }

        if (std::find(referenced_entities.begin(), referenced_entities.end(), entity_id) == referenced_entities.end())
        {
            throw CorruptedFile("Entity with ID (" + referenced_id + ") :" + reference_container.at(
                              referenced_id).dump() +
                          " has reference to " + entity_tag + ": " +
                          reference_container.at(referenced_id).at(entity_tag).dump() +
                          " instead of " + entity_tag + ": " + entity_id);
        }
    }
}

void Database::load_database(
        const DatabaseDump& dump)
{
    std::lock_guard<std::shared_timed_mutex> guard (mutex_);

    if (next_id_ != 0)
    {
        throw PreconditionNotMet("Error: Database not empty");
    }

    // Locators
    {
        DatabaseDump container = dump.at(LOCATOR_CONTAINER_TAG);
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, LOCATOR_CONTAINER_TAG, DATAWRITER_CONTAINER_TAG,
                    DATAWRITER_CONTAINER_TAG);
            check_entity_contains_all_references(dump, it, LOCATOR_CONTAINER_TAG, DATAREADER_CONTAINER_TAG,
                    DATAREADER_CONTAINER_TAG);

            // Create entity
            std::shared_ptr<Locator> entity = std::make_shared<Locator>((*it).at(NAME_TAG));
            entity->alias = (*it).at(ALIAS_TAG);

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);
        }
    }

    // Hosts
    {
        DatabaseDump container = dump.at(HOST_CONTAINER_TAG);

        // For each entity of this kind in the database
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, HOST_ENTITY_TAG, USER_CONTAINER_TAG, USER_CONTAINER_TAG);

            // Create entity
            std::shared_ptr<Host> entity = std::make_shared<Host>((*it).at(NAME_TAG));
            entity->alias = (*it).at(ALIAS_TAG);

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);
        }
    }

    // Users
    {
        DatabaseDump container = dump.at(USER_CONTAINER_TAG);

        // For each entity of this kind in the database
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, USER_CONTAINER_TAG, HOST_CONTAINER_TAG, HOST_ENTITY_TAG);
            check_entity_contains_all_references(dump, it, USER_ENTITY_TAG, PROCESS_CONTAINER_TAG,
                    PROCESS_CONTAINER_TAG);

            // Create entity
            std::shared_ptr<User> entity = std::make_shared<User>((*it).at(NAME_TAG),
                            hosts_[string_to_int((*it).at(HOST_ENTITY_TAG))]);
            entity->alias = (*it).at(ALIAS_TAG);

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);
        }
    }

    // Processes
    {
        DatabaseDump container = dump.at(PROCESS_CONTAINER_TAG);

        // For each entity of this kind in the database
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, PROCESS_CONTAINER_TAG, USER_CONTAINER_TAG, USER_ENTITY_TAG);
            check_entity_contains_all_references(dump, it, PROCESS_ENTITY_TAG, PARTICIPANT_CONTAINER_TAG,
                    PARTICIPANT_CONTAINER_TAG);

            // Create entity
            std::shared_ptr<Process> entity =
                    std::make_shared<Process>((*it).at(NAME_TAG), (*it).at(PID_TAG),
                            users_[EntityId(string_to_int((*it).at(USER_ENTITY_TAG)))]);
            entity->alias = (*it).at(ALIAS_TAG);

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);
        }
    }

    // Domains
    {
        DatabaseDump container = dump.at(DOMAIN_CONTAINER_TAG);

        // For each entity of this kind in the database
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, DOMAIN_ENTITY_TAG, PARTICIPANT_CONTAINER_TAG,
                    PARTICIPANT_CONTAINER_TAG);
            check_entity_contains_all_references(dump, it, DOMAIN_ENTITY_TAG, TOPIC_CONTAINER_TAG, TOPIC_CONTAINER_TAG);

            // Create entity
            std::shared_ptr<Domain> entity = std::make_shared<Domain>((*it).at(NAME_TAG), (*it).at(DOMAIN_ID_TAG));
            entity->alias = (*it).at(ALIAS_TAG);

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);
        }
    }

    // Topics
    {
        DatabaseDump container = dump.at(TOPIC_CONTAINER_TAG);

        // For each entity of this kind in the database
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, TOPIC_CONTAINER_TAG, DOMAIN_CONTAINER_TAG,
                    DOMAIN_ENTITY_TAG);
            check_entity_contains_all_references(dump, it, TOPIC_ENTITY_TAG, DATAWRITER_CONTAINER_TAG,
                    DATAWRITER_CONTAINER_TAG);
            check_entity_contains_all_references(dump, it, TOPIC_ENTITY_TAG, DATAREADER_CONTAINER_TAG,
                    DATAREADER_CONTAINER_TAG);

            // Create entity
            std::shared_ptr<Topic> entity =
                    std::make_shared<Topic>((*it).at(NAME_TAG), (*it).at(DATA_TYPE_TAG),
                            domains_[EntityId(string_to_int((*it).at(DOMAIN_ENTITY_TAG)))]);
            entity->alias = (*it).at(ALIAS_TAG);

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);
        }
    }

    // Participants
    {
        DatabaseDump container = dump.at(PARTICIPANT_CONTAINER_TAG);

        // For each entity of this kind in the database
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, PARTICIPANT_CONTAINER_TAG, DOMAIN_CONTAINER_TAG,
                    DOMAIN_ENTITY_TAG);
            check_entity_contains_all_references(dump, it, PARTICIPANT_ENTITY_TAG, DATAWRITER_CONTAINER_TAG,
                    DATAWRITER_CONTAINER_TAG);
            check_entity_contains_all_references(dump, it, PARTICIPANT_ENTITY_TAG, DATAREADER_CONTAINER_TAG,
                    DATAREADER_CONTAINER_TAG);

            // Create entity
            std::shared_ptr<DomainParticipant> entity = std::make_shared<DomainParticipant>(
                (*it).at(NAME_TAG), (*it).at(QOS_TAG), (*it).at(GUID_TAG), nullptr,
                domains_[EntityId(string_to_int((*it).at(DOMAIN_ENTITY_TAG)))]);
            entity->alias = (*it).at(ALIAS_TAG);

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);

            // Link participant with process
            EntityId process_id(string_to_int((*it).at(PROCESS_ENTITY_TAG)));

            if (process_id != EntityId::invalid())
            {
                check_entity_contains_all_references(dump, it, PARTICIPANT_CONTAINER_TAG, PROCESS_CONTAINER_TAG,
                        PROCESS_ENTITY_TAG);

                link_participant_with_process_nts(entity->id, process_id);
            }

            // Load data and insert into database
            load_data((*it).at(DATA_CONTAINER_TAG), entity);
        }
    }

    // DataWriters
    {
        DatabaseDump container = dump.at(DATAWRITER_CONTAINER_TAG);

        // For each entity of this kind in the database
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, DATAWRITER_CONTAINER_TAG, PARTICIPANT_CONTAINER_TAG,
                    PARTICIPANT_ENTITY_TAG);
            check_entity_contains_all_references(dump, it, DATAWRITER_CONTAINER_TAG, TOPIC_CONTAINER_TAG,
                    TOPIC_ENTITY_TAG);
            check_entity_contains_all_references(dump, it, DATAWRITER_CONTAINER_TAG, LOCATOR_CONTAINER_TAG,
                    LOCATOR_CONTAINER_TAG);

            // Get keys
            EntityId participant_id = EntityId(string_to_int((*it).at(PARTICIPANT_ENTITY_TAG)));
            EntityId participant_domain_id =
                    EntityId(string_to_int(dump.at(PARTICIPANT_CONTAINER_TAG)
                                    .at(std::to_string(participant_id.value()))
                                    .at(DOMAIN_ENTITY_TAG)));

            EntityId topic_id = EntityId(string_to_int((*it).at(TOPIC_ENTITY_TAG)));
            EntityId topic_domain_id = EntityId(string_to_int(dump.at(TOPIC_CONTAINER_TAG)
                                    .at(std::to_string(topic_id.value()))
                                    .at(DOMAIN_ENTITY_TAG)));

            // Create entity
            std::shared_ptr<DataWriter> entity = std::make_shared<DataWriter>(
                (*it).at(NAME_TAG),
                (*it).at(QOS_TAG),
                (*it).at(GUID_TAG),
                participants_[participant_domain_id][participant_id],
                topics_[topic_domain_id][topic_id]);
            entity->alias = (*it).at(ALIAS_TAG);
            entity->is_virtual_metatraffic = (*it).at(VIRTUAL_METATRAFFIC_TAG).get<bool>();

            /* Add reference to locator to the endpoint */
            for (auto it_loc = (*it).at(LOCATOR_CONTAINER_TAG).begin();
                    it_loc != (*it).at(LOCATOR_CONTAINER_TAG).end();
                    ++it_loc)
            {
                entity->locators[string_to_int(*it_loc)] =
                        locators_[EntityId(string_to_int(*it_loc))];
            }

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);

            // Load data and insert into database
            load_data((*it).at(DATA_CONTAINER_TAG), entity);
        }
    }

    // DataReaders
    {
        DatabaseDump container = dump.at(DATAREADER_CONTAINER_TAG);
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, DATAREADER_CONTAINER_TAG, PARTICIPANT_CONTAINER_TAG,
                    PARTICIPANT_ENTITY_TAG);
            check_entity_contains_all_references(dump, it, DATAREADER_CONTAINER_TAG, TOPIC_CONTAINER_TAG,
                    TOPIC_ENTITY_TAG);
            check_entity_contains_all_references(dump, it, DATAREADER_CONTAINER_TAG, LOCATOR_CONTAINER_TAG,
                    LOCATOR_CONTAINER_TAG);

            // Get keys
            EntityId participant_id = EntityId(string_to_int((*it).at(
                                PARTICIPANT_ENTITY_TAG)));
            EntityId participant_domain_id =
                    EntityId(string_to_int(dump.at(PARTICIPANT_CONTAINER_TAG)
                                    .at(std::to_string(participant_id.value()))
                                    .at(DOMAIN_ENTITY_TAG)));

            EntityId topic_id = EntityId(string_to_int((*it).at(TOPIC_ENTITY_TAG)));
            EntityId topic_domain_id = EntityId(string_to_int(dump.at(TOPIC_CONTAINER_TAG)
                                    .at(std::to_string(topic_id.value()))
                                    .at(DOMAIN_ENTITY_TAG)));

            // Create entity
            std::shared_ptr<DataReader> entity = std::make_shared<DataReader>(
                (*it).at(NAME_TAG),
                (*it).at(QOS_TAG),
                (*it).at(GUID_TAG),
                participants_[participant_domain_id][participant_id],
                topics_[topic_domain_id][topic_id]);
            entity->alias = (*it).at(ALIAS_TAG);
            entity->is_virtual_metatraffic = (*it).at(VIRTUAL_METATRAFFIC_TAG).get<bool>();

            /* Add reference to locator to the endpoint */
            for (auto it_loc = (*it).at(LOCATOR_CONTAINER_TAG).begin();
                    it_loc != (*it).at(LOCATOR_CONTAINER_TAG).end();
                    ++it_loc)
            {
                entity->locators[string_to_int(*it_loc)] =
                        locators_[EntityId(string_to_int(*it_loc))];
            }

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);

            // Load data and insert into database
            load_data((*it).at(DATA_CONTAINER_TAG), entity);
        }
    }
}

void Database::load_data(
        const DatabaseDump& dump,
        const std::shared_ptr<DomainParticipant>& entity)
{
    // discovery_time
    {
        DatabaseDump container = dump.at(DATA_KIND_DISCOVERY_TIME_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            // Data iterator
            for (auto it = container.at(remote_it.key()).begin(); it != container.at(remote_it.key()).end(); ++it)
            {
                DiscoveryTimeSample sample;

                // std::chrono::system_clock::time_point
                uint64_t src_ts = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
                sample.src_ts = nanoseconds_to_systemclock(src_ts);

                // std::chrono::system_clock::time_point
                uint64_t time = string_to_uint(std::string((*it).at(DATA_VALUE_TIME_TAG)));
                sample.time = nanoseconds_to_systemclock(time);

                // EntityId
                sample.remote_entity = EntityId(string_to_int(remote_it.key()));

                string_to_int(std::string((*it).at(DATA_VALUE_REMOTE_ENTITY_TAG)));

                // bool
                sample.discovered = (*it).at(DATA_VALUE_DISCOVERED_TAG);

                // Insert data into database
                insert_nts(entity->domain->id, entity->id, sample, true);
            }
        }
    }

    // pdp_packets
    {
        DatabaseDump container = dump.at(DATA_KIND_PDP_PACKETS_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            PdpCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->domain->id, entity->id, sample, true);
        }
    }

    // edp_packets
    {
        DatabaseDump container = dump.at(DATA_KIND_EDP_PACKETS_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            EdpCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->domain->id, entity->id, sample, true);
        }
    }

    // rtps_packets_sent
    {
        DatabaseDump container = dump.at(DATA_KIND_RTPS_PACKETS_SENT_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            // Data iterator
            for (auto it = container.at(remote_it.key()).begin(); it != container.at(remote_it.key()).end(); ++it)
            {
                RtpsPacketsSentSample sample;

                // std::chrono::system_clock::time_point
                uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
                sample.src_ts = nanoseconds_to_systemclock(time);

                // uint64_t
                sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

                // EntityId
                sample.remote_locator = EntityId(string_to_int(remote_it.key()));

                // Insert data into database
                insert_nts(entity->domain->id, entity->id, sample, true);
            }
        }
    }

    // rtps_bytes_sent
    {
        DatabaseDump container = dump.at(DATA_KIND_RTPS_BYTES_SENT_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            // Data iterator
            for (auto it = container.at(remote_it.key()).begin(); it != container.at(remote_it.key()).end(); ++it)
            {
                RtpsBytesSentSample sample;

                // std::chrono::system_clock::time_point
                uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
                sample.src_ts = nanoseconds_to_systemclock(time);

                // uint64_t
                sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

                // int16_t
                sample.magnitude_order =
                        static_cast<int16_t>(string_to_int(to_string((*it).at(DATA_VALUE_MAGNITUDE_TAG))));

                // EntityId
                sample.remote_locator = EntityId(string_to_int(remote_it.key()));

                // Insert data into database
                insert_nts(entity->domain->id, entity->id, sample, true);
            }
        }
    }

    // rtps_packets_lost
    {
        DatabaseDump container = dump.at(DATA_KIND_RTPS_PACKETS_LOST_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            // Data iterator
            for (auto it = container.at(remote_it.key()).begin(); it != container.at(remote_it.key()).end(); ++it)
            {
                RtpsPacketsLostSample sample;

                // std::chrono::system_clock::time_point
                uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
                sample.src_ts = nanoseconds_to_systemclock(time);

                // uint64_t
                sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

                // EntityId
                sample.remote_locator = EntityId(string_to_int(remote_it.key()));

                // Insert data into database
                insert_nts(entity->domain->id, entity->id, sample, true);
            }
        }
    }

    // rtps_bytes_lost
    {
        DatabaseDump container = dump.at(DATA_KIND_RTPS_BYTES_LOST_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            // Data iterator
            for (auto it = container.at(remote_it.key()).begin(); it != container.at(remote_it.key()).end(); ++it)
            {
                RtpsBytesLostSample sample;

                // std::chrono::system_clock::time_point
                uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
                sample.src_ts = nanoseconds_to_systemclock(time);

                // uint64_t
                sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

                // int16_t
                sample.magnitude_order =
                        static_cast<int16_t>(string_to_int(to_string((*it).at(DATA_VALUE_MAGNITUDE_TAG))));

                // EntityId
                sample.remote_locator = EntityId(string_to_int(remote_it.key()));

                // Insert data into database
                insert_nts(entity->domain->id, entity->id, sample, true);
            }
        }
    }

    // network_latency
    {
        DatabaseDump container = dump.at(DATA_KIND_NETWORK_LATENCY_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            // Data iterator
            for (auto it = container.at(remote_it.key()).begin(); it != container.at(remote_it.key()).end(); ++it)
            {
                NetworkLatencySample sample;

                // std::chrono::system_clock::time_point
                uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
                sample.src_ts = nanoseconds_to_systemclock(time);

                // double
                sample.data = (*it).at(DATA_VALUE_DATA_TAG);

                // EntityId
                sample.remote_locator = EntityId(string_to_int(remote_it.key()));

                // Insert data into database
                insert_nts(entity->domain->id, entity->id, sample, true);
            }
        }
    }

    // last_reported_rtps_bytes_lost
    {
        DatabaseDump container = dump.at(DATA_KIND_RTPS_BYTES_LOST_LAST_REPORTED_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            RtpsBytesLostSample sample;
            DatabaseDump sample_dump = container.at(remote_it.key());

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(sample_dump.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string(sample_dump.at(DATA_VALUE_COUNT_TAG)));

            // int16_t
            sample.magnitude_order =
                    static_cast<int16_t>(string_to_int(to_string(sample_dump.at(DATA_VALUE_MAGNITUDE_TAG))));

            // EntityId
            sample.remote_locator = EntityId(string_to_int(remote_it.key()));

            // Insert data into database
            insert_nts(entity->domain->id, entity->id, sample, true, true);
        }
    }

    // last_reported_rtps_bytes_sent
    {
        DatabaseDump container = dump.at(DATA_KIND_RTPS_BYTES_SENT_LAST_REPORTED_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            RtpsBytesSentSample sample;
            DatabaseDump sample_dump = container.at(remote_it.key());

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(sample_dump.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string(sample_dump.at(DATA_VALUE_COUNT_TAG)));

            // int16_t
            sample.magnitude_order =
                    static_cast<int16_t>(string_to_int(to_string(sample_dump.at(DATA_VALUE_MAGNITUDE_TAG))));

            // EntityId
            sample.remote_locator = EntityId(string_to_int(remote_it.key()));

            // Insert data into database
            insert_nts(entity->domain->id, entity->id, sample, true, true);
        }
    }

    // last_reported_rtps_packets_lost
    {
        DatabaseDump container = dump.at(DATA_KIND_RTPS_PACKETS_LOST_LAST_REPORTED_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            RtpsPacketsLostSample sample;
            DatabaseDump sample_dump = container.at(remote_it.key());

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(sample_dump.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string(sample_dump.at(DATA_VALUE_COUNT_TAG)));

            // EntityId
            sample.remote_locator = EntityId(string_to_int(remote_it.key()));

            // Insert data into database
            insert_nts(entity->domain->id, entity->id, sample, true, true);
        }
    }

    // last_reported_rtps_packets_sent
    {
        DatabaseDump container = dump.at(DATA_KIND_RTPS_PACKETS_SENT_LAST_REPORTED_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            RtpsPacketsSentSample sample;
            DatabaseDump sample_dump = container.at(remote_it.key());

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(sample_dump.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string(sample_dump.at(DATA_VALUE_COUNT_TAG)));

            // EntityId
            sample.remote_locator = EntityId(string_to_int(remote_it.key()));

            // Insert data into database
            insert_nts(entity->domain->id, entity->id, sample, true, true);
        }
    }

    //last_reported_edp_packets
    {
        // Only insert last reported if there are at least one
        if (!dump.at(DATA_KIND_EDP_PACKETS_TAG).empty())
        {
            DatabaseDump container = dump.at(DATA_KIND_EDP_PACKETS_LAST_REPORTED_TAG);

            EdpCountSample sample;

            //std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(container.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t count;
            sample.count = string_to_uint(to_string(container.at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->domain->id, entity->id, sample, true, true);
        }
    }

    // last_reported_pdp_packets
    {
        // Only insert last reported if there are at least one
        if (!dump.at(DATA_KIND_PDP_PACKETS_TAG).empty())
        {
            DatabaseDump container = dump.at(DATA_KIND_PDP_PACKETS_LAST_REPORTED_TAG);

            PdpCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(container.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t count;
            sample.count = string_to_uint(to_string(container.at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->domain->id, entity->id, sample, true, true);
        }
    }
}

void Database::load_data(
        const DatabaseDump& dump,
        const std::shared_ptr<DataWriter>& entity)
{
    // publication_throughput
    {
        DatabaseDump container = dump.at(DATA_KIND_PUBLICATION_THROUGHPUT_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            PublicationThroughputSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // double
            sample.data = (*it).at(DATA_VALUE_DATA_TAG);

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true);
        }
    }

    // resent_datas
    {
        DatabaseDump container = dump.at(DATA_KIND_RESENT_DATA_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            ResentDataSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true);
        }
    }

    // heartbeat_count
    {
        DatabaseDump container = dump.at(DATA_KIND_HEARTBEAT_COUNT_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            HeartbeatCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true);
        }
    }

    // gap_count
    {
        DatabaseDump container = dump.at(DATA_KIND_GAP_COUNT_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            GapCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true);
        }
    }

    // data_count
    {
        DatabaseDump container = dump.at(DATA_KIND_DATA_COUNT_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            DataCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true);
        }
    }

    // samples_datas
    {
        DatabaseDump container = dump.at(DATA_KIND_SAMPLE_DATAS_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            // Data iterator
            for (auto it = container.at(remote_it.key()).begin(); it != container.at(remote_it.key()).end(); ++it)
            {
                SampleDatasCountSample sample;

                // std::chrono::system_clock::time_point
                uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
                sample.src_ts = nanoseconds_to_systemclock(time);

                // uint64_t
                sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

                // uint64_t
                sample.sequence_number = string_to_int(remote_it.key());

                // Insert data into database
                insert_nts(entity->participant->domain->id, entity->id, sample, true);
            }
        }
    }

    // history2history_latency
    {
        DatabaseDump container = dump.at(DATA_KIND_FASTDDS_LATENCY_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            // Data iterator
            for (auto it = container.at(remote_it.key()).begin(); it != container.at(remote_it.key()).end(); ++it)
            {
                HistoryLatencySample sample;

                // std::chrono::system_clock::time_point
                uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
                sample.src_ts = nanoseconds_to_systemclock(time);

                // double
                sample.data = (*it).at(DATA_VALUE_DATA_TAG);

                // EntityId
                sample.reader = EntityId(string_to_int(remote_it.key()));

                // Insert data into database
                insert_nts(entity->participant->domain->id, entity->id, sample, true);
            }
        }
    }

    // last_reported_data_count
    {
        // Only insert last reported if there are at least one
        if (!dump.at(DATA_KIND_DATA_COUNT_TAG).empty())
        {
            DatabaseDump container = dump.at(DATA_KIND_DATA_COUNT_LAST_REPORTED_TAG);

            DataCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(container.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t count;
            sample.count = string_to_uint(to_string(container.at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true, true);
        }
    }

    // last_reported_gap_count
    {
        // Only insert last reported if there are at least one
        if (!dump.at(DATA_KIND_GAP_COUNT_TAG).empty())
        {
            DatabaseDump container = dump.at(DATA_KIND_GAP_COUNT_LAST_REPORTED_TAG);

            GapCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(container.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t count;
            sample.count = string_to_uint(to_string(container.at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true, true);
        }
    }

    // last_reported_heartbeat_count
    {
        // Only insert last reported if there are at least one
        if (!dump.at(DATA_KIND_HEARTBEAT_COUNT_TAG).empty())
        {
            DatabaseDump container = dump.at(DATA_KIND_HEARTBEAT_COUNT_LAST_REPORTED_TAG);

            HeartbeatCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(container.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t count;
            sample.count = string_to_uint(to_string(container.at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true, true);
        }
    }

    // last_reported_resent_datas
    {
        // Only insert last reported if there are at least one
        if (!dump.at(DATA_KIND_RESENT_DATA_TAG).empty())
        {
            DatabaseDump container = dump.at(DATA_KIND_RESENT_DATA_LAST_REPORTED_TAG);

            ResentDataSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(container.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t count;
            sample.count = string_to_uint(to_string(container.at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true, true);
        }
    }
}

void Database::load_data(
        const DatabaseDump& dump,
        const std::shared_ptr<DataReader>& entity)
{
    // subscription_throughput
    {
        DatabaseDump container = dump.at(DATA_KIND_SUBSCRIPTION_THROUGHPUT_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            SubscriptionThroughputSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // double
            sample.data = (*it).at(DATA_VALUE_DATA_TAG);

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true);
        }
    }

    // acknack_count
    {
        DatabaseDump container = dump.at(DATA_KIND_ACKNACK_COUNT_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            AcknackCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true);
        }
    }

    // nackfrag_count
    {
        DatabaseDump container = dump.at(DATA_KIND_NACKFRAG_COUNT_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            NackfragCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true);
        }
    }

    // last_reported_acknack_count
    {
        // Only insert last reported if there are at least one
        if (!dump.at(DATA_KIND_ACKNACK_COUNT_TAG).empty())
        {
            DatabaseDump container = dump.at(DATA_KIND_ACKNACK_COUNT_LAST_REPORTED_TAG);

            AcknackCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(container.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t count;
            sample.count = string_to_uint(to_string(container.at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true, true);
        }
    }

    // last_reported_nackfrag_count
    {
        // Only insert last reported if there are at least one
        if (!dump.at(DATA_KIND_NACKFRAG_COUNT_TAG).empty())
        {
            DatabaseDump container = dump.at(DATA_KIND_NACKFRAG_COUNT_LAST_REPORTED_TAG);

            NackfragCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(container.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t count;
            sample.count = string_to_uint(to_string(container.at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true, true);
        }
    }
}

// Conversion from bool to DiscoveryStatus
details::StatisticsBackendData::DiscoveryStatus get_status(
        bool active)
{
    if (active)
    {
        return details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
    }
    else
    {
        return details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY;
    }
}

void Database::change_entity_status_of_kind(
        const EntityId& entity_id,
        bool active,
        const EntityKind& entity_kind,
        const EntityId& domain_id) noexcept
{
    switch (entity_kind)
    {
        case EntityKind::HOST:
        {
            std::shared_ptr<Host> host;

            for (const auto& host_it : hosts_)
            {
                if (host_it.second->id == entity_id)
                {
                    host = host_it.second;
                    break;
                }
            }
            if (host != nullptr && host->active != active)
            {
                host->active = active;

                // TODO (eProsima) Workaround to avoid deadlock if callback implementation requires taking the database
                // mutex (e.g. by calling get_info). A refactor for not calling on_physical_entity_discovery from within
                // this function would be required.
                execute_without_lock([&]()
                        {
                            details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(entity_id,
                            entity_kind, get_status(active));
                        });
            }
            break;
        }
        case EntityKind::USER:
        {
            std::shared_ptr<User> user;

            for (const auto& user_it : users_)
            {
                if (user_it.second->id == entity_id)
                {
                    user = user_it.second;
                    break;
                }
            }
            if (user != nullptr && user->active != active)
            {
                user->active = active;

                // host
                {
                    bool change_status = true;

                    // If the entity now is active, the subentity must be active.
                    // But if the subentity is already active, is not necessary to change its status.
                    if (active && user->host->active)
                    {
                        change_status = false;
                    }
                    // If the entity now is inactive, check if one reference of the subentity is active.
                    // If one is still active, the status of the subentity is not changed
                    else if (!active)
                    {
                        for (const auto& entity_it : user->host->users)
                        {
                            if (entity_it.second->active)
                            {
                                change_status = false;
                                break;
                            }
                        }
                    }

                    if (change_status)
                    {
                        change_entity_status_of_kind(user->host->id, active, user->host->kind, domain_id);
                    }
                }

                // Discovering user after discovering host

                // TODO (eProsima) Workaround to avoid deadlock if callback implementation requires taking the database
                // mutex (e.g. by calling get_info). A refactor for not calling on_physical_entity_discovery from within
                // this function would be required.
                execute_without_lock([&]()
                        {
                            details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(entity_id,
                            entity_kind, get_status(active));
                        });
            }
            break;
        }
        case EntityKind::PROCESS:
        {
            std::shared_ptr<Process> process;

            for (const auto& process_it : processes_)
            {
                if (process_it.second->id == entity_id)
                {
                    process = process_it.second;
                    break;
                }
            }
            if (process != nullptr && process->active != active)
            {
                process->active = active;

                // user
                {
                    bool change_status = true;

                    // If the entity now is active, the subentity must be active.
                    // But if the subentity is already active, is not necessary to change its status.
                    if (active && process->user->active)
                    {
                        change_status = false;
                    }
                    // If the entity now is inactive, check if one reference of the subentity is active.
                    // If one is still active, the status of the subentity is not changed
                    else if (!active)
                    {
                        for (const auto& entity_it : process->user->processes)
                        {
                            if (entity_it.second->active)
                            {
                                change_status = false;
                                break;
                            }
                        }
                    }

                    if (change_status)
                    {
                        change_entity_status_of_kind(process->user->id, active, process->user->kind, domain_id);
                    }
                }

                // Discovering process after discovering user

                // TODO (eProsima) Workaround to avoid deadlock if callback implementation requires taking the database
                // mutex (e.g. by calling get_info). A refactor for not calling on_physical_entity_discovery from within
                // this function would be required.
                execute_without_lock([&]()
                        {
                            details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(entity_id,
                            entity_kind, get_status(active));
                        });
            }
            break;
        }
        case EntityKind::DOMAIN:
        {
            std::shared_ptr<Domain> domain;

            for (const auto& domain_it : domains_)
            {
                if (domain_it.second->id == entity_id)
                {
                    domain = domain_it.second;
                    break;
                }
            }
            if (domain != nullptr && domain->active != active)
            {
                domain->active = active;
            }
            break;
        }
        case EntityKind::TOPIC:
        {
            std::shared_ptr<Topic> topic;

            for (const auto& domain_it : topics_)
            {
                for (const auto& topic_it : domain_it.second)
                {
                    if (topic_it.second->id == entity_id)
                    {
                        topic = topic_it.second;
                        break;
                    }
                }
            }
            if (topic != nullptr && topic->active != active)
            {
                topic->active = active;

                // TODO (eProsima) Workaround to avoid deadlock if callback implementation requires taking the database
                // mutex (e.g. by calling get_info). A refactor for not calling on_domain_entity_discovery from within
                // this function would be required.
                execute_without_lock([&]()
                        {
                            details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(domain_id,
                            entity_id, entity_kind, get_status(active));
                        });
            }
            break;
        }
        case EntityKind::PARTICIPANT:
        {
            std::shared_ptr<DomainParticipant> participant;

            for (const auto& domain_it : participants_)
            {
                for (const auto& participant_it : domain_it.second)
                {
                    if (participant_it.second->id == entity_id)
                    {
                        participant = participant_it.second;
                        break;
                    }
                }
            }

            if (participant != nullptr && participant->active != active)
            {
                participant->active = active;

                // process
                if (participant->process != nullptr)
                {
                    bool change_status = true;

                    // If the entity now is active, the subentity must be active.
                    // But if the subentity is already active, is not necessary to change its status.
                    if (active && participant->process->active)
                    {
                        change_status = false;
                    }
                    // If the entity now is inactive, check if one reference of the subentity is active.
                    // If one is still active, the status of the subentity is not changed
                    else if (!active)
                    {
                        for (const auto& entity_it : participant->process->participants)
                        {
                            if (entity_it.second->active)
                            {
                                change_status = false;
                                break;
                            }
                        }
                    }

                    if (change_status)
                    {
                        change_entity_status_of_kind(participant->process->id, active, participant->process->kind,
                                participant->domain->id);
                    }
                }

                // If Participant is being deactivated, all its endpoints become inactive.
                // In case Participant turns back to life (e.g. liveliness) entities should not.
                if (!active)
                {
                    // subentities DataWriters
                    for (auto dw_it : participant->data_writers)
                    {
                        change_entity_status_of_kind(
                            dw_it.first,
                            active,
                            EntityKind::DATAWRITER,
                            participant->domain->id);
                    }

                    // subentities DataReaders
                    for (auto dr_it : participant->data_readers)
                    {
                        change_entity_status_of_kind(
                            dr_it.first,
                            active,
                            EntityKind::DATAREADER,
                            participant->domain->id);
                    }
                }
            }

            break;
        }
        case EntityKind::DATAWRITER:
        {
            std::shared_ptr<DataWriter> datawriter;

            for (const auto& domain_it : datawriters_)
            {
                for (const auto& datawriter_it : domain_it.second)
                {
                    if (datawriter_it.second->id == entity_id)
                    {
                        datawriter = datawriter_it.second;
                        break;
                    }
                }
            }

            if (datawriter != nullptr && (datawriter->active != active || datawriter->topic->active != active))
            {
                datawriter->active = active;

                // topic
                {
                    bool change_status = true;

                    // If the entity now is active, the subentity must be active.
                    // But if the subentity is already active, is not necessary to change its status.
                    if (active && datawriter->topic->active)
                    {
                        change_status = false;
                    }
                    // If the entity now is inactive, check if one reference of the subentity is active.
                    // If one is still active, the status of the subentity is not changed
                    else if (!active)
                    {
                        for (const auto& entity_it : datawriter->topic->data_writers)
                        {
                            if (entity_it.second->active)
                            {
                                change_status = false;
                                break;
                            }
                        }
                        if (change_status)
                        {
                            for (const auto& entity_it : datawriter->topic->data_readers)
                            {
                                if (entity_it.second->active)
                                {
                                    change_status = false;
                                    break;
                                }
                            }
                        }
                    }

                    if (change_status)
                    {
                        change_entity_status_of_kind(datawriter->topic->id, active, datawriter->topic->kind,
                                datawriter->participant->domain->id);
                    }
                }
            }

            break;
        }
        case EntityKind::DATAREADER:
        {
            std::shared_ptr<DataReader> datareader;

            for (const auto& domain_it : datareaders_)
            {
                for (const auto& datareader_it : domain_it.second)
                {
                    if (datareader_it.second->id == entity_id)
                    {
                        datareader = datareader_it.second;
                        break;
                    }
                }
            }
            if (datareader != nullptr && (datareader->active != active || datareader->topic->active != active))
            {
                datareader->active = active;

                // topic
                {
                    bool change_status = true;

                    // If the entity now is active, the subentity must be active.
                    // But if the subentity is already active, is not necessary to change its status.
                    if (active && datareader->topic->active)
                    {
                        change_status = false;
                    }
                    // If the entity now is inactive, check if one reference of the subentity is active.
                    // If one is still active, the status of the subentity is not changed
                    else if (!active)
                    {
                        for (const auto& entity_it : datareader->topic->data_readers)
                        {
                            if (entity_it.second->active)
                            {
                                change_status = false;
                                break;
                            }
                        }
                        if (change_status)
                        {
                            for (const auto& entity_it : datareader->topic->data_writers)
                            {
                                if (entity_it.second->active)
                                {
                                    change_status = false;
                                    break;
                                }
                            }
                        }
                    }

                    if (change_status)
                    {
                        change_entity_status_of_kind(datareader->topic->id, active, datareader->topic->kind,
                                datareader->participant->domain->id);
                    }
                }
            }

            break;
        }
        default:
        {
            break;
        }
    }
}

void Database::change_entity_status(
        const EntityId& entity_id,
        bool active)
{
    EntityKind entity_kind = get_entity_kind(entity_id);

    // Check that the entity is a discovered/undiscovered dds_entity or a started/stopped monitor
    assert(
        entity_kind == EntityKind::PARTICIPANT || entity_kind == EntityKind::DATAWRITER ||
        entity_kind == EntityKind::DATAREADER || entity_kind == EntityKind::DOMAIN);

    std::lock_guard<std::shared_timed_mutex> guard(mutex_);
    change_entity_status_of_kind(entity_id, active, entity_kind);
}

template<typename Functor>
void Database::execute_without_lock(
        const Functor& lambda) noexcept
{
    bool is_locked = !mutex_.try_lock();
    mutex_.unlock();
    lambda();
    if (is_locked)
    {
        mutex_.lock();
    }
}

void Database::clear_internal_references_nts_()
{
    // Check parent classes for child that could have left
    // TODO: check if locators must be also removed

    /////
    // Physical

    // Processes
    for (auto& it : processes_)
    {
        clear_inactive_entities_from_map_(it.second->participants);
    }

    // Users
    for (auto& it : users_)
    {
        clear_inactive_entities_from_map_(it.second->processes);
    }

    // Hosts
    for (auto& it : hosts_)
    {
        clear_inactive_entities_from_map_(it.second->users);
    }

    /////
    // DDS

    // Participants
    for (auto& domain_it : participants_)
    {
        for (auto& it : domain_it.second)
        {
            clear_inactive_entities_from_map_(it.second->data_readers);
            clear_inactive_entities_from_map_(it.second->data_writers);
        }
    }

    /////
    // Logic

    // Topics
    for (auto& domain_it : topics_)
    {
        for (auto& it : domain_it.second)
        {
            clear_inactive_entities_from_map_(it.second->data_readers);
            clear_inactive_entities_from_map_(it.second->data_writers);
        }
    }

    // Domain
    for (auto& it : domains_)
    {
        clear_inactive_entities_from_map_(it.second->topics);
        clear_inactive_entities_from_map_(it.second->participants);
    }
}

/**
 * @brief Setter for entity alert.
 *
 * @param alert_info The new alert information.
 * @return The AlertId of the alert.
 */
AlertId Database::insert_alert(
        AlertInfo& alert_info)
{
    std::lock_guard<std::shared_timed_mutex> guard(mutex_);
    return insert_alert_nts(alert_info);
}

/**
 * @brief Setter for entity alert.
 *
 * @param alert_info The new alert information.
 * @return The AlertId of the alert.
 */
AlertId Database::insert_alert_nts(
        AlertInfo& alert_info)
{
    // store alert_info in the database
    AlertId id = next_alert_id_++;
    alert_info.set_id(id);
    alerts_.emplace(id, std::make_shared<AlertInfo>(alert_info));
    return id;
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
