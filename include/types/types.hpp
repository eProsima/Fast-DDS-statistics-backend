/* Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file types.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_TYPES_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_TYPES_HPP_

#include <nlohmann-json/json.hpp>

#include <string>

namespace eprosima {
namespace statistics_backend {

/**
 * Type for unique identifier of Entities
 */
using EntityId = std::string;

/**
 * Qos tree structure. Please refer to https://nlohmann.github.io/json/doxygen/index.html
 */
using Qos = nlohmann::json;

/*
 * Schema for Entities kinds store in Backend
 *                  *
 *    PHYSICAL      *           LOGICAL
 *   ──────────     *          ─────────
 *                  *
 * ┌───────────┐    *     ┌───────────┐
 * │   HOST    │    *     │  DOMAIN   │1
 * └──┬────────┘    *     └─┬─────────┴─────┐
 *    │1            *       │1              │
 *    │             *       │               │
 *    │N            *       │               │N
 * ┌──▼────────┐    *       │            ┌──▼────────┐
 * │   USER    │    *       │            │   TOPIC   │
 * └──┬────────┘    *       │            ├──────────┬┘
 *    │1            *       │            │1         │1
 *    │             *       │            │          │
 *    │N            *       │N           │          │
 * ┌──▼────────┐1   *    N┌─▼─────────┐  │          │
 * │  PROCESS  ├──────────►PARTICIPANT│1 │          │
 * └───────────┘    *     └─────┬─────┴──┼───────┐  │
 *                  *           │1       │       │  │
 *                  *           │        │       │  │
 *                  *           │N       │N      │N │N
 *                  *         ┌─▼────────▼┐    ┌─▼──▼──────┐
 *                  *         │DATAREADER │    │DATAWRITER │
 *                  *         └─┬─────────┘    └┬──────────┘
 *                  *           │M              │M
 * ┌───────────┐N   *           │               │
 * │  LOCATOR  ◄────────────────┴───────────────┘
 * └───────────┘    *
 */

/**
 * Indicates the Type of an Entity in Statistics Backend structure
 */
enum EntityKind
{
    /// Host/Machine where a participant is allocated
    HOST,

    /// User that has executed a participant
    USER,

    /// Process where a participant is running
    PROCESS,

    /// Abstract DDS network by Domain or by Discovery Server
    DOMAIN,

    /// DDS Topic
    TOPIC,

    /// DDS Domain Participant
    PARTICIPANT,

    /// DDS DataWriter
    DATAWRITER,

    /// DDS DataReader
    DATAREADER,

    /// Physical locator that a communication is using (IP + port || SHM + port)
    /// Store the Locator Statistic data
    LOCATOR,
};

/**
 * Indicates the Type of Data stored by the Backend
 *
 *     | Signature               | Entities source   | Entity target | No. entities | Data Type   |
 *     |-------------------------|-------------------|---------------|--------------|-------------|
 *     | FASTDDS_LATENCY         | DataWriter        | DataReader    | 2            | EntityData  |
 *     | NETWORK_LATENCY         | Locator           | Locator       | 2            | EntityData  |
 *     | PUBLICATION_THROUGHPUT  | DataWriter        |               | 1            | EntityData  |
 *     | SUBSCRIPTION_THROUGHPUT | DataReader        |               | 1            | EntityData  |
 *     | RTPS_PACKETS_SENT       | DataWriter        | Locator       | 2            | EntityCount |
 *     | RTPS_BYTES_SENT         | DataWriter        | Locator       | 2            | ByteCount   |
 *     | RTPS_PACKETS_LOST       | DataWriter        | Locator       | 2            | EntityCount |
 *     | RTPS_BYTES_LOST         | DataWriter        | Locator       | 2            | ByteCount   |
 *     | RESENT_DATA             | DataWriter        |               | 1            | EntityCount |
 *     | HEARTBEAT_COUNT         | DataWriter        |               | 1            | EntityCount |
 *     | ACKNACK_COUNT           | DataReader        |               | 1            | EntityCount |
 *     | NACKFRAG_COUNT          | DataReader        |               | 1            | EntityCount |
 *     | GAP_COUNT               | DataWriter        |               | 1            | EntityCount |
 *     | DATA_COUNT              | DataWriter        |               | 1            | EntityCount |
 *     | PDP_PACKETS             | DomainParticipant |               | 1            | EntityCount |
 *     | EDP_PACKETS             | DomainParticipant |               | 1            | EntityCount |
 *     | DISCOVERED_ENTITY       | DDSEntity         | DDSEntity     | 2            | Time        |
 *     | SAMPLE_DATAS            | DataWriter        |               | 1            | EntityData  |
 */
