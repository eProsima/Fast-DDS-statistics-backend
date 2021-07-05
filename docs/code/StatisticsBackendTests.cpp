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

#include <fastdds_statistics_backend/StatisticsBackend.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/types.hpp>
#include <fastdds_statistics_backend/types/Bitmask.hpp>
#include <fastdds_statistics_backend/listener/DomainListener.hpp>
#include <fastdds_statistics_backend/listener/PhysicalListener.hpp>
#include <fastdds_statistics_backend/listener/CallbackMask.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>

using namespace eprosima::statistics_backend;

class CustomDomainListener : public DomainListener
{
};

class CustomPhysicalListener : public PhysicalListener
{
};

void init_monitor_examples()
{
    {
        //CONF-INIT-MONITOR-EXAMPLE
        // Init a monitor in DDS domain 0 with no listener associated.
        EntityId domain_monitor_id =
                StatisticsBackend::init_monitor(0);

        // Init a monitor for a Fast DDS Discovery Server network which server is located in IPv4
        // address 127.0.0.1 and port 11811, and that uses the default GUID prefix
        // eprosima::fastdds::rtps::DEFAULT_ROS2_SERVER_GUIDPREFIX.
        // The monitor has no listener associated.
        EntityId disc_server_monitor_id =
                StatisticsBackend::init_monitor("127.0.0.1:11811");

        // Init a monitor for a Fast DDS Discovery Server network which serveris located in IPv4
        // address 127.0.0.1 and port 11811, and that uses the GUID prefix
        // "44.53.01.5f.45.50.52.4f.53.49.4d.41".
        // The monitor has no listener associated.
        EntityId disc_server_prefix_monitor_id =
                StatisticsBackend::init_monitor("44.53.01.5f.45.50.52.4f.53.49.4d.41", "127.0.0.1:11811");
        //!--
        static_cast<void>(domain_monitor_id);
        static_cast<void>(disc_server_monitor_id);
        static_cast<void>(disc_server_prefix_monitor_id);
    }
    {
        //CONF-INIT-MONITOR-LISTENER-EXAMPLE
        CustomDomainListener domain_listener;

        // Init a monitor in DDS domain 0 with a custom listener.
        EntityId domain_monitor_id =
                StatisticsBackend::init_monitor(0, &domain_listener);

        // Init a monitor for a Fast DDS Discovery Server network which server is located in IPv4
        // address 127.0.0.1 and port 11811, and that uses the default GUID prefix
        // eprosima::fastdds::rtps::DEFAULT_ROS2_SERVER_GUIDPREFIX.
        // The monitor uses a custom listener.
        EntityId disc_server_monitor_id =
                StatisticsBackend::init_monitor("127.0.0.1:11811", &domain_listener);

        // Init a monitor for a Fast DDS Discovery Server network which serveris located in IPv4
        // address 127.0.0.1 and port 11811, and that uses the GUID prefix
        // "44.53.01.5f.45.50.52.4f.53.49.4d.41".
        // The monitor uses a custom listener.
        EntityId disc_server_prefix_monitor_id =
                StatisticsBackend::init_monitor("44.53.01.5f.45.50.52.4f.53.49.4d.41", "127.0.0.1:11811",
                    &domain_listener);
        //!--
        static_cast<void>(domain_monitor_id);
        static_cast<void>(disc_server_monitor_id);
        static_cast<void>(disc_server_prefix_monitor_id);
    }
    {
        //CONF-INIT-MONITOR-MASKS-EXAMPLE
        // Only get notifications when new data is available or when a new host is discovered
        CallbackMask callback_mask = CallbackKind::ON_DATA_AVAILABLE | CallbackKind::ON_HOST_DISCOVERY;

        // Only get notificiations about network latency or subscription throughput
        DataKindMask datakind_mask = DataKind::NETWORK_LATENCY | DataKind::SUBSCRIPTION_THROUGHPUT;

        CustomDomainListener domain_listener;

        // Init a monitor in DDS domain 0 with a custom listener, a CallbackMask, and a DataKindMask
        EntityId domain_monitor_id =
                StatisticsBackend::init_monitor(0, &domain_listener, callback_mask, datakind_mask);

        // Init a monitor for a Fast DDS Discovery Server network which server is located in IPv4
        // address 127.0.0.1 and port 11811, and that uses the default GUID prefix
        // eprosima::fastdds::rtps::DEFAULT_ROS2_SERVER_GUIDPREFIX.
        // The monitor uses a custom listener, a CallbackMask, and a DataKindMask.
        EntityId disc_server_monitor_id =
                StatisticsBackend::init_monitor("127.0.0.1:11811", &domain_listener, callback_mask, datakind_mask);

        // Init a monitor for a Fast DDS Discovery Server network which serveris located in IPv4
        // address 127.0.0.1 and port 11811, and that uses the GUID prefix
        // "44.53.01.5f.45.50.52.4f.53.49.4d.41".
        // The monitor uses a custom listener, a CallbackMask, and a DataKindMask.
        EntityId disc_server_prefix_monitor_id =
                StatisticsBackend::init_monitor("44.53.01.5f.45.50.52.4f.53.49.4d.41", "127.0.0.1:11811",
                    &domain_listener, callback_mask, datakind_mask);
        //!--
        static_cast<void>(domain_monitor_id);
        static_cast<void>(disc_server_monitor_id);
        static_cast<void>(disc_server_prefix_monitor_id);
    }
}

