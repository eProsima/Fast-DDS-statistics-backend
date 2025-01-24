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

#include <iostream>
#include <functional>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <fastdds/utils/IPLocator.hpp>

#include <database/database.hpp>
#include <database/database_queue.hpp>

#include <fastdds_statistics_backend/topic_types/monitorservice_types.hpp>
#include <fastdds_statistics_backend/topic_types/types.hpp>

using namespace eprosima::fastdds::statistics;
using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;
using namespace eprosima::fastdds::rtps;

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Throw;
using ::testing::AnyNumber;
using ::testing::StrictMock;

// Wrapper class to expose the internal attributes of the queue
class DatabaseEntityQueueWrapper : public DatabaseEntityQueue
{

public:

    DatabaseEntityQueueWrapper(
            Database* database)
        : DatabaseEntityQueue(database)
    {
    }

    const std::queue<queue_item_type> get_foreground_queue()
    {
        return *foreground_queue_;
    }

    const std::queue<queue_item_type> get_background_queue()
    {
        return *background_queue_;
    }

    void do_swap()
    {
        swap();
    }

    /**
     * @brief Processes one sample and removes it from the front queue
     *
     * This is necessary to check exception handling on the consumer
     * Consumers must be stopped and the queues swapped manually
     *
     * @return true if anything was consumed
     */
    bool consume_sample()
    {
        if (empty())
        {
            return false;
        }

        process_sample();
        pop();
        return true;
    }

};

// Wrapper class to expose the internal attributes of the queue
class DatabaseDataQueueWrapper : public DatabaseDataQueue<eprosima::fastdds::statistics::Data>
{

public:

    DatabaseDataQueueWrapper(
            Database* database)
        : DatabaseDataQueue<eprosima::fastdds::statistics::Data>(database)
    {
    }

    const std::queue<queue_item_type> get_foreground_queue()
    {
        return *foreground_queue_;
    }

    const std::queue<queue_item_type> get_background_queue()
    {
        return *background_queue_;
    }

    void do_swap()
    {
        swap();
    }

    /**
     * @brief Processes one sample and removes it from the front queue
     *
     * This is necessary to check exception handling on the consumer
     * Consumers must be stopped and the queues swapped manually
     *
     * @return true if anything was consumed
     */
    bool consume_sample()
    {
        if (empty())
        {
            return false;
        }

        process_sample();
        pop();
        return true;
    }

    void do_process_sample_type(
            EntityId& domain,
            EntityId& entity,
            EntityKind entity_kind,
            ByteToLocatorCountSample& sample,
            const StatisticsEntity2LocatorTraffic& item) const
    {
        process_sample_type<ByteToLocatorCountSample, StatisticsEntity2LocatorTraffic>(
            domain, entity, entity_kind, sample, item);
    }

};

// Wrapper class to expose the internal attributes of the queue
class DatabaseMonitorDataQueueWrapper : public DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>
{

public:

    DatabaseMonitorDataQueueWrapper(
            Database* database)
        : DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>(database)
    {
    }

    const std::queue<queue_item_type> get_foreground_queue()
    {
        return *foreground_queue_;
    }

    const std::queue<queue_item_type> get_background_queue()
    {
        return *background_queue_;
    }

    void do_swap()
    {
        swap();
    }

    /**
     * @brief Processes one sample and removes it from the front queue
     *
     * This is necessary to check exception handling on the consumer
     * Consumers must be stopped and the queues swapped manually
     *
     * @return true if anything was consumed
     */
    bool consume_sample()
    {
        if (empty())
        {
            return false;
        }

        process_sample();
        pop();
        return true;
    }

};

struct InsertDataArgs
{
    InsertDataArgs (
            std::function<void(
                const EntityId&,
                const EntityId&,
                const StatisticsSample&)> func)
        : callback_(func)
    {
    }

    void insert(
            const EntityId& domain_id,
            const EntityId& id,
            const StatisticsSample& sample)
    {
        return callback_(domain_id, id, sample);
    }

    std::function<void(
                const EntityId&,
                const EntityId&,
                const StatisticsSample&)> callback_;
};

struct InsertMonitorServiceDataArgs
{
    InsertMonitorServiceDataArgs (
            std::function<bool(
                const EntityId&,
                const EntityId&,
                const eprosima::statistics_backend::MonitorServiceSample&)> func)
        : callback_(func)
    {
    }

    bool insert(
            const EntityId& domain_id,
            const EntityId& id,
            const eprosima::statistics_backend::MonitorServiceSample& sample)
    {
        return callback_(domain_id, id, sample);
    }

    std::function<bool(
                const EntityId&,
                const EntityId&,
                const eprosima::statistics_backend::MonitorServiceSample&)> callback_;
};

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
                const eprosima::fastdds::rtps::RemoteLocatorList& locators,
                const EntityKind& kind,
                const EntityId& participant_id,
                const EntityId& topic_id,
                const std::pair<AppId, std::string> app_data)> callback_;

    std::string endpoint_guid_;
    std::string name_;
    std::string alias_;
    Qos qos_;
    bool is_virtual_metatraffic_;
    eprosima::fastdds::rtps::RemoteLocatorList locators_;
    EntityKind kind_;
    EntityId participant_id_;
    EntityId topic_id_;
    std::pair<AppId, std::string> app_data_;
};

class database_queue_tests : public ::testing::Test
{

public:

    StrictMock<Database> database;
    DatabaseEntityQueueWrapper entity_queue;
    DatabaseDataQueueWrapper data_queue;
    DatabaseMonitorDataQueueWrapper monitor_data_queue;

    database_queue_tests()
        : entity_queue(&database)
        , data_queue(&database)
        , monitor_data_queue(&database)
    {
    }

};

/* The start/stop/flush operations are common to all queues, and can be tested on a generic one */
class IntegerQueue : public DatabaseQueue<int>
{

public:

    IntegerQueue()
        : DatabaseQueue<int>()
    {
    }

    virtual ~IntegerQueue()
    {
        stop_consumer();
    }

    MOCK_METHOD0(process_sample, void());

    const std::queue<queue_item_type> get_foreground_queue()
    {
        return *foreground_queue_;
    }

    const std::queue<queue_item_type> get_background_queue()
    {
        return *background_queue_;
    }

};

TEST_F(database_queue_tests, start_stop_flush)
{
    IntegerQueue int_queue;
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Add something to the stopped queue
    EXPECT_CALL(int_queue, process_sample()).Times(0);
    EXPECT_TRUE(int_queue.stop_consumer());
    int_queue.push(timestamp, 1);
    int_queue.push(timestamp, 2);

    EXPECT_TRUE(int_queue.get_foreground_queue().empty());
    EXPECT_EQ(2u, int_queue.get_background_queue().size());

    // Flushing a stopped queue does nothing
    EXPECT_CALL(int_queue, process_sample()).Times(0);
    int_queue.flush();
    EXPECT_TRUE(int_queue.get_foreground_queue().empty());
    EXPECT_EQ(2u, int_queue.get_background_queue().size());

    // Start the queue and flush
    EXPECT_CALL(int_queue, process_sample()).Times(2);
    EXPECT_TRUE(int_queue.start_consumer());
    int_queue.flush();
    EXPECT_TRUE(int_queue.get_foreground_queue().empty());
    EXPECT_TRUE(int_queue.get_background_queue().empty());

    // Start the consumer when it is already started
    EXPECT_CALL(int_queue, process_sample()).Times(0);
    EXPECT_FALSE(int_queue.start_consumer());
    EXPECT_TRUE(int_queue.get_foreground_queue().empty());
    EXPECT_TRUE(int_queue.get_background_queue().empty());

    // Flush on an empty queue with running consumer
    EXPECT_CALL(int_queue, process_sample()).Times(0);
    int_queue.flush();
    EXPECT_TRUE(int_queue.get_foreground_queue().empty());
    EXPECT_TRUE(int_queue.get_background_queue().empty());

    // Flush on an empty queue with a stopped consumer
    EXPECT_TRUE(int_queue.stop_consumer());
    EXPECT_CALL(int_queue, process_sample()).Times(0);
    int_queue.flush();
    EXPECT_TRUE(int_queue.get_foreground_queue().empty());
    EXPECT_TRUE(int_queue.get_background_queue().empty());

    // Stop the consumer when it is already stopped
    EXPECT_CALL(int_queue, process_sample()).Times(0);
    EXPECT_FALSE(int_queue.stop_consumer());
    EXPECT_TRUE(int_queue.get_foreground_queue().empty());
    EXPECT_TRUE(int_queue.get_background_queue().empty());

    // Start the consumer with an empty queue
    EXPECT_CALL(int_queue, process_sample()).Times(0);
    EXPECT_TRUE(int_queue.start_consumer());
    EXPECT_TRUE(int_queue.get_foreground_queue().empty());
    EXPECT_TRUE(int_queue.get_background_queue().empty());
}

