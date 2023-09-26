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

#include <fastdds_statistics_backend/types/types.hpp>

namespace eprosima {
namespace statistics_backend {

void MonitorServiceSample::clear()
{
    kind = StatusKind::INVALID;
    status = EntityStatus::OK;
    src_ts = {};
}

void IncompatibleQosSample::clear()
{
    MonitorServiceSample::clear();
    incompatible_qos_status.total_count(0);
    incompatible_qos_status.last_policy_id(0);
    incompatible_qos_status.policies(std::vector<eprosima::fastdds::statistics::QosPolicyCount_s>());
}

} // namespace statistics_backend
} // namespace eprosima
