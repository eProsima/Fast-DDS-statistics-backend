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

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_MONITOR_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_MONITOR_HPP_

#include <map>
#include <string>

#include <fastdds-statistics-backend/listener/DomainListener.hpp>
#include <fastdds-statistics-backend/listener/CallbackMask.hpp>
#include <fastdds-statistics-backend/types/EntityId.hpp>
#include <fastdds-statistics-backend/types/EntityId.hpp>

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

struct Monitor
{
    DomainListener* domain_listener;
    CallbackMask domain_callback_mask;
    DataKindMask data_mask;
    fastdds::dds::DomainParticipant* participant;
    fastdds::dds::DomainParticipantListener* participant_listener;
    fastdds::dds::Subscriber* subscriber;
    std::map<std::string, fastdds::dds::Topic*> topics;
    std::map<std::string, fastdds::dds::DataReader*> readers;
    fastdds::dds::DataReaderListener* reader_listener;

};

} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_MONITOR_HPP_
