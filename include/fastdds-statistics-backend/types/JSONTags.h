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
 * @file JSONTags.h
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_JSONTAGS_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_JSONTAGS_HPP_

namespace eprosima {
namespace statistics_backend {

// Version
constexpr const char* VERSION                 = "version";
constexpr const char* ACTUAL_DUMP_VERSION     = "0.0";

// Entity containers
constexpr const char* HOST_CONTAINER          = "hosts";
constexpr const char* USER_CONTAINER          = "users";
constexpr const char* PROCESS_CONTAINER       = "processes";
constexpr const char* DOMAIN_CONTAINER        = "domains";
constexpr const char* TOPIC_CONTAINER         = "topics";
constexpr const char* PARTICIPANT_CONTAINER   = "participants";
constexpr const char* DATAWRITER_CONTAINER    = "datawriters";
constexpr const char* DATAREADER_CONTAINER    = "datareaders";
constexpr const char* LOCATOR_CONTAINER       = "locators";

// Entity
constexpr const char* HOST_ENTITY             = "host";
constexpr const char* USER_ENTITY             = "user";
constexpr const char* PROCESS_ENTITY          = "process";
constexpr const char* DOMAIN_ENTITY           = "domain";
constexpr const char* TOPIC_ENTITY            = "topic";
constexpr const char* PARTICIPANT_ENTITY      = "participant";
constexpr const char* DATAWRITER_ENTITY       = "datawriter";
constexpr const char* DATAREADER_ENTITY       = "datareader";
constexpr const char* LOCATOR_ENTITY          = "locator";

// Internal info
constexpr const char* NAME_INFO               = "name";
constexpr const char* PID_INFO                = "pid";
constexpr const char* DATA_TYPE_INFO          = "data_type";
constexpr const char* GUID_INFO               = "guid";
constexpr const char* QOS_INFO                = "qos";

// Data container
constexpr const char* DATA_CONTAINER          = "data";

// Data kind
constexpr const char* DATA_KIND_FASTDDS_LATENCY             = "history2history_latency";
constexpr const char* DATA_KIND_NETWORK_LATENCY             = "network_latency_per_locator";
constexpr const char* DATA_KIND_PUBLICATION_THROUGHPUT      = "publication_throughput";
constexpr const char* DATA_KIND_SUBSCRIPTION_THROUGHPUT     = "subscription_throughput";
constexpr const char* DATA_KIND_RTPS_PACKETS_SENT           = "rtps_packets_sent";
constexpr const char* DATA_KIND_RTPS_BYTES_SENT             = "rtps_bytes_sent";
constexpr const char* DATA_KIND_RTPS_PACKETS_LOST           = "rtps_packets_lost";
constexpr const char* DATA_KIND_RTPS_BYTES_LOST             = "rtps_bytes_lost";
constexpr const char* DATA_KIND_RESENT_DATA                 = "resent_datas";
constexpr const char* DATA_KIND_HEARTBEAT_COUNT             = "heartbeat_count";
constexpr const char* DATA_KIND_ACKNACK_COUNT               = "acknack_count";
constexpr const char* DATA_KIND_NACKFRAG_COUNT              = "nackfrag_count";
constexpr const char* DATA_KIND_GAP_COUNT                   = "gap_count";
constexpr const char* DATA_KIND_DATA_COUNT                  = "data_count";
constexpr const char* DATA_KIND_PDP_PACKETS                 = "pdp_packets";
constexpr const char* DATA_KIND_EDP_PACKETS                 = "edp_packets";
constexpr const char* DATA_KIND_DISCOVERY_TIME              = "discovery_time";
constexpr const char* DATA_KIND_SAMPLE_DATAS                = "samples_datas";

// Samples values
constexpr const char* DATA_VALUE_SRC_TIME         = "src_time";
constexpr const char* DATA_VALUE_COUNT            = "count";
constexpr const char* DATA_VALUE_DATA             = "data";
constexpr const char* DATA_VALUE_TIME             = "time";
constexpr const char* DATA_VALUE_MAGNITUDE        = "magnitude";
constexpr const char* DATA_VALUE_REMOTE_ENTITY    = "remote_id";
constexpr const char* DATA_VALUE_STATUS           = "status";

} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_JSONTAGS_HPP_
