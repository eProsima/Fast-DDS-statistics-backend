// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include "HelloWorldPublisher.h"
#include <fastrtps/attributes/ParticipantAttributes.h>
#include <fastrtps/attributes/PublisherAttributes.h>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>

#include <thread>
#include <csignal>

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

std::atomic<bool> HelloWorldPublisher::stop_(false);
std::mutex HelloWorldPublisher::PubListener::wait_matched_cv_mtx_;
std::condition_variable HelloWorldPublisher::PubListener::wait_matched_cv_;

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
    PubListener::awake();
}

bool HelloWorldPublisher::init(
        uint32_t domain,
        uint32_t num_wait_matched)
{
    hello_.index(0);
    hello_.message("HelloWorld");
    DomainParticipantQos pqos;
    pqos.name("Participant_pub");
    listener_.set_num_wait_matched(num_wait_matched);

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
        "PHYSICAL_DATA_TOPIC");

    participant_ = DomainParticipantFactory::get_instance()->create_participant(domain, pqos);

    if (participant_ == nullptr)
    {
        return false;
    }

    //REGISTER THE TYPE
    type_.register_type(participant_);

    //CREATE THE PUBLISHER
    publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);

    if (publisher_ == nullptr)
    {
        return false;
    }

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
    return true;
}

HelloWorldPublisher::~HelloWorldPublisher()
{
    if (writer_ != nullptr)
    {
        publisher_->delete_datawriter(writer_);
    }
    if (publisher_ != nullptr)
    {
        participant_->delete_publisher(publisher_);
    }
    if (topic_ != nullptr)
    {
        participant_->delete_topic(topic_);
    }
    DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

void HelloWorldPublisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.total_count;
        std::cout << "Publisher matched." << std::endl;
        if (enough_matched())
        {
            awake();
        }
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.total_count;
        std::cout << "Publisher unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void HelloWorldPublisher::PubListener::set_num_wait_matched(
        uint32_t num_wait_matched)
{
    num_wait_matched_ = num_wait_matched;
}

bool HelloWorldPublisher::PubListener::enough_matched()
{
    return matched_ >= num_wait_matched_;
}

void HelloWorldPublisher::PubListener::wait()
{
    std::unique_lock<std::mutex> lck(wait_matched_cv_mtx_);
    wait_matched_cv_.wait(lck, [this]
            {
                return enough_matched() || is_stopped();
            });
}

void HelloWorldPublisher::PubListener::awake()
{
    wait_matched_cv_.notify_one();
}

void HelloWorldPublisher::runThread(
        uint32_t samples,
        uint32_t sleep)
{
    if (samples == 0)
    {
        while (!is_stopped())
        {
            if (listener_.enough_matched())
            {
                publish();
                std::cout << "Message: " << hello_.message() << " with index: " << hello_.index()
                          << " SENT" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
            }
            else
            {
                listener_.wait();
            }
        }
    }
    else
    {
        for (uint32_t i = 0; i < samples; ++i)
        {
            if (is_stopped())
            {
                break;
            }
            if (listener_.enough_matched())
            {
                publish();
                std::cout << "Message: " << hello_.message() << " with index: " << hello_.index()
                          << " SENT" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
            }
            else
            {
                --i;
                listener_.wait();
            }
        }
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
        std::cout << "Publisher running " << samples << " samples." << std::endl;
    }
    signal(SIGINT, [](int signum)
            {
                static_cast<void>(signum); HelloWorldPublisher::stop();
            });
    thread.join();
}

void HelloWorldPublisher::publish()
{
    hello_.index(hello_.index() + 1);
    writer_->write(&hello_);
}
