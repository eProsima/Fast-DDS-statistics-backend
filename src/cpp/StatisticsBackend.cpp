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
 * @file StatisticsBackend.cpp
 */

#include <algorithm>
#include <fstream>
#include <sstream>

#include <fastdds/dds/core/status/StatusMask.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/statistics/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/statistics/topic_names.hpp>

#include <fastdds_statistics_backend/StatisticsBackend.hpp>
#include <fastdds_statistics_backend/types/JSONTags.h>

#include <database/database_queue.hpp>
#include <database/database.hpp>
#include <subscriber/StatisticsParticipantListener.hpp>
#include <subscriber/StatisticsReaderListener.hpp>
#include <topic_types/typesPubSubTypes.h>
#include "Monitor.hpp"
#include "StatisticsBackendData.hpp"
#include "detail/data_getters.hpp"
#include "detail/data_aggregation.hpp"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::statistics;

namespace eprosima {
namespace statistics_backend {


static const char* topics[] =
{
    HISTORY_LATENCY_TOPIC,
    NETWORK_LATENCY_TOPIC,
    PUBLICATION_THROUGHPUT_TOPIC,
    SUBSCRIPTION_THROUGHPUT_TOPIC,
    RTPS_SENT_TOPIC,
    RTPS_LOST_TOPIC,
    RESENT_DATAS_TOPIC,
    HEARTBEAT_COUNT_TOPIC,
    ACKNACK_COUNT_TOPIC,
    NACKFRAG_COUNT_TOPIC,
    GAP_COUNT_TOPIC,
    DATA_COUNT_TOPIC,
    PDP_PACKETS_TOPIC,
    EDP_PACKETS_TOPIC,
    DISCOVERY_TOPIC,
    SAMPLE_DATAS_TOPIC,
    PHYSICAL_DATA_TOPIC
};

void find_or_create_topic_and_type(
        std::shared_ptr<details::Monitor> monitor,
        const std::string& topic_name,
        const TypeSupport& type)
{
    // Find if the topic has been already created and if the associated type is correct
    TopicDescription* topic_desc = monitor->participant->lookup_topicdescription(topic_name);
    if (nullptr != topic_desc)
    {
        if (topic_desc->get_type_name() != type->getName())
        {
            throw Error(topic_name + " is not using expected type " + type->getName() +
                          " and is using instead type " + topic_desc->get_type_name());
        }

        try
        {
            monitor->topics[topic_name] = dynamic_cast<Topic*>(topic_desc);
        }
        catch (const std::bad_cast& e)
        {
            // TODO[ILG]: Could we support other TopicDescription types in this context?
            throw Error(topic_name + " is already used but is not a simple Topic: " + e.what());
        }

    }
    else
    {
        if (ReturnCode_t::RETCODE_PRECONDITION_NOT_MET == monitor->participant->register_type(type, type->getName()))
        {
            // Name already in use
            throw Error(std::string("Type name ") + type->getName() + " is already in use");
        }
        monitor->topics[topic_name] =
                monitor->participant->create_topic(topic_name, type->getName(), TOPIC_QOS_DEFAULT);
    }
}

void register_statistics_type_and_topic(
        std::shared_ptr<details::Monitor> monitor,
        const std::string& topic_name)
{
    if (HISTORY_LATENCY_TOPIC == topic_name)
    {
        TypeSupport history_latency_type(new WriterReaderDataPubSubType);
        find_or_create_topic_and_type(monitor, topic_name, history_latency_type);
    }
    else if (NETWORK_LATENCY_TOPIC == topic_name)
    {
        TypeSupport network_latency_type(new Locator2LocatorDataPubSubType);
        find_or_create_topic_and_type(monitor, topic_name, network_latency_type);
    }
    else if (PUBLICATION_THROUGHPUT_TOPIC == topic_name || SUBSCRIPTION_THROUGHPUT_TOPIC == topic_name)
    {
        TypeSupport throughput_type(new EntityDataPubSubType);
        find_or_create_topic_and_type(monitor, topic_name, throughput_type);
    }
    else if (RTPS_SENT_TOPIC == topic_name || RTPS_LOST_TOPIC == topic_name)
    {
        TypeSupport rtps_traffic_type(new Entity2LocatorTrafficPubSubType);
        find_or_create_topic_and_type(monitor, topic_name, rtps_traffic_type);
    }
    else if (RESENT_DATAS_TOPIC == topic_name || HEARTBEAT_COUNT_TOPIC == topic_name ||
            ACKNACK_COUNT_TOPIC == topic_name || NACKFRAG_COUNT_TOPIC == topic_name ||
            GAP_COUNT_TOPIC == topic_name || DATA_COUNT_TOPIC == topic_name ||
            PDP_PACKETS_TOPIC == topic_name || EDP_PACKETS_TOPIC == topic_name)
    {
        TypeSupport count_type(new EntityCountPubSubType);
        find_or_create_topic_and_type(monitor, topic_name, count_type);
    }
    else if (DISCOVERY_TOPIC == topic_name)
    {
        TypeSupport discovery_type(new DiscoveryTimePubSubType);
        find_or_create_topic_and_type(monitor, topic_name, discovery_type);
    }
    else if (SAMPLE_DATAS_TOPIC == topic_name)
    {
        TypeSupport sample_identity_count_type(new SampleIdentityCountPubSubType);
        find_or_create_topic_and_type(monitor, topic_name, sample_identity_count_type);
    }
    else if (PHYSICAL_DATA_TOPIC == topic_name)
    {
        TypeSupport physical_data_type(new PhysicalDataPubSubType);
        find_or_create_topic_and_type(monitor, topic_name, physical_data_type);
    }
}

void StatisticsBackend::set_physical_listener(
        PhysicalListener* listener,
        CallbackMask callback_mask,
        DataKindMask data_mask)
{
    details::StatisticsBackendData::get_instance()->lock();
    details::StatisticsBackendData::get_instance()->physical_listener_ = listener;
    details::StatisticsBackendData::get_instance()->physical_callback_mask_ = callback_mask;
    details::StatisticsBackendData::get_instance()->physical_data_mask_ = data_mask;
    details::StatisticsBackendData::get_instance()->unlock();
}

void StatisticsBackend::set_domain_listener(
        EntityId monitor_id,
        DomainListener* listener,
        CallbackMask callback_mask,
        DataKindMask data_mask)
{
    auto monitor = details::StatisticsBackendData::get_instance()->monitors_by_entity_.find(monitor_id);
    if (monitor == details::StatisticsBackendData::get_instance()->monitors_by_entity_.end())
    {
        throw BadParameter("There is no monitor with the given ID");
    }

    monitor->second->domain_listener = listener;
    monitor->second->domain_callback_mask = callback_mask;
    monitor->second->data_mask = data_mask;
}

EntityId StatisticsBackend::init_monitor(
        DomainId domain_id,
        DomainListener* domain_listener,
        CallbackMask callback_mask,
        DataKindMask data_mask)
{
    details::StatisticsBackendData::get_instance()->lock();

    /* Create monitor instance and register it in the database */
    std::shared_ptr<details::Monitor> monitor = std::make_shared<details::Monitor>();
    std::stringstream domain_name;
    domain_name << domain_id;
    std::shared_ptr<database::Domain> domain = std::make_shared<database::Domain>(domain_name.str());
    try
    {
        domain->id = details::StatisticsBackendData::get_instance()->database_->insert(domain);
    }
    catch (const std::exception&)
    {
        details::StatisticsBackendData::get_instance()->unlock();
        throw;
    }

    monitor->id = domain->id;
    monitor->domain_listener = domain_listener;
    monitor->domain_callback_mask = callback_mask;
    monitor->data_mask = data_mask;
    details::StatisticsBackendData::get_instance()->monitors_by_entity_[domain->id] = monitor;

    monitor->participant_listener = new subscriber::StatisticsParticipantListener(
        domain->id,
        details::StatisticsBackendData::get_instance()->database_.get(),
        details::StatisticsBackendData::get_instance()->entity_queue_,
        details::StatisticsBackendData::get_instance()->data_queue_);
    monitor->reader_listener = new subscriber::StatisticsReaderListener(
        details::StatisticsBackendData::get_instance()->data_queue_);

    /* Create DomainParticipant */
    DomainParticipantQos participant_qos = DomainParticipantFactory::get_instance()->get_default_participant_qos();
    /* Previous string conversion is needed for string_255 */
    std::string participant_name = "monitor_domain_" + std::to_string(domain_id);
    participant_qos.name(participant_name);
    /* Avoid using SHM transport by default */
    std::shared_ptr<eprosima::fastdds::rtps::UDPv4TransportDescriptor> udp_transport =
            std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
    participant_qos.transport().user_transports.push_back(udp_transport);
    participant_qos.transport().use_builtin_transports = false;

    StatusMask participant_mask = StatusMask::all();
    participant_mask ^= StatusMask::data_on_readers();
    monitor->participant = DomainParticipantFactory::get_instance()->create_participant(
        domain_id,
        participant_qos,
        monitor->participant_listener,
        participant_mask);

    if (monitor->participant == nullptr)
    {
        details::StatisticsBackendData::get_instance()->unlock();
        throw Error("Error initializing monitor. Could not create participant");
    }

    /* Create Subscriber */
    monitor->subscriber = monitor->participant->create_subscriber(
        SUBSCRIBER_QOS_DEFAULT,
        nullptr,
        StatusMask::none());

    if (monitor->subscriber == nullptr)
    {
        details::StatisticsBackendData::get_instance()->unlock();
        throw Error("Error initializing monitor. Could not create subscriber");
    }

    for (const auto& topic : topics)
    {
        /* Register the type and topic*/
        register_statistics_type_and_topic(monitor, topic);

        if (monitor->topics[topic] == nullptr)
        {
            details::StatisticsBackendData::get_instance()->unlock();
            throw Error("Error initializing monitor. Could not create topic " + std::string(topic));
        }

        /* Create DataReaders */
        monitor->readers[topic] = monitor->subscriber->create_datareader(
            monitor->topics[topic],
            eprosima::fastdds::statistics::dds::STATISTICS_DATAREADER_QOS,
            monitor->reader_listener,
            StatusMask::all());

        if (monitor->readers[topic] == nullptr)
        {
            details::StatisticsBackendData::get_instance()->unlock();
            throw Error("Error initializing monitor. Could not create reader for topic " + std::string(topic));
        }
    }

    details::StatisticsBackendData::get_instance()->unlock();
    return domain->id;
}

void StatisticsBackend::stop_monitor(
        EntityId monitor_id)
{
    details::StatisticsBackendData::get_instance()->lock();

    //Find the monitor
    auto it = details::StatisticsBackendData::get_instance()->monitors_by_entity_.find(monitor_id);
    if (it == details::StatisticsBackendData::get_instance()->monitors_by_entity_.end())
    {
        details::StatisticsBackendData::get_instance()->unlock();
        throw BadParameter("No monitor with such ID");
    }
    auto monitor = it->second;
    details::StatisticsBackendData::get_instance()->monitors_by_entity_.erase(it);

    // Delete everything created during monitor initialization
    for (const auto& reader : monitor->readers)
    {
        monitor->subscriber->delete_datareader(reader.second);
    }
    monitor->readers.clear();

    for (const auto& topic : monitor->topics)
    {
        monitor->participant->delete_topic(topic.second);
    }
    monitor->topics.clear();

    monitor->participant->delete_subscriber(monitor->subscriber);
    DomainParticipantFactory::get_instance()->delete_participant(monitor->participant);
    delete monitor->reader_listener;
    delete monitor->participant_listener;

    // The monitor is inactive
    details::StatisticsBackendData::get_instance()->database_->change_entity_status(monitor_id, false);

    details::StatisticsBackendData::get_instance()->unlock();
}

EntityId StatisticsBackend::init_monitor(
        std::string discovery_server_locators,
        DomainListener* domain_listener,
        CallbackMask callback_mask,
        DataKindMask data_mask)
{
    static_cast<void>(discovery_server_locators);
    static_cast<void>(domain_listener);
    static_cast<void>(callback_mask);
    static_cast<void>(data_mask);
    return EntityId();
}

void StatisticsBackend::restart_monitor(
        EntityId monitor_id)
{
    static_cast<void>(monitor_id);
}

void StatisticsBackend::clear_monitor(
        EntityId monitor_id)
{
    static_cast<void>(monitor_id);
}

std::vector<EntityId> StatisticsBackend::get_entities(
        EntityKind entity_type,
        EntityId entity_id)
{
    return details::StatisticsBackendData::get_instance()->database_->get_entity_ids(entity_type, entity_id);
}

bool StatisticsBackend::is_active(
        EntityId entity_id)
{
    return details::StatisticsBackendData::get_instance()->database_->get_entity(entity_id)->active;
}

EntityKind StatisticsBackend::get_type(
        EntityId entity_id)
{
    return details::StatisticsBackendData::get_instance()->database_->get_entity_kind(entity_id);
}

Info StatisticsBackend::get_info(
        EntityId entity_id)
{
    Info info = Info::object();

    std::shared_ptr<const database::Entity> entity =
            details::StatisticsBackendData::get_instance()->database_->get_entity(entity_id);

    info[ID_INFO_TAG] = entity_id.value();
    info[KIND_INFO_TAG] = entity_kind_str[(int)entity->kind];
    info[NAME_INFO_TAG] = entity->name;
    info[ALIAS_INFO_TAG] = entity->alias;
    info[ALIVE_INFO_TAG] = entity->active;

    switch (entity->kind)
    {
        case EntityKind::PROCESS:
        {
            std::shared_ptr<const database::Process> process =
                    std::dynamic_pointer_cast<const database::Process>(entity);
            info[PID_INFO_TAG] = process->pid;
            break;
        }
        case EntityKind::TOPIC:
        {
            std::shared_ptr<const database::Topic> topic =
                    std::dynamic_pointer_cast<const database::Topic>(entity);
            info[DATA_TYPE_INFO_TAG] = topic->data_type;
            break;
        }
        case EntityKind::PARTICIPANT:
        {
            std::shared_ptr<const database::DomainParticipant> participant =
                    std::dynamic_pointer_cast<const database::DomainParticipant>(entity);
            info[GUID_INFO_TAG] = participant->guid;
            info[QOS_INFO_TAG] = participant->qos;

            // Locators associated to endpoints
            DatabaseDump locators = DatabaseDump::array();

            // Writers registered in the participant
            for (const auto& writer : participant->data_writers)
            {
                // Locators associated to each writer
                for (const auto& locator : writer.second.get()->locators)
                {
                    locators.push_back(locator.second.get()->name);
                }
            }

            // Readers registered in the participant
            for (const auto& reader : participant->data_readers)
            {
                // Locators associated to each reader
                for (const auto& locator : reader.second.get()->locators)
                {
                    locators.push_back(locator.second.get()->name);
                }
            }

            // Remove duplicates
            auto last = std::unique(locators.begin(), locators.end(), [](
                                const std::string& first,
                                const std::string& second)
                            {
                                return first.compare(second) == 0;
                            });
            locators.erase(last, locators.end());
            info[LOCATOR_CONTAINER_TAG] = locators;
            break;
        }
        case EntityKind::DATAWRITER:
        case EntityKind::DATAREADER:
        {
            std::shared_ptr<const database::DDSEntity> dds_entity =
                    std::dynamic_pointer_cast<const database::DDSEntity>(entity);
            info[GUID_INFO_TAG] = dds_entity->guid;
            info[QOS_INFO_TAG] = dds_entity->qos;
            break;
        }
        default:
        {
            break;
        }
    }

    return info;
}

static inline void check_entity_kinds(
        EntityKind kind,
        const std::vector<EntityId>& entity_ids,
        database::Database* db,
        const char* message)
{
    for (EntityId id : entity_ids)
    {
        std::shared_ptr<const database::Entity> entity = db->get_entity(id);
        if (!entity || kind != entity->kind)
        {
            throw BadParameter(message);
        }
    }
}

static inline void check_entity_kinds(
        EntityKind kinds[3],
        const std::vector<EntityId>& entity_ids,
        database::Database* db,
        const char* message)
{
    for (EntityId id : entity_ids)
    {
        std::shared_ptr<const database::Entity> entity = db->get_entity(id);
        if (!entity || (kinds[0] != entity->kind && kinds[1] != entity->kind && kinds[2] != entity->kind))
        {
            throw BadParameter(message);
        }
    }
}

std::vector<StatisticsData> StatisticsBackend::get_data(
        DataKind data_type,
        const std::vector<EntityId>& entity_ids_source,
        const std::vector<EntityId>& entity_ids_target,
        uint16_t bins,
        Timestamp t_from,
        Timestamp t_to,
        StatisticKind statistic)
{
    // Validate data_type
    auto allowed_kinds = get_data_supported_entity_kinds(data_type);
    if (1 == allowed_kinds.size() && EntityKind::INVALID == allowed_kinds[0].second)
    {
        throw BadParameter("Method get_data called for source-target entities but data_type requires single entity");
    }

    // Validate timestamps
    auto min_separation = Timestamp::duration(bins);
    if (t_to <= t_from + min_separation)
    {
        throw BadParameter("Invalid timestamps (to should be greater than from by at least bins nanoseconds");
    }

    // Validate entity_ids_source. Note that the only case with more than one pair always has the same source kind.
    EntityKind source_kind = allowed_kinds[0].first;
    database::Database* db = details::StatisticsBackendData::get_instance()->database_.get();
    check_entity_kinds(source_kind, entity_ids_source, db, "Wrong entity id passed in entity_ids_source");

    // Validate entity_ids_target.
    if (1 == allowed_kinds.size())
    {
        EntityKind target_kind = allowed_kinds[0].second;
        check_entity_kinds(target_kind, entity_ids_target, db, "Wrong entity id passed in entity_ids_target");
    }
    else
    {
        // This should be the DISCOVERY_TIME case
        assert(3 == allowed_kinds.size());
        EntityKind target_kinds[3];
        target_kinds[0] = allowed_kinds[0].second;
        target_kinds[1] = allowed_kinds[1].second;
        target_kinds[2] = allowed_kinds[2].second;
        check_entity_kinds(target_kinds, entity_ids_target, db, "Wrong entity id passed in entity_ids_target");
    }

    std::vector<StatisticsData> ret_val;
    auto t_to_select = t_to - Timestamp::duration(1);

    if (0 == bins)
    {
        for (EntityId source_id : entity_ids_source)
        {
            for (EntityId target_id : entity_ids_target)
            {
                auto data = db->select(data_type, source_id, target_id, t_from, t_to_select);
                auto iterators = get_iterators(data_type, data);

                for (auto& it = *iterators.first; it != *iterators.second; ++it)
                {
                    ret_val.emplace_back(it.get_timestamp(), it.get_value());
                }
            }
        }
    }
    else
    {
        auto processor = get_data_aggregator(bins, t_from, t_to, statistic, ret_val);
        for (EntityId source_id : entity_ids_source)
        {
            for (EntityId target_id : entity_ids_target)
            {
                auto data = db->select(data_type, source_id, target_id, t_from, t_to_select);
                auto iterators = get_iterators(data_type, data);
                processor->add_data(iterators);
            }
        }
        processor->finish();
    }

    return ret_val;
}

std::vector<StatisticsData> StatisticsBackend::get_data(
        DataKind data_type,
        const std::vector<EntityId>& entity_ids,
        uint16_t bins,
        Timestamp t_from,
        Timestamp t_to,
        StatisticKind statistic)
{
    // Validate data_type
    auto allowed_kinds = get_data_supported_entity_kinds(data_type);
    if (1 != allowed_kinds.size() || EntityKind::INVALID != allowed_kinds[0].second)
    {
        throw BadParameter("Method get_data called for single entity but data_type requires two entities");
    }

    // Validate timestamps
    auto min_separation = Timestamp::duration(bins);
    if (t_to <= t_from + min_separation)
    {
        throw BadParameter("Invalid timestamps (to should be greater than from by at least bins nanoseconds");
    }

    // Validate entity_ids
    EntityKind allowed_kind = allowed_kinds[0].first;
    database::Database* db = details::StatisticsBackendData::get_instance()->database_.get();
    check_entity_kinds(allowed_kind, entity_ids, db, "Wrong entity id passed in entity_ids");

    std::vector<StatisticsData> ret_val;
    auto t_to_select = t_to - Timestamp::duration(1);

    if (0 == bins)
    {
        for (EntityId id : entity_ids)
        {
            auto data = db->select(data_type, id, t_from, t_to_select);
            auto iterators = get_iterators(data_type, data);

            for (auto& it = *iterators.first; it != *iterators.second; ++it)
            {
                ret_val.emplace_back(it.get_timestamp(), it.get_value());
            }
        }
    }
    else
    {
        auto processor = get_data_aggregator(bins, t_from, t_to, statistic, ret_val);
        for (EntityId id : entity_ids)
        {
            auto data = db->select(data_type, id, t_from, t_to_select);
            auto iterators = get_iterators(data_type, data);
            processor->add_data(iterators);
        }
        processor->finish();
    }

    return ret_val;
}

std::vector<StatisticsData> StatisticsBackend::get_data(
        DataKind data_type,
        const std::vector<EntityId>& entity_ids_source,
        const std::vector<EntityId>& entity_ids_target,
        uint16_t bins,
        StatisticKind statistic)
{
    return get_data(
        data_type,
        entity_ids_source,
        entity_ids_target,
        bins,
        Timestamp(),
        std::chrono::system_clock::now(),
        statistic);
}

std::vector<StatisticsData> StatisticsBackend::get_data(
        DataKind data_type,
        const std::vector<EntityId>& entity_ids,
        uint16_t bins,
        StatisticKind statistic)
{
    return get_data(
        data_type,
        entity_ids,
        bins,
        Timestamp(),
        std::chrono::system_clock::now(),
        statistic);
}

Graph StatisticsBackend::get_graph()
{
    return Graph();
}

DatabaseDump StatisticsBackend::dump_database(
        const bool clear)
{
    return details::StatisticsBackendData::get_instance()->database_->dump_database(clear);
}

void StatisticsBackend::dump_database(
        const std::string& filename,
        const bool clear)
{
    // Open the file
    std::ofstream file(filename);
    if (!file.good())
    {
        throw BadParameter("Error opening file " + filename + " to dump the database");
    }

    // Dump the data
    file << StatisticsBackend::dump_database(clear);
}

void StatisticsBackend::load_database(
        const std::string& filename)
{
    // Check if the file exists
    std::ifstream file(filename);
    if (!file.good())
    {
        throw BadParameter("File " + filename + " does not exist");
    }

    // Get the json file
    DatabaseDump dump;
    file >> dump;

    details::StatisticsBackendData::get_instance()->database_->load_database(dump);
}

void StatisticsBackend::reset()
{
    if (!details::StatisticsBackendData::get_instance()->monitors_by_entity_.empty())
    {
        std::stringstream message;
        message << "The following monitors are still active: [ ";
        for (const auto& monitor : details::StatisticsBackendData::get_instance()->monitors_by_entity_)
        {
            message << monitor.first << " ";
        }
        message << "]";
        throw PreconditionNotMet(message.str());
    }
    details::StatisticsBackendData::get_instance()->reset_instance();
}

std::vector<std::pair<EntityKind, EntityKind>> StatisticsBackend::get_data_supported_entity_kinds(
        DataKind data_kind)
{
    static std::map<DataKind, std::vector<std::pair<EntityKind, EntityKind>>> data_to_entity_map =
    {
        {DataKind::INVALID, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::INVALID, EntityKind::INVALID)})},

