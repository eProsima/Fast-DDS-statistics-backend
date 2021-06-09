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
//

/**
 * @file database_queue.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_DATABASE_QUEUE_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_DATABASE_QUEUE_HPP_

#include <condition_variable>
#include <cstring>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include <fastdds/rtps/common/Guid.h>
#include <fastdds/rtps/common/Locator.h>
#include <fastdds/rtps/common/SequenceNumber.h>
#include <fastdds/dds/log/Log.hpp>

#include <database/database.hpp>
#include <database/entities.hpp>
#include <topic_types/types.h>
#include <exception/Exception.hpp>
#include <StatisticsBackend.hpp>
#include <StatisticsBackendData.hpp>


namespace eprosima {
namespace statistics_backend {
namespace database {

/**
 * Double buffered, threadsafe queue for MPSC (multi-producer, single-consumer) comms.
 */
template<typename T>
class DatabaseQueue
{

public:

    using StatisticsData = eprosima::fastdds::statistics::Data;
    using StatisticsEventKind = eprosima::fastdds::statistics::EventKind;

    using StatisticsWriterReaderData = eprosima::fastdds::statistics::WriterReaderData;
    using StatisticsLocator2LocatorData = eprosima::fastdds::statistics::Locator2LocatorData;
    using StatisticsEntityData = eprosima::fastdds::statistics::EntityData;
    using StatisticsEntity2LocatorTraffic = eprosima::fastdds::statistics::Entity2LocatorTraffic;
    using StatisticsEntityCount = eprosima::fastdds::statistics::EntityCount;
    using StatisticsDiscoveryTime = eprosima::fastdds::statistics::DiscoveryTime;
    using StatisticsSampleIdentityCount = eprosima::fastdds::statistics::SampleIdentityCount;
    using StatisticsPhysicalData = eprosima::fastdds::statistics::PhysicalData;
    using StatisticsEntityId = eprosima::fastdds::statistics::detail::EntityId_s;
    using StatisticsGuidPrefix = eprosima::fastdds::statistics::detail::GuidPrefix_s;
    using StatisticsGuid = eprosima::fastdds::statistics::detail::GUID_s;
    using StatisticsSequenceNumber = eprosima::fastdds::statistics::detail::SequenceNumber_s;
    using StatisticsSampleIdentity = eprosima::fastdds::statistics::detail::SampleIdentity_s;
    using StatisticsLocator = eprosima::fastdds::statistics::detail::Locator_s;

    using queue_item_type = std::pair<std::chrono::system_clock::time_point, T>;

    DatabaseQueue()
        : foreground_queue_(&queue_alpha_)
        , background_queue_(&queue_beta_)
        , consuming_(false)
        , current_loop_(0)
    {
        start_consumer();
    }

    // Specializations must stop the consumer in their destructor
    // to avoid calling the abstract process_sample once the child is destroyed
    virtual ~DatabaseQueue() = default;

    /**
     * @brief Pushes to the background queue.
     *
     * @param item Item to push into the queue
     */
    void push(
            std::chrono::system_clock::time_point ts,
            const T& item)
    {
        std::unique_lock<std::mutex> guard(cv_mutex_);
        std::unique_lock<std::mutex> bg_guard(background_mutex_);
        background_queue_->push(std::make_pair(ts, item));
        cv_.notify_all();
    }

    /**
     * @brief Consume all the available data in the queue
     *
     * \post both_empty() returns true
     */
    void flush()
    {
        std::unique_lock<std::mutex> guard(cv_mutex_);
        if (!consuming_ && !consumer_thread_)
        {
            // already empty
            return;
        }

        /* Flush() two steps strategy:
           First consume the foreground queue, then swap and consume the background one.
         */
        int last_loop = -1;
        for (int i = 0; i < 2; ++i)
        {
            cv_.wait(guard,
                    [&]()
                    {
                        /* I must avoid:
                         + the two calls be processed without an intermediate run() loop (by using last_loop sequence number)
                         + deadlock by absence of run() loop activity (by using both_empty() call)
                         */
                        return !consuming_ || both_empty() ||
                        ( empty() && last_loop != current_loop_);
                    });
            last_loop = current_loop_;
        }
    }

    /**
     * @brief Stops the consumer thread and wait for it to end
     *
     * @return true if the consumer has been stopped. False if it was already stopped
     *
     */
    bool stop_consumer()
    {
        if (consuming_)
        {
            std::unique_lock<std::mutex> guard(cv_mutex_);
            consuming_ = false;
            guard.unlock();
            cv_.notify_all();
            consumer_thread_->join();
            consumer_thread_.reset();
            return true;
        }

        return false;
    }

