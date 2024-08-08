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
 * @file Publisher.hpp
 *
 */

#ifndef FASTDDS_STATISTICS_BACKEND_TEST_DDS_COMMUNICATION__PUBLISHER_HPP
#define FASTDDS_STATISTICS_BACKEND_TEST_DDS_COMMUNICATION__PUBLISHER_HPP

#include <atomic>
#include <condition_variable>
#include <mutex>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include "Communication.hpp"

/**
 * Class used to group into a single working unit a Publisher with a DataWriter, its listener, and a TypeSupport member
 * corresponding to the datatype
 */
class Publisher
{
public:

    Publisher();

    virtual ~Publisher();

    //! Initialize the publisher
    bool init(
            uint32_t domain);

    //! Publish a sample
    void publish();

    //! Run for number samples, publish every sleep seconds
    void run(
            uint32_t number,
            uint32_t sleep);

    //! Return the current state of execution
    static bool is_stopped();

    //! Trigger the end of execution
    static void stop();

    class PubListener : public eprosima::fastdds::dds::DataWriterListener
    {
    public:

        PubListener()
            : matched_(0)
        {
        }

        ~PubListener() override
        {
        }

        //! Callback executed when a DataReader is matched or unmatched
        void on_publication_matched(
                eprosima::fastdds::dds::DataWriter* writer,
                const eprosima::fastdds::dds::PublicationMatchedStatus& info) override;

        //! Getter function for matched_
        std::uint32_t get_matched()
        {
            return matched_;
        }

    private:

        //! Number of DataReaders matched to the associated DataWriter
        std::atomic<std::uint32_t> matched_;

    }
    listener;

private:

    Communication comm_;

    eprosima::fastdds::dds::DomainParticipant* participant_;

    eprosima::fastdds::dds::Publisher* publisher_;

    eprosima::fastdds::dds::Topic* topic_;

    eprosima::fastdds::dds::DataWriter* writer_;

    eprosima::fastdds::dds::TypeSupport type_;

    /**
     * Class handling discovery events and dataflow
     */

    //! Run thread for number samples, publish every sleep seconds
    void runThread(
            uint32_t number,
            uint32_t sleep);

    //! Members used for control flow purposes
    static std::atomic<bool> stop_;
};



#endif /* FASTDDS_STATISTICS_BACKEND_TEST_DDS_COMMUNICATION__PUBLISHER_HPP */
