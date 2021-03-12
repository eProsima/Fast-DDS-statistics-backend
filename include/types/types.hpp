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

#ifndef EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_TYPES_HPP_
#define EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_TYPES_HPP_

#include <nlohmann-json/json.hpp>

#include <string>

namespace eprosima {
namespace statistics_backend {

/**
 * Type for unique identifier of Entities
 */
using EntityId = std::string;

/**
 * Qos tree structure
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
    /// Store the Domain Participant Qos
    /// Store the Domain Participant Statistic data
    PARTICIPANT,

    /// DDS DataWriter
    /// Store the DataWriter Qos
    /// Store the DataWriter Statistic data
    DATAWRITER,

    /// DDS DataReader
    /// Store the DataReader Qos
    /// Store the DataReader Statistic data
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
    FASTDDS_LATENCY,

    /// Latency between Locators pair
    NETWORK_LATENCY,

    /// Throughput (Mb/s) sent by a DataWriter
    PUBLICATION_THROUGHPUT,

    /// Throughput (Mb/s) by a DataReader
    SUBSCRIPTION_THROUGHPUT,

    /// Amount of packets sent from a DDS Entity to a Locator
    RTPS_PACKETS_SENT,

    /// Amount of bytes sent from a DDS Entity to a Locator
    RTPS_BYTES_SENT,

    /// Amount of packets lost from a DDS Entity to a Locator
    RTPS_PACKETS_LOST,

    /// Amount of bytes lost from a DDS Entity to a Locator
    RTPS_BYTES_LOST,

    /// Amount of DATA/DATAFRAG sub-messages resent from a DataWriter/DomainParticipant
    RESENT_DATA,

    /// Amount of HEARTBEATs that each non discovery DataWriter/DomainParticipant sends
    HEARTBEAT_COUNT,

    /// Amount of ACKNACKs that each non discovery DataReader/DomainParticipant sends
    ACKNACK_COUNT,

    /// Amount of NACKFRAGs that each non discovery DataReader/DomainParticipant sends
    NACKFRAG_COUNT,

    /// Amount of GAPs sub-messages resent from a DataWriter/DomainParticipant
    GAP_COUNT,

    /// Amount of DATA/DATAFRAG sub-messages that each non discovery DataWriter sends
    DATA_COUNT,

    /// Amount of PDP packets sent by Participant
    PDP_PACKETS,

    /// Amount of EDP packets sent by Participant
    EDP_PACKETS,

    /// Time when a DDS Entity discovers another DDS entity
    DISCOVERED_ENTITY,

    /// Amount of DATA/DATAFRAG sub-messages needed to send a single sample
    SAMPLE_DATAS,
};

/*
 * Available statistics operation to be perform on the raw data.
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

#endif // EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_TYPES_HPP_
