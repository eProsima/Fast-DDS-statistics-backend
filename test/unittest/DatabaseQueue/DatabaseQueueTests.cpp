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

#include <database/database.hpp>
#include <database/database_queue.hpp>
#include <topic_types/types.h>

using namespace eprosima::fastdds::statistics;
using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;
using namespace eprosima::fastrtps::rtps;

using StatisticsData = eprosima::fastdds::statistics::Data;

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
class DatabaseDataQueueWrapper : public DatabaseDataQueue
{

public:

    DatabaseDataQueueWrapper(
            Database* database)
        : DatabaseDataQueue(database)
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

class database_queue_tests : public ::testing::Test
{

public:

    StrictMock<Database> database;
    DatabaseEntityQueueWrapper entity_queue;
    DatabaseDataQueueWrapper data_queue;

    database_queue_tests()
        : entity_queue(&database)
        , data_queue(&database)
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
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    Qos participant_qos;
    std::string address = "127.0.0.1";

    std::string processname = "command";
    std::string pid = "1234";
    std::string username = "user";
    std::string hostname = "host";

    EntityDiscoveryInfo info(EntityKind::PARTICIPANT);
    info.domain_id = EntityId(0);
    std::stringstream(participant_guid_str) >> info.guid;
    info.qos = participant_qos;
    info.address = address;
    info.participant_name = participant_name;

    // Precondition: The domain exists and has ID 0
    std::shared_ptr<Domain> domain;
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain));

    // Participant undiscovery: FAILURE
    {
        // Precondition: The participant does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        EXPECT_CALL(database, insert(_)).Times(0);

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

        EXPECT_CALL(database, insert(_)).Times(0);

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
        InsertEntityArgs insert_args([&](
                    std::shared_ptr<Entity> entity)
                {
                    switch(entity->kind)
                    {
                        case EntityKind::PARTICIPANT:
                        {
                                EXPECT_EQ(entity->name, participant_name);
                                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->qos, participant_qos);
                                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->domain, domain);
                                EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->process, nullptr);
                                return EntityId(1);
                        }
                        case EntityKind::HOST:
                        {
                                return EntityId(2);
                        }
                        case EntityKind::USER:
                        {
                                return EntityId(3);
                        }
                        case EntityKind::PROCESS:
                        {
                                return EntityId(4);
                        }
                        default:
                        {
                                return EntityId();
                                break;
                        }
                    }
                });

        EXPECT_CALL(database, insert(_)).Times(4)
                .WillRepeatedly(Invoke(&insert_args, &InsertEntityArgs::insert));

        // Expectation: Get host-user-process
        EXPECT_CALL(database, get_entities_by_name(EntityKind::HOST,_)).Times(1);
        EXPECT_CALL(database, get_entities_by_name(EntityKind::USER,_)).Times(1);
        EXPECT_CALL(database, get_entities_by_name(EntityKind::PROCESS,_)).Times(1);
        EXPECT_CALL(database, link_participant_with_process(_,_)).Times(1);

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, add_participant_to_graph(_,_,_,_,_)).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_graph_update(_)).Times(1);

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

        EXPECT_CALL(database, insert(_)).Times(0);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(1), true)).Times(1);

        // Expectation: Get host, user and process
        EXPECT_CALL(database, get_entities_by_name(EntityKind::HOST,_)).Times(1)
        .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(2)))));
        auto host = std::make_shared<Host>(hostname);
        host->id = EntityId(2);
        EXPECT_CALL(database, get_entity(EntityId(2))).Times(1)
        .WillOnce(Return(host));

        EXPECT_CALL(database, get_entities_by_name(EntityKind::USER,_)).Times(1)
        .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(3)))));
        auto user = std::make_shared<User>(username, host);
        user->id = EntityId(3);
        EXPECT_CALL(database, get_entity(EntityId(3))).Times(1)
        .WillOnce(Return(user));
    
        EXPECT_CALL(database, get_entities_by_name(EntityKind::PROCESS,_)).Times(1)
        .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(4)))));
        auto process = std::make_shared<Process>(processname, pid, user);
        process->id = EntityId(4);
        EXPECT_CALL(database, get_entity(EntityId(4))).Times(1)
        .WillOnce(Return(process));

        EXPECT_CALL(database, link_participant_with_process(_,_)).Times(1);

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, delete_participant_from_graph(_,_,_,_,_)).Times(1).WillOnce(Return(false));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_graph_update(_)).Times(0);

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

        EXPECT_CALL(database, insert(_)).Times(0);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(1), false)).Times(1);

        // Expectation: Get host, user and process
        EXPECT_CALL(database, get_entities_by_name(EntityKind::HOST,_)).Times(1)
        .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(2)))));
        auto host = std::make_shared<Host>(hostname);
        host->id = EntityId(2);
        EXPECT_CALL(database, get_entity(EntityId(2))).Times(1)
        .WillOnce(Return(host));

        EXPECT_CALL(database, get_entities_by_name(EntityKind::USER,_)).Times(1)
        .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(3)))));
        auto user = std::make_shared<User>(username, host);
        user->id = EntityId(3);
        EXPECT_CALL(database, get_entity(EntityId(3))).Times(1)
        .WillOnce(Return(user));
    
        EXPECT_CALL(database, get_entities_by_name(EntityKind::PROCESS,_)).Times(1)
        .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(4)))));
        auto process = std::make_shared<Process>(processname, pid, user);
        process->id = EntityId(4);
        EXPECT_CALL(database, get_entity(EntityId(4))).Times(1)
        .WillOnce(Return(process));

        EXPECT_CALL(database, link_participant_with_process(_,_)).Times(1);

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, delete_participant_from_graph(_,_,_,_,_)).Times(1).WillOnce(Return(false));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_graph_update(_)).Times(0);

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
        InsertEntityArgs insert_args([&](
                    std::shared_ptr<Entity> entity) -> EntityId
                {
                    EXPECT_EQ(entity->kind, EntityKind::PARTICIPANT);
                    EXPECT_EQ(entity->name, participant_name);
                    EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->domain, domain);
                    EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->qos, participant_qos);
                    EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->domain, domain);
                    EXPECT_EQ(std::dynamic_pointer_cast<DomainParticipant>(entity)->process, nullptr);

                    throw BadParameter("Error");
                });

        EXPECT_CALL(database, insert(_)).Times(1)
                .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert));

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

