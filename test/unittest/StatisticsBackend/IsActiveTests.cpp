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

#include <gtest_aux.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "fastdds/dds/domain/DomainParticipant.hpp"

#include <database/database.hpp>
#include <database/database_queue.hpp>
#include <DatabaseUtils.hpp>
#include <subscriber/StatisticsParticipantListener.hpp>

using namespace eprosima::statistics_backend::subscriber;

/**
 * @brief Fixture for the is_active_tests
 * - Create a database loading it from a file.
 * - Allocate the database in the Backend
 */
class is_active_tests : public ::testing::Test
{
public:

    void SetUp()
    {
        db = new DataBaseTest;
        DatabaseDump dump;
        load_file(EMPTY_ENTITIES_DUMP_FILE, dump);
        db->load_database(dump);
        host = db->hosts().begin()->second;
        user = db->users().begin()->second;
        process = db->processes().begin()->second;
        domain = db->domains().begin()->second;
        topic = db->topics().begin()->second.begin()->second;
        participant = db->participants().begin()->second.begin()->second;
        // Jump the metatraffic datawriter.
        datawriter = db->get_dds_endpoints<DataWriter>().begin()->second.begin()->second;
        datareader = db->get_dds_endpoints<DataReader>().begin()->second.begin()->second;
        locator = db->locators().begin()->second;

        // Allocating the db into the Backend make that is not necessary to delete the db.
        StatisticsBackendTest::set_database(db);

        entity_queue = new DatabaseEntityQueue(db);
        data_queue = new DatabaseDataQueue(db);
        participant_listener = new StatisticsParticipantListener(domain->id, db, entity_queue, data_queue);

        // Simulate that the backend is monitorizing the domain
        std::shared_ptr<details::Monitor> monitor = std::make_shared<details::Monitor>();
        monitor->id = domain->id;
        details::StatisticsBackendData::get_instance()->monitors_by_entity_[domain->id] = monitor;

        // Simulate the discover of the entitiy
        host->active = false;
        db->change_entity_status_test(host->id, true, domain->id);
        user->active = false;
        db->change_entity_status_test(user->id, true, domain->id);
        process->active = false;
        db->change_entity_status_test(process->id, true, domain->id);
        topic->active = false;
        db->change_entity_status_test(topic->id, true, domain->id);

        // Simulate the discover of the entities
        details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
            host->id,
            host->kind, details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
        details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
            user->id,
            user->kind, details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
        details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
            process->id,
            process->kind, details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
        details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(domain->id,
                topic->id,
                topic->kind,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
        details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(domain->id,
                participant->id,
                participant->kind,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
        details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(domain->id,
                datawriter->id,
                datawriter->kind,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
        details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(domain->id,
                datareader->id,
                datareader->kind,
                details::StatisticsBackendData::DiscoveryStatus::DISCOVERY);
    }

    void TearDown()
    {
        delete entity_queue;
        delete data_queue;
        delete participant_listener;
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

    DataBaseTest* db;

    // Entity queue, attached to the database
    DatabaseEntityQueue* entity_queue;
    // Data queue, attached to the database
    DatabaseDataQueue* data_queue;
    // Statistics participant_, that is supposed to receive the callbacks
    eprosima::fastdds::dds::DomainParticipant statistics_participant;
    // Listener under tests. Will receive a pointer to statistics_participant
    StatisticsParticipantListener* participant_listener;
};

// Windows dll do not export ParticipantProxyData class members (private APIs)
#if !defined(_WIN32)

// Check the is_active StatisticsBackend method when a participant is undiscovered
TEST_F(is_active_tests, participant)
{
    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));

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
    participant_listener->on_participant_discovery(&statistics_participant, std::move(info));

    ASSERT_FALSE(StatisticsBackendTest::is_active(host->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(user->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ParticipantDiscoveryInfo discover_info(data);
    info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT;

    // Execution: Call the listener
    participant_listener->on_participant_discovery(&statistics_participant, std::move(discover_info));

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));
}
#endif // !defined(_WIN32)

// Check the is_active StatisticsBackend method when a datawriter is undiscovered
TEST_F(is_active_tests, datawriter)
{
    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));

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
    participant_listener->on_publisher_discovery(&statistics_participant, std::move(info));

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));

    // Finish building the discovered writer info
    eprosima::fastrtps::rtps::WriterDiscoveryInfo discover_info(data);
    discover_info.status = eprosima::fastrtps::rtps::WriterDiscoveryInfo::DISCOVERED_WRITER;

    // Execution: Call the listener
    participant_listener->on_publisher_discovery(&statistics_participant, std::move(discover_info));

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));
}

