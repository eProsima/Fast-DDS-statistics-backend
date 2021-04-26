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

#include "database.hpp"

#include <iostream>
#include <mutex>  // For std::unique_lock
#include <shared_mutex>

#include <fastdds-statistics-backend/exception/Exception.hpp>
#include <fastdds-statistics-backend/types/types.hpp>

#include "samples.hpp"

namespace eprosima {
namespace statistics_backend {
namespace database {

EntityId Database::insert(
        const std::shared_ptr<Entity>& entity)
{
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);
    switch (entity->kind)
    {
        case EntityKind::HOST:
        {
            std::shared_ptr<Host> host = std::static_pointer_cast<Host>(entity);

            /* Check that host name is not empty */
            if (host->name.empty())
            {
                throw BadParameter("Host name cannot be empty");
            }

            /* Check that this is indeed a new host, and that its name is unique */
            for (auto host_it: hosts_)
            {
                if (host.get() == host_it.second.get())
                {
                    throw BadParameter("Host already exists in the database");
                }
                if (host->name == host_it.second->name)
                {
                    throw BadParameter("Host with name " + host->name + " already exists in the database");
                }
            }

            /* Insert host in the database */
            host->users.clear();
            host->id = generate_entity_id();
            hosts_[host->id] = host;
            return host->id;
        }
        case EntityKind::USER:
        {
            std::shared_ptr<User> user = std::static_pointer_cast<User>(entity);

            /* Check that user name is not empty */
            if (user->name.empty())
            {
                throw BadParameter("User name cannot be empty");
            }

            /* Check that this is indeed a new user */
            for (auto user_it: users_)
            {
                if (user.get() == user_it.second.get())
                {
                    throw BadParameter("User already exists in the database");
                }
            }

            /* Check that host exits */
            bool host_exists = false;
            for (auto host_it : hosts_)
            {
                if (user->host.get() == host_it.second.get())
                {
                    host_exists = true;
                    break;
                }
            }

            if (!host_exists)
            {
                throw BadParameter("Parent host does not exist in the database");
            }

            /* Check that a user with the same name does not exist in the host collection */
            for (auto user_it : user->host->users)
            {
                if (user->name == user_it.second->name)
                {
                    throw BadParameter(
                              "Another User with name '" + user->name
                              + "' already exists in parent host collection");
                }
            }

            /* Add user to users collection */
            user->processes.clear();
            user->id = generate_entity_id();
            users_[user->id] = user;

            /* Add user to host's users collection */
            user->host->users[user->id] = user;
            return user->id;
        }
        case EntityKind::PROCESS:
        {
            std::shared_ptr<Process> process = std::static_pointer_cast<Process>(entity);

            /* Check that process name is not empty */
            if (process->name.empty())
            {
                throw BadParameter("Process name cannot be empty");
            }

            /* Check that PID is not empty */
            if (process->pid.empty())
            {
                throw BadParameter("Process PID cannot be empty");
            }

            /* Check that this is indeed a new process */
            for (auto process_it: processes_)
            {
                if (process.get() == process_it.second.get())
                {
                    throw BadParameter("Process already exists in the database");
                }
            }

            /* Check that user exits */
            bool user_exists = false;
            for (auto user_it : users_)
            {
                if (process->user.get() == user_it.second.get())
                {
                    user_exists = true;
                    break;
                }
            }

            if (!user_exists)
            {
                throw BadParameter("Parent user does not exist in the database");
            }

            /* Check that a process with the same pid does not exist in the same host */
            for (auto user_it : hosts_[process->user->host->id]->users)
            {
                for (auto process_it : user_it.second->processes)
                {
                    if (process->pid == process_it.second->pid)
                    {
                        throw BadParameter(
                                  "Another process with PID '" + process->pid
                                  + "' already exists in the same host");
                    }
                }
            }

            /* Add process to processes collection */
            process->participants.clear();
            process->id = generate_entity_id();
            processes_[process->id] = process;

            /* Add process to user's processes collection */
            process->user->processes[process->id] = process;
            return process->id;
            break;
        }
        case EntityKind::DOMAIN:
        {
            std::shared_ptr<Domain> domain = std::static_pointer_cast<Domain>(entity);

            /* Check that domain name is not empty */
            if (domain->name.empty())
            {
                throw BadParameter("Domain name cannot be empty");
            }

            /* Check that this is indeed a new domain and that its name is unique */
            for (auto domain_it: domains_)
            {
                if (domain.get() == domain_it.second.get())
                {
                    throw BadParameter("Domain already exists in the database");
                }
                if (domain->name == domain_it.second->name)
                {
                    throw BadParameter(
                              "A Domain with name '" + domain->name + "' already exists in the database");
                }
            }

            /* Insert domain in the database */
            domain->topics.clear();
            domain->participants.clear();
            domain->id = generate_entity_id();
            domains_[domain->id] = domain;
            return domain->id;
        }
        case EntityKind::TOPIC:
        {
            std::shared_ptr<Topic> topic = std::static_pointer_cast<Topic>(entity);

            /* Check that topic name is not empty */
            if (topic->name.empty())
            {
                throw BadParameter("Topic name cannot be empty");
            }

            /* Check that topic data type is not empty */
            if (topic->data_type.empty())
            {
                throw BadParameter("Topic data type cannot be empty");
            }

            /* Check that domain exits */
            bool domain_exists = false;
            for (auto domain_it : domains_)
            {
                if (topic->domain.get() == domain_it.second.get())
                {
                    domain_exists = true;
                    break;
                }
            }

            if (!domain_exists)
            {
                throw BadParameter("Parent domain does not exist in the database");
            }

            /* Check that this is indeed a new topic and that its name is unique in the domain */
            for (auto topic_it: topics_[topic->domain->id])
            {
                if (topic.get() == topic_it.second.get())
                {
                    throw BadParameter("Topic already exists in the database");
                }
                if (topic->name == topic_it.second->name)
                {
                    throw BadParameter(
                              "A topic with name '" + topic->name +
                              "' already exists in the database for the same domain");
                }
            }

            /* Add topic to domain's collection */
            topic->data_readers.clear();
            topic->data_writers.clear();
            topic->id = generate_entity_id();
            domains_[topic->domain->id]->topics[topic->id] = topic;

            /* Insert topic in the database */
            topics_[topic->domain->id][topic->id] = topic;
            return topic->id;
        }
        case EntityKind::PARTICIPANT:
        {
            std::shared_ptr<DomainParticipant> participant = std::static_pointer_cast<DomainParticipant>(entity);

            /* Check that name is not empty */
            if (participant->name.empty())
            {
                throw BadParameter("Participant name cannot be empty");
            }

            /* Check that qos is not empty */
            if (participant->qos.empty())
            {
                throw BadParameter("Participant QoS cannot be empty");
            }

            /* Check that GUID is not empty */
            if (participant->guid.empty())
            {
                throw BadParameter("Participant GUID cannot be empty");
            }

            /* Check that domain exits */
            bool domain_exists = false;
            for (auto domain_it : domains_)
            {
                if (participant->domain.get() == domain_it.second.get())
                {
                    domain_exists = true;
                    break;
                }
            }

            if (!domain_exists)
            {
                throw BadParameter("Parent domain does not exist in the database");
            }

            /* Check that this is indeed a new participant and that its GUID is unique */
            for (auto domain_it : participants_)
            {
                for (auto participant_it : domain_it.second)
                {
                    // Check that participant is new
                    if (participant.get() == participant_it.second.get())
                    {
                        throw BadParameter("Participant already exists in the database");
                    }
                    // Check GUID uniqueness
                    if (participant->guid == participant_it.second->guid)
                    {
                        throw BadParameter(
                                  "A participant with GUID '" + participant->guid +
                                  "' already exists in the database for the same domain");
                    }
                }
            }

            /* Add participant to process' collection */
            participant->data_readers.clear();
            participant->data_writers.clear();
            participant->data.clear();
            participant->id = generate_entity_id();

            /* Add participant to domain's collection */
            participant->domain->participants[participant->id] = participant;

            /* Insert participant in the database */
            participants_[participant->domain->id][participant->id] = participant;
            return participant->id;
        }
        case EntityKind::DATAREADER:
        {
            auto data_reader = std::static_pointer_cast<DataReader>(entity);
            return insert_ddsendpoint<DataReader>(data_reader);
        }
        case EntityKind::DATAWRITER:
        {
            auto data_writer = std::static_pointer_cast<DataWriter>(entity);
            return insert_ddsendpoint<DataWriter>(data_writer);
        }
        default:
        {
            break;
        }
    }
    return EntityId();
}

void Database::insert(
        const EntityId& domain_id,
        const EntityId& entity_id,
        const StatisticsSample& sample)
{
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);

