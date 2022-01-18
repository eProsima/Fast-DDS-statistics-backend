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

#include "fastdds/dds/domain/DomainParticipant.hpp"
#include "subscriber/QosSerializer.hpp"

#include <database/database.hpp>
#include <database/database_queue.hpp>
#include <subscriber/StatisticsParticipantListener.hpp>
#include <topic_types/types.h>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::Contains;
using ::testing::StrictMock;
using ::testing::Throw;

using namespace eprosima::fastdds::statistics;
using namespace eprosima::statistics_backend::database;
using namespace eprosima::statistics_backend::subscriber;

using EntityId = eprosima::statistics_backend::EntityId;
using EntityKind = eprosima::statistics_backend::EntityKind;

struct InsertEntityArgs
{
    InsertEntityArgs (
            std::function<EntityId(std::shared_ptr<Entity>)> func)
        : callback_(func)
    {
    }

    EntityId insert(
            std::shared_ptr<Entity> entity)
    {
        entity_ = entity;
        return callback_(entity);
    }

    std::function<EntityId(std::shared_ptr<Entity> entity)> callback_;
    std::shared_ptr<Entity> entity_;
};

template<typename T>
std::string to_string(
        T data)
{
    std::stringstream ss;
    ss << data;
    return ss.str();
}

class statistics_participant_listener_tests : public ::testing::Test
{

public:

    // Mocked database, to check insertions
    StrictMock<Database> database;

    // Entity queue, attached to the mocked database
    DatabaseEntityQueue entity_queue;

    // Data queue, attached to the mocked database
    DatabaseDataQueue data_queue;

    // Mocked statistics participant_, that is supposed to receive the callbacks
    eprosima::fastdds::dds::DomainParticipant statistics_participant;

    // Listener under tests. Will receive a pointer to statistics_participant
    StatisticsParticipantListener participant_listener;

    /*
     * Most of the tests require some existing entities to be created and available
     * in the database beforehand.
     * To avoid repeating so much code, the following entities will be available for every tests.
     * Each test will implement the expectations regarding these entities being available or not
     * in the database.
     */

    // Domain entity
    std::string domain_name_;
    std::shared_ptr<Domain> domain_;

    // Participant entity
    std::string participant_name_;
    Qos participant_qos_;
    std::string participant_prefix_str_;
    eprosima::fastrtps::rtps::GuidPrefix_t guid_prefix_;
    std::string participant_guid_str_;
    eprosima::fastrtps::rtps::GUID_t participant_guid_;
    std::shared_ptr<DomainParticipant> participant_;

    // Topic entity
    std::string topic_name_;
    std::string type_name_;
    std::shared_ptr<Topic> topic_;

    // Reader entity
    std::string reader_guid_str_;
    std::string reader_entity_id_str_;
    eprosima::fastrtps::rtps::GUID_t reader_guid_;

    // Writer entity
    std::string writer_guid_str_;
    std::string writer_entity_id_str_;
    eprosima::fastrtps::rtps::GUID_t writer_guid_;

    // Meaningful prefix for metatraffic entities
    const std::string metatraffic_prefix = "___EPROSIMA___METATRAFFIC___DOMAIN_0___";

    // Metatraffic Topic entity
    std::string metatraffic_topic_name_;
    std::string metatraffic_type_name_;
    std::shared_ptr<Topic> metatraffic_topic_;

    // Metatraffic endpoint entity
    std::string metatraffic_endpoint_name_;
    std::shared_ptr<DataWriter> metatraffic_endpoint_;
    Qos metatraffic_qos_;

    // Unicast locators
    std::shared_ptr<Locator> metatraffic_unicast_locator;
    std::shared_ptr<Locator> default_unicast_locator;
    std::shared_ptr<Locator> metatraffic_multicast_locator;
    std::shared_ptr<Locator> default_multicast_locator;

    statistics_participant_listener_tests()
        : entity_queue(&database)
        , data_queue(&database)
        , participant_listener(0, &database, &entity_queue, &data_queue)
    {
        //statistics_participant.domain_id_ = 0;

        // Domain entity
        domain_name_ = std::to_string(statistics_participant.domain_id_);
        domain_ = std::make_shared<Domain>(domain_name_);
        domain_->id = 0;

        // Participant entity
        participant_name_ = "participant_ name";
        participant_prefix_str_ = "01.02.03.04.05.06.07.08.09.0a.0b.0c";
        std::stringstream(participant_prefix_str_) >> guid_prefix_;
        participant_guid_str_ = participant_prefix_str_ + "|0.0.1.c1";
        std::stringstream(participant_guid_str_) >> participant_guid_;
        participant_ =
                std::make_shared<DomainParticipant>(participant_name_, participant_qos_, participant_guid_str_,
                        std::shared_ptr<Process>(), domain_);

        // Topic entity
        topic_name_ = "topic_";
        type_name_ = "type";
        topic_ = std::make_shared<Topic>(topic_name_, type_name_, domain_);

        // Reader entity
        reader_entity_id_str_ = "0.0.0.1";
        reader_guid_str_ = participant_prefix_str_ + "|" + reader_entity_id_str_;
        std::stringstream(reader_guid_str_) >> reader_guid_;

        // Writer entity
        writer_entity_id_str_ = "0.0.0.2";
        writer_guid_str_ = participant_prefix_str_ + "|" + writer_entity_id_str_;
        std::stringstream(writer_guid_str_) >> writer_guid_;

        // Metatraffic Topic entity
        metatraffic_topic_name_ = metatraffic_prefix + "TOPIC";
        metatraffic_type_name_ = metatraffic_prefix + "TYPE";
        metatraffic_topic_ = std::make_shared<Topic>(metatraffic_topic_name_, metatraffic_type_name_, domain_);

        // Metatraffic Endpoint entity
        metatraffic_endpoint_name_ = "DataWriter_" + metatraffic_prefix + "TOPIC_0.0.1.c1";
        metatraffic_qos_ = {{"description", "This is a virtual placeholder endpoint with no real counterpart"}};
        metatraffic_endpoint_ = std::make_shared<DataWriter>(metatraffic_endpoint_name_, metatraffic_qos_,
                        participant_guid_str_, participant_, metatraffic_topic_);

        // Locators
        metatraffic_unicast_locator = std::make_shared<Locator>("UDPv4:[0.0.0.0]:1");
        default_unicast_locator = std::make_shared<Locator>("UDPv4:[0.0.0.0]:3");

        metatraffic_multicast_locator = std::make_shared<Locator>("UDPv4:[0.0.0.0]:2");
        default_multicast_locator = std::make_shared<Locator>("UDPv4:[0.0.0.0]:4");
    }

    // The domain exists and the participant does not exist
    void new_participant_discovered_precondition()
    {
        // Precondition: The Domain 0 exists and has ID 0
        EXPECT_CALL(database,
                get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_)))
                .Times(
            AnyNumber())
                .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
                std::make_pair(EntityId(0), EntityId(0)))));
        EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber()).WillRepeatedly(Return(domain_));

        // Precondition: The Participant does not exist
        EXPECT_CALL(database,
                get_entity_by_guid(EntityKind::PARTICIPANT,
                participant_guid_str_)).Times(AnyNumber()).WillRepeatedly(Throw(eprosima::statistics_backend::
                        BadParameter(
                    "Error")));
    }

    // Build the participant discovered info with participant_guid_ and participant_name_
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo build_participant_discovered_info(
            eprosima::fastrtps::rtps::ParticipantProxyData& data)
    {
        // Precondition: The discovered participant has the given GUID and name
        data.m_guid = participant_guid_;
        data.m_participantName = participant_name_;

        // Finish building the discovered reader info
        return eprosima::fastrtps::rtps::ParticipantDiscoveryInfo(data);
    }

};

