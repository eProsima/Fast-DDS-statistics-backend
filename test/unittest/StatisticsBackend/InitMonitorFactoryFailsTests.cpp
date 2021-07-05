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

#include <list>
#include <string>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantQos.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/qos/SubscriberQos.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/qos/TopicQos.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/statistics/topic_names.hpp>

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/listener/CallbackMask.hpp>
#include <fastdds_statistics_backend/listener/DomainListener.hpp>
#include <fastdds_statistics_backend/listener/PhysicalListener.hpp>
#include <fastdds_statistics_backend/StatisticsBackend.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/types.hpp>
#include <Monitor.hpp>
#include <topic_types/typesPubSubTypes.h>

using namespace eprosima::statistics_backend;
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::statistics;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::AnyNumber;
using ::testing::AtLeast;


class init_monitor_factory_fails_tests : public ::testing::Test
{

public:

    DomainParticipantFactory* domain_participant_factory_;
    DomainParticipantQos domain_participant_qos_;
    DomainParticipant domain_participant_;
    SubscriberQos subscriber_qos_;
    Subscriber subscriber_;
    TopicQos topic_qos_;
    std::map<const char*, Topic> topics_;
    std::map<const char*, TopicDescription> topic_descriptions_;
    DataReaderQos datareader_qos_;
    std::map<const char*, DataReader> data_readers_;

    // all bits in the callback mask
    static constexpr const CallbackKind all_callback_kinds_[] = {
        CallbackKind::ON_DATA_AVAILABLE,
        CallbackKind::ON_DATAREADER_DISCOVERY,
        CallbackKind::ON_DATAWRITER_DISCOVERY,
        CallbackKind::ON_HOST_DISCOVERY,
        CallbackKind::ON_LOCATOR_DISCOVERY,
        CallbackKind::ON_PARTICIPANT_DISCOVERY,
        CallbackKind::ON_PROCESS_DISCOVERY,
        CallbackKind::ON_TOPIC_DISCOVERY,
        CallbackKind::ON_USER_DISCOVERY };

    // a mask with all callbacks
    CallbackMask all_callback_mask_;

    // all bits in the datakind mask
    static constexpr const DataKind all_data_kinds_[] = {
        DataKind::ACKNACK_COUNT,
        DataKind::DATA_COUNT,
        DataKind::DISCOVERY_TIME,
        DataKind::EDP_PACKETS,
        DataKind::FASTDDS_LATENCY,
        DataKind::GAP_COUNT,
        DataKind::HEARTBEAT_COUNT,
        DataKind::NACKFRAG_COUNT,
        DataKind::NETWORK_LATENCY,
        DataKind::PDP_PACKETS,
        DataKind::PUBLICATION_THROUGHPUT,
        DataKind::RESENT_DATA,
        DataKind::RTPS_BYTES_LOST,
        DataKind::RTPS_BYTES_SENT,
        DataKind::RTPS_PACKETS_LOST,
        DataKind::RTPS_PACKETS_SENT,
        DataKind::SAMPLE_DATAS,
        DataKind::SUBSCRIPTION_THROUGHPUT };

    // a mask with all data kinds
    DataKindMask all_datakind_mask_;

    // Relation between topic names and data types
    std::map<const char*, TopicDataType*> topic_types_;

