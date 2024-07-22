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

    /**
     * @brief Register a local TypeObject.
     *        The MinimalTypeObject is generated from the CompleteTypeObject, and both are registered into the registry
     *        with the corresponding TypeIdentifiers and TypeObject serialized sizes.
     *
     * @pre type_name must not be empty.
     * @pre complete_type_object must be consistent (only checked in Debug build mode).
     *
     * @param [in] type_name Name of the type being registered.
     * @param [in] complete_type_object CompleteTypeObject related to the given type name.
     * @param [out] type_ids @ref TypeIdentifierPair corresponding to the CompleteTypeObject just registered and the
     * generated MinimalTypeObject.
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeObject registered with the
     *                      given type_name.
     *                      RETCODE_PRECONDITION_NOT_MET if the given type_name is empty or if the type_object
     *                      is inconsistent.
     */
    ReturnCode_t register_type_object(
            const std::string& type_name,
            const CompleteTypeObject& complete_type_object,
            TypeIdentifierPair& type_ids) override;

    /**
     * @brief Register a remote TypeObject.
     *        This auxiliary method might register only the minimal TypeObject and TypeIdentifier or register both
     *        TypeObjects constructing the minimal from the complete TypeObject information.
     *        TypeObject consistency is not checked in this method as the order of the dependencies received by the
     *        TypeLookupService is not guaranteed.
     *        The consistency is checked by the TypeLookupService after all dependencies are registered.
     *
     * @pre @ref TypeIdentifierPair::type_identifier1 discriminator must match TypeObject discriminator or be TK_NONE.
     *      @ref TypeIdentifierPair::type_identifier1 consistency is only checked in Debug build mode.
     *
     * @param [in] type_object Related TypeObject being registered.
     * @param [in,out] type_ids Returns the registered @ref TypeIdentifierPair.
     * @ref TypeIdentifierPair::type_identifier1 might be TK_NONE.
     * In other case this function will check it is consistence with the provided @TypeObject.
     * @return ReturnCode_t RETCODE_OK if correctly registered.
     *                      RETCODE_PRECONDITION_NOT_MET if the discriminators differ.
     *                      RETCODE_PRECONDITION_NOT_MET if the TypeIdentifier is not consistent with the given
     *                      TypeObject.
     */
    ReturnCode_t register_type_object(
            const TypeObject& type_object,
            TypeIdentifierPair& type_ids) override;

    /**
     * @brief Register an indirect hash TypeIdentifier.
     *
     * @pre TypeIdentifier must not be a direct hash TypeIdentifier.
     * @pre TypeIdentifier must be consistent (only checked in Debug build mode).
     * @pre type_name must not be empty.
     *
     * @param [in] type_name Name of the type being registered.
     * @param [inout] type_identifier @ref TypeIdentifierPair related to the given type name. It must be set in
     * @ref TypeIdentifierPair::type_identifier1. At the end this object is filled with both TypeIdentifiers.
     * @return ReturnCode_t RETCODE_OK if correctly registered in TypeObjectRegistry.
     *                      RETCODE_BAD_PARAMETER if there is already another different TypeIdentifier registered with
     *                      the given type_name.
     *                      RETCODE_PRECONDITION_NOT_MET if the given TypeIdentifier is inconsistent or a direct hash
     *                      TypeIdentifier or if the given type_name is empty.
     */
    ReturnCode_t register_type_identifier(
            const std::string& type_name,
            TypeIdentifierPair& type_identifier) override;

    /**
     * @brief Get the TypeObjects related to the given type name.
     *
     * @pre type_name must not be empty.
     *
     * @param [in] type_name Name of the type being queried.
     * @param [out] type_objects Both complete and minimal TypeObjects related with the given type_name.
     * @return ReturnCode_t RETCODE_OK if the TypeObjects are found in the registry.
     *                      RETCODE_NO_DATA if the given type_name has not been registered.
     *                      RETCODE_BAD_PARAMETER if the type_name correspond to a indirect hash TypeIdentifier.
     *                      RETCODE_PRECONDITION_NOT_MET if the type_name is empty.
     */
    ReturnCode_t get_type_objects(
            const std::string& type_name,
            TypeObjectPair& type_objects) override;

    /**
     * @brief Get the TypeIdentifiers related to the given type name.
     *
     * @pre type_name must not be empty.
     *
     * @param [in] type_name Name of the type being queried.
     * @param [out] type_identifiers For direct hash TypeIdentifiers, both minimal and complete TypeIdentifiers are
     *                              returned.
     *                              For indirect hash TypeIdentifiers, only the corresponding TypeIdentifier is returned
     * @return ReturnCode_t RETCODE_OK if the TypeIdentifiers are found in the registry.
     *                      RETCODE_NO_DATA if the type_name has not been registered.
     *                      RETCODE_PRECONDITION_NOT_MET if the type_name is empty.
     */
    ReturnCode_t get_type_identifiers(
            const std::string& type_name,
            TypeIdentifierPair& type_identifiers) override;

    /**
     * @brief Get the TypeObject related to the given TypeIdentifier.
     *
     * @pre TypeIdentifier must be a direct hash TypeIdentifier.
     *
     * @param [in] type_identifier TypeIdentifier being queried.
     * @param [out] type_object TypeObject related with the given TypeIdentifier.
     * @return ReturnCode_t RETCODE_OK if the TypeObject is found within the registry.
     *                      RETCODE_NO_DATA if the given TypeIdentifier is not found in the registry.
     *                      RETCODE_PRECONDITION_NOT_MET if the TypeIdentifier is not a direct hash.
     */
    ReturnCode_t get_type_object(
            const TypeIdentifier& type_identifier,
            TypeObject& type_object) override;

    /**
     * @brief Build the TypeInformation related to the provided @ref TypeIdentifierPair.
     *
     * @pre type_ids must not be empty. At least @ref TypeIdentifierPair::type_identifier1 must be filled.
     *
     * @param [in] type_ids @ref TypeIdentifierPair which type information is queried.
     * @param [out] type_information Related TypeInformation for the given @ref TypeIdentifier.
     * @param [in] with_dependencies
     * @return ReturnCode_t RETCODE_OK if the type_ids are found within the registry.
     *                      RETCODE_NO_DATA if the given type_ids is not found.
     *                      RETCODE_BAD_PARAMETER if the given @ref TypeIdentifier corresponds to a indirect hash TypeIdentifier.
     *                      RETCODE_PRECONDITION_NOT_MET if any type_ids is empty.
     */
    ReturnCode_t get_type_information(
            const TypeIdentifierPair& type_ids,
            TypeInformation& type_information,
            bool with_dependencies = false) override;

    /**
     * @brief Register DynamicType TypeObject.
     *
     * @param [in] dynamic_type DynamicType to be registered.
     * @param [out] type_ids @ref TypeIdentifierPair corresponding to the registered DynamicType TypeObject.
     * @return ReturnCode_t RETCODE_OK always.
     */
    ReturnCode_t register_typeobject_w_dynamic_type(
            const DynamicType::_ref_type& dynamic_type,
            TypeIdentifierPair& type_ids) override;

};

} // namespace xtypes
} // namespace dds
} // namespace fastdds
} // namespace eprosima

#endif // FASTDDS_DDS_XTYPES_TYPE_REPRESENTATION__TYPEOBJECTREGISTRY_HPP
