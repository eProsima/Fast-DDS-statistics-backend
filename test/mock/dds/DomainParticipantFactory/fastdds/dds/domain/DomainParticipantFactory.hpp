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

#include <gtest_aux.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantExtendedQos.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include "TypeObjectRegistry.hpp"
namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipantListener;
class DomainParticipant;

/**
 * @brief Fake + Stub class to supplant DomainParticipantFactory from fastrtps.
 *
 * This is not a MOCK function as mocking singletons with gmock is very hard.
 * It is much easier to use a fake function, and check that its methods are called as many times as expected
 * in the test explicitly.
 *
 * Using this is even a recommendation from gtest:
 * "If you really intend to leak a mock, you can suppress this error using testing::Mock::AllowLeak(mock_object),
 * or you may use a fake or stub instead of a mock."
 *
 * Use \c domain_participant variable to force the returnment of \c create_participant() .
 * \c domain_participant variable is never set neither deleted from this class.
 */

class DomainParticipantFactory
{

public:

    static DomainParticipantFactory* get_instance()
    {
        return get_shared_instance().get();
    }

    static std::shared_ptr<DomainParticipantFactory> get_shared_instance()
    {
        static std::shared_ptr<DomainParticipantFactory> instance(new DomainParticipantFactory());
        return instance;
    }

    ~DomainParticipantFactory()
    {
        // Do nothing
    }

    ReturnCode_t load_profiles()
    {
        load_profiles_count++;
        return RETCODE_OK;
    }

    ReturnCode_t get_participant_extended_qos_from_profile(
            const std::string&,
            DomainParticipantExtendedQos&) const
    {
        get_participant_extended_qos_from_profile_count++;
        return RETCODE_OK;
    }

    ReturnCode_t load_XML_profiles_file(
            const std::string&)
    {
        load_XML_profiles_file_count++;
        return RETCODE_OK;
    }

    DomainParticipant* create_participant(
            DomainId_t,
            const DomainParticipantQos&,
            DomainParticipantListener*,
            const StatusMask& )
    {
        create_participant_count++;
        return domain_participant;
    }

    void delete_participant(
            DomainParticipant* )
    {
        delete_participant_count++;
    }

    const DomainParticipantQos& get_default_participant_qos()
    {
        get_default_participant_qos_count++;
        return participant_qos;
    }

    ReturnCode_t get_qos(
            DomainParticipantFactoryQos& qos) const
    {
        get_qos_count++;
        qos = factory_qos;
        return RETCODE_OK;
    }

    ReturnCode_t set_qos(
            const DomainParticipantFactoryQos& qos)
    {
        set_qos_count++;
        factory_qos = qos;
        return RETCODE_OK;
    }

    xtypes::ITypeObjectRegistry& type_object_registry()
    {
        return type_registry;
    }

    xtypes::TypeObjectRegistry type_registry;

    DomainParticipantFactoryQos factory_qos{};
    DomainParticipantQos participant_qos{};
    DomainParticipant* domain_participant = nullptr;

    mutable unsigned int load_profiles_count = 0;
    mutable unsigned int get_participant_extended_qos_from_profile_count = 0;
    mutable unsigned int load_XML_profiles_file_count = 0;
    mutable unsigned int create_participant_count = 0;
    mutable unsigned int delete_participant_count = 0;
    mutable unsigned int get_default_participant_qos_count = 0;
    mutable unsigned int get_qos_count = 0;
    mutable unsigned int set_qos_count = 0;

};


} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_DOMAINPARTICIPANTFACTORY_HPP_*/
