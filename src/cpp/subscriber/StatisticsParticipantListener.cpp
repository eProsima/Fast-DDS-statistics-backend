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

/**
 * @file StatisticsParticipantListener.cpp
 */
#include <utility>

#include "subscriber/StatisticsParticipantListener.hpp"

#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>
#include <fastdds/dds/xtypes/utils.hpp>
#include <fastdds/rtps/common/EntityId_t.hpp>
#include <fastdds/rtps/writer/WriterDiscoveryStatus.hpp>

#include "database/database_queue.hpp"
#include "subscriber/QosSerializer.hpp"


namespace eprosima {
namespace statistics_backend {
namespace subscriber {

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

template<typename T>
std::string to_string(
        T data)
{
    std::stringstream ss;
    ss << data;
    return ss.str();
}

/**
 * Checks whether an entity id corresponds to a builtin statistics writer.
 * @param [in] entity_id The entity id to check.
 * @return true when the entity id corresponds to a builtin statistics writer.
 */
inline bool is_statistics_builtin(
        const EntityId_t& entity_id)
{
    return 0x60 == (0xE0 & entity_id.value[3]);
}

StatisticsParticipantListener::StatisticsParticipantListener(
        EntityId domain_id,
        database::Database* database,
        database::DatabaseEntityQueue* entity_queue,
        database::DatabaseDataQueue<eprosima::fastdds::statistics::Data>* data_queue,
        database::DatabaseDataQueue<database::ExtendedMonitorServiceStatusData>* monitor_service_data_queue)
noexcept
    : DomainParticipantListener()
    , domain_id_(domain_id)
    , database_(database)
    , entity_queue_(entity_queue)
    , data_queue_(data_queue)
    , monitor_service_status_data_queue_(monitor_service_data_queue)
{
}

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
        const ParticipantBuiltinTopicData& info)
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

void StatisticsParticipantListener::on_participant_discovery(
        DomainParticipant* /*participant*/,
        ParticipantDiscoveryStatus reason,
        const ParticipantBuiltinTopicData& info,
        bool& /*should_be_ignored*/)
{
    // First stop the data queues until the new entity is created
    data_queue_->stop_consumer();
    monitor_service_status_data_queue_->stop_consumer();

    std::chrono::system_clock::time_point timestamp = now();

    // Meaningful prefix for metatraffic entities
    const std::string metatraffic_prefix = "___EPROSIMA___METATRAFFIC___DOMAIN_" + std::to_string(domain_id_.value()) +
            "___";
    const std::string metatraffic_alias = "_metatraffic_";

    // Build the discovery info for the queue
    database::EntityDiscoveryInfo discovery_info(EntityKind::PARTICIPANT);
    discovery_info.domain_id = domain_id_;
    discovery_info.guid = info.guid;
    discovery_info.qos = subscriber::participant_proxy_data_to_backend_qos(info);

    discovery_info.address = get_address(info);
    discovery_info.participant_name = info.participant_name.to_string();

    discovery_info.entity_status = StatusLevel::OK_STATUS;

    discovery_info.discovery_source = DiscoverySource::DISCOVERY;

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

    //Get data from participant discovery info
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

    discovery_info.host = get_property_value(info.properties,
                    eprosima::fastdds::dds::parameter_policy_physical_data_host);
    discovery_info.host = discovery_info.host.empty()? "Unknown" : discovery_info.host;

    discovery_info.user = get_property_value(info.properties,
                    eprosima::fastdds::dds::parameter_policy_physical_data_user);
    discovery_info.user = discovery_info.user.empty()? "Unknown" : discovery_info.user;

    discovery_info.process = get_property_value(info.properties,
                    eprosima::fastdds::dds::parameter_policy_physical_data_process);
    discovery_info.process = discovery_info.process.empty()? "Unknown" : discovery_info.process;

    std::string app_id = get_property_value(info.properties, "fastdds.application.id");
    auto it = app_id_enum.find(app_id);
    discovery_info.app_id = it != app_id_enum.end()? it->second : AppId::UNKNOWN;

    discovery_info.app_metadata = get_property_value(info.properties, "fastdds.application.metadata");

    entity_queue_->push(timestamp, discovery_info);

    // Create metatraffic entities
    if (details::StatisticsBackendData::DiscoveryStatus::UPDATE != discovery_info.discovery_status)
    {
        // Create metatraffic endpoint and locator on the metatraffic topic.
        {
            // The endpoint QoS cannot be empty. We can use this to give a description to the user.
            database::Qos meta_traffic_qos = {
                {"description", "This is a virtual placeholder endpoint with no real counterpart"}};
            // Push it to the queue
            database::EntityDiscoveryInfo datawriter_discovery_info(EntityKind::DATAWRITER);

            for (auto dds_locator : info.metatraffic_locators.unicast)
            {
                datawriter_discovery_info.locators.add_unicast_locator(dds_locator);
            }
            for (auto dds_locator : info.metatraffic_locators.multicast)
            {
                datawriter_discovery_info.locators.add_multicast_locator(dds_locator);
            }
            for (auto dds_locator : info.default_locators.unicast)
            {
                datawriter_discovery_info.locators.add_unicast_locator(dds_locator);
            }
            for (auto dds_locator : info.default_locators.multicast)
            {
                datawriter_discovery_info.locators.add_multicast_locator(dds_locator);
            }

            datawriter_discovery_info.domain_id = domain_id_;
            datawriter_discovery_info.topic_name = metatraffic_prefix + "TOPIC";
            datawriter_discovery_info.type_name = metatraffic_prefix + "TYPE";
            datawriter_discovery_info.guid = info.guid;
            datawriter_discovery_info.qos = meta_traffic_qos;
            datawriter_discovery_info.alias = metatraffic_alias;
            datawriter_discovery_info.is_virtual_metatraffic = true;
            datawriter_discovery_info.entity_status = StatusLevel::OK_STATUS;
            datawriter_discovery_info.discovery_status = discovery_info.discovery_status;
            datawriter_discovery_info.discovery_source = DiscoverySource::DISCOVERY;
            entity_queue_->push(timestamp, datawriter_discovery_info);
        }
    }

    // Wait until the entity queues is processed and restart the data queue
    entity_queue_->flush();
    data_queue_->start_consumer();
    monitor_service_status_data_queue_->start_consumer();
}

void StatisticsParticipantListener::on_data_reader_discovery(
        DomainParticipant* participant,
        ReaderDiscoveryStatus reason,
        const SubscriptionBuiltinTopicData& info,
        bool& /*should_be_ignored*/)
{
    // Filter out our own statistics readers
    if (participant->guid().guidPrefix == info.guid.guidPrefix)
    {
        return;
    }

    // First stop the data queues until the new entity is created
    data_queue_->stop_consumer();
    monitor_service_status_data_queue_->stop_consumer();

    std::chrono::system_clock::time_point timestamp = now();

    // Build the discovery info for the queue
    database::EntityDiscoveryInfo discovery_info(EntityKind::DATAREADER);
    discovery_info.domain_id = domain_id_;
    discovery_info.guid = info.guid;
    discovery_info.qos = subscriber::reader_proxy_data_to_backend_qos(info);

    discovery_info.topic_name = info.topic_name.to_string();
    discovery_info.type_name = info.type_name.to_string();
    discovery_info.locators = info.remote_locators;

    discovery_info.discovery_source = DiscoverySource::DISCOVERY;

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
    if (ReaderDiscoveryStatus::DISCOVERED_READER == reason && info.type_information.assigned() == true)
    {
        // Create IDL representation of the discovered type
        // Get remote type information
        xtypes::TypeObject remote_type_object;
        if (RETCODE_OK != DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                    info.type_information.type_information.complete().typeid_with_size().type_id(),
                    remote_type_object))
        {
            EPROSIMA_LOG_ERROR(STATISTICS_PARTICIPANT_LISTENER,
                    "Error getting type object for type " << info.type_name);
            return;
        }

        // Build remotely discovered type
        DynamicType::_ref_type remote_type = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
            remote_type_object)->build();

