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
    EntityId participant_id = EntityId::invalid();

    std::map<std::string, EntityId> physical_entities_ids;
    physical_entities_ids[HOST_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[USER_ENTITY_TAG] = EntityId::invalid();
    physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId::invalid();

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
            std::string name = info.participant_name;

            // If the user does not provide a specific name for the participant, give it a descriptive name
            if (name.empty())
            {
                // The name will be constructed as IP:participant_id
                name = info.address + ":" + get_participant_id(info.guid);
            }

            StatusLevel status = info.entity_status;

            // Create the participant and add it to the database
            GUID_t participant_guid = info.guid;

            participant_id = database_->insert_new_participant(
                name,
                info.qos,
                to_string(participant_guid),
                info.domain_id,
                status,
                info.app_id,
                info.app_metadata);

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

        database_->process_physical_entities(
            info.host,
            info.user,
            process_name,
            process_pid,
            should_link_process_participant,
            participant_id,
            physical_entities_ids);

    }
    catch (const std::exception& e)
    {
        logError(BACKEND_DATABASE_QUEUE, e.what());
    }

    graph_updated = database_->update_participant_in_graph(
        info.domain_id, physical_entities_ids[HOST_ENTITY_TAG], physical_entities_ids[USER_ENTITY_TAG],
        physical_entities_ids[PROCESS_ENTITY_TAG], participant_id);

    if (graph_updated)
    {
        details::StatisticsBackendData::get_instance()->on_domain_view_graph_update(info.domain_id);
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
        auto topic_ids = database_->get_entities_by_name(EntityKind::TOPIC, info.topic_name);

        // Check if any of these topics is in the current domain AND shares the data type
        EntityId topic_id;
        for (const auto& topic_id_ : topic_ids)
        {
            if (topic_id_.first == info.domain_id)
            {
                if (database_->is_topic_in_database(info.type_name, topic_id_.second))
                {
                    topic_id = topic_id_.second;
                    //Found the correct topic
                    break;
                }
            }
        }

        if (database_->update_endpoint_in_graph(info.domain_id, participant_id.second, topic_id, datareader_id))
        {
            details::StatisticsBackendData::get_instance()->on_domain_view_graph_update(info.domain_id);
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
        auto topic_ids = database_->get_entities_by_name(EntityKind::TOPIC, info.topic_name);

        // Check if any of these topics is in the current domain AND shares the data type
        EntityId topic_id;
        for (const auto& topic_id_ : topic_ids)
        {
            if (topic_id_.first == info.domain_id)
            {
                if (database_->is_topic_in_database(info.type_name, topic_id_.second))
                {
                    topic_id = topic_id_.second;
                    //Found the correct topic
                    break;
                }
            }
        }

        if (database_->update_endpoint_in_graph(info.domain_id, participant_id.second, topic_id, datawriter_id))
        {
            details::StatisticsBackendData::get_instance()->on_domain_view_graph_update(info.domain_id);
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

    // Check whether the topic is already in the database
    auto topic_ids = database_->get_entities_by_name(EntityKind::TOPIC, info.topic_name);

    // Check if any of these topics is in the current domain AND shares the data type
    EntityId topic_id;
    for (const auto& topic_pair : topic_ids)
    {
        if (topic_pair.first == info.domain_id)
        {
            if (database_->is_topic_in_database(info.type_name, topic_pair.second))
            {
                topic_id = topic_pair.second;
                //Found the correct topic
                break;
            }
        }
    }

    // If no such topic exists, create a new one
    if (topic_id == EntityId::invalid())
    {
        topic_id = database_->insert_new_topic(info.topic_name, info.type_name, info.alias, info.domain_id);

        details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
            info.domain_id,
            topic_id,
            EntityKind::TOPIC,
            details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
    }

    // Create the endpoint
    EntityId endpoint_id;
    std::stringstream name;

    if (info.kind() == EntityKind::DATAREADER)
    {
        name << "DataReader_" << info.topic_name << "_" << info.guid.entityId;
    }
    else
    {
        name << "DataWriter_" << info.topic_name << "_" << info.guid.entityId;
    }

    // Endpoint AppId and metadata
    // TODO: get app data from info (parameters), not from participant
    std::pair<AppId, std::string> app_data;
    auto participant = std::static_pointer_cast<const DomainParticipant>(database_->get_entity(participant_id.second));
    if (participant != nullptr)
    {
        app_data.first = participant->app_id;
    }
    else
    {
        app_data.first = AppId::UNKNOWN;
    }
    app_data.second = "";

    endpoint_id = database_->insert_new_endpoint(
        to_string(endpoint_guid),
        name.str(),
        info.alias,
        info.qos,
        info.is_virtual_metatraffic,
        info.locators,
        info.kind(),
        participant_id.second,
        topic_id,
        app_data);

    // Force the refresh of the parent entities' status
    database_->change_entity_status(endpoint_id, true);

    if (database_->update_endpoint_in_graph(info.domain_id, participant_id.second, topic_id, endpoint_id))
    {
        details::StatisticsBackendData::get_instance()->on_domain_view_graph_update(info.domain_id);
    }
    return endpoint_id;
}

template <typename T>
EntityId DatabaseDataQueue<T>::get_or_create_locator(
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
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
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
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
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
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
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
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
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
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
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
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
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
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
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
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
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

template<>
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        ProxySample& sample,
        const std::vector<uint8_t>& item) const
{
    EntityKind entity_kind = database_->get_entity_kind_by_guid(local_entity_guid);

    sample.entity_proxy = item;
    sample.kind = StatusKind::PROXY;
    sample.status = StatusLevel::OK;

    std::string guid = deserialize_guid(local_entity_guid);

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
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        ConnectionListSample& sample,
        const std::vector<StatisticsConnection>& item) const
{
    EntityKind entity_kind = database_->get_entity_kind_by_guid(local_entity_guid);

    sample.connection_list = item;
    sample.kind = StatusKind::CONNECTION_LIST;
    sample.status = StatusLevel::OK;

    std::string guid = deserialize_guid(local_entity_guid);

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
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        IncompatibleQosSample& sample,
        const StatisticsIncompatibleQoSStatus& item) const
{
    EntityKind entity_kind = database_->get_entity_kind_by_guid(local_entity_guid);

    sample.incompatible_qos_status = item;
    sample.kind = StatusKind::INCOMPATIBLE_QOS;

    if (item.total_count())
    {
        sample.status = StatusLevel::ERROR;
    }
    else
    {
        sample.status = StatusLevel::OK;
    }

    std::string guid = deserialize_guid(local_entity_guid);

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
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        InconsistentTopicSample& sample,
        const StatisticsInconsistentTopicStatus& item) const
{
    EntityKind entity_kind = database_->get_entity_kind_by_guid(local_entity_guid);

    sample.inconsistent_topic_status = item;
    sample.kind = StatusKind::INCONSISTENT_TOPIC;

    // Appropriate behavior not yet implemented
    logWarning(BACKEND_DATABASE_QUEUE,
            "Warning processing INCONSISTENT_TOPIC status data. Status behavior not yet defined");
    sample.status = StatusLevel::OK;

    std::string guid = deserialize_guid(local_entity_guid);

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
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        LivelinessLostSample& sample,
        const StatisticsLivelinessLostStatus& item) const
{
    EntityKind entity_kind = database_->get_entity_kind_by_guid(local_entity_guid);

    sample.liveliness_lost_status = item;
    sample.kind = StatusKind::LIVELINESS_LOST;

    if (item.total_count())
    {
        sample.status = StatusLevel::WARNING;
    }
    else
    {
        sample.status = StatusLevel::OK;
    }

    std::string guid = deserialize_guid(local_entity_guid);

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
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        LivelinessChangedSample& sample,
        const StatisticsLivelinessChangedStatus& item) const
{
    EntityKind entity_kind = database_->get_entity_kind_by_guid(local_entity_guid);

    sample.liveliness_changed_status = item;
    sample.kind = StatusKind::LIVELINESS_CHANGED;
    sample.status = StatusLevel::OK;

    std::string guid = deserialize_guid(local_entity_guid);

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
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        DeadlineMissedSample& sample,
        const StatisticsDeadlineMissedStatus& item) const
{
    EntityKind entity_kind = database_->get_entity_kind_by_guid(local_entity_guid);

    sample.deadline_missed_status = item;
    sample.kind = StatusKind::DEADLINE_MISSED;


    if (item.total_count())
    {
        sample.status = StatusLevel::ERROR;
    }
    else
    {
        sample.status = StatusLevel::OK;
    }

    std::string guid = deserialize_guid(local_entity_guid);

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
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        SampleLostSample& sample,
        const StatisticsSampleLostStatus& item) const
{
    EntityKind entity_kind = database_->get_entity_kind_by_guid(local_entity_guid);

    sample.sample_lost_status = item;
    sample.kind = StatusKind::SAMPLE_LOST;


    if (item.total_count())
    {
        sample.status = StatusLevel::ERROR;
    }
    else
    {
        sample.status = StatusLevel::OK;
    }

    std::string guid = deserialize_guid(local_entity_guid);

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
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample()
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

        default:
        {
            break;
        }
    }
}

template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample()
{
    EntityId domain;
    EntityId entity;
    bool updated_entity = false;

    switch (front().second->status_kind())
    {
        case StatisticsStatusKind::PROXY:
        {
            ProxySample sample;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, item.second->local_entity(), sample,
                        item.second->value().entity_proxy());

                updated_entity = database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_status_reported(domain, entity, StatusKind::PROXY);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing PROXY status data. Data was not added to the statistics collection: "
                        + std::string(
                            e.what()));
            }
            break;
        }
        case StatisticsStatusKind::CONNECTION_LIST:
        {
            ConnectionListSample sample;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, item.second->local_entity(), sample,
                        item.second->value().connection_list());

                updated_entity = database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_status_reported(domain, entity,
                        StatusKind::CONNECTION_LIST);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing CONNECTION_LIST status data. Data was not added to the statistics collection: "
                        + std::string(
                            e.what()));
            }
            break;
        }
        case StatisticsStatusKind::INCOMPATIBLE_QOS:
        {
            IncompatibleQosSample sample;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, item.second->local_entity(), sample,
                        item.second->value().incompatible_qos_status());

                updated_entity = database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_status_reported(domain, entity,
                        StatusKind::INCOMPATIBLE_QOS);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing INCOMPATIBLE_QOS status data. Data was not added to the statistics collection: "
                        + std::string(
                            e.what()));
            }
            break;
        }
        case StatisticsStatusKind::INCONSISTENT_TOPIC:
        {
            InconsistentTopicSample sample;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, item.second->local_entity(), sample,
                        item.second->value().inconsistent_topic_status());

                updated_entity = database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_status_reported(domain, entity,
                        StatusKind::INCONSISTENT_TOPIC);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing INCONSISTENT_TOPIC status data. Data was not added to the statistics collection: "
                        + std::string(
                            e.what()));
            }
            break;
        }
        case StatisticsStatusKind::LIVELINESS_LOST:
        {
            LivelinessLostSample sample;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, item.second->local_entity(), sample,
                        item.second->value().liveliness_lost_status());

                updated_entity = database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_status_reported(domain, entity,
                        StatusKind::LIVELINESS_LOST);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing LIVELINESS_LOST status data. Data was not added to the statistics collection: "
                        + std::string(
                            e.what()));
            }
            break;
        }
        case StatisticsStatusKind::LIVELINESS_CHANGED:
        {
            LivelinessChangedSample sample;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, item.second->local_entity(), sample,
                        item.second->value().liveliness_changed_status());

                updated_entity = database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_status_reported(domain, entity,
                        StatusKind::LIVELINESS_CHANGED);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing LIVELINESS_CHANGED status data. Data was not added to the statistics collection: "
                        + std::string(
                            e.what()));
            }
            break;
        }
        case StatisticsStatusKind::DEADLINE_MISSED:
        {
            DeadlineMissedSample sample;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, item.second->local_entity(), sample,
                        item.second->value().deadline_missed_status());

                updated_entity = database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_status_reported(domain, entity,
                        StatusKind::DEADLINE_MISSED);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing DEADLINE_MISSED status data. Data was not added to the statistics collection: "
                        + std::string(
                            e.what()));
            }
            break;
        }
        case StatisticsStatusKind::SAMPLE_LOST:
        {
            SampleLostSample sample;
            queue_item_type item = front();
            sample.src_ts = item.first;
            try
            {
                process_sample_type(domain, entity, item.second->local_entity(), sample,
                        item.second->value().sample_lost_status());

                updated_entity = database_->insert(domain, entity, sample);
                details::StatisticsBackendData::get_instance()->on_status_reported(domain, entity,
                        StatusKind::SAMPLE_LOST);
            }
            catch (const eprosima::statistics_backend::Exception& e)
            {
                logWarning(BACKEND_DATABASE_QUEUE,
                        "Error processing SAMPLE_LOST status data. Data was not added to the statistics collection: "
                        + std::string(
                            e.what()));
            }
            break;
        }
        case StatisticsStatusKind::STATUSES_SIZE:
        {
            //Not yet implemented
            logWarning(BACKEND_DATABASE_QUEUE,
                    "Warning processing STATUSES_SIZE status data. Not yet implemented");
            break;
        }
        default:
        {
            break;
        }
    }

    if (updated_entity)
    {
        if (database_->update_graph_on_updated_entity(domain, entity))
        {
            details::StatisticsBackendData::get_instance()->on_domain_view_graph_update(domain);
        }
    }
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
