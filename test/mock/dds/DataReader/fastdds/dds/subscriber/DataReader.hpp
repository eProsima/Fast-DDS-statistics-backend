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
 * @file DataReader.hpp
 *
 */

#ifndef _FASTDDS_DDS_SUBSCRIBER_DATAREADER_HPP_
#define _FASTDDS_DDS_SUBSCRIBER_DATAREADER_HPP_

#include <queue>

#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/rtps/common/Guid.h>
#include <fastrtps/types/TypesBase.h>

#include <topic_types/types.h>

using eprosima::fastrtps::types::ReturnCode_t;

namespace eprosima {
namespace fastdds {
namespace dds {


class DataReader
{
protected:

    using StatisticsEventKind = eprosima::fastdds::statistics::EventKind;
    using StatisticsData = eprosima::fastdds::statistics::Data;
    using Sample = std::pair<std::shared_ptr<StatisticsData>, std::shared_ptr<SampleInfo>>;

    using StatisticsWriterReaderData = eprosima::fastdds::statistics::WriterReaderData;
    using StatisticsLocator2LocatorData = eprosima::fastdds::statistics::Locator2LocatorData;
    using StatisticsEntityData = eprosima::fastdds::statistics::EntityData;
    using StatisticsEntity2LocatorTraffic = eprosima::fastdds::statistics::Entity2LocatorTraffic;
    using StatisticsEntityCount = eprosima::fastdds::statistics::EntityCount;
    using StatisticsDiscoveryTime = eprosima::fastdds::statistics::DiscoveryTime;
    using StatisticsSampleIdentityCount = eprosima::fastdds::statistics::SampleIdentityCount;
    using StatisticsPhysicalData = eprosima::fastdds::statistics::PhysicalData;

public:

    class TopicDescription
    {
    public:

        void set_name(
                const std::string name)
        {
            topic_name_ = name;
        }

        const std::string get_name() const
        {
            return topic_name_;
        }

        std::string topic_name_;
    };

    DataReader() = default;

    ReturnCode_t take_next_sample(
            void* data,
            SampleInfo* info)
    {
        if (history_.empty())
        {
            return ReturnCode_t::RETCODE_NO_DATA;
        }

        switch (history_.front().first.get()->_d())
        {
            case StatisticsEventKind::HISTORY2HISTORY_LATENCY:
                *static_cast<StatisticsWriterReaderData*>(data) = history_.front().first->writer_reader_data();
                break;
            case StatisticsEventKind::NETWORK_LATENCY:
                *static_cast<StatisticsLocator2LocatorData*>(data) = history_.front().first->locator2locator_data();
                break;
            case StatisticsEventKind::PUBLICATION_THROUGHPUT:
            case StatisticsEventKind::SUBSCRIPTION_THROUGHPUT:
                *static_cast<StatisticsEntityData*>(data) = history_.front().first->entity_data();
                break;
            case StatisticsEventKind::RTPS_SENT:
            case StatisticsEventKind::RTPS_LOST:
                *static_cast<StatisticsEntity2LocatorTraffic*>(data) = history_.front().first->entity2locator_traffic();
                break;
            case StatisticsEventKind::RESENT_DATAS:
            case StatisticsEventKind::HEARTBEAT_COUNT:
            case StatisticsEventKind::ACKNACK_COUNT:
            case StatisticsEventKind::NACKFRAG_COUNT:
            case StatisticsEventKind::GAP_COUNT:
            case StatisticsEventKind::DATA_COUNT:
            case StatisticsEventKind::PDP_PACKETS:
            case StatisticsEventKind::EDP_PACKETS:
                *static_cast<StatisticsEntityCount*>(data) = history_.front().first->entity_count();
                break;
            case StatisticsEventKind::DISCOVERED_ENTITY:
                *static_cast<StatisticsDiscoveryTime*>(data) = history_.front().first->discovery_time();
                break;
            case StatisticsEventKind::SAMPLE_DATAS:
                *static_cast<StatisticsSampleIdentityCount*>(data) = history_.front().first->sample_identity_count();
                break;
            case StatisticsEventKind::PHYSICAL_DATA:
                *static_cast<StatisticsPhysicalData*>(data) = history_.front().first->physical_data();
                break;
        }

        *info = *(history_.front().second.get());
        history_.pop();
        return ReturnCode_t::RETCODE_OK;
    }

    const fastrtps::rtps::GUID_t& guid()
    {
        return guid_;
    }

    bool is_sample_valid(
            const void* /*data*/,
            const SampleInfo* /*info*/) const
    {
        return true;
    }

    void add_sample(
            std::shared_ptr<StatisticsData> data,
            std::shared_ptr<SampleInfo> info)
    {
        history_.push(std::make_pair(data, info));
    }

    void set_guid(
            fastrtps::rtps::GUID_t guid)
    {
        guid_ = guid;
    }

    void set_topic_name(
            const std::string& name)
    {
        topic_description_.set_name(name);
    }

    const TopicDescription* get_topicdescription()
    {
        return &topic_description_;
    }

protected:

    fastrtps::rtps::GUID_t guid_;
    std::queue<Sample> history_;
    TopicDescription topic_description_;

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_DDS_SUBSCRIBER_DATAREADER_HPP_*/
