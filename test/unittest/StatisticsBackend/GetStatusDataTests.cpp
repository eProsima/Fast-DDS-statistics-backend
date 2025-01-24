// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#include <DatabaseUtils.hpp>
#include <StatisticsBackendData.hpp>

bool StatisticsBackendTest::has_database_been_set_ = false;

/**
 * @brief Fixture for the get_status_data_tests
 * - Create a database loading it from a file.
 * - Allocate the database in the Backend
 */
class get_status_data_tests : public ::testing::Test
{
public:

    void SetUp()
    {
        db_ = new DataBaseTest;  // This will be deleted inside StatisticsBackendTest unset_database
        DatabaseDump dump;
        load_file(EMPTY_ENTITIES_DUMP_FILE, dump);
        db_->load_database(dump);
        domain_ = db_->domains().begin()->second;
        domain_ = db_->domains().begin()->second;
        participant_ = db_->participants().begin()->second.begin()->second;
        datawriter_ = db_->get_dds_endpoints<DataWriter>().begin()->second.begin()->second;
        datareader_ = db_->get_dds_endpoints<DataReader>().begin()->second.begin()->second;
        domain_id = domain_->id;
        participant_id = participant_->id;
        reader_id = datareader_->id;
        writer_id = datawriter_->id;

        // Allocating the db into the Backend make that is not necessary to delete the db.
        StatisticsBackendTest::set_database(db_);
    }

    void TearDown()
    {
        if (!StatisticsBackendTest::unset_database())
        {
            delete db_;
        }
    }

