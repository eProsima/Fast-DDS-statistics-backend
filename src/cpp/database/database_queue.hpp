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

#ifndef FASTDDS_STATISTICS_BACKEND_SRC_CPP_DATABASE__DATABASE_QUEUE_HPP
#define FASTDDS_STATISTICS_BACKEND_SRC_CPP_DATABASE__DATABASE_QUEUE_HPP

#include <condition_variable>
#include <cstring>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

#include <fastdds/rtps/common/Guid.hpp>
#include <fastdds/rtps/common/Locator.hpp>
#include <fastdds/rtps/common/SequenceNumber.hpp>
#include <fastdds/rtps/common/RemoteLocators.hpp>
#include <fastdds/dds/log/Log.hpp>

#include <fastdds_statistics_backend/types/JSONTags.h>

#include <database/database.hpp>
#include <database/entities.hpp>
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

    using StatisticsWriterReaderData = eprosima::fastdds::statistics::WriterReaderData;
    using StatisticsLocator2LocatorData = eprosima::fastdds::statistics::Locator2LocatorData;
    using StatisticsEntityData = eprosima::fastdds::statistics::EntityData;
    using StatisticsEntity2LocatorTraffic = eprosima::fastdds::statistics::Entity2LocatorTraffic;
    using StatisticsEntityCount = eprosima::fastdds::statistics::EntityCount;
    using StatisticsDiscoveryTime = eprosima::fastdds::statistics::DiscoveryTime;
    using StatisticsSampleIdentityCount = eprosima::fastdds::statistics::SampleIdentityCount;
    using StatisticsPhysicalData = eprosima::fastdds::statistics::PhysicalData;
    using StatisticsIncompatibleQoSStatus = eprosima::fastdds::statistics::IncompatibleQoSStatus_s;
    using StatisticsInconsistentTopicStatus = eprosima::fastdds::statistics::InconsistentTopicStatus_s;
    using StatisticsConnection = eprosima::fastdds::statistics::Connection;
    using StatisticsLivelinessLostStatus = eprosima::fastdds::statistics::LivelinessLostStatus_s;
    using StatisticsLivelinessChangedStatus = eprosima::fastdds::statistics::LivelinessChangedStatus_s;
    using StatisticsDeadlineMissedStatus = eprosima::fastdds::statistics::DeadlineMissedStatus_s;
    using StatisticsSampleLostStatus = eprosima::fastdds::statistics::SampleLostStatus_s;
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
        // WORKAROUND: There was an error when calling stop_consumer from 2 threads at the same time that could lead
        // to a segfault, as both try to join the thread, and one destroys the thread while the other is waiting.
        // There exist a second problem: start_consumer could be called AFTER stop_consumer set consuming_ to false
        // but BEFORE stop_consumer actually join and destroy the thread, what leads to an orphan thread
        // wandering in neverland limbo
        // SOLUTION: using a tmp variable to store the thread that must be joined and destroyed,
        // so after setting consuming_ to false, the thread still exists but it is not destroyed, and the internal
        // object thread has been reset so it could be set again from start_consumer
        std::unique_ptr<std::thread> tmp_thread_;

        {
            std::unique_lock<std::mutex> guard(cv_mutex_);
            if (consuming_)
            {
                consuming_ = false;
                tmp_thread_ = std::move(consumer_thread_);
                consumer_thread_.reset(); // Redundant call, as moving it already releases it (speak with richiware)
            }
            else
            {
                return false;
            }
        }

        // At this point, we are sure the thread was consuming and it has been stopped
        cv_.notify_all();

        // Wait for thread to finish, join it and destroy it (this is done without mutex taken or deadlock will occur)
        tmp_thread_->join();

        return true;
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
            // This should be refactor to create less threads. Task associated: #14556
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
    details::StatisticsBackendData::DiscoveryStatus discovery_status;
    EntityId domain_id;

    fastdds::rtps::GUID_t guid;
    database::Qos qos;

    // Participant data
    std::string address;
    std::string participant_name;
    AppId app_id;
    std::string app_metadata;

    //Physical data
    std::string host;
    std::string user;
    std::string process;

    // Enpoint data
    std::string topic_name;
    std::string type_name;
    fastdds::rtps::RemoteLocatorList locators;

    // Alias
    std::string alias;
    bool is_virtual_metatraffic = false;

    // Status
    StatusLevel entity_status;

    EntityDiscoveryInfo(
            EntityKind kind)
        : entity_kind(kind)
    {
    }

    EntityKind kind() const
    {
        return entity_kind;
    }

