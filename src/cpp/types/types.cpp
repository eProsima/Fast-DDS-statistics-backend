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
    status = EntityStatus::ERROR;
    src_ts = {};
}

void ProxySample::clear()
{
    MonitorServiceSample::clear();
    entity_proxy.clear();
}

void ConnectionListSample::clear()
{
    MonitorServiceSample::clear();
    connection_list.clear();
}

void IncompatibleQosSample::clear()
{
    MonitorServiceSample::clear();
    incompatible_qos_status.total_count(0);
    incompatible_qos_status.last_policy_id(0);
    incompatible_qos_status.policies(std::vector<eprosima::fastdds::statistics::QosPolicyCount_s>());
}

void InconsistentTopicSample::clear()
{
    MonitorServiceSample::clear();
    inconsistent_topic_status.total_count(0);
}

void LivelinessLostSample::clear()
{
    MonitorServiceSample::clear();
    liveliness_lost_status.total_count(0);
}

void LivelinessChangedSample::clear()
{
    MonitorServiceSample::clear();
    liveliness_changed_status.alive_count(0);
    liveliness_changed_status.not_alive_count(0);
    liveliness_changed_status.last_publication_handle(std::array<uint8_t, 16UL>());
}

void DeadlineMissedSample::clear()
{
    MonitorServiceSample::clear();
    deadline_missed_status.total_count(0);
    deadline_missed_status.last_instance_handle(std::array<uint8_t, 16UL>());
}

void SampleLostSample::clear()
{
    MonitorServiceSample::clear();
    sample_lost_status.total_count(0);
}

} // namespace statistics_backend
} // namespace eprosima
