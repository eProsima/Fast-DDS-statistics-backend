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

#ifndef MONITOR_H_
#define MONITOR_H_

#include <list>
#include <string>

#include <fastdds_statistics_backend/exception/Exception.hpp>
#include <fastdds_statistics_backend/listener/DomainListener.hpp>
#include <fastdds_statistics_backend/StatisticsBackend.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/types.hpp>

using namespace eprosima::statistics_backend;

class Monitor
{
public:

    Monitor();

    virtual ~Monitor();

    //! Initialize monitor
    bool init(
            uint32_t n_bins,
            uint32_t t_interval);

    //! Run the monitor
    void run();

    //! Get the Fast DDS latency mean of the last 10 seconds between the publisher and the subscriber
    std::vector<StatisticsData> get_fastdds_latency_mean();

    //! Get the publication throughput mean of the last 10 seconds of the publisher
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

    }
    physical_listener_;

    EntityId monitor_id_;

private:

    uint32_t n_bins_;

    uint32_t t_interval_;
};

#endif /* MONITOR_H_ */
