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

static bool should_call_listener(
        DataKind data_kind,
        CallbackKind callback_kind,
        DomainListener* listener,
        const CallbackMask& callback_mask,
        const DataKindMask& data_mask)
{
    if (nullptr == listener)
    {
        // No user listener
        return false;
    }

    if (!callback_mask.is_set(callback_kind))
    {
        // mask deactivated
        return false;
    }

    return
        (DataKind::INVALID == data_kind) ||   // Special value to avoid data_mask checking
        data_mask.is_set(data_kind);          // Data mask activated
}        

bool StatisticsBackendData::should_call_domain_listener(
        std::unique_ptr<Monitor>& monitor,
        CallbackKind callback_kind,
        DataKind data_kind)
{
    return should_call_listener(data_kind, callback_kind, monitor->domain_listener,
            monitor->domain_callback_mask, monitor->data_mask);
}

bool StatisticsBackendData::should_call_physical_listener(
        CallbackKind callback_kind,
        DataKind data_kind)
{
    return should_call_listener(data_kind, callback_kind, physical_listener_,
            physical_callback_mask_, physical_data_mask_);
}

void StatisticsBackendData::prepare_entity_discovery_status(
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
}

void StatisticsBackendData::on_domain_entity_discovery(
        EntityId domain_id,
        EntityId entity_id,
        EntityKind entity_kind,
        DiscoveryStatus discovery_status)
{
    using namespace std::placeholders;
    auto monitor = monitors_.find(domain_id);
    assert(monitor != monitors_.end());

    switch (entity_kind)
    {
        case EntityKind::PARTICIPANT:
        {
            call_discovery_listeners(
                monitor->second,
                std::bind(&DomainListener::on_participant_discovery, monitor->second->domain_listener, _1, _2, _3),
                std::bind(&PhysicalListener::on_participant_discovery, physical_listener_, _1, _2, _3),
                CallbackKind::ON_PARTICIPANT_DISCOVERY,
                domain_id, entity_id, discovery_status,
                monitor->second->participant_status_);
            break;
        }
        case EntityKind::TOPIC:
        {
            call_discovery_listeners(
                monitor->second,
                std::bind(&DomainListener::on_topic_discovery, monitor->second->domain_listener, _1, _2, _3),
                std::bind(&PhysicalListener::on_topic_discovery, physical_listener_, _1, _2, _3),
                CallbackKind::ON_TOPIC_DISCOVERY,
                domain_id, entity_id, discovery_status,
                monitor->second->topic_status_);
            break;
        }
        case EntityKind::DATAWRITER:
        {
            call_discovery_listeners(
                monitor->second,
                std::bind(&DomainListener::on_datawriter_discovery, monitor->second->domain_listener, _1, _2, _3),
                std::bind(&PhysicalListener::on_datawriter_discovery, physical_listener_, _1, _2, _3),
                CallbackKind::ON_DATAWRITER_DISCOVERY,
                domain_id, entity_id, discovery_status,
                monitor->second->datawriter_status_);
            break;
        }
        case EntityKind::DATAREADER:
        {
            call_discovery_listeners(
                monitor->second,
                std::bind(&DomainListener::on_datareader_discovery, monitor->second->domain_listener, _1, _2, _3),
                std::bind(&PhysicalListener::on_datareader_discovery, physical_listener_, _1, _2, _3),
                CallbackKind::ON_DATAREADER_DISCOVERY,
                domain_id, entity_id, discovery_status,
                monitor->second->datareader_status_);
            break;
        }
        default:
        {
            assert(false && "Invalid domain entity kind");
        }
    }
}

void StatisticsBackendData::process_physical_discovery(
        PhysicalCallback callback,
        CallbackKind callback_kind,
        EntityId participant_id,
        EntityId entity_id,
        DiscoveryStatus discovery_status,
        DomainListener::Status& status)
{
    if (should_call_physical_listener(callback_kind))
    {
        prepare_entity_discovery_status(discovery_status, status);
        (physical_listener_->*callback)(participant_id, entity_id, status);
        status.on_status_read();
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
            process_physical_discovery(&PhysicalListener::on_host_discovery, CallbackKind::ON_HOST_DISCOVERY,
                    participant_id, entity_id, DISCOVERY, host_status_);
            break;
        }
        case EntityKind::USER:
        {
            process_physical_discovery(&PhysicalListener::on_user_discovery, CallbackKind::ON_USER_DISCOVERY,
                    participant_id, entity_id, DISCOVERY, user_status_);
            break;
        }
        case EntityKind::PROCESS:
        {
            process_physical_discovery(&PhysicalListener::on_process_discovery, CallbackKind::ON_PROCESS_DISCOVERY,
                    participant_id, entity_id, DISCOVERY, process_status_);
            break;
        }
        case EntityKind::LOCATOR:
        {
            process_physical_discovery(&PhysicalListener::on_locator_discovery, CallbackKind::ON_LOCATOR_DISCOVERY,
                    participant_id, entity_id, DISCOVERY, locator_status_);
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