TEST_F(database_queue_tests, push_datawriter)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Create the writer info
    std::string datawriter_name = "DataWriter_topic_name_0.0.0.1";  //< Name constructed from the topic and entity_id
    Qos datawriter_qos;
    std::string datawriter_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
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

    // Precondition: The domain exists and has ID 0
    std::shared_ptr<Domain> domain;
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain));

    // Precondition: The participant exists and has ID 1
    std::string participant_name = "participant";
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(
        participant_name, Qos(), participant_guid_str, nullptr, domain);
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant));

    // Precondition: The topic exists and has ID 2
    std::shared_ptr<Topic> topic = std::make_shared<Topic>(topic_name, type_name, domain);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic));

    // Precondition: The locators exist and have ID 100 and 101
    std::shared_ptr<Locator> ulocator = std::make_shared<Locator>(unicast_locator_str);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, unicast_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(100)))));
    EXPECT_CALL(database, get_entity(EntityId(100))).Times(AnyNumber())
            .WillRepeatedly(Return(ulocator));

    std::shared_ptr<Locator> mlocator = std::make_shared<Locator>(multicast_locator_str);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, multicast_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(101)))));
    EXPECT_CALL(database, get_entity(EntityId(101))).Times(AnyNumber())
            .WillRepeatedly(Return(mlocator));

    // Datawriter undiscovery: FAILURE
    {
        // Precondition: The writer does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, datawriter_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        EXPECT_CALL(database, insert(_)).Times(0);

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

        EXPECT_CALL(database, insert(_)).Times(0);

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
        InsertEntityArgs insert_args([&](
                    std::shared_ptr<Entity> entity)
                {
                    EXPECT_EQ(entity->kind, EntityKind::DATAWRITER);
                    EXPECT_EQ(entity->name, datawriter_name);
                    EXPECT_EQ(entity->alias, datawriter_name);
                    EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->guid, datawriter_guid_str);
                    EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->qos, datawriter_qos);

                    return EntityId(3);
                });

        EXPECT_CALL(database, insert(_)).Times(1).WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert));

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, add_endpoint_to_graph(_,_,_,_)).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_graph_update(_)).Times(1);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(3), true)).Times(1);

        // Expectations: Request the backend to notify user (if needed)
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_domain_entity_discovery(EntityId(0), EntityId(3), EntityKind::DATAWRITER,
                details::StatisticsBackendData::DISCOVERY))
                .Times(1);

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

        EXPECT_CALL(database, insert(_)).Times(0);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(3), true)).Times(1);

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, delete_endpoint_from_graph(_,_,_,_)).Times(1).WillOnce(Return(false));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_graph_update(_)).Times(0);

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

        EXPECT_CALL(database, insert(_)).Times(0);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(3), false)).Times(1);

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, delete_endpoint_from_graph(_,_,_,_)).Times(1).WillOnce(Return(false));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_graph_update(_)).Times(0);

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
        InsertEntityArgs insert_args([&](
                    std::shared_ptr<Entity> entity) -> EntityId
                {
                    EXPECT_EQ(entity->kind, EntityKind::DATAWRITER);
                    EXPECT_EQ(entity->name, datawriter_name);
                    EXPECT_EQ(entity->alias, datawriter_name);
                    EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->guid, datawriter_guid_str);
                    EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->qos, datawriter_qos);

                    throw BadParameter("Error");
                });

        EXPECT_CALL(database, insert(_)).Times(1)
                .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert));

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
    std::string datawriter_name = "DataWriter_topic_name_0.0.0.1";  //< Name constructed from the topic and entity_id
    Qos datawriter_qos;
    std::string datawriter_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
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

    // Precondition: The domain exists and has ID 0
    std::shared_ptr<Domain> domain;
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain));

    // Precondition: The participant exists and has ID 1
    std::string participant_name = "participant";
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(
        participant_name, Qos(), participant_guid_str, nullptr, domain);
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant));

    // Precondition: The topic does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: The locators exist and have ID 100 and 101
    std::shared_ptr<Locator> ulocator = std::make_shared<Locator>(unicast_locator_str);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, unicast_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(100)))));
    EXPECT_CALL(database, get_entity(EntityId(100))).Times(AnyNumber())
            .WillRepeatedly(Return(ulocator));

    std::shared_ptr<Locator> mlocator = std::make_shared<Locator>(multicast_locator_str);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, multicast_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(101)))));
    EXPECT_CALL(database, get_entity(EntityId(101))).Times(AnyNumber())
            .WillRepeatedly(Return(mlocator));

    // Datawriter discovery: SUCCESS
    {
        // Precondition: The writer does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, datawriter_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        // Expectation: The topic is created and given ID 2
        InsertEntityArgs topic_insert_args([&](
                    std::shared_ptr<Entity> entity)
                {
                    EXPECT_EQ(entity->kind, EntityKind::TOPIC);
                    EXPECT_EQ(entity->name, topic_name);
                    EXPECT_EQ(entity->alias, topic_name);
                    EXPECT_EQ(std::dynamic_pointer_cast<Topic>(entity)->data_type, type_name);

                    return EntityId(2);
                });

        // Expectation: The datawriter is created and given ID 3
        InsertEntityArgs writer_insert_args([&](
                    std::shared_ptr<Entity> entity)
                {
                    EXPECT_EQ(entity->kind, EntityKind::DATAWRITER);
                    EXPECT_EQ(entity->name, datawriter_name);
                    EXPECT_EQ(entity->alias, datawriter_name);
                    EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->guid, datawriter_guid_str);
                    EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->qos, datawriter_qos);

                    return EntityId(3);
                });

        EXPECT_CALL(database, insert(_)).Times(2)
                .WillOnce(Invoke(&topic_insert_args, &InsertEntityArgs::insert))
                .WillOnce(Invoke(&writer_insert_args, &InsertEntityArgs::insert));

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, add_endpoint_to_graph(_,_,_,_)).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_graph_update(_)).Times(1);

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

