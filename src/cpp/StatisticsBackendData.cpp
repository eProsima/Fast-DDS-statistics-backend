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
 * @file StatisticsBackendData.cpp
 */


#include <map>
#include <string>

#include "StatisticsBackendData.hpp"
#include "Monitor.hpp"

#include <database/database.hpp>

namespace eprosima {
namespace statistics_backend {

namespace database {

class DatabaseEntityQueue;

} // namespace database

namespace details {


void StatisticsBackendData::on_data_available(
        EntityId domain_id,
        EntityId entity_id,
        DataKind data_kind)
{
    auto monitor = monitors_.find(domain_id);
    assert(monitor != monitors_.end());
    if (nullptr == monitor->second->domain_listener ||
            !monitor->second->domain_callback_mask.is_set(CallbackKind::ON_DATA_AVAILABLE))
    {
        // No user listener, or mask deactivated
        return;
    }

    if (!monitor->second->data_mask.is_set(data_kind))
    {
        // Data mask deactivated
        return;
    }

    monitor->second->domain_listener->on_data_available(domain_id, entity_id, data_kind);
}

void StatisticsBackendData::on_domain_entity_discovery(
        EntityId domain_id,
        EntityId entity_id,
        EntityKind entity_kind,
        DiscoveryStatus discovery_status)
{
    auto monitor = monitors_.find(domain_id);
    assert(monitor != monitors_.end());
    if (monitor->second->domain_listener == nullptr)
    {
        // No user listener
        return;
    }

    switch (entity_kind)
    {
        case EntityKind::PARTICIPANT:
        {
            if (!monitor->second->domain_callback_mask.is_set(CallbackKind::ON_PARTICIPANT_DISCOVERY))
            {
                // mask deactivated
                return;
            }

            if (discovery_status == DISCOVERY)
            {
                monitor->second->participant_status_.on_instance_discovered();
            }
            else if (discovery_status == UNDISCOVERY)
            {
                monitor->second->participant_status_.on_instance_undiscovered();
            }
            monitor->second->domain_listener->on_participant_discovery(domain_id, entity_id,
                    monitor->second->participant_status_);
            monitor->second->participant_status_.on_status_read();
            break;
        }
        case EntityKind::TOPIC:
        {
            if (!monitor->second->domain_callback_mask.is_set(CallbackKind::ON_TOPIC_DISCOVERY))
            {
                // mask deactivated
                return;
            }

            if (discovery_status == DISCOVERY)
            {
                monitor->second->topic_status_.on_instance_discovered();
            }
            else if (discovery_status == UNDISCOVERY)
            {
                monitor->second->topic_status_.on_instance_undiscovered();
            }
            monitor->second->domain_listener->on_topic_discovery(domain_id, entity_id, monitor->second->topic_status_);
            monitor->second->topic_status_.on_status_read();
            break;
        }
        case EntityKind::DATAWRITER:
        {
            if (!monitor->second->domain_callback_mask.is_set(CallbackKind::ON_DATAWRITER_DISCOVERY))
            {
                // mask deactivated
                return;
            }

            if (discovery_status == DISCOVERY)
            {
                monitor->second->datawriter_status_.on_instance_discovered();
            }
            else if (discovery_status == UNDISCOVERY)
            {
                monitor->second->datawriter_status_.on_instance_undiscovered();
            }
            monitor->second->domain_listener->on_datawriter_discovery(domain_id, entity_id,
                    monitor->second->datawriter_status_);
            monitor->second->datawriter_status_.on_status_read();
            break;
        }
        case EntityKind::DATAREADER:
        {
            if (!monitor->second->domain_callback_mask.is_set(CallbackKind::ON_DATAREADER_DISCOVERY))
            {
                // mask deactivated
                return;
            }

            if (discovery_status == DISCOVERY)
            {
                monitor->second->datareader_status_.on_instance_discovered();
            }
            else if (discovery_status == UNDISCOVERY)
            {
                monitor->second->datareader_status_.on_instance_undiscovered();
            }
            monitor->second->domain_listener->on_datareader_discovery(domain_id, entity_id,
                    monitor->second->datareader_status_);
            monitor->second->datareader_status_.on_status_read();
            break;
        }
        default:
        {
            throw Error("wrong entity_kind");
        }
    }
}

void StatisticsBackendData::on_physical_entity_discovery(
        EntityId participant_id,
        EntityId entity_id,
        EntityKind entity_kind)
{
    if (physical_listener_ == nullptr)
    {
        // No user listener
        return;
    }

    switch (entity_kind)
    {
        case EntityKind::HOST:
        {
            if (!physical_callback_mask_.is_set(CallbackKind::ON_HOST_DISCOVERY))
            {
                // mask deactivated
                return;
            }

            host_status_.on_instance_discovered();
            physical_listener_->on_host_discovery(participant_id, entity_id, host_status_);
            host_status_.on_status_read();
            break;
        }
        case EntityKind::USER:
        {
            if (!physical_callback_mask_.is_set(CallbackKind::ON_USER_DISCOVERY))
            {
                // mask deactivated
                return;
            }

            user_status_.on_instance_discovered();
            physical_listener_->on_user_discovery(participant_id, entity_id, user_status_);
            user_status_.on_status_read();
            break;
        }
        case EntityKind::PROCESS:
        {
            if (!physical_callback_mask_.is_set(CallbackKind::ON_PROCESS_DISCOVERY))
            {
                // mask deactivated
                return;
            }

            process_status_.on_instance_discovered();
            physical_listener_->on_process_discovery(participant_id, entity_id, process_status_);
            process_status_.on_status_read();
            break;
        }
        case EntityKind::LOCATOR:
        {
            if (!physical_callback_mask_.is_set(CallbackKind::ON_LOCATOR_DISCOVERY))
            {
                // mask deactivated
                return;
            }

            locator_status_.on_instance_discovered();
            physical_listener_->on_locator_discovery(participant_id, entity_id, locator_status_);
            locator_status_.on_status_read();
            break;
        }
        default:
        {
            throw Error("wrong entity_kind");
        }
    }
}

} // namespace details
} // namespace statistics_backend
} // namespace eprosima