// Windows dll do not export ParticipantProxyData class members (private APIs)
#if !defined(_WIN32)
TEST_F(statistics_participant_listener_tests, new_participant_discovered)
{
    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_)))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_))
            .Times(AnyNumber())
            .WillOnce(Throw(eprosima::statistics_backend::BadParameter("Error")))
            .WillRepeatedly(Return(std::pair<EntityId, EntityId>(0, 10)));
    EXPECT_CALL(database, get_entity(EntityId(10))).Times(AnyNumber())
            .WillRepeatedly(Return(std::shared_ptr<const Entity>()));

    // Precondition: The Metatrafic Topic does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The Metatrafic Endpoint does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: There are unicast locators in database
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::LOCATOR, "UDPv4:[0.0.0.0]:1"))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));
    EXPECT_CALL(database,
            get_entity(EntityId(1))).Times(AnyNumber()).WillRepeatedly(Return(metatraffic_unicast_locator));

    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::LOCATOR, "UDPv4:[0.0.0.0]:3"))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber()).WillRepeatedly(Return(default_unicast_locator));

    // Precondition: There are not multicast locators in database
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::LOCATOR, "UDPv4:[0.0.0.0]:2"))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database,
            get_entity(EntityId(2))).Times(AnyNumber()).WillRepeatedly(Return(metatraffic_multicast_locator));

    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::LOCATOR, "UDPv4:[0.0.0.0]:4"))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(4)))));
    EXPECT_CALL(database,
            get_entity(EntityId(4))).Times(AnyNumber()).WillRepeatedly(Return(default_multicast_locator));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::RTPSParticipantAllocationAttributes allocation;
    eprosima::fastrtps::rtps::ParticipantProxyData data(allocation);

    // Precondition: The discovered participant has the given GUID and name
    data.m_guid = participant_guid_;
    data.m_participantName = participant_name_;

    // Precondition: The discovered participant has unicast and multicast locators associated
    data.metatraffic_locators.unicast.push_back(1);
    data.metatraffic_locators.multicast.push_back(2);
    data.default_locators.unicast.push_back(3);
    data.default_locators.multicast.push_back(4);

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo info(data);

    // Expectation: The Participant is added to the database. We do not care about the given ID
    InsertEntityArgs insert_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::PARTICIPANT);
                EXPECT_EQ(entity->name, participant_name_);
                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->guid, participant_guid_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->qos,
                participant_proxy_data_to_backend_qos(info.info));

                return EntityId(10);
            });

    // Expectation: The Metatraffic topic is added to the database. We do not care about the given ID
    InsertEntityArgs insert_topic_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::TOPIC);
                EXPECT_EQ(entity->name, metatraffic_prefix + "TOPIC");
                EXPECT_EQ(std::dynamic_pointer_cast<Topic>(entity)->data_type, metatraffic_prefix + "TYPE");

                return EntityId(11);
            });

    // Expectation: The Metatraffic Endpoint is added to the database. We do not care about the given ID
    InsertEntityArgs insert_datawriter_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::DATAWRITER);
                EXPECT_EQ(entity->name, "DataWriter_" + metatraffic_prefix + "TOPIC_0.0.1.c1");
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->qos, metatraffic_qos_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->guid, participant_guid_str_);

                return EntityId(12);
            });

    EXPECT_CALL(database, insert(_)).Times(3)
            .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_topic_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEntityArgs::insert));

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The metatraffic endpoint change it status
    EXPECT_CALL(database, change_entity_status(EntityId(12), true)).Times(1);

    // Expectation: The metatraffic endpoint is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(12), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The topic is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(11), EntityKind::TOPIC,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, std::move(info));
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_participant_discovered_not_first)
{
    // Participant_2 entity
    std::string participant_2_name = "participant_ name";
    Qos participant_2_qos;
    std::string participant_2_prefix_str = "01.02.03.04.05.06.07.08.09.0a.0b.1c";
    eprosima::fastrtps::rtps::GuidPrefix_t guid_2_prefix;
    std::stringstream(participant_2_prefix_str) >> guid_2_prefix;
    std::string participant_2_guid_str = participant_2_prefix_str + "|0.0.1.c1";
    eprosima::fastrtps::rtps::GUID_t participant_2_guid;
    std::stringstream(participant_2_guid_str) >> participant_2_guid;
    std::shared_ptr<DomainParticipant> participant_2 =
            std::make_shared<DomainParticipant>(participant_2_name, participant_2_qos, participant_2_guid_str,
                    std::shared_ptr<Process>(), domain_);

    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: Participant_5 exists and has ID 5
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(5))));
    EXPECT_CALL(database, get_entity(EntityId(5))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));
    EXPECT_CALL(database, get_entities(EntityKind::PARTICIPANT, EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, participant_)));

    // Precondition: The Metatrafic Topic exists and has ID 6
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(6)))));
    EXPECT_CALL(database, get_entity(EntityId(6))).Times(AnyNumber())
            .WillRepeatedly(Return(metatraffic_topic_));

    // Precondition: The Participant_1 Metatrafic Endpoint exists and has ID 7
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(7))));
    EXPECT_CALL(database, get_entity(EntityId(7))).Times(AnyNumber())
            .WillRepeatedly(Return(metatraffic_endpoint_));

    // Precondition: The new Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_2_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(eprosima::statistics_backend::BadParameter("Error")))
            .WillRepeatedly(Return(std::pair<EntityId, EntityId>(0, 10)));
    EXPECT_CALL(database, get_entity(EntityId(10))).Times(AnyNumber())
            .WillRepeatedly(Return(std::shared_ptr<const Entity>()));

    // Precondition: The Metatrafic Endpoint does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_2_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: There are unicast locators in database
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::LOCATOR, "UDPv4:[0.0.0.0]:1"))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));
    EXPECT_CALL(database,
            get_entity(EntityId(1))).Times(AnyNumber()).WillRepeatedly(Return(metatraffic_unicast_locator));

    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::LOCATOR, "UDPv4:[0.0.0.0]:3"))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber()).WillRepeatedly(Return(default_unicast_locator));

    // Precondition: There are not multicast locators in database
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::LOCATOR, "UDPv4:[0.0.0.0]:2"))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database,
            get_entity(EntityId(2))).Times(AnyNumber()).WillRepeatedly(Return(metatraffic_multicast_locator));

    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::LOCATOR, "UDPv4:[0.0.0.0]:4"))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(4)))));
    EXPECT_CALL(database,
            get_entity(EntityId(4))).Times(AnyNumber()).WillRepeatedly(Return(default_multicast_locator));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::RTPSParticipantAllocationAttributes allocation;
    eprosima::fastrtps::rtps::ParticipantProxyData data(allocation);

    // Precondition: The discovered participant has the given GUID and name
    data.m_guid = participant_2_guid;
    data.m_participantName = participant_2_name;

    // Precondition: The discovered participant has unicast and multicast locators associated
    data.metatraffic_locators.unicast.push_back(1);
    data.metatraffic_locators.multicast.push_back(2);
    data.default_locators.unicast.push_back(3);
    data.default_locators.multicast.push_back(4);

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo info(data);

    // Expectation: The Participant_2 is added to the database. We do not care about the given ID
    InsertEntityArgs insert_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::PARTICIPANT);
                EXPECT_EQ(entity->name, participant_2_name);
                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->guid, participant_2_guid_str);
                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->qos,
                participant_proxy_data_to_backend_qos(info.info));

                return EntityId(10);
            });

    // Expectation: The Metatraffic endpoint is added to the database. We do not care about the given ID
    InsertEntityArgs insert_datawriter_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::DATAWRITER);
                EXPECT_EQ(entity->name, "DataWriter_" + metatraffic_prefix + "TOPIC_0.0.1.c1");
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->qos, metatraffic_qos_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->guid, participant_2_guid_str);

                return EntityId(12);
            });

    EXPECT_CALL(database, insert(_)).Times(2)
            .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEntityArgs::insert));

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The metatraffic endpoint change it status
    EXPECT_CALL(database, change_entity_status(EntityId(12), true)).Times(1);

    // Expectation: The metatraffic endpoint is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(12), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, std::move(info));
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_participant_undiscovered)
{
    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::RTPSParticipantAllocationAttributes allocation;
    eprosima::fastrtps::rtps::ParticipantProxyData data(allocation);

    // Precondition: The discovered participant has the given GUID and name
    data.m_guid = participant_guid_;
    data.m_participantName = participant_name_;

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo info(data);

    // Expectation: The Participant is not added to the database.
    EXPECT_CALL(database, insert(_)).Times(0);

    // Precondition: The Participant does not change it status
    EXPECT_CALL(database, change_entity_status(_, _)).Times(0);

    // Execution: Call the listener
    info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, std::move(info));

    info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, std::move(info));
}

// Test that discovers a participant without locators associated and an empty name.
// Participant name will be localhost:participant_id
TEST_F(statistics_participant_listener_tests, new_participant_discovered_empty_name_no_locators)
{
    participant_name_ = "";

    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_)))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber()).WillRepeatedly(Return(domain_));

    // Precondition: The Participant does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_))
            .Times(AnyNumber())
            .WillOnce(Throw(eprosima::statistics_backend::BadParameter("Error")))
            .WillRepeatedly(Return(std::pair<EntityId, EntityId>(0, 10)));
    EXPECT_CALL(database, get_entity(EntityId(10))).Times(AnyNumber())
            .WillRepeatedly(Return(std::shared_ptr<const Entity>()));

    // Precondition: The Metatrafic Topic does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The Metatrafic Endpoint does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::RTPSParticipantAllocationAttributes allocation;
    eprosima::fastrtps::rtps::ParticipantProxyData data(allocation);

    // Precondition: The discovered participant has the given GUID and name
    data.m_guid = participant_guid_;
    data.m_participantName = participant_name_;

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo info(data);

    // Expectation: The Participant is added to the database. We do not care about the given ID
    InsertEntityArgs insert_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::PARTICIPANT);
                EXPECT_EQ(entity->name, "localhost:09.0a.0b.0c");
                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->guid, participant_guid_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->qos,
                participant_proxy_data_to_backend_qos(info.info));

                return EntityId(10);
            });

    // Expectation: The Metatraffic topic is added to the database. We do not care about the given ID
    InsertEntityArgs insert_topic_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::TOPIC);
                EXPECT_EQ(entity->name, metatraffic_prefix + "TOPIC");
                EXPECT_EQ(std::dynamic_pointer_cast<Topic>(entity)->data_type, metatraffic_prefix + "TYPE");

                return EntityId(11);
            });

    // Expectation: The Metatraffic endpoint is added to the database. We do not care about the given ID
    InsertEntityArgs insert_datawriter_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::DATAWRITER);
                EXPECT_EQ(entity->name, "DataWriter_" + metatraffic_prefix + "TOPIC_0.0.1.c1");
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->qos, metatraffic_qos_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->guid, participant_guid_str_);

                return EntityId(12);
            });

    EXPECT_CALL(database, insert(_)).Times(3)
            .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_topic_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEntityArgs::insert));

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The metatraffic endpoint change it status
    EXPECT_CALL(database, change_entity_status(EntityId(12), true)).Times(1);

    // Expectation: The metatraffic endpoint is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(12), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The topic is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(11), EntityKind::TOPIC,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, std::move(info));
    entity_queue.flush();
}

// Test that discovers a participant with a locator associated and an empty name.
// Participant name will be locator_IP:participant_id
TEST_F(statistics_participant_listener_tests, new_participant_discovered_empty_name_default_unicast_locator)
{
    participant_name_ = "";

    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_)))
            .Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber()).WillRepeatedly(Return(domain_));

    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_))
            .Times(AnyNumber())
            .WillOnce(Throw(eprosima::statistics_backend::BadParameter("Error")))
            .WillRepeatedly(Return(std::pair<EntityId, EntityId>(0, 10)));
    EXPECT_CALL(database, get_entity(EntityId(10))).Times(AnyNumber())
            .WillRepeatedly(Return(std::shared_ptr<const Entity>()));

    // Precondition: The Metatrafic Topic does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The Metatrafic Endpoint does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::RTPSParticipantAllocationAttributes allocation;
    eprosima::fastrtps::rtps::ParticipantProxyData data(allocation);

    // Precondition: The discovered participant has the given GUID and name
    data.m_guid = participant_guid_;
    data.m_participantName = participant_name_;

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo info(data);

    // Precondition: The Unicast Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 37;
    dds_existing_unicast_locator.address[13] = 11;
    dds_existing_unicast_locator.address[14] = 18;
    dds_existing_unicast_locator.address[15] = 30;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber()).WillRepeatedly(Return(
                std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(
                    EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber()).WillRepeatedly(Return(existing_unicast_locator));

    // Precondition: The discovered reader contains the locator
    data.default_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Precondition: The Unicast Metatraffic Locator exists and has ID 4
    eprosima::fastrtps::rtps::Locator_t dds_existing_metatraffic_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_metatraffic_unicast_locator.address[12] = 38;
    dds_existing_metatraffic_unicast_locator.address[13] = 12;
    dds_existing_metatraffic_unicast_locator.address[14] = 19;
    dds_existing_metatraffic_unicast_locator.address[15] = 31;
    std::string existing_metatraffic_unicast_locator_name = to_string(dds_existing_metatraffic_unicast_locator);
    std::shared_ptr<Locator> existing_metatraffic_unicast_locator =
            std::make_shared<Locator>(existing_metatraffic_unicast_locator_name);
    existing_metatraffic_unicast_locator->id = 4;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_metatraffic_unicast_locator_name)).Times(
        AnyNumber()).WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(4)))));
    EXPECT_CALL(database,
            get_entity(EntityId(4))).Times(AnyNumber()).WillRepeatedly(Return(existing_metatraffic_unicast_locator));

    // Precondition: The discovered reader contains the locator
    data.metatraffic_locators.add_unicast_locator(dds_existing_metatraffic_unicast_locator);

    // Precondition: The Multicast Locator exists and has ID 5
    eprosima::fastrtps::rtps::Locator_t dds_existing_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_multicast_locator.address[12] = 36;
    dds_existing_multicast_locator.address[13] = 10;
    dds_existing_multicast_locator.address[14] = 17;
    dds_existing_multicast_locator.address[15] = 29;
    std::string existing_multicast_locator_name = to_string(dds_existing_multicast_locator);
    std::shared_ptr<Locator> existing_multicast_locator =
            std::make_shared<Locator>(existing_multicast_locator_name);
    existing_multicast_locator->id = 5;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_multicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(5)))));
    EXPECT_CALL(database, get_entity(EntityId(5))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_multicast_locator));

    // Precondition: The discovered reader contains the locator
    data.default_locators.add_multicast_locator(dds_existing_multicast_locator);

    // Precondition: The Multicast Metatraffic Locator exists and has ID 6
    eprosima::fastrtps::rtps::Locator_t dds_existing_metatraffic_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_metatraffic_multicast_locator.address[12] = 47;
    dds_existing_metatraffic_multicast_locator.address[13] = 21;
    dds_existing_metatraffic_multicast_locator.address[14] = 28;
    dds_existing_metatraffic_multicast_locator.address[15] = 40;
    std::string existing_metatraffic_multicast_locator_name = to_string(dds_existing_metatraffic_multicast_locator);
    std::shared_ptr<Locator> existing_metatraffic_multicast_locator =
            std::make_shared<Locator>(existing_metatraffic_multicast_locator_name);
    existing_metatraffic_multicast_locator->id = 6;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_metatraffic_multicast_locator_name)).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(6)))));
    EXPECT_CALL(database, get_entity(EntityId(6))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_metatraffic_multicast_locator));

    // Precondition: The discovered reader contains the locator
    data.metatraffic_locators.add_multicast_locator(dds_existing_metatraffic_multicast_locator);

    // Precondition: There are 4 locators
    EXPECT_CALL(database,
            get_entities(EntityKind::LOCATOR,
            EntityId(1))).Times(AnyNumber()).WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>({
        existing_unicast_locator, existing_metatraffic_unicast_locator, existing_multicast_locator,
        existing_metatraffic_multicast_locator})));

    // Expectation: The Participant is added to the database. We do not care about the given ID
    InsertEntityArgs insert_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::PARTICIPANT);
                EXPECT_EQ(entity->name, "37.11.18.30:09.0a.0b.0c");
                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->guid, participant_guid_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->qos,
                participant_proxy_data_to_backend_qos(info.info));

                return EntityId(10);
            });

    // Expectation: The Metatraffic topic is added to the database. We do not care about the given ID
    InsertEntityArgs insert_topic_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::TOPIC);
                EXPECT_EQ(entity->name, metatraffic_prefix + "TOPIC");
                EXPECT_EQ(std::dynamic_pointer_cast<Topic>(entity)->data_type, metatraffic_prefix + "TYPE");

                return EntityId(11);
            });

    // Expectation: The Metatraffic endpoint is added to the database. We do not care about the given ID
    InsertEntityArgs insert_datawriter_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::DATAWRITER);
                EXPECT_EQ(entity->name, "DataWriter_" + metatraffic_prefix + "TOPIC_0.0.1.c1");
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->qos, metatraffic_qos_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->guid, participant_guid_str_);

                return EntityId(12);
            });

    EXPECT_CALL(database, insert(_)).Times(3)
            .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_topic_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEntityArgs::insert));

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The metatraffic endpoint change it status
    EXPECT_CALL(database, change_entity_status(EntityId(12), true)).Times(1);

    // Expectation: The metatraffic endpoint is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(12), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The topic is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(11), EntityKind::TOPIC,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, std::move(info));
    entity_queue.flush();
}

