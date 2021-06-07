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
 * @file DurabilityTags.ipp
 */

static constexpr const char* durability_tag = "durability";

// Durability kind
static constexpr const char* durability_volatile_tag = "VOLATILE_DURABILITY_QOS";
static constexpr const char* durability_transient_local_tag = "TRANSIENT_LOCAL_DURABILITY_QOS";
static constexpr const char* durability_transient_tag = "TRANSIENT_DURABILITY_QOS";
static constexpr const char* durability_persistent_tag = "PERSISTENT_DURABILITY_QOS";

// Durability service
static constexpr const char* durability_service_tag = "durability_service";
static constexpr const char* service_cleanup_delay_tag = "service_cleanup_delay";
