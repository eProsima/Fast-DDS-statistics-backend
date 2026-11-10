// Copyright 2026 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

// Snippets for the Fast DDS Statistics Backend Pro documentation pages.
// These use Pro-only API that is not available in this repository, so this file is NOT compiled
// by the documentation tests (see docs/test/CMakeLists.txt).

#include <fastdds_statistics_backend/StatisticsBackend.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>
#include <fastdds_statistics_backend/types/types.hpp>

#include <map>
#include <string>
#include <vector>

using namespace eprosima::statistics_backend;

void statistics_control_examples()
{
    EntityId monitor_id;
    {
        //CONF-PRO-STATISTICS-READERS-EXAMPLE
        StatisticsBackend::enable_statistics_reader(monitor_id, "_fastdds_statistics_publication_throughput");
        auto active = StatisticsBackend::get_enabled_statistics_readers(monitor_id);
        // ...
        StatisticsBackend::disable_statistics_reader(monitor_id, "_fastdds_statistics_publication_throughput");
        //!--
    }
    {
        std::vector<EntityId> source_ids;
        std::vector<EntityId> target_ids;
        std::vector<EntityId> entity_ids;
        Timestamp t_from;
        Timestamp t_to;

        //CONF-PRO-GET-DATA-MULTIPLE-STATISTICS-EXAMPLE
        // Aggregated (source, target) overload:
        std::map<StatisticKind, std::vector<StatisticsData>> by_kind = StatisticsBackend::get_data(
            DataKind::FASTDDS_LATENCY,
            source_ids, target_ids,
            /* bins */ 0,
            t_from, t_to,
            {StatisticKind::MEAN, StatisticKind::MAX, StatisticKind::MIN});

        // Single-entity overload:
        std::map<StatisticKind, std::vector<StatisticsData>> by_kind_single = StatisticsBackend::get_data(
            DataKind::PUBLICATION_THROUGHPUT,
            entity_ids,
            /* bins */ 0,
            t_from, t_to,
            {StatisticKind::MEAN});
        //!--
    }
}

void type_registration_examples()
{
    std::string idl_text;
    {
        //CONF-PRO-REGISTER-TYPE-EXAMPLE
        std::string error_message;
        auto status = eprosima::statistics_backend::StatisticsBackend::register_type(
            "ShapeType",        // type_name
            idl_text,           // idl
            error_message,
            {},                 // aux_files
            "");                // struct_name
        //!--
    }
}

void topic_data_interaction_examples()
{
    EntityId monitor_id;
    std::string json_sample;
    {
        //CONF-PRO-TOPIC-SPY-TIMESTAMP-EXAMPLE
        StatisticsBackend::start_topic_spy(monitor_id, "Square",
                [](const std::string& data, std::int64_t source_timestamp_ns)
                {
                    // data, source_timestamp_ns
                });
        // ...
        StatisticsBackend::stop_topic_spy(monitor_id, "Square");
        //!--
    }
    {
        //CONF-PRO-TOPIC-PUBLISHER-EXAMPLE
        StatisticsBackend::start_topic_publisher(monitor_id, "Square");
        StatisticsBackend::publish_topic_sample(monitor_id, "Square", json_sample);
        // ...
        StatisticsBackend::stop_topic_publisher(monitor_id, "Square");
        //!--
    }
    {
        //CONF-PRO-TOPIC-SPY-RAW-EXAMPLE
        StatisticsBackend::start_topic_spy_raw(monitor_id, "CameraFeed",
                [](const RawImageSample& sample)
                {
                    // sample.data, sample.width, sample.height, sample.step, sample.encoding, sample.format
                });
        // ...
        StatisticsBackend::stop_topic_spy_raw(monitor_id, "CameraFeed");
        //!--
    }
}

void licensing_and_safety_examples()
{
    DomainId domain_id = 0;
    {
        //CONF-PRO-CHECK-LICENSE-EXAMPLE
        if (!eprosima::statistics_backend::check_license())
        {
            // No valid Fast DDS Pro or Safe DDS license was found.
        }
        //!--
    }
    {
        //CONF-PRO-HAS-PROXY-PARTICIPANTS-EXAMPLE
        if (eprosima::statistics_backend::StatisticsBackend::has_proxy_participants(domain_id))
        {
            // At least one PROXY or INFERRED participant exists in this domain.
        }
        //!--
    }
}
