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
#include <condition_variable>
#include <mutex>
#include <stdlib.h>
#include <thread>
#include <memory>

#include "Database.hpp"
#include "RandomGenerator.hpp"

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

void Database::listener(PhysicalListener* listener)
{
    const std::lock_guard<std::recursive_mutex> lock(data_mutex_);
    listener_ = listener;
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
    const std::lock_guard<std::recursive_mutex> lock(data_mutex_);
    return ++last_id_;
}

std::map<EntityId, EntityPointer>& Database::get_all_entities()
{
    const std::lock_guard<std::recursive_mutex> lock(data_mutex_);
    return entities_;
}

std::vector<EntityId> Database::get_entities(
        EntityKind entity_type,
        EntityId entity_id)
{
    if (entity_id == EntityId::all())
    {
        // Ask for all entities of a kind
        std::vector<EntityId> result;
        for (auto entity : entities_)
        {
            if (entity.second->kind() == entity_type)
            {
                result.push_back(entity.first);
            }
        }
        return result;
    }
    else
    {
        // Ask for entities of a kind from a specific entity
        return entities_[entity_id]->get_entities(entity_type);
    }
}

Info Database::get_info(EntityId entity_id)
{
    return entities_[entity_id]->get_info();
}

EntityId Database::add_domain()
{
    // Generates the Domain entity
    DomainPointer domain = RandomGenerator::random_domain();

    // Add the Entity to the entities map
    add_entity(domain);

    // Add domain in domains list
    {
        const std::lock_guard<std::recursive_mutex> lock(data_mutex_);
        domains_.push_back(domain->id());
    }

    // Generates random Entities to fill this domain
    add_entities(RandomGenerator::init_random_domain(domain));

    start();

    return domain->id();
}

void Database::add_entity(EntityPointer entity)
{
    entities_[entity->id()] = entity;
}

void Database::add_entities(std::vector<EntityPointer> entities)
{
    for (EntityPointer entity : entities)
    {
        add_entity(entity);
    }
}

size_t Database::count_domains()
{
    const std::lock_guard<std::recursive_mutex> lock(data_mutex_);
    return domains_.size();
}

void Database::generate_random_data_thread()
{
    // Wait till is ready to generate data
    std::unique_lock<std::mutex> lock(run_mutex_);
    cv_.wait(lock, [this]{return run_.load();});

    int entity_count = 0;

    // Start the loop while it does not stop
    while(run_.load())
    {
        // Sleep a time depending the number of domains we have
        uint32_t sleep_seconds = (count_domains() <= 1) ? DATA_GENERATION_TIME : DATA_GENERATION_TIME / count_domains();
        std::this_thread::sleep_for(std::chrono::seconds(sleep_seconds));

        // Exit with stop if it has activated while sleeping
        if (run_.load())
        {
            break;
        }

        // Each time a new domain will create an entity
        // Beware that when new domains are created, this would not be equitative in first n (# domains) iterations
        uint32_t domain_index = entity_count % count_domains();

        // Generates a new dds Entity
        DomainPointer domain = std::dynamic_pointer_cast<Domain>(entities_[domains_[domain_index]]);
        add_entities(RandomGenerator::random_dds_entity(domain));
    }
}

} // namespace statistics_backend
} // namespace eprosima
