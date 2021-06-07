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
 * @file Topic.hpp
 *
 */

#ifndef _FASTDDS_DDS_TOPIC_HPP_
#define _FASTDDS_DDS_TOPIC_HPP_

#include <fastrtps/types/TypesBase.h>
#include <fastdds/dds/topic/TopicDescription.hpp>

#include <topic_types/types.h>


namespace eprosima {
namespace fastdds {
namespace dds {


class Topic : public TopicDescription
{
public:

    Topic(
            const std::string& topic_name,
            const std::string& type_name)
        : TopicDescription(topic_name, type_name)
    {
    }

    Topic() = default;

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_DDS_TOPIC_HPP_*/
