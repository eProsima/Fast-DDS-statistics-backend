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
 * @file Monitor.cpp
 */

#include <chrono>
#include <csignal>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <fastdds_statistics_backend/listener/DomainListener.hpp>
#include <fastdds_statistics_backend/StatisticsBackend.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/types.hpp>

#include "Monitor.h"

using namespace eprosima::statistics_backend;

std::atomic<bool> Monitor::stop_(false);
std::mutex Monitor::terminate_cv_mtx_;
std::condition_variable Monitor::terminate_cv_;

Monitor::Monitor()
{
}

Monitor::~Monitor()
{
    StatisticsBackend::stop_monitor(monitor_id_);
}

bool Monitor::is_stopped()
{
    return stop_;
}

void Monitor::stop()
{
    stop_ = true;
    terminate_cv_.notify_all();
}

bool Monitor::init(
        uint32_t domain,
        uint32_t n_bins,
        uint32_t t_interval,
        std::string bump_file /* = "" */,
        bool reset /* = false */)
{
    n_bins_ = n_bins;
    t_interval_ = t_interval;
    monitor_id_ = StatisticsBackend::init_monitor(domain);
    bump_file_ = bump_file;
    reset_ = reset;

    if (!monitor_id_.is_valid())
    {
        std::cout << "Error creating monitor" << std::endl;
        return 1;
    }

    StatisticsBackend::set_physical_listener(&physical_listener_);

    return true;
}

void Monitor::run()
{
    stop_ = false;
    std::cout << "Monitor running. Please press CTRL+C to stop the Monitor at any time." << std::endl;
    signal(SIGINT, [](int signum)
            {
                std::cout << "SIGINT received, stopping Monitor execution." << std::endl;
                static_cast<void>(signum); Monitor::stop();
            });

    while (!is_stopped())
    {
        std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
        terminate_cv_.wait_for(lck, std::chrono::seconds(t_interval_), []
                {
                    return is_stopped();
                });
        std::cout << std::endl;
        get_fastdds_latency_mean();
        get_publication_throughput_mean();

        // TODO: decide whether inactive entities must be inside json bump
        // If not so, reset must be done before bumping file
        if (!bump_file_.empty())
        {
            bump_in_file();
        }

        if (reset_)
        {
            clear_inactive_entities();
            std::cout << "Removing inactive entities from Statistics Backend." << std::endl;
        }
    }
}

void Monitor::bump_in_file()
{
    // Get current timestamp
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");
    std::string current_time = oss.str();

    // Get file name
    std::string complete_file_name = bump_file_ + "_" + current_time + ".json";
    std::cout << "Bumping info in file " << complete_file_name << std::endl;

    // Bump to get json
    auto dump = StatisticsBackend::dump_database(reset_);

    // Store it in json file
    std::ofstream file(complete_file_name);
    // Pretty print json
    file << std::setw(4) << dump << std::endl;
}

void Monitor::clear_inactive_entities()
{
    StatisticsBackend::clear_inactive_entities();
}

/***************************************************************
* Implementation of the functions to collect the data.
***************************************************************/

std::vector<StatisticsData> Monitor::get_fastdds_latency_mean()
{
    std::vector<StatisticsData> latency_data{};

    std::vector<EntityId> topics = StatisticsBackend::get_entities(EntityKind::TOPIC);
    EntityId helloworld_topic_id = -1;
    Info topic_info;
    for (auto topic_id : topics)
    {
        topic_info = StatisticsBackend::get_info(topic_id);
        if (topic_info["name"] == "HelloWorldTopic" && topic_info["data_type"] == "HelloWorld")
        {
            helloworld_topic_id = topic_id;
        }
    }

    if (helloworld_topic_id < 0)
    {
        return latency_data;
    }

    /* Get the DataWriters and DataReaders in a Topic */
    std::vector<EntityId> topic_datawriters = StatisticsBackend::get_entities(
        EntityKind::DATAWRITER,
        helloworld_topic_id);
    std::vector<EntityId> topic_datareaders = StatisticsBackend::get_entities(
        EntityKind::DATAREADER,
        helloworld_topic_id);

    /* Get the current time */
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    /*
     * Get the median of the FASTDDS_LATENCY of the last 10 minutes
     * between the DataWriters and DataReaders publishing under and subscribed to the HelloWorld topic.
     */
    latency_data = StatisticsBackend::get_data(
        DataKind::FASTDDS_LATENCY,                                   // DataKind
        topic_datawriters,                                           // Source entities
        topic_datareaders,                                           // Target entities
        n_bins_,                                                     // Number of bins
        now - std::chrono::seconds(t_interval_),                     // t_from
        now,                                                         // t_to
        StatisticKind::MEAN);                                        // Statistic

    for (auto latency : latency_data)
    {

        std::cout << "Fast DDS Latency of HelloWorld topic: ["
                  << timestamp_to_string(latency.first) << ", " << latency.second / 1000 << " μs]" << std::endl;
    }

    return latency_data;
}