TEST_F(database_queue_tests, push_participant)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Create the participant info
    std::string participant_name = "participant name";
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    Qos participant_qos;
    std::string address = "127.0.0.1";

    std::string processname = "1234";
    std::string pid = processname;
    std::string username = "user";
    std::string hostname = "host";

    // Build the process name
    std::stringstream ss;
    ss << processname << ":" << pid;
    std::string processname_pid = ss.str();

    EntityDiscoveryInfo info(EntityKind::PARTICIPANT);
    info.domain_id = EntityId(0);
    std::stringstream(participant_guid_str) >> info.guid;
    info.qos = participant_qos;
    info.address = address;
    info.participant_name = participant_name;
    info.host = hostname;
    info.user = username;
    info.process = processname_pid;
    info.app_id = AppId::UNKNOWN;
    info.entity_status = StatusLevel::OK_STATUS;

    // Participant undiscovery: FAILURE
    {
        // Precondition: The participant does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(0);

        // Expectations: The status will throw an exception because the participant is not in the database
        EXPECT_CALL(database, change_entity_status(_, false)).Times(AnyNumber())
                .WillRepeatedly(Throw(BadParameter("Error")));

        // Expectations: No notification to user
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_entity_discovery(_, _, _, _)).Times(0);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
    // Participant update: FAILURE
    {
        // Precondition: The participant does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(0);

        // Expectations: The status will throw an exception because the participant is not in the database
        EXPECT_CALL(database, change_entity_status(_, true)).Times(AnyNumber())
                .WillRepeatedly(Throw(BadParameter("Error")));

        // Expectations: No notification to user
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_entity_discovery(_, _, _, _)).Times(0);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UPDATE;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
    // Participant discovery: SUCCESS
    {
        // Precondition: The participant does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        // Expectation: The participant is created and given ID 1
        InsertParticipantArgs insert_args([&](
                    const std::string& name,
                    const Qos& qos,
                    const std::string& guid,
                    const EntityId& domain_id,
                    const StatusLevel& status,
                    const AppId& app_id,
                    const std::string& app_metadata)
                {
                    EXPECT_EQ(name, participant_name);
                    EXPECT_EQ(qos, participant_qos);
                    EXPECT_EQ(guid, participant_guid_str);
                    EXPECT_EQ(domain_id, EntityId(0));
                    EXPECT_EQ(status, StatusLevel::OK_STATUS);
                    EXPECT_EQ(app_id, AppId::UNKNOWN);
                    EXPECT_EQ(app_metadata, "");

                    return EntityId(1);
                });

        EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(1)
                .WillOnce(Invoke(&insert_args, &InsertParticipantArgs::insert));

        // Precondition: Host-user-process exist
        ProcessPhysicalArgs process_physical_args([&](
                    const std::string& host_name,
                    const std::string& user_name,
                    const std::string& process_name,
                    const std::string& process_pid,
                    bool& should_link_process_participant,
                    const EntityId& participant_id,
                    std::map<std::string, EntityId>& physical_entities_ids)
                {
                    EXPECT_EQ(host_name, hostname);
                    EXPECT_EQ(user_name, username);
                    EXPECT_EQ(process_name, processname);
                    EXPECT_EQ(process_pid, pid);
                    EXPECT_EQ(should_link_process_participant, true);
                    EXPECT_EQ(participant_id, EntityId(1));

                    physical_entities_ids[HOST_ENTITY_TAG] = EntityId(2);
                    physical_entities_ids[USER_ENTITY_TAG] = EntityId(3);
                    physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId(4);

                });

        EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
                .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database,
                update_participant_in_graph(EntityId(0), EntityId(2), EntityId(3), EntityId(4),
                EntityId(1))).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(EntityId(0))).Times(1);

        // Expectations: Request the backend to notify user (if needed)
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_domain_entity_discovery(EntityId(0),  EntityId(1), EntityKind::PARTICIPANT,
                details::StatisticsBackendData::DISCOVERY)).Times(1);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
    // Participant update: SUCCESS
    {
        // Precondition: The participant exists and has ID 1
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
                .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

        EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(0);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(1), true)).Times(1);

        // Precondition: Host-user-process exist
        ProcessPhysicalArgs process_physical_args([&](
                    const std::string& host_name,
                    const std::string& user_name,
                    const std::string& process_name,
                    const std::string& process_pid,
                    bool& should_link_process_participant,
                    const EntityId& participant_id,
                    std::map<std::string, EntityId>& physical_entities_ids)
                {
                    EXPECT_EQ(host_name, hostname);
                    EXPECT_EQ(user_name, username);
                    EXPECT_EQ(process_name, processname);
                    EXPECT_EQ(process_pid, pid);
                    EXPECT_EQ(should_link_process_participant, false);
                    EXPECT_EQ(participant_id, EntityId(1));

                    physical_entities_ids[HOST_ENTITY_TAG] = EntityId(2);
                    physical_entities_ids[USER_ENTITY_TAG] = EntityId(3);
                    physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId(4);

                });

        EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
                .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));

        // Expectation: Do not modify graph nor notify user
        EXPECT_CALL(database,
                update_participant_in_graph(EntityId(0), EntityId(2), EntityId(3), EntityId(4),
                EntityId(1))).Times(1).WillOnce(Return(false));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(EntityId(0))).Times(0);

        // Expectations: Request the backend to notify user (if needed)
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_domain_entity_discovery(EntityId(0), EntityId(1), EntityKind::PARTICIPANT,
                details::StatisticsBackendData::UPDATE)).Times(1);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UPDATE;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
    // Participant undiscovery: SUCCESS
    {
        // Precondition: The participant exists and has ID 1
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
                .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

        EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(0);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(1), false)).Times(1);

        // Precondition: Host-user-process exist
        ProcessPhysicalArgs process_physical_args([&](
                    const std::string& host_name,
                    const std::string& user_name,
                    const std::string& process_name,
                    const std::string& process_pid,
                    bool& should_link_process_participant,
                    const EntityId& participant_id,
                    std::map<std::string, EntityId>& physical_entities_ids)
                {
                    EXPECT_EQ(host_name, hostname);
                    EXPECT_EQ(user_name, username);
                    EXPECT_EQ(process_name, processname);
                    EXPECT_EQ(process_pid, pid);
                    EXPECT_EQ(should_link_process_participant, false);
                    EXPECT_EQ(participant_id, EntityId(1));

                    physical_entities_ids[HOST_ENTITY_TAG] = EntityId(2);
                    physical_entities_ids[USER_ENTITY_TAG] = EntityId(3);
                    physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId(4);

                });

        EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
                .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database,
                update_participant_in_graph(EntityId(0), EntityId(2), EntityId(3), EntityId(4),
                EntityId(1))).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(EntityId(0))).Times(1);

        // Expectations: Request the backend to notify user (if needed)
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_domain_entity_discovery(EntityId(0), EntityId(1), EntityKind::PARTICIPANT,
                details::StatisticsBackendData::UNDISCOVERY)).Times(1);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
    // Participant discovery: THROWS
    {
        // Precondition: The participant does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        // Expectation: The participant creation throws
        InsertParticipantArgs insert_args([&](
                    const std::string& name,
                    const Qos& qos,
                    const std::string& guid,
                    const EntityId& domain_id,
                    const StatusLevel& status,
                    const AppId& app_id,
                    const std::string& app_metadata) -> EntityId
                {
                    EXPECT_EQ(name, participant_name);
                    EXPECT_EQ(qos, participant_qos);
                    EXPECT_EQ(guid, participant_guid_str);
                    EXPECT_EQ(domain_id, EntityId(0));
                    EXPECT_EQ(status, StatusLevel::OK_STATUS);
                    EXPECT_EQ(app_id, AppId::UNKNOWN);
                    EXPECT_EQ(app_metadata, "");

                    throw BadParameter("Error");
                });

        EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(1)
                .WillOnce(Invoke(&insert_args, &InsertParticipantArgs::insert));

        // Expectations: No notification to user
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_entity_discovery(_, _, _, _)).Times(0);

        // Add to the queue and wait to be processed
        entity_queue.stop_consumer();
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
        entity_queue.push(timestamp, info);
        entity_queue.do_swap();

        EXPECT_NO_THROW(entity_queue.consume_sample());
        entity_queue.start_consumer();
    }
}

TEST_F(database_queue_tests, push_participant_participant_exists)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Create the participant info
    std::string participant_name = "participant name";
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    Qos participant_qos;
    std::string address = "127.0.0.1";

    std::string processname = "1234";
    std::string pid = processname;
    std::string username = "user";
    std::string hostname = "host";

    EntityDiscoveryInfo info(EntityKind::PARTICIPANT);
    info.domain_id = EntityId(0);
    std::stringstream(participant_guid_str) >> info.guid;
    info.qos = participant_qos;
    info.address = address;
    info.participant_name = participant_name;
    info.host = hostname;
    info.user = username;
    info.process = processname;

    {
        // Precondition: The participant exists
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
                .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));
        EXPECT_CALL(database, change_entity_status(EntityId(1), true)).Times(1);
        EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(0);

        // Precondition: Host-user-process exist
        ProcessPhysicalArgs process_physical_args([&](
                    const std::string& host_name,
                    const std::string& user_name,
                    const std::string& process_name,
                    const std::string& process_pid,
                    bool& should_link_process_participant,
                    const EntityId& participant_id,
                    std::map<std::string, EntityId>& physical_entities_ids)
                {
                    EXPECT_EQ(host_name, hostname);
                    EXPECT_EQ(user_name, username);
                    EXPECT_EQ(process_name, processname);
                    EXPECT_EQ(process_pid, pid);
                    EXPECT_EQ(should_link_process_participant, false);
                    EXPECT_EQ(participant_id, EntityId(1));

                    physical_entities_ids[HOST_ENTITY_TAG] = EntityId(2);
                    physical_entities_ids[USER_ENTITY_TAG] = EntityId(3);
                    physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId(4);

                });

        EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
                .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database,
                update_participant_in_graph(EntityId(0), EntityId(2), EntityId(3), EntityId(4),
                EntityId(1))).Times(1).WillOnce(Return(false));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(EntityId(0))).Times(0);

        // Expectations: Request the backend to notify user (if needed)
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_domain_entity_discovery(EntityId(0),  EntityId(1), EntityKind::PARTICIPANT,
                details::StatisticsBackendData::DISCOVERY)).Times(1);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
}

TEST_F(database_queue_tests, push_participant_missing_physical_entity)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Create the participant info
    std::string participant_name = "participant name";
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    Qos participant_qos;
    std::string address = "127.0.0.1";

    std::string processname = "1234";
    std::string pid = processname;
    std::string username = "user";
    std::string hostname = "host";

    // Build the process name
    std::stringstream ss;
    ss << processname << ":" << pid;
    std::string processname_pid = ss.str();

    EntityDiscoveryInfo info(EntityKind::PARTICIPANT);
    info.domain_id = EntityId(0);
    std::stringstream(participant_guid_str) >> info.guid;
    info.qos = participant_qos;
    info.address = address;
    info.participant_name = participant_name;
    info.host = hostname;
    info.user = username;
    info.process = processname_pid;

    // Precondition: The participant exists
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, change_entity_status(EntityId(1), true)).Times(1);
    EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(0);

    // Expectation: Host or user or process are created
    ProcessPhysicalArgs process_physical_args([&](
                const std::string& host_name,
                const std::string& user_name,
                const std::string& process_name,
                const std::string& process_pid,
                bool& should_link_process_participant,
                const EntityId& participant_id,
                std::map<std::string, EntityId>& physical_entities_ids)
            {
                EXPECT_EQ(host_name, hostname);
                EXPECT_EQ(user_name, username);
                EXPECT_EQ(process_name, processname);
                EXPECT_EQ(process_pid, pid);
                EXPECT_EQ(should_link_process_participant, false);
                EXPECT_EQ(participant_id, EntityId(1));

                physical_entities_ids[HOST_ENTITY_TAG] = EntityId(2);
                physical_entities_ids[USER_ENTITY_TAG] = EntityId(3);
                physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId(4);

            });

    EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));

    // Expectation: Modify graph and notify user
    EXPECT_CALL(database,
            update_participant_in_graph(EntityId(0), EntityId(2), EntityId(3), EntityId(4),
            EntityId(1))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(EntityId(0))).Times(1);

    // Expectations: Request the backend to notify user (if needed)
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_domain_entity_discovery(EntityId(0),  EntityId(1), EntityKind::PARTICIPANT,
            details::StatisticsBackendData::DISCOVERY)).Times(1);

    // Add to the queue and wait to be processed
    info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
    entity_queue.push(timestamp, info);
    entity_queue.flush();
}

TEST_F(database_queue_tests, push_participant_process_insert_throws)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Create the participant info
    std::string participant_name = "participant name";
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    Qos participant_qos;
    std::string address = "127.0.0.1";

    std::string processname = "1234";
    std::string pid = processname;
    std::string username = "user";
    std::string hostname = "host";

    // Build the process name
    std::stringstream ss;
    ss << processname << ":" << pid;
    std::string processname_pid = ss.str();

    EntityDiscoveryInfo info(EntityKind::PARTICIPANT);
    info.domain_id = EntityId(0);
    std::stringstream(participant_guid_str) >> info.guid;
    info.qos = participant_qos;
    info.address = address;
    info.participant_name = participant_name;
    info.host = hostname;
    info.user = username;
    info.process = processname_pid;

    // Precondition: The participant exists
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, change_entity_status(EntityId(1), true)).Times(1);
    EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(0);

    // Expectation: The process creation throws
    ProcessPhysicalArgs process_physical_args([&](
                const std::string& host_name,
                const std::string& user_name,
                const std::string& process_name,
                const std::string& process_pid,
                bool& should_link_process_participant,
                const EntityId& participant_id,
                std::map<std::string, EntityId>& physical_entities_ids)
            {
                EXPECT_EQ(host_name, hostname);
                EXPECT_EQ(user_name, username);
                EXPECT_EQ(process_name, processname);
                EXPECT_EQ(process_pid, pid);
                EXPECT_EQ(should_link_process_participant, false);
                EXPECT_EQ(participant_id, EntityId(1));

                physical_entities_ids[HOST_ENTITY_TAG] = EntityId(2);
                physical_entities_ids[USER_ENTITY_TAG] = EntityId(3);

                throw BadParameter("Error");

            });

    EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));

    // Expectation: Do not dodify graph nor notify user
    EXPECT_CALL(database,
            update_participant_in_graph(EntityId(0), EntityId(2), EntityId(3), EntityId(),
            EntityId(1))).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(_)).Times(0);

    // Expectations: Request the backend to notify user (if needed)
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_domain_entity_discovery(EntityId(0),  EntityId(1), EntityKind::PARTICIPANT,
            details::StatisticsBackendData::DISCOVERY)).Times(1);

    // Add to the queue and wait to be processed
    info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
    entity_queue.push(timestamp, info);
    entity_queue.flush();
}

TEST_F(database_queue_tests, push_participant_user_insert_throws)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Create the participant info
    std::string participant_name = "participant name";
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    Qos participant_qos;
    std::string address = "127.0.0.1";

    std::string processname = "1234";
    std::string pid = processname;
    std::string username = "user";
    std::string hostname = "host";

    // Build the process name
    std::stringstream ss;
    ss << processname << ":" << pid;
    std::string processname_pid = ss.str();

    EntityDiscoveryInfo info(EntityKind::PARTICIPANT);
    info.domain_id = EntityId(0);
    std::stringstream(participant_guid_str) >> info.guid;
    info.qos = participant_qos;
    info.address = address;
    info.participant_name = participant_name;
    info.host = hostname;
    info.user = username;
    info.process = processname_pid;

    // Precondition: The participant exists
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, change_entity_status(EntityId(1), true)).Times(1);
    EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(0);

    // Expectation: The host creation throws
    ProcessPhysicalArgs process_physical_args([&](
                const std::string& host_name,
                const std::string& user_name,
                const std::string& process_name,
                const std::string& process_pid,
                bool& should_link_process_participant,
                const EntityId& participant_id,
                std::map<std::string, EntityId>& physical_entities_ids)
            {
                EXPECT_EQ(host_name, hostname);
                EXPECT_EQ(user_name, username);
                EXPECT_EQ(process_name, processname);
                EXPECT_EQ(process_pid, pid);
                EXPECT_EQ(should_link_process_participant, false);
                EXPECT_EQ(participant_id, EntityId(1));

                physical_entities_ids[HOST_ENTITY_TAG] = EntityId(2);

                throw BadParameter("Error");

            });

    EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));

    // Expectation: Do not modify graph nor notify user
    EXPECT_CALL(database,
            update_participant_in_graph(EntityId(0), EntityId(2), EntityId(), EntityId(),
            EntityId(1))).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(_)).Times(0);

    // Expectations: Request the backend to notify user (if needed)
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_domain_entity_discovery(EntityId(0),  EntityId(1), EntityKind::PARTICIPANT,
            details::StatisticsBackendData::DISCOVERY)).Times(1);

    // Add to the queue and wait to be processed
    info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
    entity_queue.push(timestamp, info);
    entity_queue.flush();
}

