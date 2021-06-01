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

#include <algorithm>
#include <iostream>
#include <mutex>  // For std::unique_lock
#include <shared_mutex>
#include <string>
#include <vector>

#include <fastdds-statistics-backend/exception/Exception.hpp>
#include <fastdds-statistics-backend/types/types.hpp>
#include <fastdds-statistics-backend/types/JSONTags.h>

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
        case EntityKind::LOCATOR:
        {
            std::shared_ptr<Locator> locator = std::static_pointer_cast<Locator>(entity);

            /* Check that locator name is not empty */
            if (locator->name.empty())
            {
                throw BadParameter("Locator name cannot be empty");
            }

            /* Check that this is indeed a new locator, and that its name is unique */
            for (auto locator_it: locators_)
            {
                if (locator.get() == locator_it.second.get())
                {
                    throw BadParameter("Locator already exists in the database");
                }
                if (locator->name == locator_it.second->name)
                {
                    throw BadParameter("Locator with name " + locator->name + " already exists in the database");
                }
            }

            /* Insert locator in the database */
            locator->data_readers.clear();
            locator->data_writers.clear();
            locator->data.clear();
            locator->id = generate_entity_id();
            locators_[locator->id] = locator;
            return locator->id;
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
                // Store the increment since the last report
                participant->data.rtps_packets_sent[rtps_packets_sent.remote_locator].push_back(
                    rtps_packets_sent -
                    participant->data.last_reported_rtps_packets_sent_count[rtps_packets_sent.remote_locator]);
                // Update last report
                participant->data.last_reported_rtps_packets_sent_count[rtps_packets_sent.remote_locator] =
                        rtps_packets_sent;
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
                // Store the increment since the last report
                participant->data.rtps_bytes_sent[rtps_bytes_sent.remote_locator].push_back(
                    rtps_bytes_sent -
                    participant->data.last_reported_rtps_bytes_sent_count[rtps_bytes_sent.remote_locator]);
                // Update last report
                participant->data.last_reported_rtps_bytes_sent_count[rtps_bytes_sent.remote_locator] = rtps_bytes_sent;
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
                // Store the increment since the last report
                participant->data.rtps_packets_lost[rtps_packets_lost.remote_locator].push_back(
                    rtps_packets_lost -
                    participant->data.last_reported_rtps_packets_lost_count[rtps_packets_lost.remote_locator]);
                // Update last report
                participant->data.last_reported_rtps_packets_lost_count[rtps_packets_lost.remote_locator] =
                        rtps_packets_lost;
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
                // Store the increment since the last report
                participant->data.rtps_bytes_lost[rtps_bytes_lost.remote_locator].push_back(
                    rtps_bytes_lost -
                    participant->data.last_reported_rtps_bytes_lost_count[rtps_bytes_lost.remote_locator]);
                // Update last report
                participant->data.last_reported_rtps_bytes_lost_count[rtps_bytes_lost.remote_locator] = rtps_bytes_lost;
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
                // Store the increment since the last report
                writer->data.resent_datas.push_back(resent_datas - writer->data.last_reported_resent_datas);
                // Update last report
                writer->data.last_reported_resent_datas = resent_datas;
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
                // Store the increment since the last report
                writer->data.heartbeat_count.push_back(heartbeat_count - writer->data.last_reported_heartbeat_count);
                // Update last report
                writer->data.last_reported_heartbeat_count = heartbeat_count;
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
                // Store the increment since the last report
                reader->data.acknack_count.push_back(acknack_count - reader->data.last_reported_acknack_count);
                // Update last report
                reader->data.last_reported_acknack_count = acknack_count;
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
                // Store the increment since the last report
                reader->data.nackfrag_count.push_back(nackfrag_count - reader->data.last_reported_nackfrag_count);
                // Update last report
                reader->data.last_reported_nackfrag_count = nackfrag_count;
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
                // Store the increment since the last report
                writer->data.gap_count.push_back(gap_count - writer->data.last_reported_gap_count);
                // Update last report
                writer->data.last_reported_gap_count = gap_count;
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
                // Store the increment since the last report
                writer->data.data_count.push_back(data_count - writer->data.last_reported_data_count);
                // Update last report
                writer->data.last_reported_data_count = data_count;
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
                // Store the increment since the last report
                participant->data.pdp_packets.push_back(pdp_packets - participant->data.last_reported_pdp_packets);
                // Update last report
                participant->data.last_reported_pdp_packets = pdp_packets;
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
                // Store the increment since the last report
                participant->data.edp_packets.push_back(edp_packets - participant->data.last_reported_edp_packets);
                // Update last report
                participant->data.last_reported_edp_packets = edp_packets;
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
                participant->data.discovered_entity[discovery_time.remote_entity].push_back(discovery_time);
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
                writer->data.sample_datas[sample_datas.sequence_number].push_back(sample_datas);
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

void Database::link_endpoint_with_locator(
        const EntityId& endpoint_id,
        const EntityId& locator_id)
{
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);

    /* Get the endpoint */
    std::shared_ptr<DDSEndpoint> endpoint;
    {
        /* Get the Datawriter */
        std::map<EntityId, std::shared_ptr<DataWriter>>::iterator dw_it;
        bool dw_exists = false;
        for (auto domain_it : datawriters_)
        {
            dw_it = domain_it.second.find(endpoint_id);
            if (dw_it != domain_it.second.end())
            {
                dw_exists = true;
                endpoint = dw_it->second;
                break;
            }
        }
        if (!dw_exists)
        {
            /* Get the Datareader */
            std::map<EntityId, std::shared_ptr<DataReader>>::iterator dr_it;
            bool dr_exists = false;
            for (auto domain_it : datareaders_)
            {
                dr_it = domain_it.second.find(endpoint_id);
                if (dr_it != domain_it.second.end())
                {
                    dr_exists = true;
                    endpoint = dr_it->second;
                    break;
                }
            }

            if (!dr_exists)
            {
                throw BadParameter("EntityId " + std::to_string(
                                  endpoint_id.value()) + " does not identify a known endpoint");
            }
        }
    }

    /* Get the locator */
    auto locator_it = locators_.find(locator_id);
    if (locator_it == locators_.end())
    {
        throw BadParameter("EntityId " + std::to_string(locator_id.value()) + " does not identify a known locator");
    }

    // Not storing the std::shared_ptr<Locator> in a local variable results in the locator->second
    // being moved when inserting it into the map. This causes a SEGFAULT later on when accessing to it.
    auto locator = locator_it->second;

    // Add endpoint to locator's collection
    if (endpoint->kind == EntityKind::DATAWRITER)
    {
        locator->data_writers[endpoint->id] = std::dynamic_pointer_cast<DataWriter> (endpoint);
    }
    else // Datareader
    {
        locator->data_readers[endpoint->id] = std::dynamic_pointer_cast<DataReader> (endpoint);
    }

    // Add locator to endpoint's collection
    endpoint->locators[locator_id] = locator;

    // Add reader's locators to locators_by_participant_
    locators_by_participant_[endpoint->participant->id][locator_id] = locator;

    // Add reader's participant to participants_by_locator_
    participants_by_locator_[locator_id][endpoint->participant->id] = endpoint->participant;
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
    std::vector<std::pair<EntityId, EntityId>> entities;
    switch (entity_kind)
    {
        case EntityKind::HOST:
        {
            for (auto host_it : hosts_)
            {
                if (host_it.second->name == name)
                {
                    entities.push_back(std::pair<EntityId, EntityId>(EntityId(), host_it.first));
                }
            }
            break;
        }
        case EntityKind::USER:
        {
            for (auto user_it : users_)
            {
                if (user_it.second->name == name)
                {
                    entities.push_back(std::pair<EntityId, EntityId>(EntityId(), user_it.first));
                }
            }
            break;
        }
        case EntityKind::PROCESS:
        {
            for (auto process_it : processes_)
            {
                if (process_it.second->name == name)
                {
                    entities.push_back(std::pair<EntityId, EntityId>(EntityId(), process_it.first));
                }
            }
            break;
        }
        case EntityKind::DOMAIN:
        {
            for (auto domain_it : domains_)
            {
                if (domain_it.second->name == name)
                {
                    entities.push_back(std::pair<EntityId, EntityId>(domain_it.first, domain_it.first));
                }
            }
            break;
        }
        case EntityKind::PARTICIPANT:
        {
            for (auto domain_it : participants_)
            {
                for (auto participant_it : domain_it.second)
                {
                    if (participant_it.second->name == name)
                    {
                        entities.push_back(std::pair<EntityId, EntityId>(domain_it.first, participant_it.first));
                    }
                }
            }
            break;
        }
        case EntityKind::TOPIC:
        {
            for (auto domain_it : topics_)
            {
                for (auto topic_it : domain_it.second)
                {
                    if (topic_it.second->name == name)
                    {
                        entities.push_back(std::pair<EntityId, EntityId>(domain_it.first, topic_it.first));
                    }
                }
            }
            break;
        }
        case EntityKind::DATAREADER:
        {
            for (auto domain_it : datareaders_)
            {
                for (auto datareader_it : domain_it.second)
                {
                    if (datareader_it.second->name == name)
                    {
                        entities.push_back(std::pair<EntityId, EntityId>(domain_it.first, datareader_it.first));
                    }
                }
            }
            break;
        }
        case EntityKind::DATAWRITER:
        {
            for (auto domain_it : datawriters_)
            {
                for (auto datawriter_it : domain_it.second)
                {
                    if (datawriter_it.second->name == name)
                    {
                        entities.push_back(std::pair<EntityId, EntityId>(domain_it.first, datawriter_it.first));
                    }
                }
            }
            break;
        }
        case EntityKind::LOCATOR:
        {
            for (auto locator_it : locators_)
            {
                if (locator_it.second->name == name)
                {
                    entities.push_back(std::pair<EntityId, EntityId>(EntityId(), locator_it.first));
                }
            }
            break;
        }
        default:
        {
            throw BadParameter("Incorrect EntityKind");
        }
    }
    return entities;
}

