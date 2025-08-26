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
 * @file ProxyDiscoveryInfo.hpp
 */

#ifndef FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__PROXY_DISCOVERY_INFO_HPP
#define FASTDDS_STATISTICS_BACKEND_SRC_CPP_SUBSCRIBER__PROXY_DISCOVERY_INFO_HPP

#include <fastdds/rtps/builtin/data/ParticipantBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/PublicationBuiltinTopicData.hpp>
#include <fastdds/rtps/builtin/data/SubscriptionBuiltinTopicData.hpp>
#include <fastdds/rtps/participant/ParticipantDiscoveryInfo.hpp>
#include <fastdds/rtps/reader/ReaderDiscoveryStatus.hpp>
#include <fastdds/rtps/writer/WriterDiscoveryStatus.hpp>

#include <database/database_queue.hpp>

namespace eprosima {
namespace statistics_backend {
namespace subscriber {

using EntityDiscoveryInfo = database::EntityDiscoveryInfo;

std::string get_address(
        const fastdds::rtps::ParticipantBuiltinTopicData& info);

EntityDiscoveryInfo get_discovery_info(
        const EntityId& domain_of_discoverer,
        const fastdds::rtps::ParticipantBuiltinTopicData& participant_data,
        const fastdds::rtps::ParticipantDiscoveryStatus& reason,
        const DiscoverySource& discovery_source
    );

EntityDiscoveryInfo get_discovery_info(
        const EntityId& domain_of_discoverer,
        const fastdds::rtps::SubscriptionBuiltinTopicData& reader_data,
        const fastdds::rtps::ReaderDiscoveryStatus& reason,
        const DiscoverySource&  discovery_source
    );

EntityDiscoveryInfo get_discovery_info(
        const EntityId& domain_of_discoverer,
        const fastdds::rtps::PublicationBuiltinTopicData& writer_data,
        const fastdds::rtps::WriterDiscoveryStatus& reason,
        const DiscoverySource& discovery_source
    );

} // namespace subscriber
} // namespace statistics_backend
} // namespace eprosima

#endif
