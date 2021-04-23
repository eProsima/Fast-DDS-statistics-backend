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
 * @file DomainParticipant.hpp
 *
 */

#ifndef _FASTDDS_DOMAIN_PARTICIPANT_HPP_
#define _FASTDDS_DOMAIN_PARTICIPANT_HPP_

#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/rtps/common/Guid.h>
#include <fastrtps/types/TypesBase.h>


using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace dds {

/**
 * Mock for Class DomainParticipant.
 * Only methods used on the tests are mocked
 */
class DomainParticipant
{
public:

    /**
     * This operation returns the value of the DomainParticipant QoS policies
     * @param qos DomainParticipantQos reference where the qos is going to be returned
     * @return RETCODE_OK
     */
    ReturnCode_t get_qos(
            DomainParticipantQos& qos) const
    {
        qos = qos_;
        return ReturnCode_t::RETCODE_OK;
    }

    /**
     * @brief This operation returns the value of the DomainParticipant QoS policies
     * @return A reference to the DomainParticipantQos
     */
    const DomainParticipantQos& get_qos() const
    {
        return qos_;
    }

    /**
     * This operation retrieves the domain_id used to create the DomainParticipant.
     * The domain_id identifies the DDS domain to which the DomainParticipant belongs.
     * @return The Participant's domain_id
     */
    DomainId_t get_domain_id() const
    {
        return domain_id_;
    }

    DomainParticipantQos qos_;
    DomainId_t domain_id_;

};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif /* _FASTDDS_DOMAIN_PARTICIPANT_HPP_ */
