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

#include <atomic>
#include <chrono>
#include <fstream>
#include <map>
#include <memory>
#include <string>

#include <fastdds_statistics_backend/StatisticsBackend.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/types.hpp>

#include <database/data.hpp>
#include <database/database.hpp>
#include <database/entities.hpp>
#include <database/samples.hpp>
#include <StatisticsBackendData.hpp>

constexpr const char* EMPTY_DUMP_FILE = "../Resources/empty_dump.json";
constexpr const char* EMPTY_ENTITIES_DUMP_FILE = "../Resources/empty_entities_dump.json";
constexpr const char* SIMPLE_DUMP_FILE = "../Resources/simple_dump.json";
constexpr const char* COMPLEX_DUMP_FILE = "../Resources/complex_dump.json";
constexpr const char* NO_PROCESS_PARTICIPANT_LINK_DUMP_FILE =
        "../Resources/simple_dump_no_process_participant_link.json";
constexpr const char* NO_PROCESS_PARTICIPANT_LINK_ERASED_DOMAIN_DUMP_FILE =
        "../Resources/simple_dump_no_process_participant_link_erased_domain.json";
constexpr const char* OLD_COMPLEX_DUMP_FILE = "../Resources/old_complex_dump.json";
constexpr const char* COMPLEX_ERASED_DUMP_FILE = "../Resources/complex_dump_erased_domain_1.json";

