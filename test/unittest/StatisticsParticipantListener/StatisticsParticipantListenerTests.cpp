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
#include <fastdds_statistics_backend/topic_types/types.hpp>

#include <fastdds_statistics_backend/topic_types/monitorservice_types.hpp>

#include <fastdds/rtps/common/RemoteLocators.hpp>
#include "fastdds/rtps/common/Locator.hpp"

#include <gtest_aux.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <utility>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::AnyNumber;
using ::testing::Contains;
using ::testing::StrictMock;
using ::testing::Throw;

using namespace eprosima::fastdds::statistics;
using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;
using namespace eprosima::statistics_backend::subscriber;

using EntityId = eprosima::statistics_backend::EntityId;
using EntityKind = eprosima::statistics_backend::EntityKind;

struct InsertParticipantArgs
{
    InsertParticipantArgs (
            std::function<EntityId(
                const std::string& name,
                const Qos& qos,
                const std::string& guid,
                const EntityId& domain_id,
                const StatusLevel& status,
                const AppId& app_id,
                const std::string& app_metadata)> func)
        : callback_(func)
    {
    }

    EntityId insert(
            const std::string& name,
            const Qos& qos,
            const std::string& guid,
            const EntityId& domain_id,
            const StatusLevel& status,
            const AppId& app_id,
            const std::string& app_metadata)
    {
        name_ = name;
        qos_ = qos;
        guid_ = guid;
        domain_id_ = domain_id;
        status_ = status;
        app_id_ = app_id;
        app_metadata_ = app_metadata;
        return callback_(name, qos, guid, domain_id, status, app_id, app_metadata);
    }

    std::function<EntityId(
                const std::string& name,
                const Qos& qos,
                const std::string& guid,
                const EntityId& domain_id,
                const StatusLevel& status,
                const AppId& app_id,
                const std::string& app_metadata)> callback_;

    std::string name_;
    Qos qos_;
    std::string guid_;
    EntityId domain_id_;
    StatusLevel status_;
    AppId app_id_;
    std::string app_metadata_;
};

struct ProcessPhysicalArgs
{
    ProcessPhysicalArgs (
            std::function<void(
                const std::string& host_name,
                const std::string& user_name,
                const std::string& process_name,
                const std::string& process_pid,
                bool& should_link_process_participant,
                const EntityId& participant_id,
                std::map<std::string, EntityId>& physical_entities_ids)> func)
        : callback_(func)
    {
    }

    void process(
            const std::string& host_name,
            const std::string& user_name,
            const std::string& process_name,
            const std::string& process_pid,
            bool& should_link_process_participant,
            const EntityId& participant_id,
            std::map<std::string, EntityId>& physical_entities_ids)
    {
        host_name_ = host_name;
        user_name_ = user_name;
        process_name_ = process_name;
        process_pid_ = process_pid;
        should_link_process_participant_ = should_link_process_participant;
        participant_id_ = participant_id;
        physical_entities_ids_ = physical_entities_ids;
        callback_(host_name, user_name, process_name, process_pid, should_link_process_participant, participant_id,
                physical_entities_ids);
    }

    std::function<void(
                const std::string& host_name,
                const std::string& user_name,
                const std::string& process_name,
                const std::string& process_pid,
                bool& should_link_process_participant,
                const EntityId& participant_id,
                std::map<std::string, EntityId>& physical_entities_ids)> callback_;

    std::string host_name_;
    std::string user_name_;
    std::string process_name_;
    std::string process_pid_;
    bool should_link_process_participant_;
    EntityId participant_id_;
    std::map<std::string, EntityId> physical_entities_ids_;
};
struct InsertTopicArgs
{
    InsertTopicArgs (
            std::function<EntityId(
                const std::string& name,
                const std::string& type_name,
                const std::string& alias,
                const EntityId& domain_id)> func)
        : callback_(func)
    {
    }

    EntityId insert(
            const std::string& name,
            const std::string& type_name,
            const std::string& alias,
            const EntityId& domain_id)
    {
        name_ = name;
        type_name_ = type_name;
        alias_ = alias;
        domain_id_ = domain_id;
        return callback_(name, type_name, alias, domain_id);
    }

    std::function<EntityId(
                const std::string& name,
                const std::string& type_name,
                const std::string& alias,
                const EntityId& domain_id)> callback_;

    std::string name_;
    std::string type_name_;
    std::string alias_;
    EntityId domain_id_;

};

struct InsertEndpointArgs
{
    InsertEndpointArgs (
            std::function<EntityId(
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::
                        rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)> func)
        : callback_(func)
    {
    }

    EntityId insert(
            const std::string& endpoint_guid,
            const std::string& name,
            const std::string& alias,
            const Qos& qos,
            const bool& is_virtual_metatraffic,
            const eprosima::fastdds::
                    rtps::RemoteLocatorList& locators,
            const EntityKind& kind,
            const EntityId& participant_id,
            const EntityId& topic_id,
            const std::pair<AppId, std::string> app_data)
    {
        endpoint_guid_ = endpoint_guid;
        name_ = name;
        alias_ = alias;
        qos_ = qos;
        is_virtual_metatraffic_ = is_virtual_metatraffic;
        locators_ = locators;
        kind_ = kind;
        participant_id_ = participant_id;
        topic_id_ = topic_id;
        app_data_ = app_data;
        return callback_(endpoint_guid, name, alias, qos, is_virtual_metatraffic, locators, kind, participant_id,
                       topic_id, app_data);
    }

    std::function<EntityId(
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::
                        rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)> callback_;

    std::string endpoint_guid_;
    std::string name_;
    std::string alias_;
    Qos qos_;
    bool is_virtual_metatraffic_;
    eprosima::fastdds::
            rtps::RemoteLocatorList locators_;
    EntityKind kind_;
    EntityId participant_id_;
    EntityId topic_id_;
    std::pair<AppId, std::string> app_data_;
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
    DatabaseDataQueue<eprosima::fastdds::statistics::Data> data_queue;

    // Data queue, attached to the mocked database
    DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData> monitor_service_data_queue;

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

    // Entity Qos
    Qos entity_qos;

    // Host entity
    std::string host_name_;
    std::shared_ptr<Host> host_;

    // User entity
    std::string user_name_;
    std::shared_ptr<User> user_;

    // Process entity
    std::string process_name_;
    std::string process_pid_;
    std::shared_ptr<Process> process_;

    // Participant entity
    std::string participant_name_;
    Qos participant_qos_;
    std::string participant_prefix_str_;
    eprosima::fastdds::
            rtps::GuidPrefix_t guid_prefix_;
    std::string participant_guid_str_;
    eprosima::fastdds::
            rtps::GUID_t participant_guid_;
    std::shared_ptr<DomainParticipant> participant_;

    // Topic entity
    std::string topic_name_;
    std::string type_name_;
    std::shared_ptr<Topic> topic_;

    // Reader entity
    std::string reader_guid_str_;
    std::string reader_entity_id_str_;
    eprosima::fastdds::
            rtps::GUID_t reader_guid_;

