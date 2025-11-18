/* Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file UserDataReaderListener.cpp
 */

#include "UserDataReaderListener.hpp"

#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/TopicDescription.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/utils.hpp>

#include <functional>
#include <sstream>
#include <string>

namespace eprosima {
namespace statistics_backend {
namespace subscriber {

using namespace eprosima::fastdds::dds;

UserDataReaderListener::UserDataReaderListener(
        std::function<void(const std::string& data)> on_data_received,
        UserDataContext* ctx) noexcept
    : on_data_received_(on_data_received)
    , ctx_(ctx)
{
}

void UserDataReaderListener::on_data_available(
        DataReader* reader)
{
    DynamicType::_ref_type type = ctx_->get_type_from_topic_name(reader->get_topicdescription()->get_name());

    // Type is required to deserialize the data
    if (!type)
    {
        EPROSIMA_LOG_WARNING(USER_DATA_READER_LISTENER, "Unable to process user data. Topic type is unknown.");
        return;
    }

    DynamicData::_ref_type data = DynamicDataFactory::get_instance()->create_data(type);
    assert(data != nullptr);

    SampleInfo info;
    while (RETCODE_OK == reader->take_next_sample(&data, &info))
    {
        if (info.valid_data)
        {
            std::string serialized_data = serialize_user_data(data, type);
            if (!serialized_data.empty() && on_data_received_)
            {
                on_data_received_(serialized_data);
            }
        }
    }

    DynamicDataFactory::get_instance()->delete_data(data);
}

std::string UserDataReaderListener::serialize_user_data(
        DynamicData::_ref_type data,
        DynamicType::_ref_type type) const
{
    std::stringstream serialized_data;
    if (!type)
    {
        EPROSIMA_LOG_WARNING(USER_DATA_READER_LISTENER, "Unable to serialize user data. Topic type is unknown.");
        return std::string();
    }

    if (RETCODE_OK != json_serialize(data, DynamicDataJsonFormat::EPROSIMA, serialized_data))
    {
        EPROSIMA_LOG_WARNING(USER_DATA_READER_LISTENER, "Unable to serialize user data. Serialization failed.");
        return std::string();
    }

    return serialized_data.str();
}

} // namespace subscriber
} // namespace statistics_backend
} // namespace eprosima