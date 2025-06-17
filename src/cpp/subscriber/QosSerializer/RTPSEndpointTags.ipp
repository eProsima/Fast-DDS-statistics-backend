// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file RTPSEndpointTags.ipp
 */

static constexpr const char* rtps_endpoint_tag = "endpoint_qos";

// History memory policy
static constexpr const char* history_memory_policy_tag = "history_memory_policy";
static constexpr const char* history_memory_policy_preallocated_tag = "PREALLOCATED_MEMORY_MODE";
static constexpr const char* history_memory_policy_preallocated_realloc_tag = "PREALLOCATED_WITH_REALLOC_MEMORY_MODE";
static constexpr const char* history_memory_policy_dynamic_reserve_tag = "DYNAMIC_RESERVE_MEMORY_MODE";
static constexpr const char* history_memory_policy_dynamic_reusable_tag = "DYNAMIC_REUSABLE_MEMORY_MODE";
