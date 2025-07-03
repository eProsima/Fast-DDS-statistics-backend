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

/*!
 * @file
 * This file contains the required classes to keep a TypeObject/TypeIdentifier registry.
 */

#ifndef FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION__TYPEOBJECTREGISTRY_HPP
#define FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION__TYPEOBJECTREGISTRY_HPP

#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <fastcdr/xcdr/optional.hpp>

#include <fastdds/dds/xtypes/dynamic_types/DynamicType.hpp>
#include <fastdds/dds/xtypes/dynamic_types/Types.hpp>
#include <fastdds/dds/xtypes/type_representation/ITypeObjectRegistry.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObject.hpp>
#include <fastdds/dds/xtypes/type_representation/TypeObjectUtils.hpp>
#include <fastdds/fastdds_dll.hpp>

namespace eprosima {
namespace fastdds {
namespace dds {

namespace xtypes {

using ReturnCode_t = eprosima::fastdds::dds::ReturnCode_t;

// Class which holds the TypeObject registry, including every TypeIdentifier (plain and non-plain types), every
// non-plain TypeObject and the non-plain TypeObject serialized sizes.
class TypeObjectRegistry : public ITypeObjectRegistry
{

public:

    ReturnCode_t register_type_object(
            const std::string&,
            const CompleteTypeObject&,
            TypeIdentifierPair&) override
    {
        return eprosima::fastdds::dds::RETCODE_OK;
    }

    ReturnCode_t register_type_object(
            const TypeObject&,
            TypeIdentifierPair&) override
    {
        return eprosima::fastdds::dds::RETCODE_OK;
    }

    ReturnCode_t register_type_identifier(
            const std::string&,
            TypeIdentifierPair&) override
    {
        return eprosima::fastdds::dds::RETCODE_OK;
    }

    ReturnCode_t get_complete_type_object(
            const TypeIdentifierPair&,
            CompleteTypeObject&) override
    {
        return eprosima::fastdds::dds::RETCODE_OK;
    }

    ReturnCode_t get_type_objects(
            const std::string&,
            TypeObjectPair&) override
    {
        return eprosima::fastdds::dds::RETCODE_OK;
    }

    ReturnCode_t get_type_identifiers(
            const std::string&,
            TypeIdentifierPair&) override
    {
        return eprosima::fastdds::dds::RETCODE_OK;
    }

    ReturnCode_t get_type_object(
            const TypeIdentifier&,
            TypeObject&) override
    {
        return eprosima::fastdds::dds::RETCODE_OK;
    }

    ReturnCode_t get_complete_type_object(
            const TypeIdentifierPair&,
            CompleteTypeObject&) override
    {
        return eprosima::fastdds::dds::RETCODE_OK;
    }

    ReturnCode_t get_type_information(
            const TypeIdentifierPair&,
            TypeInformation&,
            bool) override
    {
        return eprosima::fastdds::dds::RETCODE_OK;
    }

    ReturnCode_t register_typeobject_w_dynamic_type(
            const DynamicType::_ref_type&,
            TypeIdentifierPair&) override
    {
        return eprosima::fastdds::dds::RETCODE_OK;
    }

};

} // namespace xtypes
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION__TYPEOBJECTREGISTRY_HPP
