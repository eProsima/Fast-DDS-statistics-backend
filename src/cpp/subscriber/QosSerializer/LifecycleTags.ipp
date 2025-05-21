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
 * @file LifecycleTags.ipp
 */

// WriterDataLifeCycle Qos Policy tags
static constexpr const char* writer_data_lifecycle_tag = "writer_data_lifecycle";
static constexpr const char* autodispose_unregistered_instances_tag = "autodispose_unregistered_instances";

// ReaderDataLifeCycle QoS Policy tags
static constexpr const char* reader_data_lifecycle_tag = "reader_data_lifecycle";
static constexpr const char* autopurge_no_writer_samples_delay_tag = "autopurge_no_writer_samples_delay";
static constexpr const char* autopurge_disposed_samples_delay_tag = "autopurge_disposed_samples_delay";