    // Writer entity
    std::string writer_guid_str_;
    std::string writer_entity_id_str_;
    eprosima::fastdds::rtps::GUID_t writer_guid_;

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
        , monitor_service_data_queue(&database)
        , participant_listener(0, &database, &entity_queue, &data_queue, &monitor_service_data_queue)
    {
        //statistics_participant.domain_id_ = 0;

        // Domain entity
        domain_name_ = std::to_string(statistics_participant.domain_id_);
        domain_ = std::make_shared<Domain>(domain_name_);
        domain_->id = 0;

        // Host entity
        host_name_ = "host_name";
        host_ = std::make_shared<Host>(host_name_);

        // User entity
        user_name_ = "user_name";
        user_ = std::make_shared<User>(user_name_, host_);

        // Process entity
        process_pid_ = "12345";
        process_name_ = process_pid_;
        process_ = std::make_shared<Process>(process_name_, process_pid_, user_);

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

};

// Windows dll do not export ParticipantProxyData class members (private APIs)
#if !defined(_WIN32)
TEST_F(statistics_participant_listener_tests, new_participant_discovered)
{
    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_))
            .Times(AnyNumber())
            .WillOnce(Throw(eprosima::statistics_backend::BadParameter("Error")))
            .WillRepeatedly(Return(std::pair<EntityId, EntityId>(0, 10)));

    // Precondition: The Metatrafic Topic does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The Metatrafic Endpoint does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info

    eprosima::fastdds::rtps::ParticipantBuiltinTopicData data;

    // Precondition: The discovered participant has the given GUID and name
    data.guid = participant_guid_;
    data.participant_name = participant_name_;

    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_host, host_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_user, user_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_process, process_pid_);

    // Precondition: The discovered participant has unicast and multicast locators associated
    data.metatraffic_locators.unicast.push_back(1);
    data.metatraffic_locators.multicast.push_back(2);
    data.default_locators.unicast.push_back(3);
    data.default_locators.multicast.push_back(4);

    // Expectation: The Participant is added to the database. We do not care about the given ID
    InsertParticipantArgs insert_args([&](
                const std::string& name,
                const Qos& qos,
                const std::string& guid,
                const EntityId& domain_id,
                const StatusLevel& status,
                const AppId& app_id,
                const std::string& app_metadata)
            {
                EXPECT_EQ(name, participant_name_);
                EXPECT_EQ(qos, participant_proxy_data_to_backend_qos(data));
                EXPECT_EQ(guid, participant_guid_str_);
                EXPECT_EQ(domain_id, EntityId(0));
                EXPECT_EQ(status, StatusLevel::OK_STATUS);
                EXPECT_EQ(app_id, AppId::UNKNOWN);
                EXPECT_EQ(app_metadata, "");

                return EntityId(10);
            });

    EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_args, &InsertParticipantArgs::insert));

    // Expectation: The host is created and given ID 13, the user is created and given ID 14 and the process is created and given ID 15
    ProcessPhysicalArgs process_physical_args([&](
                const std::string& host_name,
                const std::string& user_name,
                const std::string& process_name,
                const std::string& process_pid,
                bool& should_link_process_participant,
                const EntityId& participant_id,
                std::map<std::string, EntityId>& physical_entities_ids)
            {
                EXPECT_EQ(host_name, host_name_);
                EXPECT_EQ(user_name, user_name_);
                EXPECT_EQ(process_name, process_name_);
                EXPECT_EQ(process_pid, process_pid_);
                EXPECT_EQ(should_link_process_participant, true);
                EXPECT_EQ(participant_id, EntityId(10));

                physical_entities_ids[HOST_ENTITY_TAG] = EntityId(13);
                physical_entities_ids[USER_ENTITY_TAG] = EntityId(14);
                physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId(15);

            });

    EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));

    // Expectation: The Metatraffic topic is added to the database. We do not care about the given ID
    InsertTopicArgs insert_topic_args([&](
                const std::string& name,
                const std::string& type_name,
                const std::string& alias,
                const EntityId& domain_id)
            {
                EXPECT_EQ(name, metatraffic_prefix + "TOPIC");
                EXPECT_EQ(type_name, metatraffic_prefix + "TYPE");
                EXPECT_EQ(alias, "_metatraffic_");
                EXPECT_EQ(domain_id, EntityId(0));

                return EntityId(11);
            });

    EXPECT_CALL(database, insert_new_topic(_, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_topic_args, &InsertTopicArgs::insert));

    // Expectation: The Metatraffic Endpoint is added to the database. We do not care about the given ID
    InsertEndpointArgs insert_datawriter_args([&](
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)
            {
                EXPECT_EQ(endpoint_guid, participant_guid_str_);
                EXPECT_EQ(name, "DataWriter_" + metatraffic_prefix + "TOPIC_0.0.1.c1");
                EXPECT_EQ(alias, "_metatraffic_");
                EXPECT_EQ(qos, metatraffic_qos_);
                EXPECT_EQ(is_virtual_metatraffic, true);
                EXPECT_EQ(locators.unicast[0], data.metatraffic_locators.unicast[0]);
                EXPECT_EQ(locators.unicast[1], data.default_locators.unicast[0]);
                EXPECT_EQ(locators.multicast[0], data.metatraffic_locators.multicast[0]);
                EXPECT_EQ(locators.multicast[1], data.default_locators.multicast[0]);

                EXPECT_EQ(kind, EntityKind::DATAWRITER);
                EXPECT_EQ(participant_id, EntityId(10));
                EXPECT_EQ(topic_id, EntityId(11));

                static_cast<void>(app_data);

                return EntityId(12);
            });

    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEndpointArgs::insert));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Expectation: Modify graph
    EXPECT_CALL(database,
            update_participant_in_graph(EntityId(0), EntityId(13), EntityId(14), EntityId(15),
            EntityId(10))).Times(1).WillOnce(Return(true));

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The metatraffic endpoint change it status
    EXPECT_CALL(database, change_entity_status(EntityId(12), true)).Times(1);

    // Expectation: Modify graph and notify user (twice, also from participant update)
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(10), EntityId(11),
            EntityId(12))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(2);

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
    eprosima::fastdds::rtps::ParticipantDiscoveryStatus status =
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_participant_discovery(&statistics_participant, status, data, should_be_ignored);
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_participant_discovered_not_first)
{
    // Participant_2 entity
    std::string participant_2_name = "participant_ name";
    Qos participant_2_qos;
    std::string participant_2_prefix_str = "01.02.03.04.05.06.07.08.09.0a.0b.1c";
    eprosima::fastdds::
            rtps::GuidPrefix_t guid_2_prefix;
    std::stringstream(participant_2_prefix_str) >> guid_2_prefix;
    std::string participant_2_guid_str = participant_2_prefix_str + "|0.0.1.c1";
    eprosima::fastdds::
            rtps::GUID_t participant_2_guid;
    std::stringstream(participant_2_guid_str) >> participant_2_guid;
    std::shared_ptr<DomainParticipant> participant_2 =
            std::make_shared<DomainParticipant>(participant_2_name, participant_2_qos, participant_2_guid_str,
                    std::shared_ptr<Process>(), domain_);

    // Precondition: Participant_5 exists and has ID 5
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(5))));

    // Precondition: The Metatrafic Topic exists and has ID 6
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(6)))));

    EXPECT_CALL(database, is_topic_in_database(_, EntityId(6))).Times(1)
            .WillOnce(Return(true));

    // Precondition: The Participant_1 Metatrafic Endpoint exists and has ID 7
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(7))));

    // Precondition: The new Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_2_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(eprosima::statistics_backend::BadParameter("Error")))
            .WillRepeatedly(Return(std::pair<EntityId, EntityId>(0, 10)));

    // Precondition: The Metatrafic Endpoint does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_2_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info
    eprosima::fastdds::rtps::ParticipantBuiltinTopicData data;

    // Precondition: The discovered participant has the given GUID and name
    data.guid = participant_2_guid;
    data.participant_name = participant_2_name;

    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_host, host_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_user, user_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_process, process_pid_);

    // Precondition: The discovered participant has unicast and multicast locators associated
    data.metatraffic_locators.unicast.push_back(1);
    data.metatraffic_locators.multicast.push_back(2);
    data.default_locators.unicast.push_back(3);
    data.default_locators.multicast.push_back(4);

    // Expectation: The Participant_2 is added to the database. We do not care about the given ID
    InsertParticipantArgs insert_args([&](
                const std::string& name,
                const Qos& qos,
                const std::string& guid,
                const EntityId& domain_id,
                const StatusLevel& status,
                const AppId& app_id,
                const std::string& app_metadata)
            {
                EXPECT_EQ(name, participant_2_name);
                EXPECT_EQ(qos, participant_proxy_data_to_backend_qos(data));
                EXPECT_EQ(guid, participant_2_guid_str);
                EXPECT_EQ(domain_id, EntityId(0));
                EXPECT_EQ(status, StatusLevel::OK_STATUS);
                EXPECT_EQ(app_id, AppId::UNKNOWN);
                EXPECT_EQ(app_metadata, "");

                return EntityId(10);
            });

    EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_args, &InsertParticipantArgs::insert));

    // Expectation: The host already exists with ID 13, the user already exists with ID 14 and the process already exists with ID 15
    ProcessPhysicalArgs process_physical_args([&](
                const std::string& host_name,
                const std::string& user_name,
                const std::string& process_name,
                const std::string& process_pid,
                bool& should_link_process_participant,
                const EntityId& participant_id,
                std::map<std::string, EntityId>& physical_entities_ids)
            {
                EXPECT_EQ(host_name, host_name_);
                EXPECT_EQ(user_name, user_name_);
                EXPECT_EQ(process_name, process_name_);
                EXPECT_EQ(process_pid, process_pid_);
                EXPECT_EQ(should_link_process_participant, true);
                EXPECT_EQ(participant_id, EntityId(10));

                physical_entities_ids[HOST_ENTITY_TAG] = EntityId(13);
                physical_entities_ids[USER_ENTITY_TAG] = EntityId(14);
                physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId(15);

            });

    EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));

    // Expectation: The Metatraffic endpoint is added to the database. We do not care about the given ID
    InsertEndpointArgs insert_datawriter_args([&](
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::
                        rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)
            {
                EXPECT_EQ(endpoint_guid, participant_2_guid_str);
                EXPECT_EQ(name, "DataWriter_" + metatraffic_prefix + "TOPIC_0.0.1.c1");
                EXPECT_EQ(alias, "_metatraffic_");
                EXPECT_EQ(qos, metatraffic_qos_);
                EXPECT_EQ(is_virtual_metatraffic, true);

                EXPECT_EQ(locators.unicast[0], data.metatraffic_locators.unicast[0]);
                EXPECT_EQ(locators.unicast[1], data.default_locators.unicast[0]);
                EXPECT_EQ(locators.multicast[0], data.metatraffic_locators.multicast[0]);
                EXPECT_EQ(locators.multicast[1], data.default_locators.multicast[0]);

                EXPECT_EQ(kind, EntityKind::DATAWRITER);
                EXPECT_EQ(participant_id, EntityId(10));
                EXPECT_EQ(topic_id, EntityId(6));

                static_cast<void>(app_data);

                return EntityId(12);
            });

    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEndpointArgs::insert));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Expectation: Modify graph
    EXPECT_CALL(database,
            update_participant_in_graph(EntityId(0), EntityId(13), EntityId(14), EntityId(15),
            EntityId(10))).Times(1).WillOnce(Return(true));

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The metatraffic endpoint change it status
    EXPECT_CALL(database, change_entity_status(EntityId(12), true)).Times(1);

    // Expectation: Modify graph and notify user (twice, also from participant update)
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(10), EntityId(6),
            EntityId(12))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(2);

    // Expectation: The metatraffic endpoint is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(12), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    eprosima::fastdds::rtps::ParticipantDiscoveryStatus status =
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_participant_discovery(&statistics_participant, status, data, should_be_ignored);
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_participant_undiscovered)
{
    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Metatraffic endpoint
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info
    eprosima::fastdds::rtps::ParticipantBuiltinTopicData data;

    // Precondition: The discovered participant has the given GUID and name
    data.guid = participant_guid_;
    data.participant_name = participant_name_;

    // Expectation: The Participant is not added to the database.
    EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(0);

    // Precondition: The Participant does not change it status
    EXPECT_CALL(database, change_entity_status(_, _)).Times(0);

    // Execution: Call the listener
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity

    eprosima::fastdds::rtps::ParticipantDiscoveryStatus status =
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus::REMOVED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, status, data, should_be_ignored);

    status = eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DROPPED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, status, data, should_be_ignored);
}

// Test that discovers a participant without locators associated and an empty name.
// Participant name will be localhost:participant_id
TEST_F(statistics_participant_listener_tests, new_participant_discovered_empty_name_no_locators)
{
    participant_name_ = "";

    // Precondition: The Participant does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_))
            .Times(AnyNumber())
            .WillOnce(Throw(eprosima::statistics_backend::BadParameter("Error")))
            .WillRepeatedly(Return(std::pair<EntityId, EntityId>(0, 10)));

    // Precondition: The Metatrafic Topic does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The Metatrafic Endpoint does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info
    eprosima::fastdds::rtps::ParticipantBuiltinTopicData data;

    // Precondition: The discovered participant has the given GUID and name
    data.guid = participant_guid_;
    data.participant_name = participant_name_;

    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_host, host_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_user, user_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_process, process_pid_);

    // Expectation: The Participant is added to the database. We do not care about the given ID
    InsertParticipantArgs insert_args([&](
                const std::string& name,
                const Qos& qos,
                const std::string& guid,
                const EntityId& domain_id,
                const StatusLevel& status,
                const AppId& app_id,
                const std::string& app_metadata)
            {
                EXPECT_EQ(name, "localhost:09.0a.0b.0c");
                EXPECT_EQ(qos, participant_proxy_data_to_backend_qos(data));
                EXPECT_EQ(guid, participant_guid_str_);
                EXPECT_EQ(domain_id, EntityId(0));
                EXPECT_EQ(status, StatusLevel::OK_STATUS);
                EXPECT_EQ(app_id, AppId::UNKNOWN);
                EXPECT_EQ(app_metadata, "");

                return EntityId(10);
            });

    EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_args, &InsertParticipantArgs::insert));

    // Expectation: The host is created and given ID 13, the user is created and given ID 14 and the process is created and given ID 15
    ProcessPhysicalArgs process_physical_args([&](
                const std::string& host_name,
                const std::string& user_name,
                const std::string& process_name,
                const std::string& process_pid,
                bool& should_link_process_participant,
                const EntityId& participant_id,
                std::map<std::string, EntityId>& physical_entities_ids)
            {
                EXPECT_EQ(host_name, host_name_);
                EXPECT_EQ(user_name, user_name_);
                EXPECT_EQ(process_name, process_name_);
                EXPECT_EQ(process_pid, process_pid_);
                EXPECT_EQ(should_link_process_participant, true);
                EXPECT_EQ(participant_id, EntityId(10));

                physical_entities_ids[HOST_ENTITY_TAG] = EntityId(13);
                physical_entities_ids[USER_ENTITY_TAG] = EntityId(14);
                physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId(15);

            });

    EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));

    // Expectation: The Metatraffic topic is added to the database. We do not care about the given ID
    InsertTopicArgs insert_topic_args([&](
                const std::string& name,
                const std::string& type_name,
                const std::string& alias,
                const EntityId& domain_id)
            {
                EXPECT_EQ(name, metatraffic_prefix + "TOPIC");
                EXPECT_EQ(type_name, metatraffic_prefix + "TYPE");
                EXPECT_EQ(alias, "_metatraffic_");
                EXPECT_EQ(domain_id, EntityId(0));

                return EntityId(11);
            });

    EXPECT_CALL(database, insert_new_topic(_, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_topic_args, &InsertTopicArgs::insert));

    // Expectation: The Metatraffic Endpoint is added to the database. We do not care about the given ID
    InsertEndpointArgs insert_datawriter_args([&](
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::
                        rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)
            {
                EXPECT_EQ(endpoint_guid, participant_guid_str_);
                EXPECT_EQ(name, "DataWriter_" + metatraffic_prefix + "TOPIC_0.0.1.c1");
                EXPECT_EQ(alias, "_metatraffic_");
                EXPECT_EQ(qos, metatraffic_qos_);
                EXPECT_EQ(is_virtual_metatraffic, true);
                EXPECT_TRUE(locators.unicast.empty());
                EXPECT_TRUE(locators.multicast.empty());
                EXPECT_EQ(kind, EntityKind::DATAWRITER);
                EXPECT_EQ(participant_id, EntityId(10));
                EXPECT_EQ(topic_id, EntityId(11));

                static_cast<void>(app_data);

                return EntityId(12);
            });

    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEndpointArgs::insert));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Expectation: Modify graph
    EXPECT_CALL(database,
            update_participant_in_graph(EntityId(0), EntityId(13), EntityId(14), EntityId(15),
            EntityId(10))).Times(1).WillOnce(Return(true));

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The metatraffic endpoint change it status
    EXPECT_CALL(database, change_entity_status(EntityId(12), true)).Times(1);

    // Expectation: Modify graph and notify user (twice, also from participant update)
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(10), EntityId(11),
            EntityId(12))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(2);

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
    eprosima::fastdds::rtps::ParticipantDiscoveryStatus status =
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_participant_discovery(&statistics_participant, status, data, should_be_ignored);
    entity_queue.flush();
}