TEST_F(database_queue_tests, push_datawriter_locator_does_not_exist)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Create the writer info
    std::string datawriter_name = "DataWriter_topic_name_0.0.0.1";  //< Name constructed from the topic and entity_id
    Qos datawriter_qos;
    std::string datawriter_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
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

    // Precondition: The domain exists and has ID 0
    std::shared_ptr<Domain> domain;
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain));

    // Precondition: The participant exists and has ID 1
    std::string participant_name = "participant";
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(
        participant_name, Qos(), participant_guid_str, nullptr, domain);
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant));

    // Precondition: The topic exists and has ID 2
    std::shared_ptr<Topic> topic = std::make_shared<Topic>(topic_name, type_name, domain);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic));

    // Precondition: The locators do not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, unicast_locator_str)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()));
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, multicast_locator_str)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Datawriter discovery: SUCCESS
    {
        // Precondition: The writer does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, datawriter_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        // Expectation: The locator is created and given ID 100
        InsertEntityArgs unicast_insert_args([&](
                    std::shared_ptr<Entity> entity)
                {
                    EXPECT_EQ(entity->kind, EntityKind::LOCATOR);
                    EXPECT_EQ(entity->name, unicast_locator_str);
                    EXPECT_EQ(entity->alias, unicast_locator_str);

                    return EntityId(100);
                });

        // Expectation: The locator is created and given ID 101
        InsertEntityArgs multicast_insert_args([&](
                    std::shared_ptr<Entity> entity)
                {
                    EXPECT_EQ(entity->kind, EntityKind::LOCATOR);
                    EXPECT_EQ(entity->name, multicast_locator_str);
                    EXPECT_EQ(entity->alias, multicast_locator_str);

                    return EntityId(101);
                });

        // Expectation: The datawriter is created and given ID 3
        InsertEntityArgs writer_insert_args([&](
                    std::shared_ptr<Entity> entity)
                {
                    EXPECT_EQ(entity->kind, EntityKind::DATAWRITER);
                    EXPECT_EQ(entity->name, datawriter_name);
                    EXPECT_EQ(entity->alias, datawriter_name);
                    EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->guid, datawriter_guid_str);
                    EXPECT_EQ(std::dynamic_pointer_cast<DataWriter>(entity)->qos, datawriter_qos);

                    return EntityId(3);
                });

        EXPECT_CALL(database, insert(_)).Times(3)
                .WillOnce(Invoke(&unicast_insert_args, &InsertEntityArgs::insert))
                .WillOnce(Invoke(&multicast_insert_args, &InsertEntityArgs::insert))
                .WillOnce(Invoke(&writer_insert_args, &InsertEntityArgs::insert));

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, add_endpoint_to_graph(_,_,_,_)).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_graph_update(_)).Times(1);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(3), true)).Times(1);

        // Expectations: Request the backend to notify user (if needed)
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_physical_entity_discovery(EntityId(100), EntityKind::LOCATOR,
                details::StatisticsBackendData::DISCOVERY))
                .Times(1);

        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_physical_entity_discovery(EntityId(101), EntityKind::LOCATOR,
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
    std::string datareader_name = "DataReader_topic_name_0.0.0.2";  //< Name constructed from the topic and entity_id
    Qos datareader_qos;
    std::string datareader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
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

    // Precondition: The domain exists and has ID 0
    std::shared_ptr<Domain> domain;
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain));

    // Precondition: The participant exists and has ID 1
    std::string participant_name = "participant";
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(
        participant_name, Qos(), participant_guid_str, nullptr, domain);
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant));

    // Precondition: The topic exists and has ID 2
    std::shared_ptr<Topic> topic = std::make_shared<Topic>(topic_name, type_name, domain);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic));

    // Precondition: The locators exist and have ID 100 and 101
    std::shared_ptr<Locator> ulocator = std::make_shared<Locator>(unicast_locator_str);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, unicast_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(100)))));
    EXPECT_CALL(database, get_entity(EntityId(100))).Times(AnyNumber())
            .WillRepeatedly(Return(ulocator));

    std::shared_ptr<Locator> mlocator = std::make_shared<Locator>(multicast_locator_str);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, multicast_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(101)))));
    EXPECT_CALL(database, get_entity(EntityId(101))).Times(AnyNumber())
            .WillRepeatedly(Return(mlocator));

    // Datareader undiscovery: FAILURE
    {
        // Precondition: The reader does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, datareader_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        EXPECT_CALL(database, insert(_)).Times(0);

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, add_endpoint_to_graph(_,_,_,_)).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_graph_update(_)).Times(1);

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

        EXPECT_CALL(database, insert(_)).Times(0);

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
        InsertEntityArgs insert_args([&](
                    std::shared_ptr<Entity> entity)
                {
                    EXPECT_EQ(entity->kind, EntityKind::DATAREADER);
                    EXPECT_EQ(entity->name, datareader_name);
                    EXPECT_EQ(entity->alias, datareader_name);
                    EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->guid, datareader_guid_str);
                    EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->qos, datareader_qos);

                    return EntityId(3);
                });

        EXPECT_CALL(database, insert(_)).Times(1).WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert));

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

        EXPECT_CALL(database, insert(_)).Times(0);

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, delete_endpoint_from_graph(_,_,_,_)).Times(1).WillOnce(Return(false));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_graph_update(_)).Times(0);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(3), true)).Times(1);

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

        EXPECT_CALL(database, insert(_)).Times(0);

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, delete_endpoint_from_graph(_,_,_,_)).Times(1).WillOnce(Return(false));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_graph_update(_)).Times(0);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(3), false)).Times(1);

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
        InsertEntityArgs insert_args([&](
                    std::shared_ptr<Entity> entity) -> EntityId
                {
                    EXPECT_EQ(entity->kind, EntityKind::DATAREADER);
                    EXPECT_EQ(entity->name, datareader_name);
                    EXPECT_EQ(entity->alias, datareader_name);
                    EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->guid, datareader_guid_str);
                    EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->qos, datareader_qos);

                    throw BadParameter("Error");
                });

        EXPECT_CALL(database, insert(_)).Times(1)
                .WillOnce(Invoke(&insert_args, &InsertEntityArgs::insert));

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
    std::string datareader_name = "DataReader_topic_name_0.0.0.2";  //< Name constructed from the topic and entity_id
    Qos datareader_qos;
    std::string datareader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
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

    // Precondition: The domain exists and has ID 0
    std::shared_ptr<Domain> domain;
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain));

    // Precondition: The participant exists and has ID 1
    std::string participant_name = "participant";
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(
        participant_name, Qos(), participant_guid_str, nullptr, domain);
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant));

    // Precondition: The topic does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: The locators exist and have ID 100 and 101
    std::shared_ptr<Locator> ulocator = std::make_shared<Locator>(unicast_locator_str);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, unicast_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(100)))));
    EXPECT_CALL(database, get_entity(EntityId(100))).Times(AnyNumber())
            .WillRepeatedly(Return(ulocator));

    std::shared_ptr<Locator> mlocator = std::make_shared<Locator>(multicast_locator_str);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, multicast_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(101)))));
    EXPECT_CALL(database, get_entity(EntityId(101))).Times(AnyNumber())
            .WillRepeatedly(Return(mlocator));

    // Datareader discovery: SUCCESS
    {
        // Precondition: The reader does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, datareader_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        // Expectation: The topic is created and given ID 2
        InsertEntityArgs topic_insert_args([&](
                    std::shared_ptr<Entity> entity)
                {
                    EXPECT_EQ(entity->kind, EntityKind::TOPIC);
                    EXPECT_EQ(entity->name, topic_name);
                    EXPECT_EQ(entity->alias, topic_name);
                    EXPECT_EQ(std::dynamic_pointer_cast<Topic>(entity)->data_type, type_name);

                    return EntityId(2);
                });

        // Expectation: The datareader is created and given ID 3
        InsertEntityArgs reader_insert_args([&](
                    std::shared_ptr<Entity> entity)
                {
                    EXPECT_EQ(entity->kind, EntityKind::DATAREADER);
                    EXPECT_EQ(entity->name, datareader_name);
                    EXPECT_EQ(entity->alias, datareader_name);
                    EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->guid, datareader_guid_str);
                    EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->qos, datareader_qos);

                    return EntityId(3);
                });

        EXPECT_CALL(database, insert(_)).Times(2)
                .WillOnce(Invoke(&topic_insert_args, &InsertEntityArgs::insert))
                .WillOnce(Invoke(&reader_insert_args, &InsertEntityArgs::insert));

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, add_endpoint_to_graph(_,_,_,_)).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_graph_update(_)).Times(1);

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