    init_monitor_factory_fails_tests()
    {
        // Fill the map of topics and types
        topic_types_[HISTORY_LATENCY_TOPIC] = new WriterReaderDataPubSubType();
        topic_types_[NETWORK_LATENCY_TOPIC] = new Locator2LocatorDataPubSubType();
        topic_types_[PUBLICATION_THROUGHPUT_TOPIC] = new EntityDataPubSubType();
        topic_types_[SUBSCRIPTION_THROUGHPUT_TOPIC] = new EntityDataPubSubType();
        topic_types_[RTPS_SENT_TOPIC] = new Entity2LocatorTrafficPubSubType();
        topic_types_[RTPS_LOST_TOPIC] = new Entity2LocatorTrafficPubSubType();
        topic_types_[RESENT_DATAS_TOPIC] = new EntityCountPubSubType();
        topic_types_[HEARTBEAT_COUNT_TOPIC] = new EntityCountPubSubType();
        topic_types_[ACKNACK_COUNT_TOPIC] = new EntityCountPubSubType();
        topic_types_[NACKFRAG_COUNT_TOPIC] = new EntityCountPubSubType();
        topic_types_[GAP_COUNT_TOPIC] = new EntityCountPubSubType();
        topic_types_[DATA_COUNT_TOPIC] = new EntityCountPubSubType();
        topic_types_[PDP_PACKETS_TOPIC] = new EntityCountPubSubType();
        topic_types_[EDP_PACKETS_TOPIC] = new EntityCountPubSubType();
        topic_types_[DISCOVERY_TOPIC] = new DiscoveryTimePubSubType();
        topic_types_[SAMPLE_DATAS_TOPIC] = new SampleIdentityCountPubSubType();
        topic_types_[PHYSICAL_DATA_TOPIC] = new PhysicalDataPubSubType();

        // Set the bits for the masks
        for (auto callback : init_monitor_factory_fails_tests::all_callback_kinds_)
        {
            all_callback_mask_.set(callback);
        }

        for (auto datakind : init_monitor_factory_fails_tests::all_data_kinds_)
        {
            all_datakind_mask_.set(datakind);
        }

        // The factories will by default return mocked instances
        domain_participant_factory_ = DomainParticipantFactory::get_instance();

        ON_CALL(*domain_participant_factory_, get_default_participant_qos())
                .WillByDefault(ReturnRef(domain_participant_qos_));

        ON_CALL(*domain_participant_factory_, create_participant(_, _, _, _))
                .WillByDefault(Return(&domain_participant_));

        ON_CALL(domain_participant_, get_default_subscriber_qos())
                .WillByDefault(ReturnRef(subscriber_qos_));

        ON_CALL(domain_participant_, get_default_topic_qos())
                .WillByDefault(ReturnRef(topic_qos_));

        ON_CALL(domain_participant_, create_subscriber(_, _, _))
                .WillByDefault(Return(&subscriber_));

        ON_CALL(subscriber_, get_default_datareader_qos())
                .WillByDefault(ReturnRef(datareader_qos_));

        ON_CALL(domain_participant_, register_type(_, _))
                .WillByDefault(Return(eprosima::fastrtps::types::ReturnCode_t::RETCODE_OK));

        for (auto topic_type : topic_types_)
        {
            topics_[topic_type.first] = Topic(topic_type.first, topic_type.second->getName());
            ON_CALL(domain_participant_, create_topic(topic_type.first, topic_type.second->getName(), _, _, _))
                    .WillByDefault(Return(&topics_[topic_type.first]));
            ON_CALL(domain_participant_, create_topic(topic_type.first, topic_type.second->getName(), _, _))
                    .WillByDefault(Return(&topics_[topic_type.first]));
            ON_CALL(domain_participant_, create_topic(topic_type.first, topic_type.second->getName(), _))
                    .WillByDefault(Return(&topics_[topic_type.first]));

            ON_CALL(domain_participant_, lookup_topicdescription(topic_type.first))
                    .WillByDefault(Return(nullptr));

            data_readers_[topic_type.first] = DataReader();
            ON_CALL(subscriber_, create_datareader(&topics_[topic_type.first], _, _, _))
                    .WillByDefault(Return(&data_readers_[topic_type.first]));
        }

        // We usually do not care about these calls
        // This can be overriden on specific tests if necessary
        EXPECT_CALL(*domain_participant_factory_, get_default_participant_qos()).Times(AnyNumber());
        EXPECT_CALL(domain_participant_, get_default_subscriber_qos()).Times(AnyNumber());
        EXPECT_CALL(domain_participant_, get_default_topic_qos()).Times(AnyNumber());
        EXPECT_CALL(subscriber_, get_default_datareader_qos()).Times(AnyNumber());

        // The default expectations
        EXPECT_CALL(*domain_participant_factory_, create_participant(_, _, _, _)).Times(AnyNumber());
        EXPECT_CALL(domain_participant_, create_subscriber(_, _, _)).Times(AnyNumber());
        EXPECT_CALL(domain_participant_, create_topic(_, _, _, _, _)).Times(AnyNumber());
        EXPECT_CALL(domain_participant_, create_topic(_, _, _, _)).Times(AnyNumber());
        EXPECT_CALL(domain_participant_, create_topic(_, _, _)).Times(AnyNumber());
        EXPECT_CALL(domain_participant_, lookup_topicdescription(_)).Times(AnyNumber());
        EXPECT_CALL(domain_participant_, register_type(_, _)).Times(AnyNumber());
        EXPECT_CALL(subscriber_, create_datareader(_, _, _, _)).Times(AnyNumber());
    }

    void check_init_monitor_failure()
    {
        DomainId domain_id = 0;
        DomainListener domain_listener;
        std::string server_guid_prefix = "44.53.01.5f.45.50.52.4f.53.49.4d.41";
        std::string server_locators = "UDPv4:[127.0.0.1]:11811";

        EXPECT_THROW(StatisticsBackend::init_monitor(
                        domain_id,
                        &domain_listener,
                        all_callback_mask_,
                        all_datakind_mask_), Error);
        EXPECT_THROW(StatisticsBackend::init_monitor(
                        server_locators,
                        &domain_listener,
                        all_callback_mask_,
                        all_datakind_mask_), Error);
        EXPECT_THROW(StatisticsBackend::init_monitor(
                        server_guid_prefix,
                        server_locators,
                        &domain_listener,
                        all_callback_mask_,
                        all_datakind_mask_), Error);
    }

    void check_init_monitor_discovery_server_failure(
            const std::string& server_locators)
    {
        DomainListener domain_listener;
        std::string server_guid_prefix = "44.53.01.5f.45.50.52.4f.53.49.4d.41";

        EXPECT_THROW(StatisticsBackend::init_monitor(
                server_locators,
                &domain_listener,
                all_callback_mask_,
                all_datakind_mask_), BadParameter);
        EXPECT_THROW(StatisticsBackend::init_monitor(
                server_guid_prefix,
                server_locators,
                &domain_listener,
                all_callback_mask_,
                all_datakind_mask_), BadParameter);
    }

