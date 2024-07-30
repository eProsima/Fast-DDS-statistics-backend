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
 * @file Subscriber.cpp
 *
 */

#include <csignal>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>

#include "CommunicationPubSubTypes.hpp"
#include "Host.hpp"
#include "Subscriber.hpp"

using namespace eprosima;

std::atomic<bool> Subscriber::stop_(false);
std::mutex Subscriber::terminate_cv_mtx_;
std::condition_variable Subscriber::terminate_cv_;

class ParListener : public fastdds::dds::DomainParticipantListener
{
public:

    ParListener()
    {
    }

    virtual ~ParListener() override
    {
    }

    /**
     * This method is called when a new Participant is discovered, or a previously discovered participant changes its QOS or is removed.
     * @param p Pointer to the Participant
     * @param info DiscoveryInfo.
     */
    void on_participant_discovery(
            fastdds::dds::DomainParticipant* /*participant*/,
            fastdds::rtps::ParticipantDiscoveryStatus reason,
            const fastdds::dds::ParticipantBuiltinTopicData& info,
            bool&) override
    {
        if (reason == fastdds::rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT)
        {
            std::cout << "Subscriber participant " << //participant->getGuid() <<
                " discovered participant " << info.guid << std::endl;
        }
        else if (reason == fastdds::rtps::ParticipantDiscoveryStatus::CHANGED_QOS_PARTICIPANT)
        {
            std::cout << "Subscriber participant " << //participant->getGuid() <<
                " detected changes on participant " << info.guid << std::endl;
        }
        else if (reason == fastdds::rtps::ParticipantDiscoveryStatus::REMOVED_PARTICIPANT)
        {
            std::cout << "Subscriber participant " << //participant->getGuid() <<
                " removed participant " << info.guid << std::endl;
        }
        else if (reason == fastdds::rtps::ParticipantDiscoveryStatus::DROPPED_PARTICIPANT)
        {
            std::cout << "Subscriber participant " << //participant->getGuid() <<
                " dropped participant " << info.guid << std::endl;
        }
    }

}
participant_listener;

Subscriber::Subscriber()
    : participant_(nullptr)
    , subscriber_(nullptr)
    , topic_(nullptr)
    , reader_(nullptr)
    , type_(new CommunicationPubSubType())
{
}

bool Subscriber::is_stopped()
{
    return stop_;
}

void Subscriber::stop()
{
    stop_ = true;
    terminate_cv_.notify_all();
}

bool Subscriber::init(
        uint32_t max_messages,
        uint32_t domain)
{
    // Do not enable entities on creation
    eprosima::fastdds::dds::DomainParticipantFactoryQos factory_qos;
    factory_qos.entity_factory().autoenable_created_entities = false;
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->set_qos(factory_qos);
    eprosima::fastdds::dds::DomainParticipantQos pqos;
    pqos.name("Participant_sub");

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
    participant_ =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
        domain, pqos, &participant_listener, eprosima::fastdds::dds::StatusMask::none());

    if (participant_ == nullptr)
    {
        std::cout << "Error creating subscriber participant" << std::endl;
        return 1;
    }

    std::cout << "Participant " << pqos.name() << " created with GUID " << participant_->guid() << std::endl;

    // REGISTER THE TYPE
    type_.register_type(participant_);

    // CREATE THE SUBSCRIBER
    fastdds::dds::StatusMask mask = fastdds::dds::StatusMask::subscription_matched()
            << fastdds::dds::StatusMask::data_available()
            << fastdds::dds::StatusMask::liveliness_changed();

    subscriber_ = participant_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT, nullptr, mask);

    if (subscriber_ == nullptr)
    {
        std::cout << "Error creating subscriber" << std::endl;
        return 1;
    }

    // CREATE THE TOPIC

    // Generate topic name
    std::ostringstream topic;
    topic << "HelloWorldTopic_" << (eprosima::Host::instance().id()) << "_" << domain;

    topic_ = participant_->create_topic(
        topic.str(),
        type_.get_type_name(),
        eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);

    if (topic_ == nullptr)
    {
        std::cout << "Error creating subscriber topic" << std::endl;
        return 1;
    }

    // CREATE THE READER
    if (max_messages > 0)
    {
        listener_.set_max_messages(max_messages);
    }
    eprosima::fastdds::dds::DataReaderQos rqos = eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT;
    reader_ = subscriber_->create_datareader(topic_, rqos, &listener_);

    if (reader_ == nullptr)
    {
        std::cout << "Error creating subscriber DataReader" << std::endl;
        return 1;
    }

    std::cout << "DataReader created with GUID " << reader_->guid() << std::endl;

    // Now that everything is created we can enable the protocols
    participant_->enable();

    return true;
}

