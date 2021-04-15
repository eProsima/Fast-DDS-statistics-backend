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

#include <fastdds-statistics-backend/exception/Exception.hpp>
#include <fastdds-statistics-backend/types/types.hpp>

#include <mutex>  // For std::unique_lock
#include <iostream>

namespace eprosima {
namespace statistics_backend {
namespace database {

EntityId Database::insert(
        const std::shared_ptr<Entity>& entity)
{
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);
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

            /* Check that this is indeed a new host */
            for (auto host_it: hosts_)
            {
                if (host.get() == host_it.second.get())
                {
                    throw BadParameter("Host already exists in the database");
                }
            }

            /* Insert host in the database */
            host->users.clear();
            host->id = generate_entity_id();
            hosts_[host->id] = host;
            return host->id;
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
            for (auto user_it: users_)
            {
                if (user.get() == user_it.second.get())
                {
                    throw BadParameter("User already exists in the database");
                }
            }

            /* Check that host exits */
            bool host_exists = false;
            for (auto host_it : hosts_)
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
            for (auto user_it : user->host->users)
            {
                if (user->name == user_it.second->name)
                {
                    throw BadParameter(
                              "Another User with name '" + user->name
                              + "' already exists in parent host collection");
                }
            }

            /* Add user to users collection */
            user->processes.clear();
            user->id = generate_entity_id();
            users_[user->id] = user;

            /* Add user to host's users collection */
            user->host->users[user->id] = user;
            return user->id;
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
            for (auto process_it: processes_)
            {
                if (process.get() == process_it.second.get())
                {
                    throw BadParameter("Process already exists in the database");
                }
            }

            /* Check that user exits */
            bool user_exists = false;
            for (auto user_it : users_)
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

            /* Check that a process with the same pid does not exist in the user's collection */
            for (auto process_it : process->user->processes)
            {
                if (process->pid == process_it.second->pid)
                {
                    throw BadParameter(
                              "Another process with PID '" + process->pid
                              + "' already exists in parent user collection");
                }
            }

            /* Add process to processes collection */
            process->participants.clear();
            process->id = generate_entity_id();
            processes_[process->id] = process;

            /* Add process to user's processes collection */
            process->user->processes[process->id] = process;
            return process->id;
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
            for (auto domain_it: domains_)
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

            /* Insert domain in the database */
            domain->topics.clear();
            domain->participants.clear();
            domain->id = generate_entity_id();
            domains_[domain->id] = domain;
            return domain->id;
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

            /* Check that domain exits */
            bool domain_exists = false;
            for (auto domain_it : domains_)
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

            /* Check that this is indeed a new topic and that its name is unique in the domain */
            for (auto topic_it: topics_[topic->domain->id])
            {
                if (topic.get() == topic_it.second.get())
                {
                    throw BadParameter("Topic already exists in the database");
                }
                if (topic->name == topic_it.second->name)
                {
                    throw BadParameter(
                              "A topic with name '" + topic->name +
                              "' already exists in the database for the same domain");
                }
            }

            /* Add topic to domain's collection */
            topic->data_readers.clear();
            topic->data_writers.clear();
            topic->id = generate_entity_id();
            domains_[topic->domain->id]->topics[topic->id] = topic;

            /* Insert topic in the database */
            topics_[topic->domain->id][topic->id] = topic;
            return topic->id;
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
            for (auto domain_it : domains_)
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
            for (auto domain_it : participants_)
            {
                for (auto participant_it : domain_it.second)
                {
                    // Check the participant in the domain
                    if (participant_it.second->domain->id == participant->domain->id)
                    {
                        if (participant.get() == participant_it.second.get())
                        {
                            throw BadParameter("Participant already exists in the database");
                        }
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

            /* Add participant to process' collection */
            participant->data_readers.clear();
            participant->data_writers.clear();
            participant->data.clear();
            participant->id = generate_entity_id();
            // participant->process->participants[participant->id] = participant;

            /* Add participant to domain's collection */
            participant->data_readers.clear();
            participant->data_writers.clear();
            participant->data.clear();
            participant->domain->participants[participant->id] = participant;

            // /* Add reader's locators to domains_by_process_ */
            // domains_by_process_[participant->process->id][participant->domain->id] = participant->domain;

            // /* Add reader's participant to processes_by_domain_ */
            // processes_by_domain_[participant->domain->id][participant->process->id] = participant->process;

            /* Insert participant in the database */
            participants_[participant->domain->id][participant->id] = participant;
            return participant->id;
        }
        case EntityKind::DATAREADER:
        {
            auto data_reader = std::static_pointer_cast<DataReader>(entity);
            return insert_ddsendpoint<DataReader>(data_reader);
        }
        case EntityKind::DATAWRITER:
        {
            auto data_writer = std::static_pointer_cast<DataWriter>(entity);
            return insert_ddsendpoint<DataWriter>(data_writer);
        }
        default:
        {
            break;
        }
    }
    return EntityId();
}

EntityId Database::generate_entity_id() noexcept
{
    return EntityId(next_id_++);
}

template<>
std::map<EntityId, std::map<EntityId, std::shared_ptr<DataReader>>>& Database::dds_endpoints<DataReader>()
{
    return datareaders_;
}

template<>
std::map<EntityId, std::map<EntityId, std::shared_ptr<DataWriter>>>& Database::dds_endpoints<DataWriter>()
{
    return datawriters_;
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
