/* Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file StatisticsReaderListener.cpp
 */

#include "StatisticsReaderListener.hpp"

#include <fastdds/dds/builtin/topic/PublicationBuiltinTopicData.hpp>
#include <fastdds/dds/builtin/topic/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/rtps/builtin/data/ParticipantBuiltinTopicData.hpp>
#include <fastdds/statistics/dds/domain/DomainParticipant.hpp>
#include <fastdds/statistics/topic_names.hpp>

#include <fastdds_statistics_backend/types/types.hpp>
#include <fastdds_statistics_backend/topic_types/monitorservice_types.hpp>
#include <database/database_queue.hpp>
#include <database/database.hpp>
#include "QosSerializer.hpp"

namespace eprosima {
namespace statistics_backend {
namespace subscriber {

using namespace eprosima::fastdds::statistics;
using namespace eprosima::fastdds::statistics::EventKind;
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

StatisticsReaderListener::StatisticsReaderListener(
        database::DatabaseDataQueue<eprosima::fastdds::statistics::Data>* data_queue,
        database::DatabaseDataQueue<database::ExtendedMonitorServiceStatusData>* monitor_service_data_queue,
        const database::Database* db)
noexcept
    : DataReaderListener()
    , data_queue_(data_queue)
    , monitor_service_status_data_queue_(monitor_service_data_queue)
    , db_(db)
{
}

template<typename T>
bool StatisticsReaderListener::get_available_data(
        eprosima::fastdds::dds::DataReader* reader,
        T& inner_data,
        std::chrono::system_clock::time_point& timestamp)
{
    SampleInfo info;
    if (reader->take_next_sample(&inner_data, &info) == RETCODE_OK)
    {
        if (!info.valid_data)
        {
            // Received data not valid
            return false;
        }
        timestamp = nanoseconds_to_systemclock(info.source_timestamp.to_ns());
        return true;
    }
    return false;
}

bool StatisticsReaderListener::get_optional_qos_from_proxy_sample(
        eprosima::fastdds::statistics::dds::DomainParticipant* participant,
        const eprosima::fastdds::statistics::MonitorServiceStatusData& data,
        database::Qos& qos)
{
    if (!participant)
    {
        EPROSIMA_LOG_ERROR(STATISTICSREADERLISTENER,
                "Participant is null, cannot extract QoS from proxy sample.");
        return false;
    }

    if (eprosima::fastdds::statistics::StatusKind::PROXY != data.status_kind())
    {
        EPROSIMA_LOG_ERROR(STATISTICSREADERLISTENER,
                "Data is not a proxy sample, cannot extract QoS.");
        return false;
    }

    switch (db_->get_entity_kind_by_guid(data.local_entity()))
    {
        case EntityKind::PARTICIPANT:
        {
            ParticipantBuiltinTopicData participant_data;

            if (RETCODE_OK !=
                    participant->fill_discovery_data_from_cdr_message(participant_data, data))
            {
                EPROSIMA_LOG_ERROR(STATISTICSREADERLISTENER,
                        "Failed to get participant data for proxy sample.");
                return false;
            }

            qos = optional_qos_to_backend_qos(participant_data);
            return true;
        }
        case EntityKind::DATAWRITER:
        {
            PublicationBuiltinTopicData publication_data;

            if (RETCODE_OK !=
                    participant->fill_discovery_data_from_cdr_message(publication_data, data))
            {
                EPROSIMA_LOG_ERROR(STATISTICSREADERLISTENER,
                        "Failed to get publication data for proxy sample.");
                return false;
            }

            qos = optional_qos_to_backend_qos(publication_data);
            return true;
        }
        case EntityKind::DATAREADER:
        {
            SubscriptionBuiltinTopicData subscription_data;

            if (RETCODE_OK !=
                    participant->fill_discovery_data_from_cdr_message(subscription_data, data))
            {
                EPROSIMA_LOG_ERROR(STATISTICSREADERLISTENER,
                        "Failed to get subscription data for proxy sample.");
                return false;
            }

            qos = optional_qos_to_backend_qos(subscription_data);
            return true;
        }
        default:
            EPROSIMA_LOG_ERROR(STATISTICSREADERLISTENER,
                    "Trying to extract optional QoS from an invalid entity kind");
            return false;
    }

    return true;
}

void StatisticsReaderListener::on_data_available(
        eprosima::fastdds::dds::DataReader* reader)
{
    static const std::map<std::string, uint32_t> statistics_topics =
    {
        {HISTORY_LATENCY_TOPIC,         HISTORY2HISTORY_LATENCY},
        {NETWORK_LATENCY_TOPIC,         NETWORK_LATENCY},
        {PUBLICATION_THROUGHPUT_TOPIC,  PUBLICATION_THROUGHPUT},
        {SUBSCRIPTION_THROUGHPUT_TOPIC, SUBSCRIPTION_THROUGHPUT},
        {RTPS_SENT_TOPIC,               RTPS_SENT},
        {RTPS_LOST_TOPIC,               RTPS_LOST},
        {RESENT_DATAS_TOPIC,            RESENT_DATAS},
        {HEARTBEAT_COUNT_TOPIC,         HEARTBEAT_COUNT},
        {ACKNACK_COUNT_TOPIC,           ACKNACK_COUNT},
        {NACKFRAG_COUNT_TOPIC,          NACKFRAG_COUNT},
        {GAP_COUNT_TOPIC,               GAP_COUNT},
        {DATA_COUNT_TOPIC,              DATA_COUNT},
        {PDP_PACKETS_TOPIC,             PDP_PACKETS},
        {EDP_PACKETS_TOPIC,             EDP_PACKETS},
        {DISCOVERY_TOPIC,               DISCOVERED_ENTITY},
        {SAMPLE_DATAS_TOPIC,            SAMPLE_DATAS},
        {PHYSICAL_DATA_TOPIC,           PHYSICAL_DATA}
    };

    std::chrono::system_clock::time_point timestamp;

    const std::string& topic_name = reader->get_topicdescription()->get_name();

    bool enqueue = false;

    if (MONITOR_SERVICE_TOPIC == topic_name)
    {
        MonitorServiceStatusData inner_data;
        if (get_available_data(reader, inner_data, timestamp))
        {
            enqueue = true;
        }
        if (!enqueue)
        {
            // Nothing to push to queue
            return;
        }

        std::shared_ptr<database::ExtendedMonitorServiceStatusData> monitor_service_status_data =
                std::make_shared<database::ExtendedMonitorServiceStatusData>();

        monitor_service_status_data->data = inner_data;

        // Deserialize optional QoS information for proxy samples
        if (fastdds::statistics::StatusKind::PROXY == inner_data.status_kind())
        {
            database::Qos qos;
            auto participant = eprosima::fastdds::statistics::dds::DomainParticipant::narrow(
                reader->get_subscriber()->get_participant());

            if (!get_optional_qos_from_proxy_sample(
                        const_cast<eprosima::fastdds::statistics::dds::DomainParticipant*>(participant),
                        inner_data,
                        qos))
            {
                EPROSIMA_LOG_ERROR(STATISTICSREADERLISTENER,
                        "Failed to get optional QoS from proxy sample.");
                return;
            }

            monitor_service_status_data->optional_qos = qos;
        }

        monitor_service_status_data_queue_->push(timestamp, monitor_service_status_data);
    }
    else
    {
        std::shared_ptr<Data> data = std::make_shared<Data>();
        if (HISTORY_LATENCY_TOPIC == topic_name)
        {
            WriterReaderData inner_data;
            if (get_available_data(reader, inner_data, timestamp))
            {
                data->writer_reader_data(inner_data);
                enqueue = true;
            }
        }
        else if (NETWORK_LATENCY_TOPIC == topic_name)
        {
            Locator2LocatorData inner_data;
            if (get_available_data(reader, inner_data, timestamp))
            {
                data->locator2locator_data(inner_data);
                enqueue = true;
            }
        }
        else if (PUBLICATION_THROUGHPUT_TOPIC == topic_name || SUBSCRIPTION_THROUGHPUT_TOPIC == topic_name)
        {
            EntityData inner_data;
            if (get_available_data(reader, inner_data, timestamp))
            {
                data->entity_data(inner_data);
                enqueue = true;
            }
        }
        else if (RTPS_SENT_TOPIC == topic_name || RTPS_LOST_TOPIC == topic_name)
        {
            Entity2LocatorTraffic inner_data;
            if (get_available_data(reader, inner_data, timestamp))
            {
                data->entity2locator_traffic(inner_data);
                enqueue = true;
            }
        }
        else if (RESENT_DATAS_TOPIC == topic_name || HEARTBEAT_COUNT_TOPIC == topic_name ||
                ACKNACK_COUNT_TOPIC == topic_name || NACKFRAG_COUNT_TOPIC == topic_name ||
                GAP_COUNT_TOPIC == topic_name ||
                DATA_COUNT_TOPIC == topic_name || PDP_PACKETS_TOPIC == topic_name || EDP_PACKETS_TOPIC == topic_name)
        {
            EntityCount inner_data;
            if (get_available_data(reader, inner_data, timestamp))
            {
                data->entity_count(inner_data);
                enqueue = true;
            }
        }
        else if (DISCOVERY_TOPIC == topic_name)
        {
            DiscoveryTime inner_data;
            if (get_available_data(reader, inner_data, timestamp))
            {
                data->discovery_time(inner_data);
                enqueue = true;
            }
        }
        else if (SAMPLE_DATAS_TOPIC == topic_name)
        {
            SampleIdentityCount inner_data;
            if (get_available_data(reader, inner_data, timestamp))
            {
                data->sample_identity_count(inner_data);
                enqueue = true;
            }
        }
        if (!enqueue)
        {
            // Nothing to push to queue
            return;
        }

        data->_d(statistics_topics.at(topic_name));
        data_queue_->push(timestamp, data);
    }
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