protected:

    EntityKind entity_kind;

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

    EntityId process_participant(
            const EntityDiscoveryInfo& info);

    EntityId process_datareader(
            const EntityDiscoveryInfo& info);

    EntityId process_datawriter(
            const EntityDiscoveryInfo& info);

    virtual void process_sample() override
    {
        try
        {
            const EntityDiscoveryInfo& info = front().second;
            assert (info.kind() == EntityKind::PARTICIPANT ||
                    info.kind() == EntityKind::DATAREADER ||
                    info.kind() == EntityKind::DATAWRITER);

            EntityId entity_id;
            switch (info.kind())
            {
                case EntityKind::PARTICIPANT:
                    entity_id = process_participant(info);
                    break;
                case EntityKind::DATAREADER:
                    entity_id = process_datareader(info);
                    break;
                case EntityKind::DATAWRITER:
                    entity_id = process_datawriter(info);
                    break;
                default:
                    break;
                    // Already asserted
            }

            details::StatisticsBackendData::get_instance()->on_domain_entity_discovery(
                info.domain_id,
                entity_id,
                info.kind(),
                info.discovery_status);
        }
        catch (const eprosima::statistics_backend::Exception& e)
        {
            EPROSIMA_LOG_ERROR(BACKEND_DATABASE_QUEUE, e.what());
        }
    }

    template<typename T>
    EntityId process_endpoint_discovery(
            const T& info);

    // Database
    Database* database_;

};

template <typename T>
class DatabaseDataQueue : public DatabaseQueue<std::shared_ptr<T>>
{

public:

    DatabaseDataQueue(
            database::Database* database)
        : DatabaseQueue<std::shared_ptr<T>>()
        , database_(database)
    {
    }

    using StatisticsGuid = eprosima::fastdds::statistics::detail::GUID_s;
    using StatisticsLocator = eprosima::fastdds::statistics::detail::Locator_s;
    using StatisticsSequenceNumber = eprosima::fastdds::statistics::detail::SequenceNumber_s;
    using StatisticsSampleIdentity = eprosima::fastdds::statistics::detail::SampleIdentity_s;

    using queue_item_type = std::pair<std::chrono::system_clock::time_point, std::shared_ptr<T>>;