// Test that discovers a participant with a locator associated and an empty name.
// Participant name will be locator_IP:participant_id
TEST_F(statistics_participant_listener_tests, new_participant_discovered_empty_name_metatraffic_unicast_locator)
{
    participant_name_ = "";

    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_)))
            .Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber()).WillRepeatedly(Return(domain_));

    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_))
            .Times(AnyNumber())
            .WillOnce(Throw(eprosima::statistics_backend::BadParameter("Error")))
            .WillRepeatedly(Return(std::pair<EntityId, EntityId>(0, 10)));
    EXPECT_CALL(database, get_entity(EntityId(10))).Times(AnyNumber())
            .WillRepeatedly(Return(std::shared_ptr<const Entity>()));

    // Precondition: The Metatrafic Topic does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The Metatrafic Endpoint does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::RTPSParticipantAllocationAttributes allocation;
    eprosima::fastrtps::rtps::ParticipantProxyData data(allocation);

    // Precondition: The discovered participant has the given GUID and name
    data.m_guid = participant_guid_;
    data.m_participantName = participant_name_;

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo info(data);

    // Precondition: The Unicast Metatraffic Locator exists and has ID 4
    eprosima::fastrtps::rtps::Locator_t dds_existing_metatraffic_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_metatraffic_unicast_locator.address[12] = 37;
    dds_existing_metatraffic_unicast_locator.address[13] = 11;
    dds_existing_metatraffic_unicast_locator.address[14] = 18;
    dds_existing_metatraffic_unicast_locator.address[15] = 30;
    std::string existing_metatraffic_unicast_locator_name = to_string(dds_existing_metatraffic_unicast_locator);
    std::shared_ptr<Locator> existing_metatraffic_unicast_locator =
            std::make_shared<Locator>(existing_metatraffic_unicast_locator_name);
    existing_metatraffic_unicast_locator->id = 4;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_metatraffic_unicast_locator_name)).Times(
        AnyNumber()).WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(4)))));
    EXPECT_CALL(database,
            get_entity(EntityId(4))).Times(AnyNumber()).WillRepeatedly(Return(existing_metatraffic_unicast_locator));

    // Precondition: The discovered reader contains the locator
    data.metatraffic_locators.add_unicast_locator(dds_existing_metatraffic_unicast_locator);

    // Precondition: The Multicast Locator exists and has ID 5
    eprosima::fastrtps::rtps::Locator_t dds_existing_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_multicast_locator.address[12] = 36;
    dds_existing_multicast_locator.address[13] = 10;
    dds_existing_multicast_locator.address[14] = 17;
    dds_existing_multicast_locator.address[15] = 29;
    std::string existing_multicast_locator_name = to_string(dds_existing_multicast_locator);
    std::shared_ptr<Locator> existing_multicast_locator =
            std::make_shared<Locator>(existing_multicast_locator_name);
    existing_multicast_locator->id = 5;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_multicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(5)))));
    EXPECT_CALL(database, get_entity(EntityId(5))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_multicast_locator));

    // Precondition: The discovered reader contains the locator
    data.default_locators.add_multicast_locator(dds_existing_multicast_locator);

    // Precondition: The Multicast Metatraffic Locator exists and has ID 6
    eprosima::fastrtps::rtps::Locator_t dds_existing_metatraffic_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_metatraffic_multicast_locator.address[12] = 47;
    dds_existing_metatraffic_multicast_locator.address[13] = 21;
    dds_existing_metatraffic_multicast_locator.address[14] = 28;
    dds_existing_metatraffic_multicast_locator.address[15] = 40;
    std::string existing_metatraffic_multicast_locator_name = to_string(dds_existing_metatraffic_multicast_locator);
    std::shared_ptr<Locator> existing_metatraffic_multicast_locator =
            std::make_shared<Locator>(existing_metatraffic_multicast_locator_name);
    existing_metatraffic_multicast_locator->id = 6;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_metatraffic_multicast_locator_name)).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(6)))));
    EXPECT_CALL(database, get_entity(EntityId(6))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_metatraffic_multicast_locator));

    // Precondition: The discovered reader contains the locator
    data.metatraffic_locators.add_multicast_locator(dds_existing_metatraffic_multicast_locator);

    // Precondition: There are 3 locators
    EXPECT_CALL(database,
            get_entities(EntityKind::LOCATOR,
            EntityId(1))).Times(AnyNumber()).WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>({
        existing_metatraffic_unicast_locator, existing_multicast_locator,
        existing_metatraffic_multicast_locator})));


    // Expectation: The Participant is added to the database. We do not care about the given ID
    InsertEntityArgs insert_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::PARTICIPANT);
                EXPECT_EQ(entity->name, "37.11.18.30:09.0a.0b.0c");
                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->guid, participant_guid_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->qos,
                participant_proxy_data_to_backend_qos(info.info));

                return EntityId(10);
            });

    // Expectation: The Metatraffic topic is added to the database. We do not care about the given ID
    InsertEntityArgs insert_topic_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::TOPIC);
                EXPECT_EQ(entity->name, metatraffic_prefix + "TOPIC");
                EXPECT_EQ(std::dynamic_pointer_cast<Topic>(entity)->data_type, metatraffic_prefix + "TYPE");

                return EntityId(11);
            });

    // Expectation: The Metatraffic endpoint is added to the database. We do not care about the given ID
    InsertEntityArgs insert_datawriter_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::DATAWRITER);
                EXPECT_EQ(entity->name, "DataWriter_" + metatraffic_prefix + "TOPIC_0.0.1.c1");
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->qos, metatraffic_qos_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->guid, participant_guid_str_);

                return EntityId(12);
            });

    EXPECT_CALL(database, insert(_)).Times(3)
            .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_topic_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEntityArgs::insert));

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The metatraffic endpoint change it status
    EXPECT_CALL(database, change_entity_status(EntityId(12), true)).Times(1);

    // Expectation: The metatraffic endpoint is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(12), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The topic is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(11), EntityKind::TOPIC,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, std::move(info));
    entity_queue.flush();
}

// Test that discovers a participant with a locator associated and an empty name.
// Participant name will be locator_IP:participant_id
TEST_F(statistics_participant_listener_tests, new_participant_discovered_empty_name_default_multicast_locator)
{
    participant_name_ = "";

    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_)))
            .Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber()).WillRepeatedly(Return(domain_));

    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_))
            .Times(AnyNumber())
            .WillOnce(Throw(eprosima::statistics_backend::BadParameter("Error")))
            .WillRepeatedly(Return(std::pair<EntityId, EntityId>(0, 10)));
    EXPECT_CALL(database, get_entity(EntityId(10))).Times(AnyNumber())
            .WillRepeatedly(Return(std::shared_ptr<const Entity>()));

    // Precondition: The Metatrafic Topic does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The Metatrafic Endpoint does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::RTPSParticipantAllocationAttributes allocation;
    eprosima::fastrtps::rtps::ParticipantProxyData data(allocation);

    // Precondition: The discovered participant has the given GUID and name
    data.m_guid = participant_guid_;
    data.m_participantName = participant_name_;

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo info(data);

    // Precondition: The Multicast Locator exists and has ID 5
    eprosima::fastrtps::rtps::Locator_t dds_existing_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_multicast_locator.address[12] = 37;
    dds_existing_multicast_locator.address[13] = 11;
    dds_existing_multicast_locator.address[14] = 18;
    dds_existing_multicast_locator.address[15] = 30;
    std::string existing_multicast_locator_name = to_string(dds_existing_multicast_locator);
    std::shared_ptr<Locator> existing_multicast_locator =
            std::make_shared<Locator>(existing_multicast_locator_name);
    existing_multicast_locator->id = 5;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_multicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(5)))));
    EXPECT_CALL(database, get_entity(EntityId(5))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_multicast_locator));

    // Precondition: The discovered reader contains the locator
    data.default_locators.add_multicast_locator(dds_existing_multicast_locator);

    // Precondition: The Multicast Metatraffic Locator exists and has ID 6
    eprosima::fastrtps::rtps::Locator_t dds_existing_metatraffic_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_metatraffic_multicast_locator.address[12] = 47;
    dds_existing_metatraffic_multicast_locator.address[13] = 21;
    dds_existing_metatraffic_multicast_locator.address[14] = 28;
    dds_existing_metatraffic_multicast_locator.address[15] = 40;
    std::string existing_metatraffic_multicast_locator_name = to_string(dds_existing_metatraffic_multicast_locator);
    std::shared_ptr<Locator> existing_metatraffic_multicast_locator =
            std::make_shared<Locator>(existing_metatraffic_multicast_locator_name);
    existing_metatraffic_multicast_locator->id = 6;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_metatraffic_multicast_locator_name)).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(6)))));
    EXPECT_CALL(database, get_entity(EntityId(6))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_metatraffic_multicast_locator));

    // Precondition: The discovered reader contains the locator
    data.metatraffic_locators.add_multicast_locator(dds_existing_metatraffic_multicast_locator);

    // Precondition: There are 2 locators
    EXPECT_CALL(database,
            get_entities(EntityKind::LOCATOR,
            EntityId(1))).Times(AnyNumber()).WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>({
        existing_multicast_locator,
        existing_metatraffic_multicast_locator})));

    // Expectation: The Participant is added to the database. We do not care about the given ID
    InsertEntityArgs insert_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::PARTICIPANT);
                EXPECT_EQ(entity->name, "37.11.18.30:09.0a.0b.0c");
                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->guid, participant_guid_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->qos,
                participant_proxy_data_to_backend_qos(info.info));

                return EntityId(10);
            });


    // Expectation: The Metatraffic topic is added to the database. We do not care about the given ID
    InsertEntityArgs insert_topic_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::TOPIC);
                EXPECT_EQ(entity->name, metatraffic_prefix + "TOPIC");
                EXPECT_EQ(std::dynamic_pointer_cast<Topic>(entity)->data_type, metatraffic_prefix + "TYPE");

                return EntityId(11);
            });

    // Expectation: The Metatraffic endpoint is added to the database. We do not care about the given ID
    InsertEntityArgs insert_datawriter_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::DATAWRITER);
                EXPECT_EQ(entity->name, "DataWriter_" + metatraffic_prefix + "TOPIC_0.0.1.c1");
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->qos, metatraffic_qos_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->guid, participant_guid_str_);

                return EntityId(12);
            });

    EXPECT_CALL(database, insert(_)).Times(3)
            .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_topic_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEntityArgs::insert));

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The metatraffic endpoint change it status
    EXPECT_CALL(database, change_entity_status(EntityId(12), true)).Times(1);

    // Expectation: The metatraffic endpoint is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(12), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The topic is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(11), EntityKind::TOPIC,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, std::move(info));
    entity_queue.flush();
}

