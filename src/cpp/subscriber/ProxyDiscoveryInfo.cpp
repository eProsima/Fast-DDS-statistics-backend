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
 * @file ProxyDiscoveryInfo.cpp
 */
#include <utility>

#include "subscriber/ProxyDiscoveryInfo.hpp"
#include "subscriber/QosSerializer.hpp"
#include "subscriber/StatisticsParticipantListener.hpp"
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/utils.hpp>
#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace statistics_backend {
namespace subscriber {

using Locator_t = fastdds::rtps::Locator_t;
using IPLocator = fastdds::rtps::IPLocator;
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

// Search for an address different from localhost in the locator list
bool search_address_in_locators(
        const eprosima::fastdds::ResourceLimitedVector<Locator_t>& locators,
        std::string& address)
{
    for (auto locator : locators)
    {
        // if the address is not localhost
        if (!IPLocator::isLocal(locator))
        {
            // Convert the locator to an address with IP format
            address =  IPLocator::ip_to_string(locator);
            return true;
        }
    }
    return false;
}

// Return a IP obtained from participant locators
std::string get_address(
        const fastdds::rtps::ParticipantBuiltinTopicData& info)
{
    // The IP is obtained from the announced locators
    // Search for a locator with an IP different from localhost
    std::string address;

    // 1. default_locators.unicast
    if (search_address_in_locators(info.default_locators.unicast, address))
    {
        return address;
    }

    // 2. metatraffic_locators.unicast
    if (search_address_in_locators(info.metatraffic_locators.unicast, address))
    {
        return address;
    }

    // 3. default_locators.multicast
    if (search_address_in_locators(info.default_locators.multicast, address))
    {
        return address;
    }

    // 4. metatraffic_locators.multicast
    if (search_address_in_locators(info.metatraffic_locators.multicast, address))
    {
        return address;
    }

    // The only option is for localhost to be the only valid IP
    return "localhost";
}

EntityDiscoveryInfo get_discovery_info(
        const EntityId& domain_of_discoverer,
        const fastdds::rtps::ParticipantBuiltinTopicData& participant_data,
        const fastdds::rtps::ParticipantDiscoveryStatus& reason,
        const DiscoverySource& discovery_source
    )
{
    // Meaningful prefix for metatraffic entities
    const std::string metatraffic_prefix = "___EPROSIMA___METATRAFFIC___DOMAIN_" + std::to_string(domain_of_discoverer.value()) +
    "___";
    const std::string metatraffic_alias = "_metatraffic_";

    EntityDiscoveryInfo discovery_info(EntityKind::PARTICIPANT);
    // domain_id is set to the discoverer domain in order to avoid issues when registering
    // the entity in the database. The entity original domain id is passed through
    // the original_domain_id field
    discovery_info.domain_id = domain_of_discoverer;
    discovery_info.original_domain_id = participant_data.domain_id;
    discovery_info.guid = participant_data.guid;
    discovery_info.qos = subscriber::participant_proxy_data_to_backend_qos(participant_data);
    discovery_info.discovery_source = discovery_source;
    discovery_info.participant_guid = participant_data.guid;
    discovery_info.address = get_address(participant_data);
    discovery_info.participant_name = participant_data.participant_name.to_string();
    discovery_info.entity_status = StatusLevel::OK_STATUS;

    switch (reason)
    {
        case ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT:
        {
            EPROSIMA_LOG_INFO(StatisticsParticipantListener, "DomainParticipant discovered: " << info.guid);
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
            break;
        }
        case ParticipantDiscoveryStatus::CHANGED_QOS_PARTICIPANT:
        {
            // TODO [ILG] : Process these messages and save the updated QoS
            EPROSIMA_LOG_INFO(StatisticsParticipantListener, "DomainParticipant updated: " << info.guid);
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UPDATE;
            break;
        }
        case ParticipantDiscoveryStatus::REMOVED_PARTICIPANT:
        case ParticipantDiscoveryStatus::DROPPED_PARTICIPANT:
        case ParticipantDiscoveryStatus::IGNORED_PARTICIPANT:
        {
            EPROSIMA_LOG_INFO(StatisticsParticipantListener, "DomainParticipant removed: " << info.guid);
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY;
            break;
        }
    }

    // Get data from participant discovery info
    auto get_property_value =
            [](const fastdds::dds::ParameterPropertyList_t& properties, const std::string& property_name) -> std::string
            {
                auto property = std::find_if(
                    properties.begin(),
                    properties.end(),
                    [&](const fastdds::dds::ParameterProperty_t& property)
                    {
                        return property.first() == property_name;
                    });
                if (property != properties.end())
                {
                    return property->second();
                }
                return std::string("");
            };

    discovery_info.host = get_property_value(participant_data.properties,
                    eprosima::fastdds::dds::parameter_policy_physical_data_host);
    discovery_info.host = discovery_info.host.empty()? "Unknown" : discovery_info.host;

    discovery_info.user = get_property_value(participant_data.properties,
                    eprosima::fastdds::dds::parameter_policy_physical_data_user);
    discovery_info.user = discovery_info.user.empty()? "Unknown" : discovery_info.user;

    discovery_info.process = get_property_value(participant_data.properties,
                    eprosima::fastdds::dds::parameter_policy_physical_data_process);
    discovery_info.process = discovery_info.process.empty()? "Unknown" : discovery_info.process;

    std::string app_id = get_property_value(participant_data.properties, "fastdds.application.id");
    auto it = app_id_enum.find(app_id);
    discovery_info.app_id = it != app_id_enum.end()? it->second : AppId::UNKNOWN;

    discovery_info.app_metadata = get_property_value(participant_data.properties, "fastdds.application.metadata");

    return discovery_info;
}

EntityDiscoveryInfo get_discovery_info(
        const EntityId& domain_of_discoverer,
        const fastdds::rtps::SubscriptionBuiltinTopicData& reader_data,
        const fastdds::rtps::ReaderDiscoveryStatus& reason,
        const DiscoverySource&  discovery_source
    )
{
    EntityDiscoveryInfo discovery_info(EntityKind::DATAREADER);

    // domain_id is set to the discoverer domain in order to avoid issues when registering
    // the entity in the database. The entity original domain id is passed through
    // the original_domain_id field
    discovery_info.domain_id = domain_of_discoverer;
    // TODO: Somehow get original domain id
    // discovery_info.original_domain_id = <something>;
    discovery_info.guid = reader_data.guid;
    discovery_info.qos = subscriber::reader_proxy_data_to_backend_qos(reader_data);

    discovery_info.topic_name = reader_data.topic_name.to_string();
    discovery_info.type_name = reader_data.type_name.to_string();
    discovery_info.locators = reader_data.remote_locators;
    discovery_info.discovery_source = discovery_source;
    discovery_info.participant_guid = reader_data.participant_guid;

    switch (reason)
    {
        case ReaderDiscoveryStatus::DISCOVERED_READER:
        {
            EPROSIMA_LOG_INFO(StatisticsParticipantListener, "DataReader discovered: " << info.guid);
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
            break;
        }
        case ReaderDiscoveryStatus::CHANGED_QOS_READER:
        {
            // TODO [ILG] : Process these messages and save the updated QoS
            EPROSIMA_LOG_INFO(StatisticsParticipantListener, "DataReader updated: " << info.guid);
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UPDATE;
            break;
        }
        case ReaderDiscoveryStatus::REMOVED_READER:
        case ReaderDiscoveryStatus::IGNORED_READER:
        {
            EPROSIMA_LOG_INFO(StatisticsParticipantListener, "DataReader removed: " << info.guid);
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY;
            break;
        }
    }

    // In case of a new data reader discovered, add type info if available
    if (ReaderDiscoveryStatus::DISCOVERED_READER == reason &&reader_data.type_information.assigned() == true)
    {
        // Create IDL representation of the discovered type
        // Get remote type information
        xtypes::TypeObject remote_type_object;
        if (RETCODE_OK != DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                    reader_data.type_information.type_information.complete().typeid_with_size().type_id(),
                    remote_type_object))
        {
            EPROSIMA_LOG_ERROR(STATISTICS_PARTICIPANT_LISTENER,
                    "Error getting type object for type " << reader_data.type_name);
            return discovery_info;
        }

        // Build remotely discovered type
        DynamicType::_ref_type remote_type = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
            remote_type_object)->build();

