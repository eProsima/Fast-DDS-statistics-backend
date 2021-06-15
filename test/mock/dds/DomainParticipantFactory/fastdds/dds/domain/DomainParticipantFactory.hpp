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
 * @file DomainParticipantFactory.hpp
 *
 */

#ifndef _FASTDDS_DOMAINPARTICIPANTFACTORY_HPP_
#define _FASTDDS_DOMAINPARTICIPANTFACTORY_HPP_

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastrtps/types/TypesBase.h>

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipantListener;
class DomainParticipant;

class DomainParticipantFactory
{

public:

    static DomainParticipantFactory* get_instance()
    {
        static DomainParticipantFactory instance;
        return &instance;
    }

    MOCK_METHOD4(
        create_participant,
        DomainParticipant *
        (
            DomainId_t domain_id,
            const DomainParticipantQos& qos,
            DomainParticipantListener * listener,
            const StatusMask& mask
        ));

    MOCK_METHOD1(
        delete_participant,
        void
        (
            DomainParticipant * participant
        ));

    MOCK_CONST_METHOD0(
        get_default_participant_qos,
        DomainParticipantQos & ());

};


} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_DOMAINPARTICIPANTFACTORY_HPP_*/
