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
 * @file TrafficInjector.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TEST_TRAFFICINJECTOR_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TEST_TRAFFICINJECTOR_HPP_

#include <MessageSerializer.h>
#include <exception/Exception.hpp>
#include <topic_types/types.h>
#include <topic_types/typesPubSubTypes.h>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include <nlohmann-json/json.hpp>

#include <string>
#include <fstream>

namespace eprosima {
namespace statistics_backend {

using Traffic = nlohmann::ordered_json;

class TrafficInjector
{

    using StatisticsEventKind = eprosima::fastdds::statistics::EventKind;
    using StatisticsData = eprosima::fastdds::statistics::Data;

    class StatisticsWriterListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:

        StatisticsWriterListener (
                TrafficInjector* injector)
            : injector_ (injector)
        {
        }

        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info)
        {
            if (info.current_count_change > 0)
            {
                injector_->matched(writer->get_topic()->get_name());
            }
        }

    private:

        TrafficInjector* injector_;
    };

public:

    TrafficInjector()
    {
        participant_ = DomainParticipantFactory::get_instance()->create_participant(
            0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
        publisher_ = participant_->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT);
    }

    ~TrafficInjector()
    {
        if (publisher_)
        {
            for (auto writer : data_writers_)
            {
                publisher_->delete_datawriter(writer.second);
            }

            for (auto topic : topics_)
            {
                participant_->delete_topic(topic.second);
            }

            participant_->delete_publisher(publisher_);
        }

        if (participant_)
        {
            DomainParticipantFactory::get_instance()->delete_participant(participant_);
        }
    }

    void load(
            std::string dump_file)
    {
        std::cout << "Loading " << dump_file << std::endl;

        std::ifstream ifs(dump_file);
        traffic_ = nlohmann::json::parse(ifs).at("traffic");

        initialize();

        next_message_ = traffic_.cbegin();
    }

    bool inject_next_message()
    {
        if (next_message_ == traffic_.cend())
        {
            std::cout << "Last message reached" << std::endl;
            return false;
        }

        send_message(*next_message_);
        ++next_message_;

        return true;
    }

    void matched(
            std::string topic_name)
    {
        std::unique_lock<std::mutex> lock(matching_mutex_);
        last_matched_topic_ = topic_name;
        matching_cv_.notify_one();
    }

private:

    using DomainParticipantFactory = eprosima::fastdds::dds::DomainParticipantFactory;
    using DomainParticipant = eprosima::fastdds::dds::DomainParticipant;
    using Publisher = eprosima::fastdds::dds::Publisher;
    using Topic = eprosima::fastdds::dds::Topic;
    using DataWriter = eprosima::fastdds::dds::DataWriter;
    using TopicDataType = eprosima::fastdds::dds::TopicDataType;
    using TypeSupport = eprosima::fastdds::dds::TypeSupport;

