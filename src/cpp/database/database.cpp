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
#include <chrono>
#include <iostream>
#include <mutex>  // For std::unique_lock
#include <shared_mutex>
#include <string>
#include <vector>

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/types/types.hpp>
#include <fastdds_statistics_backend/types/JSONTags.h>
#include <StatisticsBackendData.hpp>

#include "database_queue.hpp"
#include "samples.hpp"

namespace eprosima {
namespace statistics_backend {
namespace database {

EntityId Database::insert(
        const std::shared_ptr<Entity>& entity)
{
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);

    // Insert in the database with a unique ID
    EntityId entity_id = EntityId::invalid();
    insert_nts(entity, entity_id);

    return entity_id;
}

void Database::insert_nts(
        const std::shared_ptr<Entity>& entity,
        EntityId& entity_id)
{
    // Clear the entity
    entity->clear();

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
            for (const auto& host_it: hosts_)
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

            // Add id to the entity
            if (!entity_id.is_valid_and_unique())
            {
                entity_id = generate_entity_id();
            }
            else if (entity_id.value() >= next_id_)
            {
                next_id_ = entity_id.value() + 1;
            }
            host->id = entity_id;

            /* Insert host in the database */
            hosts_[host->id] = host;
            break;
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
            for (const auto& user_it: users_)
            {
                if (user.get() == user_it.second.get())
                {
                    throw BadParameter("User already exists in the database");
                }
            }

            /* Check that host exits */
            bool host_exists = false;
            for (const auto& host_it : hosts_)
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
            for (const auto& user_it : user->host->users)
            {
                if (user->name == user_it.second->name)
                {
                    throw BadParameter(
                              "Another User with name '" + user->name
                              + "' already exists in parent host collection");
                }
            }

            // Add id to the entity
            if (!entity_id.is_valid_and_unique())
            {
                entity_id = generate_entity_id();
            }
            else if (entity_id.value() >= next_id_)
            {
                next_id_ = entity_id.value() + 1;
            }
            user->id = entity_id;

            /* Add user to users collection */
            users_[user->id] = user;

            /* Add user to host's users collection */
            user->host->users[user->id] = user;
            break;
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
            for (const auto& process_it: processes_)
            {
                if (process.get() == process_it.second.get())
                {
                    throw BadParameter("Process already exists in the database");
                }
            }

            /* Check that user exits */
            bool user_exists = false;
            for (const auto& user_it : users_)
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
            for (const auto& user_it : hosts_[process->user->host->id]->users)
            {
                for (const auto& process_it : user_it.second->processes)
                {
                    if (process->pid == process_it.second->pid)
                    {
                        throw BadParameter(
                                  "Another process with PID '" + process->pid
                                  + "' already exists in the same host");
                    }
                }
            }

            // Add id to the entity
            if (!entity_id.is_valid_and_unique())
            {
                entity_id = generate_entity_id();
            }
            else if (entity_id.value() >= next_id_)
            {
                next_id_ = entity_id.value() + 1;
            }
            process->id = entity_id;

            /* Add process to processes collection */
            processes_[process->id] = process;

            /* Add process to user's processes collection */
            process->user->processes[process->id] = process;
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
            for (const auto& domain_it: domains_)
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

            // Add id to the entity
            if (!entity_id.is_valid_and_unique())
            {
                entity_id = generate_entity_id();
            }
            else if (entity_id.value() >= next_id_)
            {
                next_id_ = entity_id.value() + 1;
            }
            domain->id = entity_id;

            /* Insert domain in the database */
            domains_[domain->id] = domain;
            break;
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
            for (const auto& domain_it : domains_)
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

            /* Check that this is indeed a new topic and that its name and type combination is unique in the domain */
            for (const auto& topic_it: topics_[topic->domain->id])
            {
                if (topic.get() == topic_it.second.get())
                {
                    throw BadParameter("Topic already exists in the database");
                }
                if (topic->name == topic_it.second->name &&
                        topic->data_type == topic_it.second->data_type)
                {
                    throw BadParameter(
                              "A topic with name '" + topic->name +
                              "' and type '" + topic->data_type +
                              "' already exists in the database for the same domain");
                }
            }

            // Add id to the entity
            if (!entity_id.is_valid_and_unique())
            {
                entity_id = generate_entity_id();
            }
            else if (entity_id.value() >= next_id_)
            {
                next_id_ = entity_id.value() + 1;
            }
            topic->id = entity_id;

            /* Add topic to domain's collection */
            domains_[topic->domain->id]->topics[topic->id] = topic;

            /* Insert topic in the database */
            topics_[topic->domain->id][topic->id] = topic;
            break;
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
            for (const auto& domain_it : domains_)
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
            for (const auto& domain_it : participants_)
            {
                for (const auto& participant_it : domain_it.second)
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

            // Add id to the entity
            if (!entity_id.is_valid_and_unique())
            {
                entity_id = generate_entity_id();
            }
            else if (entity_id.value() >= next_id_)
            {
                next_id_ = entity_id.value() + 1;
            }
            participant->id = entity_id;

            /* Add participant to domain's collection */
            participant->domain->participants[participant->id] = participant;

            /* Insert participant in the database */
            participants_[participant->domain->id][participant->id] = participant;
            break;
        }
        case EntityKind::DATAREADER:
        {
            auto data_reader = std::static_pointer_cast<DataReader>(entity);
            insert_ddsendpoint<DataReader>(data_reader, entity_id);
            break;
        }
        case EntityKind::DATAWRITER:
        {
            auto data_writer = std::static_pointer_cast<DataWriter>(entity);
            insert_ddsendpoint<DataWriter>(data_writer, entity_id);
            break;
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
            for (const auto& locator_it: locators_)
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

            // Add id to the entity
            if (!entity_id.is_valid_and_unique())
            {
                entity_id = generate_entity_id();
            }
            else if (entity_id.value() >= next_id_)
            {
                next_id_ = entity_id.value() + 1;
            }
            locator->id = entity_id;

            /* Insert locator in the database */
            locators_[locator->id] = locator;
            break;
        }
        default:
        {
            break;
        }
    }
}

void Database::insert(
        const EntityId& domain_id,
        const EntityId& entity_id,
        const StatisticsSample& sample)
{
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);

    insert_nts(domain_id, entity_id, sample);
}

std::shared_ptr<Locator> Database::get_locator_nts(
        EntityId const& entity_id)
{
    std::shared_ptr<Locator> locator;

    // Check if the entity is a known locator
    auto locator_it = locators_.find(entity_id);

    // If the locator has not been discovered, create it
    if (locator_it == locators_.end())
    {
        // Give him a unique name, including his id
        locator = std::make_shared<Locator>("locator_" + std::to_string(entity_id.value()));
        EntityId locator_id = entity_id;
        insert_nts(locator, locator_id);
        notify_locator_discovery(locator_id);
    }
    else
    {
        locator = locator_it->second;
    }
    return locator;
}

void Database::notify_locator_discovery (
        const EntityId& locator_id)
{
    details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
        locator_id, EntityKind::LOCATOR,
        details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
}