TEST_F(database_queue_tests, push_participant_host_insert_throws)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Create the participant info
    std::string participant_name = "participant name";
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    Qos participant_qos;
    std::string address = "127.0.0.1";

    std::string processname = "1234";
    std::string pid = processname;
    std::string username = "user";
    std::string hostname = "host";

    // Build the process name
    std::stringstream ss;
    ss << processname << ":" << pid;
    std::string processname_pid = ss.str();

    EntityDiscoveryInfo info(EntityKind::PARTICIPANT);
    info.domain_id = EntityId(0);
    std::stringstream(participant_guid_str) >> info.guid;
    info.qos = participant_qos;
    info.address = address;
    info.participant_name = participant_name;
    info.host = hostname;
    info.user = username;
    info.process = processname_pid;

    // Precondition: The participant exists
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, change_entity_status(EntityId(1), true)).Times(1);
    EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(0);

    // Expectation: The host creation throws
    ProcessPhysicalArgs process_physical_args([&](
                const std::string& host_name,
                const std::string& user_name,
                const std::string& process_name,
                const std::string& process_pid,
                bool& should_link_process_participant,
                const EntityId& participant_id,
                std::map<std::string, EntityId>& physical_entities_ids)
            {
                EXPECT_EQ(host_name, hostname);
                EXPECT_EQ(user_name, username);
                EXPECT_EQ(process_name, processname);
                EXPECT_EQ(process_pid, pid);
                EXPECT_EQ(should_link_process_participant, false);
                EXPECT_EQ(participant_id, EntityId(1));

                static_cast<void>(physical_entities_ids);

                throw BadParameter("Error");

            });

    EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));


    // Expectation: Do not modify graph nor notify user
    EXPECT_CALL(database,
            update_participant_in_graph(EntityId(0), EntityId(), EntityId(), EntityId(),
            EntityId(1))).Times(1).WillOnce(Return(false));
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(_)).Times(0);

    // Expectations: Request the backend to notify user (if needed)
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_domain_entity_discovery(EntityId(0),  EntityId(1), EntityKind::PARTICIPANT,
            details::StatisticsBackendData::DISCOVERY)).Times(1);

    // Add to the queue and wait to be processed
    info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
    entity_queue.push(timestamp, info);
    entity_queue.flush();
}

TEST_F(database_queue_tests, push_participant_data_wrong_processname_format)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Create the participant info
    std::string participant_name = "participant name";
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    Qos participant_qos;
    std::string address = "127.0.0.1";

    std::string processname = "1234";
    std::string pid = processname;
    std::string username = "user";
    std::string hostname = "host";

    // Build the process name
    std::string processname_pid = processname;

    EntityDiscoveryInfo info(EntityKind::PARTICIPANT);
    info.domain_id = EntityId(0);
    std::stringstream(participant_guid_str) >> info.guid;
    info.qos = participant_qos;
    info.address = address;
    info.participant_name = participant_name;
    info.host = hostname;
    info.user = username;
    info.process = processname_pid;

    // Precondition: The participant exists
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, change_entity_status(EntityId(1), true)).Times(1);
    EXPECT_CALL(database, insert_new_participant(_, _, _, _, _, _, _)).Times(0);

    // Expectation: The process is created and given ID 4
    ProcessPhysicalArgs process_physical_args([&](
                const std::string& host_name,
                const std::string& user_name,
                const std::string& process_name,
                const std::string& process_pid,
                bool& should_link_process_participant,
                const EntityId& participant_id,
                std::map<std::string, EntityId>& physical_entities_ids)
            {
                EXPECT_EQ(host_name, hostname);
                EXPECT_EQ(user_name, username);
                EXPECT_EQ(process_name, processname);
                EXPECT_EQ(process_pid, pid);
                EXPECT_EQ(should_link_process_participant, false);
                EXPECT_EQ(participant_id, EntityId(1));

                physical_entities_ids[HOST_ENTITY_TAG] = EntityId(2);
                physical_entities_ids[USER_ENTITY_TAG] = EntityId(3);
                physical_entities_ids[PROCESS_ENTITY_TAG] = EntityId(4);

            });

    EXPECT_CALL(database, process_physical_entities(_, _, _, _, _, _, _)).Times(1)
            .WillOnce(Invoke(&process_physical_args, &ProcessPhysicalArgs::process));

    // Expectation: Modify graph and notify user
    EXPECT_CALL(database,
            update_participant_in_graph(EntityId(0), EntityId(2), EntityId(3), EntityId(4),
            EntityId(1))).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(EntityId(0))).Times(1);

    // Expectations: Request the backend to notify user (if needed)
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_domain_entity_discovery(EntityId(0),  EntityId(1), EntityKind::PARTICIPANT,
            details::StatisticsBackendData::DISCOVERY)).Times(1);

    // Add to the queue and wait to be processed
    info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
    entity_queue.push(timestamp, info);
    entity_queue.flush();
}

TEST_F(database_queue_tests, push_datawriter)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Create the writer info
    std::string datawriter_name = "topic_name_0.0.0.1";  //< Name constructed from the topic and entity_id
    Qos datawriter_qos;
    std::string datawriter_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    std::string topic_name = "topic_name";
    std::string type_name = "type_name";
    std::string unicast_locator_str = "UDPv4:[127.0.0.1]:1024";
    std::string multicast_locator_str = "UDPv4:[239.1.1.1]:1024";

    EntityDiscoveryInfo info(EntityKind::DATAWRITER);
    info.domain_id = EntityId(0);
    std::stringstream(datawriter_guid_str) >> info.guid;
    info.qos = datawriter_qos;
    info.topic_name = topic_name;
    info.type_name = type_name;
    Locator_t unicast_locator;
    std::stringstream(unicast_locator_str) >> unicast_locator;
    info.locators.add_unicast_locator(unicast_locator);
    Locator_t multicast_locator;
    std::stringstream(multicast_locator_str) >> multicast_locator;
    info.locators.add_multicast_locator(multicast_locator);

    // Precondition: The participant exists and has ID 1
    std::string participant_name = "participant";
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(2);

    // Precondition: The topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, is_topic_in_database(_, EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(true));
    EXPECT_CALL(database, insert_new_type_idl(type_name, "")).Times(AnyNumber());

    // Datawriter undiscovery: FAILURE
    {
        // Precondition: The writer does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, datawriter_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

        EXPECT_CALL(database, change_entity_status(_, false)).Times(0);

        // Expectations: No notification to user
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_entity_discovery(_, _, _, _)).Times(0);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
    // Datawriter update: FAILURE
    {
        // Precondition: The writer does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, datawriter_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

        EXPECT_CALL(database, change_entity_status(_, true)).Times(0);

        // Expectations: No notification to user
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_entity_discovery(_, _, _, _)).Times(0);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UPDATE;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
    // Datawriter discovery: SUCCESS
    {
        // Precondition: The writer does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, datawriter_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        // Expectation: The datawriter is created and given ID 3
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
                    EXPECT_EQ(endpoint_guid, datawriter_guid_str);
                    EXPECT_EQ(name, datawriter_name);
                    EXPECT_EQ(alias, "");
                    EXPECT_EQ(qos, datawriter_qos);
                    EXPECT_EQ(is_virtual_metatraffic, false);
                    EXPECT_EQ(locators.unicast[0], info.locators.unicast[0]);
                    EXPECT_EQ(locators.multicast[0], info.locators.multicast[0]);

                    EXPECT_EQ(kind, EntityKind::DATAWRITER);
                    EXPECT_EQ(participant_id, EntityId(1));
                    EXPECT_EQ(topic_id, EntityId(2));

                    static_cast<void>(app_data);

                    return EntityId(3);
                });

        EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
                .WillOnce(Invoke(&insert_datawriter_args, &InsertEndpointArgs::insert));

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, update_endpoint_in_graph(_, _, _, _)).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(_)).Times(1);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(3), true)).Times(1);

        // Expectations: Request the backend to notify user (if needed)
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_domain_entity_discovery(EntityId(0), EntityId(3), EntityKind::DATAWRITER,
                details::StatisticsBackendData::DISCOVERY)).Times(1);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
    // Datawriter update: SUCCESS
    {
        // Precondition: The writer exists and has ID 3
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, datawriter_guid_str)).Times(1)
                .WillOnce(Return(std::make_pair(EntityId(0), EntityId(3))));

        EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(3), true)).Times(1);

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, update_endpoint_in_graph(_, _, _, _)).Times(1).WillOnce(Return(false));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(_)).Times(0);

        // Expectations: Request the backend to notify user (if needed)
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_domain_entity_discovery(EntityId(0), EntityId(3), EntityKind::DATAWRITER,
                details::StatisticsBackendData::UPDATE)).Times(1);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UPDATE;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
    // Datawriter undiscovery: SUCCESS
    {
        // Precondition: The writer exists and has ID 3
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, datawriter_guid_str)).Times(1)
                .WillOnce(Return(std::make_pair(EntityId(0), EntityId(3))));

        EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(3), false)).Times(1);

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, update_endpoint_in_graph(_, _, _, _)).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(EntityId(0))).Times(1);

        // Expectations: Request the backend to notify user (if needed)
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_domain_entity_discovery(EntityId(0), EntityId(3), EntityKind::DATAWRITER,
                details::StatisticsBackendData::UNDISCOVERY)).Times(1);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
    // Datawriter undiscovery: THROWS
    {
        // Precondition: The writer does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, datawriter_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        // Expectation: The writer creation throws
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
                    const std::pair<AppId, std::string> app_data) -> EntityId
                {
                    EXPECT_EQ(endpoint_guid, datawriter_guid_str);
                    EXPECT_EQ(name, datawriter_name);
                    EXPECT_EQ(alias, "");
                    EXPECT_EQ(qos, datawriter_qos);
                    EXPECT_EQ(is_virtual_metatraffic, false);
                    EXPECT_EQ(locators.unicast[0], info.locators.unicast[0]);
                    EXPECT_EQ(locators.multicast[0], info.locators.multicast[0]);
                    EXPECT_EQ(kind, EntityKind::DATAWRITER);
                    EXPECT_EQ(participant_id, EntityId(1));
                    EXPECT_EQ(topic_id, EntityId(2));

                    static_cast<void>(app_data);

                    throw BadParameter("Error");
                });

        EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
                .WillOnce(Invoke(&insert_datawriter_args, &InsertEndpointArgs::insert));

        // Expectations: No notification to user
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_entity_discovery(_, _, _, _)).Times(0);

        // Add to the queue and wait to be processed
        entity_queue.stop_consumer();
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
        entity_queue.push(timestamp, info);
        entity_queue.do_swap();

        EXPECT_NO_THROW(entity_queue.consume_sample());
        entity_queue.start_consumer();
    }
}

