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

#include <gtest_aux.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/Guid.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class TopicDescription;
class TopicListener;
class Topic;
class PublisherListener;
class Publisher;
class SubscriberListener;
class Subscriber;

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
        return RETCODE_OK;
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

    const fastdds::rtps::GUID_t& guid()
    {
        return guid_;
    }

    MOCK_METHOD5(
        create_topic,
        Topic *
        (
            const std::string& topic_name,
            const std::string& type_name,
            const TopicQos& qos,
            TopicListener * listener,
            const StatusMask& mask
        ));

    MOCK_METHOD4(
        create_topic,
        Topic *
        (
            const std::string& topic_name,
            const std::string& type_name,
            const TopicQos& qos,
            TopicListener * listener
        ));

    MOCK_METHOD3(
        create_topic,
        Topic *
        (
            const std::string& topic_name,
            const std::string& type_name,
            const TopicQos& qos
        ));

    MOCK_METHOD1(
        delete_topic,
        void
        (
            Topic * topic
        ));

    MOCK_METHOD3(
        create_publisher,
        Publisher *
        (
            const PublisherQos& qos,
            PublisherListener * listener,
            const StatusMask& mask
        ));

    MOCK_METHOD1(
        delete_publisher,
        void
        (
            Publisher * publisher
        ));

    MOCK_METHOD3(
        create_subscriber,
        Subscriber *
        (
            const SubscriberQos& qos,
            SubscriberListener * listener,
            const StatusMask& mask
        ));

    MOCK_METHOD1(
        delete_subscriber,
        void
        (
            Subscriber * subscriber
        ));

    MOCK_CONST_METHOD0(
        get_default_topic_qos,
        TopicQos & ());

    MOCK_CONST_METHOD0(
        get_default_subscriber_qos,
        SubscriberQos & ());

    MOCK_CONST_METHOD0(
        get_default_publisher_qos,
        PublisherQos & ());

    MOCK_CONST_METHOD1(
        lookup_topicdescription,
        TopicDescription *
        (
            const std::string& topic_name
        ));

    MOCK_METHOD2(
        register_type,
        ReturnCode_t
        (
            TypeSupport type,
            const std::string& type_name
        ));

    MOCK_METHOD0(
        enable,
        ReturnCode_t
            ());

    DomainParticipantQos qos_;
    DomainId_t domain_id_;
    eprosima::fastdds::rtps::GUID_t guid_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif /* _FASTDDS_DOMAIN_PARTICIPANT_HPP_ */
