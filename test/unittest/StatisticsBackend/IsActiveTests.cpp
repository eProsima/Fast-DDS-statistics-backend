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

#include <gtest/gtest.h>

#include "fastdds/dds/domain/DomainParticipant.hpp"
#include <fastdds/dds/core/status/StatusMask.hpp>

#include <database/database.hpp>
#include <database/database_queue.hpp>
#include <DatabaseUtils.hpp>
#include <subscriber/StatisticsParticipantListener.hpp>
#include <topic_types/types.h>

using namespace eprosima::statistics_backend::subscriber;

constexpr const char* EMPTY_ENTITIES_DUMP_FILE = "resources/empty_entities_dump.json";

/**
 * @brief Fixture for the is_active_tests
 */
class is_active_tests : public ::testing::Test
{
public:

    void SetUp()
    {
        db.load_database(load_file(EMPTY_ENTITIES_DUMP_FILE));
        host = db.hosts().begin()->second;
        user = db.users().begin()->second;
        process = db.processes().begin()->second;
        domain = db.domains().begin()->second;
        topic = db.topics().begin()->second.begin()->second;
        participant = db.participants().begin()->second.begin()->second;
        datawriter = db.get_dds_endpoints<DataWriter>().begin()->second.begin()->second;
        datareader = db.get_dds_endpoints<DataReader>().begin()->second.begin()->second;
        locator = db.locators().begin()->second;
    }

    std::shared_ptr<Host> host;
    std::shared_ptr<User> user;
    std::shared_ptr<Process> process;
    std::shared_ptr<Domain> domain;
    std::shared_ptr<Topic> topic;
    std::shared_ptr<DomainParticipant> participant;
    std::shared_ptr<DataWriter> datawriter;
    std::shared_ptr<DataReader> datareader;
    std::shared_ptr<Locator> locator;

    DataBaseTest db;
};

// Check the is_active StatisticsBackend method when a participant is undiscovered
TEST_F(is_active_tests, participant_undiscovered)
{
    StatisticsBackendTest::set_database(&db);

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));

    // Entity queue, attached to the database
    DatabaseEntityQueue entity_queue(&db);

    // Data queue, attached to the database
    DatabaseDataQueue data_queue(&db);

    // Statistics participant_, that is supposed to receive the callbacks
    eprosima::fastdds::dds::DomainParticipant statistics_participant;

    // Listener under tests. Will receive a pointer to statistics_participant
    StatisticsParticipantListener participant_listener(domain->id, &db, &entity_queue, &data_queue);

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::RTPSParticipantAllocationAttributes allocation;
    eprosima::fastrtps::rtps::ParticipantProxyData data(allocation);

    // Precondition: The discovered participant has the given GUID and name
    eprosima::fastrtps::rtps::GUID_t participant_guid_;
    std::stringstream(participant->guid) >> participant_guid_;
    data.m_guid = participant_guid_;
    data.m_participantName = participant->name;

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo info(data);
    info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT;

    // Execution: Call the listener
    participant_listener.on_participant_discovery(&statistics_participant, std::move(info));

    ASSERT_FALSE(StatisticsBackendTest::is_active(host->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(user->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(process->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));
}

// Check the is_active StatisticsBackend method when a datawriter is undiscovered
TEST_F(is_active_tests, datawriter_undiscovered)
{
    StatisticsBackendTest::set_database(&db);

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));

    // Entity queue, attached to the database
    DatabaseEntityQueue entity_queue(&db);

    // Data queue, attached to the database
    DatabaseDataQueue data_queue(&db);

    // Statistics participant_, that is supposed to receive the callbacks
    eprosima::fastdds::dds::DomainParticipant statistics_participant;

    // Listener under tests. Will receive a pointer to statistics_participant
    StatisticsParticipantListener participant_listener(domain->id, &db, &entity_queue, &data_queue);

    // Start building the discovered writer info
    eprosima::fastrtps::rtps::WriterProxyData data(1, 1);

    // The discovered writer is in the participant
    eprosima::fastrtps::rtps::GUID_t writer_guid_;
    std::stringstream(datawriter->guid) >> writer_guid_;
    data.guid(writer_guid_);

    // The discovered writer is in the topic
    data.topicName(topic->name);
    data.typeName(topic->data_type);

    // The discovered writer contains the locator
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered writer info
    eprosima::fastrtps::rtps::WriterDiscoveryInfo info(data);
    info.status = eprosima::fastrtps::rtps::WriterDiscoveryInfo::REMOVED_WRITER;

    // Execution: Call the listener
    participant_listener.on_publisher_discovery(&statistics_participant, std::move(info));

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));
}

