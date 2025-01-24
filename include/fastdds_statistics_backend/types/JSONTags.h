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

#ifndef FASTDDS_STATISTICS_BACKEND_TYPES__JSON_TAGS_H
#define FASTDDS_STATISTICS_BACKEND_TYPES__JSON_TAGS_H

namespace eprosima {
namespace statistics_backend {

////////////////////////
// DATABASE DUMP TAGS //
////////////////////////

/////
// Actual Version

//! Actual version of the Database Dump
constexpr const char* ACTUAL_DUMP_VERSION           = "0.0";

/////
// Version

//! Key Tag for version
constexpr const char* VERSION_TAG                   = "version";

/////
// Entity containers

//! Key Tag for container of hosts
constexpr const char* HOST_CONTAINER_TAG            = "hosts";
//! Key Tag for container of users
constexpr const char* USER_CONTAINER_TAG            = "users";
//! Key Tag for container of processes
constexpr const char* PROCESS_CONTAINER_TAG         = "processes";
//! Key Tag for container of domains
constexpr const char* DOMAIN_CONTAINER_TAG          = "domains";
//! Key Tag for container of topics
constexpr const char* TOPIC_CONTAINER_TAG           = "topics";
//! Key Tag for container of participants
constexpr const char* PARTICIPANT_CONTAINER_TAG     = "participants";
//! Key Tag for container of datawriters
constexpr const char* DATAWRITER_CONTAINER_TAG      = "datawriters";
//! Key Tag for container of datareaders
constexpr const char* DATAREADER_CONTAINER_TAG      = "datareaders";
//! Key Tag for container of endpoints
constexpr const char* ENDPOINT_CONTAINER_TAG        = "endpoints";
//! Key Tag for container of locators
constexpr const char* LOCATOR_CONTAINER_TAG         = "locators";

/////
// Entity kind

//! Key Tag for a single host entity
constexpr const char* HOST_ENTITY_TAG               = "host";
//! Key Tag for a single user entity
constexpr const char* USER_ENTITY_TAG               = "user";
//! Key Tag for a single process entity
constexpr const char* PROCESS_ENTITY_TAG            = "process";
//! Key Tag for a single domain entity
constexpr const char* DOMAIN_ENTITY_TAG             = "domain";
//! Key Tag for a single topic entity
constexpr const char* TOPIC_ENTITY_TAG              = "topic";
//! Key Tag for a single participant entity
constexpr const char* PARTICIPANT_ENTITY_TAG        = "participant";
//! Key Tag for a single datawriter entity
constexpr const char* DATAWRITER_ENTITY_TAG         = "datawriter";
//! Key Tag for a single datareader entity
constexpr const char* DATAREADER_ENTITY_TAG         = "datareader";
//! Key Tag for a single locator entity
constexpr const char* LOCATOR_ENTITY_TAG            = "locator";

/////
// Status Level

//! OK status
constexpr const char* OK_STATUS                        = "OK";
//! Warning status
constexpr const char* WARNING_STATUS                   = "WARNING";
//! Error status
constexpr const char* ERROR_STATUS                     = "ERROR";

/////
// Internal info

//! Key tag for the ID of an entity
constexpr const char* ID_TAG                   = "id";
//! Key tag for the EntityKind of an entity
constexpr const char* KIND_TAG                 = "kind";
//! Key tag for the StatusLevel of an entity
constexpr const char* STATUS_TAG                 = "status";
//! Key tag for name of an entity
constexpr const char* NAME_TAG                 = "name";
//! Key tag for alias of an entity
constexpr const char* ALIAS_TAG                 = "alias";
//! Key tag for alive status of an entity
constexpr const char* ALIVE_TAG                  = "alive";
//! Key tag for metatraffic attribute of an entity
constexpr const char* METATRAFFIC_TAG          = "metatraffic";
//! Key tag for process id of a process entity
constexpr const char* PID_TAG                  = "pid";
//! Key tag for type name of a topic entity
constexpr const char* DATA_TYPE_TAG            = "data_type";
//! Key tag for GUID of a participant, datawriter or datareader entity
constexpr const char* GUID_TAG                 = "guid";
//! Key tag for QoS of a participant, datawriter or datareader entity
constexpr const char* QOS_TAG                  = "qos";
//! Key tag for app_id of a participant entity
constexpr const char* APP_ID_TAG                  = "app_id";
//! Key tag for app_metadata of a participant entity
constexpr const char* APP_METADATA_TAG                  = "app_metadata";
//! Key tag for meta traffic flag of a virtual endpoint
constexpr const char* VIRTUAL_METATRAFFIC_TAG       = "virtual_metatraffic";
//! Key tag for vendor id of a participant entity
constexpr const char* DDS_VENDOR_TAG                = "dds_vendor";

//! Conversion from EntityKind to string
constexpr const char* entity_kind_str[] =
{"invalid", HOST_ENTITY_TAG, USER_ENTITY_TAG, PROCESS_ENTITY_TAG, DOMAIN_ENTITY_TAG, TOPIC_ENTITY_TAG,
 PARTICIPANT_ENTITY_TAG, DATAWRITER_ENTITY_TAG, DATAREADER_ENTITY_TAG, LOCATOR_ENTITY_TAG};
//! Conversion from StatusLevel to string
constexpr const char* status_level_str[] =
{OK_STATUS, WARNING_STATUS, ERROR_STATUS};

/////
// Data container

//! Key tag for container of data kinds
constexpr const char* DATA_CONTAINER_TAG            = "data";

/////
// Data kind

//! Key tag for container of data kind FASTDDS_LATENCY
constexpr const char* DATA_KIND_FASTDDS_LATENCY_TAG             = "history2history_latency";
//! Key tag for container of data kind NETWORK_LATENCY
constexpr const char* DATA_KIND_NETWORK_LATENCY_TAG             = "network_latency_per_locator";
//! Key tag for container of data kind PUBLICATION_THROUGHPUT
constexpr const char* DATA_KIND_PUBLICATION_THROUGHPUT_TAG      = "publication_throughput";
//! Key tag for container of data kind SUBSCRIPTION_THROUGHPUT
constexpr const char* DATA_KIND_SUBSCRIPTION_THROUGHPUT_TAG     = "subscription_throughput";
//! Key tag for container of data kind RTPS_PACKETS_SENT
constexpr const char* DATA_KIND_RTPS_PACKETS_SENT_TAG           = "rtps_packets_sent";
//! Key tag for container of data kind RTPS_BYTES_SENT
constexpr const char* DATA_KIND_RTPS_BYTES_SENT_TAG             = "rtps_bytes_sent";
//! Key tag for container of data kind RTPS_PACKETS_LOST
constexpr const char* DATA_KIND_RTPS_PACKETS_LOST_TAG           = "rtps_packets_lost";
//! Key tag for container of data kind RTPS_BYTES_LOST
constexpr const char* DATA_KIND_RTPS_BYTES_LOST_TAG             = "rtps_bytes_lost";
//! Key tag for container of data kind RESENT_DATA
constexpr const char* DATA_KIND_RESENT_DATA_TAG                 = "resent_datas";
//! Key tag for container of data kind HEARTBEAT_COUNT
constexpr const char* DATA_KIND_HEARTBEAT_COUNT_TAG             = "heartbeat_count";
//! Key tag for container of data kind ACKNACK_COUNT
constexpr const char* DATA_KIND_ACKNACK_COUNT_TAG               = "acknack_count";
//! Key tag for container of data kind NACKFRAG_COUNT
constexpr const char* DATA_KIND_NACKFRAG_COUNT_TAG              = "nackfrag_count";
//! Key tag for container of data kind GAP_COUNT
constexpr const char* DATA_KIND_GAP_COUNT_TAG                   = "gap_count";
//! Key tag for container of data kind DATA_COUNT
constexpr const char* DATA_KIND_DATA_COUNT_TAG                  = "data_count";
//! Key tag for container of data kind PDP_PACKETS
constexpr const char* DATA_KIND_PDP_PACKETS_TAG                 = "pdp_packets";
//! Key tag for container of data kind EDP_PACKETS
constexpr const char* DATA_KIND_EDP_PACKETS_TAG                 = "edp_packets";
//! Key tag for container of data kind DISCOVERY_TIME
constexpr const char* DATA_KIND_DISCOVERY_TIME_TAG              = "discovery_time";
//! Key tag for container of data kind SAMPLE_DATAS
constexpr const char* DATA_KIND_SAMPLE_DATAS_TAG                = "samples_datas";

//! Key tag for container of last reported data in data kind RTPS_PACKETS_SENT
constexpr const char* DATA_KIND_RTPS_PACKETS_SENT_LAST_REPORTED_TAG             = "last_reported_rtps_packets_sent";
//! Key tag for container of last reported data in data kind RTPS_BYTES_SENT
constexpr const char* DATA_KIND_RTPS_BYTES_SENT_LAST_REPORTED_TAG               = "last_reported_rtps_bytes_sent";
//! Key tag for container of last reported data in data kind RTPS_PACKETS_LOST
constexpr const char* DATA_KIND_RTPS_PACKETS_LOST_LAST_REPORTED_TAG             = "last_reported_rtps_packets_lost";
//! Key tag for container of last reported data in data kind RTPS_BYTES_LOST
constexpr const char* DATA_KIND_RTPS_BYTES_LOST_LAST_REPORTED_TAG               = "last_reported_rtps_bytes_lost";
//! Key tag for container of last data reported in data kind PDP_PACKETS
constexpr const char* DATA_KIND_PDP_PACKETS_LAST_REPORTED_TAG                   = "last_reported_pdp_packets";
//! Key tag for container of last data reported in data kind EDP_PACKETS
constexpr const char* DATA_KIND_EDP_PACKETS_LAST_REPORTED_TAG                   = "last_reported_edp_packets";
//! Key tag for container of last data reported in data kind HEARTBEAT_COUNT
constexpr const char* DATA_KIND_HEARTBEAT_COUNT_LAST_REPORTED_TAG               = "last_reported_heartbeat_count";
//! Key tag for container of last data reported in data kind ACKNACK_COUNT
constexpr const char* DATA_KIND_ACKNACK_COUNT_LAST_REPORTED_TAG                 = "last_reported_acknack_count";
//! Key tag for container of last data reported in data kind NACKFRAG_COUNT
constexpr const char* DATA_KIND_NACKFRAG_COUNT_LAST_REPORTED_TAG                = "last_reported_nackfrag_count";
//! Key tag for container of last data reported in data kind GAP_COUNT
constexpr const char* DATA_KIND_GAP_COUNT_LAST_REPORTED_TAG                     = "last_reported_gap_count";
//! Key tag for container of last data reported in data kind DATA_COUNT
constexpr const char* DATA_KIND_DATA_COUNT_LAST_REPORTED_TAG                    = "last_reported_data_count";
//! Key tag for container of last data reported in data kind RESENT_DATA
constexpr const char* DATA_KIND_RESENT_DATA_LAST_REPORTED_TAG                    = "last_reported_resent_datas";

/////
// Samples values

//! Key tag for source time (time_point) value in data
constexpr const char* DATA_VALUE_SRC_TIME_TAG       = "src_time";
//! Key tag for count (uint64_t) value in data
constexpr const char* DATA_VALUE_COUNT_TAG          = "count";
//! Key tag for data (double) value in data
constexpr const char* DATA_VALUE_DATA_TAG           = "data";
//! Key tag for time (time_point) value in data
constexpr const char* DATA_VALUE_TIME_TAG           = "time";
//! Key tag for magnitude (int16_t) value in data
constexpr const char* DATA_VALUE_MAGNITUDE_TAG      = "magnitude";
//! Key tag for remote id (EntityId) value in data
constexpr const char* DATA_VALUE_REMOTE_ENTITY_TAG  = "remote_id";
//! Key tag for discovered (bool) value in data
constexpr const char* DATA_VALUE_DISCOVERED_TAG     = "discovered";

} //namespace statistics_backend
} //namespace eprosima

#endif // FASTDDS_STATISTICS_BACKEND_TYPES__JSON_TAGS_H