        // Serialize DynamicType into its IDL representation
        std::stringstream idl;
        idl_serialize(remote_type, idl);
        discovery_info.type_idl = idl.str();
    }

    return discovery_info;
}

EntityDiscoveryInfo get_discovery_info(
        const EntityId& domain_of_discoverer,
        const fastdds::rtps::PublicationBuiltinTopicData& writer_data,
        const fastdds::rtps::WriterDiscoveryStatus& reason,
        const DiscoverySource& discovery_source
    )
{
    EntityDiscoveryInfo discovery_info(EntityKind::DATAWRITER);

    // domain_id is set to the discoverer domain in order to avoid issues when registering
    // the entity in the database. The entity original domain id is passed through
    // the original_domain_id field
    discovery_info.domain_id = domain_of_discoverer;
    // TODO: Somehow get original domain id
    // discovery_info.original_domain_id = <something>;
    discovery_info.guid = writer_data.guid;
    discovery_info.qos = subscriber::writer_proxy_data_to_backend_qos(writer_data);

    discovery_info.topic_name = writer_data.topic_name.to_string();
    discovery_info.type_name = writer_data.type_name.to_string();
    discovery_info.locators = writer_data.remote_locators;
    discovery_info.discovery_source = discovery_source;
    discovery_info.participant_guid = writer_data.participant_guid;

    switch (reason)
    {
        case WriterDiscoveryStatus::DISCOVERED_WRITER:
        {
            EPROSIMA_LOG_INFO(StatisticsParticipantListener, "DataWriter discovered: " << info.guid);
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
            break;
        }
        case WriterDiscoveryStatus::CHANGED_QOS_WRITER:
        {
            // TODO [ILG] : Process these messages and save the updated QoS
            EPROSIMA_LOG_INFO(StatisticsParticipantListener, "DataWriter updated: " << info.guid);
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UPDATE;
            break;
        }
        case WriterDiscoveryStatus::REMOVED_WRITER:
        case WriterDiscoveryStatus::IGNORED_WRITER:
        {
            EPROSIMA_LOG_INFO(StatisticsParticipantListener, "DataWriter removed: " << info.guid);
            discovery_info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY;
            break;
        }
    }

    // In case of a new data writer discovered, add type info if available
    if (WriterDiscoveryStatus::DISCOVERED_WRITER == reason && writer_data.type_information.assigned() == true)
    {
        // Create IDL representation of the discovered type
        // Get remote type information
        xtypes::TypeObject remote_type_object;
        if (RETCODE_OK != DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                    writer_data.type_information.type_information.complete().typeid_with_size().type_id(),
                    remote_type_object))
        {
            EPROSIMA_LOG_ERROR(STATISTICS_PARTICIPANT_LISTENER,
                    "Error getting type object for type " << writer_data.type_name);
            return discovery_info;
        }

        // Build remotely discovered type
        DynamicType::_ref_type remote_type = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
            remote_type_object)->build();

        // Serialize DynamicType into its IDL representation
        std::stringstream idl;
        idl_serialize(remote_type, idl);
        discovery_info.type_idl = idl.str();
    }


    return discovery_info;
}

} // namespace subscriber
} // namespace statistics_backend
} // namespace eprosima

