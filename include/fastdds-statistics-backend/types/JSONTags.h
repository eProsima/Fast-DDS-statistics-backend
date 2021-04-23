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
 * @file StatisticsBackend.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_JSONTAGS_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_JSONTAGS_HPP_

namespace eprosima {
namespace statistics_backend {

// Version
extern const char* VERSION;
extern const char* ACTUAL_DUMP_VERSION;

// Entity containers
extern const char* HOST_CONTAINER;
extern const char* USER_CONTAINER;
extern const char* PROCESS_CONTAINER;
extern const char* DOMAIN_CONTAINER;
extern const char* TOPIC_CONTAINER;
extern const char* PARTICIPANT_CONTAINER;
extern const char* DATAWRITER_CONTAINER;
extern const char* DATAREADER_CONTAINER;
extern const char* LOCATOR_CONTAINER;

// Entity
extern const char* HOST_ENTITY;
extern const char* USER_ENTITY;
extern const char* PROCESS_ENTITY;
extern const char* DOMAIN_ENTITY;
extern const char* TOPIC_ENTITY;
extern const char* PARTICIPANT_ENTITY;
extern const char* DATAWRITER_ENTITY;
extern const char* DATAREADER_ENTITY;
extern const char* LOCATOR_ENTITY;

// Internal info
extern const char* NAME_INFO;
extern const char* PID_INFO;
extern const char* DATA_TYPE_INFO;
extern const char* GUID_INFO;
extern const char* QOS_INFO;

// Data container
extern const char* DATA_CONTAINER;

// Data kind
extern const char* DATA_KIND_FASTDDS_LATENCY;
extern const char* DATA_KIND_NETWORK_LATENCY;
extern const char* DATA_KIND_PUBLICATION_THROUGHPUT;
extern const char* DATA_KIND_SUBSCRIPTION_THROUGHPUT;
extern const char* DATA_KIND_RTPS_PACKETS_SENT;
extern const char* DATA_KIND_RTPS_BYTES_SENT;
extern const char* DATA_KIND_RTPS_PACKETS_LOST;
extern const char* DATA_KIND_RTPS_BYTES_LOST;
extern const char* DATA_KIND_RESENT_DATA;
extern const char* DATA_KIND_HEARTBEAT_COUNT;
extern const char* DATA_KIND_ACKNACK_COUNT;
extern const char* DATA_KIND_NACKFRAG_COUNT;
extern const char* DATA_KIND_GAP_COUNT;
extern const char* DATA_KIND_DATA_COUNT;
extern const char* DATA_KIND_PDP_PACKETS;
extern const char* DATA_KIND_EDP_PACKETS;
extern const char* DATA_KIND_DISCOVERY_TIME;
extern const char* DATA_KIND_SAMPLE_DATAS;

// Samples values
extern const char* DATA_VALUE_SRC_TIME;
extern const char* DATA_VALUE_COUNT;
extern const char* DATA_VALUE_DATA;
extern const char* DATA_VALUE_TIME;
extern const char* DATA_VALUE_MAGNITUDE;
extern const char* DATA_VALUE_REMOTE_ENTITY;
extern const char* DATA_VALUE_STATUS;

} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_JSONTAGS_HPP_