    virtual ~DatabaseDataQueue()
    {
        DatabaseQueue<std::shared_ptr<T>>::stop_consumer();
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
    template<typename Q, typename R>
    void process_sample_type(
            EntityId& domain,
            EntityId& entity,
            EntityKind entity_kind,
            Q& sample,
            const R& item) const
    {
        static_cast<void>(domain);
        static_cast<void>(entity);
        static_cast<void>(entity_kind);
        static_cast<void>(sample);
        static_cast<void>(item);

        throw BadParameter("Unsupported Sample type and Data type combination");
    }

    /**
     * @brief subroutine to build a MonitorServiceSample from a MonitorServiceData
     *
     * The consumer takes the @ref MonitorServiceData pushed to the queue and delegates to specializations of this subroutine
     * the task of creating the corresponding @ref MonitorServiceSample that will be added to the database.
     *
     * @tparam Q The Sample type. It should be a type extending \ref MonitorServiceSample.
     * @tparam R The type of the inner data contained in the \ref MonitorServiceData in the queue.
     *
     * @param[out] domain Buffer to receive the ID of the domain to which the \p entity belongs
     * @param[out] entity Buffer to receive the ID of the entity to which the sample refers
     * @param[in]  local_entity_guid The GUID of the entity reporting status data
     * @param[out] sample Buffer to receive the constructed sample
     * @param[in]  item The MonitorServiceData we want to process
     */
    template<typename Q, typename R>
    void process_sample_type(
            EntityId& domain,
            EntityId& entity,
            const StatisticsGuid& local_entity_guid,
            Q& sample,
            const R& item) const
    {
        static_cast<void>(domain);
        static_cast<void>(entity);
        static_cast<void>(local_entity_guid);
        static_cast<void>(sample);
        static_cast<void>(item);

        throw BadParameter("Unsupported Sample type and MonitorServiceStatus type combination");
    }

protected:

    std::string deserialize_guid(
            StatisticsGuid data) const
    {
        eprosima::fastdds::rtps::GUID_t guid;
        memcpy(guid.guidPrefix.value, data.guidPrefix().value().data(), eprosima::fastdds::rtps::GuidPrefix_t::size);
        memcpy(guid.entityId.value, data.entityId().value().data(), eprosima::fastdds::rtps::EntityId_t::size);
        std::stringstream ss;
        ss << guid;
        return ss.str();
    }

    std::string deserialize_guid(
            StatisticsLocator data) const
    {
        if (data.port() != 0)
        {
            throw Error("Wrong format: src_locator.port must be 0");
        }
        eprosima::fastdds::rtps::GUID_t guid;
        memcpy(guid.guidPrefix.value, data.address().data(), eprosima::fastdds::rtps::GuidPrefix_t::size);
        memcpy(guid.entityId.value,
                data.address().data() + eprosima::fastdds::rtps::GuidPrefix_t::size,
                eprosima::fastdds::rtps::EntityId_t::size);
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

        eprosima::fastdds::rtps::Locator_t locator(kind, port);
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

        return eprosima::fastdds::rtps::SequenceNumber_t(high, low).to64long();
    }

    std::pair<std::string, uint64_t> deserialize_sample_identity(
            StatisticsSampleIdentity data) const
    {
        std::string writer_guid = deserialize_guid(data.writer_guid());
        uint64_t sequence_number = deserialize_sequence_number(data.sequence_number());

        return std::make_pair(writer_guid, sequence_number);
    }

    EntityId get_or_create_locator(
            const std::string& locator_name) const;

    // Database
    Database* database_;

};

template<>
template<typename Q, typename R>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        Q& sample,
        const R& item) const;

template<>
template<typename Q, typename R>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        Q& sample,
        const R& item) const;

template<>
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        HistoryLatencySample& sample,
        const DatabaseQueue::StatisticsWriterReaderData& item) const;

template<>
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        NetworkLatencySample& sample,
        const StatisticsLocator2LocatorData& item) const;

template<>
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        EntityDataSample& sample,
        const StatisticsEntityData& item) const;

template<>
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        EntityToLocatorCountSample& sample,
        const StatisticsEntity2LocatorTraffic& item) const;

template<>
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        ByteToLocatorCountSample& sample,
        const StatisticsEntity2LocatorTraffic& item) const;


template<>
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        EntityCountSample& sample,
        const StatisticsEntityCount& item) const;

template<>
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        DiscoveryTimeSample& sample,
        const StatisticsDiscoveryTime& item) const;

template<>
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::Data>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        SampleDatasCountSample& sample,
        const StatisticsSampleIdentityCount& item) const;

template<>
template<typename Q, typename R>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        EntityKind entity_kind,
        Q& sample,
        const R& item) const;

template<>
template<typename Q, typename R>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        Q& sample,
        const R& item) const;

template<>
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        IncompatibleQosSample& sample,
        const DatabaseQueue::StatisticsIncompatibleQoSStatus& item) const;

template<>
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        InconsistentTopicSample& sample,
        const DatabaseQueue::StatisticsInconsistentTopicStatus& item) const;

template<>
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        ProxySample& sample,
        const std::vector<uint8_t>& item) const;

template<>
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        ConnectionListSample& sample,
        const std::vector<StatisticsConnection>& item) const;

template<>
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        LivelinessLostSample& sample,
        const StatisticsLivelinessLostStatus& item) const;

template<>
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        LivelinessChangedSample& sample,
        const StatisticsLivelinessChangedStatus& item) const;

template<>
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        DeadlineMissedSample& sample,
        const StatisticsDeadlineMissedStatus& item) const;

template<>
template<>
void DatabaseDataQueue<eprosima::fastdds::statistics::MonitorServiceStatusData>::process_sample_type(
        EntityId& domain,
        EntityId& entity,
        const StatisticsGuid& local_entity_guid,
        SampleLostSample& sample,
        const StatisticsSampleLostStatus& item) const;

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif //FASTDDS_STATISTICS_BACKEND_SRC_CPP_DATABASE__DATABASE_QUEUE_HPP
