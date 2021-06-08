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
 * @file Publisher.hpp
 *
 */

#ifndef _FASTDDS_PUBLISHER_HPP_
#define _FASTDDS_PUBLISHER_HPP_

#include "gtest/gtest.h"
#include <gmock/gmock.h>

#include <fastrtps/types/TypesBase.h>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/core/status/StatusMask.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

class DataWriterListener;
class DataWriter;
class Topic;

class Publisher
{

public:

    MOCK_METHOD4(
        create_datawriter,
        DataWriter *
        (
            Topic * topic,
            const DataWriterQos& qos,
            DataWriterListener * listener,
            const StatusMask& mask
        ));

    MOCK_CONST_METHOD0(
        get_default_datawriter_qos,
        DataWriterQos & ());
};


} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_PUBLISHER_HPP_*/