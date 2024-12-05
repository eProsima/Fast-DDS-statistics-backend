// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

/*!
 * @file CommunicationTypeObjectSupport.cxx
 * Source file containing the implementation to register the TypeObject representation of the described types in the IDL file
 *
 * This file was generated by the tool fastddsgen.
 */

#include "CommunicationTypeObjectSupport.hpp"

#include <mutex>
#include <string>

#include <fastcdr/xcdr/external.hpp>
#include <fastcdr/xcdr/optional.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/log/Log.hpp>
#include <fastdds/dds/xtypes/common.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>

#include "Communication.hpp"


using namespace eprosima::fastdds::dds::xtypes;

// TypeIdentifier is returned by reference: dependent structures/unions are registered in this same method
void register_Communication_type_identifier(
        TypeIdentifierPair& type_ids_Communication)
{

    ReturnCode_t return_code_Communication {eprosima::fastdds::dds::RETCODE_OK};
    return_code_Communication =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                    get_type_identifiers(
        "Communication", type_ids_Communication);
    if (eprosima::fastdds::dds::RETCODE_OK != return_code_Communication)
    {
        StructTypeFlag struct_flags_Communication = TypeObjectUtils::build_struct_type_flag(
            eprosima::fastdds::dds::xtypes::ExtensibilityKind::APPENDABLE,
            false, false);
        QualifiedTypeName type_name_Communication = "Communication";
        eprosima::fastcdr::optional<AppliedBuiltinTypeAnnotations> type_ann_builtin_Communication;
        eprosima::fastcdr::optional<AppliedAnnotationSeq> ann_custom_Communication;
        AppliedAnnotationSeq tmp_ann_custom_Communication;
        eprosima::fastcdr::optional<AppliedVerbatimAnnotation> verbatim_Communication;
        if (!tmp_ann_custom_Communication.empty())
        {
            ann_custom_Communication = tmp_ann_custom_Communication;
        }

        CompleteTypeDetail detail_Communication = TypeObjectUtils::build_complete_type_detail(
            type_ann_builtin_Communication, ann_custom_Communication, type_name_Communication.to_string());
        CompleteStructHeader header_Communication;
        header_Communication = TypeObjectUtils::build_complete_struct_header(TypeIdentifier(), detail_Communication);
        CompleteStructMemberSeq member_seq_Communication;
        {
            TypeIdentifierPair type_ids_index;
            ReturnCode_t return_code_index {eprosima::fastdds::dds::RETCODE_OK};
            return_code_index =
                    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                            get_type_identifiers(
                "_uint32_t", type_ids_index);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_index)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                        "index Structure member TypeIdentifier unknown to TypeObjectRegistry.");
                return;
            }
            StructMemberFlag member_flags_index = TypeObjectUtils::build_struct_member_flag(
                eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                false, false, false, false);
            MemberId member_id_index = 0x00000000;
            bool common_index_ec {false};
            CommonStructMember common_index {TypeObjectUtils::build_common_struct_member(member_id_index,
                                                     member_flags_index, TypeObjectUtils::retrieve_complete_type_identifier(
                                                         type_ids_index,
                                                         common_index_ec))};
            if (!common_index_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure index member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_index = "index";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_index;
            ann_custom_Communication.reset();
            CompleteMemberDetail detail_index = TypeObjectUtils::build_complete_member_detail(name_index,
                            member_ann_builtin_index,
                            ann_custom_Communication);
            CompleteStructMember member_index =
                    TypeObjectUtils::build_complete_struct_member(common_index, detail_index);
            TypeObjectUtils::add_complete_struct_member(member_seq_Communication, member_index);
        }
        {
            TypeIdentifierPair type_ids_message;
            ReturnCode_t return_code_message {eprosima::fastdds::dds::RETCODE_OK};
            return_code_message =
                    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->type_object_registry().
                            get_type_identifiers(
                "anonymous_string_unbounded", type_ids_message);

            if (eprosima::fastdds::dds::RETCODE_OK != return_code_message)
            {
                {
                    SBound bound = 0;
                    StringSTypeDefn string_sdefn = TypeObjectUtils::build_string_s_type_defn(bound);
                    if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                            TypeObjectUtils::build_and_register_s_string_type_identifier(string_sdefn,
                            "anonymous_string_unbounded", type_ids_message))
                    {
                        EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                                "anonymous_string_unbounded already registered in TypeObjectRegistry for a different type.");
                    }
                }
            }
            StructMemberFlag member_flags_message = TypeObjectUtils::build_struct_member_flag(
                eprosima::fastdds::dds::xtypes::TryConstructFailAction::DISCARD,
                false, false, false, false);
            MemberId member_id_message = 0x00000001;
            bool common_message_ec {false};
            CommonStructMember common_message {TypeObjectUtils::build_common_struct_member(member_id_message,
                                                       member_flags_message, TypeObjectUtils::retrieve_complete_type_identifier(
                                                           type_ids_message,
                                                           common_message_ec))};
            if (!common_message_ec)
            {
                EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION, "Structure message member TypeIdentifier inconsistent.");
                return;
            }
            MemberName name_message = "message";
            eprosima::fastcdr::optional<AppliedBuiltinMemberAnnotations> member_ann_builtin_message;
            ann_custom_Communication.reset();
            CompleteMemberDetail detail_message = TypeObjectUtils::build_complete_member_detail(name_message,
                            member_ann_builtin_message,
                            ann_custom_Communication);
            CompleteStructMember member_message = TypeObjectUtils::build_complete_struct_member(common_message,
                            detail_message);
            TypeObjectUtils::add_complete_struct_member(member_seq_Communication, member_message);
        }
        CompleteStructType struct_type_Communication = TypeObjectUtils::build_complete_struct_type(
            struct_flags_Communication, header_Communication, member_seq_Communication);
        if (eprosima::fastdds::dds::RETCODE_BAD_PARAMETER ==
                TypeObjectUtils::build_and_register_struct_type_object(struct_type_Communication,
                type_name_Communication.to_string(), type_ids_Communication))
        {
            EPROSIMA_LOG_ERROR(XTYPES_TYPE_REPRESENTATION,
                    "Communication already registered in TypeObjectRegistry for a different type.");
        }
    }
}
