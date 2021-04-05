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

#include <TrafficInjector.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include "gtest/gtest.h"

#include <iostream>


using namespace eprosima::statistics_backend;

class StatisticsReaderListener : public eprosima::fastdds::dds::DataReaderListener
{
    void on_data_available(
            eprosima::fastdds::dds::DataReader* reader)
    {
        std::cout << "Received a sample in topic " << reader->get_topicdescription()->get_name() << std::endl;
    }
};

class TrafficInjectorTests : public ::testing::Test
{

public:
    using DomainParticipantFactory = eprosima::fastdds::dds::DomainParticipantFactory;
    using DomainParticipant = eprosima::fastdds::dds::DomainParticipant;
    using Subscriber = eprosima::fastdds::dds::Subscriber;
    using Topic = eprosima::fastdds::dds::Topic;
    using DataReader = eprosima::fastdds::dds::DataReader;
    using TopicDataType = eprosima::fastdds::dds::TopicDataType;
    using TypeSupport = eprosima::fastdds::dds::TypeSupport;

    DomainParticipant* participant_;
    Subscriber* subscriber_;
    std::set<Topic*> topics_;
    std::set<DataReader*> readers_;
    StatisticsReaderListener listener_;

    void SetUp ()
    {
        participant_ = DomainParticipantFactory::get_instance()->create_participant(
                0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
        subscriber_ = participant_->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT);

        {
            std::string topic_name = "_fastdds_statistics_history2history_latency";
            TypeSupport type(new eprosima::fastdds::statistics::WriterReaderDataPubSubType());
            type.register_type(participant_);

            Topic* topic = participant_->create_topic(topic_name, type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            topics_.insert(topic);
            readers_.insert(subscriber_->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, &listener_));
        }

        {
            std::string topic_name = "_fastdds_statistics_network_latency";
            TypeSupport type(new eprosima::fastdds::statistics::Locator2LocatorDataPubSubType());
            type.register_type(participant_);

            Topic* topic = participant_->create_topic(topic_name, type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            topics_.insert(topic);
            readers_.insert(subscriber_->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, &listener_));
        }

        {
            std::string topic_name = "_fastdds_statistics_publication_throughput";
            TypeSupport type(new eprosima::fastdds::statistics::EntityDataPubSubType());
            type.register_type(participant_);

            Topic* topic = participant_->create_topic(topic_name, type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            topics_.insert(topic);
            readers_.insert(subscriber_->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, &listener_));
        }

        {
            std::string topic_name = "_fastdds_statistics_subscription_throughput";
            TypeSupport type(new eprosima::fastdds::statistics::EntityDataPubSubType());
            type.register_type(participant_);

            Topic* topic = participant_->create_topic(topic_name, type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            topics_.insert(topic);
            readers_.insert(subscriber_->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, &listener_));
        }

        {
            std::string topic_name = "_fastdds_statistics_rtps_sent";
            TypeSupport type(new eprosima::fastdds::statistics::Entity2LocatorTrafficPubSubType());
            type.register_type(participant_);

            Topic* topic = participant_->create_topic(topic_name, type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            topics_.insert(topic);
            readers_.insert(subscriber_->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, &listener_));
        }

        {
            std::string topic_name = "_fastdds_statistics_rtps_lost";
            TypeSupport type(new eprosima::fastdds::statistics::Entity2LocatorTrafficPubSubType());
            type.register_type(participant_);

            Topic* topic = participant_->create_topic(topic_name, type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            topics_.insert(topic);
            readers_.insert(subscriber_->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, &listener_));
        }

        {
            std::string topic_name = "_fastdds_statistics_resent_datas";
            TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
            type.register_type(participant_);

            Topic* topic = participant_->create_topic(topic_name, type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            topics_.insert(topic);
            readers_.insert(subscriber_->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, &listener_));
        }

        {
            std::string topic_name = "_fastdds_statistics_heartbeat_count";
            TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
            type.register_type(participant_);

            Topic* topic = participant_->create_topic(topic_name, type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            topics_.insert(topic);
            readers_.insert(subscriber_->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, &listener_));
        }

        {
            std::string topic_name = "_fastdds_statistics_acknack_count";
            TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
            type.register_type(participant_);

            Topic* topic = participant_->create_topic(topic_name, type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            topics_.insert(topic);
            readers_.insert(subscriber_->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, &listener_));
        }

        {
            std::string topic_name = "_fastdds_statistics_nackfrag_count";
            TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
            type.register_type(participant_);

            Topic* topic = participant_->create_topic(topic_name, type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            topics_.insert(topic);
            readers_.insert(subscriber_->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, &listener_));
        }

        {
            std::string topic_name = "_fastdds_statistics_gap_count";
            TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
            type.register_type(participant_);

            Topic* topic = participant_->create_topic(topic_name, type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            topics_.insert(topic);
            readers_.insert(subscriber_->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, &listener_));
        }

        {
            std::string topic_name = "_fastdds_statistics_data_count";
            TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
            type.register_type(participant_);

            Topic* topic = participant_->create_topic(topic_name, type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            topics_.insert(topic);
            readers_.insert(subscriber_->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, &listener_));
        }
 
        {
            std::string topic_name = "_fastdds_statistics_pdp_packets";
            TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
            type.register_type(participant_);

            Topic* topic = participant_->create_topic(topic_name, type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            topics_.insert(topic);
            readers_.insert(subscriber_->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, &listener_));
        }

        {
            std::string topic_name = "_fastdds_statistics_edp_packets";
            TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
            type.register_type(participant_);

            Topic* topic = participant_->create_topic(topic_name, type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            topics_.insert(topic);
            readers_.insert(subscriber_->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, &listener_));
        }

        {
            std::string topic_name = "_fastdds_statistics_discovered_entity";
            TypeSupport type(new eprosima::fastdds::statistics::DiscoveryTimePubSubType());
            type.register_type(participant_);

            Topic* topic = participant_->create_topic(topic_name, type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            topics_.insert(topic);
            readers_.insert(subscriber_->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, &listener_));
        }

        {
            std::string topic_name = "_fastdds_statistics_sample_datas";
            TypeSupport type(new eprosima::fastdds::statistics::SampleIdentityCountPubSubType());
            type.register_type(participant_);

            Topic* topic = participant_->create_topic(topic_name, type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            topics_.insert(topic);
            readers_.insert(subscriber_->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, &listener_));
        }

        {
            std::string topic_name = "_fastdds_statistics_physical_data";
            TypeSupport type(new eprosima::fastdds::statistics::PhysicalDataPubSubType());
            type.register_type(participant_);

            Topic* topic = participant_->create_topic(topic_name, type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            topics_.insert(topic);
            readers_.insert(subscriber_->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT, &listener_));
        }
    }

    void TearDown()
    {
        for (auto reader : readers_)
        {
            subscriber_->delete_datareader(reader);
        }

        for (auto topic : topics_)
        {
            participant_->delete_topic(topic);
        }

        participant_->delete_subscriber(subscriber_);

        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

};

TEST_F(TrafficInjectorTests, Basic)
{
    std::string traffic_dump = "traffic_dump.json";
    TrafficInjector tf;
    tf.load(traffic_dump);


    while (tf.inject_next_message())
    {

    }
}


int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