TEST_F(database_queue_tests, push_datareader_locator_does_not_exist)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Create the reader info
    std::string datareader_name = "DataReader_topic_name_0.0.0.2";  //< Name constructed from the topic and entity_id
    Qos datareader_qos;
    std::string datareader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
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

    // Precondition: The domain exists and has ID 0
    std::shared_ptr<Domain> domain;
    EXPECT_CALL(database, get_entity(EntityId(0))).Times(AnyNumber())
            .WillRepeatedly(Return(domain));

    // Precondition: The participant exists and has ID 1
    std::string participant_name = "participant";
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1";
    std::shared_ptr<DomainParticipant> participant = std::make_shared<DomainParticipant>(
        participant_name, Qos(), participant_guid_str, nullptr, domain);
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));
    EXPECT_CALL(database, get_entity(EntityId(1))).Times(AnyNumber())
            .WillRepeatedly(Return(participant));

    // Precondition: The topic exists and has ID 2
    std::shared_ptr<Topic> topic = std::make_shared<Topic>(topic_name, type_name, domain);
    EXPECT_CALL(database, get_entities_by_name(EntityKind::TOPIC, topic_name)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillRepeatedly(Return(topic));

    // Precondition: The locators do not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, unicast_locator_str)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()));
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, multicast_locator_str)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Datareader discovery: SUCCESS
    {
        // Precondition: The reader does not exist
        EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, datareader_guid_str)).Times(AnyNumber())
                .WillOnce(Throw(BadParameter("Error")));

        // Expectation: The locator is created and given ID 100
        InsertEntityArgs unicast_insert_args([&](
                    std::shared_ptr<Entity> entity)
                {
                    EXPECT_EQ(entity->kind, EntityKind::LOCATOR);
                    EXPECT_EQ(entity->name, unicast_locator_str);
                    EXPECT_EQ(entity->alias, unicast_locator_str);

                    return EntityId(100);
                });

        // Expectation: The locator is created and given ID 101
        InsertEntityArgs multicast_insert_args([&](
                    std::shared_ptr<Entity> entity)
                {
                    EXPECT_EQ(entity->kind, EntityKind::LOCATOR);
                    EXPECT_EQ(entity->name, multicast_locator_str);
                    EXPECT_EQ(entity->alias, multicast_locator_str);

                    return EntityId(101);
                });

        // Expectation: The datareader is created and given ID 3
        InsertEntityArgs reader_insert_args([&](
                    std::shared_ptr<Entity> entity)
                {
                    EXPECT_EQ(entity->kind, EntityKind::DATAREADER);
                    EXPECT_EQ(entity->name, datareader_name);
                    EXPECT_EQ(entity->alias, datareader_name);
                    EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->guid, datareader_guid_str);
                    EXPECT_EQ(std::dynamic_pointer_cast<DataReader>(entity)->qos, datareader_qos);

                    return EntityId(3);
                });

        EXPECT_CALL(database, insert(_)).Times(3)
                .WillOnce(Invoke(&unicast_insert_args, &InsertEntityArgs::insert))
                .WillOnce(Invoke(&multicast_insert_args, &InsertEntityArgs::insert))
                .WillOnce(Invoke(&reader_insert_args, &InsertEntityArgs::insert));

        // Expectation: Modify graph and notify user
        EXPECT_CALL(database, add_endpoint_to_graph(_,_,_,_)).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_domain_graph_update(_)).Times(1);

        // Expectations: The status will be updated
        EXPECT_CALL(database, change_entity_status(EntityId(3), true)).Times(1);

        // Expectations: Request the backend to notify user (if needed)
        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_physical_entity_discovery(EntityId(100), EntityKind::LOCATOR,
                details::StatisticsBackendData::DISCOVERY))
                .Times(1);

        EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
                on_physical_entity_discovery(EntityId(101), EntityKind::LOCATOR,
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

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string reader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the reader GUID
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsWriterReaderData inner_data;
    inner_data.data(1.0);
    inner_data.writer_guid(writer_guid);
    inner_data.reader_guid(reader_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->writer_reader_data(inner_data);
    data->_d(EventKindBits::HISTORY2HISTORY_LATENCY);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(1)
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

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string reader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the reader GUID
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsWriterReaderData inner_data;
    inner_data.data(1.0);
    inner_data.writer_guid(writer_guid);
    inner_data.reader_guid(reader_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->writer_reader_data(inner_data);
    data->_d(EventKindBits::HISTORY2HISTORY_LATENCY);

    // Precondition: The writer exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is not called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_history_latency_no_writer)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string reader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the reader GUID
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsWriterReaderData inner_data;
    inner_data.data(1.0);
    inner_data.writer_guid(writer_guid);
    inner_data.reader_guid(reader_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->writer_reader_data(inner_data);
    data->_d(EventKindBits::HISTORY2HISTORY_LATENCY);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Precondition: The reader exists and has ID 2
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(2))));

    // Expectation: The insert method is not called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_network_latency)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 16> src_locator_address = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    uint32_t src_locator_port = 0;
    std::string src_locator_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|d.e.f.10";
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the source locator
    DatabaseDataQueue::StatisticsLocator src_locator;
    src_locator.kind(LOCATOR_KIND_TCPv4);
    src_locator.port(src_locator_port);
    src_locator.address(src_locator_address);

    // Build the destination locator
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsLocator2LocatorData inner_data;
    inner_data.data(1.0);
    inner_data.src_locator(src_locator);
    inner_data.dst_locator(dst_locator);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->locator2locator_data(inner_data);
    data->_d(EventKindBits::NETWORK_LATENCY);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(1)
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

    std::array<uint8_t, 16> src_locator_address = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    uint32_t src_locator_port = 0;
    std::string src_locator_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|d.e.f.10";
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the source locator
    DatabaseDataQueue::StatisticsLocator src_locator;
    src_locator.kind(LOCATOR_KIND_TCPv4);
    src_locator.port(src_locator_port);
    src_locator.address(src_locator_address);

    // Build the destination locator
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsLocator2LocatorData inner_data;
    inner_data.data(1.0);
    inner_data.src_locator(src_locator);
    inner_data.dst_locator(dst_locator);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->locator2locator_data(inner_data);
    data->_d(EventKindBits::NETWORK_LATENCY);

    // Precondition: The participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, src_locator_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Precondition: The destination locator exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_network_latency_wrong_participant_format)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 16> src_locator_address = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    uint32_t src_locator_port = 1;
    std::string src_locator_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|d.e.f.10";
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the source locator
    DatabaseDataQueue::StatisticsLocator src_locator;
    src_locator.kind(LOCATOR_KIND_TCPv4);
    src_locator.port(src_locator_port);
    src_locator.address(src_locator_address);

    // Build the destination locator
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsLocator2LocatorData inner_data;
    inner_data.data(1.0);
    inner_data.src_locator(src_locator);
    inner_data.dst_locator(dst_locator);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->locator2locator_data(inner_data);
    data->_d(EventKindBits::NETWORK_LATENCY);

    // Precondition: The participant is not searched
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, src_locator_str)).Times(0);

    // Precondition: The destination locator exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_network_latency_no_destination_locator)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 16> src_locator_address = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    uint32_t src_locator_port = 0;
    std::string src_locator_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|d.e.f.10";
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the source locator
    DatabaseDataQueue::StatisticsLocator src_locator;
    src_locator.kind(LOCATOR_KIND_TCPv4);
    src_locator.port(src_locator_port);
    src_locator.address(src_locator_address);

    // Build the destination locator
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsLocator2LocatorData inner_data;
    inner_data.data(1.0);
    inner_data.src_locator(src_locator);
    inner_data.dst_locator(dst_locator);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->locator2locator_data(inner_data);
    data->_d(EventKindBits::NETWORK_LATENCY);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(1)
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

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityData inner_data;
    inner_data.data(1.0);
    inner_data.guid(writer_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_data(inner_data);
    data->_d(EventKindBits::PUBLICATION_THROUGHPUT);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(1)
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

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityData inner_data;
    inner_data.data(1.0);
    inner_data.guid(writer_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_data(inner_data);
    data->_d(EventKindBits::PUBLICATION_THROUGHPUT);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_subscription_throughput)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityData inner_data;
    inner_data.data(1.0);
    inner_data.guid(reader_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_data(inner_data);
    data->_d(EventKindBits::SUBSCRIPTION_THROUGHPUT);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(1)
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Expectation: The user is notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_data_available(EntityId(0), EntityId(1), DataKind::SUBSCRIPTION_THROUGHPUT)).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_subscription_throughput_no_reder)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityData inner_data;
    inner_data.data(1.0);
    inner_data.guid(reader_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_data(inner_data);
    data->_d(EventKindBits::SUBSCRIPTION_THROUGHPUT);

    // Precondition: The reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_rtps_sent)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKindBits::RTPS_SENT);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(2)
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

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKindBits::RTPS_SENT);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Throw(BadParameter("Error")));

    // Precondition: The locator exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_rtps_sent_no_locator)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKindBits::RTPS_SENT);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(2)
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

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKindBits::RTPS_LOST);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(2)
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

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKindBits::RTPS_LOST);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Throw(BadParameter("Error")));

    // Precondition: The locator exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::LOCATOR, dst_locator_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_rtps_lost_no_locator)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntity2LocatorTraffic inner_data;
    inner_data.src_guid(writer_guid);
    inner_data.dst_locator(dst_locator);
    inner_data.packet_count(1024);
    inner_data.byte_count(2048);
    inner_data.byte_magnitude_order(10);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity2locator_traffic(inner_data);
    data->_d(EventKindBits::RTPS_LOST);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(2)
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

