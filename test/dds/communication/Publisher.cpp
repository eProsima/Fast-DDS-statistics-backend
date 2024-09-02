// Copyright 2024 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file Publisher.cpp
 *
 */

#include <csignal>
#include <thread>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/qos/DataWriterQos.hpp>
#include <fastdds/dds/publisher/qos/PublisherQos.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>

#include "Publisher.hpp"
#include "CommunicationPubSubTypes.hpp"
#include "Host.hpp"

std::mutex mutex;
std::condition_variable cv;

std::atomic<bool> Publisher::stop_(false);
Publisher::Publisher()
    : participant_(nullptr)
    , publisher_(nullptr)
    , topic_(nullptr)
    , writer_(nullptr)
    , type_(new CommunicationPubSubType())
{
}

bool Publisher::is_stopped()
{
    return stop_;
}

void Publisher::stop()
{
    stop_ = true;
}

bool Publisher::init(
        uint32_t domain)
{
    comm_.index(0);
    comm_.message("HelloWorld");
    eprosima::fastdds::dds::DomainParticipantQos pqos;
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
    participant_ = eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(domain, pqos);

    if (participant_ == nullptr)
    {
        std::cout << "Error creating publisher participant" << std::endl;
        return 1;
    }

    std::cout << "Participant " << pqos.name() << " created with GUID " << participant_->guid() << std::endl;

    // REGISTER THE TYPE
    type_.register_type(participant_);

    // CREATE THE PUBLISHER
    publisher_ = participant_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);

    if (publisher_ == nullptr)
    {
        std::cerr << "Error creating publisher" << std::endl;
        return 1;
    }

    // CREATE THE TOPIC
    // Generate topic name
    std::ostringstream topic_name;
    topic_name << "HelloWorldTopic_" << (eprosima::Host::instance().id()) << "_" << domain;
    topic_ = participant_->create_topic(topic_name.str(),
                    type_.get_type_name(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        std::cerr << "Error creating publisher topic" << std::endl;
        return 1;
    }

    // CREATE THE WRITER
    // Set DataWriter Qos
    eprosima::fastdds::dds::DataWriterQos wqos;
    wqos.liveliness().lease_duration = 3;
    wqos.liveliness().announcement_period = 1;
    wqos.liveliness().kind = eprosima::fastdds::dds::AUTOMATIC_LIVELINESS_QOS;

    writer_ = publisher_->create_datawriter(topic_, wqos, &listener);

    if (writer_ == nullptr)
    {
        std::cerr << "Error creating publisher writer" << std::endl;
        return false;
    }

    std::cout << "DataWriter created with GUID " << writer_->guid() << std::endl;



    return true;
}

Publisher::~Publisher()
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
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void Publisher::PubListener::on_publication_matched(
        eprosima::fastdds::dds::DataWriter*,
        const eprosima::fastdds::dds::PublicationMatchedStatus& info)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (info.current_count_change == 1)
    {
        matched_ = info.current_count;
        std::cout << "Publisher matched." << std::endl;
        cv.notify_all();
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.current_count;
        std::cout << "Publisher unmatched." << std::endl;
        cv.notify_all();
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for PublicationMatchedStatus current count change" << std::endl;
    }
}

void Publisher::runThread(
        uint32_t samples,
        uint32_t sleep)
{
    while (!is_stopped() && (samples == 0 || comm_.index() < samples))
    {
        publish();
        std::cout << "Message: " << comm_.message() << " with index: " << comm_.index()
                  << " SENT" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }
}

void Publisher::run(
        uint32_t samples,
        uint32_t sleep)
{
    stop_ = false;
    std::thread thread(&Publisher::runThread, this, samples, sleep);
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
                static_cast<void>(signum); Publisher::stop();
            });

    thread.join();
    Publisher::stop();
}

void Publisher::publish()
{
    comm_.index(comm_.index() + 1);
    writer_->write(&comm_);
}

int main(
        int argc,
        char** argv)
{
    int arg_count = 1;
    bool exit_on_lost_liveliness = false;
    uint32_t seed = 7800, wait = 0;
    uint32_t samples = 10;
    std::string magic;
    Publisher pub;

    while (arg_count < argc)
    {
        if (strcmp(argv[arg_count], "--exit_on_lost_liveliness") == 0)
        {
            exit_on_lost_liveliness = true;
        }
        else if (strcmp(argv[arg_count], "--seed") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--seed expects a parameter" << std::endl;
                return -1;
            }

            seed = strtol(argv[arg_count], nullptr, 10);
        }
        else if (strcmp(argv[arg_count], "--wait") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--wait expects a parameter" << std::endl;
                return -1;
            }

            wait = strtol(argv[arg_count], nullptr, 10);
        }
        else if (strcmp(argv[arg_count], "--samples") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--samples expects a parameter" << std::endl;
                return -1;
            }

            samples = strtol(argv[arg_count], nullptr, 10);
        }
        else if (strcmp(argv[arg_count], "--magic") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--magic expects a parameter" << std::endl;
                return -1;
            }

            magic = argv[arg_count];
        }
        else if (strcmp(argv[arg_count], "--xmlfile") == 0)
        {
            std::cout << "--xmlfile option isn't implemented yet." << std::endl;
            if (++arg_count >= argc)
            {
                std::cout << "--xmlfile expects a parameter" << std::endl;
                return -1;
            }

            //xml_file = argv[arg_count];
        }

        ++arg_count;
    }

    /* TODO - XMLProfileManager doesn't support DDS yet
       if (xml_file)
       {
        DomainParticipantFactory::get_instance()->load_XML_profiles_file(xml_file);
       }
     */

    // Initialize the publisher
    pub.init(seed % 230);

    // Wait until all DataReaders are matched
    if (wait > 0)
    {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [&]
                {
                    return pub.listener.get_matched() >= wait;
                });
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    pub.run(samples, 10);

    // Stop the thread until publisher execution is finished
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [&]
            {
                return pub.is_stopped();
            });

    return 0;

}