// Test that discovers a participant with a locator associated and an empty name.
// Participant name will be locator_IP:participant_id
TEST_F(statistics_participant_listener_tests, new_participant_discovered_empty_name_metatraffic_multicast_locator)
{
    participant_name_ = "";

    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_)))
            .Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber()).WillRepeatedly(Return(domain_));

    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_))
            .Times(AnyNumber())
            .WillOnce(Throw(eprosima::statistics_backend::BadParameter("Error")))
            .WillRepeatedly(Return(std::pair<EntityId, EntityId>(0, 10)));
    EXPECT_CALL(database, get_entity(EntityId(10))).Times(AnyNumber())
            .WillRepeatedly(Return(std::shared_ptr<const Entity>()));

    // Precondition: The Metatrafic Topic does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The Metatrafic Endpoint does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::RTPSParticipantAllocationAttributes allocation;
    eprosima::fastrtps::rtps::ParticipantProxyData data(allocation);

    // Precondition: The discovered participant has the given GUID and name
    data.m_guid = participant_guid_;
    data.m_participantName = participant_name_;

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo info(data);

    // Precondition: The Multicast Metatraffic Locator exists and has ID 6
    eprosima::fastrtps::rtps::Locator_t dds_existing_metatraffic_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_metatraffic_multicast_locator.address[12] = 37;
    dds_existing_metatraffic_multicast_locator.address[13] = 11;
    dds_existing_metatraffic_multicast_locator.address[14] = 18;
    dds_existing_metatraffic_multicast_locator.address[15] = 30;
    std::string existing_metatraffic_multicast_locator_name = to_string(dds_existing_metatraffic_multicast_locator);
    std::shared_ptr<Locator> existing_metatraffic_multicast_locator =
            std::make_shared<Locator>(existing_metatraffic_multicast_locator_name);
    existing_metatraffic_multicast_locator->id = 6;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_metatraffic_multicast_locator_name)).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(6)))));
    EXPECT_CALL(database, get_entity(EntityId(6))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_metatraffic_multicast_locator));

    // Precondition: The discovered reader contains the locator
    data.metatraffic_locators.add_multicast_locator(dds_existing_metatraffic_multicast_locator);

    // Precondition: There are 1 locator
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>({existing_metatraffic_multicast_locator})));

    // Expectation: The Participant is added to the database. We do not care about the given ID
    InsertEntityArgs insert_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::PARTICIPANT);
                EXPECT_EQ(entity->name, "37.11.18.30:09.0a.0b.0c");
                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->guid, participant_guid_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->qos,
                participant_proxy_data_to_backend_qos(info.info));

                return EntityId(10);
            });

    // Expectation: The Metatraffic topic is added to the database. We do not care about the given ID
    InsertEntityArgs insert_topic_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::TOPIC);
                EXPECT_EQ(entity->name, metatraffic_prefix + "TOPIC");
                EXPECT_EQ(std::dynamic_pointer_cast<Topic>(entity)->data_type, metatraffic_prefix + "TYPE");

                return EntityId(11);
            });

    // Expectation: The Metatraffic endpoint is added to the database. We do not care about the given ID
    InsertEntityArgs insert_datawriter_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::DATAWRITER);
                EXPECT_EQ(entity->name, "DataWriter_" + metatraffic_prefix + "TOPIC_0.0.1.c1");
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->qos, metatraffic_qos_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->guid, participant_guid_str_);

                return EntityId(12);
            });

    EXPECT_CALL(database, insert(_)).Times(3)
            .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_topic_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEntityArgs::insert));

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The metatraffic endpoint change it status
    EXPECT_CALL(database, change_entity_status(EntityId(12), true)).Times(1);

    // Expectation: The metatraffic endpoint is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(12), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The topic is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(11), EntityKind::TOPIC,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, std::move(info));
    entity_queue.flush();
}

// Test that discoverS a participant with localhost locators associated and an empty name.
// Participant name will be localhost:participant_id
TEST_F(statistics_participant_listener_tests, new_participant_discovered_empty_name_localhost_locators)
{
    participant_name_ = "";

    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_)))
            .Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber()).WillRepeatedly(Return(domain_));

    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_))
            .Times(AnyNumber())
            .WillOnce(Throw(eprosima::statistics_backend::BadParameter("Error")))
            .WillRepeatedly(Return(std::pair<EntityId, EntityId>(0, 10)));
    EXPECT_CALL(database, get_entity(EntityId(10))).Times(AnyNumber())
            .WillRepeatedly(Return(std::shared_ptr<const Entity>()));

    // Precondition: The Metatrafic Topic does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The Metatrafic Endpoint does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::RTPSParticipantAllocationAttributes allocation;
    eprosima::fastrtps::rtps::ParticipantProxyData data(allocation);

    // Precondition: The discovered participant has the given GUID and name
    data.m_guid = participant_guid_;
    data.m_participantName = participant_name_;

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo info(data);

    // Precondition: The Unicast Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber()).WillRepeatedly(Return(
                std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(
                    EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber()).WillRepeatedly(Return(existing_unicast_locator));

    // Precondition: The discovered reader contains the locator
    data.default_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Precondition: The Unicast Metatraffic Locator exists and has ID 4
    eprosima::fastrtps::rtps::Locator_t dds_existing_metatraffic_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_metatraffic_unicast_locator.address[12] = 127;
    dds_existing_metatraffic_unicast_locator.address[15] = 1;
    std::string existing_metatraffic_unicast_locator_name = to_string(dds_existing_metatraffic_unicast_locator);
    std::shared_ptr<Locator> existing_metatraffic_unicast_locator =
            std::make_shared<Locator>(existing_metatraffic_unicast_locator_name);
    existing_metatraffic_unicast_locator->id = 4;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_metatraffic_unicast_locator_name)).Times(
        AnyNumber()).WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(4)))));
    EXPECT_CALL(database,
            get_entity(EntityId(4))).Times(AnyNumber()).WillRepeatedly(Return(existing_metatraffic_unicast_locator));

    // Precondition: The discovered reader contains the locator
    data.metatraffic_locators.add_unicast_locator(dds_existing_metatraffic_unicast_locator);

    // Precondition: The Multicast Locator exists and has ID 5
    eprosima::fastrtps::rtps::Locator_t dds_existing_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_multicast_locator.address[12] = 127;
    dds_existing_multicast_locator.address[15] = 1;
    std::string existing_multicast_locator_name = to_string(dds_existing_multicast_locator);
    std::shared_ptr<Locator> existing_multicast_locator =
            std::make_shared<Locator>(existing_multicast_locator_name);
    existing_multicast_locator->id = 5;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_multicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(5)))));
    EXPECT_CALL(database, get_entity(EntityId(5))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_multicast_locator));

    // Precondition: The discovered reader contains the locator
    data.default_locators.add_multicast_locator(dds_existing_multicast_locator);

    // Precondition: The Multicast Metatraffic Locator exists and has ID 6
    eprosima::fastrtps::rtps::Locator_t dds_existing_metatraffic_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_metatraffic_multicast_locator.address[12] = 127;
    dds_existing_metatraffic_multicast_locator.address[15] = 1;
    std::string existing_metatraffic_multicast_locator_name = to_string(dds_existing_metatraffic_multicast_locator);
    std::shared_ptr<Locator> existing_metatraffic_multicast_locator =
            std::make_shared<Locator>(existing_metatraffic_multicast_locator_name);
    existing_metatraffic_multicast_locator->id = 6;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_metatraffic_multicast_locator_name)).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(6)))));
    EXPECT_CALL(database, get_entity(EntityId(6))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_metatraffic_multicast_locator));

    // Precondition: The discovered reader contains the locator
    data.metatraffic_locators.add_multicast_locator(dds_existing_metatraffic_multicast_locator);

    // Precondition: There are 4 locators
    EXPECT_CALL(database,
            get_entities(EntityKind::LOCATOR,
            EntityId(1))).Times(AnyNumber()).WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>({
        existing_unicast_locator, existing_metatraffic_unicast_locator, existing_multicast_locator,
        existing_metatraffic_multicast_locator})));

    // Expectation: The Participant is added to the database. We do not care about the given ID
    InsertEntityArgs insert_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::PARTICIPANT);
                EXPECT_EQ(entity->name, "localhost:09.0a.0b.0c");
                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->guid, participant_guid_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->qos,
                participant_proxy_data_to_backend_qos(info.info));

                return EntityId(10);
            });

    // Expectation: The Metatraffic topic is added to the database. We do not care about the given ID
    InsertEntityArgs insert_topic_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::TOPIC);
                EXPECT_EQ(entity->name, metatraffic_prefix + "TOPIC");
                EXPECT_EQ(std::dynamic_pointer_cast<Topic>(entity)->data_type, metatraffic_prefix + "TYPE");

                return EntityId(11);
            });

    // Expectation: The Metatraffic endpoint is added to the database. We do not care about the given ID
    InsertEntityArgs insert_datawriter_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::DATAWRITER);
                EXPECT_EQ(entity->name, "DataWriter_" + metatraffic_prefix + "TOPIC_0.0.1.c1");
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->qos, metatraffic_qos_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->guid, participant_guid_str_);

                return EntityId(12);
            });

    EXPECT_CALL(database, insert(_)).Times(3)
            .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_topic_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEntityArgs::insert));

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The metatraffic endpoint change it status
    EXPECT_CALL(database, change_entity_status(EntityId(12), true)).Times(1);

    // Expectation: The metatraffic endpoint is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(12), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The topic is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(11), EntityKind::TOPIC,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, std::move(info));
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_participant_no_domain)
{
    // Precondition: The Domain 0 does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_)))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The Metatrafic Endpoint does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::RTPSParticipantAllocationAttributes allocation;
    eprosima::fastrtps::rtps::ParticipantProxyData data(allocation);

    // Precondition: The discovered participant has the given GUID and name
    data.m_guid = participant_guid_;
    data.m_participantName = participant_name_;

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo info(data);

    // Expectation: No entity is added to the database
    EXPECT_CALL(database, insert(_)).Times(0);

    participant_listener.on_participant_discovery(&statistics_participant, std::move(info));
}