enum DataKind
{
    /// Latency between a write operation (writer side) and data available
    /// (notification to user in reader side)
    FASTDDS_LATENCY             = 1 << 0,

    /// Latency between Locators pair
    NETWORK_LATENCY             = 1 << 1,

    /// Amount of data [Mb/s] sent by a DataWriter
    PUBLICATION_THROUGHPUT      = 1 << 2,

    /// Amount of data [Mb/s] received by a DataReader
    SUBSCRIPTION_THROUGHPUT     = 1 << 3,

    /// Amount of packets sent from a DDS Entity to a Locator
    RTPS_PACKETS_SENT           = 1 << 4,

    /// Amount of bytes sent from a DDS Entity to a Locator
    RTPS_BYTES_SENT             = 1 << 5,

    /// Amount of packets lost from a DDS Entity to a Locator
    RTPS_PACKETS_LOST           = 1 << 6,

    /// Amount of bytes lost from a DDS Entity to a Locator
    RTPS_BYTES_LOST             = 1 << 7,

    /// Amount of DATA/DATAFRAG sub-messages resent from a DataWriter/DomainParticipant
    RESENT_DATA                 = 1 << 8,

    /// Amount of HEARTBEATs that each non discovery DataWriter/DomainParticipant sends
    HEARTBEAT_COUNT             = 1 << 9,

    /// Amount of ACKNACKs that each non discovery DataReader/DomainParticipant sends
    ACKNACK_COUNT               = 1 << 10,

    /// Amount of NACKFRAGs that each non discovery DataReader/DomainParticipant sends
    NACKFRAG_COUNT              = 1 << 11,

    /// Amount of GAPs sub-messages sent from a DataWriter/DomainParticipant
    GAP_COUNT                   = 1 << 12,

    /// Amount of DATA/DATAFRAG sub-messages that each non discovery DataWriter sends
    DATA_COUNT                  = 1 << 13,

    /// Amount of PDP packets sent by Participant
    PDP_PACKETS                 = 1 << 14,

    /// Amount of EDP packets sent by Participant
    EDP_PACKETS                 = 1 << 15,

    /// Time when a DDS Entity discovers another DDS entity
    DISCOVERY_TIME              = 1 << 16,

    /// Amount of DATA/DATAFRAG sub-messages needed to send a single sample
    SAMPLE_DATAS                = 1 << 17,
};

/// Bitmask of data kinds, that can be constructed combining values of DataKind with the '|' operator.
using DataKindMask = int32_t;

/*
 * Available statistics operations to be performed on the raw data.
 */
enum StatisticKind
{
    /// Non accumulative kind, it chooses a data point between the set given.
    /// Implemented to take the first data in set : [0]
    NONE,

    /// Numerical mean of values in the set
    MEAN,

    /// Standard Deviation of the values in the set
    STANDARD_DEVIATION,

    /// Maximum value in the set
    MAX,

    /// Minimum value in the set
    MIN,

    /// Median value of the set
    MEDIAN,

    /// Amount of values in the set
    COUNT,

    /// Summation of the values in the set
    SUM
};

} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_TYPES_HPP_