    void load_monitor_data()
    {
        proxy_sample_.kind = StatusKind::PROXY;
        proxy_sample_.status = StatusLevel::OK_STATUS;
        proxy_sample_.src_ts = std::chrono::system_clock::now();
        proxy_sample_.entity_proxy = {1, 2, 3, 4, 5};
        db_->insert(domain_->id, participant_->id, proxy_sample_);
        db_->insert(domain_->id, datawriter_->id, proxy_sample_);
        db_->insert(domain_->id, datareader_->id, proxy_sample_);

        eprosima::fastdds::statistics::Connection connection_sample;
        connection_sample.mode(eprosima::fastdds::statistics::ConnectionMode::DATA_SHARING);
        eprosima::fastdds::statistics::detail::GUID_s guid_s;
        eprosima::fastdds::rtps::GUID_t guid_t;
        std::stringstream guid_str("01.0f.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1");
        guid_str >> guid_t;
        memcpy(guid_s.guidPrefix().value().data(), guid_t.guidPrefix.value,
                eprosima::fastdds::rtps::GuidPrefix_t::size);
        memcpy(guid_s.entityId().value().data(), guid_t.entityId.value, eprosima::fastdds::rtps::EntityId_t::size);
        connection_sample.guid(guid_s);
        eprosima::fastdds::statistics::detail::Locator_s locator;
        locator.kind(1);
        locator.port(1);
        locator.address({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
        connection_sample.announced_locators({locator});
        connection_sample.used_locators({locator});
        connection_list_sample_.kind = StatusKind::CONNECTION_LIST;
        connection_list_sample_.status = StatusLevel::OK_STATUS;
        connection_list_sample_.src_ts = std::chrono::system_clock::now();
        connection_list_sample_.connection_list = {connection_sample, connection_sample};
        db_->insert(domain_->id, participant_->id, connection_list_sample_);
        db_->insert(domain_->id, datawriter_->id, connection_list_sample_);
        db_->insert(domain_->id, datareader_->id, connection_list_sample_);

        incompatible_qos_sample_.kind = StatusKind::INCOMPATIBLE_QOS;
        incompatible_qos_sample_.status = StatusLevel::OK_STATUS;
        incompatible_qos_sample_.src_ts = std::chrono::system_clock::now();
        incompatible_qos_sample_.incompatible_qos_status.total_count(0);
        incompatible_qos_sample_.incompatible_qos_status.last_policy_id(0);
        eprosima::fastdds::statistics::QosPolicyCountSeq_s qos_policy_count_seq;
        eprosima::fastdds::statistics::QosPolicyCount_s qos_policy_count;
        qos_policy_count.policy_id(0);
        qos_policy_count.count(0);
        qos_policy_count_seq = {qos_policy_count};
        incompatible_qos_sample_.incompatible_qos_status.policies(qos_policy_count_seq);
        db_->insert(domain_->id, datawriter_->id, incompatible_qos_sample_);
        db_->insert(domain_->id, datareader_->id, incompatible_qos_sample_);

        inconsistent_topic_sample_.kind = StatusKind::INCONSISTENT_TOPIC;
        inconsistent_topic_sample_.status = StatusLevel::OK_STATUS;
        inconsistent_topic_sample_.src_ts = std::chrono::system_clock::now();
        inconsistent_topic_sample_.inconsistent_topic_status.total_count(0);
        db_->insert(domain_->id, datawriter_->id, inconsistent_topic_sample_);
        db_->insert(domain_->id, datareader_->id, inconsistent_topic_sample_);

        liveliness_lost_sample_.kind = StatusKind::LIVELINESS_LOST;
        liveliness_lost_sample_.status = StatusLevel::OK_STATUS;
        liveliness_lost_sample_.src_ts = std::chrono::system_clock::now();
        liveliness_lost_sample_.liveliness_lost_status.total_count(0);
        db_->insert(domain_->id, datawriter_->id, liveliness_lost_sample_);

        liveliness_changed_sample_.kind = StatusKind::LIVELINESS_CHANGED;
        liveliness_changed_sample_.status = StatusLevel::OK_STATUS;
        liveliness_changed_sample_.src_ts = std::chrono::system_clock::now();
        liveliness_changed_sample_.liveliness_changed_status.alive_count(1);
        liveliness_changed_sample_.liveliness_changed_status.not_alive_count(0);
        liveliness_changed_sample_.liveliness_changed_status.last_publication_handle({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                                                                                      11, 12, 13, 14, 15});
        db_->insert(domain_->id, datareader_->id, liveliness_changed_sample_);

        deadline_missed_sample_.kind = StatusKind::DEADLINE_MISSED;
        deadline_missed_sample_.status = StatusLevel::OK_STATUS;
        deadline_missed_sample_.src_ts = std::chrono::system_clock::now();
        deadline_missed_sample_.deadline_missed_status.total_count(0);
        deadline_missed_sample_.deadline_missed_status.last_instance_handle({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                                                                             13, 14, 15});
        db_->insert(domain_->id, datawriter_->id, deadline_missed_sample_);
        db_->insert(domain_->id, datareader_->id, deadline_missed_sample_);

        sample_lost_sample_.kind = StatusKind::SAMPLE_LOST;
        sample_lost_sample_.status = StatusLevel::OK_STATUS;
        sample_lost_sample_.src_ts = std::chrono::system_clock::now();
        sample_lost_sample_.sample_lost_status.total_count(0);
        db_->insert(domain_->id, datareader_->id, sample_lost_sample_);
    }

    DataBaseTest* db_;
    std::shared_ptr<Domain> domain_;
    std::shared_ptr<DomainParticipant> participant_;
    std::shared_ptr<DataWriter> datawriter_;
    std::shared_ptr<DataReader> datareader_;
    EntityId domain_id;
    EntityId participant_id;
    EntityId reader_id;
    EntityId writer_id;
    ProxySample proxy_sample_;
    ConnectionListSample connection_list_sample_;
    IncompatibleQosSample incompatible_qos_sample_;
    InconsistentTopicSample inconsistent_topic_sample_;
    LivelinessLostSample liveliness_lost_sample_;
    LivelinessChangedSample liveliness_changed_sample_;
    DeadlineMissedSample deadline_missed_sample_;
    SampleLostSample sample_lost_sample_;
};

// Tests calling StatisticsBackend::get_status_data, and then clearing data from database

TEST_F(get_status_data_tests, get_status_data_proxy)
{
    // Check that there is no data loaded
    {
        ProxySample proxy_sample_p;
        StatisticsBackendTest::get_status_data(participant_id, proxy_sample_p);
        EXPECT_EQ(proxy_sample_p, ProxySample());
        ProxySample proxy_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, proxy_sample_r);
        EXPECT_EQ(proxy_sample_r, ProxySample());
        ProxySample proxy_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, proxy_sample_w);
        EXPECT_EQ(proxy_sample_w, ProxySample());
    }