void stop_restart_examples()
{
    {
        //CONF-STOP-RESTART-EXAMPLE
        // Init a monitor in DDS domain 0 with no listener associated
        EntityId domain_monitor_id = StatisticsBackend::init_monitor(0);
        // Stop the monitor
        StatisticsBackend::stop_monitor(domain_monitor_id);
        // Restart the monitor
        StatisticsBackend::restart_monitor(domain_monitor_id);
        //!--
    }
}

void clear_examples()
{
    {
        //CONF-CLEAR-EXAMPLE
        // Init a monitor in DDS domain 0 with no listener associated
        EntityId domain_monitor_id = StatisticsBackend::init_monitor(0);
        // Stop the monitor
        StatisticsBackend::stop_monitor(domain_monitor_id);
        // Clear all data related to the monitor
        StatisticsBackend::clear_monitor(domain_monitor_id);
        //!--
    }
}

void reset_examples()
{
    {
        //CONF-RESET-EXAMPLE
        // Init a monitor in DDS domain 0 with no listener associated
        EntityId domain_monitor_id = StatisticsBackend::init_monitor(0);
        // Stop the monitor
        StatisticsBackend::stop_monitor(domain_monitor_id);
        // Reset Fast DDS Statistics Backend
        StatisticsBackend::reset();
        //!
    }
}

void set_listeners_examples()
{
    {
        //CONF-SET-LISTENERS-EXAMPLE
        // Set a physical listener with all callbacks enabled
        CustomPhysicalListener physical_listener;
        StatisticsBackend::set_physical_listener(&physical_listener, CallbackMask::all());

        // Init a monitor in DDS domain 0 with no listener associated
        EntityId domain_monitor_id = StatisticsBackend::init_monitor(0);

        // Add a domain listener to the monitor with all callbacks enabled and that does no notify
        // of any statistics data
        CustomDomainListener domain_listener;
        StatisticsBackend::set_domain_listener(
            domain_monitor_id, &domain_listener, CallbackMask::all(), DataKindMask::none());
        //!--
    }
}