TEST_F(statistics_participant_listener_tests, new_participant_discovered_participant_already_exists)
{
    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant exists and has ID 5
    participant_->id = EntityId(5);
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(5))));
    EXPECT_CALL(database, get_entity(EntityId(5))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));
    EXPECT_CALL(database, get_entities(EntityKind::PARTICIPANT, EntityId(5))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(5, participant_)));

    // Precondition: The Metatrafic Topic exists and has ID 6
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(6)))));
    EXPECT_CALL(database, get_entity(EntityId(6))).Times(AnyNumber())
            .WillRepeatedly(Return(metatraffic_topic_));

    // Precondition: The Metatrafic Endpoint exists and has ID 7
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(7))));

    // Precondition: There are unicast locators in database
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::LOCATOR, "UDPv4:[0.0.0.0]:1"))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(1)))));
    EXPECT_CALL(database,
            get_entity(EntityId(1))).Times(AnyNumber()).WillRepeatedly(Return(metatraffic_unicast_locator));

    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::LOCATOR, "UDPv4:[0.0.0.0]:3"))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber()).WillRepeatedly(Return(default_unicast_locator));

    // Precondition: There are multicast locators in database
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::LOCATOR, "UDPv4:[0.0.0.0]:2"))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database,
            get_entity(EntityId(2))).Times(AnyNumber()).WillRepeatedly(Return(metatraffic_multicast_locator));

    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::LOCATOR, "UDPv4:[0.0.0.0]:4"))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(4)))));
    EXPECT_CALL(database, get_entity(EntityId(4))).Times(AnyNumber()).WillRepeatedly(Return(default_multicast_locator));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::RTPSParticipantAllocationAttributes allocation;
    eprosima::fastrtps::rtps::ParticipantProxyData data(allocation);

    // Precondition: The discovered participant has the given GUID and name
    data.m_guid = participant_guid_;
    data.m_participantName = participant_name_;

    // Precondition: The discovered participant has unicast and multicast locators associated
    data.metatraffic_locators.unicast.push_back(1);
    data.metatraffic_locators.multicast.push_back(2);
    data.default_locators.unicast.push_back(3);
    data.default_locators.multicast.push_back(4);

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo info(data);

    EXPECT_CALL(database, insert(_)).Times(0);

    // Expectation: The Participant status is set to active
    EXPECT_CALL(database, change_entity_status(EntityId(7), true)).Times(1);
    EXPECT_CALL(database, change_entity_status(EntityId(5), true)).Times(1);

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(5), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The metatraffic endpoint is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(7), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener.
    info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, std::move(info));
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_participant_undiscovered_participant_already_exists)
{
    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant exists and has ID 1
    participant_->id = EntityId(1);
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::RTPSParticipantAllocationAttributes allocation;
    eprosima::fastrtps::rtps::ParticipantProxyData data(allocation);

    // Precondition: The discovered participant has the given GUID and name
    data.m_guid = participant_guid_;
    data.m_participantName = participant_name_;

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo info(data);

    // Expectation: The Participant is not inserted in the database.
    EXPECT_CALL(database, insert(_)).Times(0);

    // Expectation: The Participant status is set to inactive
    EXPECT_CALL(database, change_entity_status(EntityId(1), false)).Times(2);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(1), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY)).Times(2);

    // Execution: Call the listener.
    info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::REMOVED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, std::move(info));

    info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, std::move(info));

    entity_queue.flush();
}

#endif // !defined(_WIN32)

TEST_F(statistics_participant_listener_tests, new_reader_discovered)
{
    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));

    // Precondition: The Participant is linked to a host with ID 50
    std::string host_name = "hostname";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);

    EXPECT_CALL(database, get_entities(EntityKind::HOST, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, host)));
    EXPECT_CALL(database, get_entity(EntityId(50))).Times(AnyNumber())
            .WillRepeatedly(Return(host));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic_));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, existing_unicast_locator)));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::ReaderProxyData data(1, 1);

    // Precondition: The discovered reader is in the participant
    data.guid(reader_guid_);

    // Precondition: The discovered reader is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The discovered reader contains the locator
    data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ReaderDiscoveryInfo info(data);

    // Precondition: The Reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The DataReader is added to the database. We do not care about the given ID
    InsertEntityArgs insert_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::DATAREADER);
                EXPECT_EQ(entity->name, std::string("DataReader_") + topic_->name + "_" + reader_entity_id_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->topic, topic_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->participant, participant_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->guid, reader_guid_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->qos,
                reader_proxy_data_to_backend_qos(info.info));

                std::vector<std::string> expected_locator_names;
                expected_locator_names.push_back(existing_unicast_locator_name);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->locators.size(),
                expected_locator_names.size());
                for (auto loc :  std::dynamic_pointer_cast<DataReader>(entity)->locators)
                {
                    EXPECT_THAT(expected_locator_names, Contains(loc.second->name));
                    EXPECT_EQ(loc.second->id, existing_unicast_locator->id);
                }

                return EntityId(10);
            });

    EXPECT_CALL(database, insert(_)).Times(1)
            .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert));

    // Precondition: The reader change it status
    EXPECT_CALL(database, change_entity_status(EntityId(10), true)).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::DATAREADER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    participant_listener.on_subscriber_discovery(&statistics_participant, std::move(info));
}

TEST_F(statistics_participant_listener_tests, new_reader_undiscovered)
{
    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));

    // Precondition: The Participant is linked to a host with ID 50
    std::string host_name = "hostname";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);

    EXPECT_CALL(database, get_entities(EntityKind::HOST, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, host)));
    EXPECT_CALL(database, get_entity(EntityId(50))).Times(AnyNumber())
            .WillRepeatedly(Return(host));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic_));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, existing_unicast_locator)));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::ReaderProxyData data(1, 1);

    // Precondition: The discovered reader is in the participant
    data.guid(reader_guid_);

    // Precondition: The discovered reader is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The discovered reader contains the locator
    data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ReaderDiscoveryInfo info(data);

    // Precondition: The Reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The Datareader is not added to the database.
    EXPECT_CALL(database, insert(_)).Times(0);

    // Precondition: The Datareader does not change it status
    EXPECT_CALL(database, change_entity_status(_, _)).Times(0);

    // Execution: Call the listener
    info.status = eprosima::fastrtps::rtps::ReaderDiscoveryInfo::REMOVED_READER;
    participant_listener.on_subscriber_discovery(&statistics_participant, std::move(info));
}

TEST_F(statistics_participant_listener_tests, new_reader_no_topic)
{
    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));

    // Precondition: The Participant is linked to a host with ID 50
    std::string host_name = "hostname";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);

    EXPECT_CALL(database, get_entities(EntityKind::HOST, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, host)));
    EXPECT_CALL(database, get_entity(EntityId(50))).Times(AnyNumber())
            .WillRepeatedly(Return(host));

    // Precondition: The Topic does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, existing_unicast_locator)));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::ReaderProxyData data(1, 1);

    // Precondition: The discovered reader is in the participant
    data.guid(reader_guid_);

    // Precondition: The discovered reader is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The discovered reader contains the locator
    data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ReaderDiscoveryInfo info(data);

    // Precondition: The Reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The Topic is added to the database. We do not care about the given ID
    InsertEntityArgs insert_topic_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::TOPIC);
                EXPECT_EQ(entity->name, topic_name_);
                EXPECT_EQ(std::dynamic_pointer_cast<Topic>(entity)->domain, domain_);
                EXPECT_EQ(std::dynamic_pointer_cast<Topic>(entity)->data_type, type_name_);

                return EntityId(10);
            });

    // Expectation: The DataReader is added to the database. We do not care about the given ID
    InsertEntityArgs insert_reader_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::DATAREADER);
                EXPECT_EQ(entity->name, std::string("DataReader_") + topic_->name + "_" + reader_entity_id_str_);

                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->participant, participant_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->guid, reader_guid_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->qos,
                reader_proxy_data_to_backend_qos(info.info));

                // We cannot check the Topic pointer as the queue will create a new one.
                // We check the topic_ data instead
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->topic->name, topic_name_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->topic->domain, domain_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->topic->data_type, type_name_);

                std::vector<std::string> expected_locator_names;
                expected_locator_names.push_back(existing_unicast_locator_name);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->locators.size(),
                expected_locator_names.size());
                for (auto loc :  std::dynamic_pointer_cast<DataReader>(entity)->locators)
                {
                    EXPECT_THAT(expected_locator_names, Contains(loc.second->name));
                    EXPECT_EQ(loc.second->id, existing_unicast_locator->id);
                }

                return EntityId(11);
            });

    EXPECT_CALL(database, insert(_)).Times(2)
            .WillOnce(Invoke(&insert_topic_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_reader_args, &InsertEntityArgs::insert));

    // Precondition: The reader change it status
    EXPECT_CALL(database, change_entity_status(EntityId(11), true)).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(11), EntityKind::DATAREADER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::TOPIC,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    participant_listener.on_subscriber_discovery(&statistics_participant, std::move(info));
}

TEST_F(statistics_participant_listener_tests, new_reader_several_topics)
{
    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));

    // Precondition: The Participant is linked to a host with ID 50
    std::string host_name = "hostname";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);

    EXPECT_CALL(database, get_entities(EntityKind::HOST, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, host)));
    EXPECT_CALL(database, get_entity(EntityId(50))).Times(AnyNumber())
            .WillRepeatedly(Return(host));

    // Precondition: Another domain with ID 100 exists
    std::string another_domain_name = "another_domain";
    std::shared_ptr<Domain> another_domain = std::make_shared<Domain>(another_domain_name);
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, another_domain_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(100), EntityId(100)))));
    EXPECT_CALL(database, get_entity(EntityId(100))).Times(AnyNumber())
            .WillRepeatedly(Return(another_domain));

    // Precondition: Another topic with the same name and type exists in domain 100
    // and has ID 101
    std::shared_ptr<Topic> topic_another_domain = std::make_shared<Topic>(topic_name_, type_name_, another_domain);
    EXPECT_CALL(database, get_entity(EntityId(101))).Times(AnyNumber())
            .WillRepeatedly(Return(topic_another_domain));

    // Precondition: Another topic with the same name but different type exists in the initial domain 0
    // and has ID 102
    std::string another_type_name = "another_type";
    std::shared_ptr<Topic> topic_another_type = std::make_shared<Topic>(topic_name_, another_type_name, domain_);
    EXPECT_CALL(database, get_entity(EntityId(102))).Times(AnyNumber())
            .WillRepeatedly(Return(topic_another_type));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic_));

    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>
            { { EntityId(100), EntityId(101) },
                { EntityId(0), EntityId(102) },
                { EntityId(0), EntityId(2) }}));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, existing_unicast_locator)));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::ReaderProxyData data(1, 1);

    // Precondition: The discovered reader is in the participant
    data.guid(reader_guid_);

    // Precondition: The discovered reader is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The discovered reader contains the locator
    data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ReaderDiscoveryInfo info(data);

    // Precondition: The Reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The DataReader is added to the database. We do not care about the given ID
    InsertEntityArgs insert_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::DATAREADER);
                EXPECT_EQ(entity->name, std::string("DataReader_") + topic_->name + "_" + reader_entity_id_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->topic, topic_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->participant, participant_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->guid, reader_guid_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->qos,
                reader_proxy_data_to_backend_qos(info.info));

                std::vector<std::string> expected_locator_names;
                expected_locator_names.push_back(existing_unicast_locator_name);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->locators.size(),
                expected_locator_names.size());
                for (auto loc :  std::dynamic_pointer_cast<DataReader>(entity)->locators)
                {
                    EXPECT_THAT(expected_locator_names, Contains(loc.second->name));
                    EXPECT_EQ(loc.second->id, existing_unicast_locator->id);
                }

                return EntityId(10);
            });

    EXPECT_CALL(database, insert(_)).Times(1)
            .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert));

    // Precondition: The reader change it status
    EXPECT_CALL(database, change_entity_status(EntityId(10), true)).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::DATAREADER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    participant_listener.on_subscriber_discovery(&statistics_participant, std::move(info));
}

