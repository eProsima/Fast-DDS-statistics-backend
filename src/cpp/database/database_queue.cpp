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

#include <database/database_queue.hpp>

namespace eprosima {
namespace statistics_backend {
namespace database {

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& entity,
        EntityKind entity_kind,
        HistoryLatencySample& sample,
        const DatabaseQueue::StatisticsWriterReaderData& item) const
{
    sample.data = item.data();
    std::string reader_guid = deserialize_guid(item.reader_guid());
    sample.reader = database_->get_entities_by_guid(entity_kind, reader_guid).front();

    std::string writer_guid = deserialize_guid(item.writer_guid());
    entity = database_->get_entities_by_guid(entity_kind, writer_guid).front();
}

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& entity,
        EntityKind entity_kind,
        NetworkLatencySample& sample,
        const StatisticsLocator2LocatorData& item) const
{
    sample.data = item.data();
    std::string remote_locator = deserialize_locator(item.dst_locator());
    sample.remote_locator = database_->get_entities_by_name(entity_kind, remote_locator).front();

    std::string source_locator = deserialize_locator(item.src_locator());
    entity = database_->get_entities_by_name(entity_kind, source_locator).front();
}

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& entity,
        EntityKind entity_kind,
        EntityDataSample& sample,
        const StatisticsEntityData& item) const
{
    sample.data =  item.data();

    std::string guid = deserialize_guid(item.guid());
    entity = database_->get_entities_by_guid(entity_kind, guid).front();
}

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& entity,
        EntityKind entity_kind,
        EntityToLocatorCountSample& sample,
        const StatisticsEntity2LocatorTraffic& item) const
{
    sample.count = item.packet_count();
    std::string remote_locator = deserialize_locator(item.dst_locator());
    sample.remote_locator = database_->get_entities_by_name(entity_kind, remote_locator).front();

    std::string guid = deserialize_guid(item.src_guid());
    entity = database_->get_entities_by_guid(entity_kind, guid).front();
}

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& entity,
        EntityKind entity_kind,
        ByteToLocatorCountSample& sample,
        const StatisticsEntity2LocatorTraffic& item) const
{
    sample.count = item.byte_count();
    sample.magnitude_order = item.byte_magnitude_order();
    std::string remote_locator = deserialize_locator(item.dst_locator());
    sample.remote_locator = database_->get_entities_by_name(entity_kind, remote_locator).front();

    std::string guid = deserialize_guid(item.src_guid());
    entity = database_->get_entities_by_guid(entity_kind, guid).front();
}


template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& entity,
        EntityKind entity_kind,
        EntityCountSample& sample,
        const StatisticsEntityCount& item) const
{
    sample.count = item.count();

    std::string guid = deserialize_guid(item.guid());
    entity = database_->get_entities_by_guid(entity_kind, guid).front();
}

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& entity,
        EntityKind entity_kind,
        DiscoveryTimeSample& sample,
        const StatisticsDiscoveryTime& item) const
{
    sample.time = std::chrono::steady_clock::time_point (std::chrono::seconds(item.time()));
    std::string remote_entity_guid = deserialize_guid(item.remote_entity_guid());
    sample.remote_entity = database_->get_entities_by_guid(entity_kind, remote_entity_guid).front();

    std::string guid = deserialize_guid(item.local_participant_guid());
    entity = database_->get_entities_by_guid(entity_kind, guid).front();
}

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& entity,
        EntityKind entity_kind,
        SampleDatasCountSample& sample,
        const StatisticsSampleIdentityCount& item) const
{
    sample.count = item.count();

    auto sample_identity = deserialize_sample_identity(item.sample_id());
    sample.sequence_number = sample_identity.second;

    entity = database_->get_entities_by_guid(entity_kind, sample_identity.first).front();
}