    load_monitor_data();

    // Wrong entity
    {
        ProxySample proxy_sample;
        EXPECT_THROW(StatisticsBackendTest::get_status_data(domain_id, proxy_sample), BadParameter);
    }

    // Check that there is data loaded
    {
        ProxySample proxy_sample_p;
        StatisticsBackendTest::get_status_data(participant_id, proxy_sample_p);
        EXPECT_EQ(proxy_sample_p, proxy_sample_);
        EXPECT_NE(proxy_sample_p, ProxySample());
        ProxySample proxy_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, proxy_sample_r);
        EXPECT_EQ(proxy_sample_r, proxy_sample_);
        EXPECT_NE(proxy_sample_r, ProxySample());
        ProxySample proxy_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, proxy_sample_w);
        EXPECT_EQ(proxy_sample_w, proxy_sample_);
        EXPECT_NE(proxy_sample_w, ProxySample());

        //Clear proxy sample
        proxy_sample_p.clear();
        ProxySample sample_cleared;
        sample_cleared.kind = StatusKind::INVALID;
        sample_cleared.status = StatusLevel::ERROR_STATUS;
        EXPECT_EQ(proxy_sample_p, sample_cleared);
    }

    // Clearing data
    StatisticsBackendTest::clear_statistics_data();
    {
        ProxySample proxy_sample_p;
        StatisticsBackendTest::get_status_data(participant_id, proxy_sample_p);
        EXPECT_NE(proxy_sample_p, proxy_sample_);
        EXPECT_EQ(proxy_sample_p, ProxySample());
        ProxySample proxy_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, proxy_sample_r);
        EXPECT_NE(proxy_sample_r, proxy_sample_);
        EXPECT_EQ(proxy_sample_r, ProxySample());
        ProxySample proxy_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, proxy_sample_w);
        EXPECT_NE(proxy_sample_w, proxy_sample_);
        EXPECT_EQ(proxy_sample_w, ProxySample());
    }
}

TEST_F(get_status_data_tests, get_status_data_connection_list)
{
    // Check that there is no data loaded
    {
        ConnectionListSample connection_list_sample_p;
        StatisticsBackendTest::get_status_data(participant_id, connection_list_sample_p);
        EXPECT_EQ(connection_list_sample_p, ConnectionListSample());
        ConnectionListSample connection_list_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, connection_list_sample_r);
        EXPECT_EQ(connection_list_sample_r, ConnectionListSample());
        ConnectionListSample connection_list_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, connection_list_sample_w);
        EXPECT_EQ(connection_list_sample_w, ConnectionListSample());
    }

    load_monitor_data();

    // Wrong entity
    {
        ConnectionListSample connection_list_sample_p;
        EXPECT_THROW(StatisticsBackendTest::get_status_data(domain_id, connection_list_sample_p), BadParameter);
    }

    // Check that there is data loaded
    {
        ConnectionListSample connection_list_sample_p;
        StatisticsBackendTest::get_status_data(participant_id, connection_list_sample_p);
        EXPECT_EQ(connection_list_sample_p, connection_list_sample_);
        EXPECT_NE(connection_list_sample_p, ConnectionListSample());
        ConnectionListSample connection_list_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, connection_list_sample_r);
        EXPECT_EQ(connection_list_sample_r, connection_list_sample_);
        EXPECT_NE(connection_list_sample_r, ConnectionListSample());
        ConnectionListSample connection_list_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, connection_list_sample_w);
        EXPECT_EQ(connection_list_sample_w, connection_list_sample_);
        EXPECT_NE(connection_list_sample_w, ConnectionListSample());

        //Clear connection list sample
        connection_list_sample_p.clear();
        ConnectionListSample sample_cleared;
        sample_cleared.kind = StatusKind::INVALID;
        sample_cleared.status = StatusLevel::ERROR_STATUS;
        EXPECT_EQ(connection_list_sample_p, sample_cleared);
    }

    // Clearing data
    StatisticsBackendTest::clear_statistics_data();
    {
        ConnectionListSample connection_list_sample_p;
        StatisticsBackendTest::get_status_data(participant_id, connection_list_sample_p);
        EXPECT_NE(connection_list_sample_p, connection_list_sample_);
        EXPECT_EQ(connection_list_sample_p, ConnectionListSample());
        ConnectionListSample connection_list_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, connection_list_sample_r);
        EXPECT_NE(connection_list_sample_r, connection_list_sample_);
        EXPECT_EQ(connection_list_sample_r, ConnectionListSample());
        ConnectionListSample connection_list_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, connection_list_sample_w);
        EXPECT_NE(connection_list_sample_w, connection_list_sample_);
        EXPECT_EQ(connection_list_sample_w, ConnectionListSample());
    }
}

