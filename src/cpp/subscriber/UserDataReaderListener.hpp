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
 * @file UserDataReaderListener.hpp
 */

#ifndef FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__USER_DATA_READER_LISTENER_HPP
#define FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__USER_DATA_READER_LISTENER_HPP

#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>

#include <functional>
#include <string>

#include "UserDataContext.hpp"

namespace eprosima {
namespace statistics_backend {
namespace subscriber {

class UserDataReaderListener : public fastdds::dds::DataReaderListener
{

public:

    /**
     * @brief Constructor
     */
    UserDataReaderListener(
            std::function<void(const std::string& data)> on_data_received,
            UserDataContext* ctx) noexcept;

    /**
     * @brief Actions to be performed when a new Data Message is received.
     * @param reader DataReader that received the data
     */
    void on_data_available(
            fastdds::dds::DataReader* reader) override;

protected:

    std::string serialize_user_data(
            fastdds::dds::DynamicData::_ref_type data,
            fastdds::dds::DynamicType::_ref_type type) const;

    std::function<void(const std::string& data)> on_data_received_;
    UserDataContext* ctx_;

};

} // namespace subscriber
} // namespace statistics_backend
} // namespace eprosima

#endif // FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__USER_DATA_READER_LISTENER_HPP