void get_data_examples()
{
    {
        EntityId datawriter_id;

        //CONF-GET-DATA-DATAWRITER-FASTDDS_LATENCY
        /* Get the DataReaders related to a given DataWriter */
        std::vector<EntityId> datareaders = StatisticsBackend::get_entities(EntityKind::DATAREADER, datawriter_id);

        /* Get the current time */
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

        /*
         * Get the median of the FASTDDS_LATENCY of the last 10 minutes, divided into ten bins,
         * between a given DataWriter and its related DataReaders. After the operation,
         * latency_data.size() is 10. Each of the elements of latency_data is a StatisticsData
         * element which represents the median of the FASTDDS_LATENCY of that minute.
         */
        std::vector<StatisticsData> latency_data = StatisticsBackend::get_data(
            DataKind::FASTDDS_LATENCY,                                   // DataKind
            std::vector<EntityId>({datawriter_id}),                      // Source entities
            datareaders,                                                 // Target entities
            10,                                                          // Number of bins
            now - std::chrono::minutes(10),                              // t_from
            now,                                                         // t_to
            StatisticKind::MEDIAN);                                      // Statistic
        //!--
    }
    {
        EntityId topic_id;

        //CONF-GET-DATA-TOPIC-FASTDDS_LATENCY
        /* Get the DataWriters and DataReaders in a Topic */
        std::vector<EntityId> topic_datawriters = StatisticsBackend::get_entities(EntityKind::DATAWRITER, topic_id);
        std::vector<EntityId> topic_datareaders = StatisticsBackend::get_entities(EntityKind::DATAREADER, topic_id);

        /* Get the current time */
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

        /*
         * Get the median of the FASTDDS_LATENCY of the last 10 minutes, divided into ten bins,
         * between the DataWriters of Host 1 and the DataReaders of Host 2. After the operation,
         * latency_data.size() is 10. Each of the elements of latency_data is a StatisticsData
         * element which represents the median of the FASTDDS_LATENCY of that minute.
         */
        std::vector<StatisticsData> latency_data = StatisticsBackend::get_data(
            DataKind::FASTDDS_LATENCY,                                   // DataKind
            topic_datawriters,                                           // Source entities
            topic_datareaders,                                           // Target entities
            10,                                                          // Number of bins
            now - std::chrono::minutes(10),                              // t_from
            now,                                                         // t_to
            StatisticKind::MEAN);                                        // Statistic
        //!--
    }
    {
        EntityId participant_id;

        //CONF-GET-DATA-TOPIC-HEARTBEAT_COUNT
        std::vector<EntityId> participant_datawriters = StatisticsBackend::get_entities(EntityKind::DATAWRITER,
                        participant_id);

        /* Get the current time */
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

        /*
         * Get the maximum of the HEARTBEAT_COUNT of the last 10 minutes, divided into ten bins,
         * of the DataWriters of a given Participant. After the operation, heartbeat_data.size() is
         * 10. Each of the elements of heartbeat_data is a StatisticsData element which represents
         * the maximum of the HEARTBEAT_COUNT of that minute.
         */
        std::vector<StatisticsData> heartbeat_data = StatisticsBackend::get_data(
            DataKind::HEARTBEAT_COUNT,                                   // DataKind
            participant_datawriters,                                     // Source entities
            10,                                                          // Number of bins
            now - std::chrono::minutes(10),                              // t_from
            now,                                                         // t_to
            StatisticKind::MAX);                                         // Statistic
        //!--
    }
    {
        EntityId host1_id;
        EntityId host2_id;

        //CONF-GET-ALL-POINTS-EXAMPLE
        std::vector<EntityId> host1_datawriters = StatisticsBackend::get_entities(EntityKind::DATAWRITER, host1_id);
        std::vector<EntityId> host2_datareaders = StatisticsBackend::get_entities(EntityKind::DATAREADER, host2_id);

        /* Get the current time */
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();

        /*
         * Get all the FASTDDS_LATENCY data points of the last 10 minutes between the DataWriters
         * of Host 1 and the DataReaders of Host 2. data.size() == total number of data points
         * received. Since bins is 0, the statistic is left as default.
         */
        std::vector<StatisticsData> data = StatisticsBackend::get_data(
            DataKind::FASTDDS_LATENCY,                                   // DataKind
            host1_datawriters,                                           // Source entities
            host2_datareaders,                                           // Target entities
            0,                                                           // Number of bins
            now - std::chrono::minutes(10),                              // t_from
            now);                                                        // t_to
        //!--
    }
}