void DatabaseDataQueue::process_sample()
{
    switch (front().second->_d())
    {
        case StatisticsEventKind::HISTORY2HISTORY_LATENCY:
            {
                HistoryLatencySample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::DATAWRITER, sample, item.second->writer_reader_data());
                database_->insert(entity, sample);
            }
            break;
        case StatisticsEventKind::NETWORK_LATENCY:
            {
                NetworkLatencySample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::LOCATOR, sample, item.second->locator2locator_data());
                database_->insert(entity, sample);
            }
            break;
        case StatisticsEventKind::PUBLICATION_THROUGHPUT:
            {
                PublicationThroughputSample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::DATAWRITER, static_cast<EntityDataSample&>(sample), item.second->entity_data());
                database_->insert(entity, sample);
            }
            break;
        case StatisticsEventKind::SUBSCRIPTION_THROUGHPUT:
            {
                SubscriptionThroughputSample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::DATAREADER, static_cast<EntityDataSample&>(sample), item.second->entity_data());
                database_->insert(entity, sample);
            }
            break;
        case StatisticsEventKind::RTPS_SENT:
            {
                RtpsPacketsSentSample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::DATAWRITER, static_cast<EntityToLocatorCountSample&>(sample), item.second->entity2locator_traffic());
                database_->insert(entity, sample);
            }
            {
                RtpsBytesSentSample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::DATAWRITER,  static_cast<ByteToLocatorCountSample&>(sample), item.second->entity2locator_traffic());
                database_->insert(entity, sample);
            }
            break;
        case StatisticsEventKind::RTPS_LOST:
            {
                RtpsPacketsLostSample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::DATAWRITER, static_cast<EntityToLocatorCountSample&>(sample), item.second->entity2locator_traffic());
                database_->insert(entity, sample);
            }
            {
                RtpsBytesLostSample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::DATAWRITER, static_cast<ByteToLocatorCountSample&>(sample), item.second->entity2locator_traffic());
                database_->insert(entity, sample);
            }
            break;
        case StatisticsEventKind::RESENT_DATAS:
            {
                ResentDataSample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::DATAWRITER, static_cast<EntityCountSample&>(sample), item.second->entity_count());
                database_->insert(entity, sample);
            }
            break;
        case StatisticsEventKind::HEARTBEAT_COUNT:
            {
                HeartbeatCountSample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::DATAWRITER, static_cast<EntityCountSample&>(sample), item.second->entity_count());
                database_->insert(entity, sample);
            }
            break;
        case StatisticsEventKind::ACKNACK_COUNT:
            {
                AcknackCountSample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::DATAREADER, static_cast<EntityCountSample&>(sample), item.second->entity_count());
                database_->insert(entity, sample);
            }
            break;
        case StatisticsEventKind::NACKFRAG_COUNT:
            {
                NackfragCountSample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::DATAREADER, static_cast<EntityCountSample&>(sample), item.second->entity_count());
                database_->insert(entity, sample);
            }
            break;
        case StatisticsEventKind::GAP_COUNT:
            {
                GapCountSample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::DATAWRITER, static_cast<EntityCountSample&>(sample), item.second->entity_count());
                database_->insert(entity, sample);
            }
            break;
        case StatisticsEventKind::DATA_COUNT:
            {
                DataCountSample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::DATAWRITER, static_cast<EntityCountSample&>(sample), item.second->entity_count());
                database_->insert(entity, sample);
            }
            break;
        case StatisticsEventKind::PDP_PACKETS:
            {
                PdpCountSample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::PARTICIPANT, static_cast<EntityCountSample&>(sample), item.second->entity_count());
                database_->insert(entity, sample);
            }
            break;
        case StatisticsEventKind::EDP_PACKETS:
            {
                EdpCountSample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::PARTICIPANT, static_cast<EntityCountSample&>(sample), item.second->entity_count());
                database_->insert(entity, sample);
            }
            break;
        case StatisticsEventKind::DISCOVERED_ENTITY:
            {
                DiscoveryTimeSample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::PARTICIPANT, sample, item.second->discovery_time());
                database_->insert(entity, sample);
            }
            break;
        case StatisticsEventKind::SAMPLE_DATAS:
            {
                SampleDatasCountSample sample;
                EntityId entity;
                queue_item_type item = front();
                sample.src_ts = item.first;
                process_sample_type(entity, EntityKind::DATAWRITER, sample, item.second->sample_identity_count());
                database_->insert(entity, sample);
            }
            break;
        case StatisticsEventKind::PHYSICAL_DATA:
            {
                StatisticsPhysicalData item = front().second->physical_data();

                // Take the ID of the Participant from its GUID
                std::string participant_guid = deserialize_guid(item.participant_guid());
                EntityId participant_id = database_->get_entities_by_guid(EntityKind::PARTICIPANT, participant_guid).front();

                // Parse the process name and PID
                size_t separator_pos = item.process().find(':');
                if (separator_pos == std::string::npos)
                {
                    std::stringstream msg;
                    msg << "Process name " << item.process() << "does not follow the [comman]:[PID] pattern";
                    throw Error(msg.str());
                }
                std::string process_name = item.process().substr(0, separator_pos);
                std::string process_pid = item.process().substr(separator_pos + 1);

                // Check the existence of the process
                EntityId process_id;
                std::shared_ptr<Process> process;
                std::vector<EntityId> processes = database_->get_entities_by_name(EntityKind::PROCESS, process_name);
                if (processes.empty())
                {
                    // Check the existence of the user
                    std::shared_ptr<User> user;
                    std::vector<EntityId> users = database_->get_entities_by_name(EntityKind::USER, item.user());
                    if (users.empty())
                    {
                        // Check the existence of the host
                        std::shared_ptr<Host> host;
                        std::vector<EntityId> hosts = database_->get_entities_by_name(EntityKind::HOST, item.host());
                        if (hosts.empty())
                        {
                            host.reset(new Host(item.host()));
                            database_->insert(std::static_pointer_cast<Entity>(host));
                        }
                        else
                        {
                            host = std::dynamic_pointer_cast<Host>(database_->get_entity(hosts.front()));
                        }

                        user.reset(new User(item.user(), host));
                        database_->insert(std::static_pointer_cast<Entity>(user));
                    }
                    else
                    {
                        user = std::dynamic_pointer_cast<User>(database_->get_entity(users.front()));
                    }

                    process.reset(new Process(process_name, process_pid, user));
                    process_id = database_->insert(std::static_pointer_cast<Entity>(process));
                }
                else
                {
                    process_id = processes.front();
                }

                database_->link_participant_with_process(participant_id, process_id);
            }
            break;
    }
}


} //namespace database
} //namespace statistics_backend
} //namespace eprosima