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

bool StatisticsBackendTest::has_database_been_set_ = false;

/**
 * @brief Fixture for the clear_inactive_entities_tests
 * - Create a database loading it from a file.
 * - Allocate the database in the Backend
 */
class clear_inactive_entities_tests : public ::testing::Test
{
public:

    void SetUp()
    {
        db = new DataBaseTest;  // This will be deleted inside StatisticsBackendTest unset_database
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
        data_queue = new DatabaseDataQueue<eprosima::fastdds::statistics::Data>(db);
        monitor_service_data_queue = new DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>(db);
        participant_listener = new StatisticsParticipantListener(domain->id, db, entity_queue, data_queue,
                        monitor_service_data_queue);

        // Simulate that the backend is monitorizing the domain
        std::unique_ptr<details::Monitor> monitor = std::make_unique<details::Monitor>();
        monitor->id = domain->id;
        details::StatisticsBackendData::get_instance()->monitors_by_entity_[domain->id] = std::move(monitor);

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
        delete monitor_service_data_queue;
        delete participant_listener;

        if (!StatisticsBackendTest::unset_database())
        {
            delete db;
        }
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
    DatabaseEntityQueue* entity_queue = nullptr;
    // Data queue, attached to the database
    DatabaseDataQueue<eprosima::fastdds::statistics::Data>* data_queue = nullptr;
    // Monitor Service Data queue, attached to the database
    DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>* monitor_service_data_queue = nullptr;
    // Statistics participant_, that is supposed to receive the callbacks
    eprosima::fastdds::dds::DomainParticipant statistics_participant;
    // Listener under tests. Will receive a pointer to statistics_participant
    StatisticsParticipantListener* participant_listener = nullptr;
};

// Windows dll does not export ParticipantProxyData / WriterProxyData class members (private APIs)
#if !defined(_WIN32)

// Check the clear_inactive_entities StatisticsBackend method when a datawriter and datareader are undiscovered
TEST_F(clear_inactive_entities_tests, clear_inactive_entities_endpoint)
{
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));

    /////
    // Simulate a DATA(Uw) for DataWriter
    {
        // Start building the discovered writer info
        eprosima::fastdds::
rtps::WriterProxyData data(1, 1);

        // The discovered writer is in the participant
        eprosima::fastdds::
rtps::GUID_t writer_guid_;
        std::stringstream(datawriter->guid) >> writer_guid_;
        data.guid(writer_guid_);

        // The discovered writer is in the topic
        data.topicName(topic->name);
        data.typeName(topic->data_type);

        // The discovered writer contains the locator
        eprosima::fastdds::
rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
        dds_existing_unicast_locator.address[12] = 127;
        dds_existing_unicast_locator.address[15] = 1;
        data.add_unicast_locator(dds_existing_unicast_locator);

        // Finish building the discovered writer info
        eprosima::fastdds::
rtps::WriterDiscoveryInfo info(data);
        info.status = eprosima::fastdds::
rtps::WriterDiscoveryInfo::REMOVED_WRITER;

        // Execution: Call the listener
        participant_listener->on_publisher_discovery(&statistics_participant, std::move(info));
    }

    // Check that DataWriter is inactive
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));

    // Remove inactive entities
    StatisticsBackendTest::clear_inactive_entities();

    // Check that DataWriter no longer exist, and others still exist and are alive
    ASSERT_TRUE(StatisticsBackendTest::get_entities(EntityKind::DATAWRITER).empty());
    ASSERT_TRUE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));

    /////
    // Simulate a DATA(Ur) for DataReader
    {
        // Start building the discovered reader info
        eprosima::fastdds::
rtps::ReaderProxyData data(1, 1);

        // The discovered reader is in the participant
        eprosima::fastdds::
rtps::GUID_t reader_guid_;
        std::stringstream(datareader->guid) >> reader_guid_;
        data.guid(reader_guid_);

        // The discovered reader is in the topic
        data.topicName(topic->name);
        data.typeName(topic->data_type);

        // The discovered reader contains the locator
        eprosima::fastdds::
rtps::Locator_t dds_existing_unicast_locator(LOCATOR_KIND_UDPv4, 1024);
        dds_existing_unicast_locator.address[12] = 127;
        dds_existing_unicast_locator.address[15] = 1;
        data.add_unicast_locator(dds_existing_unicast_locator);

        // Finish building the discovered reader info
        eprosima::fastdds::
rtps::ReaderDiscoveryInfo info(data);
        info.status = eprosima::fastdds::
rtps::ReaderDiscoveryInfo::REMOVED_READER;

        // Execution: Call the listener
        participant_listener->on_subscriber_discovery(&statistics_participant, std::move(info));
    }

    // Check that DataReader is inactive, and thus topic
    ASSERT_FALSE(StatisticsBackendTest::is_active(topic->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datareader->id));

    // Remove inactive entities
    StatisticsBackendTest::clear_inactive_entities();

    // Check that DataWriter no longer exist, and others still exist and are alive
    ASSERT_TRUE(StatisticsBackendTest::get_entities(EntityKind::DATAREADER).empty());
    ASSERT_TRUE(StatisticsBackendTest::get_entities(EntityKind::TOPIC).empty());
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
}

// Check the clear_inactive_entities StatisticsBackend method when a participant is undiscovered
TEST_F(clear_inactive_entities_tests, clear_inactive_entities_participant)
{
    ASSERT_TRUE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_TRUE(StatisticsBackendTest::is_active(datareader->id));

    /////
    // Simulate a DATA(Up) for Participant

    // Start building the discovered reader info
    eprosima::fastdds::
rtps::RTPSParticipantAllocationAttributes allocation;
    eprosima::fastdds::
rtps::ParticipantProxyData data(allocation);

    // Precondition: The discovered participant has the given GUID and name
    eprosima::fastdds::
rtps::GUID_t participant_guid_;
    std::stringstream(participant->guid) >> participant_guid_;
    data.m_guid = participant_guid_;
    data.m_participantName = participant->name;

    // Finish building the discovered reader info
    eprosima::fastdds::
rtps::ParticipantDiscoveryInfo info(data);
    info.status = eprosima::fastdds::
rtps::ParticipantDiscoveryInfo::DROPPED_PARTICIPANT;

    // Execution: Call the listener
    participant_listener->on_participant_discovery(&statistics_participant, std::move(info));

    // Check that entities are not alive
    ASSERT_FALSE(StatisticsBackendTest::is_active(participant->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datawriter->id));
    ASSERT_FALSE(StatisticsBackendTest::is_active(datareader->id));

    // Remove inactive entities
    StatisticsBackendTest::clear_inactive_entities();

    // Check that these entities have been removed
    ASSERT_TRUE(StatisticsBackendTest::get_entities(EntityKind::PARTICIPANT).empty());
    ASSERT_TRUE(StatisticsBackendTest::get_entities(EntityKind::DATAWRITER).empty());
    ASSERT_TRUE(StatisticsBackendTest::get_entities(EntityKind::DATAREADER).empty());
}

#endif // !defined(_WIN32)

int main(

        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
