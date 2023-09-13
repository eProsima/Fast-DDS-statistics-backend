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
//

/**
 * @file database_queue.cpp
 */

#include "database_queue.hpp"

#include <fastdds/dds/log/Log.hpp>

namespace eprosima {
namespace statistics_backend {
namespace database {

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;

template<typename T>
std::string to_string(
        T data)
{
    std::stringstream ss;
    ss << data;
    return ss.str();
}

// Return the participant_id
std::string get_participant_id(
        const GUID_t& guid)
{
    // The participant_id can be obtained from the last 4 octets in the GUID prefix
    std::stringstream buffer;
    buffer << std::hex << std::setfill('0');
    for (int i = 0; i < 3; i++)
    {
        buffer << std::setw(2) << static_cast<unsigned>(guid.guidPrefix.value[i + 8]);
        buffer << ".";
    }
    buffer << std::setw(2) << static_cast<unsigned>(guid.guidPrefix.value[3 + 8]);

    return buffer.str();
}

EntityId DatabaseEntityQueue::process_participant(
        const EntityDiscoveryInfo& info)
{
    EntityId participant_id;
    
    std::shared_ptr<Host> host;
    std::shared_ptr<User> user;
    std::shared_ptr<Process> process;
    bool graph_updated = false;
    bool should_link_process_participant = false;

    try
    {
        // See if the participant is already in the database
        // This will throw if the participant is unknown
        participant_id = database_->get_entity_by_guid(
            EntityKind::PARTICIPANT, to_string(info.guid))
                        .second;

        // Update the entity status and check if its references must also change it status
        database_->change_entity_status(participant_id,
                info.discovery_status !=
                details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);
        
    }
    catch (BadParameter&)
    {
        // The participant is not in the database
        if (info.discovery_status == details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)
        {
            // Get the domain from the database
            // This may throw if the domain does not exist
            // The database MUST contain the domain, or something went wrong upstream
            std::shared_ptr<database::Domain> domain = std::const_pointer_cast<database::Domain>(
                std::static_pointer_cast<const database::Domain>(database_->get_entity(info.domain_id)));

            std::string name = info.participant_name;

            // If the user does not provide a specific name for the participant, give it a descriptive name
            if (name.empty())
            {
                // The name will be constructed as IP:participant_id
                name = info.address + ":" + get_participant_id(info.guid);
            }

            EntityStatus status = info.entity_status;

            // Create the participant and add it to the database
            GUID_t participant_guid = info.guid;
            auto participant = std::make_shared<database::DomainParticipant>(
                name,
                info.qos,
                to_string(participant_guid),
                std::shared_ptr<database::Process>(),
                domain,
                status,
                info.app_id,
                info.app_metadata);

            participant_id = database_->insert(participant);
            should_link_process_participant = true;

        }
        else
        {
            throw BadParameter("Update or undiscover a participant which is not in the database");
        }
    }

    // Process host-user-process
    try
    {
        // Get host entity
        auto hosts = database_->get_entities_by_name(EntityKind::HOST, info.host);
        if (hosts.empty())
        {
            host.reset(new Host(info.host));
            host->id = database_->insert(std::static_pointer_cast<Entity>(host));
        }
        else
        {
            // Host name reported by Fast DDS are considered unique
            std::shared_ptr<const Host> const_host = std::dynamic_pointer_cast<const Host>(database_->get_entity(
                                hosts.front().second));
            host = std::const_pointer_cast<Host>(const_host);
        }  
        
        // Get user entity
        auto users = database_->get_entities_by_name(EntityKind::USER, info.user);
        for (const auto& it : users)
        {
            std::shared_ptr<const User> const_user =
                    std::dynamic_pointer_cast<const User>(database_->get_entity(it.second));

            // The user name is unique within the host
            if (const_user->host == host)
            {
                user = std::const_pointer_cast<User>(const_user);
                break;
            }
        }
        if (!user)
        {
            user.reset(new User(info.user, host));
            user->id = database_->insert(std::static_pointer_cast<Entity>(user));
        }

        // Get process entity
        std::string process_name;
        std::string process_pid;
        size_t separator_pos = info.process.find_last_of(':');
        if (separator_pos == std::string::npos)
        {
            logInfo(BACKEND_DATABASE,
                    "Process name " + item.process() + " does not follow the [command]:[PID] pattern");
            process_name = info.process;
            process_pid = info.process;
        }
        else
        {
            process_name = info.process.substr(0, separator_pos);
            process_pid = info.process.substr(separator_pos + 1);
        }
        auto processes = database_->get_entities_by_name(EntityKind::PROCESS, process_name);
        for (const auto& it : processes)
        {
            std::shared_ptr<const Process> const_process =
                    std::dynamic_pointer_cast<const Process>(database_->get_entity(it.second));

            // There is only one process with the same name for a given user
            if (const_process->user == user)
            {
                process = std::const_pointer_cast<Process>(const_process);
                break;
            }
        }
        if (!process)
        {
            process.reset(new Process(process_name, process_pid, user));
            process->id = database_->insert(std::static_pointer_cast<Entity>(process));
            should_link_process_participant = true;
        }
        if (should_link_process_participant)
        {
            database_->link_participant_with_process(participant_id, process->id);
        }

        graph_updated = database_->update_participant_in_graph(info.domain_id, host->id, user->id, process->id, participant_id);

    }
    catch(const std::exception& e)
    {
        logError(BACKEND_DATABASE_QUEUE, e.what());

        if(host == nullptr || host->id == EntityId::invalid())
        {
            graph_updated = database_->update_participant_in_graph(info.domain_id, EntityId(), EntityId(), EntityId(), EntityId());
        }
        else if(user == nullptr || user->id == EntityId::invalid())
        {
            graph_updated = database_->update_participant_in_graph(info.domain_id, host->id, EntityId(), EntityId(), EntityId());
        }
        else if(process == nullptr || process->id == EntityId::invalid())
        {
            graph_updated = database_->update_participant_in_graph(info.domain_id, host->id, user->id, EntityId(), EntityId());
        }
    }

    if(graph_updated)
    {
        details::StatisticsBackendData::get_instance()->on_domain_graph_update(info.domain_id);
    }

    return participant_id;
}

EntityId DatabaseEntityQueue::process_datareader(
        const EntityDiscoveryInfo& info)
{
    EntityId datareader_id;

    try
    {
        // See if the reader is already in the database
        // This will throw if the reader is unknown
        datareader_id =
                database_->get_entity_by_guid(
            EntityKind::DATAREADER, to_string(info.guid)).second;

        // Update the entity status and check if its references must also change it status
        database_->change_entity_status(datareader_id,
                info.discovery_status !=
                details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);

        // Delete inactive entites
        // Get the participant from the database
        GUID_t endpoint_guid = info.guid;
        GUID_t participant_guid(endpoint_guid.guidPrefix, c_EntityId_RTPSParticipant);
        std::pair<EntityId, EntityId> participant_id;
        try
        {
            participant_id = database_->get_entity_by_guid(EntityKind::PARTICIPANT, to_string(participant_guid));
            assert(participant_id.first == info.domain_id);
        }
        catch (const Exception&)
        {
            throw BadParameter("endpoint " + to_string(endpoint_guid)
                        +  " undiscovered on Participant " + to_string(participant_guid)
                        + " but there is no such Participant in the database");
        }

        // Check whether the topic is already in the database
        std::shared_ptr<database::Topic> topic;
        auto topic_ids = database_->get_entities_by_name(EntityKind::TOPIC, info.topic_name);

        // Check if any of these topics is in the current domain AND shares the data type
        EntityId topic_id;
        for (const auto& topic_id_ : topic_ids)
        {
            if (topic_id_.first == info.domain_id)
            {
                topic = std::const_pointer_cast<database::Topic>(
                    std::static_pointer_cast<const database::Topic>(database_->get_entity(topic_id_.second)));

                if (topic->data_type == info.type_name)
                {
                    topic_id = topic_id_.second;
                    //Found the correct topic
                    break;
                }
                else
                {
                    // The data type is not the same, so it must be another topic
                    topic.reset();
                }
            }
        }

        if(database_->update_endpoint_in_graph(info.domain_id, participant_id.second, topic_id, datareader_id))
        {
            details::StatisticsBackendData::get_instance()->on_domain_graph_update(info.domain_id);
        }
    }
    catch (BadParameter&)
    {
        // The reader is not in the database
        if (info.discovery_status == details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)
        {
            datareader_id = process_endpoint_discovery(info);
        }
        else
        {
            throw BadParameter("Update or undiscover a subscriber which is not in the database");
        }
    }
    return datareader_id;
}

EntityId DatabaseEntityQueue::process_datawriter(
        const EntityDiscoveryInfo& info)
{
    EntityId datawriter_id;

    try
    {
        // See if the writer is already in the database
        // This will throw if the writer is unknown
        datawriter_id =
                database_->get_entity_by_guid(
            EntityKind::DATAWRITER, to_string(info.guid)).second;

        // Update the entity status and check if its references must also change it status
        database_->change_entity_status(datawriter_id,
                info.discovery_status !=
                details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY);

        // Delete inactive entites
        // Get the participant from the database
        GUID_t endpoint_guid = info.guid;
        GUID_t participant_guid(endpoint_guid.guidPrefix, c_EntityId_RTPSParticipant);
        std::pair<EntityId, EntityId> participant_id;
        try
        {
            participant_id = database_->get_entity_by_guid(EntityKind::PARTICIPANT, to_string(participant_guid));
            assert(participant_id.first == info.domain_id);
        }
        catch (const Exception&)
        {
            throw BadParameter("endpoint " + to_string(endpoint_guid)
                        +  " undiscovered on Participant " + to_string(participant_guid)
                        + " but there is no such Participant in the database");
        }

        // Check whether the topic is already in the database
        std::shared_ptr<database::Topic> topic;
        auto topic_ids = database_->get_entities_by_name(EntityKind::TOPIC, info.topic_name);

        // Check if any of these topics is in the current domain AND shares the data type
        EntityId topic_id;
        for (const auto& topic_id_ : topic_ids)
        {
            if (topic_id_.first == info.domain_id)
            {
                topic = std::const_pointer_cast<database::Topic>(
                    std::static_pointer_cast<const database::Topic>(database_->get_entity(topic_id_.second)));

                if (topic->data_type == info.type_name)
                {
                    topic_id = topic_id_.second;
                    //Found the correct topic
                    break;
                }
                else
                {
                    // The data type is not the same, so it must be another topic
                    topic.reset();
                }
            }
        }

        if(database_->update_endpoint_in_graph(info.domain_id, participant_id.second, topic_id, datawriter_id))
        {
            details::StatisticsBackendData::get_instance()->on_domain_graph_update(info.domain_id);
        }
    }
    catch (BadParameter&)
    {
        // The writer is not in the database
        if (info.discovery_status == details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)
        {
            datawriter_id = process_endpoint_discovery(info);
        }
        else
        {
            throw BadParameter("Update or undiscover a publisher which is not in the database");
        }
    }
    return datawriter_id;
}

template<typename T>
EntityId DatabaseEntityQueue::process_endpoint_discovery(
        const T& info)
{
    // Get the domain from the database
    // This may throw if the domain does not exist
    // The database MUST contain the domain, or something went wrong upstream
    std::shared_ptr<database::Domain> domain = std::const_pointer_cast<database::Domain>(
        std::static_pointer_cast<const database::Domain>(database_->get_entity(info.domain_id)));

    // Get the participant from the database
    GUID_t endpoint_guid = info.guid;
    GUID_t participant_guid(endpoint_guid.guidPrefix, c_EntityId_RTPSParticipant);
    std::pair<EntityId, EntityId> participant_id;
    try
    {
        participant_id = database_->get_entity_by_guid(EntityKind::PARTICIPANT, to_string(participant_guid));
        assert(participant_id.first == info.domain_id);
    }
    catch (const Exception&)
    {
        throw BadParameter("endpoint " + to_string(endpoint_guid)
                      +  " discovered on Participant " + to_string(participant_guid)
                      + " but there is no such Participant in the database");
    }
    std::shared_ptr<database::DomainParticipant> participant =
            std::const_pointer_cast<database::DomainParticipant>(
        std::static_pointer_cast<const database::DomainParticipant>(database_->get_entity(
            participant_id.second)));

    // Check whether the topic is already in the database
    std::shared_ptr<database::Topic> topic;
    auto topic_ids = database_->get_entities_by_name(EntityKind::TOPIC, info.topic_name);

    // Check if any of these topics is in the current domain AND shares the data type
    EntityId topic_id;
    for (const auto& topic_id_ : topic_ids)
    {
        if (topic_id_.first == info.domain_id)
        {
            topic = std::const_pointer_cast<database::Topic>(
                std::static_pointer_cast<const database::Topic>(database_->get_entity(topic_id_.second)));

            if (topic->data_type == info.type_name)
            {
                topic_id = topic_id_.second;
                //Found the correct topic
                break;
            }
            else
            {
                // The data type is not the same, so it must be another topic
                topic.reset();
            }
        }
    }

    // If no such topic exists, create a new one
    if (!topic)
    {
        topic = std::make_shared<database::Topic>(
            info.topic_name,
            info.type_name,
            domain);

        if (!info.alias.empty())
        {
            topic->alias = info.alias;
        }

        topic_id = database_->insert(topic);
        details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
            info.domain_id,
            topic_id,
            EntityKind::TOPIC,
            details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
    }

    // Create the endpoint
    std::shared_ptr<database::DDSEndpoint> endpoint;
    if (info.kind() == EntityKind::DATAREADER)
    {
        endpoint = create_datareader(endpoint_guid, info, participant, topic);
    }
    else
    {
        endpoint = create_datawriter(endpoint_guid, info, participant, topic);
    }


    // Mark it as the meta traffic one
    endpoint->is_virtual_metatraffic = info.is_virtual_metatraffic;
    if (!info.alias.empty())
    {
        endpoint->alias = info.alias;
    }

    /* Start processing the locator info */

    // Routine to process one locator from the locator list of the endpoint
    auto process_locators = [&](const Locator_t& dds_locator)
            {
                std::shared_ptr<database::Locator> locator;

                // Look for an existing locator
                // There can only be one
                auto locator_ids = database_->get_entities_by_name(EntityKind::LOCATOR, to_string(dds_locator));
                assert(locator_ids.empty() || locator_ids.size() == 1);

                if (!locator_ids.empty())
                {
                    // The locator exists. Add the existing one.
                    locator = std::const_pointer_cast<database::Locator>(
                        std::static_pointer_cast<const database::Locator>(database_->get_entity(locator_ids.front().
                                second)));
                }
                else
                {
                    // The locator is not in the database. Add the new one.
                    locator = std::make_shared<database::Locator>(to_string(dds_locator));
                    locator->id = database_->insert(locator);
                    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                        locator->id,
                        EntityKind::LOCATOR,
                        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
                }

                endpoint->locators[locator->id] = locator;
            };

    for (const auto& dds_locator : info.locators.unicast)
    {
        process_locators(dds_locator);
    }
    for (const auto& dds_locator : info.locators.multicast)
    {
        process_locators(dds_locator);
    }

    // insert the endpoint
    EntityId endpoint_entity = database_->insert(endpoint);

    // Force the refresh of the parent entities' status
    database_->change_entity_status(endpoint_entity, true);

    if(database_->update_endpoint_in_graph(info.domain_id, participant_id.second, topic_id, endpoint_entity))
    {
        details::StatisticsBackendData::get_instance()->on_domain_graph_update(info.domain_id);
    }
    return endpoint_entity;
}

std::shared_ptr<database::DDSEndpoint> DatabaseEntityQueue::create_datawriter(
        const GUID_t& guid,
        const EntityDiscoveryInfo& info,
        std::shared_ptr<database::DomainParticipant> participant,
        std::shared_ptr<database::Topic> topic)
{
    std::stringstream name;
    name << "DataWriter_" << info.topic_name << "_" << info.guid.entityId;

    return std::make_shared<database::DataWriter>(
        name.str(),
        info.qos,
        to_string(guid),
        participant,
        topic);
}

std::shared_ptr<database::DDSEndpoint> DatabaseEntityQueue::create_datareader(
        const GUID_t& guid,
        const EntityDiscoveryInfo& info,
        std::shared_ptr<database::DomainParticipant> participant,
        std::shared_ptr<database::Topic> topic)
{
    std::stringstream name;
    name << "DataReader_" << info.topic_name << "_" << info.guid.entityId;

    return std::make_shared<database::DataReader>(
        name.str(),
        info.qos,
        to_string(guid),
        participant,
        topic);
}

EntityId DatabaseDataQueue::get_or_create_locator(
        const std::string& locator_name) const
{
    auto found_remote_locators = database_->get_entities_by_name(EntityKind::LOCATOR, locator_name);
    // In case that the reported locator is not known, create it without being linked to an endpoint
    if (found_remote_locators.empty())
    {
        std::shared_ptr<Locator> locator = std::make_shared<Locator>(locator_name);
        locator->id = database_->insert(locator);
        details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
            locator->id,
            EntityKind::LOCATOR,
            details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
        return locator->id;
    }
    return found_remote_locators.front().second;
}

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        HistoryLatencySample& sample,
        const DatabaseQueue::StatisticsWriterReaderData& item) const
{
    sample.data = item.data();
    std::string reader_guid = deserialize_guid(item.reader_guid());
    try
    {
        auto found_reader = database_->get_entity_by_guid(EntityKind::DATAREADER, reader_guid);
        sample.reader = found_reader.second;
    }
    catch (BadParameter&)
    {
        throw Error("Reader " + reader_guid + " not found");
    }

    std::string writer_guid = deserialize_guid(item.writer_guid());
    try
    {
        auto found_entity = database_->get_entity_by_guid(entity_kind, writer_guid);
        domain = found_entity.first;
        entity = found_entity.second;
    }
    catch (BadParameter&)
    {
        throw Error("Entity " + writer_guid + " not found");
    }
}

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        NetworkLatencySample& sample,
        const StatisticsLocator2LocatorData& item) const
{
    sample.data = item.data();
    std::string remote_locator = deserialize_locator(item.dst_locator());
    sample.remote_locator = get_or_create_locator(remote_locator);

    std::string source_locator = deserialize_guid(item.src_locator());
    // This call will throw BadParameter if there is no such entity
    // We let this exception through, as it meets expectations
    auto found_entities = database_->get_entity_by_guid(entity_kind, source_locator);
    domain = found_entities.first;
    entity = found_entities.second;
}

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        EntityDataSample& sample,
        const StatisticsEntityData& item) const
{
    sample.data =  item.data();

    std::string guid = deserialize_guid(item.guid());
    try
    {
        auto found_entity = database_->get_entity_by_guid(entity_kind, guid);
        domain = found_entity.first;
        entity = found_entity.second;
    }
    catch (BadParameter&)
    {
        throw Error("Entity " + guid + " not found");
    }
}

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        EntityToLocatorCountSample& sample,
        const StatisticsEntity2LocatorTraffic& item) const
{
    sample.count = item.packet_count();
    std::string remote_locator = deserialize_locator(item.dst_locator());
    sample.remote_locator = get_or_create_locator(remote_locator);

    std::string guid = deserialize_guid(item.src_guid());
    try
    {
        auto found_entity = database_->get_entity_by_guid(entity_kind, guid);
        domain = found_entity.first;
        entity = found_entity.second;
    }
    catch (BadParameter&)
    {
        throw Error("Entity " + guid + " not found");
    }
}

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        ByteToLocatorCountSample& sample,
        const StatisticsEntity2LocatorTraffic& item) const
{
    sample.count = item.byte_count();
    sample.magnitude_order = item.byte_magnitude_order();
    std::string remote_locator = deserialize_locator(item.dst_locator());
    sample.remote_locator = get_or_create_locator(remote_locator);

    std::string guid = deserialize_guid(item.src_guid());
    try
    {
        auto found_entity = database_->get_entity_by_guid(entity_kind, guid);
        domain = found_entity.first;
        entity = found_entity.second;
    }
    catch (BadParameter&)
    {
        throw Error("Entity " + guid + " not found");
    }
}

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        EntityCountSample& sample,
        const StatisticsEntityCount& item) const
{
    sample.count = item.count();

    std::string guid = deserialize_guid(item.guid());
    try
    {
        auto found_entity = database_->get_entity_by_guid(entity_kind, guid);
        domain = found_entity.first;
        entity = found_entity.second;
    }
    catch (BadParameter&)
    {
        throw Error("Entity " + guid + " not found");
    }
}

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        DiscoveryTimeSample& sample,
        const StatisticsDiscoveryTime& item) const
{
    sample.time = nanoseconds_to_systemclock(item.time());
    std::string remote_entity_guid = deserialize_guid(item.remote_entity_guid());
    try
    {
        auto found_remote_entity = database_->get_entity_by_guid(entity_kind, remote_entity_guid);
        sample.remote_entity = found_remote_entity.second;
    }
    catch (BadParameter&)
    {
        throw Error("Remote entity " + remote_entity_guid + " not found");
    }

    std::string guid = deserialize_guid(item.local_participant_guid());
    try
    {
        auto found_entity = database_->get_entity_by_guid(entity_kind, guid);
        domain = found_entity.first;
        entity = found_entity.second;
    }
    catch (BadParameter&)
    {
        throw Error("Entity " + guid + " not found");
    }
}

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        SampleDatasCountSample& sample,
        const StatisticsSampleIdentityCount& item) const
{
    sample.count = item.count();

    auto sample_identity = deserialize_sample_identity(item.sample_id());
    sample.sequence_number = sample_identity.second;
    try
    {
        auto found_entity = database_->get_entity_by_guid(entity_kind, sample_identity.first);
        domain = found_entity.first;
        entity = found_entity.second;
    }
    catch (BadParameter&)
    {
        throw Error("Entity " + sample_identity.first + " not found");
    }
}