// Test that discovers a participant with a locator associated and an empty name.
// Participant name will be locator_IP:participant_id
TEST_F(statistics_participant_listener_tests, new_participant_discovered_empty_name_default_unicast_locator)
{
    participant_name_ = "";

    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_))
            .Times(AnyNumber())
            .WillOnce(Throw(eprosima::statistics_backend::BadParameter("Error")))
            .WillRepeatedly(Return(std::pair<EntityId, EntityId>(0, 10)));

    // Precondition: The Metatrafic Topic does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The Metatrafic Endpoint does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info
    eprosima::fastdds::rtps::ParticipantBuiltinTopicData data;

    // Precondition: The discovered participant has the given GUID and name
    data.guid = participant_guid_;
    data.participant_name = participant_name_;

    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_host, host_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_user, user_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_process, process_pid_);

    // Precondition: The Unicast Locator exists and has ID 3
    eprosima::fastdds::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 37;
    dds_existing_unicast_locator.address[13] = 11;
    dds_existing_unicast_locator.address[14] = 18;
    dds_existing_unicast_locator.address[15] = 30;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Precondition: The discovered reader contains the locator
    data.default_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Precondition: The Unicast Metatraffic Locator exists and has ID 4
    eprosima::fastdds::rtps::Locator_t dds_existing_metatraffic_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_metatraffic_unicast_locator.address[12] = 38;
    dds_existing_metatraffic_unicast_locator.address[13] = 12;
    dds_existing_metatraffic_unicast_locator.address[14] = 19;
    dds_existing_metatraffic_unicast_locator.address[15] = 31;
    std::string existing_metatraffic_unicast_locator_name = to_string(dds_existing_metatraffic_unicast_locator);
    std::shared_ptr<Locator> existing_metatraffic_unicast_locator =
            std::make_shared<Locator>(existing_metatraffic_unicast_locator_name);
    existing_metatraffic_unicast_locator->id = 4;

    // Precondition: The discovered reader contains the locator
    data.metatraffic_locators.add_unicast_locator(dds_existing_metatraffic_unicast_locator);

    // Precondition: The Multicast Locator exists and has ID 5
    eprosima::fastdds::rtps::Locator_t dds_existing_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_multicast_locator.address[12] = 36;
    dds_existing_multicast_locator.address[13] = 10;
    dds_existing_multicast_locator.address[14] = 17;
    dds_existing_multicast_locator.address[15] = 29;
    std::string existing_multicast_locator_name = to_string(dds_existing_multicast_locator);
    std::shared_ptr<Locator> existing_multicast_locator =
            std::make_shared<Locator>(existing_multicast_locator_name);
    existing_multicast_locator->id = 5;

    // Precondition: The discovered reader contains the locator
    data.default_locators.add_multicast_locator(dds_existing_multicast_locator);

    // Precondition: The Multicast Metatraffic Locator exists and has ID 6
    eprosima::fastdds::rtps::Locator_t dds_existing_metatraffic_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_metatraffic_multicast_locator.address[12] = 47;
    dds_existing_metatraffic_multicast_locator.address[13] = 21;
    dds_existing_metatraffic_multicast_locator.address[14] = 28;
    dds_existing_metatraffic_multicast_locator.address[15] = 40;
    std::string existing_metatraffic_multicast_locator_name = to_string(dds_existing_metatraffic_multicast_locator);
    std::shared_ptr<Locator> existing_metatraffic_multicast_locator =
            std::make_shared<Locator>(existing_metatraffic_multicast_locator_name);
    existing_metatraffic_multicast_locator->id = 6;

    // Precondition: The discovered reader contains the locator
    data.metatraffic_locators.add_multicast_locator(dds_existing_metatraffic_multicast_locator);

    // Expectation: The Participant is added to the database. We do not care about the given ID
    InsertParticipantArgs insert_args([&](
                const std::string& name,
                const Qos& qos,
                const std::string& guid,
                const EntityId& domain_id,
                const StatusLevel& status,
                const AppId& app_id,
                const std::string& app_metadata)
            {
                EXPECT_EQ(name, "37.11.18.30:09.0a.0b.0c");
                EXPECT_EQ(qos, participant_proxy_data_to_backend_qos(data));
                EXPECT_EQ(guid, participant_guid_str_);
                EXPECT_EQ(domain_id, EntityId(0));
                EXPECT_EQ(status, StatusLevel::OK_STATUS);
                EXPECT_EQ(app_id, AppId::UNKNOWN);
                EXPECT_EQ(app_metadata, "");

                return EntityId(10);
            });

    EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_args, &InsertParticipantArgs::insert));

    // Expectation: The host is created and given ID 13, the user is created and given ID 14 and the process is created and given ID 15
    ProcessPhysicalArgs process_physical_args([&](
                const std::string& host_name,
                const std::string& user_name,
                const std::string& process_name,
                const std::string& process_pid,
                bool& should_link_process_participant,
                const EntityId& participant_id,
                std::map<std::string, EntityId>& physical_entities_ids)
            {
                EXPECT_EQ(host_name, host_name_);
                EXPECT_EQ(user_name, user_name_);
                EXPECT_EQ(process_name, process_name_);
                EXPECT_EQ(process_pid, process_pid_);
                EXPECT_EQ(should_link_process_participant, true);
                EXPECT_EQ(participant_id, EntityId(10));

                physical_entities_ids[HOST_ENTITY_TAG] = EntityId(13);
                physical_entities_ids[USER_ENTITY_TAG] = EntityId(14);
                physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId(15);

            });

    EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));

    // Expectation: The Metatraffic topic is added to the database. We do not care about the given ID
    InsertTopicArgs insert_topic_args([&](
                const std::string& name,
                const std::string& type_name,
                const std::string& alias,
                const EntityId& domain_id)
            {
                EXPECT_EQ(name, metatraffic_prefix + "TOPIC");
                EXPECT_EQ(type_name, metatraffic_prefix + "TYPE");
                EXPECT_EQ(alias, "_metatraffic_");
                EXPECT_EQ(domain_id, EntityId(0));

                return EntityId(11);
            });

    EXPECT_CALL(database, insert_new_topic(_, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_topic_args, &InsertTopicArgs::insert));

    // Expectation: The Metatraffic Endpoint is added to the database. We do not care about the given ID
    InsertEndpointArgs insert_datawriter_args([&](
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::
                        rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)
            {
                EXPECT_EQ(endpoint_guid, participant_guid_str_);
                EXPECT_EQ(name, "DataWriter_" + metatraffic_prefix + "TOPIC_0.0.1.c1");
                EXPECT_EQ(alias, "_metatraffic_");
                EXPECT_EQ(qos, metatraffic_qos_);
                EXPECT_EQ(is_virtual_metatraffic, true);

                EXPECT_EQ(locators.unicast[0], data.metatraffic_locators.unicast[0]);
                EXPECT_EQ(locators.unicast[1], data.default_locators.unicast[0]);
                EXPECT_EQ(locators.multicast[0], data.metatraffic_locators.multicast[0]);
                EXPECT_EQ(locators.multicast[1], data.default_locators.multicast[0]);

                EXPECT_EQ(kind, EntityKind::DATAWRITER);
                EXPECT_EQ(participant_id, EntityId(10));
                EXPECT_EQ(topic_id, EntityId(11));

                static_cast<void>(app_data);

                return EntityId(12);
            });

    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEndpointArgs::insert));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Expectation: Modify graph and notify user
    EXPECT_CALL(database,
            update_participant_in_graph(EntityId(0), EntityId(13), EntityId(14), EntityId(15),
            EntityId(10))).Times(1).WillOnce(Return(true));

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The metatraffic endpoint change it status
    EXPECT_CALL(database, change_entity_status(EntityId(12), true)).Times(1);

    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(10), EntityId(11),
            EntityId(12))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(2);

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
    eprosima::fastdds::rtps::ParticipantDiscoveryStatus status =
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_participant_discovery(&statistics_participant, status, data, should_be_ignored);
    entity_queue.flush();
}

// Test that discovers a participant with a locator associated and an empty name.
// Participant name will be locator_IP:participant_id
TEST_F(statistics_participant_listener_tests, new_participant_discovered_empty_name_metatraffic_unicast_locator)
{
    participant_name_ = "";

    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_))
            .Times(AnyNumber())
            .WillOnce(Throw(eprosima::statistics_backend::BadParameter("Error")))
            .WillRepeatedly(Return(std::pair<EntityId, EntityId>(0, 10)));

    // Precondition: The Metatrafic Topic does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The Metatrafic Endpoint does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info
    eprosima::fastdds::rtps::ParticipantBuiltinTopicData data;

    // Precondition: The discovered participant has the given GUID and name
    data.guid = participant_guid_;
    data.participant_name = participant_name_;

    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_host, host_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_user, user_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_process, process_pid_);

    // Precondition: The Unicast Metatraffic Locator exists and has ID 4
    eprosima::fastdds::rtps::Locator_t dds_existing_metatraffic_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_metatraffic_unicast_locator.address[12] = 37;
    dds_existing_metatraffic_unicast_locator.address[13] = 11;
    dds_existing_metatraffic_unicast_locator.address[14] = 18;
    dds_existing_metatraffic_unicast_locator.address[15] = 30;
    std::string existing_metatraffic_unicast_locator_name = to_string(dds_existing_metatraffic_unicast_locator);
    std::shared_ptr<Locator> existing_metatraffic_unicast_locator =
            std::make_shared<Locator>(existing_metatraffic_unicast_locator_name);
    existing_metatraffic_unicast_locator->id = 4;

    // Precondition: The discovered reader contains the locator
    data.metatraffic_locators.add_unicast_locator(dds_existing_metatraffic_unicast_locator);

    // Precondition: The Multicast Locator exists and has ID 5
    eprosima::fastdds::rtps::Locator_t dds_existing_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_multicast_locator.address[12] = 36;
    dds_existing_multicast_locator.address[13] = 10;
    dds_existing_multicast_locator.address[14] = 17;
    dds_existing_multicast_locator.address[15] = 29;
    std::string existing_multicast_locator_name = to_string(dds_existing_multicast_locator);
    std::shared_ptr<Locator> existing_multicast_locator =
            std::make_shared<Locator>(existing_multicast_locator_name);
    existing_multicast_locator->id = 5;

    // Precondition: The discovered reader contains the locator
    data.default_locators.add_multicast_locator(dds_existing_multicast_locator);

    // Precondition: The Multicast Metatraffic Locator exists and has ID 6
    eprosima::fastdds::rtps::Locator_t dds_existing_metatraffic_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_metatraffic_multicast_locator.address[12] = 47;
    dds_existing_metatraffic_multicast_locator.address[13] = 21;
    dds_existing_metatraffic_multicast_locator.address[14] = 28;
    dds_existing_metatraffic_multicast_locator.address[15] = 40;
    std::string existing_metatraffic_multicast_locator_name = to_string(dds_existing_metatraffic_multicast_locator);
    std::shared_ptr<Locator> existing_metatraffic_multicast_locator =
            std::make_shared<Locator>(existing_metatraffic_multicast_locator_name);
    existing_metatraffic_multicast_locator->id = 6;

    // Precondition: The discovered reader contains the locator
    data.metatraffic_locators.add_multicast_locator(dds_existing_metatraffic_multicast_locator);

    // Expectation: The Participant is added to the database. We do not care about the given ID
    InsertParticipantArgs insert_args([&](
                const std::string& name,
                const Qos& qos,
                const std::string& guid,
                const EntityId& domain_id,
                const StatusLevel& status,
                const AppId& app_id,
                const std::string& app_metadata)
            {
                EXPECT_EQ(name, "37.11.18.30:09.0a.0b.0c");
                EXPECT_EQ(qos, participant_proxy_data_to_backend_qos(data));
                EXPECT_EQ(guid, participant_guid_str_);
                EXPECT_EQ(domain_id, EntityId(0));
                EXPECT_EQ(status, StatusLevel::OK_STATUS);
                EXPECT_EQ(app_id, AppId::UNKNOWN);
                EXPECT_EQ(app_metadata, "");

                return EntityId(10);
            });

    EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_args, &InsertParticipantArgs::insert));

    // Expectation: The host is created and given ID 13, the user is created and given ID 14 and the process is created and given ID 15
    ProcessPhysicalArgs process_physical_args([&](
                const std::string& host_name,
                const std::string& user_name,
                const std::string& process_name,
                const std::string& process_pid,
                bool& should_link_process_participant,
                const EntityId& participant_id,
                std::map<std::string, EntityId>& physical_entities_ids)
            {
                EXPECT_EQ(host_name, host_name_);
                EXPECT_EQ(user_name, user_name_);
                EXPECT_EQ(process_name, process_name_);
                EXPECT_EQ(process_pid, process_pid_);
                EXPECT_EQ(should_link_process_participant, true);
                EXPECT_EQ(participant_id, EntityId(10));

                physical_entities_ids[HOST_ENTITY_TAG] = EntityId(13);
                physical_entities_ids[USER_ENTITY_TAG] = EntityId(14);
                physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId(15);

            });

    EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));

    // Expectation: The Metatraffic topic is added to the database. We do not care about the given ID
    InsertTopicArgs insert_topic_args([&](
                const std::string& name,
                const std::string& type_name,
                const std::string& alias,
                const EntityId& domain_id)
            {
                EXPECT_EQ(name, metatraffic_prefix + "TOPIC");
                EXPECT_EQ(type_name, metatraffic_prefix + "TYPE");
                EXPECT_EQ(alias, "_metatraffic_");
                EXPECT_EQ(domain_id, EntityId(0));

                return EntityId(11);
            });

    EXPECT_CALL(database, insert_new_topic(_, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_topic_args, &InsertTopicArgs::insert));

    // Expectation: The Metatraffic Endpoint is added to the database. We do not care about the given ID
    InsertEndpointArgs insert_datawriter_args([&](
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::
                        rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)
            {
                EXPECT_EQ(endpoint_guid, participant_guid_str_);
                EXPECT_EQ(name, "DataWriter_" + metatraffic_prefix + "TOPIC_0.0.1.c1");
                EXPECT_EQ(alias, "_metatraffic_");
                EXPECT_EQ(qos, metatraffic_qos_);
                EXPECT_EQ(is_virtual_metatraffic, true);

                EXPECT_EQ(locators.unicast[0], data.metatraffic_locators.unicast[0]);
                EXPECT_EQ(locators.multicast[0], data.metatraffic_locators.multicast[0]);
                EXPECT_EQ(locators.multicast[1], data.default_locators.multicast[0]);

                EXPECT_EQ(kind, EntityKind::DATAWRITER);
                EXPECT_EQ(participant_id, EntityId(10));
                EXPECT_EQ(topic_id, EntityId(11));

                static_cast<void>(app_data);

                return EntityId(12);
            });

    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEndpointArgs::insert));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Expectation: Modify graph
    EXPECT_CALL(database,
            update_participant_in_graph(EntityId(0), EntityId(13), EntityId(14), EntityId(15),
            EntityId(10))).Times(1).WillOnce(Return(true));

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The metatraffic endpoint change it status
    EXPECT_CALL(database, change_entity_status(EntityId(12), true)).Times(1);

    // Expectation: Modify graph and notify user
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(10), EntityId(11),
            EntityId(12))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(2);

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
    eprosima::fastdds::rtps::ParticipantDiscoveryStatus status =
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_participant_discovery(&statistics_participant, status, data, should_be_ignored);
    entity_queue.flush();
}