TEST_F(database_queue_tests, push_rtps_bytes_no_writer)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntity2LocatorTraffic inner_data;
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
    EXPECT_THROW(data_queue.do_process_sample_type(domain, entity, EntityKind::DATAWRITER, sample,
            inner_data), Error);
}

TEST_F(database_queue_tests, push_rtps_bytes_no_locator)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::array<uint8_t, 16> dst_locator_address = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    uint32_t dst_locator_port = 2048;
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    std::string dst_locator_str = "TCPv4:[4.3.2.1]:2048";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the locator address
    DatabaseDataQueue::StatisticsLocator dst_locator;
    dst_locator.kind(LOCATOR_KIND_TCPv4);
    dst_locator.port(dst_locator_port);
    dst_locator.address(dst_locator_address);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntity2LocatorTraffic inner_data;
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
    EXPECT_THROW(data_queue.do_process_sample_type(domain, entity, EntityKind::DATAWRITER, sample,
            inner_data), Error);
}

TEST_F(database_queue_tests, push_resent_datas)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKindBits::RESENT_DATAS);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(1)
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

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKindBits::RESENT_DATAS);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_heartbeat_count)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKindBits::HEARTBEAT_COUNT);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(1)
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

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKindBits::HEARTBEAT_COUNT);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_acknack_count)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(reader_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKindBits::ACKNACK_COUNT);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(1)
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

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(reader_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKindBits::ACKNACK_COUNT);

    // Precondition: The reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_nackfrag_count)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(reader_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKindBits::NACKFRAG_COUNT);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(1)
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

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> reader_id = {0, 0, 0, 1};
    std::string reader_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the reader GUID
    DatabaseDataQueue::StatisticsGuidPrefix reader_prefix;
    reader_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId reader_entity_id;
    reader_entity_id.value(reader_id);
    DatabaseDataQueue::StatisticsGuid reader_guid;
    reader_guid.guidPrefix(reader_prefix);
    reader_guid.entityId(reader_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(reader_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKindBits::NACKFRAG_COUNT);

    // Precondition: The reader does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAREADER, reader_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_gap_count)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKindBits::GAP_COUNT);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(1)
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

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKindBits::GAP_COUNT);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_data_count)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKindBits::DATA_COUNT);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(1)
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

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(writer_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKindBits::DATA_COUNT);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_pdp_count)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(participant_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKindBits::PDP_PACKETS);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(1)
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

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(participant_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKindBits::PDP_PACKETS);

    // Precondition: The participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_edp_count)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(participant_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKindBits::EDP_PACKETS);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(1)
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

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsEntityCount inner_data;
    inner_data.guid(participant_guid);
    inner_data.count(1024);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->entity_count(inner_data);
    data->_d(EventKindBits::EDP_PACKETS);

    // Precondition: The participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_discovery_times)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::array<uint8_t, 4> entity_id = {0, 0, 0, 1};
    // discovery time must be rounded to tenths of nanosecond to avoid truncation by
    // windows system_clock
    uint64_t discovery_time = 1000;
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";
    std::string remote_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";
    std::chrono::system_clock::time_point discovery_timestamp =
            eprosima::statistics_backend::nanoseconds_to_systemclock(discovery_time);

    // Build the participant GUID
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the remote GUID
    DatabaseDataQueue::StatisticsGuidPrefix remote_prefix;
    remote_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId remote_entity_id;
    remote_entity_id.value(entity_id);
    DatabaseDataQueue::StatisticsGuid remote_guid;
    remote_guid.guidPrefix(remote_prefix);
    remote_guid.entityId(remote_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsDiscoveryTime inner_data;
    inner_data.local_participant_guid(participant_guid);
    inner_data.remote_entity_guid(remote_guid);
    inner_data.time(discovery_time);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->discovery_time(inner_data);
    data->_d(EventKindBits::DISCOVERED_ENTITY);

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

    EXPECT_CALL(database, insert(_, _, _)).Times(1)
            .WillOnce(Invoke(&args, &InsertDataArgs::insert));

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_discovery_times_no_participant)
{
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::array<uint8_t, 4> entity_id = {0, 0, 0, 1};
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";
    std::string remote_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    long long discovery_time = 1024;
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    // Build the participant GUID
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the remote GUID
    DatabaseDataQueue::StatisticsGuidPrefix remote_prefix;
    remote_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId remote_entity_id;
    remote_entity_id.value(entity_id);
    DatabaseDataQueue::StatisticsGuid remote_guid;
    remote_guid.guidPrefix(remote_prefix);
    remote_guid.entityId(remote_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsDiscoveryTime inner_data;
    inner_data.local_participant_guid(participant_guid);
    inner_data.remote_entity_guid(remote_guid);
    inner_data.time(discovery_time);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->discovery_time(inner_data);
    data->_d(EventKindBits::DISCOVERED_ENTITY);

    // Precondition: The participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Precondition: The remote entity exists and has ID 2
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, remote_guid_str)).Times(AnyNumber())
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(2))));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_discovery_times_no_entity)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    std::array<uint8_t, 4> entity_id = {0, 0, 0, 1};
    uint64_t discovery_time = 1024;
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";
    std::string remote_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.1";

    // Build the participant GUID
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the remote GUID
    DatabaseDataQueue::StatisticsGuidPrefix remote_prefix;
    remote_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId remote_entity_id;
    remote_entity_id.value(entity_id);
    DatabaseDataQueue::StatisticsGuid remote_guid;
    remote_guid.guidPrefix(remote_prefix);
    remote_guid.entityId(remote_entity_id);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsDiscoveryTime inner_data;
    inner_data.local_participant_guid(participant_guid);
    inner_data.remote_entity_guid(remote_guid);
    inner_data.time(discovery_time);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->discovery_time(inner_data);
    data->_d(EventKindBits::DISCOVERED_ENTITY);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The remote entity does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, remote_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_sample_datas)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    int32_t sn_high = 2048;
    uint32_t sn_low = 4096;
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    eprosima::fastrtps::rtps::SequenceNumber_t sn (sn_high, sn_low);

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    DatabaseDataQueue::StatisticsSequenceNumber sequence_number;
    sequence_number.high(sn_high);
    sequence_number.low(sn_low);

    DatabaseDataQueue::StatisticsSampleIdentity sample_identity;
    sample_identity.writer_guid(writer_guid);
    sample_identity.sequence_number(sequence_number);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsSampleIdentityCount inner_data;
    inner_data.count(1024);
    inner_data.sample_id(sample_identity);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->sample_identity_count(inner_data);
    data->_d(EventKindBits::SAMPLE_DATAS);

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
    EXPECT_CALL(database, insert(_, _, _)).Times(1)
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

    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> writer_id = {0, 0, 0, 2};
    int32_t sn_high = 2048;
    uint32_t sn_low = 4096;
    std::string writer_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.2";
    eprosima::fastrtps::rtps::SequenceNumber_t sn (sn_high, sn_low);

    // Build the writer GUID
    DatabaseDataQueue::StatisticsGuidPrefix writer_prefix;
    writer_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId writer_entity_id;
    writer_entity_id.value(writer_id);
    DatabaseDataQueue::StatisticsGuid writer_guid;
    writer_guid.guidPrefix(writer_prefix);
    writer_guid.entityId(writer_entity_id);

    DatabaseDataQueue::StatisticsSequenceNumber sequence_number;
    sequence_number.high(sn_high);
    sequence_number.low(sn_low);

    DatabaseDataQueue::StatisticsSampleIdentity sample_identity;
    sample_identity.writer_guid(writer_guid);
    sample_identity.sequence_number(sequence_number);

    // Build the Statistics data
    DatabaseDataQueue::StatisticsSampleIdentityCount inner_data;
    inner_data.count(1024);
    inner_data.sample_id(sample_identity);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->sample_identity_count(inner_data);
    data->_d(EventKindBits::SAMPLE_DATAS);

    // Precondition: The writer does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::DATAWRITER, writer_guid_str)).Times(AnyNumber())
            .WillOnce(Throw(BadParameter("Error")));

    // Expectation: The insert method is never called, data dropped
    EXPECT_CALL(database, insert(_, _, _)).Times(0);

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_data_available(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_physical_data_no_participant_exists)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
    std::string processname = "command";
    std::string pid = "1234";
    std::string username = "user";
    std::string hostname = "host";
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the process name
    std::stringstream ss;
    ss << processname << ":" << pid;
    std::string processname_pid = ss.str();

    // Build the Statistics data
    DatabaseDataQueue::StatisticsPhysicalData inner_data;
    inner_data.host(hostname);
    inner_data.user(username);
    inner_data.process(processname_pid);
    inner_data.participant_guid(participant_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->physical_data(inner_data);
    data->_d(EventKindBits::PHYSICAL_DATA);

    // Precondition: The participant does not exist
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
            .WillOnce(Throw(BadParameter("Error")));

    // Precondition: The host exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::HOST, hostname)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(2)))));

    auto host = std::make_shared<Host>(hostname);
    host->id = EntityId(2);
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillOnce(Return(host));

    // Precondition: The user exists and has ID 3
    EXPECT_CALL(database, get_entities_by_name(EntityKind::USER, username)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(3)))));

    auto user = std::make_shared<User>(username, host);
    user->id = EntityId(3);
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillOnce(Return(user));

    // Precondition: The process exists and has ID 4
    EXPECT_CALL(database, get_entities_by_name(EntityKind::PROCESS, processname)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(4)))));

    auto process = std::make_shared<Process>(processname, pid, user);
    process->id = EntityId(4);
    EXPECT_CALL(database, get_entity(EntityId(4))).Times(AnyNumber())
            .WillOnce(Return(process));

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_physical_entity_discovery(_, _, _)).Times(0);

    // Add to the queue and wait to be processed
    // The processing should not progress the exception.
    data_queue.stop_consumer();
    data_queue.push(timestamp, data);
    data_queue.do_swap();
    EXPECT_NO_THROW(data_queue.consume_sample());
}

