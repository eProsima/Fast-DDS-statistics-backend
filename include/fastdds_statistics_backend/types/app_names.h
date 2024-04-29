// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file app_names.h
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_APP_NAMES_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_APP_NAMES_HPP_

#include <fastdds_statistics_backend/types/types.hpp>

namespace eprosima {
namespace statistics_backend {

////////////////////////
// DATABASE DUMP TAGS //
////////////////////////

/////
// App id

//! Identificator for a Fast DDS Statistics Backend app
constexpr const char* FASTDDS_MONITOR_APP           = "FASTDDS_MONITOR";
//! Identificator for a DDS Router app
constexpr const char* DDS_ROUTER_APP                = "DDS_ROUTER";
//! Identificator for a Shapes Demo app
constexpr const char* SHAPES_DEMO_APP               = "SHAPES_DEMO";
//! Identificator for a Integration Service app
constexpr const char* INTEGRATION_SERVICE_APP       = "INTEGRATION_SERVICE";
//! Identificator for a Fast DDS Visualizer app
constexpr const char* FASTDDS_VISUALIZER_APP        = "FASTDDS_VISUALIZER";
//! Identificator for a Fast DDS Spy app
constexpr const char* FASTDDS_SPY_APP               = "FASTDDS_SPY";
//! Identificator for a DDS Recorder app
constexpr const char* DDS_RECORDER_APP              = "DDS_RECORDER";
//! Identificator for a DDS Replayer app
constexpr const char* DDS_REPLAYER_APP              = "DDS_REPLAYER";
//! Identificator for a AML-IP app
constexpr const char* AML_IP_APP                    = "AML_IP";
//! Identificator for a SustainML app
constexpr const char* SUSTAINML_APP                 = "SUSTAINML";

//! Conversion from AppId to string
constexpr const char* app_id_str[] =
{"UNKNOWN_APP", FASTDDS_MONITOR_APP, DDS_ROUTER_APP, SHAPES_DEMO_APP, INTEGRATION_SERVICE_APP, FASTDDS_VISUALIZER_APP,
 FASTDDS_SPY_APP, DDS_RECORDER_APP, DDS_REPLAYER_APP, AML_IP_APP, SUSTAINML_APP};

//! Conversion from string to AppId
const std::map<std::string, AppId> app_id_enum = {
    {FASTDDS_MONITOR_APP, AppId::FASTDDS_MONITOR},
    {DDS_ROUTER_APP, AppId::DDS_ROUTER},
    {SHAPES_DEMO_APP, AppId::SHAPES_DEMO},
    {INTEGRATION_SERVICE_APP, AppId::INTEGRATION_SERVICE},
    {FASTDDS_VISUALIZER_APP, AppId::FASTDDS_VISUALIZER},
    {FASTDDS_SPY_APP, AppId::FASTDDS_SPY},
    {DDS_RECORDER_APP, AppId::DDS_RECORDER},
    {DDS_REPLAYER_APP, AppId::DDS_REPLAYER},
    {AML_IP_APP, AppId::AML_IP},
    {SUSTAINML_APP, AppId::SUSTAINML}
};

} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_APP_NAMES_HPP_