// Test that discovers a participant with a locator associated and an empty name.
// Participant name will be locator_IP:participant_id
TEST_F(statistics_participant_listener_tests, new_participant_discovered_empty_name_default_multicast_locator)
{
    participant_name_ = "";

    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_))
            .Times(AnyNumber())
            .WillOnce(Throw(eprosima::statistics_backend::BadParameter("Error")))
            .WillRepeatedly(Return(std::pair<EntityId, EntityId>(0, 10)));

    // Precondition: The Host does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::HOST, host_name_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The User does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::USER, user_name_))
            .Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The Process does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::PROCESS, process_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The Metatrafic Topic does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The Metatrafic Endpoint does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info
    eprosima::fastdds::rtps::ParticipantBuiltinTopicData data;

    // Precondition: The discovered participant has the given GUID and name
    data.guid = participant_guid_;
    data.participant_name = participant_name_;

    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_host, host_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_user, user_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_process, process_pid_);

    // Precondition: The Multicast Locator exists and has ID 5
    eprosima::fastdds::
            rtps::Locator_t dds_existing_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_multicast_locator.address[12] = 37;
    dds_existing_multicast_locator.address[13] = 11;
    dds_existing_multicast_locator.address[14] = 18;
    dds_existing_multicast_locator.address[15] = 30;
    std::string existing_multicast_locator_name = to_string(dds_existing_multicast_locator);
    std::shared_ptr<Locator> existing_multicast_locator =
            std::make_shared<Locator>(existing_multicast_locator_name);
    existing_multicast_locator->id = 5;

    // Precondition: The discovered reader contains the locator
    data.default_locators.add_multicast_locator(dds_existing_multicast_locator);

    // Precondition: The Multicast Metatraffic Locator exists and has ID 6
    eprosima::fastdds::
            rtps::Locator_t dds_existing_metatraffic_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_metatraffic_multicast_locator.address[12] = 47;
    dds_existing_metatraffic_multicast_locator.address[13] = 21;
    dds_existing_metatraffic_multicast_locator.address[14] = 28;
    dds_existing_metatraffic_multicast_locator.address[15] = 40;
    std::string existing_metatraffic_multicast_locator_name = to_string(dds_existing_metatraffic_multicast_locator);
    std::shared_ptr<Locator> existing_metatraffic_multicast_locator =
            std::make_shared<Locator>(existing_metatraffic_multicast_locator_name);
    existing_metatraffic_multicast_locator->id = 6;

    // Precondition: The discovered reader contains the locator
    data.metatraffic_locators.add_multicast_locator(dds_existing_metatraffic_multicast_locator);

    // Expectation: The Participant is added to the database. We do not care about the given ID
    InsertParticipantArgs insert_args([&](
                const std::string& name,
                const Qos& qos,
                const std::string& guid,
                const EntityId& domain_id,
                const StatusLevel& status,
                const AppId& app_id,
                const std::string& app_metadata)
            {
                EXPECT_EQ(name, "37.11.18.30:09.0a.0b.0c");
                EXPECT_EQ(qos, participant_proxy_data_to_backend_qos(data));
                EXPECT_EQ(guid, participant_guid_str_);
                EXPECT_EQ(domain_id, EntityId(0));
                EXPECT_EQ(status, StatusLevel::OK_STATUS);
                EXPECT_EQ(app_id, AppId::UNKNOWN);
                EXPECT_EQ(app_metadata, "");

                return EntityId(10);
            });

    EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_args, &InsertParticipantArgs::insert));

    // Expectation: The host is created and given ID 13, the user is created and given ID 14 and the process is created and given ID 15
    ProcessPhysicalArgs process_physical_args([&](
                const std::string& host_name,
                const std::string& user_name,
                const std::string& process_name,
                const std::string& process_pid,
                bool& should_link_process_participant,
                const EntityId& participant_id,
                std::map<std::string, EntityId>& physical_entities_ids)
            {
                EXPECT_EQ(host_name, host_name_);
                EXPECT_EQ(user_name, user_name_);
                EXPECT_EQ(process_name, process_name_);
                EXPECT_EQ(process_pid, process_pid_);
                EXPECT_EQ(should_link_process_participant, true);
                EXPECT_EQ(participant_id, EntityId(10));

                physical_entities_ids[HOST_ENTITY_TAG] = EntityId(13);
                physical_entities_ids[USER_ENTITY_TAG] = EntityId(14);
                physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId(15);

            });

    EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));

    // Expectation: The Metatraffic topic is added to the database. We do not care about the given ID
    InsertTopicArgs insert_topic_args([&](
                const std::string& name,
                const std::string& type_name,
                const std::string& alias,
                const EntityId& domain_id)
            {
                EXPECT_EQ(name, metatraffic_prefix + "TOPIC");
                EXPECT_EQ(type_name, metatraffic_prefix + "TYPE");
                EXPECT_EQ(alias, "_metatraffic_");
                EXPECT_EQ(domain_id, EntityId(0));

                return EntityId(11);
            });

    EXPECT_CALL(database, insert_new_topic(_, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_topic_args, &InsertTopicArgs::insert));

    // Expectation: The Metatraffic Endpoint is added to the database. We do not care about the given ID
    InsertEndpointArgs insert_datawriter_args([&](
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::
                        rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)
            {
                EXPECT_EQ(endpoint_guid, participant_guid_str_);
                EXPECT_EQ(name, "DataWriter_" + metatraffic_prefix + "TOPIC_0.0.1.c1");
                EXPECT_EQ(alias, "_metatraffic_");
                EXPECT_EQ(qos, metatraffic_qos_);
                EXPECT_EQ(is_virtual_metatraffic, true);

                EXPECT_EQ(locators.multicast[0], data.metatraffic_locators.multicast[0]);
                EXPECT_EQ(locators.multicast[1], data.default_locators.multicast[0]);

                EXPECT_EQ(kind, EntityKind::DATAWRITER);
                EXPECT_EQ(participant_id, EntityId(10));
                EXPECT_EQ(topic_id, EntityId(11));

                static_cast<void>(app_data);

                return EntityId(12);
            });

    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEndpointArgs::insert));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Expectation: Modify graph
    EXPECT_CALL(database,
            update_participant_in_graph(EntityId(0), EntityId(13), EntityId(14), EntityId(15),
            EntityId(10))).Times(1).WillOnce(Return(true));

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The metatraffic endpoint change it status
    EXPECT_CALL(database, change_entity_status(EntityId(12), true)).Times(1);

    // Expectation: Modify graph and notify user (twice, also from participant update)
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(10), EntityId(11),
            EntityId(12))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(2);

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
    eprosima::fastdds::rtps::ParticipantDiscoveryStatus status =
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_participant_discovery(&statistics_participant, status, data, should_be_ignored);
    entity_queue.flush();
}

// Test that discovers a participant with a locator associated and an empty name.
// Participant name will be locator_IP:participant_id
TEST_F(statistics_participant_listener_tests, new_participant_discovered_empty_name_metatraffic_multicast_locator)
{
    participant_name_ = "";

    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_))
            .Times(AnyNumber())
            .WillOnce(Throw(eprosima::statistics_backend::BadParameter("Error")))
            .WillRepeatedly(Return(std::pair<EntityId, EntityId>(0, 10)));

    // Precondition: The Metatrafic Topic does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The Metatrafic Endpoint does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info
    eprosima::fastdds::rtps::ParticipantBuiltinTopicData data;

    // Precondition: The discovered participant has the given GUID and name
    data.guid = participant_guid_;
    data.participant_name = participant_name_;

    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_host, host_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_user, user_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_process, process_pid_);

    // Precondition: The Multicast Metatraffic Locator exists and has ID 6
    eprosima::fastdds::
            rtps::Locator_t dds_existing_metatraffic_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_metatraffic_multicast_locator.address[12] = 37;
    dds_existing_metatraffic_multicast_locator.address[13] = 11;
    dds_existing_metatraffic_multicast_locator.address[14] = 18;
    dds_existing_metatraffic_multicast_locator.address[15] = 30;
    std::string existing_metatraffic_multicast_locator_name = to_string(dds_existing_metatraffic_multicast_locator);
    std::shared_ptr<Locator> existing_metatraffic_multicast_locator =
            std::make_shared<Locator>(existing_metatraffic_multicast_locator_name);
    existing_metatraffic_multicast_locator->id = 6;

    // Precondition: The discovered reader contains the locator
    data.metatraffic_locators.add_multicast_locator(dds_existing_metatraffic_multicast_locator);

    // Expectation: The Participant is added to the database. We do not care about the given ID
    InsertParticipantArgs insert_args([&](
                const std::string& name,
                const Qos& qos,
                const std::string& guid,
                const EntityId& domain_id,
                const StatusLevel& status,
                const AppId& app_id,
                const std::string& app_metadata)
            {
                EXPECT_EQ(name, "37.11.18.30:09.0a.0b.0c");
                EXPECT_EQ(qos, participant_proxy_data_to_backend_qos(data));
                EXPECT_EQ(guid, participant_guid_str_);
                EXPECT_EQ(domain_id, EntityId(0));
                EXPECT_EQ(status, StatusLevel::OK_STATUS);
                EXPECT_EQ(app_id, AppId::UNKNOWN);
                EXPECT_EQ(app_metadata, "");

                return EntityId(10);
            });

    EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_args, &InsertParticipantArgs::insert));

    // Expectation: The host is created and given ID 13, the user is created and given ID 14 and the process is created and given ID 15
    ProcessPhysicalArgs process_physical_args([&](
                const std::string& host_name,
                const std::string& user_name,
                const std::string& process_name,
                const std::string& process_pid,
                bool& should_link_process_participant,
                const EntityId& participant_id,
                std::map<std::string, EntityId>& physical_entities_ids)
            {
                EXPECT_EQ(host_name, host_name_);
                EXPECT_EQ(user_name, user_name_);
                EXPECT_EQ(process_name, process_name_);
                EXPECT_EQ(process_pid, process_pid_);
                EXPECT_EQ(should_link_process_participant, true);
                EXPECT_EQ(participant_id, EntityId(10));

                physical_entities_ids[HOST_ENTITY_TAG] = EntityId(13);
                physical_entities_ids[USER_ENTITY_TAG] = EntityId(14);
                physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId(15);

            });

    EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));

    // Expectation: The Metatraffic topic is added to the database. We do not care about the given ID
    InsertTopicArgs insert_topic_args([&](
                const std::string& name,
                const std::string& type_name,
                const std::string& alias,
                const EntityId& domain_id)
            {
                EXPECT_EQ(name, metatraffic_prefix + "TOPIC");
                EXPECT_EQ(type_name, metatraffic_prefix + "TYPE");
                EXPECT_EQ(alias, "_metatraffic_");
                EXPECT_EQ(domain_id, EntityId(0));

                return EntityId(11);
            });

    EXPECT_CALL(database, insert_new_topic(_, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_topic_args, &InsertTopicArgs::insert));

    // Expectation: The Metatraffic Endpoint is added to the database. We do not care about the given ID
    InsertEndpointArgs insert_datawriter_args([&](
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::
                        rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)
            {
                EXPECT_EQ(endpoint_guid, participant_guid_str_);
                EXPECT_EQ(name, "DataWriter_" + metatraffic_prefix + "TOPIC_0.0.1.c1");
                EXPECT_EQ(alias, "_metatraffic_");
                EXPECT_EQ(qos, metatraffic_qos_);
                EXPECT_EQ(is_virtual_metatraffic, true);

                EXPECT_EQ(locators.multicast[0], data.metatraffic_locators.multicast[0]);

                EXPECT_EQ(kind, EntityKind::DATAWRITER);
                EXPECT_EQ(participant_id, EntityId(10));
                EXPECT_EQ(topic_id, EntityId(11));

                static_cast<void>(app_data);

                return EntityId(12);
            });

    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEndpointArgs::insert));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Expectation: Modify graph
    EXPECT_CALL(database,
            update_participant_in_graph(EntityId(0), EntityId(13), EntityId(14), EntityId(15),
            EntityId(10))).Times(1).WillOnce(Return(true));

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The metatraffic endpoint change it status
    EXPECT_CALL(database, change_entity_status(EntityId(12), true)).Times(1);

    // Expectation: Modify graph and notify user (twice, also from participant update)
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(10), EntityId(11),
            EntityId(12))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(2);

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
    eprosima::fastdds::rtps::ParticipantDiscoveryStatus status =
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_participant_discovery(&statistics_participant, status, data, should_be_ignored);
    entity_queue.flush();
}