std::vector<const StatisticsSample*> Database::select(
        DataKind data_type,
        EntityId entity_id_source,
        EntityId entity_id_target,
        Timestamp t_from,
        Timestamp t_to)
{
    /* Check that the given timestamps are consistent */
    if (t_to <= t_from)
    {
        throw BadParameter("Final timestamp must be strictly greater than the origin timestamp");
    }

    auto source_entity = get_entity(entity_id_source);
    auto target_entity = get_entity(entity_id_target);

    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    std::vector<const StatisticsSample*> samples;
    switch (data_type)
    {
        case DataKind::FASTDDS_LATENCY:
        {
            assert(EntityKind::DATAWRITER == source_entity->kind);
            assert(EntityKind::DATAREADER == target_entity->kind);
            auto writer = std::static_pointer_cast<const DataWriter>(source_entity);
            /* Look if the writer has information about the required reader */
            auto reader = writer->data.history2history_latency.find(entity_id_target);
            if (reader != writer->data.history2history_latency.end())
            {
                /* Look for the samples between the given timestamps */
                // TODO(jlbueno) Knowing that the samples are ordered by timestamp it would be more efficient to
                // implement a binary search (PR#58 Originally posted by @IkerLuengo in
                // https://github.com/eProsima/Fast-DDS-statistics-backend/pull/58#discussion_r629383536)
                for (auto& sample : reader->second)
                {
                    if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                    {
                        samples.push_back(&sample);
                    }
                    else if (sample.src_ts > t_to)
                    {
                        break;
                    }
                }
            }
            break;
        }
        case DataKind::NETWORK_LATENCY:
        {
            assert(EntityKind::LOCATOR == source_entity->kind);
            assert(EntityKind::LOCATOR == target_entity->kind);
            auto locator = std::static_pointer_cast<const Locator>(source_entity);
            /* Look if the locator has information about the required locator */
            auto remote_locator = locator->data.network_latency_per_locator.find(entity_id_target);
            if (remote_locator != locator->data.network_latency_per_locator.end())
            {
                /* Look for the samples between the given timestamps */
                for (auto& sample : remote_locator->second)
                {
                    if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                    {
                        samples.push_back(&sample);
                    }
                    else if (sample.src_ts > t_to)
                    {
                        break;
                    }
                }
            }
            break;
        }
        case DataKind::RTPS_PACKETS_SENT:
        {
            assert(EntityKind::PARTICIPANT == source_entity->kind);
            assert(EntityKind::LOCATOR == target_entity->kind);
            auto participant = std::static_pointer_cast<const DomainParticipant>(source_entity);
            /* Look if the participant has information about the required locator */
            auto locator = participant->data.rtps_packets_sent.find(entity_id_target);
            if (locator != participant->data.rtps_packets_sent.end())
            {
                /* Look for the samples between the given timestamps */
                for (auto& sample : locator->second)
                {
                    if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                    {
                        samples.push_back(&sample);
                    }
                    else if (sample.src_ts > t_to)
                    {
                        break;
                    }
                }
            }
            break;
        }
        case DataKind::RTPS_BYTES_SENT:
        {
            assert(EntityKind::PARTICIPANT == source_entity->kind);
            assert(EntityKind::LOCATOR == target_entity->kind);
            auto participant = std::static_pointer_cast<const DomainParticipant>(source_entity);
            /* Look if the participant has information about the required locator */
            auto locator = participant->data.rtps_bytes_sent.find(entity_id_target);
            if (locator != participant->data.rtps_bytes_sent.end())
            {
                /* Look for the samples between the given timestamps */
                for (auto& sample : locator->second)
                {
                    if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                    {
                        samples.push_back(&sample);
                    }
                    else if (sample.src_ts > t_to)
                    {
                        break;
                    }
                }
            }
            break;
        }
        case DataKind::RTPS_PACKETS_LOST:
        {
            assert(EntityKind::PARTICIPANT == source_entity->kind);
            assert(EntityKind::LOCATOR == target_entity->kind);
            auto participant = std::static_pointer_cast<const DomainParticipant>(source_entity);
            /* Look if the participant has information about the required locator */
            auto locator = participant->data.rtps_packets_lost.find(entity_id_target);
            if (locator != participant->data.rtps_packets_lost.end())
            {
                /* Look for the samples between the given timestamps */
                for (auto& sample : locator->second)
                {
                    if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                    {
                        samples.push_back(&sample);
                    }
                    else if (sample.src_ts > t_to)
                    {
                        break;
                    }
                }
            }
            break;
        }
        case DataKind::RTPS_BYTES_LOST:
        {
            assert(EntityKind::PARTICIPANT == source_entity->kind);
            assert(EntityKind::LOCATOR == target_entity->kind);
            auto participant = std::static_pointer_cast<const DomainParticipant>(source_entity);
            /* Look if the participant has information about the required locator */
            auto locator = participant->data.rtps_bytes_lost.find(entity_id_target);
            if (locator != participant->data.rtps_bytes_lost.end())
            {
                /* Look for the samples between the given timestamps */
                for (auto& sample : locator->second)
                {
                    if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                    {
                        samples.push_back(&sample);
                    }
                    else if (sample.src_ts > t_to)
                    {
                        break;
                    }
                }
            }
            break;
        }
        case DataKind::DISCOVERY_TIME:
        {
            assert(EntityKind::PARTICIPANT == source_entity->kind);
            assert(EntityKind::PARTICIPANT == target_entity->kind || EntityKind::DATAREADER == target_entity->kind ||
                    EntityKind::DATAWRITER == target_entity->kind);
            auto participant = std::static_pointer_cast<const DomainParticipant>(source_entity);
            /* Look if the participant has information about the required dds entity */
            auto dds_entity = participant->data.discovered_entity.find(entity_id_target);
            if (dds_entity != participant->data.discovered_entity.end())
            {
                /* Look for the samples between the given timestamps */
                for (auto& sample : dds_entity->second)
                {
                    if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                    {
                        samples.push_back(&sample);
                    }
                    else if (sample.src_ts > t_to)
                    {
                        break;
                    }
                }
            }
            break;
        }
        default:
        {
            throw BadParameter("Incorrect DataKind");
        }
    }
    return samples;
}

