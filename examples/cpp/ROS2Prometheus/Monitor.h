// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef MONITOR_H_
#define MONITOR_H_

#include <atomic>
#include <condition_variable>
#include <list>
#include <string>

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/listener/DomainListener.hpp>
#include <fastdds_statistics_backend/StatisticsBackend.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/types.hpp>

#include "prometheus/client_metric.h"
#include "prometheus/counter.h"
#include "prometheus/exposer.h"
#include "prometheus/family.h"
#include "prometheus/registry.h"

using namespace eprosima::statistics_backend;
using namespace eprosima::fastdds::dds;

class Monitor
{
public:
    Monitor(
            uint32_t domain,
            uint32_t n_bins,
            uint32_t t_interval,
            std::string exposer_addr = "127.0.0.1:8080");

    virtual ~Monitor();

    //! Initialize monitor
    bool init();

    //! Run the monitor
    void run();

    //! Stop the monitor
    static void stop();

    //! Get the Fast DDS latency mean of the last t_interval seconds between the talker and the listener
    std::vector<StatisticsData> get_fastdds_latency_mean();

    //! Get the publication throughput mean of the last t_interval seconds of the talker
    std::vector<StatisticsData> get_publication_throughput_mean();

    //! Serialize the timestamp of a given data value
    std::string timestamp_to_string(
        const Timestamp timestamp);

protected:

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

        //! Callback when a new Locator is discovered
        void on_locator_discovery(
                EntityId locator_id,
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

    } physical_listener_;

    //! DDS Domain Id to monitor
    DomainId domain_;

    //! Number of time intervals in which the measurement time is divided
    uint32_t n_bins_;

    //! Time interval of the returned measures
    uint32_t t_interval_;

    //! EntityId of the initialized monitor
    EntityId monitor_id_;

    //! Signals that the monitor has been stopped
    static std::atomic<bool> stop_;

    //! Protects terminate condition variable
    static std::mutex terminate_cv_mtx_;

    //! Waits during execution until SIGINT is received
    static std::condition_variable terminate_cv_;


    prometheus::Exposer exposer_;
    std::shared_ptr<prometheus::Registry> registry_;
    prometheus::Gauge* fastdds_latency_mean_;
    prometheus::Gauge* publication_throughput_mean_;
};

#endif /* MONITOR_H_ */