// Check the is_active StatisticsBackend method when a datareader is undiscovered
TEST_F(is_active_tests, datareader)
{
    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));

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
    participant_listener->on_subscriber_discovery(&statistics_participant, std::move(info));

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));

    // Finish building the discovered writer info
    eprosima::fastrtps::rtps::ReaderDiscoveryInfo discover_info(data);
    discover_info.status = eprosima::fastrtps::rtps::ReaderDiscoveryInfo::DISCOVERED_READER;

    // Execution: Call the listener
    participant_listener->on_subscriber_discovery(&statistics_participant, std::move(discover_info));

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));
}

// Check the is_active StatisticsBackend method when the endpoints are undiscovered
TEST_F(is_active_tests, endpoints)
{
    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));

    eprosima::fastrtps::rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
    dds_existing_unicast_locator.address[12] = 127;
    dds_existing_unicast_locator.address[15] = 1;

    // Start building the discovered writer info
    eprosima::fastrtps::rtps::WriterProxyData writer_data(1, 1);

    // The discovered writer is in the participant
    eprosima::fastrtps::rtps::GUID_t writer_guid_;
    std::stringstream(datawriter->guid) >> writer_guid_;
    writer_data.guid(writer_guid_);

    // The discovered writer is in the topic
    writer_data.topicName(topic->name);
    writer_data.typeName(topic->data_type);

    // The discovered writer contains the locator
    writer_data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered writer info
    eprosima::fastrtps::rtps::WriterDiscoveryInfo writer_info(writer_data);
    writer_info.status = eprosima::fastrtps::rtps::WriterDiscoveryInfo::REMOVED_WRITER;

    // Execution: Call the listener
    participant_listener->on_publisher_discovery(&statistics_participant, std::move(writer_info));

    // Start building the discovered reader info
    eprosima::fastrtps::rtps::ReaderProxyData reader_data(1, 1);

    // The discovered reader is in the participant
    eprosima::fastrtps::rtps::GUID_t reader_guid_;
    std::stringstream(datareader->guid) >> reader_guid_;
    reader_data.guid(reader_guid_);

    // The discovered reader is in the topic
    reader_data.topicName(topic->name);
    reader_data.typeName(topic->data_type);

    // The discovered reader contains the locator
    reader_data.add_unicast_locator(dds_existing_unicast_locator);

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ReaderDiscoveryInfo reader_info(reader_data);
    reader_info.status = eprosima::fastrtps::rtps::ReaderDiscoveryInfo::REMOVED_READER;

    // Execution: Call the listener
    participant_listener->on_subscriber_discovery(&statistics_participant, std::move(reader_info));

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));

    // Finish building the discovered writer info
    eprosima::fastrtps::rtps::WriterDiscoveryInfo writer_discover_info(writer_data);
    writer_discover_info.status = eprosima::fastrtps::rtps::WriterDiscoveryInfo::DISCOVERED_WRITER;

    // Finish building the discovered reader info
    eprosima::fastrtps::rtps::ReaderDiscoveryInfo reader_discover_info(reader_data);
    reader_discover_info.status = eprosima::fastrtps::rtps::ReaderDiscoveryInfo::DISCOVERED_READER;

    // Execution: Call the listener
    participant_listener->on_publisher_discovery(&statistics_participant, std::move(writer_discover_info));

    // Execution: Call the listener
    participant_listener->on_subscriber_discovery(&statistics_participant, std::move(reader_discover_info));

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));
}

// Windows dll do not export ParticipantProxyData class members (private APIs)
#if !defined(_WIN32)

