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

#include <condition_variable>
#include <fstream>
#include <mutex>
#include <string>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <nlohmann-json/json.hpp>

#include <exception/Exception.hpp>
#include <MessageSerializer.h>

#include <fastdds_statistics_backend/topic_types/monitorservice_types.hpp>
#include <fastdds_statistics_backend/topic_types/monitorservice_typesPubSubTypes.hpp>
#include <topic_types/types.hpp>
#include <topic_types/typesPubSubTypes.hpp>

#include <types/types.hpp>


namespace eprosima {
namespace statistics_backend {

using Traffic = nlohmann::ordered_json;

class TrafficInjector
{

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

    TrafficInjector(
            DomainId domain_id = 0)
    {
        participant_ = DomainParticipantFactory::get_instance()->create_participant(
            domain_id, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
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

            uint32_t kind;
            std::string topic_name;

            if (str == "HISTORY2HISTORY_LATENCY")
            {
                kind = fastdds::statistics::EventKind::HISTORY2HISTORY_LATENCY;
                topic_name = "_fastdds_statistics_history2history_latency";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::WriterReaderDataPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    std::shared_ptr<MessageSerializer> serializer =
                            std::make_shared<WriterReaderDataMessageSerializer>();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                }
            }
            else if (str == "NETWORK_LATENCY")
            {
                kind = fastdds::statistics::EventKind::NETWORK_LATENCY;
                topic_name = "_fastdds_statistics_network_latency";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::Locator2LocatorDataPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    std::shared_ptr<MessageSerializer> serializer =
                            std::make_shared<Locator2LocatorDataMessageSerializer>();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                }
            }
            else if (str == "PUBLICATION_THROUGHPUT")
            {
                kind = fastdds::statistics::EventKind::PUBLICATION_THROUGHPUT;
                topic_name = "_fastdds_statistics_publication_throughput";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityDataPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    std::shared_ptr<MessageSerializer> serializer = std::make_shared<EntityDataMessageSerializer>();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[fastdds::statistics::EventKind::SUBSCRIPTION_THROUGHPUT] = type;
                    serializers_[fastdds::statistics::EventKind::SUBSCRIPTION_THROUGHPUT] = serializer;
                }
            }
            else if (str == "SUBSCRIPTION_THROUGHPUT")
            {
                kind = fastdds::statistics::EventKind::SUBSCRIPTION_THROUGHPUT;
                topic_name = "_fastdds_statistics_subscription_throughput";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityDataPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    std::shared_ptr<MessageSerializer> serializer = std::make_shared<EntityDataMessageSerializer>();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[fastdds::statistics::EventKind::PUBLICATION_THROUGHPUT] = type;
                    serializers_[fastdds::statistics::EventKind::PUBLICATION_THROUGHPUT] = serializer;
                }
            }
            else if (str == "RTPS_SENT")
            {
                kind = fastdds::statistics::EventKind::RTPS_SENT;
                topic_name = "_fastdds_statistics_rtps_sent";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::Entity2LocatorTrafficPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    std::shared_ptr<MessageSerializer> serializer =
                            std::make_shared<Entity2LocatorTrafficMessageSerializer>();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[fastdds::statistics::EventKind::RTPS_LOST] = type;
                    serializers_[fastdds::statistics::EventKind::RTPS_LOST] = serializer;
                }
            }
            else if (str == "RTPS_LOST")
            {
                kind = fastdds::statistics::EventKind::RTPS_LOST;
                topic_name = "_fastdds_statistics_rtps_lost";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::Entity2LocatorTrafficPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    std::shared_ptr<MessageSerializer> serializer =
                            std::make_shared<Entity2LocatorTrafficMessageSerializer>();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[fastdds::statistics::EventKind::RTPS_SENT] = type;
                    serializers_[fastdds::statistics::EventKind::RTPS_SENT] = serializer;
                }
            }
            else if (str == "RESENT_DATAS")
            {
                kind = fastdds::statistics::EventKind::RESENT_DATAS;
                topic_name = "_fastdds_statistics_resent_datas";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    std::shared_ptr<MessageSerializer> serializer = std::make_shared<EntityCountMessageSerializer>();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[fastdds::statistics::EventKind::HEARTBEAT_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::HEARTBEAT_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::ACKNACK_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::ACKNACK_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::NACKFRAG_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::NACKFRAG_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::GAP_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::GAP_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::DATA_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::DATA_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::PDP_PACKETS] = type;
                    serializers_[fastdds::statistics::EventKind::PDP_PACKETS] = serializer;
                    data_types_[fastdds::statistics::EventKind::EDP_PACKETS] = type;
                    serializers_[fastdds::statistics::EventKind::EDP_PACKETS] = serializer;
                }
            }
            else if (str == "HEARTBEAT_COUNT")
            {
                kind = fastdds::statistics::EventKind::HEARTBEAT_COUNT;
                topic_name = "_fastdds_statistics_heartbeat_count";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    std::shared_ptr<MessageSerializer> serializer = std::make_shared<EntityCountMessageSerializer>();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[fastdds::statistics::EventKind::RESENT_DATAS] = type;
                    serializers_[fastdds::statistics::EventKind::RESENT_DATAS] = serializer;
                    data_types_[fastdds::statistics::EventKind::ACKNACK_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::ACKNACK_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::NACKFRAG_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::NACKFRAG_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::GAP_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::GAP_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::DATA_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::DATA_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::PDP_PACKETS] = type;
                    serializers_[fastdds::statistics::EventKind::PDP_PACKETS] = serializer;
                    data_types_[fastdds::statistics::EventKind::EDP_PACKETS] = type;
                    serializers_[fastdds::statistics::EventKind::EDP_PACKETS] = serializer;
                }
            }
            else if (str == "ACKNACK_COUNT")
            {
                kind = fastdds::statistics::EventKind::ACKNACK_COUNT;
                topic_name = "_fastdds_statistics_acknack_count";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    std::shared_ptr<MessageSerializer> serializer = std::make_shared<EntityCountMessageSerializer>();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[fastdds::statistics::EventKind::RESENT_DATAS] = type;
                    serializers_[fastdds::statistics::EventKind::RESENT_DATAS] = serializer;
                    data_types_[fastdds::statistics::EventKind::HEARTBEAT_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::HEARTBEAT_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::NACKFRAG_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::NACKFRAG_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::GAP_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::GAP_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::DATA_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::DATA_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::PDP_PACKETS] = type;
                    serializers_[fastdds::statistics::EventKind::PDP_PACKETS] = serializer;
                    data_types_[fastdds::statistics::EventKind::EDP_PACKETS] = type;
                    serializers_[fastdds::statistics::EventKind::EDP_PACKETS] = serializer;
                }
            }
            else if (str == "NACKFRAG_COUNT")
            {
                kind = fastdds::statistics::EventKind::NACKFRAG_COUNT;
                topic_name = "_fastdds_statistics_nackfrag_count";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    std::shared_ptr<MessageSerializer> serializer = std::make_shared<EntityCountMessageSerializer>();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[fastdds::statistics::EventKind::RESENT_DATAS] = type;
                    serializers_[fastdds::statistics::EventKind::RESENT_DATAS] = serializer;
                    data_types_[fastdds::statistics::EventKind::HEARTBEAT_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::HEARTBEAT_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::ACKNACK_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::ACKNACK_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::GAP_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::GAP_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::DATA_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::DATA_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::PDP_PACKETS] = type;
                    serializers_[fastdds::statistics::EventKind::PDP_PACKETS] = serializer;
                    data_types_[fastdds::statistics::EventKind::EDP_PACKETS] = type;
                    serializers_[fastdds::statistics::EventKind::EDP_PACKETS] = serializer;
                }
            }
            else if (str == "GAP_COUNT")
            {
                kind = fastdds::statistics::EventKind::GAP_COUNT;
                topic_name = "_fastdds_statistics_gap_count";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    std::shared_ptr<MessageSerializer> serializer = std::make_shared<EntityCountMessageSerializer>();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[fastdds::statistics::EventKind::RESENT_DATAS] = type;
                    serializers_[fastdds::statistics::EventKind::RESENT_DATAS] = serializer;
                    data_types_[fastdds::statistics::EventKind::HEARTBEAT_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::HEARTBEAT_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::ACKNACK_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::ACKNACK_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::NACKFRAG_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::NACKFRAG_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::DATA_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::DATA_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::PDP_PACKETS] = type;
                    serializers_[fastdds::statistics::EventKind::PDP_PACKETS] = serializer;
                    data_types_[fastdds::statistics::EventKind::EDP_PACKETS] = type;
                    serializers_[fastdds::statistics::EventKind::EDP_PACKETS] = serializer;
                }
            }
            else if (str == "DATA_COUNT")
            {
                kind = fastdds::statistics::EventKind::DATA_COUNT;
                topic_name = "_fastdds_statistics_data_count";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    std::shared_ptr<MessageSerializer> serializer = std::make_shared<EntityCountMessageSerializer>();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[fastdds::statistics::EventKind::RESENT_DATAS] = type;
                    serializers_[fastdds::statistics::EventKind::RESENT_DATAS] = serializer;
                    data_types_[fastdds::statistics::EventKind::HEARTBEAT_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::HEARTBEAT_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::ACKNACK_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::ACKNACK_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::NACKFRAG_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::NACKFRAG_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::GAP_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::GAP_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::PDP_PACKETS] = type;
                    serializers_[fastdds::statistics::EventKind::PDP_PACKETS] = serializer;
                    data_types_[fastdds::statistics::EventKind::EDP_PACKETS] = type;
                    serializers_[fastdds::statistics::EventKind::EDP_PACKETS] = serializer;
                }
            }
            else if (str == "PDP_PACKETS")
            {
                kind = fastdds::statistics::EventKind::PDP_PACKETS;
                topic_name = "_fastdds_statistics_pdp_packets";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    std::shared_ptr<MessageSerializer> serializer = std::make_shared<EntityCountMessageSerializer>();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[fastdds::statistics::EventKind::RESENT_DATAS] = type;
                    serializers_[fastdds::statistics::EventKind::RESENT_DATAS] = serializer;
                    data_types_[fastdds::statistics::EventKind::HEARTBEAT_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::HEARTBEAT_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::ACKNACK_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::ACKNACK_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::NACKFRAG_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::NACKFRAG_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::GAP_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::GAP_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::DATA_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::DATA_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::EDP_PACKETS] = type;
                    serializers_[fastdds::statistics::EventKind::EDP_PACKETS] = serializer;
                }
            }
            else if (str == "EDP_PACKETS")
            {
                kind = fastdds::statistics::EventKind::EDP_PACKETS;
                topic_name = "_fastdds_statistics_edp_packets";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::EntityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    std::shared_ptr<MessageSerializer> serializer = std::make_shared<EntityCountMessageSerializer>();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                    data_types_[fastdds::statistics::EventKind::RESENT_DATAS] = type;
                    serializers_[fastdds::statistics::EventKind::RESENT_DATAS] = serializer;
                    data_types_[fastdds::statistics::EventKind::HEARTBEAT_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::HEARTBEAT_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::ACKNACK_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::ACKNACK_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::NACKFRAG_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::NACKFRAG_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::GAP_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::GAP_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::DATA_COUNT] = type;
                    serializers_[fastdds::statistics::EventKind::DATA_COUNT] = serializer;
                    data_types_[fastdds::statistics::EventKind::PDP_PACKETS] = type;
                    serializers_[fastdds::statistics::EventKind::PDP_PACKETS] = serializer;
                }
            }
            else if (str == "DISCOVERED_ENTITY")
            {
                kind = fastdds::statistics::EventKind::DISCOVERED_ENTITY;
                topic_name = "_fastdds_statistics_discovered_entity";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::DiscoveryTimePubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    std::shared_ptr<MessageSerializer> serializer = std::make_shared<DiscoveryTimeMessageSerializer>();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                }
            }
            else if (str == "SAMPLE_DATAS")
            {
                kind = fastdds::statistics::EventKind::SAMPLE_DATAS;
                topic_name = "_fastdds_statistics_sample_datas";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::SampleIdentityCountPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    std::shared_ptr<MessageSerializer> serializer =
                            std::make_shared<SampleIdentityCountMessageSerializer>();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                }
            }
            else if (str == "PHYSICAL_DATA")
            {
                kind = fastdds::statistics::EventKind::PHYSICAL_DATA;
                topic_name = "_fastdds_statistics_physical_data";

                // The data type can be initialized from other event kind
                if (data_types_.find(kind) == data_types_.end())
                {
                    TypeSupport type(new eprosima::fastdds::statistics::PhysicalDataPubSubType());
                    type.register_type(participant_);
                    data_types_[kind] = type;

                    std::shared_ptr<MessageSerializer> serializer = std::make_shared<PhysicalDataMessageSerializer>();
                    serializers_[kind] = serializer;

                    // Add the type to all other event kinds that use it
                }
            }
            else
            {
                throw Unsupported("Unsupported event kind " + str);
            }

            event_kinds_[str] = kind;
            Topic* topic = participant_->create_topic(topic_name,
                            data_types_[kind]->get_name(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
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
        uint32_t kind = event_kinds_[msg.at("event_kind").get<std::string>()];
        StatisticsData data;

        std::cout << "Simulating statistics message [" << msg << "]" << std::endl;
        switch (kind)
        {
            case fastdds::statistics::EventKind::HISTORY2HISTORY_LATENCY:
                serializers_[kind]->deserialize(&data, msg.at("WriterReaderData"));
                data_writers_[kind]->write(&data.writer_reader_data());
                break;
            case fastdds::statistics::EventKind::NETWORK_LATENCY:
                serializers_[kind]->deserialize(&data, msg.at("locator2locator_data"));
                data_writers_[kind]->write(&data.locator2locator_data());
                break;
            case fastdds::statistics::EventKind::PUBLICATION_THROUGHPUT:
                serializers_[kind]->deserialize(&data, msg.at("entity_data"));
                data_writers_[kind]->write(&data.entity_data());
                break;
            case fastdds::statistics::EventKind::SUBSCRIPTION_THROUGHPUT:
                serializers_[kind]->deserialize(&data, msg.at("entity_data"));
                data_writers_[kind]->write(&data.entity_data());
                break;
            case fastdds::statistics::EventKind::RTPS_SENT:
                serializers_[kind]->deserialize(&data, msg.at("entity2locator_traffic"));
                data_writers_[kind]->write(&data.entity2locator_traffic());
                break;
            case fastdds::statistics::EventKind::RTPS_LOST:
                serializers_[kind]->deserialize(&data, msg.at("entity2locator_traffic"));
                data_writers_[kind]->write(&data.entity2locator_traffic());
                break;
            case fastdds::statistics::EventKind::RESENT_DATAS:
                serializers_[kind]->deserialize(&data, msg.at("entity_count"));
                data_writers_[kind]->write(&data.entity_count());
                break;
            case fastdds::statistics::EventKind::HEARTBEAT_COUNT:
                serializers_[kind]->deserialize(&data, msg.at("entity_count"));
                data_writers_[kind]->write(&data.entity_count());
                break;
            case fastdds::statistics::EventKind::ACKNACK_COUNT:
                serializers_[kind]->deserialize(&data, msg.at("entity_count"));
                data_writers_[kind]->write(&data.entity_count());
                break;
            case fastdds::statistics::EventKind::NACKFRAG_COUNT:
                serializers_[kind]->deserialize(&data, msg.at("entity_count"));
                data_writers_[kind]->write(&data.entity_count());
                break;
            case fastdds::statistics::EventKind::GAP_COUNT:
                serializers_[kind]->deserialize(&data, msg.at("entity_count"));
                data_writers_[kind]->write(&data.entity_count());
                break;
            case fastdds::statistics::EventKind::DATA_COUNT:
                serializers_[kind]->deserialize(&data, msg.at("entity_count"));
                data_writers_[kind]->write(&data.entity_count());
                break;
            case fastdds::statistics::EventKind::PDP_PACKETS:
                serializers_[kind]->deserialize(&data, msg.at("entity_count"));
                data_writers_[kind]->write(&data.entity_count());
                break;
            case fastdds::statistics::EventKind::EDP_PACKETS:
                serializers_[kind]->deserialize(&data, msg.at("entity_count"));
                data_writers_[kind]->write(&data.entity_count());
                break;
            case fastdds::statistics::EventKind::DISCOVERED_ENTITY:
                serializers_[kind]->deserialize(&data, msg.at("discovery_time"));
                data_writers_[kind]->write(&data.discovery_time());
                break;
            case fastdds::statistics::EventKind::SAMPLE_DATAS:
                serializers_[kind]->deserialize(&data, msg.at("sample_identity_count"));
                data_writers_[kind]->write(&data.sample_identity_count());
                break;
            case fastdds::statistics::EventKind::PHYSICAL_DATA:
                serializers_[kind]->deserialize(&data, msg.at("physical_data"));
                data_writers_[kind]->write(&data.physical_data());
                break;
        }
    }

    Traffic traffic_;
    Traffic::const_iterator next_message_;

    DomainParticipant* participant_;
    Publisher* publisher_;
    std::map<uint32_t, DataWriter*> data_writers_;
    std::map<uint32_t, Topic*> topics_;
    std::map<uint32_t, TypeSupport> data_types_;

    std::map<uint32_t, std::shared_ptr<MessageSerializer>> serializers_;
    std::map<uint32_t, std::string> event_names_;
    std::map<std::string, uint32_t> event_kinds_;

    std::string last_matched_topic_;
    std::mutex matching_mutex_;
    std::condition_variable matching_cv_;
};

} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TEST_TRAFFICINJECTOR_HPP_
