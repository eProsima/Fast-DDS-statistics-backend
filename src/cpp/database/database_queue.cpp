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
    auto found_remote_locators = database_->get_entities_by_name(EntityKind::LOCATOR, remote_locator);
    // In case that the reported locator is not known, create it without being linked to an endpoint
    if (found_remote_locators.empty())
    {
        std::shared_ptr<Locator> locator = std::make_shared<Locator>(remote_locator);
        locator->id = database_->insert(locator);
        details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                locator->id,
                EntityKind::LOCATOR,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
        sample.remote_locator = locator->id;
    }
    else
    {
        sample.remote_locator = found_remote_locators.front().second;
    }

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
    auto found_remote_locators = database_->get_entities_by_name(EntityKind::LOCATOR, remote_locator);
    if (found_remote_locators.empty())
    {
        throw Error("Locator " + remote_locator + " not found");
    }
    sample.remote_locator = found_remote_locators.front().second;

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
    auto found_remote_locators = database_->get_entities_by_name(EntityKind::LOCATOR, remote_locator);
    if (found_remote_locators.empty())
    {
        throw Error("Locator " + remote_locator + " not found");
    }
    sample.remote_locator = found_remote_locators.front().second;

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
            StatisticsPhysicalData item = front().second->physical_data();

            try
            {
                // Take the ID of the Participant from its GUID
                std::string participant_guid = deserialize_guid(item.participant_guid());
                auto participants = database_->get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid);
                EntityId participant_id = participants.second;

                // Parse the process name and PID
                std::string process_name;
                std::string process_pid;
                size_t separator_pos = item.process().find_last_of(':');
                if (separator_pos == std::string::npos)
                {
                    logInfo(BACKEND_DATABASE_QUEUE,
                            "Process name " + item.process() + " does not follow the [command]:[PID] pattern");
                    process_name = item.process();
                    process_pid = item.process();
                }
                else
                {
                    process_name = item.process().substr(0, separator_pos);
                    process_pid = item.process().substr(separator_pos + 1);
                }

                // Check the existence of the host
                std::shared_ptr<Host> host;
                auto hosts = database_->get_entities_by_name(EntityKind::HOST, item.host());
                if (hosts.empty())
                {
                    host.reset(new Host(item.host()));
                    host->id = database_->insert(std::static_pointer_cast<Entity>(host));
                }
                else
                {
                    // Host name reported by Fast DDS are considered unique
                    std::shared_ptr<const Host> const_host = std::dynamic_pointer_cast<const Host>(database_->get_entity(
                                        hosts.front().second));
                    host = std::const_pointer_cast<Host>(const_host);
                }

                // Check the existence of the user in that host
                std::shared_ptr<User> user;
                auto users = database_->get_entities_by_name(EntityKind::USER, item.user());
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
                    user.reset(new User(item.user(), host));
                    user->id = database_->insert(std::static_pointer_cast<Entity>(user));
                }

                // Check the existence of the process in that user
                EntityId process_id;
                std::shared_ptr<Process> process;
                auto processes = database_->get_entities_by_name(EntityKind::PROCESS, process_name);
                for (const auto& it : processes)
                {
                    std::shared_ptr<const Process> const_process =
                            std::dynamic_pointer_cast<const Process>(database_->get_entity(it.second));

                    // There is only one process with the same name for a given user
                    if (const_process->user == user)
                    {
                        process = std::const_pointer_cast<Process>(const_process);
                        process_id = const_process->id;
                        break;
                    }
                }
                if (!process)
                {
                    process.reset(new Process(process_name, process_pid, user));
                    process_id = database_->insert(std::static_pointer_cast<Entity>(process));
                }

                database_->link_participant_with_process(participant_id, process_id);
            }
            catch (const std::exception& e)
            {
                logError(BACKEND_DATABASE_QUEUE, e.what());
                return;
            }

            break;
        }
    }
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
