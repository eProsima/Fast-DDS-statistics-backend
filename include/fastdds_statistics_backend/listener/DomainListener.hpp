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
 * @file DomainListener.hpp
 */

#ifndef FASTDDS_STATISTICS_BACKEND_LISTENER__DOMAINLISTENER_HPP
#define FASTDDS_STATISTICS_BACKEND_LISTENER__DOMAINLISTENER_HPP

#include <fastdds_statistics_backend/fastdds_statistics_backend_dll.h>
#include <fastdds_statistics_backend/types/types.hpp>
#include <fastdds_statistics_backend/types/Alerts.hpp>

#include <cstdint>

namespace eprosima {
namespace statistics_backend {

class FASTDDS_STATISTICS_BACKEND_DllAPI DomainListener
{
public:

    struct Status
    {
        /**
         * @brief Total cumulative count of the entities discovered so far
         *
         * This value increases monotonically with every new discovered entity.
         */
        int32_t total_count = 0;

        /**
         * @brief The change in total_count since the last time the listener was called
         *
         * This value can be positive, negative or zero, depending on the entity being
         * discovered, undiscovered or only the QoS of the entity being changed
         * since the last time the listener was called.
         */
        int32_t total_count_change = 0;

        /**
         * @brief The number of currently discovered entities
         *
         * This value can only be positive or zero.
         */
        int32_t current_count = 0;

        /**
         * @brief The change in current_count since the last time the listener was called
         *
         * This value can be positive, negative or zero, depending on the entity being
         * discovered, undiscovered or only the QoS of the entity being changed
         * since the last time the listener was called.
         */
        int32_t current_count_change = 0;

        void on_instance_discovered()
        {
            ++total_count;
            ++total_count_change;
            ++current_count;
            ++current_count_change;
        }

        void on_instance_undiscovered()
        {
            if (current_count > 0)
            {
                --current_count;
                --current_count_change;
            }
        }

        void on_status_read()
        {
            current_count_change = 0;
            total_count_change = 0;

        }

    };

    /**
     * @brief Virtual destructor
     */
    virtual ~DomainListener() = default;

    /*!
     * This function is called when a new Topic is discovered by the library.
     *
     * @param domain_id Entity ID of the domain in which the topic has been discovered.
     * @param topic_id Entity ID of the discovered topic.
     * @param status The status of the discovered topic.
     */
    virtual void on_topic_discovery(
            EntityId domain_id,
            EntityId topic_id,
            const Status& status)
    {
        static_cast<void>(domain_id);
        static_cast<void>(topic_id);
        static_cast<void>(status);
    }

    /*!
     * This function is called when a new DomainParticipant is discovered by the library,
     * or a previously discovered DomainParticipant changes its QOS or is removed.
     *
     * @param domain_id Entity ID of the domain in which the DataReader has been discovered.
     * @param participant_id Entity ID of the discovered DomainParticipant.
     * @param status The status of the discovered DomainParticipants.
     */
    virtual void on_participant_discovery(
            EntityId domain_id,
            EntityId participant_id,
            const Status& status)
    {
        static_cast<void>(domain_id);
        static_cast<void>(participant_id);
        static_cast<void>(status);
    }

    /*!
     * This function is called when a new DataReader is discovered by the library,
     * or a previously discovered DataReader changes its QOS or is removed.
     *
     * @param domain_id Entity ID of the domain in which the DataReader has been discovered.
     * @param datareader_id Entity ID of the discovered DataReader.
     * @param status The status of the discovered DataReaders.
     */
    virtual void on_datareader_discovery(
            EntityId domain_id,
            EntityId datareader_id,
            const Status& status)
    {
        static_cast<void>(domain_id);
        static_cast<void>(datareader_id);
        static_cast<void>(status);
    }

    /*!
     * This function is called when a new DataWriter is discovered by the library,
     * or a previously discovered DataWriter changes its QOS or is removed.
     *
     * @param domain_id Entity ID of the domain in which the DataWriter has been discovered.
     * @param datawriter_id Entity ID of the discovered DataWriter.
     * @param status The status of the discovered DataWriters.
     */
    virtual void on_datawriter_discovery(
            EntityId domain_id,
            EntityId datawriter_id,
            const Status& status)
    {
        static_cast<void>(domain_id);
        static_cast<void>(datawriter_id);
        static_cast<void>(status);
    }

    /*!
     * This function is called when a new data sample is available.
     *
     * @param domain_id Entity ID of the domain to which the data belongs.
     * @param entity_id Entity ID of the entity to which the data refers.
     * @param data_kind Data kind of the received data.
     */
    virtual void on_data_available(
            EntityId domain_id,
            EntityId entity_id,
            DataKind data_kind)
    {
        static_cast<void>(domain_id);
        static_cast<void>(entity_id);
        static_cast<void>(data_kind);
    }

    /*!
     * This function is called when the database domain view graph is updated.
     *
     * @param domain_id EntityId of the domain whose graph has been updated.
     */
    virtual void on_domain_view_graph_update(
            const EntityId& domain_id)
    {
        static_cast<void>(domain_id);
    }

    /*!
     * This function is called when a new monitor service data sample is available.
     *
     * @param domain_id Entity ID of the domain to which the data belongs.
     * @param entity_id Entity ID of the entity to which the data refers.
     * @param status_kind Status kind of the received data.
     */
    virtual void on_status_reported(
            EntityId domain_id,
            EntityId entity_id,
            StatusKind status_kind)
    {
        static_cast<void>(domain_id);
        static_cast<void>(entity_id);
        static_cast<void>(status_kind);

    }


    /*!
     * This function is called when a new alert must be reported.
     *
     * @param domain_id Entity ID of the domain to which the data belongs.
     * @param entity_id Entity ID of the entity to which the data refers.
     * @param alert_kind Alert kind of the received data.
     */
    virtual void on_alert_reported(
            EntityId domain_id,
            EntityId entity_id,
            AlertKind alert_kind)
    {
        static_cast<void>(domain_id);
        static_cast<void>(entity_id);
        static_cast<void>(alert_kind);

    }


};

} // namespace statistics_backend
} // namespace eprosima

#endif // FASTDDS_STATISTICS_BACKEND_LISTENER__DOMAINLISTENER_HPP
