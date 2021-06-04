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

#include <database/database.hpp>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;


class DataBaseTest : public Database
{
public:

    const std::map<EntityId, std::shared_ptr<Host>>& hosts()
    {
        return hosts_;
    }

    const std::map<EntityId, std::shared_ptr<User>>& users()
    {
        return users_;
    }

    const std::map<EntityId, std::shared_ptr<Process>>& processes()
    {
        return processes_;
    }

    const std::map<EntityId, std::shared_ptr<Domain>>& domains()
    {
        return domains_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<Topic>>>& topics()
    {
        return topics_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>>& participants()
    {
        return participants_;
    }

    const std::map<EntityId, std::shared_ptr<Locator>>& locators()
    {
        return locators_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<Locator>>>& locators_by_participant()
    {
        return locators_by_participant_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>>& participants_by_locator()
    {
        return participants_by_locator_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<Domain>>>& domains_by_process()
    {
        return domains_by_process_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<Process>>>& processes_by_domain()
    {
        return processes_by_domain_;
    }

    template<typename T>
    std::map<EntityId, std::map<EntityId, std::shared_ptr<T>>>& get_dds_endpoints()
    {
        return dds_endpoints<T>();
    }

    Qos test_qos = {
        {"available_builtin_endpoints", 3135},
        {"lease_duration", {
             {"nanoseconds", 0},
             {"seconds", 3}
         }},
        {"properties", {
             {
                 {"name", "PARTICIPANT_TYPE"},
                 {"value", "CLIENT"}
             },
             {
                 {"name", "DS_VERSION"},
                 {"value", "2.0"}
             }
         }},
        {"user_data", "656e636c6176653d2f3b00"},
        {"vendor_id", "010f"}
    };

    const std::atomic<int64_t>& next_id()
    {
        return next_id_;
    }

};

/**
 * @brief Fixture for the database_load_insert method tests
 *
 * This fixture populates a database with several entities, whit several relations among them and with all types
 * of StatisticsData.
 *
 * The following Json object describes the populated database objects and their given unique identifiers:
 *
 * \code
 * {
 *  "host1": {
 *     "ID": 1,
 *     "users": []
 *   },
 *   "host2": {
 *     "ID": 2,
 *     "users": ["user1", "user2"]
 *   },
 *   "user1": {
 *     "ID": 3,
 *     "processes": []
 *   },
 *   "user2": {
 *     "ID": 4,
 *     "processes": ["process1", "process2"]
 *   },
 *   "process1": {
 *     "ID": 5,
 *     "participants": []
 *   },
 *   "process2": {
 *     "ID": 6,
 *     "participants": ["participant1", "participant2"]
 *   },
 *   "domain1": {
 *     "ID": 7,
 *     "participants": [],
 *     "topics": []
 *   },
 *   "domain2": {
 *     "ID": 8,
 *     "participants": ["participant1", "participant2"],
 *     "topics": ["topic1", "topic2"]
 *   },
 *   "participant1": {
 *     "ID": 9,
 *     "datareaders": [],
 *     "datawriters": []
 *   },
 *   "participant2": {
 *     "ID": 10,
 *     "datareaders": ["datareader1", "datareader2"],
 *     "datawriters": ["datawriter1", "datawriter2"]
 *   },
 *   "topic1": {
 *     "ID": 11,
 *     "datareaders": [],
 *     "datawriters": []
 *   },
 *   "topic2": {
 *     "ID": 12,
 *     "datareaders": ["datareader1", "datareader2"],
 *     "datawriters": ["datawriter1", "datawriter2"]
 *   },
 *   "datareader1": {
 *     "ID": 13,
 *     "locators": ["reader_locator1"]
 *   }
 *   "datareader2": {
 *     "ID": 14,
 *     "locators": ["reader_locator1", "reader_locator2"]
 *   }
 *   "datawriter1": {
 *     "ID": 15,
 *     "locators": ["writer_locator1"]
 *   }
 *   "datawriter2": {
 *     "ID": 16,
 *     "locators": ["writer_locator1", "writer_locator2"]
 *   }
 *   "reader_locator1": {
 *     "ID": 17
 *   }
 *   "reader_locator2": {
 *     "ID": 18
 *   }
 *   "writer_locator1": {
 *     "ID": 19
 *   }
 *   "writer_locator2": {
 *     "ID": 20
 *   }
 * }
 * \endcode
 *
 * Also insert Statistics data of all types with test values in participant2, datawriter2, datareader2 and locator2
 */
class database_load_insert_tests : public ::testing::Test
{
protected:

    typedef uint32_t TestId;

    void SetUp()
    {
        populate_database();
    }

    void populate_database()
    {
        auto host1 = std::make_shared<Host>("host1");
        db.insert(host1);

        auto host2 = std::make_shared<Host>("host2");
        db.insert(host2);

        auto user1 = std::make_shared<User>("user1", host2);
        db.insert(user1);

        auto user2 = std::make_shared<User>("user2", host2);
        db.insert(user2);

        auto process1 = std::make_shared<Process>("process1", "123", user2);
        db.insert(process1);

        auto process2 = std::make_shared<Process>("process2", "345", user2);
        db.insert(process2);

        auto domain1 = std::make_shared<Domain>("domain1");
        db.insert(domain1);

        auto domain2 = std::make_shared<Domain>("domain2");
        db.insert(domain2);

        auto participant1 = std::make_shared<DomainParticipant>("participant1", "qos", "1.2.3.4", nullptr, domain2);
        db.insert(participant1);
        db.link_participant_with_process(participant1->id, process2->id);

        auto participant2 = std::make_shared<DomainParticipant>("participant2", "qos", "5.6.7.8", nullptr, domain2);
        db.insert(participant2);
        db.link_participant_with_process(participant2->id, process2->id);

        auto topic1 = std::make_shared<Topic>("topic1", "type", domain2);
        db.insert(topic1);

        auto topic2 = std::make_shared<Topic>("topic2", "type", domain2);
        db.insert(topic2);

        auto datareader1 = std::make_shared<DataReader>("datareader1", "qos", "11.12.13.14", participant2, topic2);
        auto reader_locator1 = std::make_shared<Locator>("reader_locator1");
        reader_locator1->id = db.generate_entity_id();
        datareader1->locators[reader_locator1->id] = reader_locator1;
        db.insert(datareader1);

        auto datareader2 = std::make_shared<DataReader>("datareader2", "qos", "15.16.17.18", participant2, topic2);
        auto reader_locator2 = std::make_shared<Locator>("reader_locator2");
        reader_locator2->id = db.generate_entity_id();
        datareader2->locators[reader_locator1->id] = reader_locator1;
        datareader2->locators[reader_locator2->id] = reader_locator2;
        db.insert(datareader2);

        auto datawriter1 = std::make_shared<DataWriter>("datawriter1", "qos", "21.22.23.24", participant2, topic2);
        auto writer_locator1 = std::make_shared<Locator>("writer_locator1");
        writer_locator1->id = db.generate_entity_id();
        datawriter1->locators[writer_locator1->id] = writer_locator1;
        db.insert(datawriter1);

        auto datawriter2 = std::make_shared<DataWriter>("datawriter2", "qos", "25.26.27.28", participant2, topic2);
        auto writer_locator2 = std::make_shared<Locator>("writer_locator2");
        writer_locator2->id = db.generate_entity_id();
        datawriter2->locators[writer_locator1->id] = writer_locator1;
        datawriter2->locators[writer_locator2->id] = writer_locator2;
        db.insert(datawriter2);

        // Insert datas on domain2

        // participants
        {
            EntityId domainId = participant2->domain->id;
            EntityId entityId = participant2->id;

            // discovery_time
            {
                DiscoveryTimeSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(2));
                sample.time = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.remote_entity = EntityId(entityId);
                sample.discovered = true;

                db.insert(domainId, entityId, sample);
            }

            // pdp_packets
            {
                PdpCountSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));
                sample.count = 1;

                db.insert(domainId, entityId, sample);
            }

            // edp_packets
            {
                EdpCountSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.count = 0;

                db.insert(domainId, entityId, sample);
            }

            // rtps_packets_sent
            {
                RtpsPacketsSentSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.count = 0;
                sample.remote_locator = EntityId(reader_locator2->id);

                db.insert(domainId, entityId, sample);
            }

            // rtps_bytes_sent
            {
                RtpsBytesSentSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.count = 0;
                sample.magnitude_order = 0;
                sample.remote_locator = EntityId(reader_locator2->id);

                db.insert(domainId, entityId, sample);
            }

            // rtps_packets_lost
            {
                RtpsPacketsLostSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.count = 0;
                sample.remote_locator = EntityId(reader_locator2->id);

                db.insert(domainId, entityId, sample);
            }

            // rtps_bytes_lost
            {
                RtpsBytesLostSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.count = 0;
                sample.magnitude_order = 0;
                sample.remote_locator = EntityId(reader_locator2->id);

                db.insert(domainId, entityId, sample);
            }
        }

        // datawriters
        {
            EntityId domainId = datawriter2->participant->domain->id;
            EntityId entityId = datawriter2->id;

            // publication_throughput
            {
                PublicationThroughputSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.data = 1.1;

                db.insert(domainId, entityId, sample);
            }

            // resent_datas
            {
                ResentDataSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.count = 0;

                db.insert(domainId, entityId, sample);
            }

            // heartbeat_count
            {
                HeartbeatCountSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.count = 0;

                db.insert(domainId, entityId, sample);
            }

            // gap_count
            {
                GapCountSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.count = 0;

                db.insert(domainId, entityId, sample);
            }

            // data_count
            {
                DataCountSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.count = 0;

                db.insert(domainId, entityId, sample);
            }

            // samples_datas
            {
                SampleDatasCountSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.count = 0;
                sample.sequence_number = process2->id.value();

                db.insert(domainId, entityId, sample);
            }

            // history2history_latency
            {
                HistoryLatencySample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.data = 1.1;
                sample.reader = (EntityId(reader_locator2->id));

                db.insert(domainId, entityId, sample);
            }
        }

        // datareaders
        {
            EntityId domainId = datareader2->participant->domain->id;
            EntityId entityId = datareader2->id;

            // subscription_throughput
            {
                SubscriptionThroughputSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.data = 1.1;

                db.insert(domainId, entityId, sample);
            }

            // acknack_count
            {
                AcknackCountSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.count = 0;

                db.insert(domainId, entityId, sample);
            }

            // nackfrag_count
            {
                NackfragCountSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.count = 0;

                db.insert(domainId, entityId, sample);
            }
        }

        // locators
        {
            // writer_locator
            {
                EntityId entityId = writer_locator2->id;

                NetworkLatencySample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.data = 1.1;
                sample.remote_locator = EntityId(entityId);

                db.insert(EntityId::invalid(), entityId, sample);
            }

            // reader_locator
            {
                EntityId entityId = reader_locator2->id;

                NetworkLatencySample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(1));
                sample.data = 1.1;
                sample.remote_locator = EntityId(entityId);

                db.insert(EntityId::invalid(), entityId, sample);
            }

        }
    }

    DataBaseTest db;
};

template <typename Map>
bool key_compare (
        Map const& lhs,
        Map const& rhs)
{
    return lhs.size() == rhs.size()
           && std::equal(lhs.begin(), lhs.end(), rhs.begin(),
                   [](auto a, auto b)
                   {
                       return a.first == b.first;
                   });
}

template <typename Map>
bool map_compare (
        Map const& lhs,
        Map const& rhs)
{
    // No predicate needed because there is operator== for pairs already.
    return lhs.size() == rhs.size()
           && std::equal(lhs.begin(), lhs.end(),
                   rhs.begin());
}

/**
 * Test to check that load_database() fills the exactly same database that using database.insert()
 * with equivalent entities does. The fixture is necessary for populate a database
 * and then check the results with load the dump of the inserted one.
 */
TEST_F(database_load_insert_tests, load_insert)
{
    // Dump of the database with inserted datas
    DatabaseDump dump = db.dump_database();

    // Create db_loaded
    DataBaseTest db_loaded;
    db_loaded.load_database(dump);

    // ------------------ Compare map keys ------------------------

    // Compare ID of all maps of both databases.
    // If at() throw an exception, the db_loaded does not contain one Entity which should contain

    // hosts
    ASSERT_TRUE(key_compare(db.hosts(), db_loaded.hosts()));

    // users
    ASSERT_TRUE(key_compare(db.users(), db_loaded.users()));

    // processes
    ASSERT_TRUE(key_compare(db.processes(), db_loaded.processes()));

    // domains
    ASSERT_TRUE(key_compare(db.domains(), db_loaded.domains()));

    // topics
    ASSERT_TRUE(key_compare(db.topics(), db_loaded.topics()));
    for (auto topic : db.topics())
    {
        ASSERT_TRUE(key_compare(topic.second, db_loaded.topics().at(topic.first)));
    }

    // participants
    ASSERT_TRUE(key_compare(db.participants(), db_loaded.participants()));
    for (auto participant : db.participants())
    {
        ASSERT_TRUE(key_compare(participant.second, db_loaded.participants().at(participant.first)));
    }

    // locators
    ASSERT_TRUE(key_compare(db.locators(), db_loaded.locators()));

    // DataWriter
    ASSERT_TRUE(key_compare(db.get_dds_endpoints<DataWriter>(), db_loaded.get_dds_endpoints<DataWriter>()));
    for (auto datawriter : db.get_dds_endpoints<DataWriter>())
    {
        ASSERT_TRUE(key_compare(datawriter.second, db_loaded.get_dds_endpoints<DataWriter>().at(datawriter.first)));
    }

    // DataReader
    ASSERT_TRUE(key_compare(db.get_dds_endpoints<DataReader>(), db_loaded.get_dds_endpoints<DataReader>()));
    for (auto datareader : db.get_dds_endpoints<DataReader>())
    {
        ASSERT_TRUE(key_compare(datareader.second, db_loaded.get_dds_endpoints<DataReader>().at(datareader.first)));
    }

    // locators_by_participant
    ASSERT_TRUE(key_compare(db.locators_by_participant(), db_loaded.locators_by_participant()));
    for (auto locator : db.locators_by_participant())
    {
        ASSERT_TRUE(key_compare(locator.second, db_loaded.locators_by_participant().at(locator.first)));
    }

    // participants_by_locator
    ASSERT_TRUE(key_compare(db.participants_by_locator(), db_loaded.participants_by_locator()));
    for (auto participant : db.participants_by_locator())
    {
        ASSERT_TRUE(key_compare(participant.second, db_loaded.participants_by_locator().at(participant.first)));
    }

    // domains_by_process
    ASSERT_TRUE(key_compare(db.domains_by_process(), db_loaded.domains_by_process()));
    for (auto domain : db.domains_by_process())
    {
        ASSERT_TRUE(key_compare(domain.second, db_loaded.domains_by_process().at(domain.first)));
    }

    // processes_by_domain
    ASSERT_TRUE(key_compare(db.processes_by_domain(), db_loaded.processes_by_domain()));
    for (auto process : db.processes_by_domain())
    {
        ASSERT_TRUE(key_compare(process.second, db_loaded.processes_by_domain().at(process.first)));
    }

    // ------------------ Compare data ------------------------

    // Participants
    for (auto domainIt = db.participants().cbegin(); domainIt != db.participants().cend(); ++domainIt)
    {
        for (auto it = db.participants().at(domainIt->first).cbegin();
                it != db.participants().at(domainIt->first).cend(); ++it)
        {
            DomainParticipantData insertedData =  db.participants().at(domainIt->first).at(it->first)->data;
            DomainParticipantData loadedData =  db_loaded.participants().at(domainIt->first).at(it->first)->data;

            ASSERT_TRUE(map_compare(insertedData.rtps_packets_sent, loadedData.rtps_packets_sent));
            ASSERT_TRUE(map_compare(insertedData.last_reported_rtps_packets_sent_count,
                    loadedData.last_reported_rtps_packets_sent_count));
            ASSERT_TRUE(map_compare(insertedData.rtps_bytes_sent, loadedData.rtps_bytes_sent));
            ASSERT_TRUE(map_compare(insertedData.last_reported_rtps_bytes_sent_count,
                    loadedData.last_reported_rtps_bytes_sent_count));
            ASSERT_TRUE(map_compare(insertedData.rtps_packets_lost, loadedData.rtps_packets_lost));
            ASSERT_TRUE(map_compare(insertedData.last_reported_rtps_packets_lost_count,
                    loadedData.last_reported_rtps_packets_lost_count));
            ASSERT_TRUE(map_compare(insertedData.rtps_bytes_lost, loadedData.rtps_bytes_lost));
            ASSERT_TRUE(map_compare(insertedData.last_reported_rtps_bytes_lost_count,
                    loadedData.last_reported_rtps_bytes_lost_count));
            ASSERT_TRUE(map_compare(insertedData.discovered_entity, loadedData.discovered_entity));
            ASSERT_TRUE(insertedData.pdp_packets == loadedData.pdp_packets);
            ASSERT_TRUE(insertedData.last_reported_pdp_packets == loadedData.last_reported_pdp_packets);
            ASSERT_TRUE(insertedData.edp_packets == loadedData.edp_packets);
            ASSERT_TRUE(insertedData.last_reported_edp_packets == loadedData.last_reported_edp_packets);
        }
    }

    // DataWriter
    for (auto domainIt = db.get_dds_endpoints<DataWriter>().cbegin();
            domainIt != db.get_dds_endpoints<DataWriter>().cend(); ++domainIt)
    {
        for (auto it = db.get_dds_endpoints<DataWriter>().at(domainIt->first).cbegin();
                it != db.get_dds_endpoints<DataWriter>().at(domainIt->first).cend(); ++it)
        {
            DataWriterData insertedData =
                    db.get_dds_endpoints<DataWriter>().at(domainIt->first).at(it->first)->data;
            DataWriterData loadedData =
                    db_loaded.get_dds_endpoints<DataWriter>().at(domainIt->first).at(it->first)->data;

            ASSERT_TRUE(insertedData.publication_throughput == loadedData.publication_throughput);
            ASSERT_TRUE(insertedData.resent_datas == loadedData.resent_datas);
            ASSERT_TRUE(insertedData.last_reported_resent_datas == loadedData.last_reported_resent_datas);
            ASSERT_TRUE(insertedData.heartbeat_count == loadedData.heartbeat_count);
            ASSERT_TRUE(insertedData.last_reported_heartbeat_count == loadedData.last_reported_heartbeat_count);
            ASSERT_TRUE(insertedData.gap_count == loadedData.gap_count);
            ASSERT_TRUE(insertedData.last_reported_gap_count == loadedData.last_reported_gap_count);
            ASSERT_TRUE(insertedData.data_count == loadedData.data_count);
            ASSERT_TRUE(insertedData.last_reported_data_count == loadedData.last_reported_data_count);
            ASSERT_TRUE(map_compare(insertedData.sample_datas, loadedData.sample_datas));
            ASSERT_TRUE(map_compare(insertedData.history2history_latency, loadedData.history2history_latency));
        }
    }

    // DataReader
    for (auto domainIt = db.get_dds_endpoints<DataReader>().cbegin();
            domainIt != db.get_dds_endpoints<DataReader>().cend(); ++domainIt)
    {
        for (auto it = db.get_dds_endpoints<DataReader>().at(domainIt->first).cbegin();
                it != db.get_dds_endpoints<DataReader>().at(domainIt->first).cend(); ++it)
        {
            DataReaderData insertedData =
                    db.get_dds_endpoints<DataReader>().at(domainIt->first).at(it->first)->data;
            DataReaderData loadedData =
                    db_loaded.get_dds_endpoints<DataReader>().at(domainIt->first).at(it->first)->data;

            ASSERT_TRUE(insertedData.subscription_throughput == loadedData.subscription_throughput);
            ASSERT_TRUE(insertedData.acknack_count == loadedData.acknack_count);
            ASSERT_TRUE(insertedData.last_reported_acknack_count == loadedData.last_reported_acknack_count);
            ASSERT_TRUE(insertedData.nackfrag_count == loadedData.nackfrag_count);
            ASSERT_TRUE(insertedData.last_reported_nackfrag_count == loadedData.last_reported_nackfrag_count);

        }
    }

    // Locator
    for (auto it = db.locators().cbegin(); it != db.locators().cend(); ++it)
    {
        LocatorData insertedData = db.locators().at(it->first)->data;
        LocatorData loadedData = db_loaded.locators().at(it->first)->data;

        ASSERT_TRUE(map_compare(insertedData.network_latency_per_locator, loadedData.network_latency_per_locator));
    }

    // ------------------ Compare entities ------------------------


    // Host
    for (auto insertedIt = db.hosts().cbegin(), loadedIt = db_loaded.hosts().cbegin();
            insertedIt != db.hosts().cend() && loadedIt != db_loaded.hosts().cend(); insertedIt++, loadedIt++)
    {
        std::shared_ptr<Host> insertedEntity = insertedIt->second;
        std::shared_ptr<Host> loadedEntity = loadedIt->second;

        ASSERT_TRUE(insertedEntity->id == loadedEntity->id && insertedEntity->kind == loadedEntity->kind &&
                insertedEntity->name == loadedEntity->name);
        ASSERT_TRUE(key_compare(insertedEntity->users, loadedEntity->users));
    }

    // Users
    for (auto insertedIt = db.users().cbegin(), loadedIt = db_loaded.users().cbegin();
            insertedIt != db.users().cend() && loadedIt != db_loaded.users().cend(); insertedIt++, loadedIt++)
    {
        std::shared_ptr<User> insertedEntity = insertedIt->second;
        std::shared_ptr<User> loadedEntity = loadedIt->second;

        ASSERT_TRUE(insertedEntity->id == loadedEntity->id && insertedEntity->kind == loadedEntity->kind &&
                insertedEntity->name == loadedEntity->name);
        ASSERT_TRUE(insertedEntity->host->id == loadedEntity->host->id);
        ASSERT_TRUE(key_compare(insertedEntity->processes, loadedEntity->processes));
    }

    // Processes
    for (auto insertedIt = db.processes().cbegin(), loadedIt = db_loaded.processes().cbegin();
            insertedIt != db.processes().cend() && loadedIt != db_loaded.processes().cend(); insertedIt++, loadedIt++)
    {
        std::shared_ptr<Process> insertedEntity = insertedIt->second;
        std::shared_ptr<Process> loadedEntity = loadedIt->second;

        ASSERT_TRUE(insertedEntity->id == loadedEntity->id && insertedEntity->kind == loadedEntity->kind &&
                insertedEntity->name == loadedEntity->name);

        ASSERT_TRUE(insertedEntity->pid == loadedEntity->pid);
        ASSERT_TRUE(insertedEntity->user->id == loadedEntity->user->id);
        ASSERT_TRUE(key_compare(insertedEntity->participants, loadedEntity->participants));
    }

    // Locators
    for (auto insertedIt = db.locators().cbegin(), loadedIt = db_loaded.locators().cbegin();
            insertedIt != db.locators().cend() && loadedIt != db_loaded.locators().cend(); insertedIt++, loadedIt++)
    {
        std::shared_ptr<Locator> insertedEntity = insertedIt->second;
        std::shared_ptr<Locator> loadedEntity = loadedIt->second;

        ASSERT_TRUE(insertedEntity->id == loadedEntity->id && insertedEntity->kind == loadedEntity->kind &&
                insertedEntity->name == loadedEntity->name);

        ASSERT_TRUE(key_compare(insertedEntity->data_readers, loadedEntity->data_readers));
        ASSERT_TRUE(key_compare(insertedEntity->data_writers, loadedEntity->data_writers));
    }

    // Domains
    for (auto insertedIt = db.domains().cbegin(), loadedIt = db_loaded.domains().cbegin();
            insertedIt != db.domains().cend() && loadedIt != db_loaded.domains().cend(); insertedIt++, loadedIt++)
    {
        std::shared_ptr<Domain> insertedEntity = insertedIt->second;
        std::shared_ptr<Domain> loadedEntity = loadedIt->second;

        ASSERT_TRUE(insertedEntity->id == loadedEntity->id && insertedEntity->kind == loadedEntity->kind &&
                insertedEntity->name == loadedEntity->name);

        ASSERT_TRUE(key_compare(insertedEntity->topics, loadedEntity->topics));
        ASSERT_TRUE(key_compare(insertedEntity->participants, loadedEntity->participants));
    }

    // Participants
    for (auto domainIt = db.participants().cbegin(); domainIt != db.participants().cend(); ++domainIt)
    {
        for (auto insertedIt = db.participants().at(domainIt->first).cbegin(),
                loadedIt = db_loaded.participants().at(domainIt->first).cbegin();
                insertedIt != db.participants().at(domainIt->first).cend() &&
                loadedIt != db_loaded.participants().at(domainIt->first).cend();
                insertedIt++, loadedIt++)
        {
            std::shared_ptr<DomainParticipant> insertedEntity = insertedIt->second;
            std::shared_ptr<DomainParticipant> loadedEntity = loadedIt->second;

            ASSERT_TRUE(insertedEntity->id == loadedEntity->id && insertedEntity->kind == loadedEntity->kind &&
                    insertedEntity->name == loadedEntity->name);

            ASSERT_TRUE(insertedEntity->qos == loadedEntity->qos && insertedEntity->guid == loadedEntity->guid);

            ASSERT_TRUE(insertedEntity->process->id == loadedEntity->process->id);
            ASSERT_TRUE(insertedEntity->domain->id == loadedEntity->domain->id);

            ASSERT_TRUE(key_compare(insertedEntity->data_readers, loadedEntity->data_readers));
            ASSERT_TRUE(key_compare(insertedEntity->data_writers, loadedEntity->data_writers));
        }
    }

    // Datawriters
    for (auto domainIt = db.get_dds_endpoints<DataWriter>().cbegin(); domainIt !=
            db.get_dds_endpoints<DataWriter>().cend();
            ++domainIt)
    {
        for (auto insertedIt = db.get_dds_endpoints<DataWriter>().at(domainIt->first).cbegin(),
                loadedIt = db_loaded.get_dds_endpoints<DataWriter>().at(domainIt->first).cbegin();
                insertedIt != db.get_dds_endpoints<DataWriter>().at(domainIt->first).cend() &&
                loadedIt != db_loaded.get_dds_endpoints<DataWriter>().at(domainIt->first).cend();
                insertedIt++, loadedIt++)
        {
            std::shared_ptr<DataWriter> insertedEntity = insertedIt->second;
            std::shared_ptr<DataWriter> loadedEntity = loadedIt->second;

            ASSERT_TRUE(insertedEntity->id == loadedEntity->id && insertedEntity->kind == loadedEntity->kind &&
                    insertedEntity->name == loadedEntity->name);

            ASSERT_TRUE(insertedEntity->qos == loadedEntity->qos && insertedEntity->guid == loadedEntity->guid);

            ASSERT_TRUE(insertedEntity->participant->id == loadedEntity->participant->id);
            ASSERT_TRUE(insertedEntity->topic->id == loadedEntity->topic->id);

            ASSERT_TRUE(key_compare(insertedEntity->locators, loadedEntity->locators));
        }
    }

    // Datareaders
    for (auto domainIt = db.get_dds_endpoints<DataReader>().cbegin(); domainIt !=
            db.get_dds_endpoints<DataReader>().cend();
            ++domainIt)
    {
        for (auto insertedIt = db.get_dds_endpoints<DataReader>().at(domainIt->first).cbegin(),
                loadedIt = db_loaded.get_dds_endpoints<DataReader>().at(domainIt->first).cbegin();
                insertedIt != db.get_dds_endpoints<DataReader>().at(domainIt->first).cend() &&
                loadedIt != db_loaded.get_dds_endpoints<DataReader>().at(domainIt->first).cend();
                insertedIt++, loadedIt++)
        {
            std::shared_ptr<DataReader> insertedEntity = insertedIt->second;
            std::shared_ptr<DataReader> loadedEntity = loadedIt->second;

            ASSERT_TRUE(insertedEntity->id == loadedEntity->id && insertedEntity->kind == loadedEntity->kind &&
                    insertedEntity->name == loadedEntity->name);

            ASSERT_TRUE(insertedEntity->qos == loadedEntity->qos && insertedEntity->guid == loadedEntity->guid);

            ASSERT_TRUE(insertedEntity->participant->id == loadedEntity->participant->id);
            ASSERT_TRUE(insertedEntity->topic->id == loadedEntity->topic->id);

            ASSERT_TRUE(key_compare(insertedEntity->locators, loadedEntity->locators));
        }
    }

    // Topics
    for (auto domainIt = db.topics().cbegin(); domainIt != db.topics().cend(); ++domainIt)
    {
        for (auto insertedIt = db.topics().at(domainIt->first).cbegin(),
                loadedIt = db_loaded.topics().at(domainIt->first).cbegin();
                insertedIt != db.topics().at(domainIt->first).cend() &&
                loadedIt != db_loaded.topics().at(domainIt->first).cend();
                insertedIt++, loadedIt++)
        {
            std::shared_ptr<Topic> insertedEntity = insertedIt->second;
            std::shared_ptr<Topic> loadedEntity = loadedIt->second;

            ASSERT_TRUE(insertedEntity->id == loadedEntity->id && insertedEntity->kind == loadedEntity->kind &&
                    insertedEntity->name == loadedEntity->name);

            ASSERT_TRUE(insertedEntity->data_type == loadedEntity->data_type);
            ASSERT_TRUE(insertedEntity->domain->id == loadedEntity->domain->id);

            ASSERT_TRUE(key_compare(insertedEntity->data_readers, loadedEntity->data_readers));
            ASSERT_TRUE(key_compare(insertedEntity->data_writers, loadedEntity->data_writers));
        }
    }

    // domains_by_process
    for (auto domainIt = db.domains_by_process().cbegin(); domainIt != db.domains_by_process().cend(); ++domainIt)
    {
        for (auto insertedIt = db.domains_by_process().at(domainIt->first).cbegin(),
                loadedIt = db_loaded.domains_by_process().at(domainIt->first).cbegin();
                insertedIt != db.domains_by_process().at(domainIt->first).cend() &&
                loadedIt != db_loaded.domains_by_process().at(domainIt->first).cend();
                insertedIt++, loadedIt++)
        {
            std::shared_ptr<Domain> insertedEntity = insertedIt->second;
            std::shared_ptr<Domain> loadedEntity = loadedIt->second;

            ASSERT_TRUE(insertedEntity->id == loadedEntity->id && insertedEntity->kind == loadedEntity->kind &&
                    insertedEntity->name == loadedEntity->name);

            ASSERT_TRUE(key_compare(insertedEntity->topics, loadedEntity->topics));
            ASSERT_TRUE(key_compare(insertedEntity->participants, loadedEntity->participants));
        }
    }

    // processes_by_domain_
    for (auto domainIt = db.processes_by_domain().cbegin(); domainIt != db.processes_by_domain().cend(); ++domainIt)
    {
        for (auto insertedIt = db.processes_by_domain().at(domainIt->first).cbegin(),
                loadedIt = db_loaded.processes_by_domain().at(domainIt->first).cbegin();
                insertedIt != db.processes_by_domain().at(domainIt->first).cend() &&
                loadedIt != db_loaded.processes_by_domain().at(domainIt->first).cend();
                insertedIt++, loadedIt++)
        {
            std::shared_ptr<Process> insertedEntity = insertedIt->second;
            std::shared_ptr<Process> loadedEntity = loadedIt->second;

            ASSERT_TRUE(insertedEntity->id == loadedEntity->id && insertedEntity->kind == loadedEntity->kind &&
                    insertedEntity->name == loadedEntity->name);

            ASSERT_TRUE(insertedEntity->pid == loadedEntity->pid);
            ASSERT_TRUE(insertedEntity->user->id == loadedEntity->user->id);
            ASSERT_TRUE(key_compare(insertedEntity->participants, loadedEntity->participants));
        }
    }

    // participants_by_locator
    for (auto domainIt = db.participants_by_locator().cbegin(); domainIt != db.participants_by_locator().cend();
            ++domainIt)
    {
        for (auto insertedIt = db.participants_by_locator().at(domainIt->first).cbegin(),
                loadedIt = db_loaded.participants_by_locator().at(domainIt->first).cbegin();
                insertedIt != db.participants_by_locator().at(domainIt->first).cend() &&
                loadedIt != db_loaded.participants_by_locator().at(domainIt->first).cend();
                insertedIt++, loadedIt++)
        {
            std::shared_ptr<DomainParticipant> insertedEntity = insertedIt->second;
            std::shared_ptr<DomainParticipant> loadedEntity = loadedIt->second;

            ASSERT_TRUE(insertedEntity->id == loadedEntity->id && insertedEntity->kind == loadedEntity->kind &&
                    insertedEntity->name == loadedEntity->name);

            ASSERT_TRUE(insertedEntity->qos == loadedEntity->qos && insertedEntity->guid == loadedEntity->guid);

            ASSERT_TRUE(insertedEntity->process->id == loadedEntity->process->id);
            ASSERT_TRUE(insertedEntity->domain->id == loadedEntity->domain->id);

            ASSERT_TRUE(key_compare(insertedEntity->data_readers, loadedEntity->data_readers));
            ASSERT_TRUE(key_compare(insertedEntity->data_writers, loadedEntity->data_writers));
        }
    }

    // locators_by_participant
    for (auto domainIt = db.locators_by_participant().cbegin(); domainIt != db.locators_by_participant().cend();
            ++domainIt)
    {
        for (auto insertedIt = db.locators_by_participant().at(domainIt->first).cbegin(),
                loadedIt = db_loaded.locators_by_participant().at(domainIt->first).cbegin();
                insertedIt != db.locators_by_participant().at(domainIt->first).cend() &&
                loadedIt != db_loaded.locators_by_participant().at(domainIt->first).cend();
                insertedIt++, loadedIt++)
        {
            std::shared_ptr<Locator> insertedEntity = insertedIt->second;
            std::shared_ptr<Locator> loadedEntity = loadedIt->second;

            ASSERT_TRUE(insertedEntity->id == loadedEntity->id && insertedEntity->kind == loadedEntity->kind &&
                    insertedEntity->name == loadedEntity->name);

            ASSERT_TRUE(key_compare(insertedEntity->data_readers, loadedEntity->data_readers));
            ASSERT_TRUE(key_compare(insertedEntity->data_writers, loadedEntity->data_writers));
        }
    }

    // Compare next_id_ of both databases
    ASSERT_EQ(db.next_id(), db_loaded.next_id());

    // Compare dump of both databases
    ASSERT_EQ(dump, db_loaded.dump_database());
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