Subscriber::~Subscriber()
{
    if (participant_ != nullptr)
    {
        if (topic_ != nullptr)
        {
            participant_->delete_topic(topic_);
        }
        if (subscriber_ != nullptr)
        {
            if (reader_ != nullptr)
            {
                subscriber_->delete_datareader(reader_);
            }
            participant_->delete_subscriber(subscriber_);
        }
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }
}

void Subscriber::SubListener::set_max_messages(
        uint32_t max_messages)
{
    max_messages_ = max_messages;
}

void Subscriber::SubListener::on_subscription_matched(
        eprosima::fastdds::dds::DataReader*,
        const eprosima::fastdds::dds::SubscriptionMatchedStatus& info)
{
    if (info.current_count_change == 1)
    {
        matched_ = info.current_count;
        std::cout << "Subscriber matched." << std::endl;
    }
    else if (info.current_count_change == -1)
    {
        matched_ = info.current_count;
        std::cout << "Subscriber unmatched." << std::endl;
    }
    else
    {
        std::cout << info.current_count_change
                  << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
    }
}

void Subscriber::SubListener::on_data_available(
        eprosima::fastdds::dds::DataReader* reader)
{
    eprosima::fastdds::dds::SampleInfo info;
    while ((reader->take_next_sample(&comm_, &info) == eprosima::fastdds::dds::RETCODE_OK) && !is_stopped())
    {
        if (info.instance_state == eprosima::fastdds::dds::ALIVE_INSTANCE_STATE)
        {
            samples_++;
            // Print your structure data here.
            std::cout << "Message " << comm_.message() << " " << comm_.index() << " RECEIVED" << std::endl;
            if (max_messages_ > 0 && (samples_ >= max_messages_))
            {
                stop();
            }
        }
    }
}

void Subscriber::run(
        uint32_t samples)
{
    stop_ = false;
    if (samples > 0)
    {
        std::cout << "Subscriber running until " << samples <<
            " samples have been received. Please press CTRL+C to stop the Subscriber at any time." << std::endl;
    }
    else
    {
        std::cout << "Subscriber running. Please press CTRL+C to stop the Subscriber." << std::endl;
    }
    signal(SIGINT, [](int signum)
            {
                std::cout << "SIGINT received, stopping Subscriber execution." << std::endl;
                static_cast<void>(signum); Subscriber::stop();
            });
    std::unique_lock<std::mutex> lck(terminate_cv_mtx_);
    terminate_cv_.wait(lck, []
            {
                return is_stopped();
            });
}

int main(
        int argc,
        char** argv)
{
    int arg_count = 1;
    bool notexit = false;
    uint32_t seed = 7800;
    uint32_t samples = 10;
    //char* xml_file = nullptr;
    std::string magic;
    Subscriber sub;

    while (arg_count < argc)
    {
        if (strcmp(argv[arg_count], "--notexit") == 0)
        {
            notexit = true;
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

    // Initialize subscriber
    sub.init(samples, seed % 230);
    sub.run(samples);

    return 0;
}
