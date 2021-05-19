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

#include "gtest/gtest.h"

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
 * @brief Fixture for the get_entities method tests
 *
 * \c get_entities retrieves all the entities of a given kind that are reachable from a given entity.
 * The casuistry for this functionality is rather complex,
 * and the tests need a populated database with several entity combinations in order to be able to
 * test all this casuistry.
 *
 * This fixture populates a database with several entities and several relations among them.
 * Since the tests need to know these relations, each entity is given a unique identifier within the fixture,
 * and the fixture keeps all these entities on a map,
 * for the test to be able to get them and check the result of the execution.
 * This identifier is independent of the EntityId given by the database,
 * and is totally under control of the fixture and the tests.
 * This is important to keep the tests stable even if the Database implementation to assign an EntityId changes.
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
 *
 * Parameteres to the tests are:
 *  - std::get<0>(GetParam()) The EntityKind we are looking for
 *  - std::get<1>(GetParam()) The unique identifier of the origin Entity, as given by the fixture/testing
 *  - std::get<2>(GetParam()) A list containing the unique identifiers of the entities expected in the result
 */
class database_load_tests : public ::testing::Test
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
        entities[1] = host1;

        auto host2 = std::make_shared<Host>("host2");
        db.insert(host2);
        entities[2] = host2;

        auto user1 = std::make_shared<User>("user1", host2);
        db.insert(user1);
        entities[3] = user1;

        auto user2 = std::make_shared<User>("user2", host2);
        db.insert(user2);
        entities[4] = user2;

        auto process1 = std::make_shared<Process>("process1", "123", user2);
        db.insert(process1);
        entities[5] = process1;

        auto process2 = std::make_shared<Process>("process2", "345", user2);
        db.insert(process2);
        entities[6] = process2;

        auto domain1 = std::make_shared<Domain>("domain1");
        db.insert(domain1);
        entities[7] = domain1;

        auto domain2 = std::make_shared<Domain>("domain2");
        db.insert(domain2);
        entities[8] = domain2;

        auto participant1 = std::make_shared<DomainParticipant>("participant1", "qos", "1.2.3.4", nullptr, domain2);
        db.insert(participant1);
        db.link_participant_with_process(participant1->id, process2->id);
        entities[9] = participant1;

        auto participant2 = std::make_shared<DomainParticipant>("participant2", "qos", "5.6.7.8", nullptr, domain2);
        db.insert(participant2);
        db.link_participant_with_process(participant2->id, process2->id);
        entities[10] = participant2;

        auto topic1 = std::make_shared<Topic>("topic1", "type", domain2);
        db.insert(topic1);
        entities[11] = topic1;

        auto topic2 = std::make_shared<Topic>("topic2", "type", domain2);
        db.insert(topic2);
        entities[12] = topic2;

        auto datareader1 = std::make_shared<DataReader>("datareader1", "qos", "11.12.13.14", participant2, topic2);
        auto reader_locator1 = std::make_shared<Locator>("reader_locator1");
        reader_locator1->id = db.generate_entity_id();
        datareader1->locators[reader_locator1->id] = reader_locator1;
        db.insert(datareader1);
        entities[13] = datareader1;
        entities[17] = reader_locator1;

        auto datareader2 = std::make_shared<DataReader>("datareader2", "qos", "15.16.17.18", participant2, topic2);
        auto reader_locator2 = std::make_shared<Locator>("reader_locator2");
        reader_locator2->id = db.generate_entity_id();
        datareader2->locators[reader_locator1->id] = reader_locator1;
        datareader2->locators[reader_locator2->id] = reader_locator2;
        db.insert(datareader2);
        entities[14] = datareader2;
        entities[18] = reader_locator2;

        auto datawriter1 = std::make_shared<DataWriter>("datawriter1", "qos", "21.22.23.24", participant2, topic2);
        auto writer_locator1 = std::make_shared<Locator>("writer_locator1");
        writer_locator1->id = db.generate_entity_id();
        datawriter1->locators[writer_locator1->id] = writer_locator1;
        db.insert(datawriter1);
        entities[15] = datawriter1;
        entities[19] = writer_locator1;

        auto datawriter2 = std::make_shared<DataWriter>("datawriter2", "qos", "25.26.27.28", participant2, topic2);
        auto writer_locator2 = std::make_shared<Locator>("writer_locator2");
        writer_locator2->id = db.generate_entity_id();
        datawriter2->locators[writer_locator1->id] = writer_locator1;
        datawriter2->locators[writer_locator2->id] = writer_locator2;
        db.insert(datawriter2);
        entities[16] = datawriter2;
        entities[20] = writer_locator2;

        // Insert datas on domain2
        
        // participants
        {
            EntityId domainId = participant2->domain->id;
            EntityId entityId = participant2->id;

            // discovery_time
            {
                DiscoveryTimeSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));
                sample.time = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));    
                sample.remote_entity = EntityId(1);
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

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));
                sample.count = 1;
                
                db.insert(domainId, entityId, sample);
            }

            // rtps_packets_sent
            {
                RtpsPacketsSentSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));
                sample.count = 1;
                sample.remote_locator = EntityId(1);
                
                db.insert(domainId, entityId, sample);
            }

            // rtps_bytes_sent
            {
                RtpsBytesSentSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));
                sample.count = 1;
                sample.magnitude_order = 1;
                sample.remote_locator = EntityId(1);
                
                db.insert(domainId, entityId, sample);
            }

            // rtps_packets_lost
            {
                RtpsPacketsLostSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));   
                sample.count = 1;
                sample.remote_locator = EntityId(1);
                
                db.insert(domainId, entityId, sample);
            }

            // rtps_bytes_lost
            {
                RtpsBytesLostSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));  
                sample.count = 1;
                sample.magnitude_order = 1;
                sample.remote_locator = EntityId(1);
                
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

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));
                sample.data = 0.0;

                db.insert(domainId, entityId, sample);
            }

            // resent_datas
            {
                ResentDataSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));
                sample.count = 0;

                db.insert(domainId, entityId, sample);
            }

            // heartbeat_count
            {
                HeartbeatCountSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));
                sample.count = 0;

                db.insert(domainId, entityId, sample);
            }

            // gap_count
            {
                GapCountSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));
                sample.count = 0;

                db.insert(domainId, entityId, sample);
            }

            // data_count
            {
                DataCountSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));
                sample.count = 0;

                db.insert(domainId, entityId, sample);
            }

            // samples_datas
            {
                SampleDatasCountSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));
                sample.count = 0;
                sample.sequence_number = 0;

                db.insert(domainId, entityId, sample);
            }

            // history2history_latency
            {
                HistoryLatencySample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));
                sample.data = 0.0;
                sample.reader = (EntityId(0));

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

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));
                sample.data = 0.0;

                db.insert(domainId, entityId, sample);
            }

            // acknack_count
            {
                AcknackCountSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));
                sample.count = 1;

                db.insert(domainId, entityId, sample);
            }

            // nackfrag_count
            {
                NackfragCountSample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));
                sample.count = 1;

                db.insert(domainId, entityId, sample);
            }
        }

        // locators
        {
            // writer_locator
            {
                EntityId entityId = writer_locator2->id;

                NetworkLatencySample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));
                sample.data = 1.0;
                sample.remote_locator = EntityId(1);

                db.insert(EntityId::invalid(), entityId, sample);
            }

            // reader_locator
            {
                EntityId entityId = reader_locator2->id;

                NetworkLatencySample sample;

                sample.src_ts = std::chrono::system_clock::time_point(std::chrono::steady_clock::duration(0));
                sample.data = 1.0;
                sample.remote_locator = EntityId(1);

                db.insert(EntityId::invalid(), entityId, sample);
            }

        }
    }

    DataBaseTest db;
    std::map<TestId, std::shared_ptr<const Entity>> entities;
};