// Test that discoverS a participant with localhost locators associated and an empty name.
// Participant name will be localhost:participant_id
TEST_F(statistics_participant_listener_tests, new_participant_discovered_empty_name_localhost_locators)
{
    participant_name_ = "";

    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_))
            .Times(AnyNumber())
            .WillOnce(Throw(eprosima::statistics_backend::BadParameter("Error")))
            .WillRepeatedly(Return(std::pair<EntityId, EntityId>(0, 10)));

    // Precondition: The Metatrafic Topic does not exist
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(0)));

    // Precondition: The Metatrafic Endpoint does not exist
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info
    eprosima::fastdds::rtps::ParticipantBuiltinTopicData data;

    // Precondition: The discovered participant has the given GUID and name
    data.guid = participant_guid_;
    data.participant_name = participant_name_;

    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_host, host_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_user, user_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_process, process_pid_);

    // Precondition: The Unicast Locator exists and has ID 3
    eprosima::fastdds::
            rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Precondition: The discovered reader contains the locator
    data.default_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Precondition: The Unicast Metatraffic Locator exists and has ID 4
    eprosima::fastdds::
            rtps::Locator_t dds_existing_metatraffic_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_metatraffic_unicast_locator.address[12] = 127;
    dds_existing_metatraffic_unicast_locator.address[15] = 1;
    std::string existing_metatraffic_unicast_locator_name = to_string(dds_existing_metatraffic_unicast_locator);
    std::shared_ptr<Locator> existing_metatraffic_unicast_locator =
            std::make_shared<Locator>(existing_metatraffic_unicast_locator_name);
    existing_metatraffic_unicast_locator->id = 4;

    // Precondition: The discovered reader contains the locator
    data.metatraffic_locators.add_unicast_locator(dds_existing_metatraffic_unicast_locator);

    // Precondition: The Multicast Locator exists and has ID 5
    eprosima::fastdds::
            rtps::Locator_t dds_existing_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_multicast_locator.address[12] = 127;
    dds_existing_multicast_locator.address[15] = 1;
    std::string existing_multicast_locator_name = to_string(dds_existing_multicast_locator);
    std::shared_ptr<Locator> existing_multicast_locator =
            std::make_shared<Locator>(existing_multicast_locator_name);
    existing_multicast_locator->id = 5;

    // Precondition: The discovered reader contains the locator
    data.default_locators.add_multicast_locator(dds_existing_multicast_locator);

    // Precondition: The Multicast Metatraffic Locator exists and has ID 6
    eprosima::fastdds::
            rtps::Locator_t dds_existing_metatraffic_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_metatraffic_multicast_locator.address[12] = 127;
    dds_existing_metatraffic_multicast_locator.address[15] = 1;
    std::string existing_metatraffic_multicast_locator_name = to_string(dds_existing_metatraffic_multicast_locator);
    std::shared_ptr<Locator> existing_metatraffic_multicast_locator =
            std::make_shared<Locator>(existing_metatraffic_multicast_locator_name);
    existing_metatraffic_multicast_locator->id = 6;

    // Precondition: The discovered reader contains the locator
    data.metatraffic_locators.add_multicast_locator(dds_existing_metatraffic_multicast_locator);

    // Expectation: The Participant is added to the database. We do not care about the given ID
    InsertParticipantArgs insert_args([&](
                const std::string& name,
                const Qos& qos,
                const std::string& guid,
                const EntityId& domain_id,
                const StatusLevel& status,
                const AppId& app_id,
                const std::string& app_metadata)
            {
                EXPECT_EQ(name, "localhost:09.0a.0b.0c");
                EXPECT_EQ(qos, participant_proxy_data_to_backend_qos(data));
                EXPECT_EQ(guid, participant_guid_str_);
                EXPECT_EQ(domain_id, EntityId(0));
                EXPECT_EQ(status, StatusLevel::OK_STATUS);
                EXPECT_EQ(app_id, AppId::UNKNOWN);
                EXPECT_EQ(app_metadata, "");

                return EntityId(10);
            });

    EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_args, &InsertParticipantArgs::insert));

    // Expectation: The host is created and given ID 13, the user is created and given ID 14 and the process is created and given ID 15
    ProcessPhysicalArgs process_physical_args([&](
                const std::string& host_name,
                const std::string& user_name,
                const std::string& process_name,
                const std::string& process_pid,
                bool& should_link_process_participant,
                const EntityId& participant_id,
                std::map<std::string, EntityId>& physical_entities_ids)
            {
                EXPECT_EQ(host_name, host_name_);
                EXPECT_EQ(user_name, user_name_);
                EXPECT_EQ(process_name, process_name_);
                EXPECT_EQ(process_pid, process_pid_);
                EXPECT_EQ(should_link_process_participant, true);
                EXPECT_EQ(participant_id, EntityId(10));

                physical_entities_ids[HOST_ENTITY_TAG] = EntityId(13);
                physical_entities_ids[USER_ENTITY_TAG] = EntityId(14);
                physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId(15);

            });

    EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));

    // Expectation: The Metatraffic topic is added to the database. We do not care about the given ID
    InsertTopicArgs insert_topic_args([&](
                const std::string& name,
                const std::string& type_name,
                const std::string& alias,
                const EntityId& domain_id)
            {
                EXPECT_EQ(name, metatraffic_prefix + "TOPIC");
                EXPECT_EQ(type_name, metatraffic_prefix + "TYPE");
                EXPECT_EQ(alias, "_metatraffic_");
                EXPECT_EQ(domain_id, EntityId(0));

                return EntityId(11);
            });

    EXPECT_CALL(database, insert_new_topic(_, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_topic_args, &InsertTopicArgs::insert));

    // Expectation: The Metatraffic Endpoint is added to the database. We do not care about the given ID
    InsertEndpointArgs insert_datawriter_args([&](
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::
                        rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)
            {
                EXPECT_EQ(endpoint_guid, participant_guid_str_);
                EXPECT_EQ(name, "DataWriter_" + metatraffic_prefix + "TOPIC_0.0.1.c1");
                EXPECT_EQ(alias, "_metatraffic_");
                EXPECT_EQ(qos, metatraffic_qos_);
                EXPECT_EQ(is_virtual_metatraffic, true);

                EXPECT_EQ(locators.unicast[0], data.default_locators.unicast[0]);
                EXPECT_EQ(locators.unicast[0], data.metatraffic_locators.unicast[0]);
                EXPECT_EQ(locators.multicast[0], data.default_locators.multicast[0]);
                EXPECT_EQ(locators.multicast[0], data.metatraffic_locators.multicast[0]);

                EXPECT_EQ(kind, EntityKind::DATAWRITER);
                EXPECT_EQ(participant_id, EntityId(10));
                EXPECT_EQ(topic_id, EntityId(11));

                static_cast<void>(app_data);

                return EntityId(12);
            });

    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEndpointArgs::insert));

    // Expectation: Modify graph
    EXPECT_CALL(database,
            update_participant_in_graph(EntityId(0), EntityId(13), EntityId(14), EntityId(15),
            EntityId(10))).Times(1).WillOnce(Return(true));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The metatraffic endpoint change it status
    EXPECT_CALL(database, change_entity_status(EntityId(12), true)).Times(1);

    // Expectation: Modify graph and notify user (twice, also from participant update)
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(10), EntityId(11),
            EntityId(12))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(2);

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
    eprosima::fastdds::rtps::ParticipantDiscoveryStatus status =
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_participant_discovery(&statistics_participant, status, data, should_be_ignored);
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_participant_no_domain)
{
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
    eprosima::fastdds::rtps::ParticipantBuiltinTopicData data;

    // Precondition: The discovered participant has the given GUID and name
    data.guid = participant_guid_;
    data.participant_name = participant_name_;

    // Expectation: No entity is added to the database
    EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Throw(eprosima::statistics_backend::BadParameter("Error")));

    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    eprosima::fastdds::rtps::ParticipantDiscoveryStatus status =
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, status, data, should_be_ignored);
}

TEST_F(statistics_participant_listener_tests, new_participant_discovered_participant_already_exists)
{
    // Precondition: The Participant exists and has ID 5
    participant_->id = EntityId(5);
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(5))));

    // Precondition: The Metatrafic Topic exists and has ID 6
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::TOPIC, metatraffic_prefix + "TOPIC")).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(6)))));

    // Precondition: The Metatrafic Endpoint exists and has ID 7
    EXPECT_CALL(database,
            get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(7))));
    EXPECT_CALL(database, is_topic_in_database(_, EntityId(6))).Times(1)
            .WillOnce(Return(true));

    // Start building the discovered reader info
    eprosima::fastdds::rtps::ParticipantBuiltinTopicData data;

    // Precondition: The discovered participant has the given GUID and name
    data.guid = participant_guid_;
    data.participant_name = participant_name_;

    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_host, host_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_user, user_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_process, process_pid_);

    // Precondition: The discovered participant has unicast and multicast locators associated
    data.metatraffic_locators.unicast.push_back(1);
    data.metatraffic_locators.multicast.push_back(2);
    data.default_locators.unicast.push_back(3);
    data.default_locators.multicast.push_back(4);


    // Expectation: The host already exists with ID 13, the user already exists with ID 14 and the process already exists with ID 15
    ProcessPhysicalArgs process_physical_args([&](
                const std::string& host_name,
                const std::string& user_name,
                const std::string& process_name,
                const std::string& process_pid,
                bool& should_link_process_participant,
                const EntityId& participant_id,
                std::map<std::string, EntityId>& physical_entities_ids)
            {
                EXPECT_EQ(host_name, host_name_);
                EXPECT_EQ(user_name, user_name_);
                EXPECT_EQ(process_name, process_name_);
                EXPECT_EQ(process_pid, process_pid_);
                EXPECT_EQ(should_link_process_participant, false);
                EXPECT_EQ(participant_id, EntityId(5));

                physical_entities_ids[HOST_ENTITY_TAG] = EntityId(13);
                physical_entities_ids[USER_ENTITY_TAG] = EntityId(14);
                physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId(15);

            });

    EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));


    // Expectation: The Participant status is set to active
    EXPECT_CALL(database, change_entity_status(EntityId(7), true)).Times(1);
    EXPECT_CALL(database, change_entity_status(EntityId(5), true)).Times(1);

    // Expectation: Modify graph
    EXPECT_CALL(database, update_participant_in_graph(EntityId(0), EntityId(13), EntityId(14), EntityId(15), EntityId(
                5))).Times(1).WillOnce(Return(true));

    // Expectation: The Participant is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(5), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: Modify graph and notify user
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(5), EntityId(6), EntityId(7))).Times(1).WillOnce(Return(
                false));

    // Expectation: The metatraffic endpoint is discovered
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(7), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener.
    eprosima::fastdds::rtps::ParticipantDiscoveryStatus status =
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DISCOVERED_PARTICIPANT;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_participant_discovery(&statistics_participant, status, data, should_be_ignored);
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_participant_undiscovered_participant_already_exists)
{
    // Precondition: The Participant exists and has ID 1
    participant_->id = EntityId(1);
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Metatraffic endpoint
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Start building the discovered reader info
    eprosima::fastdds::rtps::ParticipantBuiltinTopicData data;

    // Precondition: The discovered participant has the given GUID and name
    data.guid = participant_guid_;
    data.participant_name = participant_name_;

    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_host, host_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_user, user_name_);
    data.properties.push_back(eprosima::fastdds::dds::parameter_policy_physical_data_process, process_pid_);

    // Expectation: The Participant is not inserted in the database.
    EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(0);

    // Expectation: The host already exists with ID 13, the user already exists with ID 14 and the process already exists with ID 15
    ProcessPhysicalArgs process_physical_args([&](
                const std::string& host_name,
                const std::string& user_name,
                const std::string& process_name,
                const std::string& process_pid,
                bool& should_link_process_participant,
                const EntityId& participant_id,
                std::map<std::string, EntityId>& physical_entities_ids)
            {
                EXPECT_EQ(host_name, host_name_);
                EXPECT_EQ(user_name, user_name_);
                EXPECT_EQ(process_name, process_name_);
                EXPECT_EQ(process_pid, process_pid_);
                EXPECT_EQ(should_link_process_participant, false);
                EXPECT_EQ(participant_id, EntityId(1));

                physical_entities_ids[HOST_ENTITY_TAG] = EntityId(13);
                physical_entities_ids[USER_ENTITY_TAG] = EntityId(14);
                physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId(15);

            });

    EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(2)
            .WillRepeatedly(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));

    // Expectation: The Participant status is set to inactive
    EXPECT_CALL(database, change_entity_status(EntityId(1), false)).Times(2);

    // Expectation: Modify graph and notify user
    EXPECT_CALL(database, update_participant_in_graph(EntityId(0), EntityId(13), EntityId(14), EntityId(15), EntityId(
                1))).Times(2)
            .WillOnce(Return(true))
            .WillOnce(Return(false));

    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(1), EntityKind::PARTICIPANT,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY)).Times(2);

    // Execution: Call the listener.
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    eprosima::fastdds::rtps::ParticipantDiscoveryStatus status =
            eprosima::fastdds::rtps::ParticipantDiscoveryStatus::REMOVED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, status, data, should_be_ignored);

    status = eprosima::fastdds::rtps::ParticipantDiscoveryStatus::DROPPED_PARTICIPANT;
    participant_listener.on_participant_discovery(&statistics_participant, status, data, should_be_ignored);

    entity_queue.flush();
}