void Database::insert_nts(
        const EntityId& domain_id,
        const EntityId& entity_id,
        const StatisticsSample& sample,
        const bool loading,
        const bool last_reported)
{

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
            const NetworkLatencySample& network_latency = dynamic_cast<const NetworkLatencySample&>(sample);

            // Create locator if it does not exist
            std::shared_ptr<Locator> locator = get_locator_nts(entity_id);

            // Create remote_locator if it does not exist
            get_locator_nts(network_latency.remote_locator);

            // Add the info to the locator
            locator->data.network_latency_per_locator[network_latency.remote_locator].push_back(network_latency);

            break;
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

                // Create remote_locator if it does not exist
                get_locator_nts(rtps_packets_sent.remote_locator);

                // Check if the insertion is from the load
                if (loading)
                {
                    if (last_reported)
                    {
                        // Store last reported
                        participant->data.last_reported_rtps_packets_sent_count[rtps_packets_sent.remote_locator] =
                                rtps_packets_sent;
                    }
                    else
                    {
                        // Store data directly
                        participant->data.rtps_packets_sent[rtps_packets_sent.remote_locator].push_back(
                            rtps_packets_sent);
                    }
                }
                else
                {
                    // Store the increment since the last report
                    participant->data.rtps_packets_sent[rtps_packets_sent.remote_locator].push_back(
                        rtps_packets_sent -
                        participant->data.last_reported_rtps_packets_sent_count[rtps_packets_sent.remote_locator]);
                    // Update last report
                    participant->data.last_reported_rtps_packets_sent_count[rtps_packets_sent.remote_locator] =
                            rtps_packets_sent;
                }

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

                // Create remote_locator if it does not exist
                get_locator_nts(rtps_bytes_sent.remote_locator);

                // Check if the insertion is from the load
                if (loading)
                {
                    if (last_reported)
                    {
                        // Store last reported
                        participant->data.last_reported_rtps_bytes_sent_count[rtps_bytes_sent.remote_locator] =
                                rtps_bytes_sent;
                    }
                    else
                    {
                        // Store data directly
                        participant->data.rtps_bytes_sent[rtps_bytes_sent.remote_locator].push_back(
                            rtps_bytes_sent);
                    }
                }
                else
                {
                    // Store the increment since the last report
                    participant->data.rtps_bytes_sent[rtps_bytes_sent.remote_locator].push_back(
                        rtps_bytes_sent -
                        participant->data.last_reported_rtps_bytes_sent_count[rtps_bytes_sent.remote_locator]);
                    // Update last report
                    participant->data.last_reported_rtps_bytes_sent_count[rtps_bytes_sent.remote_locator] =
                            rtps_bytes_sent;
                }

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

                // Create remote_locator if it does not exist
                get_locator_nts(rtps_packets_lost.remote_locator);

                // Check if the insertion is from the load
                if (loading)
                {
                    if (last_reported)
                    {
                        // Store last reported
                        participant->data.last_reported_rtps_packets_lost_count[rtps_packets_lost.remote_locator] =
                                rtps_packets_lost;
                    }
                    else
                    {
                        // Store data directly
                        participant->data.rtps_packets_lost[rtps_packets_lost.remote_locator].push_back(
                            rtps_packets_lost);
                    }
                }
                else
                {
                    // Store the increment since the last report
                    participant->data.rtps_packets_lost[rtps_packets_lost.remote_locator].push_back(
                        rtps_packets_lost -
                        participant->data.last_reported_rtps_packets_lost_count[rtps_packets_lost.remote_locator]);
                    // Update last report
                    participant->data.last_reported_rtps_packets_lost_count[rtps_packets_lost.remote_locator] =
                            rtps_packets_lost;
                }

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

                // Create remote_locator if it does not exist
                get_locator_nts(rtps_bytes_lost.remote_locator);

                // Check if the insertion is from the load
                if (loading)
                {
                    if (last_reported)
                    {
                        // Store last reported
                        participant->data.last_reported_rtps_bytes_lost_count[rtps_bytes_lost.remote_locator] =
                                rtps_bytes_lost;
                    }
                    else
                    {
                        // Store data directly
                        participant->data.rtps_bytes_lost[rtps_bytes_lost.remote_locator].push_back(rtps_bytes_lost);
                    }
                }
                else
                {
                    // Store the increment since the last report
                    participant->data.rtps_bytes_lost[rtps_bytes_lost.remote_locator].push_back(
                        rtps_bytes_lost -
                        participant->data.last_reported_rtps_bytes_lost_count[rtps_bytes_lost.remote_locator]);
                    // Update last report
                    participant->data.last_reported_rtps_bytes_lost_count[rtps_bytes_lost.remote_locator] =
                            rtps_bytes_lost;
                }

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

                // Check if the insertion is from the load
                if (loading)
                {
                    if (last_reported)
                    {
                        // Store last reported
                        writer->data.last_reported_resent_datas = resent_datas;
                    }
                    else
                    {
                        // Store data directly
                        writer->data.resent_datas.push_back(resent_datas);
                    }
                }
                else
                {
                    // Store the increment since the last report
                    writer->data.resent_datas.push_back(resent_datas - writer->data.last_reported_resent_datas);
                    // Update last report
                    writer->data.last_reported_resent_datas = resent_datas;
                }

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

                // Check if the insertion is from the load
                if (loading)
                {
                    if (last_reported)
                    {
                        // Store last reported
                        writer->data.last_reported_heartbeat_count = heartbeat_count;
                    }
                    else
                    {
                        // Store data directly
                        writer->data.heartbeat_count.push_back(heartbeat_count);
                    }
                }
                else
                {
                    // Store the increment since the last report
                    writer->data.heartbeat_count.push_back(heartbeat_count -
                            writer->data.last_reported_heartbeat_count);
                    // Update last report
                    writer->data.last_reported_heartbeat_count = heartbeat_count;
                }

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

                // Check if the insertion is from the load
                if (loading)
                {
                    if (last_reported)
                    {
                        // Store last reported
                        reader->data.last_reported_acknack_count = acknack_count;
                    }
                    else
                    {
                        // Store data directly
                        reader->data.acknack_count.push_back(acknack_count);
                    }
                }
                else
                {
                    // Store the increment since the last report
                    reader->data.acknack_count.push_back(acknack_count - reader->data.last_reported_acknack_count);
                    // Update last report
                    reader->data.last_reported_acknack_count = acknack_count;
                }

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

                // Check if the insertion is from the load
                if (loading)
                {
                    if (last_reported)
                    {
                        // Store last reported
                        reader->data.last_reported_nackfrag_count = nackfrag_count;
                    }
                    else
                    {
                        // Store data directly
                        reader->data.nackfrag_count.push_back(nackfrag_count);
                    }
                }
                else
                {
                    // Store the increment since the last report
                    reader->data.nackfrag_count.push_back(nackfrag_count - reader->data.last_reported_nackfrag_count);
                    // Update last report
                    reader->data.last_reported_nackfrag_count = nackfrag_count;
                }

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

                // Check if the insertion is from the load
                if (loading)
                {
                    if (last_reported)
                    {
                        // Store last reported
                        writer->data.last_reported_gap_count = gap_count;
                    }
                    else
                    {
                        // Store data directly
                        writer->data.gap_count.push_back(gap_count);
                    }
                }
                else
                {
                    // Store the increment since the last report
                    writer->data.gap_count.push_back(gap_count - writer->data.last_reported_gap_count);
                    // Update last report
                    writer->data.last_reported_gap_count = gap_count;
                }

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

                // Check if the insertion is from the load
                if (loading)
                {
                    if (last_reported)
                    {
                        // Store last reported
                        writer->data.last_reported_data_count = data_count;
                    }
                    else
                    {
                        // Store data directly
                        writer->data.data_count.push_back(data_count);
                    }
                }
                else
                {
                    // Store the increment since the last report
                    writer->data.data_count.push_back(data_count - writer->data.last_reported_data_count);
                    // Update last report
                    writer->data.last_reported_data_count = data_count;
                }

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

                // Check if the insertion is from the load
                if (loading)
                {
                    if (last_reported)
                    {
                        // Store last reported
                        participant->data.last_reported_pdp_packets = pdp_packets;
                    }
                    else
                    {
                        // Store data directly
                        participant->data.pdp_packets.push_back(pdp_packets);
                    }

                }
                else
                {
                    // Store the increment since the last report
                    participant->data.pdp_packets.push_back(pdp_packets - participant->data.last_reported_pdp_packets);
                    // Update last report
                    participant->data.last_reported_pdp_packets = pdp_packets;
                }

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

                // Check if the insertion is from the load
                if (loading)
                {
                    if (last_reported)
                    {
                        // Store last reported
                        participant->data.last_reported_edp_packets = edp_packets;
                    }
                    else
                    {
                        // Store data directly
                        participant->data.edp_packets.push_back(edp_packets);
                    }
                }
                else
                {
                    // Store the increment since the last report
                    participant->data.edp_packets.push_back(edp_packets - participant->data.last_reported_edp_packets);
                    // Update last report
                    participant->data.last_reported_edp_packets = edp_packets;
                }

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
                // Only save the last received sample for each sequence number
                writer->data.sample_datas[sample_datas.sequence_number].clear();
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

    link_participant_with_process_nts(participant_id, process_id);
}

void Database::link_participant_with_process_nts(
        const EntityId& participant_id,
        const EntityId& process_id)
{
    /* Get the participant and the domain */
    EntityId domain_id;
    std::map<EntityId, std::shared_ptr<DomainParticipant>>::iterator participant_it;
    bool participant_exists = false;
    for (auto& domain_it : participants_)
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

    // If the participant is active, the process must be active
    if (participant->active)
    {
        if (!participant->process->active)
        {
            change_entity_status_of_kind(participant->process->id, true, EntityKind::PROCESS, domain_id);
        }
    }
}