    /* Check that domain_id refers to a known domain */
    if (sample.kind != DataKind::NETWORK_LATENCY && !domains_[domain_id])
    {
        throw BadParameter(std::to_string(domain_id.value()) + " does not refer to a known domain");
    }

    switch (sample.kind)
    {
        case DataKind::FASTDDS_LATENCY:
        {
            /* Check that the entity is a known writer */
            auto writer = datawriters_[domain_id][entity_id];
            if (writer)
            {
                const HistoryLatencySample& fastdds_latency = dynamic_cast<const HistoryLatencySample&>(sample);
                writer->data.history2history_latency[fastdds_latency.reader].push_back(fastdds_latency);
                break;
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datawriter in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::NETWORK_LATENCY:
        {
            /* Check that the entity is a known locator */
            auto locator = locators_[entity_id];
            if (locator)
            {
                const NetworkLatencySample& network_latency = dynamic_cast<const NetworkLatencySample&>(sample);
                locator->data.network_latency_per_locator[network_latency.remote_locator].push_back(network_latency);
                break;
            }
            throw BadParameter(std::to_string(entity_id.value()) + " does not refer to a known locator");
        }
        case DataKind::PUBLICATION_THROUGHPUT:
        {
            /* Check that the entity is a known writer */
            auto writer = datawriters_[domain_id][entity_id];
            if (writer)
            {
                const PublicationThroughputSample& publication_throughput =
                        dynamic_cast<const PublicationThroughputSample&>(sample);
                writer->data.publication_throughput.push_back(publication_throughput);
                break;
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datawriter in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::SUBSCRIPTION_THROUGHPUT:
        {
            /* Check that the entity is a known reader */
            auto reader = datareaders_[domain_id][entity_id];
            if (reader)
            {
                const SubscriptionThroughputSample& subscription_throughput =
                        dynamic_cast<const SubscriptionThroughputSample&>(sample);
                reader->data.subscription_throughput.push_back(subscription_throughput);
                break;
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datareader in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::RTPS_PACKETS_SENT:
        {
            /* Check that the entity is a known participant */
            auto participant = participants_[domain_id][entity_id];
            if (participant)
            {
                const RtpsPacketsSentSample& rtps_packets_sent = dynamic_cast<const RtpsPacketsSentSample&>(sample);
                participant->data.rtps_packets_sent[rtps_packets_sent.remote_locator].push_back(rtps_packets_sent);
                break;
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known participant in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::RTPS_BYTES_SENT:
        {
            /* Check that the entity is a known participant */
            auto participant = participants_[domain_id][entity_id];
            if (participant)
            {
                const RtpsBytesSentSample& rtps_bytes_sent = dynamic_cast<const RtpsBytesSentSample&>(sample);
                participant->data.rtps_bytes_sent[rtps_bytes_sent.remote_locator].push_back(rtps_bytes_sent);
                break;
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known participant in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::RTPS_PACKETS_LOST:
        {
            /* Check that the entity is a known participant */
            auto participant = participants_[domain_id][entity_id];
            if (participant)
            {
                const RtpsPacketsLostSample& rtps_packets_lost = dynamic_cast<const RtpsPacketsLostSample&>(sample);
                participant->data.rtps_packets_lost[rtps_packets_lost.remote_locator].push_back(rtps_packets_lost);
                break;
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known participant in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::RTPS_BYTES_LOST:
        {
            /* Check that the entity is a known participant */
            auto participant = participants_[domain_id][entity_id];
            if (participant)
            {
                const RtpsBytesLostSample& rtps_bytes_lost = dynamic_cast<const RtpsBytesLostSample&>(sample);
                participant->data.rtps_bytes_lost[rtps_bytes_lost.remote_locator].push_back(rtps_bytes_lost);
                break;
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known participant in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::RESENT_DATA:
        {
            /* Check that the entity is a known writer */
            auto writer = datawriters_[domain_id][entity_id];
            if (writer)
            {
                const ResentDataSample& resent_datas = dynamic_cast<const ResentDataSample&>(sample);
                writer->data.resent_datas.push_back(resent_datas);
                break;
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datawriter in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::HEARTBEAT_COUNT:
        {
            /* Check that the entity is a known writer */
            auto writer = datawriters_[domain_id][entity_id];
            if (writer)
            {
                const HeartbeatCountSample& heartbeat_count = dynamic_cast<const HeartbeatCountSample&>(sample);
                writer->data.heartbeat_count.push_back(heartbeat_count);
                break;
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datawriter in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::ACKNACK_COUNT:
        {
            /* Check that the entity is a known reader */
            auto reader = datareaders_[domain_id][entity_id];
            if (reader)
            {
                const AcknackCountSample& acknack_count = dynamic_cast<const AcknackCountSample&>(sample);
                reader->data.acknack_count.push_back(acknack_count);
                break;
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datareader in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::NACKFRAG_COUNT:
        {
            /* Check that the entity is a known reader */
            auto reader = datareaders_[domain_id][entity_id];
            if (reader)
            {
                const NackfragCountSample& nackfrag_count = dynamic_cast<const NackfragCountSample&>(sample);
                reader->data.nackfrag_count.push_back(nackfrag_count);
                break;
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datareader in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::GAP_COUNT:
        {
            /* Check that the entity is a known writer */
            auto writer = datawriters_[domain_id][entity_id];
            if (writer)
            {
                const GapCountSample& gap_count = dynamic_cast<const GapCountSample&>(sample);
                writer->data.gap_count.push_back(gap_count);
                break;
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datawriter in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::DATA_COUNT:
        {
            /* Check that the entity is a known writer */
            auto writer = datawriters_[domain_id][entity_id];
            if (writer)
            {
                const DataCountSample& data_count = dynamic_cast<const DataCountSample&>(sample);
                writer->data.data_count.push_back(data_count);
                break;
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datawriter in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::PDP_PACKETS:
        {
            /* Check that the entity is a known participant */
            auto participant = participants_[domain_id][entity_id];
            if (participant)
            {
                const PdpCountSample& pdp_packets = dynamic_cast<const PdpCountSample&>(sample);
                participant->data.pdp_packets.push_back(pdp_packets);
                break;
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known participant in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::EDP_PACKETS:
        {
            /* Check that the entity is a known participant */
            auto participant = participants_[domain_id][entity_id];
            if (participant)
            {
                const EdpCountSample& edp_packets = dynamic_cast<const EdpCountSample&>(sample);
                participant->data.edp_packets.push_back(edp_packets);
                break;
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known participant in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::DISCOVERY_TIME:
        {
            /* Check that the entity is a known participant */
            auto participant = participants_[domain_id][entity_id];
            if (participant)
            {
                const DiscoveryTimeSample& discovery_time = dynamic_cast<const DiscoveryTimeSample&>(sample);
                participant->data.discovered_entity[discovery_time.remote_entity].push_back(std::pair<std::chrono::steady_clock::time_point,
                        bool>(discovery_time.time, discovery_time.discovered));
                break;
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known participant in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::SAMPLE_DATAS:
        {
            /* Check that the entity is a known writer */
            auto writer = datawriters_[domain_id][entity_id];
            if (writer)
            {
                const SampleDatasCountSample& sample_datas = dynamic_cast<const SampleDatasCountSample&>(sample);
                writer->data.sample_datas[sample_datas.sequence_number] = sample_datas.count;
                break;
            }
            throw BadParameter(std::to_string(
                              entity_id.value()) + " does not refer to a known datawriter in domain " + std::to_string(
                              domain_id.value()));
        }
        case DataKind::INVALID:
        {
            throw BadParameter("Invalid DataKind");
        }
    }
    static_cast<void>(entity_id);
    static_cast<void>(sample);
}

void Database::link_participant_with_process(
        const EntityId& participant_id,
        const EntityId& process_id)
{
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);
    /* Get the participant and the domain */
    EntityId domain_id;
    std::map<EntityId, std::shared_ptr<DomainParticipant>>::iterator participant_it;
    bool participant_exists = false;
    for (auto domain_it : participants_)
    {
        participant_it = domain_it.second.find(participant_id);
        if (participant_it != domain_it.second.end())
        {
            participant_exists = true;
            domain_id = domain_it.first;
            break;
        }
    }

    /* Check that the participant exists */
    if (!participant_exists)
    {
        throw BadParameter("EntityId " + std::to_string(
                          participant_id.value()) + " does not identify a known participant");
    }
    /* Verify that participant does not have a link already */
    else if (participant_it->second->process)
    {
        throw BadParameter("Participant with ID " + std::to_string(
                          participant_id.value()) + " in already linked with a process");
    }

    /* Get the process */
    auto process_it = processes_.find(process_id);
    if (process_it == processes_.end())
    {
        throw BadParameter("EntityId " + std::to_string(process_id.value()) + " does not identify a known process");
    }

    /* Add reference to process to the participant */
    participant_it->second->process = process_it->second;

    /* Add the participant to the process' list of participants */
    // Not storing the std::shared_ptr<DomainParticipant> in a local variable results in the participant_it->second
    // being moved when inserting it into the map. This causes a SEGFAULT later on when accessing to it.
    auto participant = participant_it->second;
    process_it->second->participants[participant_it->first] = participant;

    /* Add entry to domains_by_process_ */
    domains_by_process_[process_it->first][domain_id] = participant_it->second->domain;

    /* Add entry to processes_by_domain_ */
    processes_by_domain_[domain_id][process_it->first] = process_it->second;
}

const std::shared_ptr<const Entity> Database::get_entity(
        const EntityId& entity_id) const
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    /* Iterate over all the collections looking for the entity */
    for (auto host_it : hosts_)
    {
        if (host_it.second->id == entity_id)
        {
            return host_it.second;
        }
    }
    for (auto process_it : processes_)
    {
        if (process_it.second->id == entity_id)
        {
            return process_it.second;
        }
    }
    for (auto user_it : users_)
    {
        if (user_it.second->id == entity_id)
        {
            return user_it.second;
        }
    }
    for (auto domain_it : domains_)
    {
        if (domain_it.second->id == entity_id)
        {
            return domain_it.second;
        }
    }
    for (auto domain_it : topics_)
    {
        for (auto topic_it : domain_it.second)
        {
            if (topic_it.second->id == entity_id)
            {
                return topic_it.second;
            }
        }
    }
    for (auto domain_it : participants_)
    {
        for (auto participant_it : domain_it.second)
        {
            if (participant_it.second->id == entity_id)
            {
                return participant_it.second;
            }
        }
    }
    for (auto domain_it : datareaders_)
    {
        for (auto datareader_it : domain_it.second)
        {
            if (datareader_it.second->id == entity_id)
            {
                return datareader_it.second;
            }
        }
    }
    for (auto domain_it : datawriters_)
    {
        for (auto datawriter_it : domain_it.second)
        {
            if (datawriter_it.second->id == entity_id)
            {
                return datawriter_it.second;
            }
        }
    }
    for (auto locator_it : locators_)
    {
        if (locator_it.second->id == entity_id)
        {
            return locator_it.second;
        }
    }
    /* The entity has not been found */
    throw BadParameter("Database does not contain an entity with ID " + entity_id.value());
}

std::vector<std::pair<EntityId, EntityId>> Database::get_entities_by_name(
        EntityKind entity_kind,
        const std::string& name) const
{
    (void)entity_kind;
    (void)name;
    throw Unsupported("Not implemented yet");
}

std::vector<std::pair<EntityId, EntityId>> Database::get_entities_by_guid(
        EntityKind entity_kind,
        const std::string& guid) const
{
    (void)entity_kind;
    (void)guid;
    throw Unsupported("Not implemented yet");
}

EntityId Database::generate_entity_id() noexcept
{
    return EntityId(next_id_++);
}

template<>
std::map<EntityId, std::map<EntityId, std::shared_ptr<DataReader>>>& Database::dds_endpoints<DataReader>()
{
    return datareaders_;
}

template<>
std::map<EntityId, std::map<EntityId, std::shared_ptr<DataWriter>>>& Database::dds_endpoints<DataWriter>()
{
    return datawriters_;
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