TEST_F(statistics_participant_listener_tests, new_reader_several_locators)
{
    std::vector<std::shared_ptr<const Entity>> existing_locators;
    int64_t next_entity_id = 100;

    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));

    // Precondition: The Participant is linked to a host with ID 50
    std::string host_name = "hostname";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);

    EXPECT_CALL(database, get_entities(EntityKind::HOST, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, host)));
    EXPECT_CALL(database, get_entity(EntityId(50))).Times(AnyNumber())
            .WillRepeatedly(Return(host));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic_));

    // Precondition: One unicast Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    existing_locators.push_back(existing_unicast_locator);

    // Precondition: One multicast Locator exists and has ID 4
    eprosima::fastrtps::rtps::Locator_t dds_existing_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_multicast_locator.address[12] = 127;
    dds_existing_multicast_locator.address[15] = 2;
    std::string existing_multicast_locator_name = to_string(dds_existing_multicast_locator);
    std::shared_ptr<Locator> existing_multicast_locator =
            std::make_shared<Locator>(existing_multicast_locator_name);
    existing_multicast_locator->id = 4;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_multicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(4)))));
    EXPECT_CALL(database, get_entity(EntityId(4))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_multicast_locator));
    existing_locators.push_back(existing_multicast_locator);

    // Precondition: One unicast Locator does not exist
    eprosima::fastrtps::rtps::Locator_t dds_new_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_new_unicast_locator.address[12] = 127;
    dds_new_unicast_locator.address[15] = 3;
    std::string new_unicast_locator_name = to_string(dds_new_unicast_locator);
    std::shared_ptr<Locator> new_unicast_locator =
            std::make_shared<Locator>(new_unicast_locator_name);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, new_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: One multicast Locator does not exist
    eprosima::fastrtps::rtps::Locator_t dds_new_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_new_multicast_locator.address[12] = 127;
    dds_new_multicast_locator.address[15] = 4;
    std::string new_multicast_locator_name = to_string(dds_new_multicast_locator);
    std::shared_ptr<Locator> new_multicast_locator =
            std::make_shared<Locator>(new_multicast_locator_name);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, new_multicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: Looking for the entities from the locator returns existing locators
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_locators));

    // Precondition: The database returns EntityIDs that are not used before
    database.set_next_entity_id(next_entity_id);

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::ReaderProxyData data(2, 2);

    // Precondition: The discovered reader is in the participant
    data.guid(reader_guid_);

    // Precondition: The discovered reader is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The discovered reader contains the locators
    data.add_unicast_locator(dds_existing_unicast_locator);
    data.add_unicast_locator(dds_new_unicast_locator);
    data.add_multicast_locator(dds_existing_multicast_locator);
    data.add_multicast_locator(dds_new_multicast_locator);

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ReaderDiscoveryInfo info(data);

    // Precondition: The Reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The new unicast locator is added to the database and given ID 100
    InsertEntityArgs insert_unicast_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::LOCATOR);
                EXPECT_EQ(entity->name, new_unicast_locator_name);

                return EntityId(100);
            });

    // Expectation: The new multicast locator is added to the database and given ID 101
    InsertEntityArgs insert_multicast_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::LOCATOR);
                EXPECT_EQ(entity->name, new_multicast_locator_name);

                return EntityId(101);
            });

    // Expectation: The DataWriter is added to the database and given ID 11
    InsertEntityArgs insert_reader_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::DATAREADER);
                EXPECT_EQ(entity->name, std::string("DataReader_") + topic_->name + "_" + reader_entity_id_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->topic, topic_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->participant, participant_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->guid, reader_guid_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->qos,
                reader_proxy_data_to_backend_qos(info.info));

                std::vector<std::string> expected_locator_names;
                expected_locator_names.push_back(existing_unicast_locator_name);
                expected_locator_names.push_back(existing_multicast_locator_name);
                expected_locator_names.push_back(new_unicast_locator_name);
                expected_locator_names.push_back(new_multicast_locator_name);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->locators.size(),
                expected_locator_names.size());
                for (auto loc :  std::dynamic_pointer_cast<DataReader>(entity)->locators)
                {
                    EXPECT_THAT(expected_locator_names, Contains(loc.second->name));
                    if (loc.second->name == existing_unicast_locator->name)
                    {
                        EXPECT_EQ(loc.second->id, existing_unicast_locator->id);
                    }
                    if (loc.second->name == existing_multicast_locator->name)
                    {
                        EXPECT_EQ(loc.second->id, existing_multicast_locator->id);
                    }
                    if (loc.second->name == new_unicast_locator->name)
                    {
                        EXPECT_GE(loc.second->id, next_entity_id);
                    }
                    if (loc.second->name == new_multicast_locator->name)
                    {
                        EXPECT_GE(loc.second->id, next_entity_id);
                    }
                }

                return EntityId(11);
            });

    EXPECT_CALL(database, insert(_)).Times(3)
            .WillOnce(Invoke(&insert_unicast_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_multicast_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_reader_args, &InsertEntityArgs::insert));

    // Precondition: The reader change it status
    EXPECT_CALL(database, change_entity_status(EntityId(11), true)).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_physical_entity_discovery(EntityId(100), EntityKind::LOCATOR,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_physical_entity_discovery(EntityId(101), EntityKind::LOCATOR,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(11), EntityKind::DATAREADER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    participant_listener.on_subscriber_discovery(&statistics_participant, std::move(info));
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_reader_several_locators_no_host)
{
    std::vector<std::shared_ptr<const Entity>> existing_locators;
    int64_t next_entity_id = 100;

    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant exists and has ID 1
    // Precondition: The Participant is NOT linked to any host
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));
    EXPECT_CALL(database, get_entities(EntityKind::HOST, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>()));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic_));

    // Precondition: One unicast Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    existing_locators.push_back(existing_unicast_locator);

    // Precondition: One multicast Locator exists and has ID 4
    eprosima::fastrtps::rtps::Locator_t dds_existing_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_multicast_locator.address[12] = 127;
    dds_existing_multicast_locator.address[15] = 2;
    std::string existing_multicast_locator_name = to_string(dds_existing_multicast_locator);
    std::shared_ptr<Locator> existing_multicast_locator =
            std::make_shared<Locator>(existing_multicast_locator_name);
    existing_multicast_locator->id = 4;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_multicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(4)))));
    EXPECT_CALL(database, get_entity(EntityId(4))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_multicast_locator));
    existing_locators.push_back(existing_multicast_locator);

    // Precondition: One unicast Locator does not exist
    eprosima::fastrtps::rtps::Locator_t dds_new_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_new_unicast_locator.address[12] = 127;
    dds_new_unicast_locator.address[15] = 3;
    std::string new_unicast_locator_name = to_string(dds_new_unicast_locator);
    std::shared_ptr<Locator> new_unicast_locator =
            std::make_shared<Locator>(new_unicast_locator_name);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, new_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: One multicast Locator does not exist
    eprosima::fastrtps::rtps::Locator_t dds_new_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_new_multicast_locator.address[12] = 127;
    dds_new_multicast_locator.address[15] = 4;
    std::string new_multicast_locator_name = to_string(dds_new_multicast_locator);
    std::shared_ptr<Locator> new_multicast_locator =
            std::make_shared<Locator>(new_multicast_locator_name);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, new_multicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: Looking for the entities from the locator returns existing locators
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_locators));

    // Precondition: The database returns EntityIDs that are not used before
    database.set_next_entity_id(next_entity_id);

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::ReaderProxyData data(2, 2);

    // Precondition: The discovered reader is in the participant
    data.guid(reader_guid_);

    // Precondition: The discovered reader is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The discovered reader contains the locators
    data.add_unicast_locator(dds_existing_unicast_locator);
    data.add_unicast_locator(dds_new_unicast_locator);
    data.add_multicast_locator(dds_existing_multicast_locator);
    data.add_multicast_locator(dds_new_multicast_locator);

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ReaderDiscoveryInfo info(data);

    // Precondition: The Reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The new unicast locator is added to the database and given ID 100
    InsertEntityArgs insert_unicast_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::LOCATOR);
                EXPECT_EQ(entity->name, new_unicast_locator_name);

                return EntityId(100);
            });

    // Expectation: The new multicast locator is added to the database and given ID 101
    InsertEntityArgs insert_multicast_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::LOCATOR);
                EXPECT_EQ(entity->name, new_multicast_locator_name);

                return EntityId(101);
            });

    // Expectation: The DataWriter is added to the database and given ID 11
    InsertEntityArgs insert_reader_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::DATAREADER);
                EXPECT_EQ(entity->name, std::string("DataReader_") + topic_->name + "_" + reader_entity_id_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->topic, topic_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->participant, participant_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->guid, reader_guid_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->qos,
                reader_proxy_data_to_backend_qos(info.info));

                std::vector<std::string> expected_locator_names;
                expected_locator_names.push_back(existing_unicast_locator_name);
                expected_locator_names.push_back(existing_multicast_locator_name);
                expected_locator_names.push_back(new_unicast_locator_name);
                expected_locator_names.push_back(new_multicast_locator_name);
                EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->locators.size(),
                expected_locator_names.size());
                for (auto loc :  std::dynamic_pointer_cast<DataReader>(entity)->locators)
                {
                    EXPECT_THAT(expected_locator_names, Contains(loc.second->name));
                    if (loc.second->name == existing_unicast_locator->name)
                    {
                        EXPECT_EQ(loc.second->id, existing_unicast_locator->id);
                    }
                    if (loc.second->name == existing_multicast_locator->name)
                    {
                        EXPECT_EQ(loc.second->id, existing_multicast_locator->id);
                    }
                    if (loc.second->name == new_unicast_locator->name)
                    {
                        EXPECT_GE(loc.second->id, next_entity_id);
                    }
                    if (loc.second->name == new_multicast_locator->name)
                    {
                        EXPECT_GE(loc.second->id, next_entity_id);
                    }
                }

                return EntityId(11);
            });

    EXPECT_CALL(database, insert(_)).Times(3)
            .WillOnce(Invoke(&insert_unicast_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_multicast_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_reader_args, &InsertEntityArgs::insert));

    // Precondition: The reader change it status
    EXPECT_CALL(database, change_entity_status(EntityId(11), true)).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_physical_entity_discovery(EntityId(100), EntityKind::LOCATOR,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_physical_entity_discovery(EntityId(101), EntityKind::LOCATOR,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(11), EntityKind::DATAREADER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    participant_listener.on_subscriber_discovery(&statistics_participant, std::move(info));
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_reader_no_participant)
{
    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic_));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, existing_unicast_locator)));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::ReaderProxyData data(1, 1);

    // Precondition: The discovered reader is in the participant
    data.guid(reader_guid_);

    // Precondition: The discovered reader is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The discovered reader contains the locator
    data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ReaderDiscoveryInfo info(data);

    // Precondition: The Reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: No entity is added to the database
    EXPECT_CALL(database, insert(_)).Times(0);

    // Expectation: Nothing is inserted
    participant_listener.on_subscriber_discovery(&statistics_participant, std::move(info));
}

TEST_F(statistics_participant_listener_tests, new_reader_no_domain)
{
    // Precondition: The Domain 0 does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The Topic does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, existing_unicast_locator)));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::ReaderProxyData data(1, 1);

    // Precondition: The discovered reader is in the participant
    data.guid(reader_guid_);

    // Precondition: The discovered reader is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The Reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The discovered reader contains the locator
    data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ReaderDiscoveryInfo info(data);

    // Expectation: No entity is added to the database
    EXPECT_CALL(database, insert(_)).Times(0);

    // Expectation: Exception thrown
    participant_listener.on_subscriber_discovery(&statistics_participant, std::move(info));
}

