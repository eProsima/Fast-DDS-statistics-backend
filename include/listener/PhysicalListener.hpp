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
 * @file PhysicalListener.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_LISTENER_PHYSICALLISTENER_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_LISTENER_PHYSICALLISTENER_HPP_

#include <types/types.hpp>
#include <listener/DomainListener.hpp>

namespace eprosima {
namespace statistics_backend {

class PhysicalListener : public DomainListener
{
public:

    /**
     * @brief Virtual destructor
     */
    virtual ~PhysicalListener() = default;

    /*!
     * This function is called when a new Host is discovered by the library.
     *
     * @param participant_id Entity ID of the participant that discovered the Host.
     * @param host_id Entity ID of the discovered Host.
     * @param status The status of the discovered Host.
     */
    virtual void on_host_discovery(
            EntityId participant_id,
            EntityId host_id,
            const Status& status)
    {
        static_cast<void>(participant_id);
        static_cast<void>(host_id);
        static_cast<void>(status);
    }

    /*!
     * This function is called when a new User is discovered by the library.
     *
     * @param participant_id Entity ID of the participant that discovered the User.
     * @param user_id Entity ID of the discovered User.
     * @param status The status of the discovered User.
     */
    virtual void on_user_discovery(
            EntityId participant_id,
            EntityId user_id,
            const Status& status)
    {
        static_cast<void>(participant_id);
        static_cast<void>(user_id);
        static_cast<void>(status);
    }

    /*!
     * This function is called when a new Process is discovered by the library.
     *
     * @param participant_id Entity ID of the participant that discovered the Process.
     * @param process_id Entity ID of the discovered Process.
     * @param status The status of the discovered Process.
     */
    virtual void on_process_discovery(
            EntityId participant_id,
            EntityId process_id,
            const Status& status)
    {
        static_cast<void>(participant_id);
        static_cast<void>(process_id);
        static_cast<void>(status);
    }

    /*!
     * This function is called when a new Locator is discovered by the library.
     *
     * @param participant_id Entity ID of the participant that discovered the Locator.
     * @param locator_id Entity ID of the discovered Locator.
     * @param status The status of the discovered Locator.
     */
    virtual void on_locator_discovery(
            EntityId participant_id,
            EntityId locator_id,
            const Status& status)
    {
        static_cast<void>(participant_id);
        static_cast<void>(locator_id);
        static_cast<void>(status);
    }

};

} // namespace statistics_backend
} // namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_LISTENER_PHYSICALLISTENER_HPP_
