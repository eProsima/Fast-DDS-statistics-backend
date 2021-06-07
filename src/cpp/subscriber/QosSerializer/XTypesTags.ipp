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
 * @file XTypesTags.ipp
 */

// Representation related tags
static constexpr const char* representation_tag = "representation";
static constexpr const char* representation_xcdr_tag = "XCDR_DATA_REPRESENTATION";
static constexpr const char* representation_xcdr2_tag = "XCDR2_DATA_REPRESENTATION";
static constexpr const char* representation_xml_tag = "XML_DATA_REPRESENTATION";

// Type consistency related tags
static constexpr const char* type_consistency_tag = "type_consistency";
static constexpr const char* type_consistency_coercion_allow_tag = "ALLOW_TYPE_COERCION";
static constexpr const char* type_consistency_coercion_disallow_tag = "DISALLOW_TYPE_COERCION";
static constexpr const char* ignore_sequence_bounds_tag = "ignore_sequence_bounds";
static constexpr const char* ignore_string_bounds_tag = "ignore_string_bounds";
static constexpr const char* ignore_member_names_tag = "ignore_member_names";
static constexpr const char* prevent_type_widening_tag = "prevent_type_widening";
static constexpr const char* force_type_validation_tag = "force_type_validation";