TEST_F(database_queue_tests, push_datawriter_topic_does_not_exist)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Create the writer info
    std::string datawriter_name = "topic_name_0.0.0.1";  //< Name constructed from the topic and entity_id
    Qos datawriter_qos;
    std::string datawriter_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    std::string topic_name = "topic_name";
    std::string type_name = "type_name";
    std::string unicast_locator_str = "UDPv4:[127.0.0.1]:1024";
    std::string multicast_locator_str = "UDPv4:[239.1.1.1]:1024";

    EntityDiscoveryInfo info(EntityKind::DATAWRITER);
    info.domain_id = EntityId(0);
    std::stringstream(datawriter_guid_str) >> info.guid;
    info.qos = datawriter_qos;
    info.topic_name = topic_name;
    info.type_name = type_name;
    Locator_t unicast_locator;
    std::stringstream(unicast_locator_str) >> unicast_locator;
    info.locators.add_unicast_locator(unicast_locator);
    Locator_t multicast_locator;
    std::stringstream(multicast_locator_str) >> multicast_locator;
    info.locators.add_multicast_locator(multicast_locator);

    // Precondition: The participant exists and has ID 1
    std::string participant_name = "participant";
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Precondition: The topic does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()));

    EXPECT_CALL(database, is_type_in_database(type_name)).Times(AnyNumber())
            .WillRepeatedly(Return(false));

    // Datawriter discovery: SUCCESS
    {
        // Precondition: The writer does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, datawriter_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        // Expectation: The topic is created and given ID 2
        InsertTopicArgs insert_topic_args([&](
                    const std::string& name,
                    const std::string& type_name,
                    const std::string& alias,
                    const EntityId& domain_id)
                {
                    EXPECT_EQ(name, topic_name);
                    EXPECT_EQ(type_name, type_name);
                    EXPECT_EQ(alias, "");
                    EXPECT_EQ(domain_id, EntityId(0));

                    return EntityId(2);
                });

        EXPECT_CALL(database, insert_new_topic(_, _, _, _)).Times(1)
                .WillOnce(Invoke(&insert_topic_args, &InsertTopicArgs::insert));

        // Expectation: The datawriter is created and given ID 3
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
                    EXPECT_EQ(endpoint_guid, datawriter_guid_str);
                    EXPECT_EQ(name, datawriter_name);
                    EXPECT_EQ(alias, "");
                    EXPECT_EQ(qos, datawriter_qos);
                    EXPECT_EQ(is_virtual_metatraffic, false);
                    EXPECT_EQ(locators.unicast[0], info.locators.unicast[0]);
                    EXPECT_EQ(locators.multicast[0], info.locators.multicast[0]);
                    EXPECT_EQ(kind, EntityKind::DATAWRITER);
                    EXPECT_EQ(participant_id, EntityId(1));
                    EXPECT_EQ(topic_id, EntityId(2));

                    static_cast<void>(app_data);

                    return EntityId(3);
                });

        EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
                .WillOnce(Invoke(&insert_datawriter_args, &InsertEndpointArgs::insert));

        // Expectation: Add the type to the database
        EXPECT_CALL(database, insert_new_type_idl(type_name, "")).Times(1);

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, update_endpoint_in_graph(_, _, _, _)).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(_)).Times(1);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(3), true)).Times(1);

        // Expectations: Request the backend to notify user (if needed)
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_domain_entity_discovery(EntityId(0), EntityId(2), EntityKind::TOPIC,
                details::StatisticsBackendData::DISCOVERY))
                .Times(1);

        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_domain_entity_discovery(EntityId(0), EntityId(3), EntityKind::DATAWRITER,
                details::StatisticsBackendData::DISCOVERY))
                .Times(1);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
}

TEST_F(database_queue_tests, push_datareader)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Create the reader info
    std::string datareader_name = "topic_name_0.0.0.2";  //< Name constructed from the topic and entity_id
    Qos datareader_qos;
    std::string datareader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    std::string topic_name = "topic_name";
    std::string type_name = "type_name";
    std::string unicast_locator_str = "UDPv4:[127.0.0.1]:1024";
    std::string multicast_locator_str = "UDPv4:[239.1.1.1]:1024";

    EntityDiscoveryInfo info(EntityKind::DATAREADER);
    info.domain_id = EntityId(0);
    std::stringstream(datareader_guid_str) >> info.guid;
    info.qos = datareader_qos;
    info.topic_name = topic_name;
    info.type_name = type_name;
    Locator_t unicast_locator;
    std::stringstream(unicast_locator_str) >> unicast_locator;
    info.locators.add_unicast_locator(unicast_locator);
    Locator_t multicast_locator;
    std::stringstream(multicast_locator_str) >> multicast_locator;
    info.locators.add_multicast_locator(multicast_locator);

    // Precondition: The participant exists and has ID 1
    std::string participant_name = "participant";
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(2);

    // Precondition: The topic exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, is_topic_in_database(_, EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(true));
    EXPECT_CALL(database, insert_new_type_idl(type_name, "")).Times(AnyNumber());

    // Datareader undiscovery: FAILURE
    {
        // Precondition: The reader does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, datareader_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

        EXPECT_CALL(database, change_entity_status(_, false)).Times(0);

        // Expectations: No notification to user
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_entity_discovery(_, _, _, _)).Times(0);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
    // Datareader update: FAILURE
    {
        // Precondition: The reader does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, datareader_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

        // Expectations: The status will throw an exception because the datareader is not in the database
        EXPECT_CALL(database, change_entity_status(_, true)).Times(0);

        // Expectations: No notification to user
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_entity_discovery(_, _, _, _)).Times(0);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UPDATE;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
    // Datareader discovery: SUCCESS
    {
        // Precondition: The reader does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, datareader_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        // Expectation: The datareader is created and given ID 3
        InsertEndpointArgs insert_datareader_args([&](
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
                    EXPECT_EQ(endpoint_guid, datareader_guid_str);
                    EXPECT_EQ(name, datareader_name);
                    EXPECT_EQ(alias, "");
                    EXPECT_EQ(qos, datareader_qos);
                    EXPECT_EQ(is_virtual_metatraffic, false);
                    EXPECT_EQ(locators.unicast[0], info.locators.unicast[0]);
                    EXPECT_EQ(locators.multicast[0], info.locators.multicast[0]);
                    EXPECT_EQ(kind, EntityKind::DATAREADER);
                    EXPECT_EQ(participant_id, EntityId(1));
                    EXPECT_EQ(topic_id, EntityId(2));

                    static_cast<void>(app_data);

                    return EntityId(3);
                });

        EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
                .WillOnce(Invoke(&insert_datareader_args, &InsertEndpointArgs::insert));

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, update_endpoint_in_graph(_, _, _, _)).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(_)).Times(1);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(3), true)).Times(1);

        // Expectations: Request the backend to notify user (if needed)
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_domain_entity_discovery(EntityId(0), EntityId(3), EntityKind::DATAREADER,
                details::StatisticsBackendData::DISCOVERY))
                .Times(1);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
    // Datareader update: SUCCESS
    {
        // Precondition: The reader exists and has ID 3
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, datareader_guid_str)).Times(1)
                .WillOnce(Return(std::make_pair(EntityId(0), EntityId(3))));

        EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(3), true)).Times(1);

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, update_endpoint_in_graph(_, _, _, _)).Times(1).WillOnce(Return(false));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(_)).Times(0);

        // Expectations: Request the backend to notify user (if needed)
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_domain_entity_discovery(EntityId(0), EntityId(3), EntityKind::DATAREADER,
                details::StatisticsBackendData::UPDATE)).Times(1);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UPDATE;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
    // Datareader undiscovery: SUCCESS
    {
        // Precondition: The reader exists and has ID 3
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, datareader_guid_str)).Times(1)
                .WillOnce(Return(std::make_pair(EntityId(0), EntityId(3))));

        EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(0);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(3), false)).Times(1);

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, update_endpoint_in_graph(_, _, _, _)).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(EntityId(0))).Times(1);

        // Expectations: Request the backend to notify user (if needed)
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_domain_entity_discovery(EntityId(0), EntityId(3), EntityKind::DATAREADER,
                details::StatisticsBackendData::UNDISCOVERY)).Times(1);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::UNDISCOVERY;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
    // Datareader undiscovery: THROWS
    {
        // Precondition: The reader does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, datareader_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        // Expectation: The reader creation throws
        InsertEndpointArgs insert_datareader_args([&](
                    const std::string& endpoint_guid,
                    const std::string& name,
                    const std::string& alias,
                    const Qos& qos,
                    const bool& is_virtual_metatraffic,
                    const eprosima::fastdds::rtps::RemoteLocatorList& locators,
                    const EntityKind& kind,
                    const EntityId& participant_id,
                    const EntityId& topic_id,
                    const std::pair<AppId, std::string> app_data) -> EntityId
                {
                    EXPECT_EQ(endpoint_guid, datareader_guid_str);
                    EXPECT_EQ(name, datareader_name);
                    EXPECT_EQ(alias, "");
                    EXPECT_EQ(qos, datareader_qos);
                    EXPECT_EQ(is_virtual_metatraffic, false);
                    EXPECT_EQ(locators.unicast[0], info.locators.unicast[0]);
                    EXPECT_EQ(locators.multicast[0], info.locators.multicast[0]);
                    EXPECT_EQ(kind, EntityKind::DATAREADER);
                    EXPECT_EQ(participant_id, EntityId(1));
                    EXPECT_EQ(topic_id, EntityId(2));

                    static_cast<void>(app_data);

                    throw BadParameter("Error");
                });

        EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
                .WillOnce(Invoke(&insert_datareader_args, &InsertEndpointArgs::insert));

        // Expectations: No notification to user
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_entity_discovery(_, _, _, _)).Times(0);

        // Add to the queue and wait to be processed
        entity_queue.stop_consumer();
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
        entity_queue.push(timestamp, info);
        entity_queue.do_swap();

        EXPECT_NO_THROW(entity_queue.consume_sample());
        entity_queue.start_consumer();
    }
}

TEST_F(database_queue_tests, push_datareader_topic_does_not_exist)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Create the reader info
    std::string datareader_name = "topic_name_0.0.0.2";  //< Name constructed from the topic and entity_id
    Qos datareader_qos;
    std::string datareader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    std::string topic_name = "topic_name";
    std::string type_name = "type_name";
    std::string unicast_locator_str = "UDPv4:[127.0.0.1]:1024";
    std::string multicast_locator_str = "UDPv4:[239.1.1.1]:1024";

    EntityDiscoveryInfo info(EntityKind::DATAREADER);
    info.domain_id = EntityId(0);
    std::stringstream(datareader_guid_str) >> info.guid;
    info.qos = datareader_qos;
    info.topic_name = topic_name;
    info.type_name = type_name;
    Locator_t unicast_locator;
    std::stringstream(unicast_locator_str) >> unicast_locator;
    info.locators.add_unicast_locator(unicast_locator);
    Locator_t multicast_locator;
    std::stringstream(multicast_locator_str) >> multicast_locator;
    info.locators.add_multicast_locator(multicast_locator);

    // Precondition: The participant exists and has ID 1
    std::string participant_name = "participant";
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    // TODO: Remove when endpoint gets app data from discovery info
    EXPECT_CALL(database, get_entity(_)).Times(1);

    // Precondition: The topic does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()));
    EXPECT_CALL(database, is_type_in_database(type_name)).Times(AnyNumber())
            .WillOnce(Return(false));

    // Datareader discovery: SUCCESS
    {
        // Precondition: The reader does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, datareader_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        // Expectation: The topic is created and given ID 2
        InsertTopicArgs insert_topic_args([&](
                    const std::string& name,
                    const std::string& type_name,
                    const std::string& alias,
                    const EntityId& domain_id)
                {
                    EXPECT_EQ(name, topic_name);
                    EXPECT_EQ(type_name, type_name);
                    EXPECT_EQ(alias, "");
                    EXPECT_EQ(domain_id, EntityId(0));

                    return EntityId(2);
                });

        EXPECT_CALL(database, insert_new_topic(_, _, _, _)).Times(1)
                .WillOnce(Invoke(&insert_topic_args, &InsertTopicArgs::insert));

        // Expectation: The datareader is created and given ID 3
        InsertEndpointArgs insert_datareader_args([&](
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
                    EXPECT_EQ(endpoint_guid, datareader_guid_str);
                    EXPECT_EQ(name, datareader_name);
                    EXPECT_EQ(alias, "");
                    EXPECT_EQ(qos, datareader_qos);
                    EXPECT_EQ(is_virtual_metatraffic, false);
                    EXPECT_EQ(locators.unicast[0], info.locators.unicast[0]);
                    EXPECT_EQ(locators.multicast[0], info.locators.multicast[0]);
                    EXPECT_EQ(kind, EntityKind::DATAREADER);
                    EXPECT_EQ(participant_id, EntityId(1));
                    EXPECT_EQ(topic_id, EntityId(2));

                    static_cast<void>(app_data);

                    return EntityId(3);
                });

        EXPECT_CALL(database, insert_new_endpoint(_, _, _, _, _, _, _, _, _, _)).Times(1)
                .WillOnce(Invoke(&insert_datareader_args, &InsertEndpointArgs::insert));

        // Expectation: Add the type to the database
        EXPECT_CALL(database, insert_new_type_idl(type_name, "")).Times(1);

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, update_endpoint_in_graph(_, _, _, _)).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(_)).Times(1);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(3), true)).Times(1);

        // Expectations: Request the backend to notify user (if needed)
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_domain_entity_discovery(EntityId(0), EntityId(2), EntityKind::TOPIC,
                details::StatisticsBackendData::DISCOVERY))
                .Times(1);

        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_domain_entity_discovery(EntityId(0), EntityId(3), EntityKind::DATAREADER,
                details::StatisticsBackendData::DISCOVERY))
                .Times(1);

        // Add to the queue and wait to be processed
        info.discovery_status = details::StatisticsBackendData::DiscoveryStatus::DISCOVERY;
        entity_queue.push(timestamp, info);
        entity_queue.flush();
    }
}

