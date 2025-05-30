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
 * @file ReliabilityTags.ipp
 */

static constexpr const char* reliability_tag = "reliability";

// Reliability kind
static constexpr const char* reliability_best_effort_tag = "BEST_EFFORT_RELIABILITY_QOS";
static constexpr const char* reliability_reliable_tag = "RELIABLE_RELIABILITY_QOS";

// Other reliability related tags
static constexpr const char* max_blocking_time_tag = "max_blocking_time";
static constexpr const char* disable_heartbeat_piggyback_tag = "disable_heartbeat_piggyback";
static constexpr const char* initial_heartbeat_delay_tag = "initial_heartbeat_delay";
static constexpr const char* heartbeat_period_tag = "heartbeat_period";
static constexpr const char* nack_response_delay_tag = "nack_response_delay";
static constexpr const char* nack_supression_duration_tag = "nack_supression_duration";
static constexpr const char* initial_acknack_delay_tag = "initial_acknack_delay";
static constexpr const char* reader_times_tag = "reader_times";
static constexpr const char* writer_times_tag = "writer_times";
static constexpr const char* heartbeat_response_delay_tag = "heartbeat_response_delay";
static constexpr const char* rtps_reliable_reader_tag = "rtps_reliable_reader";
static constexpr const char* rtps_reliable_writer_tag = "rtps_reliable_writer";
