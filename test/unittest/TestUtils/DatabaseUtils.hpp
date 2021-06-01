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
 */
class PopulateDatabase
{
public:

    typedef uint32_t TestId;

    static std::map<TestId, std::shared_ptr<const Entity>> populate_database(
            Database& db)
    {
        std::map<TestId, std::shared_ptr<const Entity>> entities;

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
};
