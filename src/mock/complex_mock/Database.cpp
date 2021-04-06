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
 * @file Database.cpp
 */

#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "Database.hpp"

namespace eprosima {
namespace statistics_backend {

void Database::start()
{
    run_.store(true);
    cv_.notify_all();
}

void Database::stop()
{
    run_.store(false);
}

Database::Database()
    : last_id_(0)
    , listener_(nullptr)
    , run_(false)
{
    generate_data_thread_ = std::thread(&Database::generate_random_data_thread, this);
}

Database::~Database()
{
    stop();

    generate_data_thread_.join();

    if (listener_)
    {
        delete listener_;
    }
}

int64_t Database::next_id()
{
    const std::lock_guard<std::mutex> lock(data_mutex_);
    return ++last_id_;
}

void Database::add_domain(EntityId domain)
{
    const std::lock_guard<std::mutex> lock(data_mutex_);
    domains_.push_back(domain);
    start();
}

void Database::generate_random_data()
{

}

void Database::generate_random_data()
{

}

void Database::generate_random_data_thread()
{
    // Wait till is ready to generate data
    std::unique_lock<std::mutex> lock(run_mutex_);
    cv_.wait(lock, [this]{return run_.load();});

    //

}

} // namespace statistics_backend
} // namespace eprosima
