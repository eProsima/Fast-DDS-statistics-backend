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
 * @file TopicDescription.hpp
 *
 */

#ifndef _FASTDDS_TOPIC_DESCRIPTION_HPP_
#define _FASTDDS_TOPIC_DESCRIPTION_HPP_

#include "gtest/gtest.h"
#include <gmock/gmock.h>

#include <string>

namespace eprosima {
namespace fastdds {
namespace dds {

class TopicDescription
{
public:

    TopicDescription(
            const std::string& topic_name,
            const std::string& type_name)
        : topic_name_(topic_name)
        , type_name_(type_name)
    {
    }

    TopicDescription() = default;

    virtual ~TopicDescription() = default;

    void set_name(
            const std::string& name)
    {
        topic_name_ = name;
    }

    void set_type_name(
            const std::string& name)
    {
        type_name_ = name;
    }

    const std::string get_name() const
    {
        return topic_name_;
    }

    const std::string get_type_name() const
    {
        return type_name_;
    }

    std::string topic_name_;
    std::string type_name_;
};

} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif /* _FASTDDS_TOPIC_DESCRIPTION_HPP_ */