std::vector<StatisticsData> Monitor::get_publication_throughput_mean()
{
    std::vector<StatisticsData> publication_throughput_data{};

    std::vector<EntityId> participants = StatisticsBackend::get_entities(EntityKind::PARTICIPANT);
    EntityId participant_id = -1;
    Info participant_info;
    for (auto participant : participants)
    {
        participant_info = StatisticsBackend::get_info(participant);
        if (participant_info["name"] == "Participant_pub" && participant_info["alive"])
        {
            participant_id = participant;
            break;
        }
    }

    if (participant_id < 0)
    {
        return publication_throughput_data;
    }

    /* Get the DataWriters and DataReaders in a Topic */
    std::vector<EntityId> topic_datawriters = StatisticsBackend::get_entities(
        EntityKind::DATAWRITER,
        participant_id);

    /* Get the current time */
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    /*
     *
     */
    publication_throughput_data = StatisticsBackend::get_data(
        DataKind::PUBLICATION_THROUGHPUT,                            // DataKind
        topic_datawriters,                                           // Source entities
        n_bins_,                                                     // Number of bins
        now - std::chrono::seconds(t_interval_),                     // t_from
        now,                                                         // t_to
        StatisticKind::MEAN);                                        // Statistic

    for (auto publication_throughput : publication_throughput_data)
    {

        std::cout << "Publication throughput of Participant " << std::string(participant_info["name"]) << ": ["
                  << timestamp_to_string(publication_throughput.first) << ", "
                  << publication_throughput.second << " B/s]" << std::endl;
    }

    return publication_throughput_data;
}

/***************************************************************
* Monitor Listener callbacks implementation
***************************************************************/
void Monitor::Listener::on_participant_discovery(
        EntityId domain_id,
        EntityId participant_id,
        const DomainListener::Status& status)
{
    static_cast<void>(domain_id);
    Info participant_info = StatisticsBackend::get_info(participant_id);

    if (status.current_count_change == 1)
    {
        std::cout << "Participant with GUID " << std::string(participant_info["guid"]) << " discovered." << std::endl;
    }
    else
    {
        std::cout << "Participant with GUID " << std::string(participant_info["guid"]) << " update info." << std::endl;
    }
}

void Monitor::Listener::on_datareader_discovery(
        EntityId domain_id,
        EntityId datareader_id,
        const DomainListener::Status& status)
{
    static_cast<void>(domain_id);
    Info datareader_info = StatisticsBackend::get_info(datareader_id);

    if (!datareader_info["metatraffic"])
    {
        if (status.current_count_change == 1)
        {
            std::cout << "DataReader with GUID " << std::string(datareader_info["guid"]) << " discovered." << std::endl;
        }
        else
        {
            std::cout << "DataReader with GUID " << std::string(datareader_info["guid"]) << " update info." <<
                std::endl;
        }
    }
}

void Monitor::Listener::on_datawriter_discovery(
        EntityId domain_id,
        EntityId datawriter_id,
        const DomainListener::Status& status)
{
    static_cast<void>(domain_id);
    Info datawriter_info = StatisticsBackend::get_info(datawriter_id);

    if (!datawriter_info["metatraffic"])
    {
        if (status.current_count_change == 1)
        {
            std::cout << "DataWriter with GUID " << std::string(datawriter_info["guid"]) << " discovered." << std::endl;
        }
        else
        {
            std::cout << "DataWriter with GUID " << std::string(datawriter_info["guid"]) << " update info." <<
                std::endl;
        }
    }
}

void Monitor::Listener::on_host_discovery(
        EntityId host_id,
        const DomainListener::Status& status)
{
    Info host_info = StatisticsBackend::get_info(host_id);

    if (status.current_count_change == 1)
    {
        std::cout << "Host " << std::string(host_info["name"]) << " discovered." << std::endl;
    }
    else
    {
        std::cout << "Host " << std::string(host_info["name"]) << " update info." << std::endl;
    }
}

void Monitor::Listener::on_user_discovery(
        EntityId user_id,
        const DomainListener::Status& status)
{
    Info user_info = StatisticsBackend::get_info(user_id);

    if (status.current_count_change == 1)
    {
        std::cout << "User " << std::string(user_info["name"]) << " discovered." << std::endl;
    }
    else
    {
        std::cout << "User " << std::string(user_info["name"]) << " update info." << std::endl;
    }
}

void Monitor::Listener::on_process_discovery(
        EntityId process_id,
        const DomainListener::Status& status)
{
    Info process_info = StatisticsBackend::get_info(process_id);

    if (status.current_count_change == 1)
    {
        std::cout << "Process " << std::string(process_info["name"]) << " discovered." << std::endl;
    }
    else
    {
        std::cout << "Process " << std::string(process_info["name"]) << " update info." << std::endl;
    }
}

void Monitor::Listener::on_topic_discovery(
        EntityId domain_id,
        EntityId topic_id,
        const DomainListener::Status& status)
{
    static_cast<void>(domain_id);
    Info topic_info = StatisticsBackend::get_info(topic_id);

    if (!topic_info["metatraffic"])
    {
        if (status.current_count_change == 1)
        {
            std::cout << "Topic " << std::string(topic_info["name"]) << " discovered." << std::endl;
        }
        else
        {
            std::cout << "Topic " << std::string(topic_info["name"]) << " update info." << std::endl;
        }
    }
}

/***************************************************************
* Utils
***************************************************************/
std::string Monitor::timestamp_to_string(
        const Timestamp timestamp)
{
    auto timestamp_t = std::chrono::system_clock::to_time_t(timestamp);
    auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp.time_since_epoch()).count();
    msec %= 1000;
    std::stringstream ss;

#ifdef _WIN32
    struct tm timestamp_tm;
    _localtime64_s(&timestamp_tm, &timestamp_t);
    ss << std::put_time(&timestamp_tm, "%F %T") << "." << std::setw(3) << std::setfill('0') << msec;
#else
    ss << std::put_time(localtime(&timestamp_t), "%F %T") << "." << std::setw(3) << std::setfill('0') << msec;
#endif // ifdef _WIN32

    return ss.str();
}