#endif // !defined(_WIN32)

TEST_F(statistics_participant_listener_tests, new_reader_discovered)
{
    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, is_topic_in_database(_, EntityId(2))).Times(1)
            .WillOnce(Return(true));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastdds::
            rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Start building the discovered reader info
    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);

    // Precondition: The discovered reader is in the participant
    data.guid = reader_guid_;

    // Precondition: The discovered reader is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The discovered reader contains the locator
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Precondition: The Reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The DataReader is added to the database. We do not care about the given ID
    InsertEndpointArgs insert_datareader_args([&](
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::
                        rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)
            {
                EXPECT_EQ(endpoint_guid, reader_guid_str_);
                EXPECT_EQ(name, std::string("DataReader_") + topic_->name + "_" + reader_entity_id_str_);
                EXPECT_EQ(alias, "");
                EXPECT_EQ(qos, reader_proxy_data_to_backend_qos(data));
                EXPECT_EQ(is_virtual_metatraffic, false);
                EXPECT_EQ(locators.unicast[0], data.remote_locators.unicast[0]);

                EXPECT_EQ(kind, EntityKind::DATAREADER);
                EXPECT_EQ(participant_id, EntityId(1));
                EXPECT_EQ(topic_id, EntityId(2));

                static_cast<void>(app_data);

                return EntityId(10);
            });

    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_datareader_args, &InsertEndpointArgs::insert));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Precondition: The reader change it status
    EXPECT_CALL(database, change_entity_status(EntityId(10), true)).Times(1);

    // Expectation: Modify graph and notify user
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(1), EntityId(2), EntityId(10))).Times(1).WillOnce(Return(
                true));
    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::DATAREADER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    eprosima::fastdds::rtps::ReaderDiscoveryStatus status =
            eprosima::fastdds::rtps::ReaderDiscoveryStatus::DISCOVERED_READER;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_data_reader_discovery(&statistics_participant, status, data, should_be_ignored);
}

TEST_F(statistics_participant_listener_tests, new_reader_undiscovered)
{
    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastdds::
            rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Start building the discovered reader info
    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);

    // Precondition: The discovered reader is in the participant
    data.guid = reader_guid_;

    // Precondition: The discovered reader is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The discovered reader contains the locator
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Precondition: The Reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The Datareader is not added to the database.
    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

    // Precondition: The Datareader does not change it status
    EXPECT_CALL(database, change_entity_status(_, _)).Times(0);

    // Execution: Call the listener
    eprosima::fastdds::rtps::ReaderDiscoveryStatus status =
            eprosima::fastdds::rtps::ReaderDiscoveryStatus::REMOVED_READER;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_data_reader_discovery(&statistics_participant, status, data, should_be_ignored);
}

TEST_F(statistics_participant_listener_tests, new_reader_no_topic)
{
    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The Topic does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastdds::
            rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Start building the discovered reader info
    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);

    // Precondition: The discovered reader is in the participant
    data.guid = reader_guid_;

    // Precondition: The discovered reader is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The discovered reader contains the locator
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Precondition: The Reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The Topic is added to the database. We do not care about the given ID
    InsertTopicArgs insert_topic_args([&](
                const std::string& name,
                const std::string& type_name,
                const std::string& alias,
                const EntityId& domain_id)
            {
                EXPECT_EQ(name, topic_name_);
                EXPECT_EQ(type_name, type_name_);
                EXPECT_EQ(alias, "");
                EXPECT_EQ(domain_id, EntityId(0));

                return EntityId(10);
            });

    EXPECT_CALL(database, insert_new_topic(_, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_topic_args, &InsertTopicArgs::insert));

    // Expectation: The DataReader is added to the database. We do not care about the given ID
    InsertEndpointArgs insert_datareader_args([&](
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::
                        rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)
            {
                EXPECT_EQ(endpoint_guid, reader_guid_str_);
                EXPECT_EQ(name, std::string("DataReader_") + topic_->name + "_" + reader_entity_id_str_);
                EXPECT_EQ(alias, "");
                EXPECT_EQ(qos, reader_proxy_data_to_backend_qos(data));
                EXPECT_EQ(is_virtual_metatraffic, false);

                EXPECT_EQ(locators.unicast[0], data.remote_locators.unicast[0]);

                EXPECT_EQ(kind, EntityKind::DATAREADER);
                EXPECT_EQ(participant_id, EntityId(1));
                EXPECT_EQ(topic_id, EntityId(10));

                static_cast<void>(app_data);

                return EntityId(11);
            });

    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_datareader_args, &InsertEndpointArgs::insert));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Precondition: The reader change it status
    EXPECT_CALL(database, change_entity_status(EntityId(11), true)).Times(1);

    // Expectation: Modify graph and notify user (twice, also from participant update)
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(1), EntityId(10),
            EntityId(11))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(1);

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
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    eprosima::fastdds::rtps::ReaderDiscoveryStatus status =
            eprosima::fastdds::rtps::ReaderDiscoveryStatus::DISCOVERED_READER;
    participant_listener.on_data_reader_discovery(&statistics_participant, status, data, should_be_ignored);
}

TEST_F(statistics_participant_listener_tests, new_reader_several_topics)
{
    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: Another domain with ID 100 exists
    std::string another_domain_name = "another_domain";
    std::shared_ptr<Domain> another_domain = std::make_shared<Domain>(another_domain_name);
    EXPECT_CALL(database,
            get_entities_by_name(EntityKind::DOMAIN, another_domain_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(100), EntityId(100)))));

    // Precondition: Another topic with the same name and type exists in domain 100
    // and has ID 101
    std::shared_ptr<Topic> topic_another_domain = std::make_shared<Topic>(topic_name_, type_name_, another_domain);
    EXPECT_CALL(database, is_topic_in_database(_, EntityId(101))).Times(0);

    // Precondition: Another topic with the same name but different type exists in the initial domain 0
    // and has ID 102
    std::string another_type_name = "another_type";
    std::shared_ptr<Topic> topic_another_type = std::make_shared<Topic>(topic_name_, another_type_name, domain_);
    EXPECT_CALL(database, is_topic_in_database(_, EntityId(102))).Times(1)
            .WillOnce(Return(false));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>
            { { EntityId(100), EntityId(101) },
                { EntityId(0), EntityId(102) },
                { EntityId(0), EntityId(2) }}));
    EXPECT_CALL(database, is_topic_in_database(_, EntityId(2))).Times(1)
            .WillOnce(Return(true));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastdds::
            rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Start building the discovered reader info
    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);

    // Precondition: The discovered reader is in the participant
    data.guid = reader_guid_;

    // Precondition: The discovered reader is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The discovered reader contains the locator
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);



    // Precondition: The Reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The DataReader is added to the database. We do not care about the given ID
    InsertEndpointArgs insert_datareader_args([&](
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::
                        rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)
            {
                EXPECT_EQ(endpoint_guid, reader_guid_str_);
                EXPECT_EQ(name, std::string("DataReader_") + topic_->name + "_" + reader_entity_id_str_);
                EXPECT_EQ(alias, "");
                EXPECT_EQ(qos, reader_proxy_data_to_backend_qos(data));
                EXPECT_EQ(is_virtual_metatraffic, false);

                EXPECT_EQ(locators.unicast[0], data.remote_locators.unicast[0]);

                EXPECT_EQ(kind, EntityKind::DATAREADER);
                EXPECT_EQ(participant_id, EntityId(1));
                EXPECT_EQ(topic_id, EntityId(2));

                static_cast<void>(app_data);

                return EntityId(10);
            });

    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_datareader_args, &InsertEndpointArgs::insert));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Precondition: The reader change it status
    EXPECT_CALL(database, change_entity_status(EntityId(10), true)).Times(1);

    // Expectation: Modify graph and notify user (twice, also from participant update)
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(1), EntityId(2), EntityId(10))).Times(1).WillOnce(Return(
                true));
    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::DATAREADER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    eprosima::fastdds::rtps::ReaderDiscoveryStatus status =
            eprosima::fastdds::rtps::ReaderDiscoveryStatus::DISCOVERED_READER;
    participant_listener.on_data_reader_discovery(&statistics_participant, status, data, should_be_ignored);
}

TEST_F(statistics_participant_listener_tests, new_reader_several_locators)
{
    std::vector<std::shared_ptr<const Entity>> existing_locators;
    int64_t next_entity_id = 100;

    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, is_topic_in_database(_, EntityId(2))).Times(1)
            .WillOnce(Return(true));

    // Precondition: One unicast Locator exists and has ID 3
    eprosima::fastdds::
            rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Precondition: One multicast Locator exists and has ID 4
    eprosima::fastdds::
            rtps::Locator_t dds_existing_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_multicast_locator.address[12] = 127;
    dds_existing_multicast_locator.address[15] = 2;
    std::string existing_multicast_locator_name = to_string(dds_existing_multicast_locator);
    std::shared_ptr<Locator> existing_multicast_locator =
            std::make_shared<Locator>(existing_multicast_locator_name);
    existing_multicast_locator->id = 4;

    // Precondition: One unicast Locator does not exist
    eprosima::fastdds::
            rtps::Locator_t dds_new_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_new_unicast_locator.address[12] = 127;
    dds_new_unicast_locator.address[15] = 3;
    std::string new_unicast_locator_name = to_string(dds_new_unicast_locator);
    std::shared_ptr<Locator> new_unicast_locator =
            std::make_shared<Locator>(new_unicast_locator_name);

    // Precondition: One multicast Locator does not exist
    eprosima::fastdds::
            rtps::Locator_t dds_new_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_new_multicast_locator.address[12] = 127;
    dds_new_multicast_locator.address[15] = 4;
    std::string new_multicast_locator_name = to_string(dds_new_multicast_locator);
    std::shared_ptr<Locator> new_multicast_locator =
            std::make_shared<Locator>(new_multicast_locator_name);

    // Precondition: The database returns EntityIDs that are not used before
    database.set_next_entity_id(next_entity_id);

    // Start building the discovered reader info
    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(2);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(2);

    // Precondition: The discovered reader is in the participant
    data.guid = reader_guid_;

    // Precondition: The discovered reader is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The discovered reader contains the locators
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);
    data.remote_locators.add_unicast_locator(dds_new_unicast_locator);
    data.remote_locators.add_multicast_locator(dds_existing_multicast_locator);
    data.remote_locators.add_multicast_locator(dds_new_multicast_locator);



    // Precondition: The Reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The DataReader is added to the database and given ID 11.
    InsertEndpointArgs insert_datareader_args([&](
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::
                        rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)
            {
                EXPECT_EQ(endpoint_guid, reader_guid_str_);
                EXPECT_EQ(name, std::string("DataReader_") + topic_->name + "_" + reader_entity_id_str_);
                EXPECT_EQ(alias, "");
                EXPECT_EQ(qos, reader_proxy_data_to_backend_qos(data));
                EXPECT_EQ(is_virtual_metatraffic, false);

                EXPECT_EQ(locators.unicast[0], data.remote_locators.unicast[0]);
                EXPECT_EQ(locators.multicast[0], data.remote_locators.multicast[0]);
                EXPECT_EQ(locators.unicast[1], data.remote_locators.unicast[1]);
                EXPECT_EQ(locators.multicast[1], data.remote_locators.multicast[1]);

                EXPECT_EQ(kind, EntityKind::DATAREADER);
                EXPECT_EQ(participant_id, EntityId(1));
                EXPECT_EQ(topic_id, EntityId(2));

                static_cast<void>(app_data);

                return EntityId(11);
            });

    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_datareader_args, &InsertEndpointArgs::insert));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Precondition: The reader change it status
    EXPECT_CALL(database, change_entity_status(EntityId(11), true)).Times(1);

    // Expectation: Modify graph and notify user (twice, also from participant update)
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(1), EntityId(2), EntityId(11))).Times(1).WillOnce(Return(
                true));
    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(11), EntityKind::DATAREADER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    eprosima::fastdds::rtps::ReaderDiscoveryStatus status =
            eprosima::fastdds::rtps::ReaderDiscoveryStatus::DISCOVERED_READER;
    participant_listener.on_data_reader_discovery(&statistics_participant, status, data, should_be_ignored);
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_reader_several_locators_no_host)
{
    std::vector<std::shared_ptr<const Entity>> existing_locators;
    int64_t next_entity_id = 100;

    // Precondition: The Participant exists and has ID 1
    // Precondition: The Participant is NOT linked to any host
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, is_topic_in_database(_, EntityId(2))).Times(1)
            .WillOnce(Return(true));

    // Precondition: One unicast Locator exists and has ID 3
    eprosima::fastdds::
            rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    existing_locators.push_back(existing_unicast_locator);

    // Precondition: One multicast Locator exists and has ID 4
    eprosima::fastdds::
            rtps::Locator_t dds_existing_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_multicast_locator.address[12] = 127;
    dds_existing_multicast_locator.address[15] = 2;
    std::string existing_multicast_locator_name = to_string(dds_existing_multicast_locator);
    std::shared_ptr<Locator> existing_multicast_locator =
            std::make_shared<Locator>(existing_multicast_locator_name);
    existing_multicast_locator->id = 4;
    existing_locators.push_back(existing_multicast_locator);

    // Precondition: One unicast Locator does not exist
    eprosima::fastdds::
            rtps::Locator_t dds_new_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_new_unicast_locator.address[12] = 127;
    dds_new_unicast_locator.address[15] = 3;
    std::string new_unicast_locator_name = to_string(dds_new_unicast_locator);
    std::shared_ptr<Locator> new_unicast_locator =
            std::make_shared<Locator>(new_unicast_locator_name);

    // Precondition: One multicast Locator does not exist
    eprosima::fastdds::
            rtps::Locator_t dds_new_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_new_multicast_locator.address[12] = 127;
    dds_new_multicast_locator.address[15] = 4;
    std::string new_multicast_locator_name = to_string(dds_new_multicast_locator);
    std::shared_ptr<Locator> new_multicast_locator =
            std::make_shared<Locator>(new_multicast_locator_name);

    // Precondition: The database returns EntityIDs that are not used before
    database.set_next_entity_id(next_entity_id);

    // Start building the discovered reader info
    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(2);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(2);

    // Precondition: The discovered reader is in the participant
    data.guid = reader_guid_;

    // Precondition: The discovered reader is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The discovered reader contains the locators
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);
    data.remote_locators.add_unicast_locator(dds_new_unicast_locator);
    data.remote_locators.add_multicast_locator(dds_existing_multicast_locator);
    data.remote_locators.add_multicast_locator(dds_new_multicast_locator);



    // Precondition: The Reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The DataReader is added to the database and given ID 11.
    InsertEndpointArgs insert_datareader_args([&](
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::
                        rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)
            {
                EXPECT_EQ(endpoint_guid, reader_guid_str_);
                EXPECT_EQ(name, std::string("DataReader_") + topic_->name + "_" + reader_entity_id_str_);
                EXPECT_EQ(alias, "");
                EXPECT_EQ(qos, reader_proxy_data_to_backend_qos(data));
                EXPECT_EQ(is_virtual_metatraffic, false);

                EXPECT_EQ(locators.unicast[0], data.remote_locators.unicast[0]);
                EXPECT_EQ(locators.multicast[0], data.remote_locators.multicast[0]);
                EXPECT_EQ(locators.unicast[1], data.remote_locators.unicast[1]);
                EXPECT_EQ(locators.multicast[1], data.remote_locators.multicast[1]);

                EXPECT_EQ(kind, EntityKind::DATAREADER);
                EXPECT_EQ(participant_id, EntityId(1));
                EXPECT_EQ(topic_id, EntityId(2));

                static_cast<void>(app_data);

                return EntityId(11);
            });

    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_datareader_args, &InsertEndpointArgs::insert));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Precondition: The reader change it status
    EXPECT_CALL(database, change_entity_status(EntityId(11), true)).Times(1);

    // Expectation: Modify graph and notify user (twice, also from participant update)
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(1), EntityId(2), EntityId(11))).Times(1).WillOnce(Return(
                true));
    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(11), EntityKind::DATAREADER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    eprosima::fastdds::rtps::ReaderDiscoveryStatus status =
            eprosima::fastdds::rtps::ReaderDiscoveryStatus::DISCOVERED_READER;
    participant_listener.on_data_reader_discovery(&statistics_participant, status, data, should_be_ignored);
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_reader_no_participant)
{
    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastdds::
            rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Start building the discovered reader info
    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);

    // Precondition: The discovered reader is in the participant
    data.guid = reader_guid_;

    // Precondition: The discovered reader is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The discovered reader contains the locator
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Precondition: The Reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: No entity is added to the database
    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

    // Expectation: Nothing is inserted
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    eprosima::fastdds::rtps::ReaderDiscoveryStatus status =
            eprosima::fastdds::rtps::ReaderDiscoveryStatus::DISCOVERED_READER;
    participant_listener.on_data_reader_discovery(&statistics_participant, status, data, should_be_ignored);
}