const std::shared_ptr<const Entity> Database::get_entity(
        const EntityId& entity_id,
        const EntityKind entity_kind /* = EntityKind::INVALID */) const
{
    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    /* Iterate over all the collections looking for the entity */
    if (EntityKind::INVALID == entity_kind || EntityKind::HOST == entity_kind)
    {
        for (const auto& host_it : hosts_)
        {
            if (host_it.second->id == entity_id)
            {
                return host_it.second;
            }
        }
    }

    if (EntityKind::INVALID == entity_kind || EntityKind::PROCESS == entity_kind)
    {
        for (const auto& process_it : processes_)
        {
            if (process_it.second->id == entity_id)
            {
                return process_it.second;
            }
        }
    }

    if (EntityKind::INVALID == entity_kind || EntityKind::USER == entity_kind)
    {
        for (const auto& user_it : users_)
        {
            if (user_it.second->id == entity_id)
            {
                return user_it.second;
            }
        }
    }

    if (EntityKind::INVALID == entity_kind || EntityKind::DOMAIN == entity_kind)
    {
        for (const auto& domain_it : domains_)
        {
            if (domain_it.second->id == entity_id)
            {
                return domain_it.second;
            }
        }
    }

    if (EntityKind::INVALID == entity_kind || EntityKind::TOPIC == entity_kind)
    {
        for (const auto& domain_it : topics_)
        {
            for (const auto& topic_it : domain_it.second)
            {
                if (topic_it.second->id == entity_id)
                {
                    return topic_it.second;
                }
            }
        }
    }

    if (EntityKind::INVALID == entity_kind || EntityKind::PARTICIPANT == entity_kind)
    {
        for (const auto& domain_it : participants_)
        {
            for (const auto& participant_it : domain_it.second)
            {
                if (participant_it.second->id == entity_id)
                {
                    return participant_it.second;
                }
            }
        }
    }

    if (EntityKind::INVALID == entity_kind || EntityKind::DATAREADER == entity_kind)
    {
        for (const auto& domain_it : datareaders_)
        {
            for (const auto& datareader_it : domain_it.second)
            {
                if (datareader_it.second->id == entity_id)
                {
                    return datareader_it.second;
                }
            }
        }
    }

    if (EntityKind::INVALID == entity_kind || EntityKind::DATAWRITER == entity_kind)
    {
        for (const auto& domain_it : datawriters_)
        {
            for (const auto& datawriter_it : domain_it.second)
            {
                if (datawriter_it.second->id == entity_id)
                {
                    return datawriter_it.second;
                }
            }
        }
    }

    if (EntityKind::INVALID == entity_kind || EntityKind::LOCATOR == entity_kind)
    {
        for (const auto& locator_it : locators_)
        {
            if (locator_it.second->id == entity_id)
            {
                return locator_it.second;
            }
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
            for (const auto& host_it : hosts_)
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
            for (const auto& user_it : users_)
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
            for (const auto& process_it : processes_)
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
            for (const auto& domain_it : domains_)
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
            for (const auto& domain_it : participants_)
            {
                for (const auto& participant_it : domain_it.second)
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
            for (const auto& domain_it : topics_)
            {
                for (const auto& topic_it : domain_it.second)
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
            for (const auto& domain_it : datareaders_)
            {
                for (const auto& datareader_it : domain_it.second)
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
            for (const auto& domain_it : datawriters_)
            {
                for (const auto& datawriter_it : domain_it.second)
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
            for (const auto& locator_it : locators_)
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

void Database::erase(
        EntityId& domain_id)
{
    // Check that the given domain_id corresponds to a known monitor.
    // Upper layer ensures that the monitor has been stopped.
    // Upper layer also ensures that the monitor_id is valid and corresponds to a known domain.
    if (EntityKind::DOMAIN != get_entity_kind(domain_id))
    {
        throw BadParameter("Incorrect EntityKind");
    }

    // The mutex should be taken only once. get_entity_kind already locks.
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);

    for (auto& reader : datareaders_[domain_id])
    {
        // Unlink related locators
        for (auto locator : reader.second->locators)
        {
            locators_[locator.first]->data_readers.erase(reader.first);
        }
    }
    // Erase datareaders map element
    datareaders_.erase(domain_id);

    // Remove datawriters and unlink related locators
    for (auto& writer : datawriters_[domain_id])
    {
        // Unlink related locators
        for (auto locator : writer.second->locators)
        {
            locators_[locator.first]->data_writers.erase(writer.first);
        }
    }
    // Erase datawriters map element
    datawriters_.erase(domain_id);

    // Erase topics map element
    topics_.erase(domain_id);

    // Remove participants and unlink related process
    for (auto& participant : participants_[domain_id])
    {
        // Unlink related process if it exists
        if (participant.second->process)
        {
            processes_[participant.second->process->id]->participants.erase(participant.first);
        }
        // Erase locators_by_participant map element
        locators_by_participant_.erase(participant.second->id);
        // Erase participants_by_locator_ participant reference
        for (auto& locator : participants_by_locator_)
        {
            locator.second.erase(participant.second->id);
        }
    }
    // Erase participants map element
    participants_.erase(domain_id);

    // Erase processes_by_domain_ map element
    processes_by_domain_.erase(domain_id);
    // Erase domains_by_process_ domain reference
    for (auto& process : domains_by_process_)
    {
        process.second.erase(domain_id);
    }

    // Erase domain map element
    domains_.erase(domain_id);
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

    if (DataKind::INVALID == data_type)
    {
        throw BadParameter("Incorrect DataKind");
    }

    std::shared_ptr<const eprosima::statistics_backend::database::Entity> source_entity;
    std::shared_ptr<const eprosima::statistics_backend::database::Entity> target_entity;

    for (auto kinds : StatisticsBackend::get_data_supported_entity_kinds(data_type))
    {
        try
        {
            source_entity = get_entity(entity_id_source, kinds.first);
            target_entity = get_entity(entity_id_source, kinds.second);
        }
        catch(const std::exception& e)
        {
            // It has not found the entity, check next kinds possibility
            continue;
        }

        // In case it has found it, follow with that entity
        break;
    }

    std::cout << "select0: " << source_entity << std::endl;
    std::cout << "select0: " << target_entity << std::endl;
    if (source_entity)
    {
        std::cout << "-- select000: " << std::endl;
    }
    if (!source_entity)
    {
        std::cout << "-- select1111: " << std::endl;
    }

    assert(source_entity);
    assert(target_entity);
    assert(!source_entity);

    if (source_entity)
    {
        std::cout << "-- select000: " << std::endl;
    }
    if (!source_entity)
    {
        std::cout << "-- select1111: " << std::endl;
    }

    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    std::vector<const StatisticsSample*> samples;
    switch (data_type)
    {
        case DataKind::FASTDDS_LATENCY:
        {
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

    if (DataKind::INVALID == data_type)
    {
        throw BadParameter("Incorrect DataKind");
    }

    std::shared_ptr<const eprosima::statistics_backend::database::Entity> entity;

    for (auto kinds : StatisticsBackend::get_data_supported_entity_kinds(data_type))
    {
        try
        {
            entity = get_entity(entity_id, kinds.first);
        }
        catch(const std::exception& e)
        {
            // It has not found the entity, check next kinds possibility
            continue;
        }

        // In case it has found it, follow with that entity
        break;
    }

    std::cout << "select1: " << entity << std::endl;

    assert(entity);

    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    std::vector<const StatisticsSample*> samples;
    switch (data_type)
    {
        case DataKind::PUBLICATION_THROUGHPUT:
        {
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
        case DataKind::SAMPLE_DATAS:
        {
            auto writer = std::static_pointer_cast<const DataWriter>(entity);
            /* This case is different from the above. Check all map keys and add sample if it is between the given
               timestamps. The samples do not need to be ordered by source timestamp so they should be sorted */
            for (auto& sample : writer->data.sample_datas)
            {
                // Vector only has one element
                if (sample.second[0].src_ts >= t_from && sample.second[0].src_ts <= t_to)
                {
                    samples.push_back(&sample.second[0]);
                }
            }
            std::sort(samples.begin(), samples.end(), [](
                        const StatisticsSample* first,
                        const StatisticsSample* second)
                    {
                        return first->src_ts < second->src_ts;
                    });
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

std::pair<EntityId, EntityId> Database::get_entity_by_guid(
        EntityKind entity_kind,
        const std::string& guid) const
{
    switch (entity_kind)
    {
        case EntityKind::PARTICIPANT:
        {
            for (const auto& domain_it : participants_)
            {
                for (const auto& participant_it : domain_it.second)
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
            for (const auto& domain_it : datareaders_)
            {
                for (const auto& datareader_it : domain_it.second)
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
            for (const auto& domain_it : datawriters_)
            {
                for (const auto& datawriter_it : domain_it.second)
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
}

EntityKind Database::get_entity_kind(
        EntityId entity_id) const
{
    return get_entity(entity_id)->kind;
}

const std::vector<std::shared_ptr<const Entity>> Database::get_entities(
        EntityKind entity_kind,
        const EntityId& entity_id) const
{
    std::shared_ptr<const Entity> origin;

    // If entity_id is all, return all the entities of type entity_kind
    if (entity_id != EntityId::all())
    {
        // This call will throw BadParameter if there is no such entity.
        // We let this exception through, as it meets expectations
        origin = get_entity(entity_id);
        assert(origin->kind != EntityKind::INVALID);
    }

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
    for (const auto& entity : get_entities(entity_kind, entity_id))
    {
        entitiesIds.push_back(entity->id);
    }

    return entitiesIds;
}

// Auxiliar function to convert a map to a vector
template<typename T>
void map_to_vector(
        const std::map<EntityId, std::shared_ptr<T>>& map,
        std::vector<std::shared_ptr<const Entity>>& vec)
{
    for (auto elem : map)
    {
        vec.push_back(elem.second);
    }
}

// Auxiliar function to convert a map of maps to a vector
template <typename T>
void map_of_maps_to_vector(
        const std::map<EntityId, std::map<EntityId, std::shared_ptr<T>>>& map,
        std::vector<std::shared_ptr<const Entity>>& vec)
{
    for (const auto& elem : map)
    {
        map_to_vector(elem.second, vec);
    }
}

const std::vector<std::shared_ptr<const Entity>> Database::get_entities(
        EntityKind entity_kind,
        const std::shared_ptr<const Entity>& origin) const
{
    std::vector<std::shared_ptr<const Entity>> entities;

    // If entity_id is all, origin is nullptr. Return all the entities of type entity_kind
    if (origin == nullptr)
    {
        switch (entity_kind)
        {
            case EntityKind::HOST:
                map_to_vector(hosts_, entities);
                break;
            case EntityKind::USER:
                map_to_vector(users_, entities);
                break;
            case EntityKind::PROCESS:
                map_to_vector(processes_, entities);
                break;
            case EntityKind::DOMAIN:
                map_to_vector(domains_, entities);
                break;
            case EntityKind::TOPIC:
                map_of_maps_to_vector(topics_, entities);
                break;
            case EntityKind::PARTICIPANT:
                map_of_maps_to_vector(participants_, entities);
                break;
            case EntityKind::DATAWRITER:
                map_of_maps_to_vector(datawriters_, entities);
                break;
            case EntityKind::DATAREADER:
                map_of_maps_to_vector(datareaders_, entities);
                break;
            case EntityKind::LOCATOR:
                map_to_vector(locators_, entities);
                break;
            default:
                throw BadParameter("Invalid EntityKind");
        }
    }
    else
    {
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
                        for (const auto& user : host->users)
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
                        for (const auto& user : host->users)
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
                        for (const auto& process : user->processes)
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
                        for (const auto& process : user->processes)
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
                        for (const auto& participant : process->participants)
                        {
                            entities.push_back(participant.second);
                        }
                        break;
                    case EntityKind::DOMAIN:
                    case EntityKind::TOPIC:
                    case EntityKind::DATAREADER:
                    case EntityKind::DATAWRITER:
                    case EntityKind::LOCATOR:
                        for (const auto& participant : process->participants)
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
                        for (const auto& participant : domain->participants)
                        {
                            entities.push_back(participant.second);
                        }
                        break;
                    case EntityKind::TOPIC:
                        for (const auto& topic : domain->topics)
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
                        for (const auto& participant : domain->participants)
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
                        // May not have the relation process - participant yet
                        if (participant->process)
                        {
                            auto sub_entities = get_entities(entity_kind, participant->process);
                            entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                        }
                    }
                    break;
                    case EntityKind::PROCESS:
                        // May not have the relation process - participant yet
                        if (participant->process)
                        {
                            entities.push_back(participant->process);
                        }
                        break;
                    case EntityKind::DOMAIN:
                        entities.push_back(participant->domain);
                        break;
                    case EntityKind::PARTICIPANT:
                        entities.push_back(participant);
                        break;
                    case EntityKind::DATAWRITER:
                        for (const auto& writer : participant->data_writers)
                        {
                            entities.push_back(writer.second);
                        }
                        break;
                    case EntityKind::DATAREADER:
                        for (const auto& reader : participant->data_readers)
                        {
                            entities.push_back(reader.second);
                        }
                        break;
                    case EntityKind::TOPIC:
                    case EntityKind::LOCATOR:
                        for (const auto& writer : participant->data_writers)
                        {
                            auto sub_entities = get_entities(entity_kind, writer.second);
                            entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                        }
                        for (const auto& reader : participant->data_readers)
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
                        for (const auto& writer : topic->data_writers)
                        {
                            entities.push_back(writer.second);
                        }
                        break;
                    case EntityKind::DATAREADER:
                        for (const auto& reader : topic->data_readers)
                        {
                            entities.push_back(reader.second);
                        }
                        break;
                    case EntityKind::HOST:
                    case EntityKind::USER:
                    case EntityKind::PROCESS:
                    case EntityKind::PARTICIPANT:
                    case EntityKind::LOCATOR:
                        for (const auto& writer : topic->data_writers)
                        {
                            auto sub_entities = get_entities(entity_kind, writer.second);
                            entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                        }
                        for (const auto& reader : topic->data_readers)
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
                        for (const auto& locator : writer->locators)
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
                        for (const auto& locator : reader->locators)
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
                        for (const auto& reader : locator->data_readers)
                        {
                            entities.push_back(reader.second);
                        }
                        break;
                    case EntityKind::DATAWRITER:
                        for (const auto& writer : locator->data_writers)
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
                        for (const auto& writer : locator->data_writers)
                        {
                            auto sub_entities = get_entities(entity_kind, writer.second);
                            entities.insert(entities.end(), sub_entities.begin(), sub_entities.end());
                        }
                        for (const auto& reader : locator->data_readers)
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

template<>
void Database::insert_ddsendpoint_to_locator(
        std::shared_ptr<DataWriter>& endpoint,
        std::shared_ptr<Locator>& locator)
{
    locator->data_writers[endpoint->id] = endpoint;
}

template<>
void Database::insert_ddsendpoint_to_locator(
        std::shared_ptr<DataReader>& endpoint,
        std::shared_ptr<Locator>& locator)
{
    locator->data_readers[endpoint->id] = endpoint;
}

DatabaseDump Database::dump_database(
        bool clear)
{
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);

    DatabaseDump dump = DatabaseDump::object();

    // Add version
    dump[VERSION_TAG] = ACTUAL_DUMP_VERSION;

    // Hosts
    {
        DatabaseDump container = DatabaseDump::object();

        // For each entity of this kind in the database
        for (const auto& it : hosts_)
        {
            container[id_to_string(it.first.value())] = dump_entity_(it.second);
        }

        dump[HOST_CONTAINER_TAG] = container;
    }

    // Users
    {
        DatabaseDump container = DatabaseDump::object();

        // For each entity of this kind in the database
        for (const auto& it : users_)
        {
            container[id_to_string(it.first.value())] = dump_entity_(it.second);
        }

        dump[USER_CONTAINER_TAG] = container;
    }

    // Processes
    {
        DatabaseDump container = DatabaseDump::object();

        // For each entity of this kind in the database
        for (const auto& it : processes_)
        {
            container[id_to_string(it.first.value())] = dump_entity_(it.second);
        }

        dump[PROCESS_CONTAINER_TAG] = container;
    }

    // Domain
    {
        DatabaseDump container = DatabaseDump::object();

        // For each entity of this kind in the database
        for (const auto& it : domains_)
        {
            container[id_to_string(it.first.value())] = dump_entity_(it.second);
        }

        dump[DOMAIN_CONTAINER_TAG] = container;
    }

    // Topic
    {
        DatabaseDump container = DatabaseDump::object();
        // For each domain
        for (const auto& super_it : topics_)
        {
            // For each entity of this kind in the domain
            for (const auto& it : super_it.second)
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
        for (const auto& super_it : participants_)
        {
            // For each entity of this kind in the domain
            for (const auto& it : super_it.second)
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
        for (const auto& super_it : datawriters_)
        {
            // For each entity of this kind in the domain
            for (const auto& it : super_it.second)
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
        for (const auto& super_it : datareaders_)
        {
            // For each entity of this kind in the domain
            for (const auto& it : super_it.second)
            {
                container[id_to_string(it.first.value())] = dump_entity_(it.second);
            }
        }
        dump[DATAREADER_CONTAINER_TAG] = container;
    }

    // Locator
    {
        DatabaseDump container = DatabaseDump::object();

        // For each entity of this kind in the database
        for (const auto& it : locators_)
        {
            container[id_to_string(it.first.value())] = dump_entity_(it.second);
        }

        dump[LOCATOR_CONTAINER_TAG] = container;
    }

    if (clear)
    {
        clear_statistics_data();
    }

    return dump;
}

DatabaseDump Database::dump_entity_(
        const std::shared_ptr<Host>& entity)
{
    DatabaseDump entity_info = DatabaseDump::object();
    entity_info[NAME_INFO_TAG] = entity->name;
    entity_info[ALIAS_INFO_TAG] = entity->alias;

    // Populate subentity array
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->users)
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
    entity_info[ALIAS_INFO_TAG] = entity->alias;

    entity_info[HOST_ENTITY_TAG] = id_to_string(entity->host->id);

    // Populate subentity array
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->processes)
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
    entity_info[ALIAS_INFO_TAG] = entity->alias;
    entity_info[PID_INFO_TAG] = entity->pid;

    entity_info[USER_ENTITY_TAG] = id_to_string(entity->user->id);

    // Populate subentity array
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->participants)
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
    entity_info[ALIAS_INFO_TAG] = entity->alias;

    // Populate subentity array for Topics
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->topics)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[TOPIC_CONTAINER_TAG] = subentities;
    }

    // Populate subentity array for Participants
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->participants)
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
    entity_info[ALIAS_INFO_TAG] = entity->alias;
    entity_info[DATA_TYPE_INFO_TAG] = entity->data_type;

    entity_info[DOMAIN_ENTITY_TAG] = id_to_string(entity->domain->id);

    // Populate subentity array for DataWriters
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->data_writers)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[DATAWRITER_CONTAINER_TAG] = subentities;
    }

    // Populate subentity array for DataReaders
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->data_readers)
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
    entity_info[ALIAS_INFO_TAG] = entity->alias;
    entity_info[GUID_INFO_TAG] = entity->guid;
    entity_info[QOS_INFO_TAG] = entity->qos;

    entity_info[DOMAIN_ENTITY_TAG] = id_to_string(entity->domain->id);
    if (entity->process)
    {
        entity_info[PROCESS_ENTITY_TAG] = id_to_string(entity->process->id);
    }
    else
    {
        entity_info[PROCESS_ENTITY_TAG] = id_to_string(EntityId::invalid());
    }

    // Populate subentity array for DataWriters
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->data_writers)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[DATAWRITER_CONTAINER_TAG] = subentities;
    }
    // Populate subentity array for DataReaders
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->data_readers)
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
    entity_info[ALIAS_INFO_TAG] = entity->alias;
    entity_info[GUID_INFO_TAG] = entity->guid;
    entity_info[QOS_INFO_TAG] = entity->qos;

    entity_info[PARTICIPANT_ENTITY_TAG] = id_to_string(entity->participant->id);
    entity_info[TOPIC_ENTITY_TAG] = id_to_string(entity->topic->id);

    // Populate subentity array for Locators
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->locators)
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

        // resent_data last reported
        data[DATA_KIND_RESENT_DATA_LAST_REPORTED_TAG] = dump_data_(entity->data.last_reported_resent_datas);

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
    entity_info[ALIAS_INFO_TAG] = entity->alias;
    entity_info[GUID_INFO_TAG] = entity->guid;
    entity_info[QOS_INFO_TAG] = entity->qos;

    entity_info[PARTICIPANT_ENTITY_TAG] = id_to_string(entity->participant->id);
    entity_info[TOPIC_ENTITY_TAG] = id_to_string(entity->topic->id);

    // Populate subentity array for Locators
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->locators)
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
    entity_info[ALIAS_INFO_TAG] = entity->alias;

    // Populate subentity array for DataWriters
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->data_writers)
        {
            subentities.push_back(id_to_string(sub_it.first));
        }
        entity_info[DATAWRITER_CONTAINER_TAG] = subentities;
    }

    // Populate subentity array for DataReaders
    {
        DatabaseDump subentities = DatabaseDump::array();
        for (const auto& sub_it : entity->data_readers)
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

    for (const auto& it : data)
    {
        DatabaseDump samples = DatabaseDump::array();

        for (const auto& sample : it.second)
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

    for (const auto& it : data)
    {
        DatabaseDump samples = DatabaseDump::array();

        for (const auto& sample : it.second)
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

    for (const auto& it : data)
    {
        DatabaseDump samples = DatabaseDump::array();

        for (const auto& sample : it.second)
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

    for (const auto& it : data)
    {
        DatabaseDump samples = DatabaseDump::array();

        for (const auto& sample : it.second)
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

    for (const auto& it : data)
    {
        DatabaseDump samples = DatabaseDump::array();

        for (const auto& sample : it.second)
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

    for (const auto& it : data)
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

    for (const auto& it : data)
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

    for (const auto& it : data)
    {
        data_dump[id_to_string(it.first.value())] = dump_data_(it.second);
    }

    return data_dump;
}

DatabaseDump Database::dump_data_(
        const std::map<EntityId, ByteCountSample>& data)
{
    DatabaseDump data_dump = DatabaseDump::object();

    for (const auto& it : data)
    {
        data_dump[id_to_string(it.first.value())] = dump_data_(it.second);
    }

    return data_dump;
}

void Database::clear_statistics_data()
{
    // Participants
    for (const auto& super_it : participants_)
    {
        // For each entity of this kind in the domain
        for (const auto& it : super_it.second)
        {
            it.second->data.clear(false);
        }
    }
    // Datawriters
    for (const auto& super_it : datawriters_)
    {
        // For each entity of this kind in the domain
        for (const auto& it : super_it.second)
        {
            it.second->data.clear(false);
        }
    }
    // Datareaders
    for (const auto& super_it : datareaders_)
    {
        // For each entity of this kind in the domain
        for (const auto& it : super_it.second)
        {
            it.second->data.clear(false);
        }
    }
    // Locators
    for (const auto& it : locators_)
    {
        it.second->data.clear();
    }
}

/**
 * @brief Check that in the 'dump', the references of the entity iterator 'it' of type 'entity_tag'
 * to entities of type 'reference_tag' are consistent and mutual. For this, the referenced entities must
 * have reference to 'it' of type 'entity_tag'.
 *
 * Example -> Check that each user, reference host[0]:
 *
 * \code
 * {
 *      host["0"]
 *      {
 *          users: ["1","5","9"]
 *      }
 *      user["1"]
 *      {
 *          host: "0"
 *      }
 * }
 * \endcode
 *
 * @param dump reference to the database dump.
 * @param it iterator to the dump of the entity.
 * @param entity_tag Type of the entity to check.
 * @param reference_tag Type of the referenced entity to check.
 * @throws eprosima::statistics_backend::FileCorrupted if the references are not consistent and mutual.
 */
void check_entity_contains_all_references(
        DatabaseDump const& dump,
        nlohmann::json::iterator const& it,
        std::string const& entity_tag,
        std::string const& reference_container_tag,
        std::string const& reference_tag)
{
    std::string entity_id = it.key();
    DatabaseDump references_id = (*it).at(reference_tag);
    DatabaseDump reference_container = dump.at(reference_container_tag);

    // Check all 'references_id' in the 'reference_container'
    for (auto refIt = references_id.begin(); refIt != references_id.end(); ++refIt)
    {
        std::string referenced_id = *refIt;

        // 1) Check that the 'referenced_id' entity exists.
        if (!reference_container.contains(referenced_id))
        {
            throw CorruptedFile(
                      "Entity container: " + reference_container.dump() + " do not have a Entity with ID: " +
                      referenced_id);
        }

        // 2) Check that referenced entity contains a reference to an 'entity_id' of type 'entity_tag'.
        std::vector<std::string> referenced_entities;
        auto referenced_entities_json = reference_container.at(referenced_id).at(entity_tag);
        if (referenced_entities_json.is_array())
        {
            referenced_entities.insert(referenced_entities.end(),
                    referenced_entities_json.begin(),
                    referenced_entities_json.end());
        }
        else
        {
            referenced_entities.push_back(referenced_entities_json.get<std::string>());
        }

        if (std::find(referenced_entities.begin(), referenced_entities.end(), entity_id) == referenced_entities.end())
        {
            throw CorruptedFile("Entity with ID (" + referenced_id + ") :" + reference_container.at(
                              referenced_id).dump() +
                          " has reference to " + entity_tag + ": " +
                          reference_container.at(referenced_id).at(entity_tag).dump() +
                          " instead of " + entity_tag + ": " + entity_id);
        }
    }
}

void Database::load_database(
        const DatabaseDump& dump)
{
    std::unique_lock<std::shared_timed_mutex> lock(mutex_);

    if (next_id_ != 0)
    {
        throw PreconditionNotMet("Error: Database not empty");
    }

    // Locators
    {
        DatabaseDump container = dump.at(LOCATOR_CONTAINER_TAG);
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, LOCATOR_CONTAINER_TAG, DATAWRITER_CONTAINER_TAG,
                    DATAWRITER_CONTAINER_TAG);
            check_entity_contains_all_references(dump, it, LOCATOR_CONTAINER_TAG, DATAREADER_CONTAINER_TAG,
                    DATAREADER_CONTAINER_TAG);

            // Create entity
            std::shared_ptr<Locator> entity = std::make_shared<Locator>((*it).at(NAME_INFO_TAG));
            entity->alias = (*it).at(ALIAS_INFO_TAG);

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);

            // Load data and insert into database
            load_data((*it).at(DATA_CONTAINER_TAG), entity);
        }
    }

    // Hosts
    {
        DatabaseDump container = dump.at(HOST_CONTAINER_TAG);

        // For each entity of this kind in the database
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, HOST_ENTITY_TAG, USER_CONTAINER_TAG, USER_CONTAINER_TAG);

            // Create entity
            std::shared_ptr<Host> entity = std::make_shared<Host>((*it).at(NAME_INFO_TAG));
            entity->alias = (*it).at(ALIAS_INFO_TAG);

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);
        }
    }

    // Users
    {
        DatabaseDump container = dump.at(USER_CONTAINER_TAG);

        // For each entity of this kind in the database
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, USER_CONTAINER_TAG, HOST_CONTAINER_TAG, HOST_ENTITY_TAG);
            check_entity_contains_all_references(dump, it, USER_ENTITY_TAG, PROCESS_CONTAINER_TAG,
                    PROCESS_CONTAINER_TAG);

            // Create entity
            std::shared_ptr<User> entity = std::make_shared<User>((*it).at(NAME_INFO_TAG),
                            hosts_[string_to_int((*it).at(HOST_ENTITY_TAG))]);
            entity->alias = (*it).at(ALIAS_INFO_TAG);

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);
        }
    }

    // Processes
    {
        DatabaseDump container = dump.at(PROCESS_CONTAINER_TAG);

        // For each entity of this kind in the database
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, PROCESS_CONTAINER_TAG, USER_CONTAINER_TAG, USER_ENTITY_TAG);
            check_entity_contains_all_references(dump, it, PROCESS_ENTITY_TAG, PARTICIPANT_CONTAINER_TAG,
                    PARTICIPANT_CONTAINER_TAG);

            // Create entity
            std::shared_ptr<Process> entity =
                    std::make_shared<Process>((*it).at(NAME_INFO_TAG), (*it).at(PID_INFO_TAG),
                            users_[EntityId(string_to_int((*it).at(USER_ENTITY_TAG)))]);
            entity->alias = (*it).at(ALIAS_INFO_TAG);

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);
        }
    }

    // Domains
    {
        DatabaseDump container = dump.at(DOMAIN_CONTAINER_TAG);

        // For each entity of this kind in the database
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, DOMAIN_ENTITY_TAG, PARTICIPANT_CONTAINER_TAG,
                    PARTICIPANT_CONTAINER_TAG);
            check_entity_contains_all_references(dump, it, DOMAIN_ENTITY_TAG, TOPIC_CONTAINER_TAG, TOPIC_CONTAINER_TAG);

            // Create entity
            std::shared_ptr<Domain> entity = std::make_shared<Domain>((*it).at(NAME_INFO_TAG));
            entity->alias = (*it).at(ALIAS_INFO_TAG);

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);
        }
    }

    // Topics
    {
        DatabaseDump container = dump.at(TOPIC_CONTAINER_TAG);

        // For each entity of this kind in the database
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, TOPIC_CONTAINER_TAG, DOMAIN_CONTAINER_TAG,
                    DOMAIN_ENTITY_TAG);
            check_entity_contains_all_references(dump, it, TOPIC_ENTITY_TAG, DATAWRITER_CONTAINER_TAG,
                    DATAWRITER_CONTAINER_TAG);
            check_entity_contains_all_references(dump, it, TOPIC_ENTITY_TAG, DATAREADER_CONTAINER_TAG,
                    DATAREADER_CONTAINER_TAG);

            // Create entity
            std::shared_ptr<Topic> entity =
                    std::make_shared<Topic>((*it).at(NAME_INFO_TAG), (*it).at(DATA_TYPE_INFO_TAG),
                            domains_[EntityId(string_to_int((*it).at(DOMAIN_ENTITY_TAG)))]);
            entity->alias = (*it).at(ALIAS_INFO_TAG);

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);
        }
    }

    // Participants
    {
        DatabaseDump container = dump.at(PARTICIPANT_CONTAINER_TAG);

        // For each entity of this kind in the database
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, PARTICIPANT_CONTAINER_TAG, DOMAIN_CONTAINER_TAG,
                    DOMAIN_ENTITY_TAG);
            check_entity_contains_all_references(dump, it, PARTICIPANT_ENTITY_TAG, DATAWRITER_CONTAINER_TAG,
                    DATAWRITER_CONTAINER_TAG);
            check_entity_contains_all_references(dump, it, PARTICIPANT_ENTITY_TAG, DATAREADER_CONTAINER_TAG,
                    DATAREADER_CONTAINER_TAG);

            // Create entity
            std::shared_ptr<DomainParticipant> entity = std::make_shared<DomainParticipant>(
                (*it).at(NAME_INFO_TAG), (*it).at(QOS_INFO_TAG), (*it).at(GUID_INFO_TAG), nullptr,
                domains_[EntityId(string_to_int((*it).at(DOMAIN_ENTITY_TAG)))]);
            entity->alias = (*it).at(ALIAS_INFO_TAG);

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);

            // Link participant with process
            EntityId process_id(string_to_int((*it).at(PROCESS_ENTITY_TAG)));

            if (process_id != EntityId::invalid())
            {
                check_entity_contains_all_references(dump, it, PARTICIPANT_CONTAINER_TAG, PROCESS_CONTAINER_TAG,
                        PROCESS_ENTITY_TAG);

                link_participant_with_process_nts(entity->id, process_id);
            }

            // Load data and insert into database
            load_data((*it).at(DATA_CONTAINER_TAG), entity);
        }
    }

    // DataWriters
    {
        DatabaseDump container = dump.at(DATAWRITER_CONTAINER_TAG);

        // For each entity of this kind in the database
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, DATAWRITER_CONTAINER_TAG, PARTICIPANT_CONTAINER_TAG,
                    PARTICIPANT_ENTITY_TAG);
            check_entity_contains_all_references(dump, it, DATAWRITER_CONTAINER_TAG, TOPIC_CONTAINER_TAG,
                    TOPIC_ENTITY_TAG);
            check_entity_contains_all_references(dump, it, DATAWRITER_CONTAINER_TAG, LOCATOR_CONTAINER_TAG,
                    LOCATOR_CONTAINER_TAG);

            // Get keys
            EntityId participant_id = EntityId(string_to_int((*it).at(PARTICIPANT_ENTITY_TAG)));
            EntityId participant_domain_id =
                    EntityId(string_to_int(dump.at(PARTICIPANT_CONTAINER_TAG)
                                    .at(std::to_string(participant_id.value()))
                                    .at(DOMAIN_ENTITY_TAG)));

            EntityId topic_id = EntityId(string_to_int((*it).at(TOPIC_ENTITY_TAG)));
            EntityId topic_domain_id = EntityId(string_to_int(dump.at(TOPIC_CONTAINER_TAG)
                                    .at(std::to_string(topic_id.value()))
                                    .at(DOMAIN_ENTITY_TAG)));

            // Create entity
            std::shared_ptr<DataWriter> entity = std::make_shared<DataWriter>(
                (*it).at(NAME_INFO_TAG),
                (*it).at(QOS_INFO_TAG),
                (*it).at(GUID_INFO_TAG),
                participants_[participant_domain_id][participant_id],
                topics_[topic_domain_id][topic_id]);
            entity->alias = (*it).at(ALIAS_INFO_TAG);

            /* Add reference to locator to the endpoint */
            for (auto it_loc = (*it).at(LOCATOR_CONTAINER_TAG).begin();
                    it_loc != (*it).at(LOCATOR_CONTAINER_TAG).end();
                    ++it_loc)
            {
                entity->locators[string_to_int(*it_loc)] =
                        locators_[EntityId(string_to_int(*it_loc))];
            }

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);

            // Load data and insert into database
            load_data((*it).at(DATA_CONTAINER_TAG), entity);
        }
    }

    // DataReaders
    {
        DatabaseDump container = dump.at(DATAREADER_CONTAINER_TAG);
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            // Check that entity has correct references to other entities
            check_entity_contains_all_references(dump, it, DATAREADER_CONTAINER_TAG, PARTICIPANT_CONTAINER_TAG,
                    PARTICIPANT_ENTITY_TAG);
            check_entity_contains_all_references(dump, it, DATAREADER_CONTAINER_TAG, TOPIC_CONTAINER_TAG,
                    TOPIC_ENTITY_TAG);
            check_entity_contains_all_references(dump, it, DATAREADER_CONTAINER_TAG, LOCATOR_CONTAINER_TAG,
                    LOCATOR_CONTAINER_TAG);

            // Get keys
            EntityId participant_id = EntityId(string_to_int((*it).at(
                                PARTICIPANT_ENTITY_TAG)));
            EntityId participant_domain_id =
                    EntityId(string_to_int(dump.at(PARTICIPANT_CONTAINER_TAG)
                                    .at(std::to_string(participant_id.value()))
                                    .at(DOMAIN_ENTITY_TAG)));

            EntityId topic_id = EntityId(string_to_int((*it).at(TOPIC_ENTITY_TAG)));
            EntityId topic_domain_id = EntityId(string_to_int(dump.at(TOPIC_CONTAINER_TAG)
                                    .at(std::to_string(topic_id.value()))
                                    .at(DOMAIN_ENTITY_TAG)));

            // Create entity
            std::shared_ptr<DataReader> entity = std::make_shared<DataReader>(
                (*it).at(NAME_INFO_TAG),
                (*it).at(QOS_INFO_TAG),
                (*it).at(GUID_INFO_TAG),
                participants_[participant_domain_id][participant_id],
                topics_[topic_domain_id][topic_id]);
            entity->alias = (*it).at(ALIAS_INFO_TAG);

            /* Add reference to locator to the endpoint */
            for (auto it_loc = (*it).at(LOCATOR_CONTAINER_TAG).begin();
                    it_loc != (*it).at(LOCATOR_CONTAINER_TAG).end();
                    ++it_loc)
            {
                entity->locators[string_to_int(*it_loc)] =
                        locators_[EntityId(string_to_int(*it_loc))];
            }

            // Insert into database
            EntityId entity_id = EntityId(string_to_int(it.key()));
            insert_nts(entity, entity_id);

            // Load data and insert into database
            load_data((*it).at(DATA_CONTAINER_TAG), entity);
        }
    }
}

