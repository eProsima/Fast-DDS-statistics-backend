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


using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

/**
 * This class populates a database with several entities and several relations among them.
 * Since the tests need to know these relations, each entity is given a unique identifier within the class,
 * and the class keeps all these entities on a map,
 * for the test to be able to get them and check the result of the execution.
 * This identifier is independent of the EntityId given by the database,
 * and is totally under control of the class and the tests.
 * This is important to keep the tests stable even if the Database implementation to assign an EntityId changes.
 *
 * Identifier '0' does not track any entity, instead is used to insert an entity with EntityId:all()
 * for use on several tests.
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
 *  Also insert Statistics data of all types with test values in participant2, datawriter2, datareader2 and locator2
 */
class PopulateDatabase
{
public:

    typedef uint32_t TestId;

    static std::map<TestId, std::shared_ptr<const Entity>> populate_database(
            Database& db)
    {
        std::map<TestId, std::shared_ptr<const Entity>> entities;

        auto entityAll = std::make_shared<Entity>(EntityKind::INVALID, "ALL");
        entityAll->id = EntityId::all();
        entities[0] = entityAll;

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

        auto reader_locator1 = std::make_shared<Locator>("reader_locator1");
        db.insert(reader_locator1);
        entities[17] = reader_locator1;

        auto datareader1 = std::make_shared<DataReader>("datareader1", "qos", "11.12.13.14", participant2, topic2);
        db.insert(datareader1);
        db.link_endpoint_with_locator(datareader1->id, reader_locator1->id);
        entities[13] = datareader1;

        auto reader_locator2 = std::make_shared<Locator>("reader_locator2");
        db.insert(reader_locator2);
        entities[18] = reader_locator2;

        auto datareader2 = std::make_shared<DataReader>("datareader2", "qos", "15.16.17.18", participant2, topic2);
        db.insert(datareader2);
        db.link_endpoint_with_locator(datareader2->id, reader_locator1->id);
        db.link_endpoint_with_locator(datareader2->id, reader_locator2->id);
        entities[14] = datareader2;

        auto writer_locator1 = std::make_shared<Locator>("writer_locator1");
        db.insert(writer_locator1);
        entities[19] = writer_locator1;

        auto datawriter1 = std::make_shared<DataWriter>("datawriter1", "qos", "21.22.23.24", participant2, topic2);
        db.insert(datawriter1);
        db.link_endpoint_with_locator(datawriter1->id, writer_locator1->id);
        entities[15] = datawriter1;

        auto writer_locator2 = std::make_shared<Locator>("writer_locator2");
        db.insert(writer_locator2);
        entities[20] = writer_locator2;

        auto datawriter2 = std::make_shared<DataWriter>("datawriter2", "qos", "25.26.27.28", participant2, topic2);
        db.insert(datawriter2);
        db.link_endpoint_with_locator(datawriter2->id, writer_locator1->id);
        db.link_endpoint_with_locator(datawriter2->id, writer_locator2->id);
        entities[16] = datawriter2;

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

        return entities;
    }

};

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
