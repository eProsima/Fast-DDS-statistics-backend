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

#include <database/database.hpp>
#include <database/entities.hpp>
#include <topic_types/types.h>
#include <exception/Exception.hpp>


namespace eprosima {
namespace statistics_backend {
namespace database {


/**
 * Double buffered, threadsafe queue for MPSC (multi-producer, single-consumer) comms.
 */
template<typename T>
class DatabaseQueue {

public:

    using queue_item_type = std::pair<std::chrono::steady_clock::time_point, T>;

   DatabaseQueue(
            Database* database)
        : foreground_queue_(&queue_alpha_)
        , background_queue_(&queue_beta_)
        , database_(database)
        , consuming_(false)
        , current_loop_(0)
    {
    }

   virtual ~DatabaseQueue()
   {
   }

    /**
     * @brief Pushes to the background queue.
     * 
     * @param item Item to push into the queue
     */
    void push(std::chrono::steady_clock::time_point ts, const T& item)
    {
    }

    /**
     * @brief Consume all the available data in the queue
     * 
     * \post both_empty() returns true
     */
   void flush()
   {
   }

    /**
     * @brief Stops the consumer thread and wait for it to end
     * 
     * @return true if the consumer has been stopped. False if it was already stopped 
     * 
     */
   bool stop_consumer()
   {
        return true;
   }

    /**
     * @brief Starts the consumer
     * 
     * @return true if the consumer has been started. False if it was already started 
     */
   bool start_consumer() noexcept
   {
        return true;
   }

protected:

   /**
    * @brief Clears foreground queue and swaps queues.
    */
   void swap()
   {
   }

   /**
    * @brief Returns a reference to the front element in the foreground queue.
    * 
    * \pre Empty() is not False. Otherwise, the resulting behavior is undefined.
    * 
    * @return A reference to the front element in the queue
    */
   queue_item_type& front()
   {
      return queue_item_type();
   }

   /**
    * @brief Returns a reference to the front element in the foreground queue
    * 
    * \pre Empty() is not False. Otherwise, the resulting behavior is undefined.
    *
    * @return A const reference to the front element in the queue
    */
   const queue_item_type& front() const
   {
      return queue_item_type();
   }

   /**
    * @brief Pops the front element in the foreground queue
    * 
    * \pre Empty() is not False. Otherwise, the resulting behavior is undefined.
    */
   void pop()
   {
   }

   /**
    * @brief Check whether the foreground queue is empty
    * 
    * @return true if the queue is empty.
    */
   bool empty() const
   {
       true;
   }

   /**
    * @brief Check whether both queues are empty
    * 
    * @return true if both queues are empty.
    */
   bool both_empty() const
   {
       true;
   }

    /**
     * @brief Consuming thread
     */
    void run()
    {
    }

    /**
     * @brief Consume all that there is in the background queue
     * 
     */
    void consume_all()
    {
    }

    virtual void process_sample() = 0;

};

class DatabaseEntityQueue : public DatabaseQueue<std::shared_ptr<Entity>>
{

public:

   DatabaseEntityQueue(
            database::Database* database)
    : DatabaseQueue<std::shared_ptr<Entity>>(database)
   {
   }

    virtual ~DatabaseEntityQueue()
    {
    }

protected:

    virtual void process_sample() override
    {
    }

};

class DatabaseDataQueue : public DatabaseQueue<std::shared_ptr<eprosima::fastdds::statistics::Data>>
{

public:

   DatabaseDataQueue(
            database::Database* database)
    : DatabaseQueue<std::shared_ptr<eprosima::fastdds::statistics::Data>>(database)
   {
   }

    virtual ~DatabaseDataQueue()
    {
    }

    virtual void process_sample() override;

};

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_DATABASE_QUEUE_HPP_
