/* Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <set>

#include "entities.hpp"

namespace eprosima {
namespace statistics_backend {
namespace database {

bool Entity::is_metatraffic_topic(
        std::string topic_name)
{
    bool is_metatraffic = false;
    std::set<std::string> metatraffic_topics_keywords = {
        "___EPROSIMA___METATRAFFIC___",
        "ros_discovery_info",
        "rosout",
        "parameter_events",
        "get_parameters",
        "set_parameters",
        "get_parameter_types",
        "set_parameters_atomically",
        "describe_parameters",
        "list_parameters",
        "_fastdds_statistics_",
        "_fastdds_monitor_service_"};

    std::set<std::string>::iterator it = metatraffic_topics_keywords.begin();
    size_t found;
    while (it != metatraffic_topics_keywords.end())
    {
        found = topic_name.rfind(*it);
        if (found != std::string::npos)
        {
            is_metatraffic = true;
            break;
        }
        it++;
    }
    return is_metatraffic;
}

DdsVendor DDSEntity::dds_vendor_by_guid(
        const std::string& guid)
{
    // GUID vendor prefixes
    const std::map<std::string, DdsVendor> dds_vendor_prefixes = {
        {"01.0f", DdsVendor::FASTDDS},
        {"01.15", DdsVendor::SAFEDDS}
    };

    std::string prefix = guid.substr(0, 5); // Assuming the prefix is the first 5 characters

    auto it = dds_vendor_prefixes.find(prefix);
    if (it != dds_vendor_prefixes.end())
    {
        return it->second;
    }
    return DdsVendor::UNKNOWN;
}

DDSEndpoint::DDSEndpoint(
        EntityKind entity_kind, /* EntityKind::INVALID */
        std::string endpoint_name, /* "INVALID" */
        Qos endpoint_qos, /* {} */
        std::string endpoint_guid, /* "|GUID UNKNOWN|" */
        details::fragile_ptr<DomainParticipant> endpoint_participant, /* nullptr */
        details::fragile_ptr<Topic> endpoint_topic, /* nullptr */
        StatusLevel status, /* StatusLevel::OK_STATUS */
        AppId endpoint_app_id, /* AppId::UNKNOWN */
        std::string endpoint_app_metadata /* "" */) noexcept
    : DDSEntity(entity_kind, endpoint_name, endpoint_qos, endpoint_guid, status, endpoint_app_id, endpoint_app_metadata)
    , participant(endpoint_participant)
    , topic(endpoint_topic)
{
    if (topic != nullptr)
    {
        metatraffic = topic->metatraffic;
    }
}

void Host::clear()
{
    users.clear();
}

void User::clear()
{
    processes.clear();
}

void Process::clear()
{
    participants.clear();
}

void Domain::clear()
{
    topics.clear();
    participants.clear();
}

void DomainParticipant::clear()
{
    data_readers.clear();
    data_writers.clear();
    data.clear();
}

void Topic::clear()
{
    data_readers.clear();
    data_writers.clear();
}

void DataReader::clear()
{
    data.clear();
}

void DataWriter::clear()
{
    data.clear();
}

void Locator::clear()
{
    data_readers.clear();
    data_writers.clear();
}

template<>
std::map<EntityId, details::fragile_ptr<DataReader>>& DomainParticipant::ddsendpoints<DataReader>()
{
    return data_readers;
}

template<>
std::map<EntityId, details::fragile_ptr<DataWriter>>& DomainParticipant::ddsendpoints<DataWriter>()
{
    return data_writers;
}

template<>
std::map<EntityId, details::fragile_ptr<DataReader>>& Topic::ddsendpoints<DataReader>()
{
    return data_readers;
}

template<>
std::map<EntityId, details::fragile_ptr<DataWriter>>& Topic::ddsendpoints<DataWriter>()
{
    return data_writers;
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
