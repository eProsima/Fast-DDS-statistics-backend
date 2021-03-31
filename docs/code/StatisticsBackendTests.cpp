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

#include <fastdds-statistics-backend/StatisticsBackend.hpp>
#include <fastdds-statistics-backend/types/EntityId.hpp>
#include <fastdds-statistics-backend/types/types.hpp>
#include <fastdds-statistics-backend/types/Bitmask.hpp>
#include <fastdds-statistics-backend/listener/DomainListener.hpp>
#include <fastdds-statistics-backend/listener/PhysicalListener.hpp>
#include <fastdds-statistics-backend/listener/CallbackMask.hpp>

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
        // Init a monitor in DDS domain 0 with no listener associated
        EntityId domain_monitor_id = StatisticsBackend::init_monitor(0);

        // Init a monitor for a Fast DDS Discovery Server network which server is located in IPv4
        // address 127.0.0.1 and port 11811. The monitor has no listener associated
        EntityId disc_server_monitor_id = StatisticsBackend::init_monitor("127.0.0.1:11811");
        //!--
        static_cast<void>(domain_monitor_id);
        static_cast<void>(disc_server_monitor_id);
    }
    {
        //CONF-INIT-MONITOR-LISTENER-EXAMPLE
        // Init a monitor in DDS domain 0 with a custom listener
        CustomDomainListener domain_listener;
        EntityId domain_monitor_id = StatisticsBackend::init_monitor(0, &domain_listener);
        //!--
        static_cast<void>(domain_monitor_id);
    }
    {
        //CONF-INIT-MONITOR-MASKS-EXAMPLE
        // Only get notifications when new data is available or when a new host is discovered
        CallbackMask callback_mask = CallbackKind::ON_DATA_AVAILABLE | CallbackKind::ON_HOST_DISCOVERY;

        // Only get notificiations about network latency or subscription throughput
        DataKindMask datakind_mask = DataKind::NETWORK_LATENCY | DataKind::SUBSCRIPTION_THROUGHPUT;

        // Init a monitor in DDS domain 0 with a custom listener, a CallbackMask, and a DataKindMask
        CustomDomainListener domain_listener;
        EntityId domain_monitor_id = StatisticsBackend::init_monitor(0, &domain_listener, callback_mask, datakind_mask);
        //!--
        static_cast<void>(domain_monitor_id);
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
        EntityId source_entity_id;
        EntityId target_entity_id;

        //CONF-GET-DATA-OVERLOAD-EXAMPLE
        /*
         * Get the median of the FASTDDS_LATENCY of the last 10 minutes, divided into ten bins,
         * between a given source and a target entity. After the operation, latency_data.size() is
         * 10. Each of the elements of latency_data is a StatisticsData element which represents the
         * median of the FASTDDS_LATENCY of that minute.
         */
        std::vector<StatisticsData> latency_data = StatisticsBackend::get_data(
            DataKind::FASTDDS_LATENCY,                                   // DataKind
            source_entity_id,                                            // Source entity
            target_entity_id,                                            // Target entity
            10,                                                          // Number of bins
            std::chrono::system_clock::now() - std::chrono::minutes(10), // t_from
            std::chrono::system_clock::now(),                            // t_to
            StatisticKind::MEDIAN);                                      // Statistic

        /*
         * Get the maximum of the HEARTBEAT_COUNT of the last 10 minutes, divided into ten bins,
         * of a given source entity. After the operation, heartbeat_data.size() is 10. Each of the
         * elements of heartbeat_data is a StatisticsData element which represents the maximum of
         * the HEARTBEAT_COUNT of that minute.
         */
        std::vector<StatisticsData> heartbeat_data = StatisticsBackend::get_data(
            DataKind::HEARTBEAT_COUNT,                                   // DataKind
            source_entity_id,                                            // Source entity
            10,                                                          // Number of bins
            std::chrono::system_clock::now() - std::chrono::minutes(10), // t_from
            std::chrono::system_clock::now(),                            // t_to
            StatisticKind::MAX);                                         // Statistic
        //!--

        //CONF-GET-ALL-POINTS-EXAMPLE
        /*
         * Get all the FASTDDS_LATENCY data points of the last 10 minutes between a source and a
         * target entity. data.size() == total number of data points received. Since bins is 0,
         * the statistic is left as default.
         */
        std::vector<StatisticsData> data = StatisticsBackend::get_data(
            DataKind::FASTDDS_LATENCY,                                   // DataKind
            source_entity_id,                                            // Source entity
            target_entity_id,                                            // Target entity
            0,                                                           // Number of bins
            std::chrono::system_clock::now() - std::chrono::minutes(10), // t_from
            std::chrono::system_clock::now());                           // t_to
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
        for (auto host : graph["hosts"])
        {
            std::cout << "Host name: " << host["name"] << std::endl;
            // Iterate over users
            for (auto user : host["users"])
            {
                std::cout << "\tUser name: " << user["name"] << std::endl;
                // Iterate over processes
                for (auto process : user["processes"])
                {
                    std::cout << "\t\tProcess name: " << process["name"] << std::endl;
                    std::cout << "\t\tProcess PID:  " << process["pid"] << std::endl;
                    // Iterate over the list of participant IDs
                    for (auto participant_id : process["participants"])
                    {
                        // Look for the actual participant in the domains
                        for (auto domain : graph["domains"])
                        {
                            for (auto participant : domain["participants"])
                            {
                                // Check if the participant is the one that is being looked for
                                if (participant["entity_id"] == participant_id)
                                {
                                    std::cout << "\t\t\tParticipant name: " << participant["name"] << std::endl;
                                    std::cout << "\t\t\tParticipant GUID: " << participant["guid"] << std::endl;
                                    // Iterate over data writers
                                    for (auto datawriter : participant["datawriters"])
                                    {
                                        std::cout << "\t\t\t\tDatawriter name: " << datawriter["name"] << std::endl;
                                        std::cout << "\t\t\t\tDatawriter GUID: " << datawriter["guid"] << std::endl;
                                    }
                                    // Iterate over data readers
                                    for (auto datareader : participant["datareaders"])
                                    {
                                        std::cout << "\t\t\t\tDatareader name: " << datareader["name"] << std::endl;
                                        std::cout << "\t\t\t\tDatareader GUID: " << datareader["guid"] << std::endl;
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

void get_entities_example()
{
    {
        //CONF-GET-ENTITIES-ALL-EXAMPLE
        // Get all hosts
        std::vector<EntityId> hosts = StatisticsBackend::get_entities(EntityKind::HOST);
        for (EntityId host : hosts)
        {
            std::cout << "Host ID: " << host << std::endl;
        }
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
        bool check = entity_id.valid();
        //!--
        static_cast<void>(entity_id);
        static_cast<void>(check);
    }
    {
        //ENTITYID-COMPARE-EXAMPLE
        EntityId entity_id_1;
        EntityId entity_id_2;
        bool check = entity_id_1 < entity_id_2;
        check = entity_id_1 <= entity_id_2;
        check = entity_id_1 > entity_id_2;
        check = entity_id_1 >= entity_id_2;
        check = entity_id_1 == entity_id_2;
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
