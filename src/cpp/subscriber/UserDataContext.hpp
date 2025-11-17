/* Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file UserDataContext.hpp
 */

#ifndef FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__USER_DATA_CONTEXT_HPP
#define FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__USER_DATA_CONTEXT_HPP

#include <map>
#include <mutex>
#include <string>

#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>

namespace eprosima {
namespace statistics_backend {
namespace subscriber {

class UserDataContext
{

public:

    void register_user_data_topic(
            const std::string& topic_name,
            fastdds::dds::DynamicType::_ref_type type);

    fastdds::dds::DynamicType::_ref_type get_type_from_topic_name(
            const std::string& topic_name);

    fastdds::dds::DynamicType::_ref_type get_type_from_type_name(
            const std::string& type_name);

protected:

    void register_user_data_topic_nts(
            const std::string& topic_name,
            fastdds::dds::DynamicType::_ref_type type);

    fastdds::dds::DynamicType::_ref_type get_type_from_topic_name_nts(
            const std::string& topic_name);

    fastdds::dds::DynamicType::_ref_type get_type_from_type_name_nts(
            const std::string& type_name);

    std::mutex mutex_;
    std::map<std::string, fastdds::dds::DynamicType::_ref_type> discovered_user_data_types_;
    std::map<std::string, std::string> discovered_topics_;

};

} // namespace subscriber
} // namespace statistics_backend
} // namespace eprosima

#endif // FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__USER_DATA_CONTEXT_HPP
