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
 * @file HelloWorldPublisher.cpp
 *
 */

#include <csignal>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <xmlparser/attributes/ParticipantAttributes.hpp>
#include <xmlparser/attributes/PublisherAttributes.hpp>

#include "HelloWorldPublisher.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

std::atomic<bool> HelloWorldPublisher::stop_(false);

HelloWorldPublisher::HelloWorldPublisher()
    : participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
    , type_(new HelloWorldPubSubType())
{
}

bool HelloWorldPublisher::is_stopped()
{
    return stop_;
}

void HelloWorldPublisher::stop()
{
    stop_ = true;
}

bool HelloWorldPublisher::init(
        uint32_t domain)
{
    hello_.index(0);
    hello_.message("HelloWorld");
    DomainParticipantQos pqos;
    pqos.name("Participant_pub");

    // Activate Fast DDS Statistics module
    pqos.properties().properties().emplace_back("fastdds.statistics",
            "HISTORY_LATENCY_TOPIC;" \
            "NETWORK_LATENCY_TOPIC;" \
            "PUBLICATION_THROUGHPUT_TOPIC;" \
            "SUBSCRIPTION_THROUGHPUT_TOPIC;" \
            "RTPS_SENT_TOPIC;" \
            "RTPS_LOST_TOPIC;" \
            "HEARTBEAT_COUNT_TOPIC;" \
            "ACKNACK_COUNT_TOPIC;" \
            "NACKFRAG_COUNT_TOPIC;" \
            "GAP_COUNT_TOPIC;" \
            "DATA_COUNT_TOPIC;" \
            "RESENT_DATAS_TOPIC;" \
            "SAMPLE_DATAS_TOPIC;" \
            "PDP_PACKETS_TOPIC;" \
            "EDP_PACKETS_TOPIC;" \
            "DISCOVERY_TOPIC;" \
            "PHYSICAL_DATA_TOPIC;" \
            "MONITOR_SERVICE_TOPIC");

    // CREATE THE PARTICIPANT
    participant_ = DomainParticipantFactory::get_instance()->create_participant(domain, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    std::cout << "Participant " << pqos.name() << " created with GUID " << participant_->guid() << std::endl;

    // REGISTER THE TYPE
    type_.register_type(participant_);

    // CREATE THE PUBLISHER
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    if (publisher_ == nullptr)
    {
        return false;
    }

    // CREATE THE TOPIC
    topic_ = participant_->create_topic("HelloWorldTopic", "HelloWorld", TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        return false;
    }

    // CREATE THE WRITER
    writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &listener_);

    if (writer_ == nullptr)
    {
        return false;
    }

    std::cout << "DataWriter created with GUID " << writer_->guid() << std::endl;

    return true;
}

HelloWorldPublisher::~HelloWorldPublisher()
{
    if (participant_ != nullptr)
    {
        if (publisher_ != nullptr)
        {
            if (writer_ != nullptr)
            {
                publisher_->delete_datawriter(writer_);
            }
            participant_->delete_publisher(publisher_);
        }
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void HelloWorldPublisher::PubListener::on_publication_matched(
        DataWriter*,
        const PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.current_count;
        std::cout << "Publisher matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.current_count;
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void HelloWorldPublisher::runThread(
        uint32_t samples,
        uint32_t sleep)
{
    while (!is_stopped() && (samples == 0 || hello_.index() < samples))
    {
        publish();
        std::cout << "Message: " << hello_.message() << " with index: " << hello_.index()
                  << " SENT" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }
}

void HelloWorldPublisher::run(
        uint32_t samples,
        uint32_t sleep)
{
    stop_ = false;
    std::thread thread(&HelloWorldPublisher::runThread, this, samples, sleep);
    if (samples == 0)
    {
        std::cout << "Publisher running. Please press CTRL+C to stop the Publisher at any time." << std::endl;
    }
    else
    {
        std::cout << "Publisher running " << samples <<
            " samples. Please press CTRL+C to stop the Publisher at any time." << std::endl;
    }
    signal(SIGINT, [](int signum)
            {
                std::cout << "SIGINT received, stopping Publisher execution." << std::endl;
                static_cast<void>(signum); HelloWorldPublisher::stop();
            });
    thread.join();
}

void HelloWorldPublisher::publish()
{
    hello_.index(hello_.index() + 1);
    writer_->write(&hello_);
}