// Check the is_active StatisticsBackend method when a datareader is undiscovered
TEST_F(is_active_tests, datareader_undiscovered)
{
    StatisticsBackendTest::set_database(&db);

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));

    // Entity queue, attached to the database
    DatabaseEntityQueue entity_queue(&db);

    // Data queue, attached to the database
    DatabaseDataQueue data_queue(&db);

    // Statistics participant_, that is supposed to receive the callbacks
    eprosima::fastdds::dds::DomainParticipant statistics_participant;

    // Listener under tests. Will receive a pointer to statistics_participant
    StatisticsParticipantListener participant_listener(domain->id, &db, &entity_queue, &data_queue);

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::ReaderProxyData data(1, 1);

    // The discovered reader is in the participant
    eprosima::fastrtps::rtps::GUID_t reader_guid_;
    std::stringstream(datareader->guid) >> reader_guid_;
    data.guid(reader_guid_);

    // The discovered reader is in the topic
    data.topicName(topic->name);
    data.typeName(topic->data_type);

    // The discovered reader contains the locator
    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;
    data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ReaderDiscoveryInfo info(data);
    info.status = eprosima::fastrtps::rtps::ReaderDiscoveryInfo::REMOVED_READER;

    // Execution: Call the listener
    participant_listener.on_subscriber_discovery(&statistics_participant, std::move(info));

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));
}

// Check the is_active StatisticsBackend method when the endpoints are undiscovered
TEST_F(is_active_tests, endpoints_undiscovered)
{
    StatisticsBackendTest::set_database(&db);

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));

    // Entity queue, attached to the database
    DatabaseEntityQueue entity_queue(&db);

    // Data queue, attached to the database
    DatabaseDataQueue data_queue(&db);

    // Statistics participant_, that is supposed to receive the callbacks
    eprosima::fastdds::dds::DomainParticipant statistics_participant;

    // Listener under tests. Will receive a pointer to statistics_participant
    StatisticsParticipantListener participant_listener(domain->id, &db, &entity_queue, &data_queue);

    // Writer
    {
        // Start building the discovered writer info
        eprosima::fastrtps::rtps::WriterProxyData data(1, 1);

        // The discovered writer is in the participant
        eprosima::fastrtps::rtps::GUID_t writer_guid_;
        std::stringstream(datawriter->guid) >> writer_guid_;
        data.guid(writer_guid_);

        // The discovered writer is in the topic
        data.topicName(topic->name);
        data.typeName(topic->data_type);

        // The discovered writer contains the locator
        eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
        dds_existing_unicast_locator.address[12] = 127;
        dds_existing_unicast_locator.address[15] = 1;
        data.add_unicast_locator(dds_existing_unicast_locator);

        // Finish building the discovered writer info
        eprosima::fastrtps::rtps::WriterDiscoveryInfo info(data);
        info.status = eprosima::fastrtps::rtps::WriterDiscoveryInfo::REMOVED_WRITER;

        // Execution: Call the listener
        participant_listener.on_publisher_discovery(&statistics_participant, std::move(info));
    }

    // Reader
    {
        // Start building the discovered reader info
        eprosima::fastrtps::rtps::ReaderProxyData data(1, 1);

        // The discovered reader is in the participant
        eprosima::fastrtps::rtps::GUID_t reader_guid_;
        std::stringstream(datareader->guid) >> reader_guid_;
        data.guid(reader_guid_);

        // The discovered reader is in the topic
        data.topicName(topic->name);
        data.typeName(topic->data_type);

        // The discovered reader contains the locator
        eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
        dds_existing_unicast_locator.address[12] = 127;
        dds_existing_unicast_locator.address[15] = 1;
        data.add_unicast_locator(dds_existing_unicast_locator);

        // Finish building the discovered reader info
        eprosima::fastrtps::rtps::ReaderDiscoveryInfo info(data);
        info.status = eprosima::fastrtps::rtps::ReaderDiscoveryInfo::REMOVED_READER;

        // Execution: Call the listener
        participant_listener.on_subscriber_discovery(&statistics_participant, std::move(info));
    }

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));
}