TEST_F(statistics_participant_listener_tests, new_reader_discovered_reader_already_exists)
{
    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));

    // Precondition: The Participant is linked to a host with ID 50
    std::string host_name = "hostname";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);

    EXPECT_CALL(database, get_entities(EntityKind::HOST, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, host)));
    EXPECT_CALL(database, get_entity(EntityId(50))).Times(AnyNumber())
            .WillRepeatedly(Return(host));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic_));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, existing_unicast_locator)));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::ReaderProxyData data(1, 1);

    // Precondition: The discovered reader is in the participant
    data.guid(reader_guid_);

    // Precondition: The discovered reader is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The discovered reader contains the locator
    data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ReaderDiscoveryInfo info(data);

    // Precondition: The Reader exists and has ID 10
    std::shared_ptr<DataReader> reader =
            std::make_shared<DataReader>(reader_guid_str_, reader_proxy_data_to_backend_qos(
                        info.info), reader_guid_str_, participant_, topic_);
    reader->id = 10;
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(10))));
    EXPECT_CALL(database, get_entity(EntityId(10))).Times(AnyNumber())
            .WillRepeatedly(Return(reader));

    // Expectation: The DataReader is not inserted in the database.
    EXPECT_CALL(database, insert(_)).Times(0);

    // Expectation: The DataReader status is set to active
    EXPECT_CALL(database, change_entity_status(EntityId(10), true)).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::DATAREADER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener.
    participant_listener.on_subscriber_discovery(&statistics_participant, std::move(info));
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_reader_undiscovered_reader_already_exists)
{
    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));

    // Precondition: The Participant is linked to a host with ID 50
    std::string host_name = "hostname";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);

    EXPECT_CALL(database, get_entities(EntityKind::HOST, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, host)));
    EXPECT_CALL(database, get_entity(EntityId(50))).Times(AnyNumber())
            .WillRepeatedly(Return(host));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic_));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, existing_unicast_locator)));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::ReaderProxyData data(1, 1);

    // Precondition: The discovered reader is in the participant
    data.guid(reader_guid_);

    // Precondition: The discovered reader is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The discovered reader contains the locator
    data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ReaderDiscoveryInfo info(data);

    // Precondition: The Reader exists and has ID 10
    std::shared_ptr<DataReader> reader =
            std::make_shared<DataReader>(reader_guid_str_, reader_proxy_data_to_backend_qos(
                        info.info), reader_guid_str_, participant_, topic_);
    reader->id = 10;
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(10))));
    EXPECT_CALL(database, get_entity(EntityId(10))).Times(AnyNumber())
            .WillRepeatedly(Return(reader));

    // Expectation: The DataReader is not inserted in the database.
    EXPECT_CALL(database, insert(_)).Times(0);

    // Expectation: The DataReader status is set to inactive
    EXPECT_CALL(database, change_entity_status(EntityId(10), false)).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::DATAREADER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY)).Times(1);

    // Execution: Call the listener.
    info.status = eprosima::fastrtps::rtps::ReaderDiscoveryInfo::REMOVED_READER;
    participant_listener.on_subscriber_discovery(&statistics_participant, std::move(info));

    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_writer_discovered)
{
    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));

    // Precondition: The Participant is linked to a host with ID 50
    std::string host_name = "hostname";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);

    EXPECT_CALL(database, get_entities(EntityKind::HOST, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, host)));
    EXPECT_CALL(database, get_entity(EntityId(50))).Times(AnyNumber())
            .WillRepeatedly(Return(host));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic_));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, existing_unicast_locator)));

    // Start building the discovered writer info
    eprosima::fastrtps::rtps::WriterProxyData data(1, 1);

    // Precondition: The discovered writer is in the participant
    data.guid(writer_guid_);

    // Precondition: The discovered writer is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The discovered writer contains the locator
    data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered writer info
    eprosima::fastrtps::rtps::WriterDiscoveryInfo info(data);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The DataWriter is added to the database. We do not care about the given ID
    InsertEntityArgs insert_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::DATAWRITER);
                EXPECT_EQ(entity->name, std::string("DataWriter_") + topic_->name + "_" + writer_entity_id_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->topic, topic_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->participant, participant_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->guid, writer_guid_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->qos,
                writer_proxy_data_to_backend_qos(info.info));

                std::vector<std::string> expected_locator_names;
                expected_locator_names.push_back(existing_unicast_locator_name);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->locators.size(),
                expected_locator_names.size());
                for (auto loc :  std::dynamic_pointer_cast<DataWriter>(entity)->locators)
                {
                    EXPECT_THAT(expected_locator_names, Contains(loc.second->name));
                    EXPECT_EQ(loc.second->id, existing_unicast_locator->id);
                }

                return EntityId(10);
            });

    EXPECT_CALL(database, insert(_)).Times(1)
            .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert));

    // Precondition: The writer change it status
    EXPECT_CALL(database, change_entity_status(EntityId(10), true)).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    participant_listener.on_publisher_discovery(&statistics_participant, std::move(info));
}

TEST_F(statistics_participant_listener_tests, new_writer_undiscovered)
{
    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));

    // Precondition: The Participant is linked to a host with ID 50
    std::string host_name = "hostname";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);

    EXPECT_CALL(database, get_entities(EntityKind::HOST, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, host)));
    EXPECT_CALL(database, get_entity(EntityId(50))).Times(AnyNumber())
            .WillRepeatedly(Return(host));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic_));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, existing_unicast_locator)));

    // Start building the discovered writer info
    eprosima::fastrtps::rtps::WriterProxyData data(1, 1);

    // Precondition: The discovered writer is in the participant
    data.guid(writer_guid_);

    // Precondition: The discovered writer is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The discovered writer contains the locator
    data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered writer info
    eprosima::fastrtps::rtps::WriterDiscoveryInfo info(data);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The writer is not added to the database.
    EXPECT_CALL(database, insert(_)).Times(0);

    // Precondition: The writer does not change it status
    EXPECT_CALL(database, change_entity_status(_, _)).Times(0);

    // Execution: Call the listener
    info.status = eprosima::fastrtps::rtps::WriterDiscoveryInfo::REMOVED_WRITER;
    participant_listener.on_publisher_discovery(&statistics_participant, std::move(info));
}

TEST_F(statistics_participant_listener_tests, new_writer_no_topic)
{
    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));

    // Precondition: The Participant is linked to a host with ID 50
    std::string host_name = "hostname";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);

    EXPECT_CALL(database, get_entities(EntityKind::HOST, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, host)));
    EXPECT_CALL(database, get_entity(EntityId(50))).Times(AnyNumber())
            .WillRepeatedly(Return(host));

    // Precondition: The Topic does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, existing_unicast_locator)));

    // Start building the discovered writer info
    eprosima::fastrtps::rtps::WriterProxyData data(1, 1);

    // Precondition: The discovered writer is in the participant
    data.guid(writer_guid_);

    // Precondition: The discovered writer is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The discovered writer contains the locator
    data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered writer info
    eprosima::fastrtps::rtps::WriterDiscoveryInfo info(data);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The Topic is added to the database. We do not care about the given ID
    InsertEntityArgs insert_topic_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::TOPIC);
                EXPECT_EQ(entity->name, topic_name_);
                EXPECT_EQ(std::dynamic_pointer_cast<Topic>(entity)->domain, domain_);
                EXPECT_EQ(std::dynamic_pointer_cast<Topic>(entity)->data_type, type_name_);

                return EntityId(10);
            });

    // Expectation: The DataWriter is added to the database. We do not care about the given ID
    InsertEntityArgs insert_writer_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::DATAWRITER);
                EXPECT_EQ(entity->name, std::string("DataWriter_") + topic_->name + "_" + writer_entity_id_str_);

                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->participant, participant_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->guid, writer_guid_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->qos,
                writer_proxy_data_to_backend_qos(info.info));

                // We cannot check the Topic pointer as the queue will create a new one.
                // We check the topic_ data instead
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->topic->name, topic_name_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->topic->domain, domain_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->topic->data_type, type_name_);

                std::vector<std::string> expected_locator_names;
                expected_locator_names.push_back(existing_unicast_locator_name);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->locators.size(),
                expected_locator_names.size());
                for (auto loc :  std::dynamic_pointer_cast<DataWriter>(entity)->locators)
                {
                    EXPECT_THAT(expected_locator_names, Contains(loc.second->name));
                    EXPECT_EQ(loc.second->id, existing_unicast_locator->id);
                }

                return EntityId(11);
            });

    EXPECT_CALL(database, insert(_)).Times(2)
            .WillOnce(Invoke(&insert_topic_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_writer_args, &InsertEntityArgs::insert));

    // Precondition: The writer change it status
    EXPECT_CALL(database, change_entity_status(EntityId(11), true)).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::TOPIC,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(11), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    participant_listener.on_publisher_discovery(&statistics_participant, std::move(info));
}