    /**
     * @brief Starts the consumer
     *
     * @return true if the consumer has been started. False if it was already started
     */
    bool start_consumer() noexcept
    {
        std::unique_lock<std::mutex> guard(cv_mutex_);
        if (!consuming_ && !consumer_thread_)
        {
            consuming_ = true;
            consumer_thread_.reset(new std::thread(&DatabaseQueue::run, this));
            return true;
        }

        return false;
    }

protected:

    /**
     * @brief Clears foreground queue and swaps queues.
     */
    void swap()
    {
        std::unique_lock<std::mutex> fg_guard(foreground_mutex_);
        std::unique_lock<std::mutex> bg_guard(background_mutex_);

        // Clear the foreground queue.
        std::queue<queue_item_type>().swap(*foreground_queue_);

        auto* swap        = background_queue_;
        background_queue_ = foreground_queue_;
        foreground_queue_ = swap;
    }

    /**
     * @brief Returns a reference to the front element in the foreground queue.
     *
     * \pre empty() is not False. Otherwise, the resulting behavior is undefined.
     *
     * @return A reference to the front element in the queue
     */
    queue_item_type& front()
    {
        std::unique_lock<std::mutex> fg_guard(foreground_mutex_);
        return foreground_queue_->front();
    }

    /**
     * @brief Returns a reference to the front element in the foreground queue
     *
     * \pre empty() is not False. Otherwise, the resulting behavior is undefined.
     *
     * @return A const reference to the front element in the queue
     */
    const queue_item_type& front() const
    {
        std::unique_lock<std::mutex> fg_guard(foreground_mutex_);
        return foreground_queue_->front();
    }

    /**
     * @brief Pops the front element in the foreground queue
     *
     * \pre empty() is not False. Otherwise, the resulting behavior is undefined.
     */
    void pop()
    {
        std::unique_lock<std::mutex> fg_guard(foreground_mutex_);
        foreground_queue_->pop();
    }

    /**
     * @brief Check whether the foreground queue is empty
     *
     * @return true if the queue is empty.
     */
    bool empty() const
    {
        std::unique_lock<std::mutex> fg_guard(foreground_mutex_);
        return foreground_queue_->empty();
    }

    /**
     * @brief Check whether both queues are empty
     *
     * @return true if both queues are empty.
     */
    bool both_empty() const
    {
        std::unique_lock<std::mutex> fg_guard(foreground_mutex_);
        std::unique_lock<std::mutex> bg_guard(background_mutex_);
        return foreground_queue_->empty() && background_queue_->empty();
    }

    /**
     * @brief Consuming thread
     */
    void run()
    {
        // Consume whatever there is in the queue
        // Needs lock acquired to sync with potential flush
        std::unique_lock<std::mutex> guard(cv_mutex_);
        consume_all(guard);

        while (consuming_)
        {
            cv_.wait(guard,
                    [&]()
                    {
                        return !consuming_ || !both_empty();
                    });

            if (!consuming_)
            {
                return;
            }

            consume_all(guard);
        }
    }

    /**
     * @brief Consume all the elements in the background queue
     *
     */
    void consume_all(
            std::unique_lock<std::mutex>& guard)
    {
        swap();
        while (!empty())
        {
            guard.unlock();
            process_sample();
            guard.lock();
            pop();
        }

        // We don't care about the overflow
        ++current_loop_;
        cv_.notify_all();
    }

    /**
     * @brief Processes the next sample in the foreground queue
     */
    virtual void process_sample() = 0;


    // Underlying queues
    std::queue<queue_item_type> queue_alpha_;
    std::queue<queue_item_type> queue_beta_;

    // Front and background queue references (double buffering)
    std::queue<queue_item_type>* foreground_queue_;
    std::queue<queue_item_type>* background_queue_;

    mutable std::mutex foreground_mutex_;
    mutable std::mutex background_mutex_;

    // Consumer
    std::unique_ptr<std::thread> consumer_thread_;
    std::condition_variable cv_;
    std::mutex cv_mutex_;
    bool consuming_;
    unsigned char current_loop_;
};

struct EntityDiscoveryInfo
{
    std::shared_ptr<Entity> entity;
    EntityId domain_id;
};

class DatabaseEntityQueue : public DatabaseQueue<EntityDiscoveryInfo>
{

public:

    DatabaseEntityQueue(
            database::Database* database)
        : DatabaseQueue<EntityDiscoveryInfo>()
        , database_(database)
    {
    }

    virtual ~DatabaseEntityQueue()
    {
        stop_consumer();
    }

protected:

