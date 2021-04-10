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

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
