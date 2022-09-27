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

#include <string>
#include <vector>
#include <chrono>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <cmath>

#include "Monitor.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds_statistics_backend/listener/DomainListener.hpp>
#include <fastdds_statistics_backend/StatisticsBackend.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/types.hpp>

#include <prometheus/client_metric.h>
#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/family.h>
#include <prometheus/registry.h>

using namespace eprosima::statistics_backend;
using namespace eprosima::fastdds::dds;
using namespace prometheus;

Monitor::Monitor()
    : registry_(std::make_shared<Registry>())
    , fastdds_latency_mean_(nullptr)
    , publication_throughput_mean_(nullptr)
{
}

Monitor::~Monitor()
{
    StatisticsBackend::stop_monitor(monitor_id_);
}

bool Monitor::init()
{
    /******************************
     * Initialize prometheus server
     ******************************/
    auto& fastdds_latency_gauge = BuildGauge()
            .Name("fastdds_latency")
            .Help("Fast DDS Latency")
            .Register(*registry_);

    fastdds_latency_mean_ =
        &fastdds_latency_gauge.Add({{"statistic", "mean"}});

    auto& publication_throughput_gauge = BuildGauge()
            .Name("publication_throughput")
            .Help("Publication throughput")
            .Register(*registry_);

    publication_throughput_mean_ =
        &publication_throughput_gauge.Add({{"statistic", "mean"}});

    /* Ask the exposer to scrape the registry on incoming HTTP requests */
    exposer_.RegisterCollectable(registry_);

    /******************************
     * Initialize monitor
     ******************************/
    monitor_id_ = StatisticsBackend::init_monitor(0);
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
    std::cout << "Monitor running. Please press enter to stop the Monitor" << std::endl;

    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        get_fastdds_latency_mean();
        get_publication_throughput_mean();
    }
}

/***************************************************************
 * Implementation of the functions to collect the data.
 ***************************************************************/

std::vector<StatisticsData> Monitor::get_fastdds_latency_mean()
{
    std::vector<StatisticsData> latency_data{};

    std::vector<EntityId> topics = StatisticsBackend::get_entities(EntityKind::TOPIC);
    EntityId chatter_topic_id = -1;
    Info topic_info;
    for (auto topic_id : topics)
    {
        topic_info = StatisticsBackend::get_info(topic_id);
        if (topic_info["name"] == "rt/chatter" && topic_info["data_type"] == "std_msgs::msg::dds_::String_")
        {
            chatter_topic_id = topic_id;
            break;
        }
    }

    if (chatter_topic_id < 0)
    {
        return latency_data;
    }

    /* Get the DataWriters and DataReaders in a Topic */
    std::vector<EntityId> topic_datawriters = StatisticsBackend::get_entities(
            EntityKind::DATAWRITER,
            chatter_topic_id);
    std::vector<EntityId> topic_datareaders = StatisticsBackend::get_entities(
            EntityKind::DATAREADER,
            chatter_topic_id);

    /* Get the current time */
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    /*
    * Get the mean of the FASTDDS_LATENCY of the last 5 seconds
    * between the DataWriters and DataReaders publishing under and subscribed to the rt/chatter topic.
    */
    latency_data = StatisticsBackend::get_data(
        DataKind::FASTDDS_LATENCY,                                   // DataKind
        topic_datawriters,                                           // Source entities
        topic_datareaders,                                           // Target entities
        1,                                                           // Number of bins
        now - std::chrono::seconds(5),                               // t_from
        now,                                                         // t_to
        StatisticKind::MEAN);                                        // Statistic

    for (auto latency : latency_data)
    {
        if (std::isnan(latency.second))
        {
            return latency_data;
        }

        std::int64_t timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                latency.first.time_since_epoch()).count();
        fastdds_latency_mean_->Set(latency.second/1000, timestamp_ms);

        std::cout << "ROS 2 Latency in topic " << topic_info["name"] << ": ["
                  << timestamp_to_string(latency.first) << ", " << latency.second/1000 << " μs]" << std::endl;
    }

    return latency_data;
}

