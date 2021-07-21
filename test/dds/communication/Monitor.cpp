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

/**
 * @file Monitor.cpp
 */

#include <list>
#include <string>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/statistics/topic_names.hpp>
#include <fastrtps/xmlparser/XMLProfileManager.h>

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/listener/CallbackMask.hpp>
#include <fastdds_statistics_backend/listener/DomainListener.hpp>
#include <fastdds_statistics_backend/StatisticsBackend.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/types.hpp>

#include <database/database_queue.hpp>
#include <Monitor.hpp>
#include <StatisticsBackendData.hpp>
#include <topic_types/typesPubSubTypes.h>

using namespace eprosima::statistics_backend;
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::statistics;

class MonitorListener : public DomainListener
{
public:

    MonitorListener()
        : num_entities_discovered_(0)
    {
    }

    virtual ~MonitorListener() override
    {
    }

    /*!
     * This function is called when a new DomainParticipant is discovered by the library,
     * or a previously discovered DomainParticipant changes its QOS or is removed.
     *
     * @param domain_id Entity ID of the domain in which the DataReader has been discovered.
     * @param participant_id Entity ID of the discovered DomainParticipant.
     * @param status The status of the discovered DomainParticipants.
     */
    void on_participant_discovery(
            EntityId domain_id,
            EntityId participant_id,
            const Status& status) override
    {
        static_cast<void>(domain_id);
        static_cast<void>(participant_id);

        std::unique_lock<std::mutex> lock(mutex_);
        num_entities_discovered_ += status.current_count_change;
        cv_.notify_all();
    }

    /*!
     * This function is called when a new Topic is discovered by the library.
     *
     * @param domain_id Entity ID of the domain in which the topic has been discovered.
     * @param topic_id Entity ID of the discovered topic.
     * @param status The status of the discovered topic.
     */
    virtual void on_topic_discovery(
            EntityId domain_id,
            EntityId topic_id,
            const Status& status)
    {
        static_cast<void>(domain_id);
        static_cast<void>(topic_id);

        std::unique_lock<std::mutex> lock(mutex_);
        num_entities_discovered_ += status.current_count_change;
        cv_.notify_all();
    }

    /*!
     * This function is called when a new DataReader is discovered by the library,
     * or a previously discovered DataReader changes its QOS or is removed.
     *
     * @param domain_id Entity ID of the domain in which the DataReader has been discovered.
     * @param datareader_id Entity ID of the discovered DataReader.
     * @param status The status of the discovered DataReaders.
     */
    virtual void on_datareader_discovery(
            EntityId domain_id,
            EntityId datareader_id,
            const Status& status)
    {
        static_cast<void>(domain_id);
        static_cast<void>(datareader_id);

        std::unique_lock<std::mutex> lock(mutex_);
        num_entities_discovered_ += status.current_count_change;
        cv_.notify_all();
    }

    /*!
     * This function is called when a new DataWriter is discovered by the library,
     * or a previously discovered DataWriter changes its QOS or is removed.
     *
     * @param domain_id Entity ID of the domain in which the DataWriter has been discovered.
     * @param datawriter_id Entity ID of the discovered DataWriter.
     * @param status The status of the discovered DataWriters.
     */
    virtual void on_datawriter_discovery(
            EntityId domain_id,
            EntityId datawriter_id,
            const Status& status)
    {
        static_cast<void>(domain_id);
        static_cast<void>(datawriter_id);

        std::unique_lock<std::mutex> lock(mutex_);
        num_entities_discovered_ += status.current_count_change;
        cv_.notify_all();
    }

    std::mutex mutex_;
    std::condition_variable cv_;
    unsigned int num_entities_discovered_;
};


/* ARGUMENTS
 */