constexpr const char* DESCRIPTION_TAG = "description";

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
 *     "ID": 15,
 *     "locators": ["reader_locator1", "reader_locator2"]
 *   }
 *   "datawriter1": {
 *     "ID": 17,
 *     "locators": ["writer_locator1"]
 *   }
 *   "datawriter2": {
 *     "ID": 19,
 *     "locators": ["writer_locator1", "writer_locator2"]
 *   }
 *   "reader_locator1": {
 *     "ID": 14
 *   }
 *   "reader_locator2": {
 *     "ID": 16
 *   }
 *   "writer_locator1": {
 *     "ID": 18
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

    typedef size_t TestId;

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

        auto participant1 = std::make_shared<DomainParticipant>("participant1", "qos",
                        "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c1", nullptr,
                        domain2);
        db.insert(participant1);
        db.link_participant_with_process(participant1->id, process2->id);
        entities[9] = participant1;

        auto participant2 = std::make_shared<DomainParticipant>("participant2", "qos",
                        "01.02.03.04.05.06.07.08.09.0a.0b.0c|0.0.1.c2", nullptr,
                        domain2);
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
        entities[14] = reader_locator1;

        auto datareader2 = std::make_shared<DataReader>("datareader2", "qos", "15.16.17.18", participant2, topic2);
        auto reader_locator2 = std::make_shared<Locator>("reader_locator2");
        reader_locator2->id = db.generate_entity_id();
        datareader2->locators[reader_locator1->id] = reader_locator1;
        datareader2->locators[reader_locator2->id] = reader_locator2;
        db.insert(datareader2);
        entities[15] = datareader2;
        entities[16] = reader_locator2;

        auto datawriter1 = std::make_shared<DataWriter>("datawriter1", "qos", "21.22.23.24", participant2, topic2);
        auto writer_locator1 = std::make_shared<Locator>("writer_locator1");
        writer_locator1->id = db.generate_entity_id();
        datawriter1->locators[writer_locator1->id] = writer_locator1;
        db.insert(datawriter1);
        entities[17] = datawriter1;
        entities[18] = writer_locator1;

        auto datawriter2 = std::make_shared<DataWriter>("datawriter2", "qos", "25.26.27.28", participant2, topic2);
        auto writer_locator2 = std::make_shared<Locator>("writer_locator2");
        writer_locator2->id = db.generate_entity_id();
        datawriter2->locators[writer_locator1->id] = writer_locator1;
        datawriter2->locators[writer_locator2->id] = writer_locator2;
        db.insert(datawriter2);
        entities[19] = datawriter2;
        entities[20] = writer_locator2;

        // Insert datas on domain2

        // participants
        {
            EntityId domainId = participant2->domain->id;
            EntityId entityId = participant2->id;

            // discovery_time
            {
                DiscoveryTimeSample sample;

                sample.src_ts = nanoseconds_to_systemclock(2);
                sample.time = nanoseconds_to_systemclock(1);
                sample.remote_entity = EntityId(entityId);
                sample.discovered = true;

                db.insert(domainId, entityId, sample);
            }

            // pdp_packets
            {
                PdpCountSample sample;

                sample.src_ts = nanoseconds_to_systemclock(0);
                sample.count = 1;

                db.insert(domainId, entityId, sample);
            }

            // edp_packets
            {
                EdpCountSample sample;

                sample.src_ts = nanoseconds_to_systemclock(1);
                sample.count = 0;

                db.insert(domainId, entityId, sample);
            }

            // rtps_packets_sent
            {
                RtpsPacketsSentSample sample;

                sample.src_ts = nanoseconds_to_systemclock(1);
                sample.count = 0;
                sample.remote_locator = EntityId(reader_locator2->id);

                db.insert(domainId, entityId, sample);
            }

            // rtps_bytes_sent
            {
                RtpsBytesSentSample sample;

                sample.src_ts = nanoseconds_to_systemclock(1);
                sample.count = 0;
                sample.magnitude_order = 0;
                sample.remote_locator = EntityId(reader_locator2->id);

                db.insert(domainId, entityId, sample);
            }

            // rtps_packets_lost
            {
                RtpsPacketsLostSample sample;

                sample.src_ts = nanoseconds_to_systemclock(1);
                sample.count = 0;
                sample.remote_locator = EntityId(reader_locator2->id);

                db.insert(domainId, entityId, sample);
            }

            // rtps_bytes_lost
            {
                RtpsBytesLostSample sample;

                sample.src_ts = nanoseconds_to_systemclock(1);
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

                sample.src_ts = nanoseconds_to_systemclock(1);
                sample.data = 1.1;

                db.insert(domainId, entityId, sample);
            }

            // resent_datas
            {
                ResentDataSample sample;

                sample.src_ts = nanoseconds_to_systemclock(1);
                sample.count = 0;

                db.insert(domainId, entityId, sample);
            }

            // heartbeat_count
            {
                HeartbeatCountSample sample;

                sample.src_ts = nanoseconds_to_systemclock(1);
                sample.count = 0;

                db.insert(domainId, entityId, sample);
            }

            // gap_count
            {
                GapCountSample sample;

                sample.src_ts = nanoseconds_to_systemclock(1);
                sample.count = 0;

                db.insert(domainId, entityId, sample);
            }

            // data_count
            {
                DataCountSample sample;

                sample.src_ts = nanoseconds_to_systemclock(1);
                sample.count = 0;

                db.insert(domainId, entityId, sample);
            }

            // samples_datas
            {
                SampleDatasCountSample sample;

                sample.src_ts = nanoseconds_to_systemclock(1);
                sample.count = 0;
                sample.sequence_number = process2->id.value();

                db.insert(domainId, entityId, sample);
            }

            // history2history_latency
            {
                HistoryLatencySample sample;

                sample.src_ts = nanoseconds_to_systemclock(1);
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

                sample.src_ts = nanoseconds_to_systemclock(1);
                sample.data = 1.1;

                db.insert(domainId, entityId, sample);
            }

            // acknack_count
            {
                AcknackCountSample sample;

                sample.src_ts = nanoseconds_to_systemclock(1);
                sample.count = 0;

                db.insert(domainId, entityId, sample);
            }

            // nackfrag_count
            {
                NackfragCountSample sample;

                sample.src_ts = nanoseconds_to_systemclock(1);
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

                sample.src_ts = nanoseconds_to_systemclock(1);
                sample.data = 1.1;
                sample.remote_locator = EntityId(entityId);

                db.insert(EntityId::invalid(), entityId, sample);
            }

            // reader_locator
            {
                EntityId entityId = reader_locator2->id;

                NetworkLatencySample sample;

                sample.src_ts = nanoseconds_to_systemclock(1);
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

    const std::map<EntityId, std::shared_ptr<Host>>& hosts() const
    {
        return hosts_;
    }

    const std::map<EntityId, std::shared_ptr<User>>& users() const
    {
        return users_;
    }

    const std::map<EntityId, std::shared_ptr<Process>>& processes() const
    {
        return processes_;
    }

    const std::map<EntityId, std::shared_ptr<Domain>>& domains() const
    {
        return domains_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<Topic>>>& topics() const
    {
        return topics_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>>& participants() const
    {
        return participants_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<DataWriter>>>& datawriters() const
    {
        return datawriters_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<DataReader>>>& datareaders() const
    {
        return datareaders_;
    }

    const std::map<EntityId, std::shared_ptr<Locator>>& locators() const
    {
        return locators_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<Locator>>>& locators_by_participant() const
    {
        return locators_by_participant_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<DomainParticipant>>>& participants_by_locator() const
    {
        return participants_by_locator_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<Domain>>>& domains_by_process() const
    {
        return domains_by_process_;
    }

    const std::map<EntityId, std::map<EntityId, std::shared_ptr<Process>>>& processes_by_domain() const
    {
        return processes_by_domain_;
    }

    template<typename T>
    FASTDDS_STATISTICS_BACKEND_DllAPI
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

    inline std::string get_id_to_string(
            EntityId id)
    {
        return id_to_string(id);
    }

    inline std::string get_time_to_string(
            std::chrono::system_clock::time_point time)
    {
        return time_to_string(time);
    }

    inline long long get_string_to_int(
            std::string const& str)
    {
        return string_to_int(str);
    }

    inline unsigned long long get_string_to_uint(
            std::string const& str)
    {
        return string_to_uint(str);
    }

    void change_entity_status_test(
            const EntityId& entity_id,
            bool active,
            const EntityId domain_id)
    {
        EntityKind entity_kind = get_entity_kind(entity_id);
        change_entity_status_of_kind(entity_id, active, entity_kind, domain_id);
    }

};

/**
 * Wrapper class around StatisticsBackend to access protected methods and data to use in tests
 */
class StatisticsBackendTest : public StatisticsBackend
{
public:

    static void set_database(
            Database* db)
    {
        details::StatisticsBackendData::get_instance()->database_.reset(db);

        details::StatisticsBackendData::get_instance()->monitors_by_entity_.clear();

        // Need to reconstruct the monitors
        auto domains = details::StatisticsBackendData::get_instance()->database_->get_entities(
            EntityKind::DOMAIN, EntityId::all());
        for (auto domain : domains)
        {
            std::shared_ptr<details::Monitor> monitor = std::make_shared<details::Monitor>();
            std::stringstream domain_name;
            domain_name << domain->name;
            monitor->id = domain->id;
            monitor->domain_listener = nullptr;
            details::StatisticsBackendData::get_instance()->monitors_by_entity_[domain->id] = monitor;
        }
    }

};

/**
 * Load a .json file, returning a dump of it.
 * Also remove meta information not necessary on dump.
 */

void load_file(
        std::string filename,
        DatabaseDump& dump)
{
    // Check if the file exists
    std::ifstream file(filename);
    if (!file.good())
    {
        FAIL() << "File " + filename + " does not exist";
    }

    // Get the json
    file >> dump;

    // Erase the description tag if existing
    if (dump.contains(DESCRIPTION_TAG))
    {
        dump.erase(DESCRIPTION_TAG);
    }
}