void Database::load_data(
        const DatabaseDump& dump,
        const std::shared_ptr<DomainParticipant>& entity)
{
    // discovery_time
    {
        DatabaseDump container = dump.at(DATA_KIND_DISCOVERY_TIME_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            // Data iterator
            for (auto it = container.at(remote_it.key()).begin(); it != container.at(remote_it.key()).end(); ++it)
            {
                DiscoveryTimeSample sample;

                // std::chrono::system_clock::time_point
                uint64_t src_ts = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
                sample.src_ts = nanoseconds_to_systemclock(src_ts);

                // std::chrono::system_clock::time_point
                uint64_t time = string_to_uint(std::string((*it).at(DATA_VALUE_TIME_TAG)));
                sample.time = nanoseconds_to_systemclock(time);

                // EntityId
                sample.remote_entity = EntityId(string_to_int(remote_it.key()));

                string_to_int(std::string((*it).at(DATA_VALUE_REMOTE_ENTITY_TAG)));

                // bool
                sample.discovered = (*it).at(DATA_VALUE_DISCOVERED_TAG);

                // Insert data into database
                insert_nts(entity->domain->id, entity->id, sample, true);
            }
        }
    }

    // pdp_packets
    {
        DatabaseDump container = dump.at(DATA_KIND_PDP_PACKETS_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            PdpCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->domain->id, entity->id, sample, true);
        }
    }

    // edp_packets
    {
        DatabaseDump container = dump.at(DATA_KIND_EDP_PACKETS_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            EdpCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->domain->id, entity->id, sample, true);
        }
    }

    // rtps_packets_sent
    {
        DatabaseDump container = dump.at(DATA_KIND_RTPS_PACKETS_SENT_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            // Data iterator
            for (auto it = container.at(remote_it.key()).begin(); it != container.at(remote_it.key()).end(); ++it)
            {
                RtpsPacketsSentSample sample;

                // std::chrono::system_clock::time_point
                uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
                sample.src_ts = nanoseconds_to_systemclock(time);

                // uint64_t
                sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

                // EntityId
                sample.remote_locator = EntityId(string_to_int(remote_it.key()));

                // Insert data into database
                insert_nts(entity->domain->id, entity->id, sample, true);
            }
        }
    }

    // rtps_bytes_sent
    {
        DatabaseDump container = dump.at(DATA_KIND_RTPS_BYTES_SENT_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            // Data iterator
            for (auto it = container.at(remote_it.key()).begin(); it != container.at(remote_it.key()).end(); ++it)
            {
                RtpsBytesSentSample sample;

                // std::chrono::system_clock::time_point
                uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
                sample.src_ts = nanoseconds_to_systemclock(time);

                // uint64_t
                sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

                // int16_t
                sample.magnitude_order =
                        static_cast<int16_t>(string_to_int(to_string((*it).at(DATA_VALUE_MAGNITUDE_TAG))));

                // EntityId
                sample.remote_locator = EntityId(string_to_int(remote_it.key()));

                // Insert data into database
                insert_nts(entity->domain->id, entity->id, sample, true);
            }
        }
    }

    // rtps_packets_lost
    {
        DatabaseDump container = dump.at(DATA_KIND_RTPS_PACKETS_LOST_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            // Data iterator
            for (auto it = container.at(remote_it.key()).begin(); it != container.at(remote_it.key()).end(); ++it)
            {
                RtpsPacketsLostSample sample;

                // std::chrono::system_clock::time_point
                uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
                sample.src_ts = nanoseconds_to_systemclock(time);

                // uint64_t
                sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

                // EntityId
                sample.remote_locator = EntityId(string_to_int(remote_it.key()));

                // Insert data into database
                insert_nts(entity->domain->id, entity->id, sample, true);
            }
        }
    }

    // rtps_bytes_lost
    {
        DatabaseDump container = dump.at(DATA_KIND_RTPS_BYTES_LOST_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            // Data iterator
            for (auto it = container.at(remote_it.key()).begin(); it != container.at(remote_it.key()).end(); ++it)
            {
                RtpsBytesLostSample sample;

                // std::chrono::system_clock::time_point
                uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
                sample.src_ts = nanoseconds_to_systemclock(time);

                // uint64_t
                sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

                // int16_t
                sample.magnitude_order =
                        static_cast<int16_t>(string_to_int(to_string((*it).at(DATA_VALUE_MAGNITUDE_TAG))));

                // EntityId
                sample.remote_locator = EntityId(string_to_int(remote_it.key()));

                // Insert data into database
                insert_nts(entity->domain->id, entity->id, sample, true);
            }
        }
    }

    // last_reported_rtps_bytes_lost
    {
        DatabaseDump container = dump.at(DATA_KIND_RTPS_BYTES_LOST_LAST_REPORTED_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            RtpsBytesLostSample sample;
            DatabaseDump sample_dump = container.at(remote_it.key());

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(sample_dump.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string(sample_dump.at(DATA_VALUE_COUNT_TAG)));

            // int16_t
            sample.magnitude_order =
                    static_cast<int16_t>(string_to_int(to_string(sample_dump.at(DATA_VALUE_MAGNITUDE_TAG))));

            // EntityId
            sample.remote_locator = EntityId(string_to_int(remote_it.key()));

            // Insert data into database
            insert_nts(entity->domain->id, entity->id, sample, true, true);
        }
    }

    // last_reported_rtps_bytes_sent
    {
        DatabaseDump container = dump.at(DATA_KIND_RTPS_BYTES_SENT_LAST_REPORTED_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            RtpsBytesSentSample sample;
            DatabaseDump sample_dump = container.at(remote_it.key());

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(sample_dump.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string(sample_dump.at(DATA_VALUE_COUNT_TAG)));

            // int16_t
            sample.magnitude_order =
                    static_cast<int16_t>(string_to_int(to_string(sample_dump.at(DATA_VALUE_MAGNITUDE_TAG))));

            // EntityId
            sample.remote_locator = EntityId(string_to_int(remote_it.key()));

            // Insert data into database
            insert_nts(entity->domain->id, entity->id, sample, true, true);
        }
    }

    // last_reported_rtps_packets_lost
    {
        DatabaseDump container = dump.at(DATA_KIND_RTPS_PACKETS_LOST_LAST_REPORTED_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            RtpsPacketsLostSample sample;
            DatabaseDump sample_dump = container.at(remote_it.key());

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(sample_dump.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string(sample_dump.at(DATA_VALUE_COUNT_TAG)));

            // EntityId
            sample.remote_locator = EntityId(string_to_int(remote_it.key()));

            // Insert data into database
            insert_nts(entity->domain->id, entity->id, sample, true, true);
        }
    }

    // last_reported_rtps_packets_sent
    {
        DatabaseDump container = dump.at(DATA_KIND_RTPS_PACKETS_SENT_LAST_REPORTED_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            RtpsPacketsSentSample sample;
            DatabaseDump sample_dump = container.at(remote_it.key());

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(sample_dump.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string(sample_dump.at(DATA_VALUE_COUNT_TAG)));

            // EntityId
            sample.remote_locator = EntityId(string_to_int(remote_it.key()));

            // Insert data into database
            insert_nts(entity->domain->id, entity->id, sample, true, true);
        }
    }

    //last_reported_edp_packets
    {
        // Only insert last reported if there are at least one
        if (!dump.at(DATA_KIND_EDP_PACKETS_TAG).empty())
        {
            DatabaseDump container = dump.at(DATA_KIND_EDP_PACKETS_LAST_REPORTED_TAG);

            EdpCountSample sample;

            //std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(container.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t count;
            sample.count = string_to_uint(to_string(container.at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->domain->id, entity->id, sample, true, true);
        }
    }

    // last_reported_pdp_packets
    {
        // Only insert last reported if there are at least one
        if (!dump.at(DATA_KIND_PDP_PACKETS_TAG).empty())
        {
            DatabaseDump container = dump.at(DATA_KIND_PDP_PACKETS_LAST_REPORTED_TAG);

            PdpCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(container.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t count;
            sample.count = string_to_uint(to_string(container.at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->domain->id, entity->id, sample, true, true);
        }
    }
}

void Database::load_data(
        const DatabaseDump& dump,
        const std::shared_ptr<DataWriter>& entity)
{
    // publication_throughput
    {
        DatabaseDump container = dump.at(DATA_KIND_PUBLICATION_THROUGHPUT_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            PublicationThroughputSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // double
            sample.data = (*it).at(DATA_VALUE_DATA_TAG);

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true);
        }
    }

    // resent_datas
    {
        DatabaseDump container = dump.at(DATA_KIND_RESENT_DATA_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            ResentDataSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true);
        }
    }

    // heartbeat_count
    {
        DatabaseDump container = dump.at(DATA_KIND_HEARTBEAT_COUNT_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            HeartbeatCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true);
        }
    }

    // gap_count
    {
        DatabaseDump container = dump.at(DATA_KIND_GAP_COUNT_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            GapCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true);
        }
    }

    // data_count
    {
        DatabaseDump container = dump.at(DATA_KIND_DATA_COUNT_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            DataCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true);
        }
    }

    // samples_datas
    {
        DatabaseDump container = dump.at(DATA_KIND_SAMPLE_DATAS_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            // Data iterator
            for (auto it = container.at(remote_it.key()).begin(); it != container.at(remote_it.key()).end(); ++it)
            {
                SampleDatasCountSample sample;

                // std::chrono::system_clock::time_point
                uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
                sample.src_ts = nanoseconds_to_systemclock(time);

                // uint64_t
                sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

                // uint64_t
                sample.sequence_number = string_to_int(remote_it.key());

                // Insert data into database
                insert_nts(entity->participant->domain->id, entity->id, sample, true);
            }
        }
    }

    // history2history_latency
    {
        DatabaseDump container = dump.at(DATA_KIND_FASTDDS_LATENCY_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            // Data iterator
            for (auto it = container.at(remote_it.key()).begin(); it != container.at(remote_it.key()).end(); ++it)
            {
                HistoryLatencySample sample;

                // std::chrono::system_clock::time_point
                uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
                sample.src_ts = nanoseconds_to_systemclock(time);

                // double
                sample.data = (*it).at(DATA_VALUE_DATA_TAG);

                // EntityId
                sample.reader = EntityId(string_to_int(remote_it.key()));

                // Insert data into database
                insert_nts(entity->participant->domain->id, entity->id, sample, true);
            }
        }
    }

    // last_reported_data_count
    {
        // Only insert last reported if there are at least one
        if (!dump.at(DATA_KIND_DATA_COUNT_TAG).empty())
        {
            DatabaseDump container = dump.at(DATA_KIND_DATA_COUNT_LAST_REPORTED_TAG);

            DataCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(container.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t count;
            sample.count = string_to_uint(to_string(container.at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true, true);
        }
    }

    // last_reported_gap_count
    {
        // Only insert last reported if there are at least one
        if (!dump.at(DATA_KIND_GAP_COUNT_TAG).empty())
        {
            DatabaseDump container = dump.at(DATA_KIND_GAP_COUNT_LAST_REPORTED_TAG);

            GapCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(container.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t count;
            sample.count = string_to_uint(to_string(container.at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true, true);
        }
    }

    // last_reported_heartbeat_count
    {
        // Only insert last reported if there are at least one
        if (!dump.at(DATA_KIND_HEARTBEAT_COUNT_TAG).empty())
        {
            DatabaseDump container = dump.at(DATA_KIND_HEARTBEAT_COUNT_LAST_REPORTED_TAG);

            HeartbeatCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(container.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t count;
            sample.count = string_to_uint(to_string(container.at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true, true);
        }
    }

    // last_reported_resent_datas
    {
        // Only insert last reported if there are at least one
        if (!dump.at(DATA_KIND_RESENT_DATA_TAG).empty())
        {
            DatabaseDump container = dump.at(DATA_KIND_RESENT_DATA_LAST_REPORTED_TAG);

            ResentDataSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(container.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t count;
            sample.count = string_to_uint(to_string(container.at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true, true);
        }
    }
}

void Database::load_data(
        const DatabaseDump& dump,
        const std::shared_ptr<DataReader>& entity)
{
    // subscription_throughput
    {
        DatabaseDump container = dump.at(DATA_KIND_SUBSCRIPTION_THROUGHPUT_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            SubscriptionThroughputSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // double
            sample.data = (*it).at(DATA_VALUE_DATA_TAG);

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true);
        }
    }

    // acknack_count
    {
        DatabaseDump container = dump.at(DATA_KIND_ACKNACK_COUNT_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            AcknackCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true);
        }
    }

    // nackfrag_count
    {
        DatabaseDump container = dump.at(DATA_KIND_NACKFRAG_COUNT_TAG);

        // Data iterator
        for (auto it = container.begin(); it != container.end(); ++it)
        {
            NackfragCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t
            sample.count = string_to_uint(to_string((*it).at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true);
        }
    }

    // last_reported_acknack_count
    {
        // Only insert last reported if there are at least one
        if (!dump.at(DATA_KIND_ACKNACK_COUNT_TAG).empty())
        {
            DatabaseDump container = dump.at(DATA_KIND_ACKNACK_COUNT_LAST_REPORTED_TAG);

            AcknackCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(container.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t count;
            sample.count = string_to_uint(to_string(container.at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true, true);
        }
    }

    // last_reported_nackfrag_count
    {
        // Only insert last reported if there are at least one
        if (!dump.at(DATA_KIND_NACKFRAG_COUNT_TAG).empty())
        {
            DatabaseDump container = dump.at(DATA_KIND_NACKFRAG_COUNT_LAST_REPORTED_TAG);

            NackfragCountSample sample;

            // std::chrono::system_clock::time_point
            uint64_t time = string_to_uint(std::string(container.at(DATA_VALUE_SRC_TIME_TAG)));
            sample.src_ts = nanoseconds_to_systemclock(time);

            // uint64_t count;
            sample.count = string_to_uint(to_string(container.at(DATA_VALUE_COUNT_TAG)));

            // Insert data into database
            insert_nts(entity->participant->domain->id, entity->id, sample, true, true);
        }
    }
}

void Database::load_data(
        const DatabaseDump& dump,
        const std::shared_ptr<Locator>& entity)
{
    // NetworkLatency
    {
        DatabaseDump container = dump.at(DATA_KIND_NETWORK_LATENCY_TAG);

        // RemoteEntities iterator
        for (auto remote_it = container.begin(); remote_it != container.end(); ++remote_it)
        {
            // Data iterator
            for (auto it = container.at(remote_it.key()).begin(); it != container.at(remote_it.key()).end(); ++it)
            {
                NetworkLatencySample sample;

                // std::chrono::system_clock::time_point
                uint64_t time = string_to_uint((*it).at(DATA_VALUE_SRC_TIME_TAG));
                sample.src_ts = nanoseconds_to_systemclock(time);

                // double
                sample.data = (*it).at(DATA_VALUE_DATA_TAG);

                // EntityId
                sample.remote_locator = EntityId(string_to_int(remote_it.key()));

                // Insert data into database
                insert_nts(EntityId::invalid(), entity->id, sample, true);
            }
        }
    }
}

// Conversion from bool to DiscoveryStatus
details::StatisticsBackendData::DiscoveryStatus get_status(
        bool active)
{
    if (active)
    {
        return details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
    }
    else
    {
        return details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY;
    }
}

void Database::change_entity_status_of_kind(
        const EntityId& entity_id,
        bool active,
        const EntityKind& entity_kind,
        const EntityId& domain_id) noexcept
{
    switch (entity_kind)
    {
        case EntityKind::HOST:
        {
            std::shared_ptr<Host> host;

            for (const auto& host_it : hosts_)
            {
                if (host_it.second->id == entity_id)
                {
                    host = host_it.second;
                    break;
                }
            }
            if (host != nullptr && host->active != active)
            {
                host->active = active;
                details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(entity_id,
                        entity_kind, get_status(active));
            }
            break;
        }
        case EntityKind::USER:
        {
            std::shared_ptr<User> user;

            for (const auto& user_it : users_)
            {
                if (user_it.second->id == entity_id)
                {
                    user = user_it.second;
                    break;
                }
            }
            if (user != nullptr && user->active != active)
            {
                user->active = active;

                // host
                {
                    bool change_status = true;

                    // If the entity now is active, the subentity must be active.
                    // But if the subentity is already active, is not necessary to change its status.
                    if (active && user->host->active)
                    {
                        change_status = false;
                    }
                    // If the entity now is inactive, check if one reference of the subentity is active.
                    // If one is still active, the status of the subentity is not changed
                    else if (!active)
                    {
                        for (const auto& entity_it : user->host->users)
                        {
                            if (entity_it.second->active)
                            {
                                change_status = false;
                                break;
                            }
                        }
                    }

                    if (change_status)
                    {
                        change_entity_status_of_kind(user->host->id, active, user->host->kind, domain_id);
                    }
                }

                // Discovering user after discovering host
                details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(entity_id,
                        entity_kind, get_status(active));
            }
            break;
        }
        case EntityKind::PROCESS:
        {
            std::shared_ptr<Process> process;

            for (const auto& process_it : processes_)
            {
                if (process_it.second->id == entity_id)
                {
                    process = process_it.second;
                    break;
                }
            }
            if (process != nullptr && process->active != active)
            {
                process->active = active;

                // user
                {
                    bool change_status = true;

                    // If the entity now is active, the subentity must be active.
                    // But if the subentity is already active, is not necessary to change its status.
                    if (active && process->user->active)
                    {
                        change_status = false;
                    }
                    // If the entity now is inactive, check if one reference of the subentity is active.
                    // If one is still active, the status of the subentity is not changed
                    else if (!active)
                    {
                        for (const auto& entity_it : process->user->processes)
                        {
                            if (entity_it.second->active)
                            {
                                change_status = false;
                                break;
                            }
                        }
                    }

                    if (change_status)
                    {
                        change_entity_status_of_kind(process->user->id, active, process->user->kind, domain_id);
                    }
                }

                // Discovering process after discovering user
                details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(entity_id,
                        entity_kind, get_status(active));
            }
            break;
        }
        case EntityKind::DOMAIN:
        {
            std::shared_ptr<Domain> domain;

            for (const auto& domain_it : domains_)
            {
                if (domain_it.second->id == entity_id)
                {
                    domain = domain_it.second;
                    break;
                }
            }
            if (domain != nullptr && domain->active != active)
            {
                domain->active = active;
            }
            break;
        }
        case EntityKind::TOPIC:
        {
            std::shared_ptr<Topic> topic;

            for (const auto& domain_it : topics_)
            {
                for (const auto& topic_it : domain_it.second)
                {
                    if (topic_it.second->id == entity_id)
                    {
                        topic = topic_it.second;
                        break;
                    }
                }
            }
            if (topic != nullptr && topic->active != active)
            {
                topic->active = active;

                details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(domain_id, entity_id,
                        entity_kind, get_status(active));
            }
            break;
        }
        case EntityKind::PARTICIPANT:
        {
            std::shared_ptr<DomainParticipant> participant;

            for (const auto& domain_it : participants_)
            {
                for (const auto& participant_it : domain_it.second)
                {
                    if (participant_it.second->id == entity_id)
                    {
                        participant = participant_it.second;
                        break;
                    }
                }
            }

            if (participant != nullptr && participant->active != active)
            {
                participant->active = active;

                // process
                if (participant->process != nullptr)
                {
                    bool change_status = true;

                    // If the entity now is active, the subentity must be active.
                    // But if the subentity is already active, is not necessary to change its status.
                    if (active && participant->process->active)
                    {
                        change_status = false;
                    }
                    // If the entity now is inactive, check if one reference of the subentity is active.
                    // If one is still active, the status of the subentity is not changed
                    else if (!active)
                    {
                        for (const auto& entity_it : participant->process->participants)
                        {
                            if (entity_it.second->active)
                            {
                                change_status = false;
                                break;
                            }
                        }
                    }

                    if (change_status)
                    {
                        change_entity_status_of_kind(participant->process->id, active, participant->process->kind,
                                participant->domain->id);
                    }
                }
            }

            break;
        }
        case EntityKind::DATAWRITER:
        {
            std::shared_ptr<DataWriter> datawriter;

            for (const auto& domain_it : datawriters_)
            {
                for (const auto& datawriter_it : domain_it.second)
                {
                    if (datawriter_it.second->id == entity_id)
                    {
                        datawriter = datawriter_it.second;
                        break;
                    }
                }
            }

            if (datawriter != nullptr && (datawriter->active != active || datawriter->topic->active != active))
            {
                datawriter->active = active;

                // topic
                {
                    bool change_status = true;

                    // If the entity now is active, the subentity must be active.
                    // But if the subentity is already active, is not necessary to change its status.
                    if (active && datawriter->topic->active)
                    {
                        change_status = false;
                    }
                    // If the entity now is inactive, check if one reference of the subentity is active.
                    // If one is still active, the status of the subentity is not changed
                    else if (!active)
                    {
                        for (const auto& entity_it : datawriter->topic->data_writers)
                        {
                            if (entity_it.second->active)
                            {
                                change_status = false;
                                break;
                            }
                        }
                        if (change_status)
                        {
                            for (const auto& entity_it : datawriter->topic->data_readers)
                            {
                                if (entity_it.second->active)
                                {
                                    change_status = false;
                                    break;
                                }
                            }
                        }
                    }

                    if (change_status)
                    {
                        change_entity_status_of_kind(datawriter->topic->id, active, datawriter->topic->kind,
                                datawriter->participant->domain->id);
                    }
                }
            }

            break;
        }
        case EntityKind::DATAREADER:
        {
            std::shared_ptr<DataReader> datareader;

            for (const auto& domain_it : datareaders_)
            {
                for (const auto& datareader_it : domain_it.second)
                {
                    if (datareader_it.second->id == entity_id)
                    {
                        datareader = datareader_it.second;
                        break;
                    }
                }
            }
            if (datareader != nullptr && (datareader->active != active || datareader->topic->active != active))
            {
                datareader->active = active;

                // topic
                {
                    bool change_status = true;

                    // If the entity now is active, the subentity must be active.
                    // But if the subentity is already active, is not necessary to change its status.
                    if (active && datareader->topic->active)
                    {
                        change_status = false;
                    }
                    // If the entity now is inactive, check if one reference of the subentity is active.
                    // If one is still active, the status of the subentity is not changed
                    else if (!active)
                    {
                        for (const auto& entity_it : datareader->topic->data_readers)
                        {
                            if (entity_it.second->active)
                            {
                                change_status = false;
                                break;
                            }
                        }
                        if (change_status)
                        {
                            for (const auto& entity_it : datareader->topic->data_writers)
                            {
                                if (entity_it.second->active)
                                {
                                    change_status = false;
                                    break;
                                }
                            }
                        }
                    }

                    if (change_status)
                    {
                        change_entity_status_of_kind(datareader->topic->id, active, datareader->topic->kind,
                                datareader->participant->domain->id);
                    }
                }
            }

            break;
        }
        default:
        {
            break;
        }
    }
}

void Database::change_entity_status(
        const EntityId& entity_id,
        bool active)
{
    EntityKind entity_kind = get_entity_kind(entity_id);

    // Check that the entity is a discovered/undiscovered dds_entity or a started/stopped monitor
    assert(
        entity_kind == EntityKind::PARTICIPANT || entity_kind == EntityKind::DATAWRITER ||
        entity_kind == EntityKind::DATAREADER || entity_kind == EntityKind::DOMAIN);

    std::shared_lock<std::shared_timed_mutex> lock(mutex_);
    change_entity_status_of_kind(entity_id, active, entity_kind);
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
