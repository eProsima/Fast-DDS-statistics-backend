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
 * @file ResourceTags.ipp
 */

// History related tags
static constexpr const char* history_tag = "history";
static constexpr const char* history_depth_tag = "history_depth";
static constexpr const char* history_kind_tag = "history_kind";
static constexpr const char* history_keep_last_tag = "KEEP_LAST_HISTORY_QOS";
static constexpr const char* history_keep_all_tag = "KEEP_ALL_HISTORY_QOS";

// Resource limits QoS related tags
static constexpr const char* max_instances_tag = "max_instances";
static constexpr const char* max_samples_tag = "max_samples";
static constexpr const char* max_samples_per_instance_tag = "max_samples_per_instance";
static constexpr const char* allocated_samples_tag = "allocated_samples";
static constexpr const char* extra_samples_tag = "extra_samples";
static constexpr const char* resource_limits_tag = "resource_limits";

// Writer Resource Limits QoS related tags
static constexpr const char* writer_resource_limits_tag = "writer_resource_limits";
static constexpr const char* matched_subscriber_allocation_tag = "matched_subscriber_allocation";
static constexpr const char* reader_filters_allocation_tag = "reader_filters_allocation";

// Reader Resource Limits QoS related tags
static constexpr const char* reader_resource_limits_tag = "reader_resource_limits";
static constexpr const char* matched_publisher_allocation_tag = "matched_publisher_allocation";
static constexpr const char* sample_infos_allocation_tag = "sample_infos_allocation";
static constexpr const char* outstanding_reads_allocation_tag = "outstanding_reads_allocation";
static constexpr const char* max_samples_per_read_tag = "max_samples_per_read";

// Resource limited collection configuration related tags
static constexpr const char* resource_limited_container_initial_number_tag = "initial";
static constexpr const char* resource_limited_container_maximum_number_tag = "maximum";
static constexpr const char* resource_limited_container_increment_tag = "increment";