TEST_F(database_queue_tests, push_history_latency)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the reader GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueueWrapper::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsWriterReaderData inner_data;
    inner_data.data(1.0);
    inner_data.writer_guid(writer_guid);
    inner_data.reader_guid(reader_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->writer_reader_data(inner_data);
    data->_d(EventKind::HISTORY2HISTORY_LATENCY);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The reader exists and has ID 2
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(2))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::FASTDDS_LATENCY);
                EXPECT_EQ(dynamic_cast<const HistoryLatencySample&>(sample).reader, 2);
                EXPECT_EQ(dynamic_cast<const HistoryLatencySample&>(sample).data, 1.0);
            });

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::FASTDDS_LATENCY)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_history_latency_no_reader)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the reader GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueueWrapper::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsWriterReaderData inner_data;
    inner_data.data(1.0);
    inner_data.writer_guid(writer_guid);
    inner_data.reader_guid(reader_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->writer_reader_data(inner_data);
    data->_d(EventKind::HISTORY2HISTORY_LATENCY);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is not called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_history_latency_no_writer)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the reader GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueueWrapper::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsWriterReaderData inner_data;
    inner_data.data(1.0);
    inner_data.writer_guid(writer_guid);
    inner_data.reader_guid(reader_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->writer_reader_data(inner_data);
    data->_d(EventKind::HISTORY2HISTORY_LATENCY);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Precondition: The reader exists and has ID 2
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(2))));

    // Expectation: The insert method is not called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_network_latency)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 16> src_locator_address = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    eprosima::fastdds::rtps::Locator_t src_locator_t;
    uint16_t src_locator_t_physical_port = 0;
    uint16_t src_locator_t_logical_port = 0;
    IPLocator::setPhysicalPort(src_locator_t, src_locator_t_physical_port);
    IPLocator::setLogicalPort(src_locator_t, src_locator_t_logical_port);
    uint32_t src_locator_port = src_locator_t.port;
    std::string src_locator_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|d.e.f.10";

    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    eprosima::fastdds::rtps::Locator_t dst_locator_t;
    uint16_t dst_locator_t_physical_port = 2048;
    uint16_t dst_locator_t_logical_port = 2048;
    IPLocator::setPhysicalPort(dst_locator_t, dst_locator_t_physical_port);
    IPLocator::setLogicalPort(dst_locator_t, dst_locator_t_logical_port);
    uint32_t dst_locator_port = dst_locator_t.port;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:" + std::to_string(dst_locator_t_physical_port) + "-" +
            std::to_string(dst_locator_t_logical_port);

    // Build the source locator
    DatabaseDataQueueWrapper::StatisticsLocator src_locator;
    src_locator.kind(LOCATOR_KIND_TCPv4);
    src_locator.port(src_locator_port);
    src_locator.address(src_locator_address);

    // Build the destination locator
    DatabaseDataQueueWrapper::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsLocator2LocatorData inner_data;
    inner_data.data(1.0);
    inner_data.src_locator(src_locator);
    inner_data.dst_locator(dst_locator);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->locator2locator_data(inner_data);
    data->_d(EventKind::NETWORK_LATENCY);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, src_locator_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The destination locator exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(1)
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::NETWORK_LATENCY);
                EXPECT_EQ(dynamic_cast<const NetworkLatencySample&>(sample).remote_locator, 2);
            });

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::NETWORK_LATENCY)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_network_latency_no_participant)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 16> src_locator_address = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    eprosima::fastdds::rtps::Locator_t src_locator_t;
    uint16_t src_locator_t_physical_port = 0;
    uint16_t src_locator_t_logical_port = 0;
    IPLocator::setPhysicalPort(src_locator_t, src_locator_t_physical_port);
    IPLocator::setLogicalPort(src_locator_t, src_locator_t_logical_port);
    uint32_t src_locator_port = src_locator_t.port;
    std::string src_locator_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|d.e.f.10";

    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    eprosima::fastdds::rtps::Locator_t dst_locator_t;
    uint16_t dst_locator_t_physical_port = 2048;
    uint16_t dst_locator_t_logical_port = 0;
    IPLocator::setPhysicalPort(dst_locator_t, dst_locator_t_physical_port);
    IPLocator::setLogicalPort(dst_locator_t, dst_locator_t_logical_port);
    uint32_t dst_locator_port = dst_locator_t.port;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:" + std::to_string(dst_locator_t_physical_port) + "-" +
            std::to_string(dst_locator_t_logical_port);

    // Build the source locator
    DatabaseDataQueueWrapper::StatisticsLocator src_locator;
    src_locator.kind(LOCATOR_KIND_TCPv4);
    src_locator.port(src_locator_port);
    src_locator.address(src_locator_address);

    // Build the destination locator
    DatabaseDataQueueWrapper::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsLocator2LocatorData inner_data;
    inner_data.data(1.0);
    inner_data.src_locator(src_locator);
    inner_data.dst_locator(dst_locator);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->locator2locator_data(inner_data);
    data->_d(EventKind::NETWORK_LATENCY);

    // Precondition: The participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, src_locator_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Precondition: The destination locator exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_network_latency_wrong_participant_format)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 16> src_locator_address = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    eprosima::fastdds::rtps::Locator_t src_locator_t;
    uint16_t src_locator_t_physical_port = 1;
    uint16_t src_locator_t_logical_port = 0;
    IPLocator::setPhysicalPort(src_locator_t, src_locator_t_physical_port);
    IPLocator::setLogicalPort(src_locator_t, src_locator_t_logical_port);
    uint32_t src_locator_port = src_locator_t.port;
    std::string src_locator_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|d.e.f.10";

    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    eprosima::fastdds::rtps::Locator_t dst_locator_t;
    uint16_t dst_locator_t_physical_port = 2048;
    uint16_t dst_locator_t_logical_port = 0;
    IPLocator::setPhysicalPort(dst_locator_t, dst_locator_t_physical_port);
    IPLocator::setLogicalPort(dst_locator_t, dst_locator_t_logical_port);
    uint32_t dst_locator_port = dst_locator_t.port;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:" + std::to_string(dst_locator_t_physical_port) + "-" +
            std::to_string(dst_locator_t_logical_port);

    // Build the source locator
    DatabaseDataQueueWrapper::StatisticsLocator src_locator;
    src_locator.kind(LOCATOR_KIND_TCPv4);
    src_locator.port(src_locator_port);
    src_locator.address(src_locator_address);

    // Build the destination locator
    DatabaseDataQueueWrapper::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsLocator2LocatorData inner_data;
    inner_data.data(1.0);
    inner_data.src_locator(src_locator);
    inner_data.dst_locator(dst_locator);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->locator2locator_data(inner_data);
    data->_d(EventKind::NETWORK_LATENCY);

    // Precondition: The participant is not searched
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, src_locator_str)).Times(0);

    // Precondition: The destination locator exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_network_latency_no_destination_locator)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 16> src_locator_address = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    eprosima::fastdds::rtps::Locator_t src_locator_t;
    uint16_t src_locator_t_physical_port = 0;
    uint16_t src_locator_t_logical_port = 0;
    IPLocator::setPhysicalPort(src_locator_t, src_locator_t_physical_port);
    IPLocator::setLogicalPort(src_locator_t, src_locator_t_logical_port);
    uint32_t src_locator_port = src_locator_t.port;
    std::string src_locator_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|d.e.f.10";

    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    eprosima::fastdds::rtps::Locator_t dst_locator_t;
    uint16_t dst_locator_t_physical_port = 2048;
    uint16_t dst_locator_t_logical_port = 0;
    IPLocator::setPhysicalPort(dst_locator_t, dst_locator_t_physical_port);
    IPLocator::setLogicalPort(dst_locator_t, dst_locator_t_logical_port);
    uint32_t dst_locator_port = dst_locator_t.port;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:" + std::to_string(dst_locator_t_physical_port) + "-" +
            std::to_string(dst_locator_t_logical_port);

    // Build the source locator
    DatabaseDataQueueWrapper::StatisticsLocator src_locator;
    src_locator.kind(LOCATOR_KIND_TCPv4);
    src_locator.port(src_locator_port);
    src_locator.address(src_locator_address);

    // Build the destination locator
    DatabaseDataQueueWrapper::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsLocator2LocatorData inner_data;
    inner_data.data(1.0);
    inner_data.src_locator(src_locator);
    inner_data.dst_locator(dst_locator);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->locator2locator_data(inner_data);
    data->_d(EventKind::NETWORK_LATENCY);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, src_locator_str)).Times(AnyNumber())
            .WillOnce(Return(std::pair<EntityId, EntityId>(std::make_pair(EntityId(0), EntityId(1)))));

    // Precondition: The destination locator does not exist the first time
    // Precondition: The destination locator exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()))
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::NETWORK_LATENCY);
                EXPECT_EQ(dynamic_cast<const NetworkLatencySample&>(sample).remote_locator, 2);
            });

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Expectation: the remote locator is created and given ID 2
    InsertEntityArgs insert_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::LOCATOR);
                EXPECT_EQ(entity->name, dst_locator_str);
                EXPECT_EQ(entity->alias, dst_locator_str);
                return EntityId(2);
            });

    EXPECT_CALL(database, insert(_)).Times(1)
            .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::NETWORK_LATENCY)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_publication_throughput)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityData inner_data;
    inner_data.data(1.0);
    inner_data.guid(writer_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_data(inner_data);
    data->_d(EventKind::PUBLICATION_THROUGHPUT);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::PUBLICATION_THROUGHPUT);
                EXPECT_EQ(dynamic_cast<const PublicationThroughputSample&>(sample).data, 1.0);
            });

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::PUBLICATION_THROUGHPUT)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_publication_throughput_no_writer)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityData inner_data;
    inner_data.data(1.0);
    inner_data.guid(writer_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_data(inner_data);
    data->_d(EventKind::PUBLICATION_THROUGHPUT);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_subscription_throughput)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueueWrapper::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityData inner_data;
    inner_data.data(1.0);
    inner_data.guid(reader_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_data(inner_data);
    data->_d(EventKind::SUBSCRIPTION_THROUGHPUT);

    // Precondition: The reader exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, DataKind::SUBSCRIPTION_THROUGHPUT);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(dynamic_cast<const SubscriptionThroughputSample&>(sample).data, 1.0);
            });

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::SUBSCRIPTION_THROUGHPUT)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_subscription_throughput_no_reader)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueueWrapper::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityData inner_data;
    inner_data.data(1.0);
    inner_data.guid(reader_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_data(inner_data);
    data->_d(EventKind::SUBSCRIPTION_THROUGHPUT);

    // Precondition: The reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_rtps_sent)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    eprosima::fastdds::rtps::Locator_t dst_locator_t;
    uint16_t dst_locator_t_physical_port = 2048;
    uint16_t dst_locator_t_logical_port = 0;
    IPLocator::setPhysicalPort(dst_locator_t, dst_locator_t_physical_port);
    IPLocator::setLogicalPort(dst_locator_t, dst_locator_t_logical_port);
    uint32_t dst_locator_port = dst_locator_t.port;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:" + std::to_string(dst_locator_t_physical_port) + "-" +
            std::to_string(dst_locator_t_logical_port);

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueueWrapper::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKind::RTPS_SENT);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(2)
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The locator exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(2)
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args1([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::RTPS_PACKETS_SENT);
                EXPECT_EQ(dynamic_cast<const RtpsPacketsSentSample&>(sample).count, 1024u);
                EXPECT_EQ(dynamic_cast<const RtpsPacketsSentSample&>(sample).remote_locator, 2);
            });

    InsertDataArgs args2([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::RTPS_BYTES_SENT);
                EXPECT_EQ(dynamic_cast<const RtpsBytesSentSample&>(sample).count, 2048u);
                EXPECT_EQ(dynamic_cast<const RtpsBytesSentSample&>(sample).magnitude_order, 10);
                EXPECT_EQ(dynamic_cast<const RtpsBytesSentSample&>(sample).remote_locator, 2);
            });

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(2)
            .WillOnce(Invoke(&args1, &InsertDataArgs::insert))
            .WillOnce(Invoke(&args2, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::RTPS_PACKETS_SENT)).Times(1);
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::RTPS_BYTES_SENT)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_rtps_sent_no_writer)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    eprosima::fastdds::rtps::Locator_t dst_locator_t;
    uint16_t dst_locator_t_physical_port = 2048;
    uint16_t dst_locator_t_logical_port = 0;
    IPLocator::setPhysicalPort(dst_locator_t, dst_locator_t_physical_port);
    IPLocator::setLogicalPort(dst_locator_t, dst_locator_t_logical_port);
    uint32_t dst_locator_port = dst_locator_t.port;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:" + std::to_string(dst_locator_t_physical_port) + "-" +
            std::to_string(dst_locator_t_logical_port);

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueueWrapper::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKind::RTPS_SENT);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Throw(BadParameter("Error")));

    // Precondition: The locator exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_rtps_sent_no_locator)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    eprosima::fastdds::rtps::Locator_t dst_locator_t;
    uint16_t dst_locator_t_physical_port = 2048;
    uint16_t dst_locator_t_logical_port = 0;
    IPLocator::setPhysicalPort(dst_locator_t, dst_locator_t_physical_port);
    IPLocator::setLogicalPort(dst_locator_t, dst_locator_t_logical_port);
    uint32_t dst_locator_port = dst_locator_t.port;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:" + std::to_string(dst_locator_t_physical_port) + "-" +
            std::to_string(dst_locator_t_logical_port);

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueueWrapper::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKind::RTPS_SENT);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The destination locator does not exist the first time
    // Precondition: The destination locator exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()))
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: the remote locator is created and given ID 2
    InsertEntityArgs insert_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::LOCATOR);
                EXPECT_EQ(entity->name, dst_locator_str);
                EXPECT_EQ(entity->alias, dst_locator_str);
                return EntityId(2);
            });

    EXPECT_CALL(database, insert(_)).Times(1)
            .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args1([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::RTPS_PACKETS_SENT);
                EXPECT_EQ(dynamic_cast<const RtpsPacketsSentSample&>(sample).count, 1024u);
                EXPECT_EQ(dynamic_cast<const RtpsPacketsSentSample&>(sample).remote_locator, 2);
            });

    InsertDataArgs args2([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::RTPS_BYTES_SENT);
                EXPECT_EQ(dynamic_cast<const RtpsBytesSentSample&>(sample).count, 2048u);
                EXPECT_EQ(dynamic_cast<const RtpsBytesSentSample&>(sample).magnitude_order, 10);
                EXPECT_EQ(dynamic_cast<const RtpsBytesSentSample&>(sample).remote_locator, 2);
            });

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(2)
            .WillOnce(Invoke(&args1, &InsertDataArgs::insert))
            .WillOnce(Invoke(&args2, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::RTPS_PACKETS_SENT)).Times(1);
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::RTPS_BYTES_SENT)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_rtps_lost)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    eprosima::fastdds::rtps::Locator_t dst_locator_t;
    uint16_t dst_locator_t_physical_port = 2048;
    uint16_t dst_locator_t_logical_port = 0;
    IPLocator::setPhysicalPort(dst_locator_t, dst_locator_t_physical_port);
    IPLocator::setLogicalPort(dst_locator_t, dst_locator_t_logical_port);
    uint32_t dst_locator_port = dst_locator_t.port;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:" + std::to_string(dst_locator_t_physical_port) + "-" +
            std::to_string(dst_locator_t_logical_port);

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueueWrapper::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKind::RTPS_LOST);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(2)
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The locator exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(2)
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args1([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::RTPS_PACKETS_LOST);
                EXPECT_EQ(dynamic_cast<const RtpsPacketsLostSample&>(sample).count, 1024u);
                EXPECT_EQ(dynamic_cast<const RtpsPacketsLostSample&>(sample).remote_locator, 2);
            });

    InsertDataArgs args2([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::RTPS_BYTES_LOST);
                EXPECT_EQ(dynamic_cast<const RtpsBytesLostSample&>(sample).count, 2048u);
                EXPECT_EQ(dynamic_cast<const RtpsBytesLostSample&>(sample).magnitude_order, 10);
                EXPECT_EQ(dynamic_cast<const RtpsBytesLostSample&>(sample).remote_locator, 2);
            });

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(2)
            .WillOnce(Invoke(&args1, &InsertDataArgs::insert))
            .WillOnce(Invoke(&args2, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::RTPS_PACKETS_LOST)).Times(1);
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::RTPS_BYTES_LOST)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_rtps_lost_no_writer)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    eprosima::fastdds::rtps::Locator_t dst_locator_t;
    uint16_t dst_locator_t_physical_port = 2048;
    uint16_t dst_locator_t_logical_port = 0;
    IPLocator::setPhysicalPort(dst_locator_t, dst_locator_t_physical_port);
    IPLocator::setLogicalPort(dst_locator_t, dst_locator_t_logical_port);
    uint32_t dst_locator_port = dst_locator_t.port;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:" + std::to_string(dst_locator_t_physical_port) + "-" +
            std::to_string(dst_locator_t_logical_port);

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueueWrapper::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKind::RTPS_LOST);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Throw(BadParameter("Error")));

    // Precondition: The locator exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_rtps_lost_no_locator)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    eprosima::fastdds::rtps::Locator_t dst_locator_t;
    uint16_t dst_locator_t_physical_port = 2048;
    uint16_t dst_locator_t_logical_port = 0;
    IPLocator::setPhysicalPort(dst_locator_t, dst_locator_t_physical_port);
    IPLocator::setLogicalPort(dst_locator_t, dst_locator_t_logical_port);
    uint32_t dst_locator_port = dst_locator_t.port;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:" + std::to_string(dst_locator_t_physical_port) + "-" +
            std::to_string(dst_locator_t_logical_port);

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueueWrapper::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKind::RTPS_LOST);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The destination locator does not exist the first time
    // Precondition: The destination locator exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()))
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: the remote locator is created and given ID 2
    InsertEntityArgs insert_args([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::LOCATOR);
                EXPECT_EQ(entity->name, dst_locator_str);
                EXPECT_EQ(entity->alias, dst_locator_str);
                return EntityId(2);
            });

    EXPECT_CALL(database, insert(_)).Times(1)
            .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args1([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::RTPS_PACKETS_LOST);
                EXPECT_EQ(dynamic_cast<const RtpsPacketsLostSample&>(sample).count, 1024u);
                EXPECT_EQ(dynamic_cast<const RtpsPacketsLostSample&>(sample).remote_locator, 2);
            });

    InsertDataArgs args2([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::RTPS_BYTES_LOST);
                EXPECT_EQ(dynamic_cast<const RtpsBytesLostSample&>(sample).count, 2048u);
                EXPECT_EQ(dynamic_cast<const RtpsBytesLostSample&>(sample).magnitude_order, 10);
                EXPECT_EQ(dynamic_cast<const RtpsBytesLostSample&>(sample).remote_locator, 2);
            });

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(2)
            .WillOnce(Invoke(&args1, &InsertDataArgs::insert))
            .WillOnce(Invoke(&args2, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::RTPS_PACKETS_LOST)).Times(1);
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::RTPS_BYTES_LOST)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

