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
 * @file Database.hpp
 */

#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>

#include <fastdds-statistics-backend/listener/PhysicalListener.hpp>
#include <fastdds-statistics-backend/types/types.hpp>

#include "headers/EntityPointer.hpp"

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_COMPLEXMOCK_ENTITYCOLLECTOR_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_COMPLEXMOCK_ENTITYCOLLECTOR_HPP_

namespace eprosima {
namespace statistics_backend {

#define DATA_GENERATION_TIME 10

class Database
{
public:

    static Database* get_instance()
    {
        static Database instance;
        return &instance;
    }

    void start();
    void stop();

protected:

    Database();

    ~Database();

    int64_t next_id();

    void add_domain(EntityId domain);

    void static generate_random_data();

    void generate_random_data_thread();

private:

    std::vector<EntityId> domains_;
    std::map<EntityId, EntityPointer> entities_;
    int64_t last_id_;

    PhysicalListener* listener_;

    std::thread generate_data_thread_;
    std::mutex data_mutex_;
    std::mutex run_mutex_;
    std::atomic<bool> run_;
    std::condition_variable cv_;
};

} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_COMPLEXMOCK_ENTITYCOLLECTOR_HPP_
