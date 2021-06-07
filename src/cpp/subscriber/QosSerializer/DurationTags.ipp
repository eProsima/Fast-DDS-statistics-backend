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
 * @file DurationTags.ipp
 */

static constexpr const char* duration_tag = "duration";
static constexpr const char* duration_period_tag = "period";
static constexpr const char* duration_seconds_tag = "seconds";
static constexpr const char* duration_nanoseconds_tag = "nanoseconds";

// Time based filter tags
static constexpr const char* time_based_filter_tag = "time_based_filter";
static constexpr const char* minimum_separation_tag = "minimum_separation";

// Other duration valued tags
static constexpr const char* deadline_tag = "deadline";
static constexpr const char* latency_budget_tag = "latency_budget";
static constexpr const char* lifespan_tag = "lifespan";
static constexpr const char* disable_positive_acks_tag = "disable_positive_acks";
