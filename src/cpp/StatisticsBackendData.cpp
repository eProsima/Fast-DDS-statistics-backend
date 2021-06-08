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

namespace details {


void StatisticsBackendData::on_data_available(
        EntityId domain_id,
        EntityId entity_id,
        DataKind data_kind)
{
    auto monitor = monitors_.find(domain_id);
    assert(monitor != monitors_.end());

    if (should_call_domain_listener(monitor->second, CallbackKind::ON_DATA_AVAILABLE, data_kind))
    {
        monitor->second->domain_listener->on_data_available(domain_id, entity_id, data_kind);
    }
    else if (should_call_physical_listener(CallbackKind::ON_DATA_AVAILABLE, data_kind))
    {
        physical_listener_->on_data_available(domain_id, entity_id, data_kind);
    }
}

bool StatisticsBackendData::should_call_domain_listener(
        std::unique_ptr<Monitor>& monitor,
        CallbackKind callback_kind,
        DataKind data_kind)
{
    if (nullptr == monitor->domain_listener)
    {
        // No user listener
        return false;
    }

    if (!monitor->domain_callback_mask.is_set(callback_kind))
    {
        // mask deactivated
        return false;
    }

    if (data_kind != DataKind::INVALID && !monitor->data_mask.is_set(data_kind))
    {
        // Data mask deactivated
        return false;
    }

    return true;
}

bool StatisticsBackendData::should_call_physical_listener(
        CallbackKind callback_kind,
        DataKind data_kind)
{
    if ( nullptr == physical_listener_)
    {
        // No user listener
        return false;
    }

    if (!physical_callback_mask_.is_set(callback_kind))
    {
        // mask deactivated
        return false;
    }

    if (data_kind != DataKind::INVALID && !physical_data_mask_.is_set(data_kind))
    {
        // Data mask deactivated
        return false;
    }

    return true;
}

bool StatisticsBackendData::prepare_entity_discovery_status(
        DiscoveryStatus discovery_status,
        DomainListener::Status& status)
{
    if (DISCOVERY == discovery_status)
    {
        status.on_instance_discovered();
    }
    else if (UNDISCOVERY == discovery_status)
    {
        status.on_instance_undiscovered();
    }
    return true;
}

void StatisticsBackendData::on_domain_entity_discovery(
        EntityId domain_id,
        EntityId entity_id,
        EntityKind entity_kind,
        DiscoveryStatus discovery_status)
{
    auto monitor = monitors_.find(domain_id);
    assert(monitor != monitors_.end());

    switch (entity_kind)
    {
        case EntityKind::PARTICIPANT:
        {
            if (should_call_domain_listener(monitor->second, CallbackKind::ON_PARTICIPANT_DISCOVERY))
            {
                prepare_entity_discovery_status(discovery_status, monitor->second->participant_status_);
                monitor->second->domain_listener->on_participant_discovery(domain_id, entity_id,
                        monitor->second->participant_status_);
                monitor->second->participant_status_.on_status_read();
            }
            else if (should_call_physical_listener(CallbackKind::ON_PARTICIPANT_DISCOVERY))
            {
                prepare_entity_discovery_status(discovery_status, monitor->second->participant_status_);
                physical_listener_->on_participant_discovery(domain_id, entity_id,
                        monitor->second->participant_status_);
                monitor->second->participant_status_.on_status_read();
            }
            break;
        }
        case EntityKind::TOPIC:
        {
            if (should_call_domain_listener(monitor->second, CallbackKind::ON_TOPIC_DISCOVERY))
            {
                prepare_entity_discovery_status(discovery_status, monitor->second->topic_status_);
                monitor->second->domain_listener->on_topic_discovery(domain_id, entity_id,
                        monitor->second->topic_status_);
                monitor->second->topic_status_.on_status_read();
            }
            else if (should_call_physical_listener(CallbackKind::ON_TOPIC_DISCOVERY))
            {
                prepare_entity_discovery_status(discovery_status, monitor->second->topic_status_);
                physical_listener_->on_topic_discovery(domain_id, entity_id, monitor->second->topic_status_);
                monitor->second->topic_status_.on_status_read();
            }
            break;
        }
        case EntityKind::DATAWRITER:
        {
            if (should_call_domain_listener(monitor->second, CallbackKind::ON_DATAWRITER_DISCOVERY))
            {
                prepare_entity_discovery_status(discovery_status, monitor->second->datawriter_status_);
                monitor->second->domain_listener->on_datawriter_discovery(domain_id, entity_id,
                        monitor->second->datawriter_status_);
                monitor->second->datawriter_status_.on_status_read();
            }
            else if (should_call_physical_listener(CallbackKind::ON_DATAWRITER_DISCOVERY))
            {
                prepare_entity_discovery_status(discovery_status, monitor->second->datawriter_status_);
                physical_listener_->on_datawriter_discovery(domain_id, entity_id, monitor->second->datawriter_status_);
                monitor->second->datawriter_status_.on_status_read();
            }
            break;
        }
        case EntityKind::DATAREADER:
        {
            if (should_call_domain_listener(monitor->second, CallbackKind::ON_DATAREADER_DISCOVERY))
            {
                prepare_entity_discovery_status(discovery_status, monitor->second->datareader_status_);
                monitor->second->domain_listener->on_datareader_discovery(domain_id, entity_id,
                        monitor->second->datareader_status_);
                monitor->second->datareader_status_.on_status_read();
            }
            else if (should_call_physical_listener(CallbackKind::ON_DATAREADER_DISCOVERY))
            {
                prepare_entity_discovery_status(discovery_status, monitor->second->datareader_status_);
                physical_listener_->on_datareader_discovery(domain_id, entity_id, monitor->second->datareader_status_);
                monitor->second->datareader_status_.on_status_read();
            }
            break;
        }
        default:
        {
            assert(false && "Invalid domain entity kind");
        }
    }
}

void StatisticsBackendData::on_physical_entity_discovery(
        EntityId participant_id,
        EntityId entity_id,
        EntityKind entity_kind)
{
    switch (entity_kind)
    {
        case EntityKind::HOST:
        {
            if (should_call_physical_listener(CallbackKind::ON_HOST_DISCOVERY))
            {
                prepare_entity_discovery_status(DISCOVERY, host_status_);
                physical_listener_->on_host_discovery(participant_id, entity_id, host_status_);
                host_status_.on_status_read();
            }
            break;
        }
        case EntityKind::USER:
        {
            if (should_call_physical_listener(CallbackKind::ON_USER_DISCOVERY))
            {
                prepare_entity_discovery_status(DISCOVERY, user_status_);
                physical_listener_->on_user_discovery(participant_id, entity_id, user_status_);
                user_status_.on_status_read();
            }
            break;
        }
        case EntityKind::PROCESS:
        {
            if (should_call_physical_listener(CallbackKind::ON_PROCESS_DISCOVERY))
            {
                prepare_entity_discovery_status(DISCOVERY, process_status_);
                physical_listener_->on_process_discovery(participant_id, entity_id, process_status_);
                process_status_.on_status_read();
            }
            break;
        }
        case EntityKind::LOCATOR:
        {
            if (should_call_physical_listener(CallbackKind::ON_LOCATOR_DISCOVERY))
            {
                prepare_entity_discovery_status(DISCOVERY, locator_status_);
                physical_listener_->on_locator_discovery(participant_id, entity_id, locator_status_);
                locator_status_.on_status_read();
            }
            break;
        }
        default:
        {
            assert(false && "Invalid physical entity kind");
        }
    }
}

} // namespace details
} // namespace statistics_backend
} // namespace eprosima