    void initialize()
    {
        for (auto it : traffic_)
        {
            std::string str = it.at("event_kind");
            if (event_kinds_.find(str) != event_kinds_.end())
            {
                // This kind of message is already initialized
                continue;
            }

            StatisticsEventKind kind;
            std::string topic_name;

            if (str == "HISTORY2HISTORY_LATENCY")
            {
                kind = StatisticsEventKind::HISTORY2HISTORY_LATENCY;
                topic_name = "_fastdds_statistics_history2history_latency";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::WriterReaderDataPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    MessageSerializer* serializer = new WriterReaderDataMessageSerializer();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                }
            }
            else if (str == "NETWORK_LATENCY")
            {
                kind = StatisticsEventKind::NETWORK_LATENCY;
                topic_name = "_fastdds_statistics_network_latency";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::Locator2LocatorDataPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    MessageSerializer* serializer = new Locator2LocatorDataMessageSerializer();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                }
            }
            else if (str == "PUBLICATION_THROUGHPUT")
            {
                kind = StatisticsEventKind::PUBLICATION_THROUGHPUT;
                topic_name = "_fastdds_statistics_publication_throughput";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityDataPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    MessageSerializer* serializer = new EntityDataMessageSerializer();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[StatisticsEventKind::SUBSCRIPTION_THROUGHPUT] = type;
                    serializers_[StatisticsEventKind::SUBSCRIPTION_THROUGHPUT] = serializer;
                }
            }
            else if (str == "SUBSCRIPTION_THROUGHPUT")
            {
                kind = StatisticsEventKind::SUBSCRIPTION_THROUGHPUT;
                topic_name = "_fastdds_statistics_subscription_throughput";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityDataPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    MessageSerializer* serializer = new EntityDataMessageSerializer();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[StatisticsEventKind::PUBLICATION_THROUGHPUT] = type;
                    serializers_[StatisticsEventKind::PUBLICATION_THROUGHPUT] = serializer;
                }
            }
            else if (str == "RTPS_SENT")
            {
                kind = StatisticsEventKind::RTPS_SENT;
                topic_name = "_fastdds_statistics_rtps_sent";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::Entity2LocatorTrafficPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    MessageSerializer* serializer = new Entity2LocatorTrafficMessageSerializer();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[StatisticsEventKind::RTPS_LOST] = type;
                    serializers_[StatisticsEventKind::RTPS_LOST] = serializer;
                }
            }
            else if (str == "RTPS_LOST")
            {
                kind = StatisticsEventKind::RTPS_LOST;
                topic_name = "_fastdds_statistics_rtps_lost";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::Entity2LocatorTrafficPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    MessageSerializer* serializer = new Entity2LocatorTrafficMessageSerializer();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[StatisticsEventKind::RTPS_SENT] = type;
                    serializers_[StatisticsEventKind::RTPS_SENT] = serializer;
                }
            }
            else if (str == "RESENT_DATAS")
            {
                kind = StatisticsEventKind::RESENT_DATAS;
                topic_name = "_fastdds_statistics_resent_datas";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    MessageSerializer* serializer = new EntityCountMessageSerializer();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[StatisticsEventKind::HEARTBEAT_COUNT] = type;
                    serializers_[StatisticsEventKind::HEARTBEAT_COUNT] = serializer;
                    data_types_[StatisticsEventKind::ACKNACK_COUNT] = type;
                    serializers_[StatisticsEventKind::ACKNACK_COUNT] = serializer;
                    data_types_[StatisticsEventKind::NACKFRAG_COUNT] = type;
                    serializers_[StatisticsEventKind::NACKFRAG_COUNT] = serializer;
                    data_types_[StatisticsEventKind::GAP_COUNT] = type;
                    serializers_[StatisticsEventKind::GAP_COUNT] = serializer;
                    data_types_[StatisticsEventKind::DATA_COUNT] = type;
                    serializers_[StatisticsEventKind::DATA_COUNT] = serializer;
                    data_types_[StatisticsEventKind::PDP_PACKETS] = type;
                    serializers_[StatisticsEventKind::PDP_PACKETS] = serializer;
                    data_types_[StatisticsEventKind::EDP_PACKETS] = type;
                    serializers_[StatisticsEventKind::EDP_PACKETS] = serializer;
                }
            }
            else if (str == "HEARTBEAT_COUNT")
            {
                kind = StatisticsEventKind::HEARTBEAT_COUNT;
                topic_name = "_fastdds_statistics_heartbeat_count";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    MessageSerializer* serializer = new EntityCountMessageSerializer();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[StatisticsEventKind::RESENT_DATAS] = type;
                    serializers_[StatisticsEventKind::RESENT_DATAS] = serializer;
                    data_types_[StatisticsEventKind::ACKNACK_COUNT] = type;
                    serializers_[StatisticsEventKind::ACKNACK_COUNT] = serializer;
                    data_types_[StatisticsEventKind::NACKFRAG_COUNT] = type;
                    serializers_[StatisticsEventKind::NACKFRAG_COUNT] = serializer;
                    data_types_[StatisticsEventKind::GAP_COUNT] = type;
                    serializers_[StatisticsEventKind::GAP_COUNT] = serializer;
                    data_types_[StatisticsEventKind::DATA_COUNT] = type;
                    serializers_[StatisticsEventKind::DATA_COUNT] = serializer;
                    data_types_[StatisticsEventKind::PDP_PACKETS] = type;
                    serializers_[StatisticsEventKind::PDP_PACKETS] = serializer;
                    data_types_[StatisticsEventKind::EDP_PACKETS] = type;
                    serializers_[StatisticsEventKind::EDP_PACKETS] = serializer;
                }
            }
            else if (str == "ACKNACK_COUNT")
            {
                kind = StatisticsEventKind::ACKNACK_COUNT;
                topic_name = "_fastdds_statistics_acknack_count";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    MessageSerializer* serializer = new EntityCountMessageSerializer();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[StatisticsEventKind::RESENT_DATAS] = type;
                    serializers_[StatisticsEventKind::RESENT_DATAS] = serializer;
                    data_types_[StatisticsEventKind::HEARTBEAT_COUNT] = type;
                    serializers_[StatisticsEventKind::HEARTBEAT_COUNT] = serializer;
                    data_types_[StatisticsEventKind::NACKFRAG_COUNT] = type;
                    serializers_[StatisticsEventKind::NACKFRAG_COUNT] = serializer;
                    data_types_[StatisticsEventKind::GAP_COUNT] = type;
                    serializers_[StatisticsEventKind::GAP_COUNT] = serializer;
                    data_types_[StatisticsEventKind::DATA_COUNT] = type;
                    serializers_[StatisticsEventKind::DATA_COUNT] = serializer;
                    data_types_[StatisticsEventKind::PDP_PACKETS] = type;
                    serializers_[StatisticsEventKind::PDP_PACKETS] = serializer;
                    data_types_[StatisticsEventKind::EDP_PACKETS] = type;
                    serializers_[StatisticsEventKind::EDP_PACKETS] = serializer;
                }
            }
            else if (str == "NACKFRAG_COUNT")
            {
                kind = StatisticsEventKind::NACKFRAG_COUNT;
                topic_name = "_fastdds_statistics_nackfrag_count";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    MessageSerializer* serializer = new EntityCountMessageSerializer();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[StatisticsEventKind::RESENT_DATAS] = type;
                    serializers_[StatisticsEventKind::RESENT_DATAS] = serializer;
                    data_types_[StatisticsEventKind::HEARTBEAT_COUNT] = type;
                    serializers_[StatisticsEventKind::HEARTBEAT_COUNT] = serializer;
                    data_types_[StatisticsEventKind::ACKNACK_COUNT] = type;
                    serializers_[StatisticsEventKind::ACKNACK_COUNT] = serializer;
                    data_types_[StatisticsEventKind::GAP_COUNT] = type;
                    serializers_[StatisticsEventKind::GAP_COUNT] = serializer;
                    data_types_[StatisticsEventKind::DATA_COUNT] = type;
                    serializers_[StatisticsEventKind::DATA_COUNT] = serializer;
                    data_types_[StatisticsEventKind::PDP_PACKETS] = type;
                    serializers_[StatisticsEventKind::PDP_PACKETS] = serializer;
                    data_types_[StatisticsEventKind::EDP_PACKETS] = type;
                    serializers_[StatisticsEventKind::EDP_PACKETS] = serializer;
                }
            }
            else if (str == "GAP_COUNT")
            {
                kind = StatisticsEventKind::GAP_COUNT;
                topic_name = "_fastdds_statistics_gap_count";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    MessageSerializer* serializer = new EntityCountMessageSerializer();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[StatisticsEventKind::RESENT_DATAS] = type;
                    serializers_[StatisticsEventKind::RESENT_DATAS] = serializer;
                    data_types_[StatisticsEventKind::HEARTBEAT_COUNT] = type;
                    serializers_[StatisticsEventKind::HEARTBEAT_COUNT] = serializer;
                    data_types_[StatisticsEventKind::ACKNACK_COUNT] = type;
                    serializers_[StatisticsEventKind::ACKNACK_COUNT] = serializer;
                    data_types_[StatisticsEventKind::NACKFRAG_COUNT] = type;
                    serializers_[StatisticsEventKind::NACKFRAG_COUNT] = serializer;
                    data_types_[StatisticsEventKind::DATA_COUNT] = type;
                    serializers_[StatisticsEventKind::DATA_COUNT] = serializer;
                    data_types_[StatisticsEventKind::PDP_PACKETS] = type;
                    serializers_[StatisticsEventKind::PDP_PACKETS] = serializer;
                    data_types_[StatisticsEventKind::EDP_PACKETS] = type;
                    serializers_[StatisticsEventKind::EDP_PACKETS] = serializer;
                }
            }
            else if (str == "DATA_COUNT")
            {
                kind = StatisticsEventKind::DATA_COUNT;
                topic_name = "_fastdds_statistics_data_count";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    MessageSerializer* serializer = new EntityCountMessageSerializer();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[StatisticsEventKind::RESENT_DATAS] = type;
                    serializers_[StatisticsEventKind::RESENT_DATAS] = serializer;
                    data_types_[StatisticsEventKind::HEARTBEAT_COUNT] = type;
                    serializers_[StatisticsEventKind::HEARTBEAT_COUNT] = serializer;
                    data_types_[StatisticsEventKind::ACKNACK_COUNT] = type;
                    serializers_[StatisticsEventKind::ACKNACK_COUNT] = serializer;
                    data_types_[StatisticsEventKind::NACKFRAG_COUNT] = type;
                    serializers_[StatisticsEventKind::NACKFRAG_COUNT] = serializer;
                    data_types_[StatisticsEventKind::GAP_COUNT] = type;
                    serializers_[StatisticsEventKind::GAP_COUNT] = serializer;
                    data_types_[StatisticsEventKind::PDP_PACKETS] = type;
                    serializers_[StatisticsEventKind::PDP_PACKETS] = serializer;
                    data_types_[StatisticsEventKind::EDP_PACKETS] = type;
                    serializers_[StatisticsEventKind::EDP_PACKETS] = serializer;
                }
            }
            else if (str == "PDP_PACKETS")
            {
                kind = StatisticsEventKind::PDP_PACKETS;
                topic_name = "_fastdds_statistics_pdp_packets";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    MessageSerializer* serializer = new EntityCountMessageSerializer();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[StatisticsEventKind::RESENT_DATAS] = type;
                    serializers_[StatisticsEventKind::RESENT_DATAS] = serializer;
                    data_types_[StatisticsEventKind::HEARTBEAT_COUNT] = type;
                    serializers_[StatisticsEventKind::HEARTBEAT_COUNT] = serializer;
                    data_types_[StatisticsEventKind::ACKNACK_COUNT] = type;
                    serializers_[StatisticsEventKind::ACKNACK_COUNT] = serializer;
                    data_types_[StatisticsEventKind::NACKFRAG_COUNT] = type;
                    serializers_[StatisticsEventKind::NACKFRAG_COUNT] = serializer;
                    data_types_[StatisticsEventKind::GAP_COUNT] = type;
                    serializers_[StatisticsEventKind::GAP_COUNT] = serializer;
                    data_types_[StatisticsEventKind::DATA_COUNT] = type;
                    serializers_[StatisticsEventKind::DATA_COUNT] = serializer;
                    data_types_[StatisticsEventKind::EDP_PACKETS] = type;
                    serializers_[StatisticsEventKind::EDP_PACKETS] = serializer;
                }
            }
            else if (str == "EDP_PACKETS")
            {
                kind = StatisticsEventKind::EDP_PACKETS;
                topic_name = "_fastdds_statistics_edp_packets";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    MessageSerializer* serializer = new EntityCountMessageSerializer();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[StatisticsEventKind::RESENT_DATAS] = type;
                    serializers_[StatisticsEventKind::RESENT_DATAS] = serializer;
                    data_types_[StatisticsEventKind::HEARTBEAT_COUNT] = type;
                    serializers_[StatisticsEventKind::HEARTBEAT_COUNT] = serializer;
                    data_types_[StatisticsEventKind::ACKNACK_COUNT] = type;
                    serializers_[StatisticsEventKind::ACKNACK_COUNT] = serializer;
                    data_types_[StatisticsEventKind::NACKFRAG_COUNT] = type;
                    serializers_[StatisticsEventKind::NACKFRAG_COUNT] = serializer;
                    data_types_[StatisticsEventKind::GAP_COUNT] = type;
                    serializers_[StatisticsEventKind::GAP_COUNT] = serializer;
                    data_types_[StatisticsEventKind::DATA_COUNT] = type;
                    serializers_[StatisticsEventKind::DATA_COUNT] = serializer;
                    data_types_[StatisticsEventKind::PDP_PACKETS] = type;
                    serializers_[StatisticsEventKind::PDP_PACKETS] = serializer;
                }
            }
            else if (str == "DISCOVERED_ENTITY")
            {
                kind = StatisticsEventKind::DISCOVERED_ENTITY;
                topic_name = "_fastdds_statistics_discovered_entity";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::DiscoveryTimePubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    MessageSerializer* serializer = new DiscoveryTimeMessageSerializer();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                }
            }
            else if (str == "SAMPLE_DATAS")
            {
                kind = StatisticsEventKind::SAMPLE_DATAS;
                topic_name = "_fastdds_statistics_sample_datas";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::SampleIdentityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    MessageSerializer* serializer = new SampleIdentityCountMessageSerializer();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                }
            }
            else if (str == "PHYSICAL_DATA")
            {
                kind = StatisticsEventKind::PHYSICAL_DATA;
                topic_name = "_fastdds_statistics_physical_data";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::PhysicalDataPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    MessageSerializer* serializer = new PhysicalDataMessageSerializer();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                }
            }
            else
            {
                throw Unsupported("Unsupported kind " + kind);
            }

            event_kinds_[str] = kind;
            Topic* topic = participant_->create_topic(topic_name,
                            data_types_[kind]->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
            if (!topic)
            {
                throw std::runtime_error("Error creating topic");
            }
            StatisticsWriterListener listener(this);
            DataWriter* writer = publisher_->create_datawriter(topic, eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT,
                            &listener);
            if (!writer)
            {
                throw std::runtime_error("Error creating writer");
            }
            topics_[kind] = topic;
            data_writers_[kind] = writer;

            //Wait until matched
            std::unique_lock<std::mutex> lock(matching_mutex_);
            std::cout << "Waiting to match on " << topic_name << std::endl;
            matching_cv_.wait(lock, [&]()
                    {
                        return last_matched_topic_ == topic_name;
                    });
            std::cout << "Matched on " << topic_name << std::endl;
        }
    }

    void send_message(
            const MessageSerializer::Message& msg)
    {
        StatisticsEventKind kind = event_kinds_[msg.at("event_kind").get<std::string>()];
        StatisticsData data;

        std::cout << "Simulating statistics message [" << msg << "]" << std::endl;
        switch (kind)
        {
            case StatisticsEventKind::HISTORY2HISTORY_LATENCY:
                serializers_[kind]->deserialize(&data, msg.at("WriterReaderData"));
                data_writers_[kind]->write(&data);
                break;
            case StatisticsEventKind::NETWORK_LATENCY:
                serializers_[kind]->deserialize(&data, msg.at("locator2locator_data"));
                data_writers_[kind]->write(&data);
                break;
            case StatisticsEventKind::PUBLICATION_THROUGHPUT:
                serializers_[kind]->deserialize(&data, msg.at("entity_data"));
                data_writers_[kind]->write(&data);
                break;
            case StatisticsEventKind::SUBSCRIPTION_THROUGHPUT:
                serializers_[kind]->deserialize(&data, msg.at("entity_data"));
                data_writers_[kind]->write(&data);
                break;
            case StatisticsEventKind::RTPS_SENT:
                serializers_[kind]->deserialize(&data, msg.at("entity2locator_traffic"));
                data_writers_[kind]->write(&data);
                break;
            case StatisticsEventKind::RTPS_LOST:
                serializers_[kind]->deserialize(&data, msg.at("entity2locator_traffic"));
                data_writers_[kind]->write(&data);
                break;
            case StatisticsEventKind::RESENT_DATAS:
                serializers_[kind]->deserialize(&data, msg.at("entity_count"));
                data_writers_[kind]->write(&data);
                break;
            case StatisticsEventKind::HEARTBEAT_COUNT:
                serializers_[kind]->deserialize(&data, msg.at("entity_count"));
                data_writers_[kind]->write(&data);
                break;
            case StatisticsEventKind::ACKNACK_COUNT:
                serializers_[kind]->deserialize(&data, msg.at("entity_count"));
                data_writers_[kind]->write(&data);
                break;
            case StatisticsEventKind::NACKFRAG_COUNT:
                serializers_[kind]->deserialize(&data, msg.at("entity_count"));
                data_writers_[kind]->write(&data);
                break;
            case StatisticsEventKind::GAP_COUNT:
                serializers_[kind]->deserialize(&data, msg.at("entity_count"));
                data_writers_[kind]->write(&data);
                break;
            case StatisticsEventKind::DATA_COUNT:
                serializers_[kind]->deserialize(&data, msg.at("entity_count"));
                data_writers_[kind]->write(&data);
                break;
            case StatisticsEventKind::PDP_PACKETS:
                serializers_[kind]->deserialize(&data, msg.at("entity_count"));
                data_writers_[kind]->write(&data);
                break;
            case StatisticsEventKind::EDP_PACKETS:
                serializers_[kind]->deserialize(&data, msg.at("entity_count"));
                data_writers_[kind]->write(&data);
                break;
            case StatisticsEventKind::DISCOVERED_ENTITY:
                serializers_[kind]->deserialize(&data, msg.at("discovery_time"));
                data_writers_[kind]->write(&data);
                break;
            case StatisticsEventKind::SAMPLE_DATAS:
                serializers_[kind]->deserialize(&data, msg.at("sample_identity_count"));
                data_writers_[kind]->write(&data);
                break;
            case StatisticsEventKind::PHYSICAL_DATA:
                serializers_[kind]->deserialize(&data, msg.at("physical_data"));
                data_writers_[kind]->write(&data);
                break;
        }
    }

    Traffic traffic_;
    Traffic::const_iterator next_message_;

    DomainParticipant* participant_;
    Publisher* publisher_;
    std::map<StatisticsEventKind, DataWriter*> data_writers_;
    std::map<StatisticsEventKind, Topic*> topics_;
    std::map<StatisticsEventKind, TypeSupport> data_types_;

    std::map<StatisticsEventKind, MessageSerializer*> serializers_;
    std::map<StatisticsEventKind, std::string> event_names_;
    std::map<std::string, StatisticsEventKind> event_kinds_;

    std::string last_matched_topic_;
    std::mutex matching_mutex_;
    std::condition_variable matching_cv_;
};

} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TEST_TRAFFICINJECTOR_HPP_
