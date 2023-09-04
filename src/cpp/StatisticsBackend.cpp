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
#include <stdlib.h>
#include <string>
#include <set>

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
#include <fastdds/rtps/attributes/RTPSParticipantAttributes.h>
#include <fastdds/rtps/attributes/ServerAttributes.h>
#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/transport/UDPv4TransportDescriptor.h>
#include <fastdds/statistics/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/statistics/topic_names.hpp>
#include <fastrtps/utils/IPLocator.h>

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
#include "detail/ScopeExit.hpp"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::statistics;
using namespace eprosima::statistics_backend::details;

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
        details::Monitor& monitor,
        const std::string& topic_name,
        const TypeSupport& type)
{
    // Find if the topic has been already created and if the associated type is correct
    TopicDescription* topic_desc = monitor.participant->lookup_topicdescription(topic_name);
    if (nullptr != topic_desc)
    {
        if (topic_desc->get_type_name() != type->getName())
        {
            throw Error(topic_name + " is not using expected type " + type->getName() +
                          " and is using instead type " + topic_desc->get_type_name());
        }

        try
        {
            monitor.topics[topic_name] = dynamic_cast<Topic*>(topic_desc);
        }
        catch (const std::bad_cast& e)
        {
            // TODO[ILG]: Could we support other TopicDescription types in this context?
            throw Error(topic_name + " is already used but is not a simple Topic: " + e.what());
        }

    }
    else
    {
        if (ReturnCode_t::RETCODE_PRECONDITION_NOT_MET == monitor.participant->register_type(type, type->getName()))
        {
            // Name already in use
            throw Error(std::string("Type name ") + type->getName() + " is already in use");
        }
        monitor.topics[topic_name] =
                monitor.participant->create_topic(topic_name, type->getName(), TOPIC_QOS_DEFAULT);
    }
}

