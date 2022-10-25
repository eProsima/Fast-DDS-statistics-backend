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

#include "StatisticsBackendData.hpp"

#include <map>
#include <string>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/qos/DomainParticipantFactoryQos.hpp>
#include <fastdds/dds/domain/DomainParticipantListener.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>

#include <fastdds_statistics_backend/listener/DomainListener.hpp>
#include <fastdds_statistics_backend/listener/PhysicalListener.hpp>

#include "Monitor.hpp"
#include <database/database_queue.hpp>
#include <database/database.hpp>

namespace eprosima {
namespace statistics_backend {
namespace details {

// Declare singleton instance as default (nullptr)
SingletonType StatisticsBackendData::instance_;

StatisticsBackendData::StatisticsBackendData()
    : database_(new database::Database)
    , entity_queue_(new database::DatabaseEntityQueue(database_.get()))
    , data_queue_(new database::DatabaseDataQueue(database_.get()))
    , physical_listener_(nullptr)
    , lock_(mutex_, std::defer_lock)
    , participant_factory_instance_(eprosima::fastdds::dds::DomainParticipantFactory::get_shared_instance())
{
    // Do nothing
}

StatisticsBackendData::~StatisticsBackendData()
{
    // Destroy each monitor
    while (!monitors_by_entity_.empty())
    {
        // Beware that stop_monitor removes the monitor from monitors_by_entity_
        // so we cannot use iterators here
        auto monitor = monitors_by_entity_.begin()->second;
        stop_monitor(monitor->id);
    }

    if (entity_queue_)
    {
        entity_queue_->stop_consumer();
    }
    if (data_queue_)
    {
        data_queue_->stop_consumer();
    }

    delete entity_queue_;
    delete data_queue_;
}

const SingletonType& StatisticsBackendData::get_instance()
{
    if (!instance_)
    {
        instance_ =
                SingletonType(
            new StatisticsBackendData(),
            [](StatisticsBackendData* s)
            {
                delete s;
            });
    }
    return instance_;
}

void StatisticsBackendData::reset_instance()
{
    instance_.reset(new StatisticsBackendData());
}

void StatisticsBackendData::lock()
{
    lock_.lock();
}

void StatisticsBackendData::unlock()
{
    lock_.unlock();
}

void StatisticsBackendData::on_data_available(
        EntityId domain_id,
        EntityId entity_id,
        DataKind data_kind)
{
    auto monitor = monitors_by_entity_.find(domain_id);
    assert(monitor != monitors_by_entity_.end());

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
        std::shared_ptr<Monitor>& monitor,
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

    // If data_kind is INVALID, we do not care about the data kind mask
    // We assume an entity was discovered
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

    // If data_kind is INVALID, we do not care about the data kind mask
    // We assume an entity was discovered
    if (data_kind != DataKind::INVALID && !physical_data_mask_.is_set(data_kind))
    {
        // Data mask deactivated
        return false;
    }

    return true;
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
    auto monitor = monitors_by_entity_.find(domain_id);
    assert(monitor != monitors_by_entity_.end());

    switch (entity_kind)
    {
        case EntityKind::PARTICIPANT:
        {
            // The status must be recorded regardless of the callback
            prepare_entity_discovery_status(discovery_status, monitor->second->participant_status_);

            if (should_call_domain_listener(monitor->second, CallbackKind::ON_PARTICIPANT_DISCOVERY))
            {
                monitor->second->domain_listener->on_participant_discovery(domain_id, entity_id,
                        monitor->second->participant_status_);
                monitor->second->participant_status_.on_status_read();
            }
            else if (should_call_physical_listener(CallbackKind::ON_PARTICIPANT_DISCOVERY))
            {
                physical_listener_->on_participant_discovery(domain_id, entity_id,
                        monitor->second->participant_status_);
                monitor->second->participant_status_.on_status_read();
            }
            break;
        }
        case EntityKind::TOPIC:
        {
            // The status must be recorded regardless of the callback
            prepare_entity_discovery_status(discovery_status, monitor->second->topic_status_);

            if (should_call_domain_listener(monitor->second, CallbackKind::ON_TOPIC_DISCOVERY))
            {
                monitor->second->domain_listener->on_topic_discovery(domain_id, entity_id,
                        monitor->second->topic_status_);
                monitor->second->topic_status_.on_status_read();
            }
            else if (should_call_physical_listener(CallbackKind::ON_TOPIC_DISCOVERY))
            {
                physical_listener_->on_topic_discovery(domain_id, entity_id, monitor->second->topic_status_);
                monitor->second->topic_status_.on_status_read();
            }
            break;
        }
        case EntityKind::DATAWRITER:
        {
            // The status must be recorded regardless of the callback
            prepare_entity_discovery_status(discovery_status, monitor->second->datawriter_status_);

            if (should_call_domain_listener(monitor->second, CallbackKind::ON_DATAWRITER_DISCOVERY))
            {
                monitor->second->domain_listener->on_datawriter_discovery(domain_id, entity_id,
                        monitor->second->datawriter_status_);
                monitor->second->datawriter_status_.on_status_read();
            }
            else if (should_call_physical_listener(CallbackKind::ON_DATAWRITER_DISCOVERY))
            {
                physical_listener_->on_datawriter_discovery(domain_id, entity_id, monitor->second->datawriter_status_);
                monitor->second->datawriter_status_.on_status_read();
            }
            break;
        }
        case EntityKind::DATAREADER:
        {
            // The status must be recorded regardless of the callback
            prepare_entity_discovery_status(discovery_status, monitor->second->datareader_status_);

            if (should_call_domain_listener(monitor->second, CallbackKind::ON_DATAREADER_DISCOVERY))
            {
                monitor->second->domain_listener->on_datareader_discovery(domain_id, entity_id,
                        monitor->second->datareader_status_);
                monitor->second->datareader_status_.on_status_read();
            }
            else if (should_call_physical_listener(CallbackKind::ON_DATAREADER_DISCOVERY))
            {
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
        EntityId entity_id,
        EntityKind entity_kind,
        DiscoveryStatus discovery_status)
{
    assert(discovery_status != DiscoveryStatus::UPDATE);

    switch (entity_kind)
    {
        case EntityKind::HOST:
        {
            // The status must be recorded regardless of the callback
            prepare_entity_discovery_status(discovery_status, host_status_);

            if (should_call_physical_listener(CallbackKind::ON_HOST_DISCOVERY))
            {
                physical_listener_->on_host_discovery(entity_id, host_status_);
                host_status_.on_status_read();
            }
            break;
        }
        case EntityKind::USER:
        {
            // The status must be recorded regardless of the callback
            prepare_entity_discovery_status(discovery_status, user_status_);

            if (should_call_physical_listener(CallbackKind::ON_USER_DISCOVERY))
            {
                physical_listener_->on_user_discovery(entity_id, user_status_);
                user_status_.on_status_read();
            }
            break;
        }
        case EntityKind::PROCESS:
        {
            // The status must be recorded regardless of the callback
            prepare_entity_discovery_status(discovery_status, process_status_);

            if (should_call_physical_listener(CallbackKind::ON_PROCESS_DISCOVERY))
            {
                physical_listener_->on_process_discovery(entity_id, process_status_);
                process_status_.on_status_read();
            }
            break;
        }
        case EntityKind::LOCATOR:
        {
            // The status must be recorded regardless of the callback
            prepare_entity_discovery_status(discovery_status, locator_status_);

            if (should_call_physical_listener(CallbackKind::ON_LOCATOR_DISCOVERY))
            {
                physical_listener_->on_locator_discovery(entity_id, locator_status_);
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

void StatisticsBackendData::stop_monitor(
        EntityId monitor_id)
{
    lock();

    //Find the monitor
    auto it = monitors_by_entity_.find(monitor_id);
    if (it == monitors_by_entity_.end())
    {
        unlock();
        throw BadParameter("No monitor with such ID");
    }
    auto monitor = it->second;
    monitors_by_entity_.erase(it);

    // Delete everything created during monitor initialization
    // These values are not always set, as could come from an error creating Monitor, or for test sake.
    if (monitor->participant)
    {
        if (monitor->subscriber)
        {
            for (auto& reader : monitor->readers)
            {
                monitor->subscriber->delete_datareader(reader.second);
            }

            monitor->participant->delete_subscriber(monitor->subscriber);
        }

        for (auto& topic : monitor->topics)
        {
            monitor->participant->delete_topic(topic.second);
        }

        fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(monitor->participant);
    }

    if (monitor->reader_listener)
    {
        delete monitor->reader_listener;
    }

    if (monitor->participant_listener)
    {
        delete monitor->participant_listener;
    }

    // The monitor is inactive
    // NOTE: for test sake, this is not always set
    if (database_ && database_->is_entity_present(monitor_id))
    {
        database_->change_entity_status(monitor_id, false);
    }

    unlock();
}

} // namespace details
} // namespace statistics_backend
} // namespace eprosima