TEST_F(database_queue_tests, push_physical_data_no_process_exists)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
    std::string processname = "command";
    std::string pid = "1234";
    std::string username = "user";
    std::string hostname = "host";
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the process name
    std::stringstream ss;
    ss << processname << ":" << pid;
    std::string processname_pid = ss.str();

    // Build the Statistics data
    DatabaseDataQueue::StatisticsPhysicalData inner_data;
    inner_data.host(hostname);
    inner_data.user(username);
    inner_data.process(processname_pid);
    inner_data.participant_guid(participant_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->physical_data(inner_data);
    data->_d(EventKindBits::PHYSICAL_DATA);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The host exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::HOST, hostname)).Times(1)
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(2)))));

    auto host = std::make_shared<Host>(hostname);
    host->id = EntityId(2);
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(1)
            .WillOnce(Return(host));

    // Precondition: The user exists and has ID 3
    EXPECT_CALL(database, get_entities_by_name(EntityKind::USER, username)).Times(1)
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(3)))));

    auto user = std::make_shared<User>(username, host);
    user->id = EntityId(3);
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(1)
            .WillOnce(Return(user));

    // Precondition: The process does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::PROCESS, processname)).Times(1)
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Expectation: The process is created and given ID 4
    InsertEntityArgs insert_args_process([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::PROCESS);
                EXPECT_EQ(entity->name, processname);
                EXPECT_EQ(entity->alias, processname);
                EXPECT_EQ(std::dynamic_pointer_cast<Process>(entity)->pid, pid);
                EXPECT_EQ(std::dynamic_pointer_cast<Process>(entity)->user, user);

                return EntityId(4);
            });

    EXPECT_CALL(database, insert(_)).Times(1)
            .WillOnce(Invoke(&insert_args_process, &InsertEntityArgs::insert));

    // Expectation: The user is notified of the new process
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_physical_entity_discovery(EntityId(4), EntityKind::PROCESS,
            details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The link method is called with appropriate arguments
    EXPECT_CALL(database, link_participant_with_process(EntityId(1), EntityId(4))).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_physical_data_no_process_exists_process_insert_throws)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
    std::string processname = "command";
    std::string pid = "1234";
    std::string username = "user";
    std::string hostname = "host";
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the process name
    std::stringstream ss;
    ss << processname << ":" << pid;
    std::string processname_pid = ss.str();

    // Build the Statistics data
    DatabaseDataQueue::StatisticsPhysicalData inner_data;
    inner_data.host(hostname);
    inner_data.user(username);
    inner_data.process(processname_pid);
    inner_data.participant_guid(participant_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->physical_data(inner_data);
    data->_d(EventKindBits::PHYSICAL_DATA);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The host exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::HOST, hostname)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    auto host = std::make_shared<Host>(hostname);
    host->id = EntityId(2);
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(1)
            .WillOnce(Return(host));

    // Precondition: The user exists and has ID 3
    EXPECT_CALL(database, get_entities_by_name(EntityKind::USER, username)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(3)))));

    auto user = std::make_shared<User>(username, host);
    user->id = EntityId(3);
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(1)
            .WillOnce(Return(user));

    // Precondition: The process does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::PROCESS, processname)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Expectation: The process creation throws
    InsertEntityArgs insert_args_process([&](
                std::shared_ptr<Entity> entity) -> EntityId
            {
                EXPECT_EQ(entity->kind, EntityKind::PROCESS);
                EXPECT_EQ(entity->name, processname);
                EXPECT_EQ(entity->alias, processname);
                EXPECT_EQ(std::dynamic_pointer_cast<Process>(entity)->pid, pid);
                EXPECT_EQ(std::dynamic_pointer_cast<Process>(entity)->user, user);

                throw BadParameter("Error");
            });

    EXPECT_CALL(database, insert(_)).Times(1)
            .WillOnce(Invoke(&insert_args_process, &InsertEntityArgs::insert));

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_physical_entity_discovery(_, _, _)).Times(0);


    // Expectation: The link method is not called
    EXPECT_CALL(database, link_participant_with_process(EntityId(1), EntityId(4))).Times(0);

    // Add to the queue and wait to be processed
    data_queue.stop_consumer();
    data_queue.push(timestamp, data);
    data_queue.do_swap();

    EXPECT_NO_THROW(data_queue.consume_sample());
}