void DatabaseDataQueue::process_sample()
{
    switch (front().second->_d())
    {
        case StatisticsEventKind::HISTORY2HISTORY_LATENCY:
        {
            HistoryLatencySample sample;
            EntityId domain;
            EntityId entity;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, EntityKind::DATAWRITER, sample, item.second->writer_reader_data());
                database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity,
                        DataKind::FASTDDS_LATENCY);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing HISTORY2HISTORY_LATENCY event. Data was not added to the statistics collection: "
                        + std::string(
                            e.what()));
            }
            break;
        }
        case StatisticsEventKind::NETWORK_LATENCY:
        {
            NetworkLatencySample sample;
            EntityId domain;
            EntityId entity;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, EntityKind::PARTICIPANT, sample,
                        item.second->locator2locator_data());
                database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity,
                        DataKind::NETWORK_LATENCY);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing NETWORK_LATENCY event. Data was not added to the statistics collection: "
                        + std::string(e.what()));
            }
            break;
        }
        case StatisticsEventKind::PUBLICATION_THROUGHPUT:
        {
            PublicationThroughputSample sample;
            EntityId domain;
            EntityId entity;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, EntityKind::DATAWRITER, static_cast<EntityDataSample&>(sample),
                        item.second->entity_data());
                database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity,
                        DataKind::PUBLICATION_THROUGHPUT);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing PUBLICATION_THROUGHPUT event. Data was not added to the statistics collection: "
                        + std::string(
                            e.what()));
            }
            break;
        }
        case StatisticsEventKind::SUBSCRIPTION_THROUGHPUT:
        {
            SubscriptionThroughputSample sample;
            EntityId domain;
            EntityId entity;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, EntityKind::DATAREADER, static_cast<EntityDataSample&>(sample),
                        item.second->entity_data());
                database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity,
                        DataKind::SUBSCRIPTION_THROUGHPUT);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing SUBSCRIPTION_THROUGHPUT event. Data was not added to the statistics collection: "
                        + std::string(
                            e.what()));
            }
            break;
        }
        case StatisticsEventKind::RTPS_SENT:
        {
            RtpsPacketsSentSample packet_sample;
            RtpsBytesSentSample byte_sample;
            EntityId domain;
            EntityId entity;
            queue_item_type item = front();
            packet_sample.src_ts = item.first;
            byte_sample.src_ts = item.first;

            try
            {
                process_sample_type(domain, entity, EntityKind::DATAWRITER,
                        static_cast<EntityToLocatorCountSample&>(packet_sample),
                        item.second->entity2locator_traffic());
                database_->insert(domain, entity, packet_sample);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing RTPS_SENT event. Data was not added to the statistics collection: "
                        + std::string(e.what()));
            }
            try
            {
                process_sample_type(domain, entity, EntityKind::DATAWRITER,
                        static_cast<ByteToLocatorCountSample&>(byte_sample),
                        item.second->entity2locator_traffic());
                database_->insert(domain, entity, byte_sample);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity,
                        DataKind::RTPS_PACKETS_SENT);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity,
                        DataKind::RTPS_BYTES_SENT);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing RTPS_SENT event. Data was not added to the statistics collection: "
                        + std::string(e.what()));
            }
            break;
        }
        case StatisticsEventKind::RTPS_LOST:
        {

            RtpsPacketsLostSample packet_sample;
            RtpsBytesLostSample byte_sample;
            EntityId domain;
            EntityId entity;
            queue_item_type item = front();
            packet_sample.src_ts = item.first;
            byte_sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, EntityKind::DATAWRITER,
                        static_cast<EntityToLocatorCountSample&>(packet_sample),
                        item.second->entity2locator_traffic());
                database_->insert(domain, entity, packet_sample);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing RTPS_LOST event. Data was not added to the statistics collection: "
                        + std::string(e.what()));
            }
            try
            {
                process_sample_type(domain, entity, EntityKind::DATAWRITER,
                        static_cast<ByteToLocatorCountSample&>(byte_sample),
                        item.second->entity2locator_traffic());
                database_->insert(domain, entity, byte_sample);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity,
                        DataKind::RTPS_PACKETS_LOST);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity,
                        DataKind::RTPS_BYTES_LOST);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing RTPS_LOST event. Data was not added to the statistics collection: "
                        + std::string(e.what()));
            }
            break;
        }
        case StatisticsEventKind::RESENT_DATAS:
        {
            ResentDataSample sample;
            EntityId domain;
            EntityId entity;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, EntityKind::DATAWRITER, static_cast<EntityCountSample&>(sample),
                        item.second->entity_count());
                database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity,
                        DataKind::RESENT_DATA);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing RESENT_DATAS event. Data was not added to the statistics collection: "
                        + std::string(e.what()));
            }
            break;
        }
        case StatisticsEventKind::HEARTBEAT_COUNT:
        {
            HeartbeatCountSample sample;
            EntityId domain;
            EntityId entity;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, EntityKind::DATAWRITER, static_cast<EntityCountSample&>(sample),
                        item.second->entity_count());
                database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity,
                        DataKind::HEARTBEAT_COUNT);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing HEARTBEAT_COUNT event. Data was not added to the statistics collection: "
                        + std::string(e.what()));
            }
            break;
        }
        case StatisticsEventKind::ACKNACK_COUNT:
        {
            AcknackCountSample sample;
            EntityId domain;
            EntityId entity;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, EntityKind::DATAREADER, static_cast<EntityCountSample&>(sample),
                        item.second->entity_count());
                database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity,
                        DataKind::ACKNACK_COUNT);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing ACKNACK_COUNT event. Data was not added to the statistics collection: "
                        + std::string(e.what()));
            }
            break;
        }
        case StatisticsEventKind::NACKFRAG_COUNT:
        {
            NackfragCountSample sample;
            EntityId domain;
            EntityId entity;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, EntityKind::DATAREADER, static_cast<EntityCountSample&>(sample),
                        item.second->entity_count());
                database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity,
                        DataKind::NACKFRAG_COUNT);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing NACKFRAG_COUNT event. Data was not added to the statistics collection: "
                        + std::string(e.what()));
            }
            break;
        }
        case StatisticsEventKind::GAP_COUNT:
        {
            GapCountSample sample;
            EntityId domain;
            EntityId entity;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, EntityKind::DATAWRITER, static_cast<EntityCountSample&>(sample),
                        item.second->entity_count());
                database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity, DataKind::GAP_COUNT);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing GAP_COUNT event. Data was not added to the statistics collection: "
                        + std::string(e.what()));
            }
            break;
        }
        case StatisticsEventKind::DATA_COUNT:
        {
            DataCountSample sample;
            EntityId domain;
            EntityId entity;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, EntityKind::DATAWRITER, static_cast<EntityCountSample&>(sample),
                        item.second->entity_count());
                database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity, DataKind::DATA_COUNT);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing DATA_COUNT event. Data was not added to the statistics collection: "
                        + std::string(e.what()));
            }
            break;
        }
        case StatisticsEventKind::PDP_PACKETS:
        {
            PdpCountSample sample;
            EntityId domain;
            EntityId entity;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, EntityKind::PARTICIPANT, static_cast<EntityCountSample&>(sample),
                        item.second->entity_count());
                database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity,
                        DataKind::PDP_PACKETS);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing PDP_PACKETS event. Data was not added to the statistics collection: "
                        + std::string(e.what()));
            }
            break;
        }
        case StatisticsEventKind::EDP_PACKETS:
        {
            EdpCountSample sample;
            EntityId domain;
            EntityId entity;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, EntityKind::PARTICIPANT, static_cast<EntityCountSample&>(sample),
                        item.second->entity_count());
                database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity,
                        DataKind::EDP_PACKETS);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing EDP_PACKETS event. Data was not added to the statistics collection: "
                        + std::string(e.what()));
            }
            break;
        }
        case StatisticsEventKind::DISCOVERED_ENTITY:
        {
            DiscoveryTimeSample sample;
            EntityId domain;
            EntityId entity;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, EntityKind::PARTICIPANT, sample, item.second->discovery_time());
                database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity,
                        DataKind::DISCOVERY_TIME);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing DISCOVERED_ENTITY event. Data was not added to the statistics collection: "
                        + std::string(e.what()));
            }
            break;
        }
        case StatisticsEventKind::SAMPLE_DATAS:
        {
            SampleDatasCountSample sample;
            EntityId domain;
            EntityId entity;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, EntityKind::DATAWRITER, sample,
                        item.second->sample_identity_count());
                database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_data_available(domain, entity,
                        DataKind::SAMPLE_DATAS);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing SAMPLE_DATAS event. Data was not added to the statistics collection: "
                        + std::string(e.what()));
            }
            break;
        }

        case StatisticsEventKind::PHYSICAL_DATA:
        {
            break;
        }
    }
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
