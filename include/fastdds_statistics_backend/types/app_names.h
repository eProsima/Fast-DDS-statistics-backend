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

namespace eprosima {
namespace statistics_backend {

////////////////////////
// DATABASE DUMP TAGS //
////////////////////////

/////
// App id

//! Identificator for a Fast DDS Statistics Backend app
constexpr const char* FASTDDS_MONITOR_APP           = "FASTDDS MONITOR";
//! Identificator for a DDS Router app
constexpr const char* DDS_ROUTER_APP                = "DDS ROUTER";
//! Identificator for a Shapes Demo app
constexpr const char* SHAPES_DEMO_APP               = "SHAPES DEMO";
//! Identificator for a Integration Service app
constexpr const char* INTEGRATION_SERVICE_APP       = "INTEGRATION SERVICE";
//! Identificator for a Fast DDS Visualizer app
constexpr const char* FASTDDS_VISUALIZER_APP        = "FASTDDS VISUALIZER";
//! Identificator for a Fast DDS Spy app
constexpr const char* FASTDDS_SPY_APP               = "FASTDDS SPY";
//! Identificator for a DDS Recorder app
constexpr const char* DDS_RECORDER_APP              = "DDS RECORDER";
//! Identificator for a DDS Replayer app
constexpr const char* DDS_REPLAYER_APP              = "DDS REPLAYER";
//! Identificator for a AML-IP app
constexpr const char* AML_IP_APP                    = "AML-IP";

//! Conversion from AppId to string
constexpr const char* app_id_str[] =
{"UNKNOWN_APP", FASTDDS_MONITOR_APP, DDS_ROUTER_APP, SHAPES_DEMO_APP, INTEGRATION_SERVICE_APP, FASTDDS_VISUALIZER_APP,
 FASTDDS_SPY_APP, DDS_RECORDER_APP, DDS_REPLAYER_APP, AML_IP_APP};

} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_APP_NAMES_HPP_