TEST_F(get_status_data_tests, get_status_data_incompatible_qos)
{
    // Check that there is no data loaded
    {
        IncompatibleQosSample incompatible_qos_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, incompatible_qos_sample_r);
        EXPECT_EQ(incompatible_qos_sample_r, IncompatibleQosSample());
        IncompatibleQosSample incompatible_qos_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, incompatible_qos_sample_w);
        EXPECT_EQ(incompatible_qos_sample_w, IncompatibleQosSample());
    }

    load_monitor_data();

    // Wrong entity
    {
        IncompatibleQosSample incompatible_qos_sample;
        EXPECT_THROW(StatisticsBackendTest::get_status_data(participant_id, incompatible_qos_sample), BadParameter);
    }

    // Check that there is data loaded
    {
        IncompatibleQosSample incompatible_qos_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, incompatible_qos_sample_r);
        EXPECT_EQ(incompatible_qos_sample_r, incompatible_qos_sample_);
        EXPECT_NE(incompatible_qos_sample_r, IncompatibleQosSample());
        IncompatibleQosSample incompatible_qos_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, incompatible_qos_sample_w);
        EXPECT_EQ(incompatible_qos_sample_w, incompatible_qos_sample_);
        EXPECT_NE(incompatible_qos_sample_w, IncompatibleQosSample());

        //Clear incompatible qos sample
        incompatible_qos_sample_w.clear();
        IncompatibleQosSample sample_cleared;
        sample_cleared.kind = StatusKind::INVALID;
        sample_cleared.status = StatusLevel::ERROR_STATUS;
        EXPECT_EQ(incompatible_qos_sample_w, sample_cleared);
    }

    // Clearing data
    StatisticsBackendTest::clear_statistics_data();
    {
        IncompatibleQosSample incompatible_qos_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, incompatible_qos_sample_r);
        EXPECT_NE(incompatible_qos_sample_r, incompatible_qos_sample_);
        EXPECT_EQ(incompatible_qos_sample_r, IncompatibleQosSample());
        IncompatibleQosSample incompatible_qos_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, incompatible_qos_sample_w);
        EXPECT_NE(incompatible_qos_sample_w, incompatible_qos_sample_);
        EXPECT_EQ(incompatible_qos_sample_w, IncompatibleQosSample());
    }
}