// Check the is_active StatisticsBackend method when the dds_entities are undiscovered
TEST_F(is_active_tests, dds_entities_undiscovered)
{
    StatisticsBackendTest::set_database(&db);

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));

    // Entity queue, attached to the database
    DatabaseEntityQueue entity_queue(&db);

    // Data queue, attached to the database
    DatabaseDataQueue data_queue(&db);

    // Statistics participant_, that is supposed to receive the callbacks
    eprosima::fastdds::dds::DomainParticipant statistics_participant;

    // Listener under tests. Will receive a pointer to statistics_participant
    StatisticsParticipantListener participant_listener(domain->id, &db, &entity_queue, &data_queue);

    // Participant
    {
        // Start building the discovered reader info
        eprosima::fastrtps::rtps::RTPSParticipantAllocationAttributes allocation;
        eprosima::fastrtps::rtps::ParticipantProxyData data(allocation);

        // Precondition: The discovered participant has the given GUID and name
        eprosima::fastrtps::rtps::GUID_t participant_guid_;
        std::stringstream(participant->guid) >> participant_guid_;
        data.m_guid = participant_guid_;
        data.m_participantName = participant->name;

        // Finish building the discovered reader info
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo info(data);
        info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT;

        // Execution: Call the listener
        participant_listener.on_participant_discovery(&statistics_participant, std::move(info));
    }

    // Writer
    {
        // Start building the discovered writer info
        eprosima::fastrtps::rtps::WriterProxyData data(1, 1);

        // The discovered writer is in the participant
        eprosima::fastrtps::rtps::GUID_t writer_guid_;
        std::stringstream(datawriter->guid) >> writer_guid_;
        data.guid(writer_guid_);

        // The discovered writer is in the topic
        data.topicName(topic->name);
        data.typeName(topic->data_type);

        // The discovered writer contains the locator
        eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
        dds_existing_unicast_locator.address[12] = 127;
        dds_existing_unicast_locator.address[15] = 1;
        data.add_unicast_locator(dds_existing_unicast_locator);

        // Finish building the discovered writer info
        eprosima::fastrtps::rtps::WriterDiscoveryInfo info(data);
        info.status = eprosima::fastrtps::rtps::WriterDiscoveryInfo::REMOVED_WRITER;

        // Execution: Call the listener
        participant_listener.on_publisher_discovery(&statistics_participant, std::move(info));
    }

    // Reader
    {
        // Start building the discovered reader info
        eprosima::fastrtps::rtps::ReaderProxyData data(1, 1);

        // The discovered reader is in the participant
        eprosima::fastrtps::rtps::GUID_t reader_guid_;
        std::stringstream(datareader->guid) >> reader_guid_;
        data.guid(reader_guid_);

        // The discovered reader is in the topic
        data.topicName(topic->name);
        data.typeName(topic->data_type);

        // The discovered reader contains the locator
        eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
        dds_existing_unicast_locator.address[12] = 127;
        dds_existing_unicast_locator.address[15] = 1;
        data.add_unicast_locator(dds_existing_unicast_locator);

        // Finish building the discovered reader info
        eprosima::fastrtps::rtps::ReaderDiscoveryInfo info(data);
        info.status = eprosima::fastrtps::rtps::ReaderDiscoveryInfo::REMOVED_READER;

        // Execution: Call the listener
        participant_listener.on_subscriber_discovery(&statistics_participant, std::move(info));
    }

    ASSERT_FALSE(StatisticsBackendTest::is_active(host->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(user->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(process->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));
}

int main(

    int argc,
    char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
