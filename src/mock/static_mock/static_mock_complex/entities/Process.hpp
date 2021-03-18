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
 * @file Process.hpp
 */

#include "Entity.hpp"

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATICMOCKCOMPLEX_PROCESS_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATICMOCKCOMPLEX_PROCESS_HPP_

namespace eprosima {
namespace statistics_backend {

class User;
class Participant;

class Process : public Entity
{
public:

    std::vector<EntityId> get_entities(
        EntityKind entity_type) const override;

    void add_participant(Participant* participant);

    void user(User* user);

private:
    std::map<EntityId, Participant *> participants_;
    User* user;
};

} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATICMOCKCOMPLEX_PROCESS_HPP_