TEST_F(statistics_participant_listener_tests, new_reader_no_domain)
{
    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The Topic does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastdds::
            rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Start building the discovered reader info
    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);

    // Precondition: The discovered reader is in the participant
    data.guid = reader_guid_;

    // Precondition: The discovered reader is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The Reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The discovered reader contains the locator
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Expectation: No entity is added to the database
    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

    // Expectation: Exception thrown
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    eprosima::fastdds::rtps::ReaderDiscoveryStatus status =
            eprosima::fastdds::rtps::ReaderDiscoveryStatus::DISCOVERED_READER;
    participant_listener.on_data_reader_discovery(&statistics_participant, status, data, should_be_ignored);
}

TEST_F(statistics_participant_listener_tests, new_reader_discovered_reader_already_exists)
{
    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, is_topic_in_database(_, EntityId(2))).Times(1)
            .WillOnce(Return(true));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastdds::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Start building the discovered reader info
    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);

    // Precondition: The discovered reader is in the participant
    data.guid = reader_guid_;

    // Precondition: The discovered reader is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The discovered reader contains the locator
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Precondition: The Reader exists and has ID 10
    std::shared_ptr<DataReader> reader =
            std::make_shared<DataReader>(reader_guid_str_, reader_proxy_data_to_backend_qos(
                        data), reader_guid_str_, participant_, topic_);
    reader->id = 10;
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(10))));

    // Expectation: The DataReader is not inserted in the database.
    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

    // Expectation: The DataReader status is set to active
    EXPECT_CALL(database, change_entity_status(EntityId(10), true)).Times(1);

    // Expectation: Modify graph and notify user (twice, also from participant update)
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(1), EntityId(2),
            EntityId(10))).Times(1).WillOnce(Return(false));

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::DATAREADER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener.
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    eprosima::fastdds::rtps::ReaderDiscoveryStatus status =
            eprosima::fastdds::rtps::ReaderDiscoveryStatus::DISCOVERED_READER;
    participant_listener.on_data_reader_discovery(&statistics_participant, status, data, should_be_ignored);
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_reader_undiscovered_reader_already_exists)
{
    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, is_topic_in_database(_, EntityId(2))).Times(1)
            .WillOnce(Return(true));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastdds::
            rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Start building the discovered reader info
    eprosima::fastdds::rtps::SubscriptionBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);

    // Precondition: The discovered reader is in the participant
    data.guid = reader_guid_;

    // Precondition: The discovered reader is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The discovered reader contains the locator
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Precondition: The Reader exists and has ID 10
    std::shared_ptr<DataReader> reader =
            std::make_shared<DataReader>(reader_guid_str_, reader_proxy_data_to_backend_qos(
                        data), reader_guid_str_, participant_, topic_);
    reader->id = 10;
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(10))));

    // Expectation: The DataReader is not inserted in the database.
    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

    // Expectation: The DataReader status is set to inactive
    EXPECT_CALL(database, change_entity_status(EntityId(10), false)).Times(1);

    // Expectation: Modify graph and notify user (twice, also from participant update)
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(1), EntityId(2),
            EntityId(10))).Times(1).WillOnce(Return(false));

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::DATAREADER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY)).Times(1);

    // Execution: Call the listener.
    eprosima::fastdds::rtps::ReaderDiscoveryStatus status =
            eprosima::fastdds::rtps::ReaderDiscoveryStatus::REMOVED_READER;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_data_reader_discovery(&statistics_participant, status, data, should_be_ignored);

    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_writer_discovered)
{
    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, is_topic_in_database(_, EntityId(2))).Times(1)
            .WillOnce(Return(true));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastdds::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Start building the discovered writer info
    eprosima::fastdds::rtps::PublicationBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);

    // Precondition: The discovered writer is in the participant
    data.guid = writer_guid_;

    // Precondition: The discovered writer is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The discovered writer contains the locator
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The DataWriter is added to the database. We do not care about the given ID
    InsertEndpointArgs insert_datawriter_args([&](
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)
            {
                EXPECT_EQ(endpoint_guid, writer_guid_str_);
                EXPECT_EQ(name, std::string("DataWriter_") + topic_->name + "_" + writer_entity_id_str_);
                EXPECT_EQ(alias, "");
                EXPECT_EQ(qos, writer_proxy_data_to_backend_qos(data));
                EXPECT_EQ(is_virtual_metatraffic, false);
                EXPECT_EQ(locators.unicast[0], data.remote_locators.unicast[0]);

                EXPECT_EQ(kind, EntityKind::DATAWRITER);
                EXPECT_EQ(participant_id, EntityId(1));
                EXPECT_EQ(topic_id, EntityId(2));

                static_cast<void>(app_data);

                return EntityId(10);
            });

    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEndpointArgs::insert));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Precondition: The writer change it status
    EXPECT_CALL(database, change_entity_status(EntityId(10), true)).Times(1);

    // Expectation: Modify graph and notify user
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(1), EntityId(2), EntityId(10))).Times(1).WillOnce(Return(
                true));
    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    eprosima::fastdds::rtps::WriterDiscoveryStatus status =
            eprosima::fastdds::rtps::WriterDiscoveryStatus::DISCOVERED_WRITER;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_data_writer_discovery(&statistics_participant, status, data, should_be_ignored);
}

TEST_F(statistics_participant_listener_tests, new_writer_undiscovered)
{
    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastdds::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Start building the discovered writer info
    eprosima::fastdds::rtps::PublicationBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);

    // Precondition: The discovered writer is in the participant
    data.guid = writer_guid_;

    // Precondition: The discovered writer is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The discovered writer contains the locator
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The writer is not added to the database.
    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

    // Precondition: The writer does not change it status
    EXPECT_CALL(database, change_entity_status(_, _)).Times(0);

    // Execution: Call the listener
    eprosima::fastdds::rtps::WriterDiscoveryStatus status =
            eprosima::fastdds::rtps::WriterDiscoveryStatus::REMOVED_WRITER;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_data_writer_discovery(&statistics_participant, status, data, should_be_ignored);
}

TEST_F(statistics_participant_listener_tests, new_writer_no_topic)
{
    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The Topic does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastdds::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Start building the discovered writer info
    eprosima::fastdds::rtps::PublicationBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);

    // Precondition: The discovered writer is in the participant
    data.guid = writer_guid_;

    // Precondition: The discovered writer is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The discovered writer contains the locator
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The Topic is added to the database. We do not care about the given ID
    InsertTopicArgs insert_topic_args([&](
                const std::string& name,
                const std::string& type_name,
                const std::string& alias,
                const EntityId& domain_id)
            {
                EXPECT_EQ(name, topic_name_);
                EXPECT_EQ(type_name, type_name_);
                EXPECT_EQ(alias, "");
                EXPECT_EQ(domain_id, EntityId(0));

                return EntityId(10);
            });

    EXPECT_CALL(database, insert_new_topic(_, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_topic_args, &InsertTopicArgs::insert));

    // Expectation: The DataWriter is added to the database. We do not care about the given ID
    InsertEndpointArgs insert_datawriter_args([&](
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)
            {
                EXPECT_EQ(endpoint_guid, writer_guid_str_);
                EXPECT_EQ(name, std::string("DataWriter_") + topic_->name + "_" + writer_entity_id_str_);
                EXPECT_EQ(alias, "");
                EXPECT_EQ(qos, writer_proxy_data_to_backend_qos(data));
                EXPECT_EQ(is_virtual_metatraffic, false);

                EXPECT_EQ(locators.unicast[0], data.remote_locators.unicast[0]);

                EXPECT_EQ(kind, EntityKind::DATAWRITER);
                EXPECT_EQ(participant_id, EntityId(1));
                EXPECT_EQ(topic_id, EntityId(10));

                static_cast<void>(app_data);

                return EntityId(11);
            });

    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEndpointArgs::insert));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Precondition: The writer change it status
    EXPECT_CALL(database, change_entity_status(EntityId(11), true)).Times(1);

    // Expectation: Modify graph and notify user (twice, also from participant update)
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(1), EntityId(10),
            EntityId(11))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(1);

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
    eprosima::fastdds::rtps::WriterDiscoveryStatus status =
            eprosima::fastdds::rtps::WriterDiscoveryStatus::DISCOVERED_WRITER;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_data_writer_discovery(&statistics_participant, status, data, should_be_ignored);
}

