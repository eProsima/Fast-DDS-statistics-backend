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

/**
 * Class to act as a communication bridge between the StatisticsParticipantListener and
 * the UserDataReaderListener in order to share the discovered user data types.
 * It stores the discovered types associated with their topic names.
 */
class UserDataContext
{

public:

    /**
     * Register a user data topic type associated with its topic name.
     * @param topic_name Topic name.
     * @param type Dynamic type associated with the topic.
     */
    void register_user_data_topic(
            const std::string& topic_name,
            fastdds::dds::DynamicType::_ref_type type);
    /**
     * Get the dynamic type associated with a topic name.
     * @param topic_name Topic name.
     * @return Dynamic type associated with the topic name.
     */
    fastdds::dds::DynamicType::_ref_type get_type_from_topic_name(
            const std::string& topic_name);

    /**
     * Get the dynamic type associated with a type name.
     * @param type_name Type name.
     * @return Dynamic type associated with the type name.
     */
    fastdds::dds::DynamicType::_ref_type get_type_from_type_name(
            const std::string& type_name);

protected:

    /**
     * Register a user data topic type associated with its topic name. Non thread-safe version.
     * @param topic_name Topic name.
     * @param type Dynamic type associated with the topic.
     */
    void register_user_data_topic_nts(
            const std::string& topic_name,
            fastdds::dds::DynamicType::_ref_type type);

    /**
     * Get the dynamic type associated with a topic name. Non thread-safe version.
     * @param topic_name Topic name.
     * @return Dynamic type associated with the topic name.
     */
    fastdds::dds::DynamicType::_ref_type get_type_from_topic_name_nts(
            const std::string& topic_name);

    /**
     * Get the dynamic type associated with a type name. Non thread-safe version.
     * @param type_name Type name.
     * @return Dynamic type associated with the type name.
     */
    fastdds::dds::DynamicType::_ref_type get_type_from_type_name_nts(
            const std::string& type_name);

    // Mutex to protect access to the maps
    std::mutex mutex_;
    // Map of type name to dynamic type
    std::map<std::string, fastdds::dds::DynamicType::_ref_type> discovered_user_data_types_;
    // Map of topic name to type name
    std::map<std::string, std::string> discovered_topics_;
};

} // namespace subscriber
} // namespace statistics_backend
} // namespace eprosima

#endif // FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__USER_DATA_CONTEXT_HPP
