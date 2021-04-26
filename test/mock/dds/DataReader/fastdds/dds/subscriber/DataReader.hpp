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
using StatisticsData = eprosima::fastdds::statistics::Data;


namespace eprosima {
namespace fastdds {
namespace dds {


class DataReader
{
protected:

    using Sample = std::pair<std::shared_ptr<StatisticsData>, std::shared_ptr<SampleInfo>>;

public:

    DataReader() = default;

    ReturnCode_t take_next_sample(
            void* data,
            SampleInfo* info)
    {
        if (history_.empty())
        {
            return ReturnCode_t::RETCODE_NO_DATA;
        }

        *static_cast<StatisticsData*>(data) = *(history_.front().first.get());
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


protected:

    fastrtps::rtps::GUID_t guid_;
    std::queue<Sample> history_;

};

} /* namespace dds */
} /* namespace fastdds */
} /* namespace eprosima */

#endif /* _FASTDDS_DDS_SUBSCRIBER_DATAREADER_HPP_*/