TEST_F(database_queue_tests, push_physical_data_no_process_no_user_exists)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
    std::string processname = "command";
    std::string pid = "1234";
    std::string username = "user";
    std::string hostname = "host";
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the process name
    std::stringstream ss;
    ss << processname << ":" << pid;
    std::string processname_pid = ss.str();

    // Build the Statistics data
    DatabaseDataQueue::StatisticsPhysicalData inner_data;
    inner_data.host(hostname);
    inner_data.user(username);
    inner_data.process(processname_pid);
    inner_data.participant_guid(participant_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->physical_data(inner_data);
    data->_d(EventKindBits::PHYSICAL_DATA);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The host exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::HOST, hostname)).Times(1)
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(2)))));

    auto host = std::make_shared<Host>(hostname);
    host->id = EntityId(2);
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(1)
            .WillOnce(Return(host));

    // Precondition: The user does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::USER, username)).Times(1)
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: The process does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::PROCESS, processname)).Times(1)
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Expectation: The user is created and given ID 3
    InsertEntityArgs insert_args_user([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::USER);
                EXPECT_EQ(entity->name, username);
                EXPECT_EQ(entity->alias, username);
                EXPECT_EQ(std::dynamic_pointer_cast<User>(entity)->host, host);

                return EntityId(3);
            });

    // Expectation: The user is notified of the new process
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_physical_entity_discovery(EntityId(3), EntityKind::USER,
            details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The process is created and given ID 4
    InsertEntityArgs insert_args_process([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::PROCESS);
                EXPECT_EQ(entity->name, processname);
                EXPECT_EQ(entity->alias, processname);
                EXPECT_EQ(std::dynamic_pointer_cast<Process>(entity)->pid, pid);
                EXPECT_EQ(std::dynamic_pointer_cast<Process>(entity)->user, insert_args_user.entity_);

                return EntityId(4);
            });

    // Expectation: The user is notified of the new process
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_physical_entity_discovery(EntityId(4), EntityKind::PROCESS,
            details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    EXPECT_CALL(database, insert(_)).Times(2)
            .WillOnce(Invoke(&insert_args_user, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_args_process, &InsertEntityArgs::insert));

    // Expectation: The link method is called with appropriate arguments
    EXPECT_CALL(database, link_participant_with_process(EntityId(1), EntityId(4))).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_physical_data_no_process_no_user_exists_user_insert_throws)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
    std::string processname = "command";
    std::string pid = "1234";
    std::string username = "user";
    std::string hostname = "host";
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the process name
    std::stringstream ss;
    ss << processname << ":" << pid;
    std::string processname_pid = ss.str();

    // Build the Statistics data
    DatabaseDataQueue::StatisticsPhysicalData inner_data;
    inner_data.host(hostname);
    inner_data.user(username);
    inner_data.process(processname_pid);
    inner_data.participant_guid(participant_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->physical_data(inner_data);
    data->_d(EventKindBits::PHYSICAL_DATA);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The host exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::HOST, hostname)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>(1,
            std::make_pair(EntityId(0), EntityId(2)))));

    auto host = std::make_shared<Host>(hostname);
    host->id = EntityId(2);
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(1)
            .WillOnce(Return(host));

    // Precondition: The user does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::USER, username)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: The process does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::PROCESS, processname)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Expectation: The user creation throws
    InsertEntityArgs insert_args_user([&](
                std::shared_ptr<Entity> entity) -> EntityId
            {
                EXPECT_EQ(entity->kind, EntityKind::USER);
                EXPECT_EQ(entity->name, username);
                EXPECT_EQ(entity->alias, username);
                EXPECT_EQ(std::dynamic_pointer_cast<User>(entity)->host, host);

                throw BadParameter("Error");
            });

    EXPECT_CALL(database, insert(_)).Times(1)
            .WillOnce(Invoke(&insert_args_user, &InsertEntityArgs::insert));

    // Expectation: The user is not notified of the new user
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_physical_entity_discovery(_, _, _)).Times(0);

    // Expectation: The link method is not called
    EXPECT_CALL(database, link_participant_with_process(EntityId(1), EntityId(4))).Times(0);

    // Add to the queue and wait to be processed
    data_queue.stop_consumer();
    data_queue.push(timestamp, data);
    data_queue.do_swap();

    EXPECT_NO_THROW(data_queue.consume_sample());
}