// Check the is_active StatisticsBackend method discover a datawriter on an inactive domain
TEST_F(is_active_tests, discover_datawriter_on_inactive_domain)
{
    // Participant undiscovered
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
        participant_listener->on_participant_discovery(&statistics_participant, std::move(info));
    }
    // Datawriter undiscovered
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
        participant_listener->on_publisher_discovery(&statistics_participant, std::move(info));
    }
    // Datareader undiscovered
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
        participant_listener->on_subscriber_discovery(&statistics_participant, std::move(info));
    }

    ASSERT_FALSE(StatisticsBackendTest::is_active(host->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(user->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));

    // Create new process
    auto process1 = std::make_shared<Process>("process1", "123", user);
    db->insert(process1);

    // Discover new participant
    {
        // Start building the discovered reader info
        eprosima::fastrtps::rtps::RTPSParticipantAllocationAttributes allocation;
        eprosima::fastrtps::rtps::ParticipantProxyData data(allocation);

        // Precondition: The discovered participant has the given GUID and name
        eprosima::fastrtps::rtps::GUID_t participant_guid_;
        std::stringstream("01.0f.00.00.00.00.00.00.00.00.00.01|0.0.1.c1") >> participant_guid_;
        data.m_guid = participant_guid_;
        data.m_participantName = participant->name + "_1";

        // Finish building the discovered reader info
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo info(data);
        info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT;

        // Execution: Call the listener
        participant_listener->on_participant_discovery(&statistics_participant, std::move(info));
    }

    // Link participant - process
    auto participant_id =
            db->get_entity_by_guid(EntityKind::PARTICIPANT, "01.0f.00.00.00.00.00.00.00.00.00.01|0.0.1.c1").second;
    db->link_participant_with_process(participant_id, process1->id);

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant_id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process1->id));

    // Discover new datawriter
    {
        // Start building the discovered writer info
        eprosima::fastrtps::rtps::WriterProxyData data(1, 1);

        // The discovered writer is in the participant
        eprosima::fastrtps::rtps::GUID_t writer_guid_;
        std::stringstream("01.0f.00.00.00.00.00.00.00.00.00.01|0.0.0.0") >> writer_guid_;
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
        info.status = eprosima::fastrtps::rtps::WriterDiscoveryInfo::DISCOVERED_WRITER;

        // Execution: Call the listener
        participant_listener->on_publisher_discovery(&statistics_participant, std::move(info));
    }

    auto datawriter_id =
            db->get_entity_by_guid(EntityKind::DATAWRITER, "01.0f.00.00.00.00.00.00.00.00.00.01|0.0.0.0").second;

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant_id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process1->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter_id));
}

// Check the is_active StatisticsBackend method discover a datareader on an inactive domain
TEST_F(is_active_tests, discover_datareader_on_inactive_domain)
{
    // Participant undiscovered
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
        participant_listener->on_participant_discovery(&statistics_participant, std::move(info));
    }
    // Datareader undiscovered
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
        participant_listener->on_subscriber_discovery(&statistics_participant, std::move(info));
    }
    // Datawriter undiscovered
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
        participant_listener->on_publisher_discovery(&statistics_participant, std::move(info));
    }

    ASSERT_FALSE(StatisticsBackendTest::is_active(host->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(user->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));

    // Create new process
    auto process1 = std::make_shared<Process>("process1", "123", user);
    db->insert(process1);

    // Discover new participant
    {
        // Start building the discovered reader info
        eprosima::fastrtps::rtps::RTPSParticipantAllocationAttributes allocation;
        eprosima::fastrtps::rtps::ParticipantProxyData data(allocation);

        // Precondition: The discovered participant has the given GUID and name
        eprosima::fastrtps::rtps::GUID_t participant_guid_;
        std::stringstream("01.0f.00.00.00.00.00.00.00.00.00.01|0.0.1.c1") >> participant_guid_;
        data.m_guid = participant_guid_;
        data.m_participantName = participant->name + "_1";

        // Finish building the discovered reader info
        eprosima::fastrtps::rtps::ParticipantDiscoveryInfo info(data);
        info.status = eprosima::fastrtps::rtps::ParticipantDiscoveryInfo::DISCOVERED_PARTICIPANT;

        // Execution: Call the listener
        participant_listener->on_participant_discovery(&statistics_participant, std::move(info));
    }

    // Link participant - process
    auto participant_id =
            db->get_entity_by_guid(EntityKind::PARTICIPANT, "01.0f.00.00.00.00.00.00.00.00.00.01|0.0.1.c1").second;
    db->link_participant_with_process(participant_id, process1->id);

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant_id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process1->id));

    // Discover new datareader
    {
        // Start building the discovered reader info
        eprosima::fastrtps::rtps::ReaderProxyData data(1, 1);

        // The discovered reader is in the participant
        eprosima::fastrtps::rtps::GUID_t reader_guid_;
        std::stringstream("01.0f.00.00.00.00.00.00.00.00.00.01|0.0.0.0") >> reader_guid_;
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
        info.status = eprosima::fastrtps::rtps::ReaderDiscoveryInfo::DISCOVERED_READER;

        // Execution: Call the listener
        participant_listener->on_subscriber_discovery(&statistics_participant, std::move(info));
    }

    auto datareader_id =
            db->get_entity_by_guid(EntityKind::DATAREADER, "01.0f.00.00.00.00.00.00.00.00.00.01|0.0.0.0").second;

    ASSERT_TRUE(StatisticsBackendTest::is_active(host->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(user->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(process->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(domain->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datareader->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(locator->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant_id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(process1->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader_id));
}

#endif // !defined(_WIN32)

int main(

        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