TEST_F(get_status_data_tests, get_status_data_inconsistent_topic)
{
    // Check that there is no data loaded
    {
        InconsistentTopicSample inconsistent_topic_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, inconsistent_topic_sample_r);
        EXPECT_EQ(inconsistent_topic_sample_r, InconsistentTopicSample());
        InconsistentTopicSample inconsistent_topic_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, inconsistent_topic_sample_w);
        EXPECT_EQ(inconsistent_topic_sample_w, InconsistentTopicSample());
    }

    load_monitor_data();

    // Wrong entity
    {
        InconsistentTopicSample inconsistent_topic_sample;
        EXPECT_THROW(StatisticsBackendTest::get_status_data(participant_id, inconsistent_topic_sample), BadParameter);
    }

    // Check that there is data loaded
    {
        InconsistentTopicSample inconsistent_topic_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, inconsistent_topic_sample_r);
        EXPECT_EQ(inconsistent_topic_sample_r, inconsistent_topic_sample_);
        EXPECT_NE(inconsistent_topic_sample_r, InconsistentTopicSample());
        InconsistentTopicSample inconsistent_topic_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, inconsistent_topic_sample_w);
        EXPECT_EQ(inconsistent_topic_sample_w, inconsistent_topic_sample_);
        EXPECT_NE(inconsistent_topic_sample_w, InconsistentTopicSample());

        //Clear inconsistent topic sample
        inconsistent_topic_sample_w.clear();
        InconsistentTopicSample sample_cleared;
        sample_cleared.kind = StatusKind::INVALID;
        sample_cleared.status = StatusLevel::ERROR_STATUS;
        EXPECT_EQ(inconsistent_topic_sample_w, sample_cleared);
    }

    // Clearing data
    StatisticsBackendTest::clear_statistics_data();
    {
        InconsistentTopicSample inconsistent_topic_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, inconsistent_topic_sample_r);
        EXPECT_NE(inconsistent_topic_sample_r, inconsistent_topic_sample_);
        EXPECT_EQ(inconsistent_topic_sample_r, InconsistentTopicSample());
        InconsistentTopicSample inconsistent_topic_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, inconsistent_topic_sample_w);
        EXPECT_NE(inconsistent_topic_sample_w, inconsistent_topic_sample_);
        EXPECT_EQ(inconsistent_topic_sample_w, InconsistentTopicSample());
    }
}


TEST_F(get_status_data_tests, get_status_data_liveliness_lost)
{
    // Check that there is no data loaded
    {
        LivelinessLostSample liveliness_lost_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, liveliness_lost_sample_w);
        EXPECT_EQ(liveliness_lost_sample_w, LivelinessLostSample());
    }

    load_monitor_data();

    // Wrong entity
    {
        LivelinessLostSample liveliness_lost_sample;
        EXPECT_THROW(StatisticsBackendTest::get_status_data(reader_id, liveliness_lost_sample), BadParameter);
    }

    // Check that there is data loaded
    {
        LivelinessLostSample liveliness_lost_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, liveliness_lost_sample_w);
        EXPECT_EQ(liveliness_lost_sample_w, liveliness_lost_sample_);
        EXPECT_NE(liveliness_lost_sample_w, LivelinessLostSample());

        //Clear liveliness lost sample
        liveliness_lost_sample_w.clear();
        LivelinessLostSample sample_cleared;
        sample_cleared.kind = StatusKind::INVALID;
        sample_cleared.status = StatusLevel::ERROR_STATUS;
        EXPECT_EQ(liveliness_lost_sample_w, sample_cleared);
    }

    // Clearing data
    StatisticsBackendTest::clear_statistics_data();
    {
        LivelinessLostSample liveliness_lost_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, liveliness_lost_sample_w);
        EXPECT_NE(liveliness_lost_sample_w, liveliness_lost_sample_);
        EXPECT_EQ(liveliness_lost_sample_w, LivelinessLostSample());
    }
}

TEST_F(get_status_data_tests, get_status_data_liveliness_changed)
{
    // Check that there is no data loaded
    {
        LivelinessChangedSample liveliness_changed_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, liveliness_changed_sample_r);
        EXPECT_EQ(liveliness_changed_sample_r, LivelinessChangedSample());
    }

    load_monitor_data();

    // Wrong entity
    {
        LivelinessChangedSample liveliness_changed_sample;
        EXPECT_THROW(StatisticsBackendTest::get_status_data(writer_id, liveliness_changed_sample), BadParameter);
    }

    // Check that there is data loaded
    {
        LivelinessChangedSample liveliness_changed_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, liveliness_changed_sample_r);
        EXPECT_EQ(liveliness_changed_sample_r, liveliness_changed_sample_);
        EXPECT_NE(liveliness_changed_sample_r, LivelinessChangedSample());

        //Clear liveliness changed sample
        liveliness_changed_sample_r.clear();
        LivelinessChangedSample sample_cleared;
        sample_cleared.kind = StatusKind::INVALID;
        sample_cleared.status = StatusLevel::ERROR_STATUS;
        EXPECT_EQ(liveliness_changed_sample_r, sample_cleared);
    }

    // Clearing data
    StatisticsBackendTest::clear_statistics_data();
    {
        LivelinessChangedSample liveliness_changed_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, liveliness_changed_sample_r);
        EXPECT_NE(liveliness_changed_sample_r, liveliness_changed_sample_);
        EXPECT_EQ(liveliness_changed_sample_r, LivelinessChangedSample());
    }
}