TEST_F(statistics_participant_listener_tests, new_writer_several_locators)
{
    std::vector<std::shared_ptr<const Entity>> existing_locators;
    int64_t next_entity_id = 100;

    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, is_topic_in_database(_, EntityId(2))).Times(1)
            .WillOnce(Return(true));

    // Precondition: One unicast Locator exists and has ID 3
    eprosima::fastdds::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    existing_locators.push_back(existing_unicast_locator);

    // Precondition: One multicast Locator exists and has ID 4
    eprosima::fastdds::rtps::Locator_t dds_existing_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_multicast_locator.address[12] = 127;
    dds_existing_multicast_locator.address[15] = 2;
    std::string existing_multicast_locator_name = to_string(dds_existing_multicast_locator);
    std::shared_ptr<Locator> existing_multicast_locator =
            std::make_shared<Locator>(existing_multicast_locator_name);
    existing_multicast_locator->id = 4;
    existing_locators.push_back(existing_multicast_locator);

    // Precondition: One unicast Locator does not exist
    eprosima::fastdds::rtps::Locator_t dds_new_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_new_unicast_locator.address[12] = 127;
    dds_new_unicast_locator.address[15] = 3;
    std::string new_unicast_locator_name = to_string(dds_new_unicast_locator);
    std::shared_ptr<Locator> new_unicast_locator =
            std::make_shared<Locator>(new_unicast_locator_name);

    // Precondition: One multicast Locator does not exist
    eprosima::fastdds::rtps::Locator_t dds_new_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_new_multicast_locator.address[12] = 127;
    dds_new_multicast_locator.address[15] = 4;
    std::string new_multicast_locator_name = to_string(dds_new_multicast_locator);
    std::shared_ptr<Locator> new_multicast_locator =
            std::make_shared<Locator>(new_multicast_locator_name);

    // Precondition: The database returns EntityIDs that are not used before
    database.set_next_entity_id(next_entity_id);

    // Start building the discovered writer info
    eprosima::fastdds::rtps::PublicationBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(2);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(2);

    // Precondition: The discovered writer is in the participant
    data.guid = writer_guid_;

    // Precondition: The discovered writer is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The discovered writer contains the locators
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);
    data.remote_locators.add_unicast_locator(dds_new_unicast_locator);
    data.remote_locators.add_multicast_locator(dds_existing_multicast_locator);
    data.remote_locators.add_multicast_locator(dds_new_multicast_locator);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));


    // Expectation: The DataWriter is added to the database and given ID 11
    InsertEndpointArgs insert_datawriter_args([&](
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)
            {
                EXPECT_EQ(endpoint_guid, writer_guid_str_);
                EXPECT_EQ(name, std::string("DataWriter_") + topic_->name + "_" + writer_entity_id_str_);
                EXPECT_EQ(alias, "");
                EXPECT_EQ(qos, writer_proxy_data_to_backend_qos(data));
                EXPECT_EQ(is_virtual_metatraffic, false);

                EXPECT_EQ(locators.unicast[0], data.remote_locators.unicast[0]);
                EXPECT_EQ(locators.multicast[0], data.remote_locators.multicast[0]);
                EXPECT_EQ(locators.unicast[1], data.remote_locators.unicast[1]);
                EXPECT_EQ(locators.multicast[1], data.remote_locators.multicast[1]);

                EXPECT_EQ(kind, EntityKind::DATAWRITER);
                EXPECT_EQ(participant_id, EntityId(1));
                EXPECT_EQ(topic_id, EntityId(2));

                static_cast<void>(app_data);

                return EntityId(11);
            });

    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEndpointArgs::insert));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Precondition: The writer change it status
    EXPECT_CALL(database, change_entity_status(EntityId(11), true)).Times(1);

    // Expectation: Modify graph and notify user (twice, also from participant update)
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(1), EntityId(2), EntityId(11))).Times(1).WillOnce(Return(
                true));
    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(11), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    eprosima::fastdds::rtps::WriterDiscoveryStatus status =
            eprosima::fastdds::rtps::WriterDiscoveryStatus::DISCOVERED_WRITER;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_data_writer_discovery(&statistics_participant, status, data, should_be_ignored);
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_writer_several_locators_no_host)
{
    std::vector<std::shared_ptr<const Entity>> existing_locators;
    int64_t next_entity_id = 100;

    // Precondition: The Participant exists and has ID 1
    // Precondition: The Participant is NOT linked to any host
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, is_topic_in_database(_, EntityId(2))).Times(1)
            .WillOnce(Return(true));

    // Precondition: One unicast Locator exists and has ID 3
    eprosima::fastdds::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;
    existing_locators.push_back(existing_unicast_locator);

    // Precondition: One multicast Locator exists and has ID 4
    eprosima::fastdds::rtps::Locator_t dds_existing_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_multicast_locator.address[12] = 127;
    dds_existing_multicast_locator.address[15] = 2;
    std::string existing_multicast_locator_name = to_string(dds_existing_multicast_locator);
    std::shared_ptr<Locator> existing_multicast_locator =
            std::make_shared<Locator>(existing_multicast_locator_name);
    existing_multicast_locator->id = 4;
    existing_locators.push_back(existing_multicast_locator);

    // Precondition: One unicast Locator does not exist
    eprosima::fastdds::rtps::Locator_t dds_new_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_new_unicast_locator.address[12] = 127;
    dds_new_unicast_locator.address[15] = 3;
    std::string new_unicast_locator_name = to_string(dds_new_unicast_locator);
    std::shared_ptr<Locator> new_unicast_locator =
            std::make_shared<Locator>(new_unicast_locator_name);

    // Precondition: One multicast Locator does not exist
    eprosima::fastdds::rtps::Locator_t dds_new_multicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_new_multicast_locator.address[12] = 127;
    dds_new_multicast_locator.address[15] = 4;
    std::string new_multicast_locator_name = to_string(dds_new_multicast_locator);
    std::shared_ptr<Locator> new_multicast_locator =
            std::make_shared<Locator>(new_multicast_locator_name);

    // Precondition: The database returns EntityIDs that are not used before
    database.set_next_entity_id(next_entity_id);

    // Start building the discovered writer info
    eprosima::fastdds::rtps::PublicationBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(2);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(2);

    // Precondition: The discovered writer is in the participant
    data.guid = writer_guid_;

    // Precondition: The discovered writer is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The discovered writer contains the locators
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);
    data.remote_locators.add_unicast_locator(dds_new_unicast_locator);
    data.remote_locators.add_multicast_locator(dds_existing_multicast_locator);
    data.remote_locators.add_multicast_locator(dds_new_multicast_locator);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: The DataWriter is added to the database and given ID 11
    InsertEndpointArgs insert_datawriter_args([&](
                const std::string& endpoint_guid,
                const std::string& name,
                const std::string& alias,
                const Qos& qos,
                const bool& is_virtual_metatraffic,
                const eprosima::fastdds::rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)
            {
                EXPECT_EQ(endpoint_guid, writer_guid_str_);
                EXPECT_EQ(name, std::string("DataWriter_") + topic_->name + "_" + writer_entity_id_str_);
                EXPECT_EQ(alias, "");
                EXPECT_EQ(qos, writer_proxy_data_to_backend_qos(data));
                EXPECT_EQ(is_virtual_metatraffic, false);

                EXPECT_EQ(locators.unicast[0], data.remote_locators.unicast[0]);
                EXPECT_EQ(locators.multicast[0], data.remote_locators.multicast[0]);
                EXPECT_EQ(locators.unicast[1], data.remote_locators.unicast[1]);
                EXPECT_EQ(locators.multicast[1], data.remote_locators.multicast[1]);

                EXPECT_EQ(kind, EntityKind::DATAWRITER);
                EXPECT_EQ(participant_id, EntityId(1));
                EXPECT_EQ(topic_id, EntityId(2));

                static_cast<void>(app_data);

                return EntityId(11);
            });

    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&insert_datawriter_args, &InsertEndpointArgs::insert));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Precondition: The writer change it status
    EXPECT_CALL(database, change_entity_status(EntityId(11), true)).Times(1);

    // Expectation: Modify graph and notify user (twice, also from participant update)
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(1), EntityId(2), EntityId(11))).Times(1).WillOnce(Return(
                true));
    EXPECT_CALL(*eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
            on_domain_view_graph_update(EntityId(0))).Times(1);

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(11), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener
    eprosima::fastdds::rtps::WriterDiscoveryStatus status =
            eprosima::fastdds::rtps::WriterDiscoveryStatus::DISCOVERED_WRITER;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_data_writer_discovery(&statistics_participant, status, data, should_be_ignored);
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_writer_no_participant)
{
    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastdds::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Start building the discovered writer info
    eprosima::fastdds::rtps::PublicationBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);

    // Precondition: The discovered writer is in the participant
    data.guid = writer_guid_;

    // Precondition: The discovered writer is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The discovered writer contains the locator
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Expectation: No entity is added to the database
    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

    // Expectation: Nothing is inserted
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    eprosima::fastdds::rtps::WriterDiscoveryStatus status =
            eprosima::fastdds::rtps::WriterDiscoveryStatus::DISCOVERED_WRITER;
    participant_listener.on_data_writer_discovery(&statistics_participant, status, data, should_be_ignored);

}

TEST_F(statistics_participant_listener_tests, new_writer_no_domain)
{
    // Precondition: The Participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The Topic does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastdds::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Start building the discovered writer info
    eprosima::fastdds::rtps::PublicationBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);

    // Precondition: The discovered writer is in the participant
    data.guid = writer_guid_;

    // Precondition: The discovered writer is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Throw(eprosima::statistics_backend::BadParameter("Error")));

    // Precondition: The discovered writer contains the locator
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Expectation: No entity is added to the database
    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

    // Expectation: Exception thrown
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    eprosima::fastdds::rtps::WriterDiscoveryStatus status =
            eprosima::fastdds::rtps::WriterDiscoveryStatus::DISCOVERED_WRITER;
    participant_listener.on_data_writer_discovery(&statistics_participant, status, data, should_be_ignored);
}

TEST_F(statistics_participant_listener_tests, new_writer_discovered_writer_already_exists)
{
    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, is_topic_in_database(_, EntityId(2))).Times(1)
            .WillOnce(Return(true));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastdds::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Start building the discovered writer info
    eprosima::fastdds::rtps::PublicationBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);

    // Precondition: The discovered writer is in the participant
    data.guid = writer_guid_;

    // Precondition: The discovered writer is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The discovered writer contains the locator
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Precondition: The writer exists and has ID 10
    std::shared_ptr<DataWriter> writer =
            std::make_shared<DataWriter>(writer_guid_str_, writer_proxy_data_to_backend_qos(
                        data), writer_guid_str_, participant_, topic_);
    writer->id = EntityId(10);
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(10))));

    // Expectation: The DataWriter is not inserted in the database.
    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

    // Expectation: The DataWriter status is set to active
    EXPECT_CALL(database, change_entity_status(EntityId(10), true)).Times(1);

    // Expectation: Modify graph and notify user (twice, also from participant update)
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(1), EntityId(2),
            EntityId(10))).Times(1).WillOnce(Return(false));

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Execution: Call the listener.
    eprosima::fastdds::rtps::WriterDiscoveryStatus status =
            eprosima::fastdds::rtps::WriterDiscoveryStatus::DISCOVERED_WRITER;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_data_writer_discovery(&statistics_participant, status, data, should_be_ignored);
    entity_queue.flush();
}

TEST_F(statistics_participant_listener_tests, new_writer_undiscovered_writer_already_exists)
{
    // Precondition: The Participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The Topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, is_topic_in_database(_, EntityId(2))).Times(1)
            .WillOnce(Return(true));

    // Precondition: The Locator exists and has ID 3
    eprosima::fastdds::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    std::string existing_unicast_locator_name = to_string(dds_existing_unicast_locator);
    std::shared_ptr<Locator> existing_unicast_locator =
            std::make_shared<Locator>(existing_unicast_locator_name);
    existing_unicast_locator->id = 3;

    // Start building the discovered writer info
    eprosima::fastdds::rtps::PublicationBuiltinTopicData data;

    // Set max number of unicast/multicast locators
    data.remote_locators.unicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);
    data.remote_locators.multicast = eprosima::fastdds::ResourceLimitedContainerConfig::fixed_size_configuration(1);

    // Precondition: The discovered writer is in the participant
    data.guid = writer_guid_;

    // Precondition: The discovered writer is in the topic
    data.topic_name = topic_name_;
    data.type_name = type_name_;

    // Precondition: The discovered writer contains the locator
    data.remote_locators.add_unicast_locator(dds_existing_unicast_locator);

    // Precondition: The writer exists and has ID 10
    std::shared_ptr<DataWriter> writer =
            std::make_shared<DataWriter>(writer_guid_str_, writer_proxy_data_to_backend_qos(
                        data), writer_guid_str_, participant_, topic_);
    writer->id = EntityId(10);
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str_)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(10))));

    // Expectation: The DataWriter is not inserted in the database.
    EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

    // Expectation: The DataWriter status is set to active
    EXPECT_CALL(database, change_entity_status(EntityId(10), false)).Times(1);

    // Expectation: Modify graph and notify user (twice, also from participant update)
    EXPECT_CALL(database,
            update_endpoint_in_graph(EntityId(0), EntityId(1), EntityId(2),
            EntityId(10))).Times(1).WillOnce(Return(false));

    // Expectation: The user listener is called
    EXPECT_CALL(
        *eprosima::statistics_backend::details::StatisticsBackendData::get_instance(),
        on_domain_entity_discovery(EntityId(0), EntityId(10), EntityKind::DATAWRITER,
        eprosima::statistics_backend::details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY)).Times(1);

    // Execution: Call the listener.
    eprosima::fastdds::rtps::WriterDiscoveryStatus status =
            eprosima::fastdds::rtps::WriterDiscoveryStatus::REMOVED_WRITER;
    bool should_be_ignored = false; // Set to false to avoid ignoring the entity
    participant_listener.on_data_writer_discovery(&statistics_participant, status, data, should_be_ignored);
    entity_queue.flush();
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