template <typename Map>
bool key_compare (Map const &lhs, Map const &rhs) {
    return lhs.size() == rhs.size()
        && std::equal(lhs.begin(), lhs.end(), rhs.begin(), 
                      [] (auto a, auto b) { return a.first == b.first; });
}

template <typename Map>
bool map_compare (Map const &lhs, Map const &rhs) {
    // No predicate needed because there is operator== for pairs already.
    return lhs.size() == rhs.size()
        && std::equal(lhs.begin(), lhs.end(),
                      rhs.begin());
}


// Test the load of a database
TEST_F(database_load_tests, load_database)
{
    // Dump of the database with inserted datas
    DatabaseDump dump = db.dump_database();

    // Create db_loaded
    DataBaseTest db_loaded;
    db_loaded.load_database(dump);

    // Compare ID of all maps of both databases.
    // If at() throw an exception, the db_loaded does not contain one Entity which should contain

    // hosts
    ASSERT_TRUE(key_compare(db.hosts(),db_loaded.hosts()));

    // users
    ASSERT_TRUE(key_compare(db.users(),db_loaded.users()));

    // processes
    ASSERT_TRUE(key_compare(db.processes(),db_loaded.processes()));

    // domains
    ASSERT_TRUE(key_compare(db.domains(),db_loaded.domains()));

    // topics
    ASSERT_TRUE(key_compare(db.topics(), db_loaded.topics()));
    for (auto it = db.topics().cbegin(); it != db.topics().cend(); ++it)
    {
        ASSERT_TRUE(key_compare(it->second, db_loaded.topics().at(it->first)));
    }

    // participants
    ASSERT_TRUE(key_compare(db.participants(), db_loaded.participants()));
    for (auto it = db.participants().cbegin(); it != db.participants().cend(); ++it)
    {
        ASSERT_TRUE(key_compare(it->second, db_loaded.participants().at(it->first)));
    }

    // locators
    ASSERT_TRUE(key_compare(db.locators(),db_loaded.locators()));

    // DataWriter
    ASSERT_TRUE(key_compare(db.get_dds_endpoints<DataWriter>(), db_loaded.get_dds_endpoints<DataWriter>()));
    for (auto it = db.get_dds_endpoints<DataWriter>().cbegin(); it != db.get_dds_endpoints<DataWriter>().cend(); ++it)
    {
        ASSERT_TRUE(key_compare(it->second, db_loaded.get_dds_endpoints<DataWriter>().at(it->first)));
    }

    // DataReader
    ASSERT_TRUE(key_compare(db.get_dds_endpoints<DataReader>(), db_loaded.get_dds_endpoints<DataReader>()));
    for (auto it = db.get_dds_endpoints<DataReader>().cbegin(); it != db.get_dds_endpoints<DataReader>().cend(); ++it)
    {
        ASSERT_TRUE(key_compare(it->second, db_loaded.get_dds_endpoints<DataReader>().at(it->first)));
    }

    // locators_by_participant
    ASSERT_TRUE(key_compare(db.locators_by_participant(), db_loaded.locators_by_participant()));
    for (auto it = db.locators_by_participant().cbegin(); it != db.locators_by_participant().cend(); ++it)
    {
        ASSERT_TRUE(key_compare(it->second, db_loaded.locators_by_participant().at(it->first)));
    }

    // participants_by_locator
    ASSERT_TRUE(key_compare(db.participants_by_locator(), db_loaded.participants_by_locator()));
    for (auto it = db.participants_by_locator().cbegin(); it != db.participants_by_locator().cend(); ++it)
    {
        ASSERT_TRUE(key_compare(it->second, db_loaded.participants_by_locator().at(it->first)));
    }

    // domains_by_process
    ASSERT_TRUE(key_compare(db.domains_by_process(), db_loaded.domains_by_process()));
    for (auto it = db.domains_by_process().cbegin(); it != db.domains_by_process().cend(); ++it)
    {
        ASSERT_TRUE(key_compare(it->second, db_loaded.domains_by_process().at(it->first)));
    }

    // processes_by_domain
    ASSERT_TRUE(key_compare(db.processes_by_domain(), db_loaded.processes_by_domain()));
    for (auto it = db.processes_by_domain().cbegin(); it != db.processes_by_domain().cend(); ++it)
    {
        ASSERT_TRUE(key_compare(it->second, db_loaded.processes_by_domain().at(it->first)));
    }

    // Compare next_id_ of both databases
    ASSERT_EQ(db.next_id(),db_loaded.next_id());

    // Compare dump of both databases
    ASSERT_EQ(dump, db_loaded.dump_database());

    // Compare data

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
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}