void register_statistics_type_and_topic(
        details::Monitor& monitor,
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

EntityId create_and_register_monitor(
        const std::string& domain_name,
        DomainListener* domain_listener,
        const CallbackMask& callback_mask,
        const DataKindMask& data_mask,
        const DomainParticipantQos& participant_qos,
        const DomainId domain_id = 0)
{
    // NOTE: This method is quite awful to read because of the error handle of every entity
    // This could be done much nicer encapsulating this in Monitor creation in destruction, but you know...
    // Why do not call stop_monitor in error case?, youll ask. Well, mutexes are treated rarely here, as this static
    // class locks and unlocks StatisticsBackendData mutex, what makes very difficult to do some coherent
    // calls from one to another.
    // What should happen is that all this logic is moved to StatisticsBackendData. You know, some day...

    auto& backend_data = StatisticsBackendData::get_instance();
    std::lock_guard<details::StatisticsBackendData> guard(*backend_data);

    /* Create monitor instance and register it in the database */
    std::shared_ptr<database::Domain> domain = std::make_shared<database::Domain>(domain_name);
    domain->id = backend_data->database_->insert(domain);

    // TODO: in case this function fails afterwards, the domain will be kept in the database without associated
    // Participant. There must exist a way in database to delete a domain, or to make a rollback.

    // Create monitor and set its variables
    details::StatisticsBackendData::get_instance()->monitors_by_entity_[domain->id] =
            std::make_unique<details::Monitor>();
    std::unique_ptr<details::Monitor>& monitor =
            details::StatisticsBackendData::get_instance()->monitors_by_entity_[domain->id];

    monitor->id = domain->id;
    monitor->domain_listener = domain_listener;
    monitor->domain_callback_mask = callback_mask;
    monitor->data_mask = data_mask;
    auto se_erase_monitor_database_ =
            EPROSIMA_BACKEND_MAKE_SCOPE_EXIT(backend_data->monitors_by_entity_.erase(domain->id));

    monitor->participant_listener = new subscriber::StatisticsParticipantListener(
        domain->id,
        backend_data->database_.get(),
        backend_data->entity_queue_,
        backend_data->data_queue_);
    auto se_participant_listener_ = EPROSIMA_BACKEND_MAKE_SCOPE_EXIT(delete monitor->participant_listener);

    monitor->reader_listener = new subscriber::StatisticsReaderListener(
        backend_data->data_queue_);
    auto se_reader_listener_ = EPROSIMA_BACKEND_MAKE_SCOPE_EXIT(delete monitor->reader_listener);

    /* Create DomainParticipant */
    StatusMask participant_mask = StatusMask::all();
    participant_mask ^= StatusMask::data_on_readers();
    monitor->participant = DomainParticipantFactory::get_instance()->create_participant(
        domain_id,
        participant_qos,
        monitor->participant_listener,
        participant_mask);

    if (monitor->participant == nullptr)
    {
        throw Error("Error initializing monitor. Could not create participant");
    }
    auto se_participant_ =
            EPROSIMA_BACKEND_MAKE_SCOPE_EXIT(
        DomainParticipantFactory::get_instance()->delete_participant(monitor->participant));

    /* Create Subscriber */
    monitor->subscriber = monitor->participant->create_subscriber(
        SUBSCRIBER_QOS_DEFAULT,
        nullptr,
        StatusMask::none());

    if (monitor->subscriber == nullptr)
    {
        throw Error("Error initializing monitor. Could not create subscriber");
    }
    auto se_subscriber_ =
            EPROSIMA_BACKEND_MAKE_SCOPE_EXIT(monitor->participant->delete_subscriber(monitor->subscriber));

    auto se_topics_datareaders_ =
            EPROSIMA_BACKEND_MAKE_SCOPE_EXIT(
            {
                for (auto& it : monitor->readers)
                {
                    if (nullptr != it.second)
                    {
                        monitor->subscriber->delete_datareader(it.second);
                    }
                }
                for (auto& it : monitor->topics)
                {
                    if (nullptr != it.second)
                    {
                        monitor->participant->delete_topic(it.second);
                    }
                }
            }
        );

    for (const auto& topic : topics)
    {
        /* Register the type and topic*/
        try
        {
            register_statistics_type_and_topic(*monitor, topic);
        }
        catch (const std::exception& e)
        {
            throw Error("Error registering topic " + std::string(topic) + " : " + e.what());
        }

        /* Create DataReaders */
        monitor->readers[topic] = monitor->subscriber->create_datareader(
            monitor->topics[topic],
            eprosima::fastdds::statistics::dds::STATISTICS_DATAREADER_QOS,
            monitor->reader_listener,
            StatusMask::all());

        if (monitor->readers[topic] == nullptr)
        {
            throw Error("Error initializing monitor. Could not create reader for topic " + std::string(topic));
        }
    }

    se_erase_monitor_database_.cancel();
    se_participant_listener_.cancel();
    se_reader_listener_.cancel();
    se_participant_.cancel();
    se_subscriber_.cancel();
    se_topics_datareaders_.cancel();

    return domain->id;
}

void StatisticsBackend::set_physical_listener(
        PhysicalListener* listener,
        CallbackMask callback_mask,
        DataKindMask data_mask)
{
    auto& backend_data = StatisticsBackendData::get_instance();
    std::lock_guard<StatisticsBackendData> guard(*backend_data);

    backend_data->physical_listener_ = listener;
    backend_data->physical_callback_mask_ = callback_mask;
    backend_data->physical_data_mask_ = data_mask;
}

void StatisticsBackend::set_domain_listener(
        EntityId monitor_id,
        DomainListener* listener,
        CallbackMask callback_mask,
        DataKindMask data_mask)
{
    auto& statistics_backend_data = StatisticsBackendData::get_instance();
    auto monitor = statistics_backend_data->monitors_by_entity_.find(monitor_id);
    if (monitor == statistics_backend_data->monitors_by_entity_.end())
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
    /* Deactivate statistics in case they were set */
#ifdef _WIN32
    _putenv_s("FASTDDS_STATISTICS=", "");
#else
    unsetenv("FASTDDS_STATISTICS");
#endif // ifdef _WIN32

    /* Set domain_name */
    std::stringstream domain_name;
    domain_name << domain_id;

    /* Set DomainParticipantQoS */
    /* Since configuring the default Qos from an XML is a posibility, we need to load the XML profiles just in case */
    DomainParticipantFactory::get_instance()->load_profiles();
    DomainParticipantQos participant_qos = DomainParticipantFactory::get_instance()->get_default_participant_qos();
    /* Previous string conversion is needed for string_255 */
    std::string participant_name = "monitor_domain_" + std::to_string(domain_id);
    participant_qos.name(participant_name);
    if (participant_qos.transport().use_builtin_transports)
    {
        /* Avoid using SHM transport by default */
        std::shared_ptr<eprosima::fastdds::rtps::UDPv4TransportDescriptor> udp_transport =
                std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
        participant_qos.transport().user_transports.push_back(udp_transport);
        participant_qos.transport().use_builtin_transports = false;
    }

    return create_and_register_monitor(domain_name.str(), domain_listener, callback_mask, data_mask, participant_qos,
                   domain_id);
}

void StatisticsBackend::stop_monitor(
        EntityId monitor_id)
{
    StatisticsBackendData::get_instance()->stop_monitor(monitor_id);
}

EntityId StatisticsBackend::init_monitor(
        std::string discovery_server_locators,
        DomainListener* domain_listener,
        CallbackMask callback_mask,
        DataKindMask data_mask)
{
    return init_monitor(DEFAULT_ROS2_SERVER_GUIDPREFIX, discovery_server_locators, domain_listener, callback_mask,
                   data_mask);
}

EntityId StatisticsBackend::init_monitor(
        std::string discovery_server_guid_prefix,
        std::string discovery_server_locators,
        DomainListener* domain_listener,
        CallbackMask callback_mask,
        DataKindMask data_mask)
{
    /* Deactivate statistics in case they were set */
#ifdef _WIN32
    _putenv_s("FASTDDS_STATISTICS=", "");
#else
    unsetenv("FASTDDS_STATISTICS");
#endif // ifdef _WIN32

    /* Set DomainParticipantQoS */
    /* Since configuring the default Qos from an XML is a posibility, we need to load the XML profiles just in case */
    DomainParticipantFactory::get_instance()->load_profiles();
    DomainParticipantQos participant_qos = DomainParticipantFactory::get_instance()->get_default_participant_qos();
    participant_qos.name("monitor_discovery_server_" + discovery_server_guid_prefix);

    /* Avoid using SHM transport by default */
    std::shared_ptr<eprosima::fastdds::rtps::UDPv4TransportDescriptor> udp_transport =
            std::make_shared<eprosima::fastdds::rtps::UDPv4TransportDescriptor>();
    participant_qos.transport().user_transports.push_back(udp_transport);
    participant_qos.transport().use_builtin_transports = false;

    participant_qos.wire_protocol().builtin.discovery_config.discoveryProtocol =
            eprosima::fastrtps::rtps::DiscoveryProtocol_t::SUPER_CLIENT;
    RemoteServerAttributes server;
    // Set the server guidPrefix
    server.ReadguidPrefix(discovery_server_guid_prefix.c_str());
    // Add locators
    std::stringstream locators(discovery_server_locators);
    std::string locator_str;
    while (std::getline(locators, locator_str, ';'))
    {
        std::stringstream ss(locator_str);
        eprosima::fastrtps::rtps::Locator_t locator;
        ss >> locator;
        if (!IsLocatorValid(locator) || !IsAddressDefined(locator) || ss.rdbuf()->in_avail() != 0)
        {
            throw BadParameter("Invalid locator format: " + locator_str);
        }
        if (locator.port > std::numeric_limits<uint32_t>::max())
        {
            throw BadParameter(std::to_string(locator.port) + " is out of range");
        }

        // Check unicast/multicast address
        if (eprosima::fastrtps::rtps::IPLocator::isMulticast(locator))
        {
            server.metatrafficMulticastLocatorList.push_back(locator);
        }
        else
        {
            server.metatrafficUnicastLocatorList.push_back(locator);
        }
    }
    participant_qos.wire_protocol().builtin.discovery_config.m_DiscoveryServers.push_back(server);

    return create_and_register_monitor(discovery_server_guid_prefix, domain_listener, callback_mask, data_mask,
                   participant_qos);
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
    return StatisticsBackendData::get_instance()->database_->get_entity_ids(entity_type, entity_id);
}

bool StatisticsBackend::is_active(
        EntityId entity_id)
{
    return StatisticsBackendData::get_instance()->database_->get_entity(entity_id)->active;
}

bool StatisticsBackend::is_metatraffic(
        EntityId entity_id)
{
    return StatisticsBackendData::get_instance()->database_->get_entity(entity_id)->metatraffic;
}

EntityKind StatisticsBackend::get_type(
        EntityId entity_id)
{
    return StatisticsBackendData::get_instance()->database_->get_entity_kind(entity_id);
}

EntityStatus StatisticsBackend::get_status(
        EntityId entity_id)
{
    return StatisticsBackendData::get_instance()->database_->get_entity_status(entity_id);
}

Info StatisticsBackend::get_info(
        EntityId entity_id)
{
    Info info = Info::object();

    std::shared_ptr<const database::Entity> entity =
            StatisticsBackendData::get_instance()->database_->get_entity(entity_id);

    info[ID_INFO_TAG] = entity_id.value();
    info[KIND_INFO_TAG] = entity_kind_str[(int)entity->kind];
    info[NAME_INFO_TAG] = entity->name;
    info[ALIAS_INFO_TAG] = entity->alias;
    info[ALIVE_INFO_TAG] = entity->active;
    info[METATRAFFIC_INFO_TAG] = entity->metatraffic;
    info[STATUS_INFO_TAG] = entity->status;

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
            std::set<std::string> locator_set;

            // Writers registered in the participant
            for (const auto& writer : participant->data_writers)
            {
                // Locators associated to each writer
                for (const auto& locator : writer.second.get()->locators)
                {
                    locator_set.insert(locator.second.get()->name);
                }
            }

            // Readers registered in the participant
            for (const auto& reader : participant->data_readers)
            {
                // Locators associated to each reader
                for (const auto& locator : reader.second.get()->locators)
                {
                    locator_set.insert(locator.second.get()->name);
                }
            }

            DatabaseDump locators = DatabaseDump::array();
            for (const auto& locator : locator_set)
            {
                locators.push_back(locator);
            }
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
    database::Database* db = StatisticsBackendData::get_instance()->database_.get();
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
    database::Database* db = StatisticsBackendData::get_instance()->database_.get();
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
        the_initial_time(),
        now(),
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
        the_initial_time(),
        now(),
        statistic);
}

