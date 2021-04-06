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
 * @file DDSEntity.hpp
 */

#include "../utils.hpp"
#include "Entity.hpp"

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_COMPLEXMOCK_DDSENTITY_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_COMPLEXMOCK_DDSENTITY_HPP_

namespace eprosima {
namespace statistics_backend {

#define DEFAULT_GUID "00::0X"

class DDSEntity : public Entity
{
    DDSEntity(
        EntityId id,
        EntityKind kind,
        std::string name,
        std::string guid)
        : Entity(id, kind, name)
        , guid_(DEFAULT_GUID)
        , qos_(random_qos(id))
    {
    }

    Info get_info() const override
    {
        Info info = Entity::get_info();
        info["guid"] = guid();
        info["QoS"] = qos();

        return info;
    }

    std::string guid() const
    {
        return guid_;
    }

    void guid(const std::string guid)
    {
        guid_ = guid;
    }

    Info qos() const
    {
        return qos_;
    }

private:
    std::string guid_;
    Info qos_;
};

} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_COMPLEXMOCK_DDSENTITY_HPP_
