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
 * @file MonitorDynamic.cpp
 */

#include <list>
#include <string>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

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

/* ARGUMENTS
 */

int main(
        int argc,
        char** argv)
{
    int arg_count = 1;
    uint32_t seed = 7800;

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

    std::cout << details::StatisticsBackendData::get_instance()->database_->dump_database() << std::endl;

    // Init the monitor
    DomainId domain_id = seed % 230;
    EntityId monitor_id = StatisticsBackend::init_monitor(domain_id);
    if (!monitor_id.is_valid())
    {
        std::cout << "Error creating monitor" << std::endl;
        return 1;
    }

    while (true)
    {
        std::cout << std::endl;
        std::cout << details::StatisticsBackendData::get_instance()->database_->dump_database() << std::endl;
        std::cout << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    // Stop the monitor
    try
    {
        StatisticsBackend::stop_monitor(monitor_id);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error stopping monitor" << e.what() << '\n';
    }

    // Reset the singleton instead of removing the monitor
    details::StatisticsBackendData::reset_instance();

    return 0;
}
