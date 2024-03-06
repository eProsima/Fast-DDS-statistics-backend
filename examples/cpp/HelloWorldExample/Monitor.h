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
 * @file Monitor.h
 */

#ifndef _EPROSIMA_FASTDDSSTATISTICSBACKEND_EXAMPLES_CPP_HELLOWORLDEXAMPLE_MONITOR_H_
#define _EPROSIMA_FASTDDSSTATISTICSBACKEND_EXAMPLES_CPP_HELLOWORLDEXAMPLE_MONITOR_H_

#include <atomic>
#include <condition_variable>
#include <list>
#include <mutex>
#include <string>

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/listener/DomainListener.hpp>
#include <fastdds_statistics_backend/StatisticsBackend.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/types.hpp>
#include <fastdds_statistics_backend/types/JSONTags.h>

using namespace eprosima::statistics_backend;

class Monitor
{
public:

    Monitor();

    virtual ~Monitor();

    //! Initialize monitor
    bool init(
            uint32_t domain,
            uint16_t n_bins,
            uint32_t t_interval,
            std::string dump_file = "",
            bool reset = false);

    //! Run the monitor
    void run();

    //! Return the current state of execution
    static bool is_stopped();

    //! Trigger the end of execution
    static void stop();

    //! Get the Fast DDS latency mean of the last 10 seconds between the publisher and the subscriber
    std::vector<StatisticsData> get_fastdds_latency_mean();

    //! Get the publication throughput mean of the last 10 seconds of the publisher
    std::vector<StatisticsData> get_publication_throughput_mean();

    //! Serialize the timestamp of a given data value
    std::string timestamp_to_string(
            const Timestamp timestamp);

protected:

    /**
     * @brief Bump the internal database in file \c dump_file_ .
     *
     * If \c reset_ is active, it clears the statistical data.
     */
    void dump_in_file();

    /**
     * @brief Clear the inactive entities of the StatisticsBackend
     *
     */
    void clear_inactive_entities();

    class Listener : public eprosima::statistics_backend::PhysicalListener
    {
    public:

        Listener()
        {
        }

        ~Listener() override
        {
        }

        //! Callback when a new Host is discovered
        void on_host_discovery(
                EntityId host_id,
                const DomainListener::Status& status) override;

        //! Callback when a new User is discovered
        void on_user_discovery(
                EntityId user_id,
                const DomainListener::Status& status) override;

        //! Callback when a new Process is discovered
        void on_process_discovery(
                EntityId process_id,
                const DomainListener::Status& status) override;

        //! Callback when a new Topic is discovered
        void on_topic_discovery(
                EntityId domain_id,
                EntityId topic_id,
                const DomainListener::Status& status) override;

        //! Callback when a new Participant is discovered
        void on_participant_discovery(
                EntityId domain_id,
                EntityId participant_id,
                const DomainListener::Status& status) override;

        //! Callback when a new DataReader is discovered
        void on_datareader_discovery(
                EntityId domain_id,
                EntityId datareader_id,
                const DomainListener::Status& status) override;

        //! Callback when a new DataWriter is discovered
        void on_datawriter_discovery(
                EntityId domain_id,
                EntityId datawriter_id,
                const DomainListener::Status& status) override;

    }
    physical_listener_;

    EntityId monitor_id_;

private:

    //! Number of time intervals in which the measurement time is divided
    uint16_t n_bins_;

    //! Time interval of the returned measures
    uint32_t t_interval_;

    //! Path where the dump_file will be stored
    std::string dump_file_;

    //! Whether the internal data must be removed every interval
    bool reset_;

    //! Member used for control flow purposes
    static std::atomic<bool> stop_;

    //! Protects terminate condition variable
    static std::mutex terminate_cv_mtx_;

    //! Waits during execution until SIGINT is received
    static std::condition_variable terminate_cv_;
};

#endif /* _EPROSIMA_FASTDDSSTATISTICSBACKEND_EXAMPLES_CPP_HELLOWORLDEXAMPLE_MONITOR_H_ */
