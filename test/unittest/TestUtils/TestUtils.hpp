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

#include <fstream>
#include <string>

#include <fastdds_statistics_backend/nlohmann-json/json.hpp>

constexpr const char* DOMAIN_VIEW_GRAPH_TWO_PARTICIPANTS_DUMP_FILE =
        "../Resources/domain_view_graph_two_participants.json";

constexpr const char* DOMAIN_VIEW_GRAPH_UNDISCOVER_PARTICIPANT_DUMP_FILE =
        "../Resources/domain_view_graph_undiscover_participant.json";

constexpr const char* DOMAIN_VIEW_GRAPH_UNDISCOVER_ENDPOINT_DUMP_FILE =
        "../Resources/domain_view_graph_undiscover_endpoint.json";

constexpr const char* DOMAIN_VIEW_GRAPH_EMPTY_DUMP_FILE =
        "../Resources/domain_view_graph_empty.json";

constexpr const char* DOMAIN_VIEW_GRAPH_HOST_INSERT_FAILURE_DUMP_FILE =
        "../Resources/domain_view_graph_host_insert_failure.json";

constexpr const char* DOMAIN_VIEW_GRAPH_USER_INSERT_FAILURE_DUMP_FILE =
        "../Resources/domain_view_graph_user_insert_failure.json";

constexpr const char* DOMAIN_VIEW_GRAPH_PROCESS_INSERT_FAILURE_DUMP_FILE =
        "../Resources/domain_view_graph_process_insert_failure.json";

constexpr const char* DESCRIPTION_TAG = "description";

/**
 * Load a .json file, returning a dump of it.
 * Also remove meta information not necessary on dump.
 */

void load_file(
        std::string filename,
        nlohmann::json& dump)
{
    // Check if the file exists
    std::ifstream file(filename);
    if (!file.good())
    {
        FAIL() << "File " + filename + " does not exist";
    }

    // Get the json
    file >> dump;

    // Erase the description tag if existing
    if (dump.contains(DESCRIPTION_TAG))
    {
        dump.erase(DESCRIPTION_TAG);
    }
}