        {DataKind::FASTDDS_LATENCY, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::DATAREADER)})},

        {DataKind::NETWORK_LATENCY, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::LOCATOR, EntityKind::LOCATOR)})},

        {DataKind::PUBLICATION_THROUGHPUT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::INVALID)})},

        {DataKind::SUBSCRIPTION_THROUGHPUT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAREADER, EntityKind::INVALID)})},

        {DataKind::RTPS_PACKETS_SENT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::LOCATOR)})},

        {DataKind::RTPS_BYTES_SENT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::LOCATOR)})},

        {DataKind::RTPS_PACKETS_LOST, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::LOCATOR)})},

        {DataKind::RTPS_BYTES_LOST, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::LOCATOR)})},

        {DataKind::RESENT_DATA, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::INVALID)})},

        {DataKind::HEARTBEAT_COUNT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::INVALID)})},

        {DataKind::ACKNACK_COUNT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAREADER, EntityKind::INVALID)})},

        {DataKind::NACKFRAG_COUNT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAREADER, EntityKind::INVALID)})},

        {DataKind::GAP_COUNT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::INVALID)})},

        {DataKind::DATA_COUNT, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::INVALID)})},

        {DataKind::PDP_PACKETS, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::INVALID)})},

        {DataKind::EDP_PACKETS, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::INVALID)})},

        {DataKind::DISCOVERY_TIME, std::vector<std::pair<EntityKind, EntityKind>>(
                    {
                        std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::PARTICIPANT),
                        std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::DATAWRITER),
                        std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::DATAREADER)})},

        {DataKind::SAMPLE_DATAS, std::vector<std::pair<EntityKind, EntityKind>>(
             {std::pair<EntityKind, EntityKind> (EntityKind::DATAWRITER, EntityKind::INVALID)})}
    };

    return data_to_entity_map[data_kind];
}

void StatisticsBackend::set_alias(
        EntityId entity_id,
        const std::string& alias)
{
    std::shared_ptr<const database::Entity> const_entity =
            details::StatisticsBackendData::get_instance()->database_->get_entity(entity_id);
    std::shared_ptr<database::Entity> entity = std::const_pointer_cast<database::Entity>(const_entity);
    entity->alias = alias;
}

} // namespace statistics_backend
} // namespace eprosima
