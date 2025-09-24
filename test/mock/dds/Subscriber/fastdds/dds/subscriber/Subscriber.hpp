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
 * @file Subscriber.hpp
 *
 */

#ifndef _FASTDDS_SUBSCRIBER_HPP_
#define _FASTDDS_SUBSCRIBER_HPP_

#include <gtest_aux.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/rtps/history/IPayloadPool.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DataReaderListener;
class DataReader;
class TopicDescription;

class Subscriber
{

public:

    MOCK_METHOD5(
        create_datareader_mock_method,
        DataReader *
        (
            TopicDescription * topic,
            const DataReaderQos& qos,
            DataReaderListener * listener,
            const StatusMask& mask,
            std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool
        ));

    DataReader* create_datareader(
            TopicDescription* topic,
            const DataReaderQos& reader_qos,
            DataReaderListener* listener = nullptr,
            const StatusMask& mask = StatusMask::all(),
            std::shared_ptr<fastdds::rtps::IPayloadPool> payload_pool = nullptr)
    {
        return create_datareader_mock_method(
            topic,
            reader_qos,
            listener,
            mask,
            payload_pool
        );
    }

    MOCK_METHOD1(
        delete_datareader,
        void
        (
            DataReader * datareader
        ));

    MOCK_CONST_METHOD0(
        get_default_datareader_qos,
        DataReaderQos & ());

    MOCK_CONST_METHOD0(
        get_participant,
        DomainParticipant * ());
};


} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_SUBSCRIBER_HPP_*/
