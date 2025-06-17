// Copyright 2025 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file WireProtocolConfigTags.ipp
 */

static constexpr const char* wire_protocol_config_tag = "wire_protocol_config";
static constexpr const char* guid_prefix_tag = "guid_prefix";

// BuiltinAttributes related tags
static constexpr const char* builtin_attributes_tag = "builtin_attributes";
static constexpr const char* discovery_config_tag = "discovery_config";
static constexpr const char* discovery_protocol_tag = "discovery_protocol";
static constexpr const char* discovery_protocol_none_tag = "NONE";
static constexpr const char* discovery_protocol_simple_tag = "SIMPLE";
static constexpr const char* discovery_protocol_external_tag = "EXTERNAL";
static constexpr const char* discovery_protocol_client_tag = "CLIENT";
static constexpr const char* discovery_protocol_server_tag = "SERVER";
static constexpr const char* discovery_protocol_backup_tag = "BACKUP";
static constexpr const char* discovery_protocol_super_client_tag = "SUPER_CLIENT";
static constexpr const char* use_simple_edp_tag = "use_simple_edp";
static constexpr const char* use_static_edp_tag = "use_static_edp";
static constexpr const char* initial_announcements_tag = "initial_announcements";
static constexpr const char* initial_announcements_count_tag = "count";
static constexpr const char* initial_announcements_period_tag = "period";
static constexpr const char* simple_edp_attributes_tag = "simple_edp_attributes";
static constexpr const char* simple_edp_use_pub_writer_sub_reader_tag = "use_PublicationWriterANDSubscriptionReader";
static constexpr const char* simple_edp_use_pub_reader_sub_writer_tag = "use_PublicationReaderANDSubscriptionWriter";
static constexpr const char* simple_edp_enable_builtin_secure_pub_writer_sub_reader_tag =
        "enable_builtin_secure_publications_writer_and_subscriptions_reader";
static constexpr const char* simple_edp_enable_builtin_secure_sub_writer_pub_reader_tag =
        "enable_builtin_secure_subscriptions_writer_and_publications_reader";
static constexpr const char* discovery_server_client_syncperiod_tag = "discoveryServer_client_sync_period";
static constexpr const char* discovery_servers_tag = "discovery_servers";
static constexpr const char* participant_filtering_tag = "ignore_participant_flags";
static constexpr const char* participant_filtering_no_filter_tag = "NO_FILTER";
static constexpr const char* participant_filtering_filter_different_host_tag = "FILTER_DIFFERENT_HOST";
static constexpr const char* participant_filtering_filter_different_process_tag = "FILTER_DIFFERENT_PROCESS";
static constexpr const char* participant_filtering_filter_same_process_tag = "FILTER_SAME_PROCESS";
static constexpr const char* static_edp_xml_config_tag = "static_edp_xml_config";
static constexpr const char* use_writer_liveliness_protocol_tag = "use_writer_liveliness_protocol";
static constexpr const char* network_configuration_tag = "network_configuration";
static constexpr const char* initial_peers_tag = "initial_peers";
static constexpr const char* reader_history_memory_policy_tag = "reader_history_memory_policy";
static constexpr const char* reader_payload_size_tag = "reader_payload_size";
static constexpr const char* writer_history_memory_policy_tag = "writer_history_memory_policy";
static constexpr const char* writer_payload_size_tag = "writer_payload_size";
static constexpr const char* mutation_tries_tag = "mutation_tries";
static constexpr const char* avoid_builtin_multicast_tag = "avoid_builtin_multicast";
static constexpr const char* port_parameters_tag = "port";
static constexpr const char* port_parameters_port_base_tag = "port_base";
static constexpr const char* port_parameters_domain_id_gain_tag = "domain_id_gain";
static constexpr const char* port_parameters_participant_id_gain_tag = "participant_id_gain";
static constexpr const char* port_parameters_offsetd0_tag = "offset_d0";
static constexpr const char* port_parameters_offsetd1_tag = "offset_d1";
static constexpr const char* port_parameters_offsetd2_tag = "offset_d2";
static constexpr const char* port_parameters_offsetd3_tag = "offset_d3";
static constexpr const char* port_parameters_offsetd4_tag = "offset_d4";
static constexpr const char* easy_mode_ip_tag = "easy_mode_ip";
