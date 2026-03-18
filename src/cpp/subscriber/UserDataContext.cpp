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
 * @file UserDataContext.cpp
 */

#include "UserDataContext.hpp"

#include <map>
#include <mutex>
#include <string>

#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

namespace eprosima {
namespace statistics_backend {
namespace subscriber {

using namespace eprosima::fastdds::dds;

void UserDataContext::register_user_data_topic(
        const std::string& topic_name,
        fastdds::dds::DynamicType::_ref_type type)
{
    std::lock_guard<std::mutex> lock(mutex_);
    register_user_data_topic_nts(topic_name, type);
}

fastdds::dds::DynamicType::_ref_type UserDataContext::get_type_from_topic_name(
        const std::string& topic_name)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return get_type_from_topic_name_nts(topic_name);
}

void UserDataContext::register_user_data_topic_nts(
        const std::string& topic_name,
        fastdds::dds::DynamicType::_ref_type type)
{
    if (topic_name.empty())
    {
        EPROSIMA_LOG_ERROR(USER_DATA_CONTEXT,
                "Cannot add topic with empty name");
        return;
    }

    if (!type)
    {
        return;
    }

    std::string type_name = type->get_name().to_string();

    // Check if topic is already present
    auto it = discovered_topics_.find(topic_name);
    if (it != discovered_topics_.end())
    {
        if (discovered_user_data_types_.at(it->second) != type)
        {
            EPROSIMA_LOG_WARNING(USER_DATA_CONTEXT,
                    "Type with name '" << topic_name << "' already exists with different value. Ignoring insertion...");
        }

        return;
    }

    discovered_topics_[topic_name] = type_name;
    discovered_user_data_types_[type_name] = type;
}

fastdds::dds::DynamicType::_ref_type UserDataContext::get_type_from_topic_name_nts(
        const std::string& topic_name)
{
    if (topic_name.empty())
    {
        EPROSIMA_LOG_ERROR(USER_DATA_CONTEXT,
                "Topic name cannot be empty");
        return nullptr;
    }

    // Check if "topic_name" exists
    auto it = discovered_topics_.find(topic_name);
    if (it != discovered_topics_.end())
    {
        return discovered_user_data_types_.at(it->second);
    }

    EPROSIMA_LOG_WARNING(USER_DATA_CONTEXT,
            "Topic with name '" << topic_name << "' not found");

    return nullptr;
}

void UserDataContext::register_qos_for_spy_reader(
        const fastdds::dds::PublicationBuiltinTopicData& info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    register_qos_for_spy_reader_nts(info);
}

fastdds::dds::DataReaderQos UserDataContext::get_spy_reader_qos(
        const std::string& topic_name)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = spy_reader_qos_per_topic_.find(topic_name);
    if (it != spy_reader_qos_per_topic_.end())
    {
        return it->second;
    }

    // If not found, default to DATAREADER_QOS_DEFAULT.
    return fastdds::dds::DATAREADER_QOS_DEFAULT;
}

void UserDataContext::register_qos_for_spy_reader_nts(
        const fastdds::dds::PublicationBuiltinTopicData& info)
{
    std::string topic_name = info.topic_name.to_string();

    if (topic_name.empty())
    {
        return;
    }

    // Only store QoS for the first discovered writer on each topic
    if (spy_reader_qos_per_topic_.count(topic_name) > 0)
    {
        return;
    }

    // Copy only QoS that may affect matching
    fastdds::dds::DataReaderQos reader_qos;
    reader_qos.reliability(info.reliability);
    reader_qos.durability(info.durability);
    reader_qos.deadline(info.deadline);
    reader_qos.latency_budget(info.latency_budget);
    reader_qos.liveliness(info.liveliness);
    reader_qos.ownership(info.ownership);
    reader_qos.destination_order(info.destination_order);
    if (info.history.has_value())
    {
        reader_qos.history(info.history.value());
    }

    spy_reader_qos_per_topic_[topic_name] = reader_qos;
}

} // namespace subscriber
} // namespace statistics_backend
} // namespace eprosima