TEST_F(get_status_data_tests, get_status_data_deadline_missed)
{
    // Check that there is no data loaded
    {
        DeadlineMissedSample deadline_missed_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, deadline_missed_sample_r);
        EXPECT_EQ(deadline_missed_sample_r, DeadlineMissedSample());
        DeadlineMissedSample deadline_missed_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, deadline_missed_sample_w);
        EXPECT_EQ(deadline_missed_sample_w, DeadlineMissedSample());
    }

    load_monitor_data();

    // Wrong entity
    {
        DeadlineMissedSample deadline_missed_sample;
        EXPECT_THROW(StatisticsBackendTest::get_status_data(participant_id, deadline_missed_sample), BadParameter);
    }

    // Check that there is data loaded
    {
        DeadlineMissedSample deadline_missed_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, deadline_missed_sample_r);
        EXPECT_EQ(deadline_missed_sample_r, deadline_missed_sample_);
        EXPECT_NE(deadline_missed_sample_r, DeadlineMissedSample());
        DeadlineMissedSample deadline_missed_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, deadline_missed_sample_w);
        EXPECT_EQ(deadline_missed_sample_w, deadline_missed_sample_);
        EXPECT_NE(deadline_missed_sample_w, DeadlineMissedSample());

        //Clear deadline missed sample
        deadline_missed_sample_r.clear();
        DeadlineMissedSample sample_cleared;
        sample_cleared.kind = StatusKind::INVALID;
        sample_cleared.status = StatusLevel::ERROR_STATUS;
        EXPECT_EQ(deadline_missed_sample_r, sample_cleared);
    }

    // Clearing data
    StatisticsBackendTest::clear_statistics_data();
    {
        DeadlineMissedSample deadline_missed_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, deadline_missed_sample_r);
        EXPECT_NE(deadline_missed_sample_r, deadline_missed_sample_);
        EXPECT_EQ(deadline_missed_sample_r, DeadlineMissedSample());
        DeadlineMissedSample deadline_missed_sample_w;
        StatisticsBackendTest::get_status_data(writer_id, deadline_missed_sample_w);
        EXPECT_NE(deadline_missed_sample_w, deadline_missed_sample_);
        EXPECT_EQ(deadline_missed_sample_w, DeadlineMissedSample());
    }
}

TEST_F(get_status_data_tests, get_status_data_sample_lost)
{
    // Check that there is no data loaded
    {
        SampleLostSample sample_lost_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, sample_lost_sample_r);
        EXPECT_EQ(sample_lost_sample_r, SampleLostSample());
    }

    load_monitor_data();

    // Wrong entity
    {
        SampleLostSample sample_lost_sample;
        EXPECT_THROW(StatisticsBackendTest::get_status_data(participant_id, sample_lost_sample), BadParameter);
    }

    // Check that there is data loaded
    {
        SampleLostSample sample_lost_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, sample_lost_sample_r);
        EXPECT_EQ(sample_lost_sample_r, sample_lost_sample_);
        EXPECT_NE(sample_lost_sample_r, SampleLostSample());

        //Clear sample lost sample
        sample_lost_sample_r.clear();
        SampleLostSample sample_cleared;
        sample_cleared.kind = StatusKind::INVALID;
        sample_cleared.status = StatusLevel::ERROR_STATUS;
        EXPECT_EQ(sample_lost_sample_r, sample_cleared);
    }

    // Clearing data
    StatisticsBackendTest::clear_statistics_data();
    {
        SampleLostSample sample_lost_sample_r;
        StatisticsBackendTest::get_status_data(reader_id, sample_lost_sample_r);
        EXPECT_NE(sample_lost_sample_r, sample_lost_sample_);
        EXPECT_EQ(sample_lost_sample_r, SampleLostSample());
    }
}

int main(

        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
