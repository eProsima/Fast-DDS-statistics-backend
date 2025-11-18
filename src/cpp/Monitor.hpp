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
 * @file Monitor.hpp
 */

#ifndef FASTDDS_STATISTICS_BACKEND_SRC_CPP__MONITOR_HPP
#define FASTDDS_STATISTICS_BACKEND_SRC_CPP__MONITOR_HPP

#include <map>
#include <string>

#include <fastdds_statistics_backend/listener/CallbackMask.hpp>
#include <fastdds_statistics_backend/listener/DomainListener.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>

#include <fastdds/dds/topic/TypeSupport.hpp>

#include "subscriber/UserDataContext.hpp"

namespace eprosima {
namespace fastdds {
namespace dds {

class DomainParticipant;
class DomainParticipantListener;
class Subscriber;
class Topic;
class DataReader;
class DataReaderListener;

} // namespace dds
} // namespace fastdds

namespace statistics_backend {
namespace details {

/**
 * @brief Structure holding all the information related to a backend monitor.
 *
 * The backend will create and maintain an instance for each monitored domain.
 */
struct Monitor
{
    //! The EntityId of the monitored domain
    EntityId id{};

    //! The user listener for this monitor
    DomainListener* domain_listener = nullptr;

    //! The callback mask applied to the \c domain_listener
    CallbackMask domain_callback_mask{};

    //! The data mask applied to the \c domain_listener->on_data_available
    DataKindMask data_mask{};

    //! The participant created to communicate with the statistics reporting endpoints in this monitor
    fastdds::dds::DomainParticipant* participant = nullptr;

    //! The listener linked to the \c statistics_participant
    //! It will process the entity discoveries
    fastdds::dds::DomainParticipantListener* participant_listener = nullptr;

    //! The subscriber created to communicate with other endpoints in this monitor
    fastdds::dds::Subscriber* subscriber = nullptr;

    //! Holds the topic object created for each of the statistics topics
    std::map<std::string, fastdds::dds::Topic*> statistics_topics{};

    //! Holds the datareader object created for each statistics topic
    std::map<std::string, fastdds::dds::DataReader*> statistics_readers{};

    //! The listener linked to the \c statistics readers
    //! All statistics readers will use the same listener
    //! The listener will decide how to process the data according to the topic of the reader
    fastdds::dds::DataReaderListener* statistics_reader_listener = nullptr;

    //! Holds the topic object created for each of the user data topics
    std::map<std::string, fastdds::dds::Topic*> user_data_topics{};

    //! Holds the datareader object created for each user data topic
    std::map<std::string, fastdds::dds::DataReader*> user_data_readers{};

    //! Holds the datareader listener object created for each user data topic
    std::map<std::string, fastdds::dds::DataReaderListener*> user_data_listeners{};

    //! Holds the information required to process the user data
    subscriber::UserDataContext user_data_context;

    //! Participant discovery status. Used in the participant discovery user callback
    DomainListener::Status participant_status_{};

    //! Topic discovery status. Used in the topic discovery user callback
    DomainListener::Status topic_status_{};

    //! Datareader discovery status. Used in the datareader discovery user callback
    DomainListener::Status datareader_status_{};

    //! DataWriter discovery status. Used in the datawriter discovery user callback
    DomainListener::Status datawriter_status_{};
};

} // namespace details
} // namespace statistics_backend
} // namespace eprosima

#endif //FASTDDS_STATISTICS_BACKEND_SRC_CPP__MONITOR_HPP