Graph StatisticsBackend::get_graph()
{
    return Graph();
}

DatabaseDump StatisticsBackend::dump_database(
        const bool clear)
{
    return StatisticsBackendData::get_instance()->database_->dump_database(clear);
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

void StatisticsBackend::clear_statistics_data(
        const Timestamp& t_to /* = the_end_of_time() */)
{
    details::StatisticsBackendData::get_instance()->database_->clear_statistics_data(t_to);
}

void StatisticsBackend::clear_inactive_entities()
{
    details::StatisticsBackendData::get_instance()->database_->clear_inactive_entities();
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

    StatisticsBackendData::get_instance()->database_->load_database(dump);
}

void StatisticsBackend::reset()
{
    if (!StatisticsBackendData::get_instance()->monitors_by_entity_.empty())
    {
        std::stringstream message;
        message << "The following monitors are still active: [ ";
        for (const auto& monitor : StatisticsBackendData::get_instance()->monitors_by_entity_)
        {
            message << monitor.first << " ";
        }
        message << "]";
        throw PreconditionNotMet(message.str());
    }
    StatisticsBackendData::get_instance()->reset_instance();
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
             {std::pair<EntityKind, EntityKind> (EntityKind::PARTICIPANT, EntityKind::LOCATOR)})},

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
            StatisticsBackendData::get_instance()->database_->get_entity(entity_id);
    std::shared_ptr<database::Entity> entity = std::const_pointer_cast<database::Entity>(const_entity);
    entity->alias = alias;
}

} // namespace statistics_backend
} // namespace eprosima