void get_data_supported_entity_kinds_examples()
{
    {
        EntityId host1_id;
        EntityId host2_id;

        //CONF-GET-FASTDDS-LATENCY-SUPPORTED-ENTITY-KINDS
        /* Get all the EntityKind pairs related to DISCOVERED_ENTITY. */
        std::vector<std::pair<EntityKind, EntityKind>> types_list =
                StatisticsBackend::get_data_supported_entity_kinds(DataKind::DISCOVERY_TIME);

        /* Iterate over all the valid pairs composing the final result */
        std::vector<StatisticsData> discovery_times;
        for (std::pair<EntityKind, EntityKind> type_pair : types_list)
        {
            /* Take the data for this pair and append it to the existing data */
            std::vector<StatisticsData> tmp = StatisticsBackend::get_data(
                DataKind::DISCOVERY_TIME,
                StatisticsBackend::get_entities(type_pair.first, host1_id),
                StatisticsBackend::get_entities(type_pair.second, host2_id));

            discovery_times.insert(discovery_times.end(), tmp.begin(), tmp.end());
        }
        //!--
    }
}

int get_graph_examples(
        uint8_t test)
{
    if (test == 1)
    {
        //CONF-GET-GRAPH-EXAMPLE
        Graph graph = StatisticsBackend::get_graph();
        //!--

        // Load the file to test whether the snippet works on the example
        std::ifstream file_example("graph_example.json");
        graph = Graph::parse(file_example);

        //CONF-NAVIGATE-GRAPH-EXAMPLE
        // Iterate over hosts
        for (const auto& host : graph["hosts"])
        {
            if (host["alive"])
            {
                std::cout << "Host name: " << host["name"] << std::endl;
                std::cout << "Host alias: " << host["alias"] << std::endl;
                // Iterate over users
                for (const auto& user : host["users"])
                {
                    if (user["alive"])
                    {
                        std::cout << "\tUser name: " << user["name"] << std::endl;
                        std::cout << "\tUser alias: " << user["alias"] << std::endl;
                        // Iterate over processes
                        for (const auto& process : user["processes"])
                        {
                            if (process["alive"])
                            {
                                std::cout << "\t\tProcess name: " << process["name"] << std::endl;
                                std::cout << "\t\tProcess alias: " << process["alias"] << std::endl;
                                std::cout << "\t\tProcess PID:  " << process["pid"] << std::endl;
                                // Iterate over the list of participant IDs
                                for (const auto& participant_id : process["participants"])
                                {
                                    // Look for the actual participant in the domains
                                    for (const auto& domain : graph["domains"])
                                    {
                                        for (const auto& participant : domain["participants"])
                                        {
                                            // Check if the participant is the one that is being looked for
                                            if (participant["entity_id"] == participant_id && participant["alive"])
                                            {
                                                std::cout << "\t\t\tParticipant name: " << participant["name"] <<
                                                    std::endl;
                                                std::cout << "\t\t\tParticipant alias: " << participant["alias"] <<
                                                    std::endl;
                                                std::cout << "\t\t\tParticipant GUID: " << participant["guid"] <<
                                                    std::endl;
                                                // Iterate over data writers
                                                for (const auto& datawriter : participant["datawriters"])
                                                {
                                                    if (datawriter["alive"])
                                                    {
                                                        std::cout << "\t\t\t\tDatawriter name: " <<
                                                            datawriter["name"] << std::endl;
                                                        std::cout << "\t\t\t\tDatawriter alias: " <<
                                                            datawriter["alias"] << std::endl;
                                                        std::cout << "\t\t\t\tDatawriter GUID: " <<
                                                            datawriter["guid"] << std::endl;
                                                        // Iterate over topics
                                                        for (const auto& topic : domain["topics"])
                                                        {
                                                            // Check if the topic is the one that is being looked for
                                                            if (topic["entity_id"] == datawriter["topic"])
                                                            {
                                                                std::cout << "\t\t\t\tDatawriter topic name: " <<
                                                                    topic["name"] << std::endl;
                                                                std::cout << "\t\t\t\tDatareader topic alias: " <<
                                                                    topic["alias"] << std::endl;
                                                                break;
                                                            }
                                                        }
                                                        // Iterate over the list of locator IDs
                                                        for (const auto& locator_id : datawriter["locators"])
                                                        {
                                                            for (const auto& locator : graph["locators"])
                                                            {
                                                                // Check if the locator is the one that is being looked for
                                                                if (locator["entity_id"] == locator_id)
                                                                {
                                                                    std::cout << "\t\t\t\tDatawriter locator name: " <<
                                                                        locator["name"] << std::endl;
                                                                    std::cout << "\t\t\t\tDatawriter locator alias: " <<
                                                                        locator["alias"] << std::endl;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                // Iterate over data readers
                                                for (const auto& datareader : participant["datareaders"])
                                                {
                                                    if (datareader["alive"])
                                                    {
                                                        std::cout << "\t\t\t\tDatareader name: " <<
                                                            datareader["name"] << std::endl;
                                                        std::cout << "\t\t\t\tDatareader alias: " <<
                                                            datareader["alias"] << std::endl;
                                                        std::cout << "\t\t\t\tDatareader GUID: " <<
                                                            datareader["guid"] << std::endl;
                                                        // Iterate over topics
                                                        for (const auto& topic : domain["topics"])
                                                        {
                                                            // Check if the topic is the one that is being looked for
                                                            if (topic["entity_id"] == datareader["topic"])
                                                            {
                                                                std::cout << "\t\t\t\tDatareader topic name: " <<
                                                                    topic["name"] << std::endl;
                                                                std::cout << "\t\t\t\tDatareader topic alias: " <<
                                                                    topic["alias"] << std::endl;
                                                                break;
                                                            }
                                                        }
                                                        // Iterate over the list of locator IDs
                                                        for (const auto& locator_id : datareader["locators"])
                                                        {
                                                            for (const auto& locator : graph["locators"])
                                                            {
                                                                // Check if the locator is the one that is being looked for
                                                                if (locator["entity_id"] == locator_id)
                                                                {
                                                                    std::cout << "\t\t\t\tDatareader locator name: " <<
                                                                        locator["name"] << std::endl;
                                                                    std::cout << "\t\t\t\tDatareader locator alias: " <<
                                                                        locator["alias"] << std::endl;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        //!--
        return 0;
    }
    return 0;
}

void dump_load_examples()
{
    {
        //CONF-DUMP-LOAD-EXAMPLE
        // Save the database to a file
        StatisticsBackend::dump_database("new_backend_dump.json", false);

        // Reset the Backend to empty the current database contents
        StatisticsBackend::reset();

        // Load an old backup to the emptied Backend
        StatisticsBackend::load_database("old_backend_dump.json");
        //!--
    }
    {
        //CONF-DUMP-AND_CLEAR-EXAMPLE
        // Save the database to a file, cleaning the statistics data
        StatisticsBackend::dump_database("new_backend_dump.json", true);
        //!--
    }
}

void get_entities_example()
{
    {
        //CONF-GET-ENTITIES-DEFAULT-EXAMPLE
        // Get all hosts
        std::vector<EntityId> hosts = StatisticsBackend::get_entities(EntityKind::HOST);
        for (EntityId host : hosts)
        {
            std::cout << "Host ID: " << host << std::endl;
        }
        //!--
    }
    {
        //CONF-GET-ENTITIES-ALL-EXAMPLE
        StatisticsBackend::get_entities(EntityKind::HOST, EntityId::all());
        //!--
    }
    {
        EntityId host_id;
        //CONF-GET-ENTITIES-EXAMPLE
        // Get all participants running in a host
        std::vector<EntityId> participants = StatisticsBackend::get_entities(EntityKind::PARTICIPANT, host_id);
        for (EntityId participant : participants)
        {
            std::cout << "Participant ID: " << participant << std::endl;
        }
        //!--
    }
}

void get_info_example()
{
    {
        EntityId host_id;
        EntityId user_id;
        EntityId process_id;
        EntityId locator_id;
        EntityId domain_id;
        EntityId participant_id;
        EntityId datawriter_id;
        EntityId datareader_id;
        EntityId topic_id;
        //CONF-GET-QOS-EXAMPLE
        Info host_info = StatisticsBackend::get_info(host_id);
        Info user_info = StatisticsBackend::get_info(user_id);
        Info process_info = StatisticsBackend::get_info(process_id);
        Info locator_info = StatisticsBackend::get_info(locator_id);
        Info domain_info = StatisticsBackend::get_info(domain_id);
        Info participant_info = StatisticsBackend::get_info(participant_id);
        Info datareader_info = StatisticsBackend::get_info(datareader_id);
        Info datawriter_info = StatisticsBackend::get_info(datawriter_id);
        Info topic_info = StatisticsBackend::get_info(topic_id);
        //!--
    }
}

void get_type_example()
{
    {
        EntityId entity_id;
        //CONF-GET-TYPE-EXAMPLE
        EntityKind kind = StatisticsBackend::get_type(entity_id);
        //!--
        static_cast<void>(kind);
    }
}

void set_alias_example()
{
    {
        EntityId entity_id;
        //CONF-SET-ALIAS-EXAMPLE
        StatisticsBackend::set_alias(entity_id, "my_alias");
        //!--
    }
}

void is_active_example()
{
    {
        EntityId entity_id;
        //CONF-IS-ACTIVE-EXAMPLE
        bool active = StatisticsBackend::is_active(entity_id);
        //!--
        static_cast<void>(active);
    }
}

void entity_id()
{
    {
        //ENTITYID-ALL-EXAMPLE
        EntityId all = EntityId::all();
        //!--
        static_cast<void>(all);
    }
    {
        //ENTITYID-INVALID-EXAMPLE
        EntityId invalid = EntityId::invalid();
        //!--
        static_cast<void>(invalid);
    }
    {
        //ENTITYID-INVALIDATE-EXAMPLE
        EntityId entity_id;
        entity_id.invalidate();
        //!--
        static_cast<void>(entity_id);
    }
    {
        //ENTITYID-VALID-EXAMPLE
        EntityId entity_id;
        bool check = entity_id.is_valid();
        //!--
        static_cast<void>(entity_id);
        static_cast<void>(check);
    }
    {
        //ENTITYID-IS_ALL-EXAMPLE
        EntityId entity_id;
        bool check = entity_id.is_all();
        //!--
        static_cast<void>(entity_id);
        static_cast<void>(check);
    }
    {
        //ENTITYID-VALID_AND_UNIQUE-EXAMPLE
        EntityId entity_id;
        bool check = entity_id.is_valid_and_unique();
        //!--
        static_cast<void>(entity_id);
        static_cast<void>(check);
    }
    {
        //ENTITYID-COMPARE-EXAMPLE
        EntityId entity_id_1;
        EntityId entity_id_2;
        bool check = entity_id_1 < entity_id_2;
        static_cast<void>(check);
        check = entity_id_1 <= entity_id_2;
        static_cast<void>(check);
        check = entity_id_1 > entity_id_2;
        static_cast<void>(check);
        check = entity_id_1 >= entity_id_2;
        static_cast<void>(check);
        check = entity_id_1 == entity_id_2;
        static_cast<void>(check);
        check = entity_id_1 != entity_id_2;
        //!--
        static_cast<void>(entity_id_1);
        static_cast<void>(entity_id_2);
        static_cast<void>(check);
    }
    {
        //ENTITYID-OSTREAM-EXAMPLE
        EntityId entity_id;
        std::cout << "EntityId: " << entity_id << std::endl;
        //!--
        static_cast<void>(entity_id);
    }
}

int main(
        int argc,
        const char** argv)
{
    if (argc != 2)
    {
        std::cout << "Wrong number of arguments" << std::endl;
        return 1;
    }

    if (strncmp(argv[1], "get_graph_parse", 15) == 0)
    {
        return get_graph_examples(1);
    }
    return 0;
}
