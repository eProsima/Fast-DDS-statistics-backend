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
 * @file Subscriber.hpp
 *
 */

#ifndef FASTDDS_STATISTICS_BACKEND_EXAMPLES_CPP_HELLOWORLD_EXAMPLE__HELLO_WORLD_SUBSCRIBER_H
#define FASTDDS_STATISTICS_BACKEND_EXAMPLES_CPP_HELLOWORLD_EXAMPLE__HELLO_WORLD_SUBSCRIBER_H

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <fastdds/dds/core/status/SubscriptionMatchedStatus.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>

#include "Communication.hpp"

/**
 * Class used to group into a single working unit a Subscriber with a DataReader, its listener, and a TypeSupport member
 * corresponding to the HelloWorld datatype
 */
class Subscriber
{
public:

    Subscriber();

    virtual ~Subscriber();

    //! Initialize the subscriber
    bool init(
            uint32_t max_messages,
            uint32_t domain);

    //! RUN the subscriber until number samples are received
    void run(
            uint32_t number);

    //! Return the current state of execution
    static bool is_stopped();

    //! Trigger the end of execution
    static void stop();
    class SubListener : public eprosima::fastdds::dds::DataReaderListener
    {
    public:

        SubListener()
            : matched_(0)
            , samples_(0)
            , max_messages_(0)
        {
        }

        ~SubListener() override
        {
        }

        //! Set the maximum number of messages to receive before exiting
        void set_max_messages(
                uint32_t max_messages);

        //! Callback executed when a new sample is received
        void on_data_available(
                eprosima::fastdds::dds::DataReader* reader) override;

        //! Callback executed when a DataWriter is matched or unmatched
        void on_subscription_matched(
                eprosima::fastdds::dds::DataReader* reader,
                const eprosima::fastdds::dds::SubscriptionMatchedStatus& info) override;
                
        uint32_t get_matched()
        {
            return matched_;
        }
    private:

        Communication comm_;

        //! Number of DataWriters matched to the associated DataReader
        int matched_;

        //! Number of samples received
        uint32_t samples_;

        //! Number of messages to be received before triggering termination of execution
        uint32_t max_messages_;

        //! Avoids race conditions in callback execution
        std::mutex mutex_;
        std::condition_variable cv_;
    } listener;

private:

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Subscriber* subscriber_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataReader* reader_;

    eprosima::fastdds::dds::TypeSupport type_;

    /**
     * Class handling discovery and dataflow events
     */

    //! Member used for control flow purposes
    static std::atomic<bool> stop_;

    //! Protects terminate condition variable
    static std::mutex terminate_cv_mtx_;

    //! Waits during execution until SIGINT or max_messages_ samples are received
    static std::condition_variable terminate_cv_;
};

#endif /* FASTDDS_STATISTICS_BACKEND_EXAMPLES_CPP_HELLOWORLD_EXAMPLE__HELLO_WORLD_SUBSCRIBER_H */