//TODO(jepemi) Test currently not executed as it is not supported by the monitor yet. It passes.
TEST_F(database_queue_tests, push_rtps_bytes_no_writer)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    eprosima::fastdds::rtps::Locator_t dst_locator_t;
    uint16_t dst_locator_t_physical_port = 2048;
    uint16_t dst_locator_t_logical_port = 0;
    IPLocator::setPhysicalPort(dst_locator_t, dst_locator_t_physical_port);
    IPLocator::setLogicalPort(dst_locator_t, dst_locator_t_logical_port);
    uint32_t dst_locator_port = dst_locator_t.port;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:" + std::to_string(dst_locator_t_physical_port) + "-" +
            std::to_string(dst_locator_t_logical_port);

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueueWrapper::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Throw(BadParameter("Error")));

    // Precondition: The locator exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    ByteToLocatorCountSample sample;
    EntityId domain;
    EntityId entity;
    EXPECT_THROW(data_queue.do_process_sample_type(domain, entity, EntityKind::DATAWRITER, sample, inner_data), Error);
}

//TODO(jepemi) Test currently not executed as it is not supported by the monitor yet. It fails.
TEST_F(database_queue_tests, push_rtps_bytes_no_locator)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    eprosima::fastdds::rtps::Locator_t dst_locator_t;
    uint16_t dst_locator_t_physical_port = 2048;
    uint16_t dst_locator_t_logical_port = 0;
    IPLocator::setPhysicalPort(dst_locator_t, dst_locator_t_physical_port);
    IPLocator::setLogicalPort(dst_locator_t, dst_locator_t_logical_port);
    uint32_t dst_locator_port = dst_locator_t.port;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:" + std::to_string(dst_locator_t_physical_port) + "-" +
            std::to_string(dst_locator_t_logical_port);

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueueWrapper::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The locator does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed    // Add to the queue and wait to be processed
    ByteToLocatorCountSample sample;
    EntityId domain;
    EntityId entity;
    EXPECT_THROW(data_queue.do_process_sample_type(domain, entity, EntityKind::DATAWRITER, sample, inner_data), Error);
}