    virtual void process_sample() override
    {
        try
        {
            auto id = database_->insert(front().second.entity);
            if (EntityKind::HOST  == front().second.entity->kind ||
                    EntityKind::USER == front().second.entity->kind ||
                    EntityKind::PROCESS == front().second.entity->kind ||
                    EntityKind::LOCATOR == front().second.entity->kind)
            {
                details::StatisticsBackendData::get_instance()->on_physical_entity_discovery(
                    front().second.domain_id, id,
                    front().second.entity->kind);
            }
            else if(EntityKind::DOMAIN != front().second.entity->kind)
            {
                details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(front().second.domain_id, id,
                        front().second.entity->kind, details::StatisticsBackendData::DISCOVERY);
            }
        }
        catch (const eprosima::statistics_backend::Exception& e)
        {
            logError(BACKEND_DATABASE_QUEUE, e.what());
        }
    }

    // Database
    Database* database_;

};

class DatabaseDataQueue : public DatabaseQueue<std::shared_ptr<eprosima::fastdds::statistics::Data>>
{

public:

    DatabaseDataQueue(
            database::Database* database)
        : DatabaseQueue<std::shared_ptr<eprosima::fastdds::statistics::Data>>()
        , database_(database)
    {
    }

    virtual ~DatabaseDataQueue()
    {
        stop_consumer();
    }

    virtual void process_sample() override;

    /**
     * @brief subroutine to build a StatisticsSample from a StatisticsData
     *
     * The consumer takes the @ref StatisticsData pushed to the queue and delegates to specializations of this subroutine
     * the task of creating the corresponding @ref StatisticsSample that will be added to the database.
     *
     * @tparam T The Sample type. It should be a type extending \ref StatisticsSample.
     * @tparam Q The type of the inner data contained in the \ref StatisticsData in the queue.
     *
     * @param[out] domain Buffer to receive the ID of the domain to which the \p entity belongs
     * @param[out] entity Buffer to receive the ID of the entity to which the sample refers
     * @param[in]  entity_kind The entity kind of the expected entity whose ID will be received in \p entity
     * @param[out] sample Buffer to receive the constructed sample
     * @param[in]  item The StatisticsData we want to process
     */
    template<typename T, typename Q>
    void process_sample_type(
            EntityId& domain,
            EntityId& entity,
            EntityKind entity_kind,
            T& sample,
            const Q& item) const
    {
        throw BadParameter("Unsupported Sample type and Data type combination");
    }

protected:

    std::string deserialize_guid(
            StatisticsGuid data) const
    {
        eprosima::fastrtps::rtps::GUID_t guid;
        memcpy(guid.guidPrefix.value, data.guidPrefix().value().data(), eprosima::fastrtps::rtps::GuidPrefix_t::size);
        memcpy(guid.entityId.value, data.entityId().value().data(), eprosima::fastrtps::rtps::EntityId_t::size);
        std::stringstream ss;
        ss << guid;
        return ss.str();
    }

    std::string deserialize_locator(
            StatisticsLocator data) const
    {
        int32_t kind = data.kind();
        uint32_t port = data.port();
        std::array<uint8_t, 16> address = data.address();

        eprosima::fastrtps::rtps::Locator_t locator(kind, port);
        memcpy(locator.address, address.data(), address.size());
        std::stringstream ss;
        ss << locator;
        return ss.str();
    }

    uint64_t deserialize_sequence_number(
            StatisticsSequenceNumber data) const
    {
        int32_t high = data.high();
        uint32_t low = data.low();

        return eprosima::fastrtps::rtps::SequenceNumber_t(high, low).to64long();
    }

    std::pair<std::string, uint64_t> deserialize_sample_identity(
            StatisticsSampleIdentity data) const
    {
        std::string writer_guid = deserialize_guid(data.writer_guid());
        uint64_t sequence_number = deserialize_sequence_number(data.sequence_number());

        return std::make_pair(writer_guid, sequence_number);
    }

    // Database
    Database* database_;

};

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        HistoryLatencySample& sample,
        const DatabaseQueue::StatisticsWriterReaderData& item) const;

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        NetworkLatencySample& sample,
        const StatisticsLocator2LocatorData& item) const;

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        EntityDataSample& sample,
        const StatisticsEntityData& item) const;

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        EntityToLocatorCountSample& sample,
        const StatisticsEntity2LocatorTraffic& item) const;

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        ByteToLocatorCountSample& sample,
        const StatisticsEntity2LocatorTraffic& item) const;


template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        EntityCountSample& sample,
        const StatisticsEntityCount& item) const;

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        DiscoveryTimeSample& sample,
        const StatisticsDiscoveryTime& item) const;

template<>
void DatabaseDataQueue::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        SampleDatasCountSample& sample,
        const StatisticsSampleIdentityCount& item) const;

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_DATABASE_QUEUE_HPP_