    ~init_monitor_factory_fails_tests()
    {
        // Clear memory
        for (auto it : topic_types_)
        {
            delete it.second;
        }
    }

};

constexpr const CallbackKind init_monitor_factory_fails_tests::all_callback_kinds_[];
constexpr const DataKind init_monitor_factory_fails_tests::all_data_kinds_[];


TEST_F(init_monitor_factory_fails_tests, init_monitor_participant_creation_fails)
{
    // Expect failure on the participant creation
    EXPECT_CALL(*domain_participant_factory_, create_participant(_, _, _, _)).Times(3)
            .WillRepeatedly(Return(nullptr));

    check_init_monitor_failure();
}

TEST_F(init_monitor_factory_fails_tests, init_monitor_subscriber_creation_fails)
{
    // Expect failure on the subscriber creation
    EXPECT_CALL(domain_participant_, create_subscriber(_, _, _)).Times(3)
            .WillRepeatedly(Return(nullptr));

    check_init_monitor_failure();
}

TEST_F(init_monitor_factory_fails_tests, init_monitor_datareader_creation_fails)
{
    // Expect failure on the datareader creation
    EXPECT_CALL(subscriber_, create_datareader(_, _, _, _)).Times(3)
            .WillRepeatedly(Return(nullptr));

    check_init_monitor_failure();
}

TEST_F(init_monitor_factory_fails_tests, init_monitor_topic_creation_fails)
{
    // Expect failure on the topic creation
    // We need to cover all parameter cases to be implementation agnostic
    ON_CALL(domain_participant_, create_topic(_, _, _, _, _))
            .WillByDefault(Return(nullptr));
    ON_CALL(domain_participant_, create_topic(_, _, _, _))
            .WillByDefault(Return(nullptr));
    ON_CALL(domain_participant_, create_topic(_, _, _))
            .WillByDefault(Return(nullptr));

    check_init_monitor_failure();
}

TEST_F(init_monitor_factory_fails_tests, init_monitor_register_type_fails)
{
    // Expect failure on the type registration
    ON_CALL(domain_participant_, register_type(_, _))
            .WillByDefault(Return(eprosima::fastrtps::types::ReturnCode_t::RETCODE_PRECONDITION_NOT_MET));

    check_init_monitor_failure();
}

TEST_F(init_monitor_factory_fails_tests, init_monitor_topic_exists)
{
    DomainId domain_id = 0;
    DomainListener domain_listener;
    std::string server_guid_prefix = "44.53.01.5f.45.50.52.4f.53.49.4d.41";
    std::string server_locators = "UDPv4:[127.0.0.1]:11811";

    for (auto topic_type : topic_types_)
    {
        // The Topic already exists
        ON_CALL(domain_participant_, lookup_topicdescription(topic_type.first))
                .WillByDefault(Return(&topics_[topic_type.first]));
    }

    // The topics need no creation
    EXPECT_CALL(domain_participant_, create_topic(_, _, _, _, _)).Times(0);
    EXPECT_CALL(domain_participant_, create_topic(_, _, _, _)).Times(0);
    EXPECT_CALL(domain_participant_, create_topic(_, _, _)).Times(0);

    EXPECT_NO_THROW(StatisticsBackend::init_monitor(
                domain_id,
                &domain_listener,
                all_callback_mask_,
                all_datakind_mask_));
    EXPECT_NO_THROW(StatisticsBackend::init_monitor(
                server_locators,
                &domain_listener,
                all_callback_mask_,
                all_datakind_mask_));
    EXPECT_NO_THROW(StatisticsBackend::init_monitor(
                server_guid_prefix,
                server_locators,
                &domain_listener,
                all_callback_mask_,
                all_datakind_mask_));
}

TEST_F(init_monitor_factory_fails_tests, init_monitor_topic_exists_with_another_type)
{
    Topic topic("custom_topic", "custom_type");
    ON_CALL(domain_participant_, lookup_topicdescription(_))
            .WillByDefault(Return(&topic));

    check_init_monitor_failure();
}

TEST_F(init_monitor_factory_fails_tests, init_monitor_invalid_ipv4)
{
    std::string server_locators = "UDPv4:[192.356.0.1]:11811";

    check_init_monitor_discovery_server_failure(server_locators);
}

TEST_F(init_monitor_factory_fails_tests, init_monitor_invalid_ipv6)
{
    std::string server_locators = "UDPv6:[::0:G5a]:11811";

    check_init_monitor_discovery_server_failure(server_locators);
}

TEST_F(init_monitor_factory_fails_tests, init_monitor_invalid_port)
{
    std::string server_locators = "UDPv4:[192.356.0.1]:-11811";

    check_init_monitor_discovery_server_failure(server_locators);
}

TEST_F(init_monitor_factory_fails_tests, init_monitor_port_out_of_range)
{
    std::string server_locators = "UDPv4:[192.356.0.1]:4294967296";

    check_init_monitor_discovery_server_failure(server_locators);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