TEST_F(database_queue_tests, push_resent_datas)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::RESENT_DATAS);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::RESENT_DATA);
                EXPECT_EQ(dynamic_cast<const ResentDataSample&>(sample).count, 1024u);
            });

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::RESENT_DATA)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_resent_datas_no_writer)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::RESENT_DATAS);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_heartbeat_count)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::HEARTBEAT_COUNT);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::HEARTBEAT_COUNT);
                EXPECT_EQ(dynamic_cast<const HeartbeatCountSample&>(sample).count, 1024u);
            });

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::HEARTBEAT_COUNT)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_heartbeat_count_no_writer)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::HEARTBEAT_COUNT);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_acknack_count)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueueWrapper::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityCount inner_data;
    inner_data.guid(reader_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::ACKNACK_COUNT);

    // Precondition: The reader exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::ACKNACK_COUNT);
                EXPECT_EQ(dynamic_cast<const AcknackCountSample&>(sample).count, 1024u);
            });

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::ACKNACK_COUNT)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_acknack_count_no_reader)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueueWrapper::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityCount inner_data;
    inner_data.guid(reader_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::ACKNACK_COUNT);

    // Precondition: The reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_nackfrag_count)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueueWrapper::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityCount inner_data;
    inner_data.guid(reader_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::NACKFRAG_COUNT);

    // Precondition: The reader exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::NACKFRAG_COUNT);
                EXPECT_EQ(dynamic_cast<const NackfragCountSample&>(sample).count, 1024u);
            });

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::NACKFRAG_COUNT)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_nackfrag_count_no_reader)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueueWrapper::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityCount inner_data;
    inner_data.guid(reader_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::NACKFRAG_COUNT);

    // Precondition: The reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_gap_count)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::GAP_COUNT);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::GAP_COUNT);
                EXPECT_EQ(dynamic_cast<const GapCountSample&>(sample).count, 1024u);
            });

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::GAP_COUNT)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_gap_count_no_writer)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::GAP_COUNT);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_data_count)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::DATA_COUNT);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::DATA_COUNT);
                EXPECT_EQ(dynamic_cast<const DataCountSample&>(sample).count, 1024u);
            });

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::DATA_COUNT)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_data_count_no_writer)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::DATA_COUNT);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_pdp_count)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueueWrapper::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityCount inner_data;
    inner_data.guid(participant_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::PDP_PACKETS);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::PDP_PACKETS);
                EXPECT_EQ(dynamic_cast<const PdpCountSample&>(sample).count, 1024u);
            });

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::PDP_PACKETS)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_pdp_count_no_participant)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueueWrapper::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityCount inner_data;
    inner_data.guid(participant_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::PDP_PACKETS);

    // Precondition: The participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_edp_count)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueueWrapper::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityCount inner_data;
    inner_data.guid(participant_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::EDP_PACKETS);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::EDP_PACKETS);
                EXPECT_EQ(dynamic_cast<const EdpCountSample&>(sample).count, 1024u);
            });

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::EDP_PACKETS)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_edp_count_no_participant)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueueWrapper::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsEntityCount inner_data;
    inner_data.guid(participant_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKind::EDP_PACKETS);

    // Precondition: The participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_discovery_times)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::array<uint8_t, 4> entity_id = {0, 0, 0, 1};
    // discovery time must be rounded to tenths of nanosecond to avoid truncation by
    // windows system_clock
    uint64_t discovery_time = 1000;
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";
    std::string remote_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    std::chrono::system_clock::time_point discovery_timestamp =
            eprosima::statistics_backend::nanoseconds_to_systemclock(discovery_time);

    // Build the participant GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueueWrapper::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the remote GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix remote_prefix;
    remote_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId remote_entity_id;
    remote_entity_id.value(entity_id);
    DatabaseDataQueueWrapper::StatisticsGuid remote_guid;
    remote_guid.guidPrefix(remote_prefix);
    remote_guid.entityId(remote_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsDiscoveryTime inner_data;
    inner_data.local_participant_guid(participant_guid);
    inner_data.remote_entity_guid(remote_guid);
    inner_data.time(discovery_time);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->discovery_time(inner_data);
    data->_d(EventKind::DISCOVERED_ENTITY);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The remote entity exists and has ID 2
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, remote_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(2))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::DISCOVERY_TIME);
                EXPECT_EQ(dynamic_cast<const DiscoveryTimeSample&>(sample).remote_entity, 2);
                EXPECT_EQ(dynamic_cast<const DiscoveryTimeSample&>(sample).time, discovery_timestamp);
            });

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::DISCOVERY_TIME)).Times(1);

    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_discovery_times_no_participant)
{
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::array<uint8_t, 4> entity_id = {0, 0, 0, 1};
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";
    std::string remote_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    long long discovery_time = 1024;
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the participant GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueueWrapper::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the remote GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix remote_prefix;
    remote_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId remote_entity_id;
    remote_entity_id.value(entity_id);
    DatabaseDataQueueWrapper::StatisticsGuid remote_guid;
    remote_guid.guidPrefix(remote_prefix);
    remote_guid.entityId(remote_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsDiscoveryTime inner_data;
    inner_data.local_participant_guid(participant_guid);
    inner_data.remote_entity_guid(remote_guid);
    inner_data.time(discovery_time);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->discovery_time(inner_data);
    data->_d(EventKind::DISCOVERED_ENTITY);

    // Precondition: The participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Precondition: The remote entity exists and has ID 2
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, remote_guid_str)).Times(AnyNumber())
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(2))));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_discovery_times_no_entity)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::array<uint8_t, 4> entity_id = {0, 0, 0, 1};
    uint64_t discovery_time = 1024;
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";
    std::string remote_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the participant GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueueWrapper::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the remote GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix remote_prefix;
    remote_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId remote_entity_id;
    remote_entity_id.value(entity_id);
    DatabaseDataQueueWrapper::StatisticsGuid remote_guid;
    remote_guid.guidPrefix(remote_prefix);
    remote_guid.entityId(remote_entity_id);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsDiscoveryTime inner_data;
    inner_data.local_participant_guid(participant_guid);
    inner_data.remote_entity_guid(remote_guid);
    inner_data.time(discovery_time);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->discovery_time(inner_data);
    data->_d(EventKind::DISCOVERED_ENTITY);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The remote entity does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, remote_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_sample_datas)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    int32_t sn_high = 2048;
    uint32_t sn_low = 4096;
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    eprosima::fastdds::
            rtps::SequenceNumber_t sn (sn_high, sn_low);

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    DatabaseDataQueueWrapper::StatisticsSequenceNumber sequence_number;
    sequence_number.high(sn_high);
    sequence_number.low(sn_low);

    DatabaseDataQueueWrapper::StatisticsSampleIdentity sample_identity;
    sample_identity.writer_guid(writer_guid);
    sample_identity.sequence_number(sequence_number);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsSampleIdentityCount inner_data;
    inner_data.count(1024);
    inner_data.sample_id(sample_identity);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->sample_identity_count(inner_data);
    data->_d(EventKind::SAMPLE_DATAS);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Expectation: The insert method is called with appropriate arguments
    InsertDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const StatisticsSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.src_ts, timestamp);
                EXPECT_EQ(sample.kind, DataKind::SAMPLE_DATAS);
                EXPECT_EQ(dynamic_cast<const SampleDatasCountSample&>(sample).count, 1024u);
                EXPECT_EQ(dynamic_cast<const SampleDatasCountSample&>(sample).sequence_number, sn.to64long());
            });
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(1)
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::SAMPLE_DATAS)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_sample_datas_no_writer)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    int32_t sn_high = 2048;
    uint32_t sn_low = 4096;
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    eprosima::fastdds::rtps::SequenceNumber_t sn (sn_high, sn_low);

    // Build the writer GUID
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    DatabaseDataQueueWrapper::StatisticsSequenceNumber sequence_number;
    sequence_number.high(sn_high);
    sequence_number.low(sn_low);

    DatabaseDataQueueWrapper::StatisticsSampleIdentity sample_identity;
    sample_identity.writer_guid(writer_guid);
    sample_identity.sequence_number(sequence_number);

    // Build the Statistics data
    DatabaseDataQueueWrapper::StatisticsSampleIdentityCount inner_data;
    inner_data.count(1024);
    inner_data.sample_id(sample_identity);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->sample_identity_count(inner_data);
    data->_d(EventKind::SAMPLE_DATAS);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const StatisticsSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_proxy)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the participant GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";
    DatabaseDataQueueWrapper::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueueWrapper::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind = eprosima::fastdds::statistics::StatusKind::PROXY;
    MonitorServiceData value;
    std::vector<uint8_t> entity_proxy = {1, 2, 3, 4, 5};
    value.entity_proxy(entity_proxy);
    data->local_entity(participant_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity_kind_by_guid(participant_guid)).Times(1)
            .WillOnce(Return(EntityKind::PARTICIPANT));

    // Expectation: The insert method is called with appropriate arguments
    InsertMonitorServiceDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const MonitorServiceSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, eprosima::statistics_backend::StatusKind::PROXY);
                EXPECT_EQ(sample.status, StatusLevel::OK_STATUS);
                EXPECT_EQ(dynamic_cast<const ProxySample&>(sample).entity_proxy, entity_proxy);

                return true;
            });
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertMonitorServiceDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(EntityId(0), EntityId(1), eprosima::statistics_backend::StatusKind::PROXY)).Times(1);

    // Update graph
    EXPECT_CALL(database, update_graph_on_updated_entity(EntityId(0), EntityId(1))).Times(1)
            .WillOnce(Return(true));
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_view_graph_update(EntityId(0))).Times(1);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_proxy_no_entity)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the participant GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";
    DatabaseDataQueueWrapper::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueueWrapper::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind = eprosima::fastdds::statistics::StatusKind::PROXY;
    MonitorServiceData value;
    std::vector<uint8_t> entity_proxy = {1, 2, 3, 4, 5};
    value.entity_proxy(entity_proxy);
    data->local_entity(participant_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));
    EXPECT_CALL(database, get_entity_kind_by_guid(participant_guid)).Times(1)
            .WillOnce(Return(EntityKind::PARTICIPANT));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_connection_list)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the participant GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";
    DatabaseDataQueueWrapper::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueueWrapper::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build connection list sequence
    std::vector<Connection> connection_list;
    Connection connection;
    connection.mode(eprosima::fastdds::statistics::ConnectionMode::DATA_SHARING);
    std::array<uint8_t, 4> other_entity_id = {0, 0, 0, 1};
    std::string entity_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    DatabaseDataQueueWrapper::StatisticsEntityId entity_id;
    entity_id.value(other_entity_id);
    DatabaseDataQueueWrapper::StatisticsGuid entity_guid;
    entity_guid.guidPrefix(participant_prefix);
    entity_guid.entityId(entity_id);
    connection.guid(entity_guid);
    eprosima::fastdds::statistics::detail::Locator_s locator;
    locator.kind(1);
    locator.port(1);
    locator.address({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
    connection.announced_locators({locator});
    connection.used_locators({locator});
    connection_list = {connection, connection};

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind =
            eprosima::fastdds::statistics::StatusKind::CONNECTION_LIST;
    MonitorServiceData value;
    value.connection_list(connection_list);
    data->local_entity(participant_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity_kind_by_guid(participant_guid)).Times(1)
            .WillOnce(Return(EntityKind::PARTICIPANT));

    // Expectation: The insert method is called with appropriate arguments
    InsertMonitorServiceDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const MonitorServiceSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, eprosima::statistics_backend::StatusKind::CONNECTION_LIST);
                EXPECT_EQ(sample.status, StatusLevel::OK_STATUS);
                EXPECT_EQ(dynamic_cast<const ConnectionListSample&>(sample).connection_list, connection_list);

                return false;
            });
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertMonitorServiceDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(EntityId(0), EntityId(1),
            eprosima::statistics_backend::StatusKind::CONNECTION_LIST)).Times(1);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_connection_list_no_entity)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the participant GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";
    DatabaseDataQueueWrapper::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueueWrapper::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build connection list sequence
    std::vector<Connection> connection_list;
    Connection connection;
    connection.mode(eprosima::fastdds::statistics::ConnectionMode::DATA_SHARING);
    std::array<uint8_t, 4> other_entity_id = {0, 0, 0, 1};
    std::string entity_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    DatabaseDataQueueWrapper::StatisticsEntityId entity_id;
    entity_id.value(other_entity_id);
    DatabaseDataQueueWrapper::StatisticsGuid entity_guid;
    entity_guid.guidPrefix(participant_prefix);
    entity_guid.entityId(entity_id);
    connection.guid(entity_guid);
    eprosima::fastdds::statistics::detail::Locator_s locator;
    locator.kind(1);
    locator.port(1);
    locator.address({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
    connection.announced_locators({locator});
    connection.used_locators({locator});
    connection_list = {connection, connection};

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind =
            eprosima::fastdds::statistics::StatusKind::CONNECTION_LIST;
    MonitorServiceData value;
    value.connection_list(connection_list);
    data->local_entity(participant_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));
    EXPECT_CALL(database, get_entity_kind_by_guid(participant_guid)).Times(1)
            .WillOnce(Return(EntityKind::PARTICIPANT));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_incompatible_qos)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the writer GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    int32_t sn_high = 2048;
    uint32_t sn_low = 4096;
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    eprosima::fastdds::rtps::SequenceNumber_t sn (sn_high, sn_low);
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build incompatible qos status
    IncompatibleQoSStatus_s incompatible_qos_status;
    incompatible_qos_status.total_count(0);
    incompatible_qos_status.last_policy_id(0);
    QosPolicyCountSeq_s qos_policy_count_seq;
    QosPolicyCount_s qos_policy_count;
    qos_policy_count.policy_id(0);
    qos_policy_count.count(0);
    qos_policy_count_seq = {qos_policy_count};
    incompatible_qos_status.policies(qos_policy_count_seq);

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind =
            eprosima::fastdds::statistics::StatusKind::INCOMPATIBLE_QOS;
    MonitorServiceData value;
    value.incompatible_qos_status(incompatible_qos_status);
    data->local_entity(writer_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity_kind_by_guid(writer_guid)).Times(1)
            .WillOnce(Return(EntityKind::DATAWRITER));

    // Expectation: The insert method is called with appropriate arguments
    InsertMonitorServiceDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const MonitorServiceSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, eprosima::statistics_backend::StatusKind::INCOMPATIBLE_QOS);
                EXPECT_EQ(sample.status, StatusLevel::OK_STATUS);
                EXPECT_EQ(dynamic_cast<const IncompatibleQosSample&>(sample).incompatible_qos_status,
                incompatible_qos_status);

                return false;
            });
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertMonitorServiceDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(EntityId(0), EntityId(1),
            eprosima::statistics_backend::StatusKind::INCOMPATIBLE_QOS)).Times(1);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_incompatible_qos_no_entity)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the writer GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    int32_t sn_high = 2048;
    uint32_t sn_low = 4096;
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    eprosima::fastdds::rtps::SequenceNumber_t sn (sn_high, sn_low);
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build incompatible qos status
    IncompatibleQoSStatus_s incompatible_qos_status;
    incompatible_qos_status.total_count(1);
    incompatible_qos_status.last_policy_id(0);
    QosPolicyCountSeq_s qos_policy_count_seq;
    QosPolicyCount_s qos_policy_count;
    qos_policy_count.policy_id(0);
    qos_policy_count.count(0);
    qos_policy_count_seq = {qos_policy_count};
    incompatible_qos_status.policies(qos_policy_count_seq);

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind =
            eprosima::fastdds::statistics::StatusKind::INCOMPATIBLE_QOS;
    MonitorServiceData value;
    value.incompatible_qos_status(incompatible_qos_status);
    data->local_entity(writer_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));
    EXPECT_CALL(database, get_entity_kind_by_guid(writer_guid)).Times(1)
            .WillOnce(Return(EntityKind::DATAWRITER));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_inconsistent_topic)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the writer GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    int32_t sn_high = 2048;
    uint32_t sn_low = 4096;
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    eprosima::fastdds::rtps::SequenceNumber_t sn (sn_high, sn_low);
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build inconsistent topic status
    InconsistentTopicStatus_s inconsistent_topic_status;
    inconsistent_topic_status.total_count(0);

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind =
            eprosima::fastdds::statistics::StatusKind::INCONSISTENT_TOPIC;
    MonitorServiceData value;
    value.inconsistent_topic_status(inconsistent_topic_status);
    data->local_entity(writer_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity_kind_by_guid(writer_guid)).Times(1)
            .WillOnce(Return(EntityKind::DATAWRITER));

    // Expectation: The insert method is called with appropriate arguments
    InsertMonitorServiceDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const MonitorServiceSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, eprosima::statistics_backend::StatusKind::INCONSISTENT_TOPIC);
                EXPECT_EQ(sample.status, StatusLevel::OK_STATUS);
                EXPECT_EQ(dynamic_cast<const InconsistentTopicSample&>(sample).inconsistent_topic_status,
                inconsistent_topic_status);

                return false;
            });
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertMonitorServiceDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(EntityId(0), EntityId(1),
            eprosima::statistics_backend::StatusKind::INCONSISTENT_TOPIC)).Times(1);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_inconsistent_topic_no_entity)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the writer GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    int32_t sn_high = 2048;
    uint32_t sn_low = 4096;
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    eprosima::fastdds::rtps::SequenceNumber_t sn (sn_high, sn_low);
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build inconsistent topic status
    InconsistentTopicStatus_s inconsistent_topic_status;
    inconsistent_topic_status.total_count(1);

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind =
            eprosima::fastdds::statistics::StatusKind::INCONSISTENT_TOPIC;
    MonitorServiceData value;
    value.inconsistent_topic_status(inconsistent_topic_status);
    data->local_entity(writer_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));
    EXPECT_CALL(database, get_entity_kind_by_guid(writer_guid)).Times(1)
            .WillOnce(Return(EntityKind::DATAWRITER));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_liveliness_lost)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the writer GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    int32_t sn_high = 2048;
    uint32_t sn_low = 4096;
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    eprosima::fastdds::rtps::SequenceNumber_t sn (sn_high, sn_low);
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build liveliness lost status
    LivelinessLostStatus_s liveliness_lost_status;
    liveliness_lost_status.total_count(0);

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind =
            eprosima::fastdds::statistics::StatusKind::LIVELINESS_LOST;
    MonitorServiceData value;
    value.liveliness_lost_status(liveliness_lost_status);
    data->local_entity(writer_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity_kind_by_guid(writer_guid)).Times(1)
            .WillOnce(Return(EntityKind::DATAWRITER));

    // Expectation: The insert method is called with appropriate arguments
    InsertMonitorServiceDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const MonitorServiceSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, eprosima::statistics_backend::StatusKind::LIVELINESS_LOST);
                EXPECT_EQ(sample.status, StatusLevel::OK_STATUS);
                EXPECT_EQ(dynamic_cast<const LivelinessLostSample&>(sample).liveliness_lost_status,
                liveliness_lost_status);

                return false;
            });
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertMonitorServiceDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(EntityId(0), EntityId(1),
            eprosima::statistics_backend::StatusKind::LIVELINESS_LOST)).Times(1);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_liveliness_lost_no_entity)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the writer GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    int32_t sn_high = 2048;
    uint32_t sn_low = 4096;
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    eprosima::fastdds::rtps::SequenceNumber_t sn (sn_high, sn_low);
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build liveliness lost status
    LivelinessLostStatus_s liveliness_lost_status;
    liveliness_lost_status.total_count(1);

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind =
            eprosima::fastdds::statistics::StatusKind::LIVELINESS_LOST;
    MonitorServiceData value;
    value.liveliness_lost_status(liveliness_lost_status);
    data->local_entity(writer_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));
    EXPECT_CALL(database, get_entity_kind_by_guid(writer_guid)).Times(1)
            .WillOnce(Return(EntityKind::DATAWRITER));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_liveliness_changed)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the reader GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    DatabaseDataQueueWrapper::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueueWrapper::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build liveliness changed status
    LivelinessChangedStatus_s liveliness_changed_status;
    liveliness_changed_status.alive_count(0);
    liveliness_changed_status.not_alive_count(0);
    liveliness_changed_status.last_publication_handle({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind =
            eprosima::fastdds::statistics::StatusKind::LIVELINESS_CHANGED;
    MonitorServiceData value;
    value.liveliness_changed_status(liveliness_changed_status);
    data->local_entity(reader_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The reader exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity_kind_by_guid(reader_guid)).Times(1)
            .WillOnce(Return(EntityKind::DATAREADER));

    // Expectation: The insert method is called with appropriate arguments
    InsertMonitorServiceDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const MonitorServiceSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, eprosima::statistics_backend::StatusKind::LIVELINESS_CHANGED);
                EXPECT_EQ(sample.status, StatusLevel::OK_STATUS);
                EXPECT_EQ(dynamic_cast<const LivelinessChangedSample&>(sample).liveliness_changed_status,
                liveliness_changed_status);

                return false;
            });
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertMonitorServiceDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(EntityId(0), EntityId(1),
            eprosima::statistics_backend::StatusKind::LIVELINESS_CHANGED)).Times(1);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_liveliness_changed_no_entity)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the reader GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    DatabaseDataQueueWrapper::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueueWrapper::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build liveliness changed status
    LivelinessChangedStatus_s liveliness_changed_status;
    liveliness_changed_status.alive_count(0);
    liveliness_changed_status.not_alive_count(0);
    liveliness_changed_status.last_publication_handle({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind =
            eprosima::fastdds::statistics::StatusKind::LIVELINESS_CHANGED;
    MonitorServiceData value;
    value.liveliness_changed_status(liveliness_changed_status);
    data->local_entity(reader_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));
    EXPECT_CALL(database, get_entity_kind_by_guid(reader_guid)).Times(1)
            .WillOnce(Return(EntityKind::DATAREADER));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}


