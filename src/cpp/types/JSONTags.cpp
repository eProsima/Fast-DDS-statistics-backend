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

#include <fastdds-statistics-backend/types/JSONTags.h>

namespace eprosima {
namespace statistics_backend {

// Version
const char* VERSION                 = "version";
const char* ACTUAL_DUMP_VERSION     = "0.0";

// Entity containers
const char* HOST_CONTAINER          = "hosts";
const char* USER_CONTAINER          = "users";
const char* PROCESS_CONTAINER       = "processes";
const char* DOMAIN_CONTAINER        = "domains";
const char* TOPIC_CONTAINER         = "topics";
const char* PARTICIPANT_CONTAINER   = "participants";
const char* DATAWRITER_CONTAINER    = "datawriters";
const char* DATAREADER_CONTAINER    = "datareaders";
const char* LOCATOR_CONTAINER       = "locators";

// Entity
const char* HOST_ENTITY             = "host";
const char* USER_ENTITY             = "user";
const char* PROCESS_ENTITY          = "process";
const char* DOMAIN_ENTITY           = "domain";
const char* TOPIC_ENTITY            = "topic";
const char* PARTICIPANT_ENTITY      = "participant";
const char* DATAWRITER_ENTITY       = "datawriter";
const char* DATAREADER_ENTITY       = "datareader";
const char* LOCATOR_ENTITY          = "locator";

// Internal info
const char* NAME_INFO               = "name";
const char* PID_INFO                = "pid";
const char* DATA_TYPE_INFO          = "data_type";
const char* GUID_INFO               = "guid";
const char* QOS_INFO                = "qos";

// Data container
const char* DATA_CONTAINER          = "data";

// Data kind
const char* DATA_KIND_FASTDDS_LATENCY             = "history2history_latency";
const char* DATA_KIND_NETWORK_LATENCY             = "network_latency_per_locator";
const char* DATA_KIND_PUBLICATION_THROUGHPUT      = "publication_throughput";
const char* DATA_KIND_SUBSCRIPTION_THROUGHPUT     = "subscription_throughput";
const char* DATA_KIND_RTPS_PACKETS_SENT           = "rtps_packets_sent";
const char* DATA_KIND_RTPS_BYTES_SENT             = "rtps_bytes_sent";
const char* DATA_KIND_RTPS_PACKETS_LOST           = "rtps_packets_lost";
const char* DATA_KIND_RTPS_BYTES_LOST             = "rtps_bytes_lost";
const char* DATA_KIND_RESENT_DATA                 = "resent_datas";
const char* DATA_KIND_HEARTBEAT_COUNT             = "heartbeat_count";
const char* DATA_KIND_ACKNACK_COUNT               = "acknack_count";
const char* DATA_KIND_NACKFRAG_COUNT              = "nackfrag_count";
const char* DATA_KIND_GAP_COUNT                   = "gap_count";
const char* DATA_KIND_DATA_COUNT                  = "data_count";
const char* DATA_KIND_PDP_PACKETS                 = "pdp_packets";
const char* DATA_KIND_EDP_PACKETS                 = "edp_packets";
const char* DATA_KIND_DISCOVERY_TIME              = "discovery_time";
const char* DATA_KIND_SAMPLE_DATAS                = "samples_datas";

// Samples values
const char* DATA_VALUE_SRC_TIME         = "src_time";
const char* DATA_VALUE_COUNT            = "count";
const char* DATA_VALUE_DATA             = "data";
const char* DATA_VALUE_TIME             = "time";
const char* DATA_VALUE_MAGNITUDE        = "magnitude";
const char* DATA_VALUE_REMOTE_ENTITY    = "remote_id";
const char* DATA_VALUE_STATUS           = "status";

} // namespace statistics_backend
} // namespace eprosima
