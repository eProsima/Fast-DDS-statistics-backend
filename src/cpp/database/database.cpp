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


namespace eprosima {
namespace statistics_backend {
namespace database {

EntityId Database::insert(
        const std::shared_ptr<Entity>& entity)
{
    switch (entity->kind)
    {
        case EntityKind::HOST:
        {
            std::shared_ptr<Host> host = std::static_pointer_cast<Host>(entity);

            /* Check that this is indeed a new host */
            for (auto host_it: hosts_)
            {
                if (host.get() == host_it.second.get())
                {
                   throw BadParameter("Host already exists in the database");
                }
            }

            /* Insert host in the database */
            host->id = generate_entity_id();
            hosts_[host->id] = host;
            return host->id;
            break;
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
            user->id = generate_entity_id();
            users_[user->id] = user;

            /* Add user to host's users collection */
            user->host->users[user->id] = user;
            return user->id;
            break;
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