TEST_F(database_queue_tests, push_monitor_deadline_missed)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the reader GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    DatabaseDataQueueWrapper::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueueWrapper::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build deadeline missed status
    DeadlineMissedStatus_s deadline_missed_status;
    deadline_missed_status.total_count(0);
    deadline_missed_status.last_instance_handle({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind =
            eprosima::fastdds::statistics::StatusKind::DEADLINE_MISSED;
    MonitorServiceData value;
    value.deadline_missed_status(deadline_missed_status);
    data->local_entity(reader_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The reader exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity_kind_by_guid(reader_guid)).Times(1)
            .WillOnce(Return(EntityKind::DATAREADER));

    // Expectation: The insert method is called with appropriate arguments
    InsertMonitorServiceDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const MonitorServiceSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, eprosima::statistics_backend::StatusKind::DEADLINE_MISSED);
                EXPECT_EQ(sample.status, StatusLevel::OK_STATUS);
                EXPECT_EQ(dynamic_cast<const DeadlineMissedSample&>(sample).deadline_missed_status,
                deadline_missed_status);

                return false;
            });
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertMonitorServiceDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(EntityId(0), EntityId(1),
            eprosima::statistics_backend::StatusKind::DEADLINE_MISSED)).Times(1);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_deadline_missed_no_entity)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the reader GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    DatabaseDataQueueWrapper::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueueWrapper::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build deadeline missed status
    DeadlineMissedStatus_s deadline_missed_status;
    deadline_missed_status.total_count(1);
    deadline_missed_status.last_instance_handle({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind =
            eprosima::fastdds::statistics::StatusKind::DEADLINE_MISSED;
    MonitorServiceData value;
    value.deadline_missed_status(deadline_missed_status);
    data->local_entity(reader_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));
    EXPECT_CALL(database, get_entity_kind_by_guid(reader_guid)).Times(1)
            .WillOnce(Return(EntityKind::DATAREADER));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_sample_lost)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the reader GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    DatabaseDataQueueWrapper::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueueWrapper::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build sample lost status
    SampleLostStatus_s sample_lost_status;
    sample_lost_status.total_count(0);

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind = eprosima::fastdds::statistics::StatusKind::SAMPLE_LOST;
    MonitorServiceData value;
    value.sample_lost_status(sample_lost_status);
    data->local_entity(reader_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The reader exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity_kind_by_guid(reader_guid)).Times(1)
            .WillOnce(Return(EntityKind::DATAREADER));

    // Expectation: The insert method is called with appropriate arguments
    InsertMonitorServiceDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const MonitorServiceSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, eprosima::statistics_backend::StatusKind::SAMPLE_LOST);
                EXPECT_EQ(sample.status, StatusLevel::OK_STATUS);
                EXPECT_EQ(dynamic_cast<const SampleLostSample&>(sample).sample_lost_status, sample_lost_status);

                return false;
            });
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertMonitorServiceDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(EntityId(0), EntityId(1),
            eprosima::statistics_backend::StatusKind::SAMPLE_LOST)).Times(1);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_sample_lost_no_entity)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the reader GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    DatabaseDataQueueWrapper::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueueWrapper::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build sample lost status
    SampleLostStatus_s sample_lost_status;
    sample_lost_status.total_count(1);

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind = eprosima::fastdds::statistics::StatusKind::SAMPLE_LOST;
    MonitorServiceData value;
    value.sample_lost_status(sample_lost_status);
    data->local_entity(reader_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));
    EXPECT_CALL(database, get_entity_kind_by_guid(reader_guid)).Times(1)
            .WillOnce(Return(EntityKind::DATAREADER));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_extended_incompatible_qos)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the writer GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build incompatible qos status
    eprosima::fastdds::statistics::ExtendedIncompatibleQoSStatus_s status;
    std::stringstream remote_entity_guid_str("01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1");
    eprosima::fastdds::statistics::detail::GUID_s remote_entity_guid_s;
    eprosima::fastdds::rtps::GUID_t remote_entity_guid_t;

    remote_entity_guid_str >> remote_entity_guid_t;
    memcpy(remote_entity_guid_s.guidPrefix().value().data(), remote_entity_guid_t.guidPrefix.value,
            eprosima::fastdds::rtps::GuidPrefix_t::size);
    memcpy(remote_entity_guid_s.entityId().value().data(), remote_entity_guid_t.entityId.value,
            eprosima::fastdds::rtps::EntityId_t::size);

    status.remote_guid(remote_entity_guid_s);
    status.current_incompatible_policies(std::vector<uint32_t>{1, 2, 3});

    eprosima::fastdds::statistics::ExtendedIncompatibleQoSStatusSeq_s status_seq({status});

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind =
            eprosima::fastdds::statistics::StatusKind::EXTENDED_INCOMPATIBLE_QOS;
    MonitorServiceData value;
    value.extended_incompatible_qos_status({status});
    data->local_entity(writer_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity_kind_by_guid(writer_guid)).Times(1)
            .WillOnce(Return(EntityKind::DATAWRITER));

    // Expectation: The insert method is called with appropriate arguments
    InsertMonitorServiceDataArgs args([&](
                const EntityId& domain_id,
                const EntityId& entity_id,
                const MonitorServiceSample& sample)
            {
                EXPECT_EQ(entity_id, 1);
                EXPECT_EQ(domain_id, 0);
                EXPECT_EQ(sample.kind, eprosima::statistics_backend::StatusKind::EXTENDED_INCOMPATIBLE_QOS);
                EXPECT_EQ(sample.status, StatusLevel::ERROR_STATUS);
                EXPECT_EQ(dynamic_cast<const ExtendedIncompatibleQosSample&>(sample).extended_incompatible_qos_status,
                status_seq);

                return false;
            });
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(1)
            .WillRepeatedly(Invoke(&args, &InsertMonitorServiceDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(EntityId(0), EntityId(1),
            eprosima::statistics_backend::StatusKind::EXTENDED_INCOMPATIBLE_QOS)).Times(1);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_extended_incompatible_qos_no_entity)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the writer GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    DatabaseDataQueueWrapper::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueueWrapper::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build incompatible qos status
    eprosima::fastdds::statistics::ExtendedIncompatibleQoSStatus_s status;
    std::stringstream remote_entity_guid_str("01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1");
    eprosima::fastdds::statistics::detail::GUID_s remote_entity_guid_s;
    eprosima::fastdds::rtps::GUID_t remote_entity_guid_t;

    remote_entity_guid_str >> remote_entity_guid_t;
    memcpy(remote_entity_guid_s.guidPrefix().value().data(), remote_entity_guid_t.guidPrefix.value,
            eprosima::fastdds::rtps::GuidPrefix_t::size);
    memcpy(remote_entity_guid_s.entityId().value().data(), remote_entity_guid_t.entityId.value,
            eprosima::fastdds::rtps::EntityId_t::size);

    status.remote_guid(remote_entity_guid_s);
    status.current_incompatible_policies(std::vector<uint32_t>{1, 2, 3});

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind =
            eprosima::fastdds::statistics::StatusKind::EXTENDED_INCOMPATIBLE_QOS;
    MonitorServiceData value;
    value.extended_incompatible_qos_status({status});
    data->local_entity(writer_guid);
    data->status_kind(kind);
    data->value(value);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));
    EXPECT_CALL(database, get_entity_kind_by_guid(writer_guid)).Times(1)
            .WillOnce(Return(EntityKind::DATAWRITER));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

TEST_F(database_queue_tests, push_monitor_statuses_size)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the reader GUID
    std::array<uint8_t, 12> prefix = {1, 15, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    DatabaseDataQueueWrapper::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueueWrapper::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueueWrapper::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Monitor Service data
    std::shared_ptr<MonitorServiceStatusData> data = std::make_shared<MonitorServiceStatusData>();
    eprosima::fastdds::statistics::StatusKind::StatusKind kind =
            eprosima::fastdds::statistics::StatusKind::STATUSES_SIZE;
    MonitorServiceData value;
    uint8_t octet = 1;
    value.statuses_size(octet);
    data->local_entity(reader_guid);
    data->status_kind(kind);
    data->value(value);

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, testing::Matcher<const MonitorServiceSample&>(_))).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_status_reported(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    monitor_data_queue.push(timestamp, data);
    monitor_data_queue.flush();
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