std::vector<StatisticsData> Monitor::get_publication_throughput_mean()
{
    std::vector<StatisticsData> publication_throughput_data{};

    std::vector<EntityId> topics = StatisticsBackend::get_entities(EntityKind::TOPIC);
    EntityId chatter_topic_id = -1;
    Info topic_info;
    for (auto topic_id : topics)
    {
        topic_info = StatisticsBackend::get_info(topic_id);
        if (topic_info["name"] == "rt/chatter" && topic_info["data_type"] == "std_msgs::msg::dds_::String_")
        {
            chatter_topic_id = topic_id;
            break;
        }
    }

    if (chatter_topic_id < 0)
    {
        return publication_throughput_data;
    }

    /* Get the DataWriters and DataReaders in a Topic */
    std::vector<EntityId> chatter_datawriters = StatisticsBackend::get_entities(
            EntityKind::DATAWRITER,
            chatter_topic_id);

    /* Get the current time */
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

    publication_throughput_data = StatisticsBackend::get_data(
        DataKind::PUBLICATION_THROUGHPUT,                            // DataKind
        chatter_datawriters,                                         // Source entities
        1,                                                           // Number of bins
        now - std::chrono::seconds(10),                               // t_from
        now,                                                         // t_to
        StatisticKind::MEAN);                                        // Statistic

    for (auto publication_throughput : publication_throughput_data)
    {
        if (std::isnan(publication_throughput.second))
        {
            return publication_throughput_data;
        }

        std::int64_t timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                publication_throughput.first.time_since_epoch()).count();
        publication_throughput_mean_->Set(publication_throughput.second, timestamp_ms);

        std::cout << "Publication throughput in topic " << topic_info["name"] << ": ["
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

    if (status.current_count_change == 1)
    {
        std::cout << "Participant " << participant_id << " discovered." << std::endl;
    }
    else
    {
        std::cout << "Participant " << participant_id << " update info." << std::endl;
    }
}

void Monitor::Listener::on_datareader_discovery(
        EntityId domain_id,
        EntityId datareader_id,
        const DomainListener::Status& status)
{
    static_cast<void>(domain_id);

    if (status.current_count_change == 1)
    {
        std::cout << "DataReader " << datareader_id << " discovered." << std::endl;
    }
    else
    {
        std::cout << "DataReader " << datareader_id << " update info." << std::endl;
    }
}

void Monitor::Listener::on_datawriter_discovery(
        EntityId domain_id,
        EntityId datawriter_id,
        const DomainListener::Status& status)
{
    static_cast<void>(domain_id);

    if (status.current_count_change == 1)
    {
        std::cout << "DataWriter " << datawriter_id << " discovered." << std::endl;
    }
    else
    {
        std::cout << "DataWriter " << datawriter_id << " update info." << std::endl;
    }
}

void Monitor::Listener::on_host_discovery(
        EntityId host_id,
        const DomainListener::Status& status)
{
    Info host_info = StatisticsBackend::get_info(host_id);
    if (status.current_count_change == 1)
    {
        std::cout << "Host " << host_info["name"] << " discovered." << std::endl;
    }
    else
    {
        std::cout << "Host " << host_info["name"] << " update info." << std::endl;
    }
}

void Monitor::Listener::on_user_discovery(
        EntityId user_id,
        const DomainListener::Status& status)
{
    Info user_info = StatisticsBackend::get_info(user_id);
    if (status.current_count_change == 1)
    {
        std::cout << "User " << user_info["name"] << " discovered." << std::endl;
    }
    else
    {
        std::cout << "User " << user_info["name"] << " update info." << std::endl;
    }
}

void Monitor::Listener::on_process_discovery(
        EntityId process_id,
        const DomainListener::Status& status)
{
    Info process_info = StatisticsBackend::get_info(process_id);
    if (status.current_count_change == 1)
    {
        std::cout << "Process " << process_info["name"] << " discovered." << std::endl;
    }
    else
    {
        std::cout << "Process " << process_info["name"] << " update info." << std::endl;
    }
}

void Monitor::Listener::on_locator_discovery(
        EntityId locator_id,
        const DomainListener::Status& status)
{
    if (status.current_count_change == 1)
    {
        std::cout << "Locator " << locator_id << " discovered." << std::endl;
    }
    else
    {
        std::cout << "Locator " << locator_id << " update info." << std::endl;
    }
}

void Monitor::Listener::on_topic_discovery(
        EntityId domain_id,
        EntityId topic_id,
        const DomainListener::Status& status)
{
    Info topic_info = StatisticsBackend::get_info(topic_id);
    Info domain_info = StatisticsBackend::get_info(domain_id);
    if (status.current_count_change == 1)
    {
        std::cout << "Topic " << topic_info["name"] << ":" << topic_info["data_type"]
                  << " discovered in Domain " << domain_info["name"] << std::endl;
    }
    else
    {
        std::cout << "Topic " << topic_info["name"] << ":" << topic_info["data_type"]
                  << " updated info in Domain " << domain_info["name"] << std::endl;
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

