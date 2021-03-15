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
 * @file StatisticsBackend.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATISTICSBACKEND_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATISTICSBACKEND_HPP_

#include <listener/DomainListener.hpp>
#include <listener/PhysicalListener.hpp>
#include <listener/CallbackMask.hpp>
#include <types/types.hpp>

namespace eprosima {
namespace statistics_backend {

class StatisticsBackend
{

public:

    /**
     * @brief Type of the data returned by the backend.
     * 
     * The first field represents the time at which the data was recorded.
     * This can be the time of the raw data point if no bins are being used,
     * or the starting time of the bin (see get_data()).
     * 
     * The second field represents the data value itself.
     * This will be the value of the calculated statistic, or the raw data
     * if no statistic has been requested (see get_data()).
     * 
     * \sa get_data()
     * 
     */
    using StatisticData = std::pair<time_t, double>;

    /**
     * Enumeration of available statistics operation to be perform on the raw data.
     */
    enum StatisticKind
    {
        /// No statistic, use raw data samples
        NONE,

        /// Numerical mean of the data samples
        MEAN,

        /// Standard Deviation of the data samples
        STANDARD_DEVIATION,
        
        /// Maximum value of the data samples
        MAX,
        
        /// Minimum value of the data samples
        MIN,
        
        /// Median value of the data samples
        MEDIAN,
        
        /// Amount of data samples
        COUNT,
        
        /// Sum of the data sample values
        SUM
    };


    /**
     * @brief Set the listener for the physical domain events.
     * 
     * Any physical listener already configured will be replaced by the new one.
     * The provided pointer to the listener can be null, in which case,
     * any physical listener already configured will be removed.
     * 
     * @param listener the listener with the callback implementations.
     * @param callback_mask Mask of the callbacks. Only the events that have the mask bit set will be informed.
     */
    static void set_physical_listener(
        PhysicalListener* listener,
        CallbackMask callback_mask);

    /**
     * @brief Starts monitoring on a given domain
     * 
     * @param domain The domain ID of the DDS domain to monitor
     * @param domain_listener Listener with the callback to use to inform of events
     * @param callback_mask Mask of the callbacks. Only the events that have the mask bit set will be informed
     * @param data_mask Mask of the data types that will be monitored
     */
    static void init_monitor(
            DomainId domain,
            DomainListener* domain_listener = nullptr,
            CallbackMask callback_mask = CallbackKind::NONE,
            DataKindMask data_mask = DataKind::NONE);

    /**
     * @brief ????????
     * 
     * @param discovery_server_locators ????????
     * @param domain_listener Listener with the callback to use to inform of events
     * @param callback_mask Mask of the callbacks. Only the events that have the mask bit set will be informed
     * @param data_mask Mask of the data types that will be monitored
     */
    static void init_monitor(
            std::string discovery_server_locators,
            DomainListener* domain_listener = nullptr,
            CallbackMask callback_mask = CallbackKind::NONE,
            DataKindMask data_mask = DataKind::NONE);

    /**
     * @brief Get all the entities of a given type related to another entity
     * 
     * @param entity_id The ID of the origin entity from we are searching
     * @param entity_type The type of entities we are looking for
     * @return All entities of type \c entity_type that are related to \c entity_id
     */
    static std::vector<EntityId> get_entities(
            EntityId entity_id,
            EntityKind entity_type);

    // Is still needed????????
    static EntityKind get_type(
            EntityId entity_id);

    /**
     * @brief Get the QoS of a given entity
     * 
     * @param entity_id The entity for which we want to retrieve the QoS
     * @return Qos object describing the entity's QoS
     */
    static Qos get_qos(
            EntityId entity_id);

    /**
     * @brief Get the name of a given entity
     * 
     * @param entity_id The entity for which we want to retrieve the name
     * @return a string representing the name of the entity
     */
    static std::string get_name(
            EntityId entity_id);

    // What is a summary?????????
    static StatisticsSummary get_summary(
            EntityId entity_id);

    /**
     * @brief Provides access to the data measured during the monitoring.
     * 
     * Use this method for data types that relate to two entities,
     * as described in DataType.
     * 
     * For data types that relate to a single entity,
     * use the overloaded method that takes a single entity as argument.
     * 
     * \par Measurement time and intervals
     * 
     * \c t_from and \c t_to define the time interval for which the measurements will be returned.
     * This time interval is further divided int \c bin segments of equal length,
     * and a measurement is returned for each segment.
     * 
     * If \c bin is zero, no statistic is calculated and the raw data values in the requested
     * time interval are returned
     * 
     * \par Statistics
     * 
     * The kind of statistic calculated for each \c bin segment is indicated by \c statistic.
     * In this implementation, if \c statistic is \c NONE, the first raw data point in the segment is returned.
     * 
     * \sa StatisticsBackend
     * 
     * @param data_type The type of the measure being requested
     * @param entity_id_source Id of the source entity of the requested data
     * @param entity_id_target Id of the target entity of the requested data
     * @param bins Number of time intervals in which the measurement time is divided
     * @param statistic Statistic to calculate for each of the bins
     * @param t_from starting time of the returned measures.
     * @param t_to ending time of the returned measures.
     * @return a vector of \c bin elements with the values of the requested statistic
     */
    static std::vector<StatisticData> get_data(
            DataKind data_type,
            EntityId entity_id_source,
            EntityId entity_id_target,
            uint16_t bins = 0,
            StatisticKind statistic = NONE,
            Time t_from = INIT_TIME,
            Time t_to = NOW_TIME);

    /**
     * @brief Provides access to the data measured during the monitoring.
     * 
     * Use this method for data types that relate to a single entity,
     * as described in DataType.
     * 
     * For data types that relate to two entities,
     * use the overloaded method that takes a source and a target entity as arguments.
     * 
     * \par Measurement time and intervals
     * 
     * \c t_from and \c t_to define the time interval for which the measurements will be returned.
     * This time interval is further divided int \c bin segments of equal length,
     * and a measurement is returned for each segment.
     * 
     * If \c bin is zero, no statistic is calculated and the raw data values in the requested
     * time interval are returned
     * 
     * \par Statistics
     * 
     * The kind of statistic calculated for each \c bin segment is indicated by \c statistic.
     * In this implementation, if \c statistic is \c NONE, the first raw data point in the segment is returned.
     * 
     * \sa StatisticsBackend
     * 
     * @param data_type The type of the measure being requested
     * @param entity_id Id of the entity of the requested data
     * @param bins Number of time intervals in which the measurement time is divided
     * @param statistic Statistic to calculate for each of the bins
     * @param t_from starting time of the returned measures.
     * @param t_to ending time of the returned measures.
     * @return a vector of \c bin elements with the values of the requested statistic
     */
    static std::vector<StatisticData> get_data(
            DataKind data_type,
            EntityId entity_id,
            uint16_t bins = 0,
            StatisticKind statistic = NONE,
            Time t_from = INIT_TIME,
            Time t_to = NOW_TIME);

protected:

    StatisticsBackend()
    {
    }

    /**
     * StatisticsBackend
     */
    static StatisticsBackend* get_instance()
    {
        static StatisticsBackend instance;
        return &instance;
    }


};

} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_STATISTICSBACKEND_HPP_
