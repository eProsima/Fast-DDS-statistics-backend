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
 * @file LivelinessTags.ipp
 */

static constexpr const char* liveliness_tag = "liveliness";

// Liveliness kind
static constexpr const char* liveliness_automatic_tag = "AUTOMATIC_LIVELINESS_QOS";
static constexpr const char* liveliness_manual_participant_tag = "MANUAL_BY_PARTICIPANT_LIVELINESS_QOS";
static constexpr const char* liveliness_manual_topic_tag = "MANUAL_BY_TOPIC_LIVELINESS_QOS";

// Other liveliness related tags
static constexpr const char* announcement_period_tag = "announcement_period";
static constexpr const char* lease_duration_tag = "lease_duration";
