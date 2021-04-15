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
        start_consumer();
    }

   virtual ~DatabaseQueue()
   {
       // Especializations must stop the consumer in their destructor,
       // to avoid calling the abstract process_sample once the child is destroyed
   }

    /**
     * @brief Pushes to the background queue.
     * 
     * @param item Item to push into the queue
     */
    void push(std::chrono::steady_clock::time_point ts, const T& item)
    {
        std::unique_lock<std::mutex> guard(background_mutex_);
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
                            + the two calls be processed without an intermediate Run() loop (by using last_loop sequence number)
                            + deadlock by absence of Run() loop activity (by using BothEmpty() call)
                            */
                            return !consuming_ ||
                            ( empty() &&
                            ( last_loop != current_loop_ || both_empty()) );
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

      auto* swap       = background_queue_;
      background_queue_ = foreground_queue_;
      foreground_queue_ = swap;
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
      std::unique_lock<std::mutex> guard(foreground_mutex_);
      return foreground_queue_->front();
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
      std::unique_lock<std::mutex> guard(foreground_mutex_);
      return foreground_queue_->front();
   }

   /**
    * @brief Pops the front element in the foreground queue
    * 
    * \pre Empty() is not False. Otherwise, the resulting behavior is undefined.
    */
   void pop()
   {
      std::unique_lock<std::mutex> guard(foreground_mutex_);
      foreground_queue_->pop();
   }

   /**
    * @brief Check whether the foreground queue is empty
    * 
    * @return true if the queue is empty.
    */
   bool empty() const
   {
      std::unique_lock<std::mutex> guard(foreground_mutex_);
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
        consume_all();

        std::unique_lock<std::mutex> guard(cv_mutex_);
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

            guard.unlock();
            consume_all();
            guard.lock();
        }
    }

    /**
     * @brief Consume all that there is in the background queue
     * 
     */
    void consume_all()
    {
        swap();
        while (!empty())
        {
            process_sample();
            pop();
        }

        // We don't care about the overflow
        ++current_loop_;

        cv_.notify_all();
    }

    virtual void process_sample() = 0;


   // Underlying queues
   std::queue<queue_item_type> queue_alpha_;
   std::queue<queue_item_type> queue_beta_;

   // Front and background queue references (double buffering)
   std::queue<queue_item_type>* foreground_queue_;
   std::queue<queue_item_type>* background_queue_;

   mutable std::mutex foreground_mutex_;
   mutable std::mutex background_mutex_;

    // Database
    Database* database_;

    // Consumer
    std::unique_ptr<std::thread> consumer_thread_;
    std::condition_variable cv_;
    std::mutex cv_mutex_;
    bool consuming_;
    unsigned char current_loop_;
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
        stop_consumer();
    }

protected:

    virtual void process_sample() override
    {
        database_->insert(front().second);
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
        stop_consumer();
    }

    virtual void process_sample() override;

};

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_DATABASE_QUEUE_HPP_