std::vector<const StatisticsSample*> Database::select(
        DataKind data_type,
        EntityId entity_id,
        Timestamp t_from,
        Timestamp t_to)
{
    /* Check that the given timestamps are consistent */
    if (t_to <= t_from)
    {
        throw BadParameter("Final timestamp must be strictly greater than the origin timestamp");
    }

    auto entity = get_entity(entity_id);

    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    std::vector<const StatisticsSample*> samples;
    switch (data_type)
    {
        case DataKind::PUBLICATION_THROUGHPUT:
        {
            assert(EntityKind::DATAWRITER == entity->kind);
            auto writer = std::static_pointer_cast<const DataWriter>(entity);
            /* Look for the samples between the given timestamps */
            // TODO(jlbueno) Knowing that the samples are ordered by timestamp it would be more efficient to
            // implement a binary search (PR#58 Originally posted by @IkerLuengo in
            // https://github.com/eProsima/Fast-DDS-statistics-backend/pull/58#discussion_r629383536)
            for (auto& sample : writer->data.publication_throughput)
            {
                /* The data is assumed to be ordered by timestamp */
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::SUBSCRIPTION_THROUGHPUT:
        {
            assert(EntityKind::DATAREADER == entity->kind);
            auto reader = std::static_pointer_cast<const DataReader>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : reader->data.subscription_throughput)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::RESENT_DATA:
        {
            assert(EntityKind::DATAWRITER == entity->kind);
            auto writer = std::static_pointer_cast<const DataWriter>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : writer->data.resent_datas)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::HEARTBEAT_COUNT:
        {
            assert(EntityKind::DATAWRITER == entity->kind);
            auto writer = std::static_pointer_cast<const DataWriter>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : writer->data.heartbeat_count)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::ACKNACK_COUNT:
        {
            assert(EntityKind::DATAREADER == entity->kind);
            auto reader = std::static_pointer_cast<const DataReader>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : reader->data.acknack_count)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::NACKFRAG_COUNT:
        {
            assert(EntityKind::DATAREADER == entity->kind);
            auto reader = std::static_pointer_cast<const DataReader>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : reader->data.nackfrag_count)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::GAP_COUNT:
        {
            assert(EntityKind::DATAWRITER == entity->kind);
            auto writer = std::static_pointer_cast<const DataWriter>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : writer->data.gap_count)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::DATA_COUNT:
        {
            assert(EntityKind::DATAWRITER == entity->kind);
            auto writer = std::static_pointer_cast<const DataWriter>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : writer->data.data_count)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::PDP_PACKETS:
        {
            assert(EntityKind::PARTICIPANT == entity->kind);
            auto participant = std::static_pointer_cast<const DomainParticipant>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : participant->data.pdp_packets)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        case DataKind::EDP_PACKETS:
        {
            assert(EntityKind::PARTICIPANT == entity->kind);
            auto participant = std::static_pointer_cast<const DomainParticipant>(entity);
            /* Look for the samples between the given timestamps */
            for (auto& sample : participant->data.edp_packets)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
            break;
        }
        // Any other data_type corresponds to a sample which needs two entities or a DataKind::INVALID
        default:
        {
            throw BadParameter("Incorrect DataKind");
        }
    }
    return samples;
}

std::vector<const StatisticsSample*> Database::select(
        DataKind data_type,
        EntityId entity_id,
        uint64_t sequence_number,
        Timestamp t_from,
        Timestamp t_to)
{
    /* Check that the given timestamps are consistent */
    if (t_to <= t_from)
    {
        throw BadParameter("Final timestamp must be strictly greater than the origin timestamp");
    }

    auto entity = get_entity(entity_id);

    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    std::vector<const StatisticsSample*> samples;
    if (DataKind::SAMPLE_DATAS == data_type)
    {
        assert(EntityKind::DATAWRITER == entity->kind);
        auto writer = std::static_pointer_cast<const DataWriter>(entity);
        /* Look if the writer has information about the required sequence number */
        auto seq_number = writer->data.sample_datas.find(sequence_number);
        if (seq_number != writer->data.sample_datas.end())
        {
            /* Look for the samples between the given timestamps */
            // TODO(jlbueno) Knowing that the samples are ordered by timestamp it would be more efficient to
            // implement a binary search (PR#58 Originally posted by @IkerLuengo in
            // https://github.com/eProsima/Fast-DDS-statistics-backend/pull/58#discussion_r629383536)
            for (auto& sample : seq_number->second)
            {
                if (sample.src_ts >= t_from && sample.src_ts <= t_to)
                {
                    samples.push_back(&sample);
                }
                else if (sample.src_ts > t_to)
                {
                    break;
                }
            }
        }
    }
    else
    {
        throw BadParameter("Incorrect DataKind");
    }
    return samples;
}

std::pair<EntityId, EntityId> Database::get_entity_by_guid(
        EntityKind entity_kind,
        const std::string& guid) const
{
    switch (entity_kind)
    {
        case EntityKind::PARTICIPANT:
        {
            for (auto domain_it : participants_)
            {
                for (auto participant_it : domain_it.second)
                {
                    if (participant_it.second->guid == guid)
                    {
                        return std::pair<EntityId, EntityId>(domain_it.first, participant_it.first);
                    }
                }
            }
            break;
        }
        case EntityKind::DATAREADER:
        {
            for (auto domain_it : datareaders_)
            {
                for (auto datareader_it : domain_it.second)
                {
                    if (datareader_it.second->guid == guid)
                    {
                        return std::pair<EntityId, EntityId>(domain_it.first, datareader_it.first);
                    }
                }
            }
            break;
        }
        case EntityKind::DATAWRITER:
        {
            for (auto domain_it : datawriters_)
            {
                for (auto datawriter_it : domain_it.second)
                {
                    if (datawriter_it.second->guid == guid)
                    {
                        return std::pair<EntityId, EntityId>(domain_it.first, datawriter_it.first);
                    }
                }
            }
            break;
        }
        default:
        {
            throw BadParameter("Incorrect EntityKind");
        }
    }

    throw BadParameter("No entity of type " + std::to_string(
                      static_cast<int>(entity_kind)) + " and GUID " + guid + " exists");
    return std::make_pair<EntityId, EntityId>(EntityId::invalid(), EntityId::invalid());
}

EntityKind Database::get_entity_kind(
        EntityId entity_id) const
{
    return get_entity(entity_id).get()->kind;
}

const std::vector<std::shared_ptr<const Entity>> Database::get_entities(
        EntityKind entity_kind,
        const EntityId& entity_id) const
{
    // This call will throw BadParameter if there is no such entity.
    // We let this exception through, as it meets expectations
    std::shared_ptr<const Entity> origin = get_entity(entity_id);
    assert (origin->kind != EntityKind::INVALID);

    auto entities = get_entities(entity_kind, origin);

    // Remove duplicates by sorting and unique-ing
    std::sort(entities.begin(), entities.end(), [](
                const std::shared_ptr<const Entity>& first,
                const std::shared_ptr<const Entity>& second)
            {
                return first.get() < second.get();
            });
    auto last = std::unique(entities.begin(), entities.end(), [](
                        const std::shared_ptr<const Entity>& first,
                        const std::shared_ptr<const Entity>& second)
                    {
                        return first.get() == second.get();
                    });
    entities.erase(last, entities.end());

    return entities;
}

std::vector<EntityId> Database::get_entity_ids(
        EntityKind entity_kind,
        const EntityId& entity_id) const
{
    std::vector<EntityId> entitiesIds;
    for (auto entity : get_entities(entity_kind, entity_id))
    {
        entitiesIds.push_back(entity->id);
    }

    return entitiesIds;
}

const std::vector<std::shared_ptr<const Entity>> Database::get_entities(
        EntityKind entity_kind,
        const std::shared_ptr<const Entity>& origin) const
{
    std::vector<std::shared_ptr<const Entity>> entities;

    switch (origin->kind)
    {
        case EntityKind::HOST:
        {
            const std::shared_ptr<const Host>& host = std::dynamic_pointer_cast<const Host>(origin);
            switch (entity_kind)
            {
                case EntityKind::HOST:
                    entities.push_back(host);
                    break;
                case EntityKind::USER:
                    for (auto user : host->users)
                    {
                        entities.push_back(user.second);
                    }
                    break;
                case EntityKind::PROCESS:
                case EntityKind::DOMAIN:
                case EntityKind::PARTICIPANT:
                case EntityKind::TOPIC:
                case EntityKind::DATAREADER:
                case EntityKind::DATAWRITER:
                case EntityKind::LOCATOR:
                    for (auto user : host->users)
                    {
                        auto sub_entities = get_entities(entity_kind, user.second);
                        entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                    }
                    break;
                default:
                    throw BadParameter("Invalid EntityKind");
            }
            break;
        }
        case EntityKind::USER:
        {
            const std::shared_ptr<const User>& user = std::dynamic_pointer_cast<const User>(origin);
            switch (entity_kind)
            {
                case EntityKind::HOST:
                    entities.push_back(user->host);
                    break;
                case EntityKind::USER:
                    entities.push_back(user);
                    break;
                case EntityKind::PROCESS:
                    for (auto process : user->processes)
                    {
                        entities.push_back(process.second);
                    }
                    break;
                case EntityKind::DOMAIN:
                case EntityKind::PARTICIPANT:
                case EntityKind::TOPIC:
                case EntityKind::DATAREADER:
                case EntityKind::DATAWRITER:
                case EntityKind::LOCATOR:
                    for (auto process : user->processes)
                    {
                        auto sub_entities = get_entities(entity_kind, process.second);
                        entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                    }
                    break;
                default:
                    throw BadParameter("Invalid EntityKind");
            }
            break;
        }
        case EntityKind::PROCESS:
        {
            const std::shared_ptr<const Process>& process = std::dynamic_pointer_cast<const Process>(origin);
            switch (entity_kind)
            {
                case EntityKind::HOST:
                {
                    auto sub_entities = get_entities(entity_kind, process->user);
                    entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                }
                break;
                case EntityKind::USER:
                    entities.push_back(process->user);
                    break;
                case EntityKind::PROCESS:
                    entities.push_back(process);
                    break;
                case EntityKind::PARTICIPANT:
                    for (auto participant : process->participants)
                    {
                        entities.push_back(participant.second);
                    }
                    break;
                case EntityKind::DOMAIN:
                case EntityKind::TOPIC:
                case EntityKind::DATAREADER:
                case EntityKind::DATAWRITER:
                case EntityKind::LOCATOR:
                    for (auto participant : process->participants)
                    {
                        auto sub_entities = get_entities(entity_kind, participant.second);
                        entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                    }
                    break;
                default:
                    throw BadParameter("Invalid EntityKind");
            }
            break;
        }
        case EntityKind::DOMAIN:
        {
            const std::shared_ptr<const Domain>& domain = std::dynamic_pointer_cast<const Domain>(origin);
            switch (entity_kind)
            {
                case EntityKind::DOMAIN:
                    entities.push_back(domain);
                    break;
                case EntityKind::PARTICIPANT:
                    for (auto participant : domain->participants)
                    {
                        entities.push_back(participant.second);
                    }
                    break;
                case EntityKind::TOPIC:
                    for (auto topic : domain->topics)
                    {
                        entities.push_back(topic.second);
                    }
                    break;
                case EntityKind::HOST:
                case EntityKind::USER:
                case EntityKind::PROCESS:
                case EntityKind::DATAREADER:
                case EntityKind::DATAWRITER:
                case EntityKind::LOCATOR:
                    for (auto participant : domain->participants)
                    {
                        auto sub_entities = get_entities(entity_kind, participant.second);
                        entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                    }
                    break;
                default:
                    throw BadParameter("Invalid EntityKind");
            }
            break;
        }
        case EntityKind::PARTICIPANT:
        {
            const std::shared_ptr<const DomainParticipant>& participant =
                    std::dynamic_pointer_cast<const DomainParticipant>(origin);
            switch (entity_kind)
            {
                case EntityKind::HOST:
                case EntityKind::USER:
                {
                    auto sub_entities = get_entities(entity_kind, participant->process);
                    entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                }
                break;
                case EntityKind::PROCESS:
                    entities.push_back(participant->process);
                    break;
                case EntityKind::DOMAIN:
                    entities.push_back(participant->domain);
                    break;
                case EntityKind::PARTICIPANT:
                    entities.push_back(participant);
                    break;
                case EntityKind::DATAWRITER:
                    for (auto writer : participant->data_writers)
                    {
                        entities.push_back(writer.second);
                    }
                    break;
                case EntityKind::DATAREADER:
                    for (auto reader : participant->data_readers)
                    {
                        entities.push_back(reader.second);
                    }
                    break;
                case EntityKind::TOPIC:
                case EntityKind::LOCATOR:
                    for (auto writer : participant->data_writers)
                    {
                        auto sub_entities = get_entities(entity_kind, writer.second);
                        entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                    }
                    for (auto reader : participant->data_readers)
                    {
                        auto sub_entities = get_entities(entity_kind, reader.second);
                        entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                    }
                    break;
                default:
                    throw BadParameter("Invalid EntityKind");
            }
            break;
        }
        case EntityKind::TOPIC:
        {
            const std::shared_ptr<const Topic>& topic = std::dynamic_pointer_cast<const Topic>(origin);
            switch (entity_kind)
            {
                case EntityKind::DOMAIN:
                    entities.push_back(topic->domain);
                    break;
                case EntityKind::TOPIC:
                    entities.push_back(topic);
                    break;
                case EntityKind::DATAWRITER:
                    for (auto writer : topic->data_writers)
                    {
                        entities.push_back(writer.second);
                    }
                    break;
                case EntityKind::DATAREADER:
                    for (auto reader : topic->data_readers)
                    {
                        entities.push_back(reader.second);
                    }
                    break;
                case EntityKind::HOST:
                case EntityKind::USER:
                case EntityKind::PROCESS:
                case EntityKind::PARTICIPANT:
                case EntityKind::LOCATOR:
                    for (auto writer : topic->data_writers)
                    {
                        auto sub_entities = get_entities(entity_kind, writer.second);
                        entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                    }
                    for (auto reader : topic->data_readers)
                    {
                        auto sub_entities = get_entities(entity_kind, reader.second);
                        entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                    }
                    break;
                default:
                    throw BadParameter("Invalid EntityKind");
            }
            break;
        }
        case EntityKind::DATAWRITER:
        {
            const std::shared_ptr<const DataWriter>& writer = std::dynamic_pointer_cast<const DataWriter>(origin);
            switch (entity_kind)
            {
                case EntityKind::TOPIC:
                    entities.push_back(writer->topic);
                    break;
                case EntityKind::PARTICIPANT:
                    entities.push_back(writer->participant);
                    break;
                case EntityKind::DATAWRITER:
                    entities.push_back(writer);
                    break;
                case EntityKind::LOCATOR:
                    for (auto locator : writer->locators)
                    {
                        entities.push_back(locator.second);
                    }
                    break;
                case EntityKind::DATAREADER:
                {
                    auto sub_entities = get_entities(entity_kind, writer->topic);
                    entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                }
                break;
                case EntityKind::HOST:
                case EntityKind::USER:
                case EntityKind::PROCESS:
                case EntityKind::DOMAIN:
                {
                    auto sub_entities = get_entities(entity_kind, writer->participant);
                    entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                }
                break;
                default:
                    throw BadParameter("Invalid EntityKind");
            }
            break;
        }
        case EntityKind::DATAREADER:
        {
            const std::shared_ptr<const DataReader>& reader = std::dynamic_pointer_cast<const DataReader>(origin);
            switch (entity_kind)
            {
                case EntityKind::TOPIC:
                    entities.push_back(reader->topic);
                    break;
                case EntityKind::PARTICIPANT:
                    entities.push_back(reader->participant);
                    break;
                case EntityKind::DATAREADER:
                    entities.push_back(reader);
                    break;
                case EntityKind::LOCATOR:
                    for (auto locator : reader->locators)
                    {
                        entities.push_back(locator.second);
                    }
                    break;
                case EntityKind::DATAWRITER:
                {
                    auto sub_entities = get_entities(entity_kind, reader->topic);
                    entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                }
                break;
                case EntityKind::HOST:
                case EntityKind::USER:
                case EntityKind::PROCESS:
                case EntityKind::DOMAIN:
                {
                    auto sub_entities = get_entities(entity_kind, reader->participant);
                    entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                }
                break;
                default:
                    throw BadParameter("Invalid EntityKind");
            }
            break;
        }
        case EntityKind::LOCATOR:
        {
            const std::shared_ptr<const Locator>& locator = std::dynamic_pointer_cast<const Locator>(origin);
            switch (entity_kind)
            {
                case EntityKind::DATAREADER:
                    for (auto reader : locator->data_readers)
                    {
                        entities.push_back(reader.second);
                    }
                    break;
                case EntityKind::DATAWRITER:
                    for (auto writer : locator->data_writers)
                    {
                        entities.push_back(writer.second);
                    }
                    break;
                case EntityKind::LOCATOR:
                    entities.push_back(locator);
                    break;
                case EntityKind::HOST:
                case EntityKind::USER:
                case EntityKind::PROCESS:
                case EntityKind::PARTICIPANT:
                case EntityKind::TOPIC:
                case EntityKind::DOMAIN:
                    for (auto writer : locator->data_writers)
                    {
                        auto sub_entities = get_entities(entity_kind, writer.second);
                        entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                    }
                    for (auto reader : locator->data_readers)
                    {
                        auto sub_entities = get_entities(entity_kind, reader.second);
                        entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                    }
                    break;
                default:
                    throw BadParameter("Invalid EntityKind");
            }
            break;
        }
        default:
            throw BadParameter("Invalid EntityKind");
    }

    return entities;
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

DatabaseDump Database::dump_database()
{
    DatabaseDump dump = DatabaseDump::object();

    // Add version
    dump[VERSION_TAG] = ACTUAL_DUMP_VERSION;

    // Hosts
    {
        DatabaseDump container = DatabaseDump::object();

        // For each entity of this kind in database
        for (auto it : hosts_)
        {
            container[id_to_string(it.first.value())] = dump_entity_(it.second);
        }

        dump[HOST_CONTAINER_TAG] = container;
    }

    // Users
    {
        DatabaseDump container = DatabaseDump::object();

        // For each entity of this kind in database
        for (auto it : users_)
        {
            container[id_to_string(it.first.value())] = dump_entity_(it.second);
        }

        dump[USER_CONTAINER_TAG] = container;
    }

    // Processes
    {
        DatabaseDump container = DatabaseDump::object();

        // For each entity of this kind in database
        for (auto it : processes_)
        {
            container[id_to_string(it.first.value())] = dump_entity_(it.second);
        }

        dump[PROCESS_CONTAINER_TAG] = container;
    }

    // Domain
    {
        DatabaseDump container = DatabaseDump::object();

        // For each entity of this kind in database
        for (auto it : domains_)
        {
            container[id_to_string(it.first.value())] = dump_entity_(it.second);
        }

        dump[DOMAIN_CONTAINER_TAG] = container;
    }

    // Topic
    {
        DatabaseDump container = DatabaseDump::object();
        // For each domain
        for (auto super_it : topics_)
        {
            // For each entity of this kind in domain
            for (auto it : super_it.second)
            {
                container[id_to_string(it.first.value())] = dump_entity_(it.second);
            }
        }
        dump[TOPIC_CONTAINER_TAG] = container;
    }

    // Participant
    {
        DatabaseDump container = DatabaseDump::object();
        // For each domain
        for (auto super_it : participants_)
        {
            // For each entity of this kind in domain
            for (auto it : super_it.second)
            {
                container[id_to_string(it.first.value())] = dump_entity_(it.second);
            }
        }
        dump[PARTICIPANT_CONTAINER_TAG] = container;
    }

    // DataWriter
    {
        DatabaseDump container = DatabaseDump::object();
        // For each domain
        for (auto super_it : datawriters_)
        {
            // For each entity of this kind in domain
            for (auto it : super_it.second)
            {
                container[id_to_string(it.first.value())] = dump_entity_(it.second);
            }
        }
        dump[DATAWRITER_CONTAINER_TAG] = container;
    }

    // DataReader
    {
        DatabaseDump container = DatabaseDump::object();
        // For each domain
        for (auto super_it : datareaders_)
        {
            // For each entity of this kind in domain
            for (auto it : super_it.second)
            {
                container[id_to_string(it.first.value())] = dump_entity_(it.second);
            }
        }
        dump[DATAREADER_CONTAINER_TAG] = container;
    }

    // Locator
    {
        DatabaseDump container = DatabaseDump::object();

        // For each entity of this kind in database
        for (auto it : locators_)
        {
            container[id_to_string(it.first.value())] = dump_entity_(it.second);
        }

        dump[LOCATOR_CONTAINER_TAG] = container;
    }

    return dump;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<Host>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_INFO_TAG] = entity->name;

    // Populate subentity array
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (auto sub_it : entity->users)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[USER_CONTAINER_TAG] = subentities;
    }

    return entity_info;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<User>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_INFO_TAG] = entity->name;

    entity_info[HOST_ENTITY_TAG] = id_to_string(entity->host->id);

    // Populate subentity array
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (auto sub_it : entity->processes)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[PROCESS_CONTAINER_TAG] = subentities;
    }

    return entity_info;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<Process>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_INFO_TAG] = entity->name;
    entity_info[PID_INFO_TAG] = entity->pid;

    entity_info[USER_ENTITY_TAG] = id_to_string(entity->user->id);

    // Populate subentity array
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (auto sub_it : entity->participants)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[PARTICIPANT_CONTAINER_TAG] = subentities;
    }

    return entity_info;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<Domain>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_INFO_TAG] = entity->name;

    // Populate subentity array for Topics
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (auto sub_it : entity->topics)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[TOPIC_CONTAINER_TAG] = subentities;
    }

    // Populate subentity array for Participants
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (auto sub_it : entity->participants)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[PARTICIPANT_CONTAINER_TAG] = subentities;
    }

    return entity_info;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<Topic>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_INFO_TAG] = entity->name;
    entity_info[DATA_TYPE_INFO_TAG] = entity->data_type;

    entity_info[DOMAIN_ENTITY_TAG] = id_to_string(entity->domain->id);

    // Populate subentity array for DataWriters
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (auto sub_it : entity->data_writers)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[DATAWRITER_CONTAINER_TAG] = subentities;
    }

    // Populate subentity array for DataReaders
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (auto sub_it : entity->data_readers)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[DATAREADER_CONTAINER_TAG] = subentities;
    }

    return entity_info;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<DomainParticipant>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_INFO_TAG] = entity->name;
    entity_info[GUID_INFO_TAG] = entity->guid;
    entity_info[QOS_INFO_TAG] = entity->qos;

    entity_info[DOMAIN_ENTITY_TAG] = id_to_string(entity->domain->id);
    entity_info[PROCESS_ENTITY_TAG] = id_to_string(entity->process->id);

    // Populate subentity array for DataWriters
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (auto sub_it : entity->data_writers)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[DATAWRITER_CONTAINER_TAG] = subentities;
    }
    // Populate subentity array for DataReaders
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (auto sub_it : entity->data_readers)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[DATAREADER_CONTAINER_TAG] = subentities;
    }

    // Store data from the entity
    {
        DatabaseDump data = DatabaseDump::object();

        // discovered_entity
        data[DATA_KIND_DISCOVERY_TIME_TAG] = dump_data_(entity->data.discovered_entity);

        // pdp_packets
        data[DATA_KIND_PDP_PACKETS_TAG] = dump_data_(entity->data.pdp_packets);

        // edp_packets
        data[DATA_KIND_EDP_PACKETS_TAG] = dump_data_(entity->data.edp_packets);

        // rtps_packets_sent
        data[DATA_KIND_RTPS_PACKETS_SENT_TAG] = dump_data_(entity->data.rtps_packets_sent);

        // rtps_bytes_sent
        data[DATA_KIND_RTPS_BYTES_SENT_TAG] = dump_data_(entity->data.rtps_bytes_sent);

        // rtps_packets_lost
        data[DATA_KIND_RTPS_PACKETS_LOST_TAG] = dump_data_(entity->data.rtps_packets_lost);

        // rtps_bytes_lost
        data[DATA_KIND_RTPS_BYTES_LOST_TAG] = dump_data_(entity->data.rtps_bytes_lost);

        // pdp_packets last reported
        data[DATA_KIND_PDP_PACKETS_LAST_REPORTED_TAG] = dump_data_(entity->data.last_reported_pdp_packets);

        // edp_packets last reported
        data[DATA_KIND_EDP_PACKETS_LAST_REPORTED_TAG] = dump_data_(entity->data.last_reported_edp_packets);

        // rtps_packets_sent last reported
        data[DATA_KIND_RTPS_PACKETS_SENT_LAST_REPORTED_TAG] =
                dump_data_(entity->data.last_reported_rtps_packets_sent_count);

        // rtps_bytes_sent last reported
        data[DATA_KIND_RTPS_BYTES_SENT_LAST_REPORTED_TAG] =
                dump_data_(entity->data.last_reported_rtps_bytes_sent_count);

        // rtps_packets_lost last reported
        data[DATA_KIND_RTPS_PACKETS_LOST_LAST_REPORTED_TAG] =
                dump_data_(entity->data.last_reported_rtps_packets_lost_count);

        // rtps_bytes_lost last reported
        data[DATA_KIND_RTPS_BYTES_LOST_LAST_REPORTED_TAG] =
                dump_data_(entity->data.last_reported_rtps_bytes_lost_count);

        entity_info[DATA_CONTAINER_TAG] = data;
    }

    return entity_info;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<DataWriter>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_INFO_TAG] = entity->name;
    entity_info[GUID_INFO_TAG] = entity->guid;
    entity_info[QOS_INFO_TAG] = entity->qos;

    entity_info[PARTICIPANT_ENTITY_TAG] = id_to_string(entity->participant->id);
    entity_info[TOPIC_ENTITY_TAG] = id_to_string(entity->topic->id);

    // Populate subentity array for Locators
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (auto sub_it : entity->locators)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[LOCATOR_CONTAINER_TAG] = subentities;
    }

    // Store data from the entity
    {
        DatabaseDump data = DatabaseDump::object();

        // publication_throughput
        data[DATA_KIND_PUBLICATION_THROUGHPUT_TAG] = dump_data_(entity->data.publication_throughput);

        // resent_datas
        data[DATA_KIND_RESENT_DATA_TAG] = dump_data_(entity->data.resent_datas);

        // heartbeat_count
        data[DATA_KIND_HEARTBEAT_COUNT_TAG] = dump_data_(entity->data.heartbeat_count);

        // gap_count
        data[DATA_KIND_GAP_COUNT_TAG] = dump_data_(entity->data.gap_count);

        // data_count
        data[DATA_KIND_DATA_COUNT_TAG] = dump_data_(entity->data.data_count);

        // sample_datas
        data[DATA_KIND_SAMPLE_DATAS_TAG] = dump_data_(entity->data.sample_datas);

        // history2history_latency
        data[DATA_KIND_FASTDDS_LATENCY_TAG] = dump_data_(entity->data.history2history_latency);

        // heartbeat_count last reported
        data[DATA_KIND_HEARTBEAT_COUNT_LAST_REPORTED_TAG] = dump_data_(entity->data.last_reported_heartbeat_count);

        // gap_count last reported
        data[DATA_KIND_GAP_COUNT_LAST_REPORTED_TAG] = dump_data_(entity->data.last_reported_gap_count);

        // data_count last reported
        data[DATA_KIND_DATA_COUNT_LAST_REPORTED_TAG] = dump_data_(entity->data.last_reported_data_count);

        entity_info[DATA_CONTAINER_TAG] = data;
    }

    return entity_info;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<DataReader>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_INFO_TAG] = entity->name;
    entity_info[GUID_INFO_TAG] = entity->guid;
    entity_info[QOS_INFO_TAG] = entity->qos;

    entity_info[PARTICIPANT_ENTITY_TAG] = id_to_string(entity->participant->id);
    entity_info[TOPIC_ENTITY_TAG] = id_to_string(entity->topic->id);

    // Populate subentity array for Locators
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (auto sub_it : entity->locators)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[LOCATOR_CONTAINER_TAG] = subentities;
    }

    // Store data from the entity
    {
        DatabaseDump data = DatabaseDump::object();

        // subscription_throughput
        data[DATA_KIND_SUBSCRIPTION_THROUGHPUT_TAG] = dump_data_(entity->data.subscription_throughput);

        // acknack_count
        data[DATA_KIND_ACKNACK_COUNT_TAG] = dump_data_(entity->data.acknack_count);

        // nackfrag_count
        data[DATA_KIND_NACKFRAG_COUNT_TAG] = dump_data_(entity->data.nackfrag_count);

        // acknack_count last reported
        data[DATA_KIND_ACKNACK_COUNT_LAST_REPORTED_TAG] = dump_data_(entity->data.last_reported_acknack_count);

        // nackfrag_count last reported
        data[DATA_KIND_NACKFRAG_COUNT_LAST_REPORTED_TAG] = dump_data_(entity->data.last_reported_nackfrag_count);

        entity_info[DATA_CONTAINER_TAG] = data;
    }

    return entity_info;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<Locator>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_INFO_TAG] = entity->name;

    // Populate subentity array for DataWriters
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (auto sub_it : entity->data_writers)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[DATAWRITER_CONTAINER_TAG] = subentities;
    }

    // Populate subentity array for DataReaders
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (auto sub_it : entity->data_readers)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[DATAREADER_CONTAINER_TAG] = subentities;
    }

    // Store data from the entity
    {
        DatabaseDump data = DatabaseDump::object();

        // network_latency_per_locator
        data[DATA_KIND_NETWORK_LATENCY_TAG] = dump_data_(entity->data.network_latency_per_locator);

        entity_info[DATA_CONTAINER_TAG] = data;
    }

    return entity_info;
}