int main(
        int argc,
        char** argv)
{
    int arg_count = 1;
    uint32_t seed = 7800;
    unsigned int num_participants = 2;
    unsigned int num_readers = 1;
    // Each participant creates a meta traffic endpoint
    unsigned int num_writers = num_participants + 1;
    // Additionally, we need a meta traffic topic per domain
    unsigned int num_topics = 2;
    unsigned int num_entities = num_participants + num_topics + num_readers + num_writers;

    // Process arguments
    while (arg_count < argc)
    {
        if (strcmp(argv[arg_count], "--seed") == 0)
        {
            if (++arg_count >= argc)
            {
                std::cout << "--seed expects a parameter" << std::endl;
                return -1;
            }

            seed = strtol(argv[arg_count], nullptr, 10);
        }

        ++arg_count;
    }

    DomainId domain_id = seed % 230;
    MonitorListener listener;
    EntityId monitor_id;

    // Test: The database starts empty
    {
        // Check the database is empty
        try
        {
            if (!StatisticsBackend::get_entities(EntityKind::HOST).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::USER).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::PROCESS).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::DOMAIN).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::TOPIC).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::PARTICIPANT).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::DATAWRITER).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::DATAREADER).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::LOCATOR).empty())
            {
                throw Error("Error: database contains unexpected entities");
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return 1;
        }
    }

    // Test: Init the monitor activate the monitor entity
    {
        // Init the monitor
        domain_id = seed % 230;
        monitor_id = StatisticsBackend::init_monitor(domain_id, &listener);
        if (!monitor_id.is_valid())
        {
            std::cout << "Error creating monitor" << std::endl;
            return 1;
        }

        // Check the database only contains the monitor domain
        try
        {
            if (!StatisticsBackend::get_entities(EntityKind::HOST).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::USER).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::PROCESS).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::TOPIC).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::PARTICIPANT).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::DATAWRITER).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::DATAREADER).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::LOCATOR).empty() ||
                    (StatisticsBackend::get_entities(EntityKind::DOMAIN).size() != 1) ||
                    (StatisticsBackend::get_entities(EntityKind::DOMAIN).begin()->value() != monitor_id))
            {
                throw Error("Error: database contains unexpected entities");
            }
            for (auto entity : StatisticsBackend::get_entities(EntityKind::DOMAIN, monitor_id))
            {
                if (!StatisticsBackend::is_active(entity))
                {
                    throw Error("Error: DOMAIN with id: " + std::to_string(
                                      entity.value()) + " is inactive after init_monitor");
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return 1;
        }
    }

    // Test: After discover participants and endpoints, all the entities are active
    {
        // Start the subprocesses
        std::cout << "Init Monitor_" << seed << std::endl;

        // Wait until discover all entities
        {
            std::unique_lock<std::mutex> lock(listener.mutex_);
            listener.cv_.wait(lock, [&]
                    {
                        return listener.num_entities_discovered_ >= num_entities;
                    });
        }

        // Check that the database contains all entities related to subscriber and publisher entities
        // and check that all entities are active
        try
        {
            if (!StatisticsBackend::get_entities(EntityKind::HOST).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::USER).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::PROCESS).empty())
            {
                throw Error("Error: database contains unexpected entities");
            }
            else if (StatisticsBackend::get_entities(EntityKind::DOMAIN).size() != 1 ||
                    StatisticsBackend::get_entities(EntityKind::DOMAIN).begin()->value() != monitor_id)
            {
                throw Error("Error: database contains unexpected DOMAIN");
            }
            else if (StatisticsBackend::get_entities(EntityKind::TOPIC).size() != num_topics ||
                    StatisticsBackend::get_entities(EntityKind::TOPIC, monitor_id).size() != num_topics)
            {
                throw Error("Error: database contains unexpected TOPIC");
            }
            else if (StatisticsBackend::get_entities(EntityKind::PARTICIPANT).size() != num_participants ||
                    StatisticsBackend::get_entities(EntityKind::PARTICIPANT, monitor_id).size() != num_participants)
            {
                throw Error("Error: database contains unexpected PARTICIPANT");
            }
            else if (StatisticsBackend::get_entities(EntityKind::DATAWRITER).size() != num_writers ||
                    StatisticsBackend::get_entities(EntityKind::DATAWRITER, monitor_id).size() != num_writers)
            {
                throw Error("Error: database contains unexpected DATAWRITER");
            }
            else if (StatisticsBackend::get_entities(EntityKind::DATAREADER).size() != num_readers ||
                    StatisticsBackend::get_entities(EntityKind::DATAREADER, monitor_id).size() != num_readers)
            {
                throw Error("Error: database contains unexpected DATAREADER");
            }
            else if (StatisticsBackend::get_entities(EntityKind::LOCATOR).size() != num_readers + num_writers ||
                    StatisticsBackend::get_entities(EntityKind::LOCATOR, monitor_id).size() != num_readers + num_writers)
            {
                throw Error("Error: database contains unexpected LOCATOR");
            }
            for (auto entity : StatisticsBackend::get_entities(EntityKind::DOMAIN, monitor_id))
            {
                if (!StatisticsBackend::is_active(entity))
                {
                    throw Error("Error: DOMAIN with id: " + std::to_string(
                                      entity.value()) + " is inactive after discovering participants");
                }
            }
            for (auto entity : StatisticsBackend::get_entities(EntityKind::TOPIC, monitor_id))
            {
                if (!StatisticsBackend::is_active(entity))
                {
                    throw Error("Error: TOPIC with id: " + std::to_string(
                                      entity.value()) + " is inactive after discovering participants");
                }
            }
            for (auto entity : StatisticsBackend::get_entities(EntityKind::PARTICIPANT, monitor_id))
            {
                if (!StatisticsBackend::is_active(entity))
                {
                    throw Error("Error: PARTICIPANT with id: " + std::to_string(
                                      entity.value()) + " is inactive after discovering participants");
                }
            }
            for (auto entity : StatisticsBackend::get_entities(EntityKind::DATAWRITER, monitor_id))
            {
                if (!StatisticsBackend::is_active(entity))
                {
                    throw Error("Error: DATAWRITER with id: " + std::to_string(
                                      entity.value()) + " is inactive after discovering participants");
                }
            }
            for (auto entity : StatisticsBackend::get_entities(EntityKind::DATAREADER, monitor_id))
            {
                if (!StatisticsBackend::is_active(entity))
                {
                    throw Error("Error: DATAREADER with id: " + std::to_string(
                                      entity.value()) + " is inactive after discovering participants");
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Stop Monitor_" << seed << "\n" << e.what() << '\n';
            return 1;
        }
    }

    // Test: After undiscover participants and endpoints, all the entities except the monitor are inactive
    {
        // Wait until Undiscover participants and endpoints. The topic is not undiscovered by callback
        {
            std::unique_lock<std::mutex> lock(listener.mutex_);
            listener.cv_.wait(lock, [&]
                    {
                        return listener.num_entities_discovered_ == 0;
                    });
        }

        // Check that the database contains all entities related to subscriber and publisher entities
        // and check that all entities except the monitor are inactive
        try
        {
            if (!StatisticsBackend::get_entities(EntityKind::HOST).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::USER).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::PROCESS).empty())
            {
                throw Error("Error: database contains unexpected entities");
            }
            else if (StatisticsBackend::get_entities(EntityKind::DOMAIN).size() != 1 ||
                    StatisticsBackend::get_entities(EntityKind::DOMAIN).begin()->value() != monitor_id)
            {
                throw Error("Error: database contains unexpected DOMAIN");
            }
            else if (StatisticsBackend::get_entities(EntityKind::TOPIC).size() != num_topics ||
                    StatisticsBackend::get_entities(EntityKind::TOPIC, monitor_id).size() != num_topics)
            {
                throw Error("Error: database contains unexpected TOPIC");
            }
            else if (StatisticsBackend::get_entities(EntityKind::PARTICIPANT).size() != num_participants ||
                    StatisticsBackend::get_entities(EntityKind::PARTICIPANT, monitor_id).size() != num_participants)
            {
                throw Error("Error: database contains unexpected PARTICIPANT");
            }
            else if (StatisticsBackend::get_entities(EntityKind::DATAWRITER).size() != num_writers ||
                    StatisticsBackend::get_entities(EntityKind::DATAWRITER, monitor_id).size() != num_writers)
            {
                throw Error("Error: database contains unexpected DATAWRITER");
            }
            else if (StatisticsBackend::get_entities(EntityKind::DATAREADER).size() != num_readers ||
                    StatisticsBackend::get_entities(EntityKind::DATAREADER, monitor_id).size() != num_readers)
            {
                throw Error("Error: database contains unexpected DATAREADER");
            }
            else if (StatisticsBackend::get_entities(EntityKind::LOCATOR).size() != num_readers + num_writers ||
                    StatisticsBackend::get_entities(EntityKind::LOCATOR, monitor_id).size() != num_readers + num_writers)
            {
                throw Error("Error: database contains unexpected LOCATOR");
            }
            for (auto entity : StatisticsBackend::get_entities(EntityKind::DOMAIN, monitor_id))
            {
                if (!StatisticsBackend::is_active(entity))
                {
                    throw Error("Error: DOMAIN with id: " + std::to_string(
                                      entity.value()) + " is inactive after undiscovering participants");
                }
            }
            for (auto entity : StatisticsBackend::get_entities(EntityKind::TOPIC, monitor_id))
            {
                if (StatisticsBackend::is_active(entity))
                {
                    throw Error("Error: TOPIC with id: " + std::to_string(
                                      entity.value()) + " is active after undiscovering participants");
                }
            }
            for (auto entity : StatisticsBackend::get_entities(EntityKind::PARTICIPANT, monitor_id))
            {
                if (StatisticsBackend::is_active(entity))
                {
                    throw Error("Error: PARTICIPANT with id: " + std::to_string(
                                      entity.value()) + " is active after undiscovering participants");
                }
            }
            for (auto entity : StatisticsBackend::get_entities(EntityKind::DATAWRITER, monitor_id))
            {
                if (StatisticsBackend::is_active(entity))
                {
                    throw Error("Error: DATAWRITER with id: " + std::to_string(
                                      entity.value()) + " is active after undiscovering participants");
                }
            }
            for (auto entity : StatisticsBackend::get_entities(EntityKind::DATAREADER, monitor_id))
            {
                if (StatisticsBackend::is_active(entity))
                {
                    throw Error("Error: DATAREADER with id: " + std::to_string(
                                      entity.value()) + " is active after undiscovering participants");
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Stop Monitor_" << seed << "\n" << e.what() << '\n';
            return 1;
        }
    }

    // Test: Stopping the monitor deactivate all entities
    {
        // Stop the monitor
        try
        {
            StatisticsBackend::stop_monitor(monitor_id);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Stop Monitor_" << seed << "\n" << "Error stopping monitor" << e.what() << '\n';
            return 1;
        }

        // Check that the database contains all entities related to subscriber and publisher entities
        // and check that all entities are inactive
        try
        {
            if (!StatisticsBackend::get_entities(EntityKind::HOST).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::USER).empty() ||
                    !StatisticsBackend::get_entities(EntityKind::PROCESS).empty())
            {
                throw Error("Error: database contains unexpected entities");
            }
            else if (StatisticsBackend::get_entities(EntityKind::DOMAIN).size() != 1 ||
                    StatisticsBackend::get_entities(EntityKind::DOMAIN).begin()->value() != monitor_id)
            {
                throw Error("Error: database contains unexpected DOMAIN");
            }
            else if (StatisticsBackend::get_entities(EntityKind::TOPIC).size() != num_topics ||
                    StatisticsBackend::get_entities(EntityKind::TOPIC, monitor_id).size() != num_topics)
            {
                throw Error("Error: database contains unexpected TOPIC");
            }
            else if (StatisticsBackend::get_entities(EntityKind::PARTICIPANT).size() != num_participants ||
                    StatisticsBackend::get_entities(EntityKind::PARTICIPANT, monitor_id).size() != num_participants)
            {
                throw Error("Error: database contains unexpected PARTICIPANT");
            }
            else if (StatisticsBackend::get_entities(EntityKind::DATAWRITER).size() != num_writers ||
                    StatisticsBackend::get_entities(EntityKind::DATAWRITER, monitor_id).size() != num_writers)
            {
                throw Error("Error: database contains unexpected DATAWRITER");
            }
            else if (StatisticsBackend::get_entities(EntityKind::DATAREADER).size() != num_readers ||
                    StatisticsBackend::get_entities(EntityKind::DATAREADER, monitor_id).size() != num_readers)
            {
                throw Error("Error: database contains unexpected DATAREADER");
            }
            else if (StatisticsBackend::get_entities(EntityKind::LOCATOR).size() != num_readers + num_writers ||
                    StatisticsBackend::get_entities(EntityKind::LOCATOR, monitor_id).size() != num_readers + num_writers)
            {
                throw Error("Error: database contains unexpected LOCATOR");
            }
            for (auto entity : StatisticsBackend::get_entities(EntityKind::DOMAIN, monitor_id))
            {
                if (StatisticsBackend::is_active(entity))
                {
                    throw Error("Error: DOMAIN with id: " + std::to_string(
                                      entity.value()) + " is active after stopping monitor");
                }
            }
            for (auto entity : StatisticsBackend::get_entities(EntityKind::TOPIC, monitor_id))
            {
                if (StatisticsBackend::is_active(entity))
                {
                    throw Error("Error: TOPIC with id: " + std::to_string(
                                      entity.value()) + " is active after stopping monitor");
                }
            }
            for (auto entity : StatisticsBackend::get_entities(EntityKind::PARTICIPANT, monitor_id))
            {
                if (StatisticsBackend::is_active(entity))
                {
                    throw Error("Error: PARTICIPANT with id: " + std::to_string(
                                      entity.value()) + " is active after stopping monitor");
                }
            }
            for (auto entity : StatisticsBackend::get_entities(EntityKind::DATAWRITER, monitor_id))
            {
                if (StatisticsBackend::is_active(entity))
                {
                    throw Error("Error: DATAWRITER with id: " + std::to_string(
                                      entity.value()) + " is active after stopping monitor");
                }
            }
            for (auto entity : StatisticsBackend::get_entities(EntityKind::DATAREADER, monitor_id))
            {
                if (StatisticsBackend::is_active(entity))
                {
                    throw Error("Error: DATAREADER with id: " + std::to_string(
                                      entity.value()) + " is active after stopping monitor");
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Stop Monitor_" << seed << "\n" << e.what() << '\n';
            return 1;
        }
    }

    /* This can be activated once the clear_monitor is in place */
    // Test: Clearing the monitor empties the database except physical data
    // {
    //     try
    //     {
    //         StatisticsBackend::clear_monitor(monitor_id);

    //         // Check the database is empty
    //         if (!StatisticsBackend::get_entities(EntityKind::HOST).empty() ||
    //                 !StatisticsBackend::get_entities(EntityKind::USER).empty() ||
    //                 !StatisticsBackend::get_entities(EntityKind::PROCESS).empty() ||
    //                 !StatisticsBackend::get_entities(EntityKind::DOMAIN).empty() ||
    //                 !StatisticsBackend::get_entities(EntityKind::TOPIC).empty() ||
    //                 !StatisticsBackend::get_entities(EntityKind::PARTICIPANT).empty() ||
    //                 !StatisticsBackend::get_entities(EntityKind::DATAWRITER).empty() ||
    //                 !StatisticsBackend::get_entities(EntityKind::DATAREADER).empty())
    //         {
    //             throw Error("Error: database contains unexpected entities");
    //         }

    //         if (StatisticsBackend::get_entities(EntityKind::LOCATOR).size() != num_endpoints ||
    //                 StatisticsBackend::get_entities(EntityKind::LOCATOR, monitor_id).size() != num_endpoints)
    //         {
    //             throw Error("Error: database contains unexpected LOCATOR");
    //         }

    //     }
    //     catch (const std::exception& e)
    //     {
    //         std::cerr << "Stop Monitor_" << seed << "\n" << e.what() << '\n';
    //         return 1;
    //     }
    // }

    std::cerr << "Stop Monitor_" << seed << '\n';
    return 0;
}