TEST_F(database_queue_tests, push_physical_data_no_process_no_user_no_host_exists)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
    std::string processname = "command";
    std::string pid = "1234";
    std::string username = "user";
    std::string hostname = "host";
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the process name
    std::stringstream ss;
    ss << processname << ":" << pid;
    std::string processname_pid = ss.str();

    // Build the Statistics data
    DatabaseDataQueue::StatisticsPhysicalData inner_data;
    inner_data.host(hostname);
    inner_data.user(username);
    inner_data.process(processname_pid);
    inner_data.participant_guid(participant_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->physical_data(inner_data);
    data->_d(EventKindBits::PHYSICAL_DATA);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The host does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::HOST, hostname)).Times(1)
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: The user does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::USER, username)).Times(1)
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: The process does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::PROCESS, processname)).Times(1)
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Expectation: The host is created and given ID 3
    InsertEntityArgs insert_args_host([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::HOST);
                EXPECT_EQ(entity->name, hostname);
                EXPECT_EQ(entity->alias, hostname);

                return EntityId(3);
            });

    // Expectation: The user is notified of the new host
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_physical_entity_discovery(EntityId(3), EntityKind::HOST,
            details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The user is created and given ID 4
    InsertEntityArgs insert_args_user([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::USER);
                EXPECT_EQ(entity->name, username);
                EXPECT_EQ(entity->alias, username);
                EXPECT_EQ(std::dynamic_pointer_cast<User>(entity)->host, insert_args_host.entity_);

                return EntityId(4);
            });

    // Expectation: The user is notified of the new user
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_physical_entity_discovery(EntityId(4), EntityKind::USER,
            details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The process is created and given ID 5
    InsertEntityArgs insert_args_process([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::PROCESS);
                EXPECT_EQ(entity->name, processname);
                EXPECT_EQ(entity->alias, processname);
                EXPECT_EQ(std::dynamic_pointer_cast<Process>(entity)->pid, pid);
                EXPECT_EQ(std::dynamic_pointer_cast<Process>(entity)->user, insert_args_user.entity_);

                return EntityId(5);
            });

    // Expectation: The user is notified of the new process
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_physical_entity_discovery(EntityId(5), EntityKind::PROCESS,
            details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    EXPECT_CALL(database, insert(_)).Times(3)
            .WillOnce(Invoke(&insert_args_host, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_args_user, &InsertEntityArgs::insert))
            .WillOnce(Invoke(&insert_args_process, &InsertEntityArgs::insert));

    // Expectation: The link method is called with appropriate arguments
    EXPECT_CALL(database, link_participant_with_process(EntityId(1), EntityId(5))).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

TEST_F(database_queue_tests, push_physical_data_no_process_no_user_no_host_exists_host_insert_throws)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
    std::string processname = "command";
    std::string pid = "1234";
    std::string username = "user";
    std::string hostname = "host";
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the process name
    std::stringstream ss;
    ss << processname << ":" << pid;
    std::string processname_pid = ss.str();

    // Build the Statistics data
    DatabaseDataQueue::StatisticsPhysicalData inner_data;
    inner_data.host(hostname);
    inner_data.user(username);
    inner_data.process(processname_pid);
    inner_data.participant_guid(participant_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->physical_data(inner_data);
    data->_d(EventKindBits::PHYSICAL_DATA);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(AnyNumber())
            .WillRepeatedly(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The host does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::HOST, hostname)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: The user does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::USER, username)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Precondition: The process does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::PROCESS, processname)).Times(AnyNumber())
            .WillRepeatedly(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Expectation: The host creation throws
    InsertEntityArgs insert_args_host([&](
                std::shared_ptr<Entity> entity) -> EntityId
            {
                EXPECT_EQ(entity->kind, EntityKind::HOST);
                EXPECT_EQ(entity->name, hostname);
                EXPECT_EQ(entity->alias, hostname);

                throw BadParameter("Error");
            });

    EXPECT_CALL(database, insert(_)).Times(1)
            .WillOnce(Invoke(&insert_args_host, &InsertEntityArgs::insert));

    // Expectation: The user is not notified
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(), on_physical_entity_discovery(_, _, _)).Times(0);

    // Expectation: The link method is not called
    EXPECT_CALL(database, link_participant_with_process(EntityId(1), EntityId(5))).Times(0);

    // Add to the queue and wait to be processed
    data_queue.stop_consumer();
    data_queue.push(timestamp, data);
    data_queue.do_swap();

    EXPECT_NO_THROW(data_queue.consume_sample());
}

TEST_F(database_queue_tests, push_physical_data_wrong_processname_format)
{
    std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
    std::string processname = "command";
    std::string pid = "1234";
    std::string username = "user";
    std::string hostname = "host";
    std::string participant_guid_str = "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.0.0";

    // Build the participant GUID
    std::array<uint8_t, 12> prefix = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    std::array<uint8_t, 4> participant_id = {0, 0, 0, 0};
    DatabaseDataQueue::StatisticsGuidPrefix participant_prefix;
    participant_prefix.value(prefix);
    DatabaseDataQueue::StatisticsEntityId participant_entity_id;
    participant_entity_id.value(participant_id);
    DatabaseDataQueue::StatisticsGuid participant_guid;
    participant_guid.guidPrefix(participant_prefix);
    participant_guid.entityId(participant_entity_id);

    // Build the process name with the wrong format
    std::stringstream ss;
    ss << processname << pid;
    std::string processname_pid = ss.str();

    // Build the Statistics data
    DatabaseDataQueue::StatisticsPhysicalData inner_data;
    inner_data.host(hostname);
    inner_data.user(username);
    inner_data.process(processname_pid);
    inner_data.participant_guid(participant_guid);

    std::shared_ptr<eprosima::fastdds::statistics::Data> data = std::make_shared<eprosima::fastdds::statistics::Data>();
    data->physical_data(inner_data);
    data->_d(EventKindBits::PHYSICAL_DATA);

    // Precondition: The participant exists and has ID 1
    EXPECT_CALL(database, get_entity_by_guid(EntityKind::PARTICIPANT, participant_guid_str)).Times(1)
            .WillOnce(Return(std::make_pair(EntityId(0), EntityId(1))));

    // Precondition: The host exists and has ID 2
    EXPECT_CALL(database, get_entities_by_name(EntityKind::HOST, hostname)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(2)))));

    auto host = std::make_shared<Host>(hostname);
    host->id = EntityId(2);
    EXPECT_CALL(database, get_entity(EntityId(2))).Times(AnyNumber())
            .WillOnce(Return(host));

    // Precondition: The user exists and has ID 3
    EXPECT_CALL(database, get_entities_by_name(EntityKind::USER, username)).Times(AnyNumber())
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>(1, std::make_pair(EntityId(0), EntityId(3)))));

    auto user = std::make_shared<User>(username, host);
    user->id = EntityId(3);
    EXPECT_CALL(database, get_entity(EntityId(3))).Times(AnyNumber())
            .WillOnce(Return(user));

    // Precondition: The process does not exist
    EXPECT_CALL(database, get_entities_by_name(EntityKind::PROCESS, processname_pid)).Times(1)
            .WillOnce(Return(std::vector<std::pair<EntityId, EntityId>>()));

    // Expectation: The process is created and given ID 4
    InsertEntityArgs insert_args_process([&](
                std::shared_ptr<Entity> entity)
            {
                EXPECT_EQ(entity->kind, EntityKind::PROCESS);
                EXPECT_EQ(entity->name, processname_pid);
                EXPECT_EQ(entity->alias, processname_pid);
                EXPECT_EQ(std::dynamic_pointer_cast<Process>(entity)->pid, processname_pid);
                EXPECT_EQ(std::dynamic_pointer_cast<Process>(entity)->user, user);

                return EntityId(4);
            });

    EXPECT_CALL(database, insert(_)).Times(1)
            .WillOnce(Invoke(&insert_args_process, &InsertEntityArgs::insert));

    // Expectation: The user is notified of the new process
    EXPECT_CALL(*details::StatisticsBackendData::get_instance(),
            on_physical_entity_discovery(EntityId(4), EntityKind::PROCESS,
            details::StatisticsBackendData::DiscoveryStatus::DISCOVERY)).Times(1);

    // Expectation: The link method is called with appropriate arguments
    EXPECT_CALL(database, link_participant_with_process(EntityId(1), EntityId(4))).Times(1);

    // Add to the queue and wait to be processed
    data_queue.push(timestamp, data);
    data_queue.flush();
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