DatabaseDump Database::dump_data_(
        const std::map<EntityId, std::vector<ByteCountSample>>& data)
{
    DatabaseDump data_dump = DatabaseDump::object();

    for (auto it : data)
    {
        DatabaseDump samples = DatabaseDump::array();

        for (auto sample : it.second)
        {
            DatabaseDump value = DatabaseDump::object();
            value[DATA_VALUE_SRC_TIME_TAG] = time_to_string(sample.src_ts);
            value[DATA_VALUE_COUNT_TAG] = sample.count;
            value[DATA_VALUE_MAGNITUDE_TAG] = sample.magnitude_order;

            samples.push_back(value);
        }

        data_dump[id_to_string(it.first.value())] = samples;
    }

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const std::map<EntityId, std::vector<EntityCountSample>>& data)
{
    DatabaseDump data_dump = DatabaseDump::object();

    for (auto it : data)
    {
        DatabaseDump samples = DatabaseDump::array();

        for (auto sample : it.second)
        {
            DatabaseDump value = DatabaseDump::object();
            value[DATA_VALUE_SRC_TIME_TAG] = time_to_string(sample.src_ts);
            value[DATA_VALUE_COUNT_TAG] = sample.count;

            samples.push_back(value);
        }

        data_dump[id_to_string(it.first.value())] = samples;
    }

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const std::map<EntityId, std::vector<EntityDataSample>>& data)
{
    DatabaseDump data_dump = DatabaseDump::object();

    for (auto it : data)
    {
        DatabaseDump samples = DatabaseDump::array();

        for (auto sample : it.second)
        {
            DatabaseDump value = DatabaseDump::object();
            value[DATA_VALUE_SRC_TIME_TAG] = time_to_string(sample.src_ts);
            value[DATA_VALUE_DATA_TAG] = sample.data;

            samples.push_back(value);
        }

        data_dump[id_to_string(it.first.value())] = samples;
    }

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const std::map<EntityId, std::vector<DiscoveryTimeSample>>& data)
{
    DatabaseDump data_dump = DatabaseDump::object();

    for (auto it : data)
    {
        DatabaseDump samples = DatabaseDump::array();

        for (auto sample : it.second)
        {
            DatabaseDump value = DatabaseDump::object();
            value[DATA_VALUE_SRC_TIME_TAG] = time_to_string(sample.src_ts);
            value[DATA_VALUE_TIME_TAG] = time_to_string(sample.time);
            value[DATA_VALUE_REMOTE_ENTITY_TAG] = id_to_string(sample.remote_entity.value());
            value[DATA_VALUE_DISCOVERED_TAG] = sample.discovered;

            samples.push_back(value);
        }

        data_dump[id_to_string(it.first.value())] = samples;
    }

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const std::map<uint64_t, std::vector<EntityCountSample>>& data)
{
    DatabaseDump data_dump = DatabaseDump::object();

    for (auto it : data)
    {
        DatabaseDump samples = DatabaseDump::array();

        for (auto sample : it.second)
        {
            DatabaseDump value = DatabaseDump::object();
            value[DATA_VALUE_SRC_TIME_TAG] = time_to_string(sample.src_ts);
            value[DATA_VALUE_COUNT_TAG] = sample.count;

            samples.push_back(value);
        }

        data_dump[id_to_string(it.first)] = samples;
    }

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const std::vector<EntityCountSample>& data)
{
    DatabaseDump data_dump = DatabaseDump::array();

    for (auto it : data)
    {
        DatabaseDump value = DatabaseDump::object();
        value[DATA_VALUE_SRC_TIME_TAG] = time_to_string(it.src_ts);
        value[DATA_VALUE_COUNT_TAG] = it.count;

        data_dump.push_back(value);
    }

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const std::vector<EntityDataSample>& data)
{
    DatabaseDump data_dump = DatabaseDump::array();

    for (auto it : data)
    {
        DatabaseDump value = DatabaseDump::object();
        value[DATA_VALUE_SRC_TIME_TAG] = time_to_string(it.src_ts);
        value[DATA_VALUE_DATA_TAG] = it.data;

        data_dump.push_back(value);
    }

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const EntityCountSample& data)
{
    DatabaseDump data_dump = DatabaseDump::object();
    data_dump[DATA_VALUE_SRC_TIME_TAG] = time_to_string(data.src_ts);
    data_dump[DATA_VALUE_COUNT_TAG] = data.count;

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const ByteCountSample& data)
{
    DatabaseDump data_dump = DatabaseDump::object();
    data_dump[DATA_VALUE_SRC_TIME_TAG] = time_to_string(data.src_ts);
    data_dump[DATA_VALUE_COUNT_TAG] = data.count;
    data_dump[DATA_VALUE_MAGNITUDE_TAG] = data.magnitude_order;

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const std::map<EntityId, EntityCountSample>& data)
{
    DatabaseDump data_dump = DatabaseDump::object();

    for (auto it : data)
    {
        data_dump[id_to_string(it.first.value())] = dump_data_(it.second);
    }

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const std::map<EntityId, ByteCountSample>& data)
{
    DatabaseDump data_dump = DatabaseDump::object();

    for (auto it : data)
    {
        data_dump[id_to_string(it.first.value())] = dump_data_(it.second);
    }

    return data_dump;
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