TEST_F(statistics_participant_listener_tests, new_writer_several_locators
        )
{
    std::vector<std::shared_ptr<const Entity>> existing_locators;
    int64_t next_entity_id = 100;

    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));

    // Precondition: The Participant is linked to a host with ID 50
    std::string host_name = "hostname";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);

    EXPECT_CALL(database, get_entities(EntityKind::HOST, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, host)));
    EXPECT_CALL(database, get_entity(EntityId(50))).Times(AnyNumber())
            .WillRepeatedly(Return(host));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic_));

    // Precondition: One unicast Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    existing_locators.push_back(existing_unicast_locator);

    // Precondition: One multicast Locator exists and has ID 4
    eprosima::fastrtps::rtps::Locator_t dds_existing_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_multicast_locator.address[12] = 127;
    dds_existing_multicast_locator.address[15] = 2;
    std::string existing_multicast_locator_name = to_string(dds_existing_multicast_locator);
    std::shared_ptr<Locator> existing_multicast_locator =
            std::make_shared<Locator>(existing_multicast_locator_name);
    existing_multicast_locator->id = 4;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_multicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(4)))));
    EXPECT_CALL(database, get_entity(EntityId(4))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_multicast_locator));
    existing_locators.push_back(existing_multicast_locator);

    // Precondition: One unicast Locator does not exist
    eprosima::fastrtps::rtps::Locator_t dds_new_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_new_unicast_locator.address[12] = 127;
    dds_new_unicast_locator.address[15] = 3;
    std::string new_unicast_locator_name = to_string(dds_new_unicast_locator);
    std::shared_ptr<Locator> new_unicast_locator =
            std::make_shared<Locator>(new_unicast_locator_name);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, new_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: One multicast Locator does not exist
    eprosima::fastrtps::rtps::Locator_t dds_new_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_new_multicast_locator.address[12] = 127;
    dds_new_multicast_locator.address[15] = 4;
    std::string new_multicast_locator_name = to_string(dds_new_multicast_locator);
    std::shared_ptr<Locator> new_multicast_locator =
            std::make_shared<Locator>(new_multicast_locator_name);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, new_multicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: Looking for the entities from the locator returns existing locators
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_locators));

    // Precondition: The database returns EntityIDs that are not used before
    database.set_next_entity_id(next_entity_id);

    // Start building the discovered writer info
    eprosima::fastrtps::rtps::WriterProxyData data(2, 2);

    // Precondition: The discovered writer is in the participant
    data.guid(writer_guid_);

    // Precondition: The discovered writer is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The discovered writer contains the locators
    data.add_unicast_locator(dds_existing_unicast_locator);
    data.add_unicast_locator(dds_new_unicast_locator);
    data.add_multicast_locator(dds_existing_multicast_locator);
    data.add_multicast_locator(dds_new_multicast_locator);

    // Finish building the discovered writer info
    eprosima::fastrtps::rtps::WriterDiscoveryInfo info(data);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The new unicast locator is added to the database and given ID 100
    InsertEntityArgs insert_unicast_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::LOCATOR);
                EXPECT_EQ(entity->name, new_unicast_locator_name);

                return EntityId(100);
            });

    // Expectation: The new multicast locator is added to the database and given ID 101
    InsertEntityArgs insert_multicast_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::LOCATOR);
                EXPECT_EQ(entity->name, new_multicast_locator_name);

                return EntityId(101);
            });

    // Expectation: The DataWriter is added to the database and given ID 11
    InsertEntityArgs insert_writer_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::DATAWRITER);
                EXPECT_EQ(entity->name, std::string("DataWriter_") + topic_->name + "_" + writer_entity_id_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->topic, topic_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->participant, participant_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->guid, writer_guid_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->qos,
                writer_proxy_data_to_backend_qos(info.info));

                std::vector<std::string> expected_locator_names;
                expected_locator_names.push_back(existing_unicast_locator_name);
                expected_locator_names.push_back(existing_multicast_locator_name);
                expected_locator_names.push_back(new_unicast_locator_name);
                expected_locator_names.push_back(new_multicast_locator_name);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->locators.size(),
                expected_locator_names.size());
                for (auto loc :  std::dynamic_pointer_cast<DataWriter>(entity)->locators)
                {
                    EXPECT_THAT(expected_locator_names, Contains(loc.second->name));
                    if (loc.second->name == existing_unicast_locator->name)
                    {
                        EXPECT_EQ(loc.second->id, existing_unicast_locator->id);
                    }
                    if (loc.second->name == existing_multicast_locator->name)
                    {
                        EXPECT_EQ(loc.second->id, existing_multicast_locator->id);
                    }
                    if (loc.second->name == new_unicast_locator->name)
                    {
                        EXPECT_GE(loc.second->id, next_entity_id);
                    }
                    if (loc.second->name == new_multicast_locator->name)
                    {
                        EXPECT_GE(loc.second->id, next_entity_id);
                    }
                }

                return EntityId(11);
            });

    EXPECT_CALL(database, insert(_)).Times(3)
            .WillOnce(Invoke(&insert_unicast_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_multicast_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_writer_args, &InsertEntityArgs::insert));

    // Precondition: The writer change it status
    EXPECT_CALL(database, change_entity_status(EntityId(11), true)).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_physical_entity_discovery( EntityId(100), EntityKind::LOCATOR,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_physical_entity_discovery(EntityId(101), EntityKind::LOCATOR,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(11), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    participant_listener.on_publisher_discovery(&statistics_participant, std::move(info));
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_writer_several_locators_no_host)
{
    std::vector<std::shared_ptr<const Entity>> existing_locators;
    int64_t next_entity_id = 100;

    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant exists and has ID 1
    // Precondition: The Participant is NOT linked to any host
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));
    EXPECT_CALL(database, get_entities(EntityKind::HOST, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>()));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic_));

    // Precondition: One unicast Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    existing_locators.push_back(existing_unicast_locator);

    // Precondition: One multicast Locator exists and has ID 4
    eprosima::fastrtps::rtps::Locator_t dds_existing_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_multicast_locator.address[12] = 127;
    dds_existing_multicast_locator.address[15] = 2;
    std::string existing_multicast_locator_name = to_string(dds_existing_multicast_locator);
    std::shared_ptr<Locator> existing_multicast_locator =
            std::make_shared<Locator>(existing_multicast_locator_name);
    existing_multicast_locator->id = 4;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_multicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(4)))));
    EXPECT_CALL(database, get_entity(EntityId(4))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_multicast_locator));
    existing_locators.push_back(existing_multicast_locator);

    // Precondition: One unicast Locator does not exist
    eprosima::fastrtps::rtps::Locator_t dds_new_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_new_unicast_locator.address[12] = 127;
    dds_new_unicast_locator.address[15] = 3;
    std::string new_unicast_locator_name = to_string(dds_new_unicast_locator);
    std::shared_ptr<Locator> new_unicast_locator =
            std::make_shared<Locator>(new_unicast_locator_name);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, new_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: One multicast Locator does not exist
    eprosima::fastrtps::rtps::Locator_t dds_new_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_new_multicast_locator.address[12] = 127;
    dds_new_multicast_locator.address[15] = 4;
    std::string new_multicast_locator_name = to_string(dds_new_multicast_locator);
    std::shared_ptr<Locator> new_multicast_locator =
            std::make_shared<Locator>(new_multicast_locator_name);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, new_multicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: Looking for the entities from the locator returns existing locators
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_locators));

    // Precondition: The database returns EntityIDs that are not used before
    database.set_next_entity_id(next_entity_id);

    // Start building the discovered writer info
    eprosima::fastrtps::rtps::WriterProxyData data(2, 2);

    // Precondition: The discovered writer is in the participant
    data.guid(writer_guid_);

    // Precondition: The discovered writer is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The discovered writer contains the locators
    data.add_unicast_locator(dds_existing_unicast_locator);
    data.add_unicast_locator(dds_new_unicast_locator);
    data.add_multicast_locator(dds_existing_multicast_locator);
    data.add_multicast_locator(dds_new_multicast_locator);

    // Finish building the discovered writer info
    eprosima::fastrtps::rtps::WriterDiscoveryInfo info(data);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The new unicast locator is added to the database and given ID 100
    InsertEntityArgs insert_unicast_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::LOCATOR);
                EXPECT_EQ(entity->name, new_unicast_locator_name);

                return EntityId(100);
            });

    // Expectation: The new multicast locator is added to the database and given ID 101
    InsertEntityArgs insert_multicast_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::LOCATOR);
                EXPECT_EQ(entity->name, new_multicast_locator_name);

                return EntityId(101);
            });

    // Expectation: The DataWriter is added to the database and given ID 11
    InsertEntityArgs insert_writer_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::DATAWRITER);
                EXPECT_EQ(entity->name, std::string("DataWriter_") + topic_->name + "_" + writer_entity_id_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->topic, topic_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->participant, participant_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->guid, writer_guid_str_);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->qos,
                writer_proxy_data_to_backend_qos(info.info));

                std::vector<std::string> expected_locator_names;
                expected_locator_names.push_back(existing_unicast_locator_name);
                expected_locator_names.push_back(existing_multicast_locator_name);
                expected_locator_names.push_back(new_unicast_locator_name);
                expected_locator_names.push_back(new_multicast_locator_name);
                EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->locators.size(),
                expected_locator_names.size());
                for (auto loc :  std::dynamic_pointer_cast<DataWriter>(entity)->locators)
                {
                    EXPECT_THAT(expected_locator_names, Contains(loc.second->name));
                    if (loc.second->name == existing_unicast_locator->name)
                    {
                        EXPECT_EQ(loc.second->id, existing_unicast_locator->id);
                    }
                    if (loc.second->name == existing_multicast_locator->name)
                    {
                        EXPECT_EQ(loc.second->id, existing_multicast_locator->id);
                    }
                    if (loc.second->name == new_unicast_locator->name)
                    {
                        EXPECT_GE(loc.second->id, next_entity_id);
                    }
                    if (loc.second->name == new_multicast_locator->name)
                    {
                        EXPECT_GE(loc.second->id, next_entity_id);
                    }
                }

                return EntityId(11);
            });

    EXPECT_CALL(database, insert(_)).Times(3)
            .WillOnce(Invoke(&insert_unicast_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_multicast_args, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_writer_args, &InsertEntityArgs::insert));

    // Precondition: The writer change it status
    EXPECT_CALL(database, change_entity_status(EntityId(11), true)).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_physical_entity_discovery(EntityId(100), EntityKind::LOCATOR,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_physical_entity_discovery(EntityId(101), EntityKind::LOCATOR,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(11), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    participant_listener.on_publisher_discovery(&statistics_participant, std::move(info));
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_writer_no_participant)
{
    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic_));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, existing_unicast_locator)));

    // Start building the discovered writer info
    eprosima::fastrtps::rtps::WriterProxyData data(1, 1);

    // Precondition: The discovered writer is in the participant
    data.guid(writer_guid_);

    // Precondition: The discovered writer is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The discovered writer contains the locator
    data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered writer info
    eprosima::fastrtps::rtps::WriterDiscoveryInfo info(data);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: No entity is added to the database
    EXPECT_CALL(database, insert(_)).Times(0);

    // Expectation: Nothing is inserted
    participant_listener.on_publisher_discovery(&statistics_participant, std::move(info));

}

TEST_F(statistics_participant_listener_tests, new_writer_no_domain)
{
    // Precondition: The Domain 0 does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The Topic does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, existing_unicast_locator)));

    // Start building the discovered writer info
    eprosima::fastrtps::rtps::WriterProxyData data(1, 1);

    // Precondition: The discovered writer is in the participant
    data.guid(writer_guid_);

    // Precondition: The discovered writer is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The discovered writer contains the locator
    data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered writer info
    eprosima::fastrtps::rtps::WriterDiscoveryInfo info(data);

    // Expectation: No entity is added to the database
    EXPECT_CALL(database, insert(_)).Times(0);

    // Expectation: Exception thrown
    participant_listener.on_publisher_discovery(&statistics_participant, std::move(info));
}

TEST_F(statistics_participant_listener_tests, new_writer_discovered_writer_already_exists)
{
    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));

    // Precondition: The Participant is linked to a host with ID 50
    std::string host_name = "hostname";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);

    EXPECT_CALL(database, get_entities(EntityKind::HOST, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, host)));
    EXPECT_CALL(database, get_entity(EntityId(50))).Times(AnyNumber())
            .WillRepeatedly(Return(host));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic_));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, existing_unicast_locator)));

    // Start building the discovered writer info
    eprosima::fastrtps::rtps::WriterProxyData data(1, 1);

    // Precondition: The discovered writer is in the participant
    data.guid(writer_guid_);

    // Precondition: The discovered writer is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The discovered writer contains the locator
    data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered writer info
    eprosima::fastrtps::rtps::WriterDiscoveryInfo info(data);

    // Precondition: The writer exists and has ID 10
    std::shared_ptr<DataWriter> writer =
            std::make_shared<DataWriter>(writer_guid_str_, writer_proxy_data_to_backend_qos(
                        info.info), writer_guid_str_, participant_, topic_);
    writer->id = EntityId(10);
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(10))));
    EXPECT_CALL(database, get_entity(EntityId(10))).Times(AnyNumber())
            .WillRepeatedly(Return(writer));

    // Expectation: The DataWriter is not inserted in the database.
    EXPECT_CALL(database, insert(_)).Times(0);

    // Expectation: The DataWriter status is set to active
    EXPECT_CALL(database, change_entity_status(EntityId(10), true)).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener.
    participant_listener.on_publisher_discovery(&statistics_participant, std::move(info));
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_writer_undiscovered_writer_already_exists)
{
    // Precondition: The Domain 0 exists and has ID 0
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, std::to_string(statistics_participant.domain_id_))).Times(
        AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(0)))));
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain_));

    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant_));

    // Precondition: The Participant is linked to a host with ID 50
    std::string host_name = "hostname";
    std::shared_ptr<Host> host = std::make_shared<Host>(host_name);

    EXPECT_CALL(database, get_entities(EntityKind::HOST, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, host)));
    EXPECT_CALL(database, get_entity(EntityId(50))).Times(AnyNumber())
            .WillRepeatedly(Return(host));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic_));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, existing_unicast_locator_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(), EntityId(3)))));
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillRepeatedly(Return(existing_unicast_locator));
    EXPECT_CALL(database, get_entities(EntityKind::LOCATOR, EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::shared_ptr<const Entity>>(1, existing_unicast_locator)));

    // Start building the discovered writer info
    eprosima::fastrtps::rtps::WriterProxyData data(1, 1);

    // Precondition: The discovered writer is in the participant
    data.guid(writer_guid_);

    // Precondition: The discovered writer is in the topic
    data.topicName(topic_name_);
    data.typeName(type_name_);

    // Precondition: The discovered writer contains the locator
    data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered writer info
    eprosima::fastrtps::rtps::WriterDiscoveryInfo info(data);

    // Precondition: The writer exists and has ID 10
    std::shared_ptr<DataWriter> writer =
            std::make_shared<DataWriter>(writer_guid_str_, writer_proxy_data_to_backend_qos(
                        info.info), writer_guid_str_, participant_, topic_);
    writer->id = EntityId(10);
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(10))));
    EXPECT_CALL(database, get_entity(EntityId(10))).Times(AnyNumber())
            .WillRepeatedly(Return(writer));

    // Expectation: The DataWriter is not inserted in the database.
    EXPECT_CALL(database, insert(_)).Times(0);

    // Expectation: The DataWriter status is set to active
    EXPECT_CALL(database, change_entity_status(EntityId(10), false)).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY)).Times(1);

    // Execution: Call the listener.
    info.status = eprosima::fastrtps::rtps::WriterDiscoveryInfo::REMOVED_WRITER;
    participant_listener.on_publisher_discovery(&statistics_participant, std::move(info));
    entity_queue.flush();
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