        // Serialize DynamicType into its IDL representation
        std::stringstream idl;
        idl_serialize(remote_type, idl);
        discovery_info.type_idl = idl.str();
    }

    entity_queue_->push(timestamp, discovery_info);

    // Wait until the entity queue is processed and restart the data queues
    entity_queue_->flush();
    data_queue_->start_consumer();
    monitor_service_status_data_queue_->start_consumer();
}

void StatisticsParticipantListener::on_data_writer_discovery(
        DomainParticipant* participant,
        WriterDiscoveryStatus reason,
        const PublicationBuiltinTopicData& info,
        bool& /*should_be_ignored*/)
{
    // Contrary to what it's done in on_subscriber_discovery, here we do not filter our own datawritters, as
    // deactivation of fastdds statistics module is enforced for the statistics backend, and hence none is ever created
    static_cast<void>(participant);

    // First stop the data queues until the new entity is created
    data_queue_->stop_consumer();
    monitor_service_status_data_queue_->stop_consumer();

    std::chrono::system_clock::time_point timestamp = now();

    // Build the discovery info for the queue
    database::EntityDiscoveryInfo discovery_info(EntityKind::DATAWRITER);
    discovery_info.domain_id = domain_id_;
    discovery_info.guid = info.guid;
    discovery_info.qos = subscriber::writer_proxy_data_to_backend_qos(info);

    discovery_info.topic_name = info.topic_name.to_string();
    discovery_info.type_name = info.type_name.to_string();
    discovery_info.locators = info.remote_locators;

    discovery_info.discovery_source = DiscoverySource::DISCOVERY;

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
    if (WriterDiscoveryStatus::DISCOVERED_WRITER == reason && info.type_information.assigned() == true)
    {
        // Create IDL representation of the discovered type
        // Get remote type information
        xtypes::TypeObject remote_type_object;
        if (RETCODE_OK != DomainParticipantFactory::get_instance()->type_object_registry().get_type_object(
                    info.type_information.type_information.complete().typeid_with_size().type_id(),
                    remote_type_object))
        {
            EPROSIMA_LOG_ERROR(STATISTICS_PARTICIPANT_LISTENER,
                    "Error getting type object for type " << info.type_name);
            return;
        }

        // Build remotely discovered type
        DynamicType::_ref_type remote_type = DynamicTypeBuilderFactory::get_instance()->create_type_w_type_object(
            remote_type_object)->build();

        // Serialize DynamicType into its IDL representation
        std::stringstream idl;
        idl_serialize(remote_type, idl);
        discovery_info.type_idl = idl.str();
    }

    entity_queue_->push(timestamp, discovery_info);

    // Wait until the entity queue is processed and restart the data queues
    entity_queue_->flush();
    data_queue_->start_consumer();
    monitor_service_status_data_queue_->start_consumer();
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
