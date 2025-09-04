/* Copyright 2021 Proyectos y Sistemas de Mantenimiento SL (eProsima).
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file entities.hpp
 */

#ifndef FASTDDS_STATISTICS_BACKEND_SRC_CPP_DATABASE__ENTITIES_HPP
#define FASTDDS_STATISTICS_BACKEND_SRC_CPP_DATABASE__ENTITIES_HPP

#include <regex>
#include <string>

#include <fastdds_statistics_backend/types/types.hpp>
#include <fastdds_statistics_backend/types/EntityId.hpp>

#include <types/fragile_ptr.hpp>

#include "data.hpp"

namespace eprosima {
namespace statistics_backend {
namespace database {

struct User;
struct Process;
struct DomainParticipant;
struct Topic;
struct DataReader;
struct DataWriter;
struct Locator;

/*
 * Base struct for all the database possible entities.
 *
 * Entities constitute the nodes of the database. They hold the relation they have
 * both upwards and downwards, plus they hold the statistical data reported by Fast
 * DDS Statistics Module.
 */
struct Entity
{
    Entity(
            EntityKind entity_kind = EntityKind::INVALID,
            std::string entity_name = "INVALID",
            bool entity_metatraffic = false,
            bool entity_active = true,
            StatusLevel entity_status = StatusLevel::OK_STATUS,
            DiscoverySource entity_discovery_source = DiscoverySource::UNKNOWN) noexcept
        : kind(entity_kind)
        , name(normalize_entity_name(entity_name))
        , alias(normalize_entity_name(entity_name))
        , metatraffic(entity_metatraffic)
        , active(entity_active)
        , status(entity_status)
        , discovery_source(entity_discovery_source)
    {
    }

    virtual ~Entity() = default;

    /**
     * Clear the maps and data
     */
    virtual void clear()
    {
    }

    /**
     * @brief Check whether a specific topic corresponds to a metatraffic topic.
     *
     * @param topic_name Name of the topic to check.
     * @return true if metatraffic topic, false otherwise.
     */
    static bool is_metatraffic_topic(
            std::string topic_name);

    /**
     * @brief Normalize the entity name to an ascii-valid name.
     *
     * @param entity_name Name of the name to convert.
     * @return The normalized string name.
     */
    static std::string normalize_entity_name(
            const std::string& entity_name);

    /**
     * @brief Check whether the entity is a DDS entity, i.e: derived from the DDSEntity base class.
     */
    bool is_dds_entity() const
    {
        return kind == EntityKind::DATAWRITER ||
               kind == EntityKind::DATAREADER ||
               kind == EntityKind::PARTICIPANT;
    }

    //! The unique identification of the entity
    EntityId id;

    //! The kind of entity
    EntityKind kind;

    //! A human-readable name for the entity
    std::string name;

    //! A user defined name for the entity
    std::string alias;

    //! Flag to signal that this entity is related to a topic used to share meta traffic data.
    bool metatraffic;

    //! Active means that there is statistical data being reported within the entity.
    bool active;

    //! The status of entity
    StatusLevel status;

    //! The source of discovery for this entity
    DiscoverySource discovery_source;
};

/*
 * Host entities hold data about the real host involved in the communication
 */
struct Host : Entity
{
    Host(
            std::string host_name,
            StatusLevel status = StatusLevel::OK_STATUS) noexcept
        : Entity(EntityKind::HOST, host_name, false, false, status)
    {
    }

    /**
     * Clear the maps and data
     */
    void clear() final;

    /*
     * Collection of users within the host which are involved in the communication.
     * The collection is ordered by the EntityId of the user nodes.
     */
    std::map<EntityId, details::fragile_ptr<User>> users;
};

/*
 * User entities hold data about the host users involved in the communication
 */
struct User : Entity
{
    User(
            std::string user_name,
            details::fragile_ptr<Host> user_host,
            StatusLevel status = StatusLevel::OK_STATUS) noexcept
        : Entity(EntityKind::USER, user_name, false, false, status)
        , host(user_host)
    {
    }

    /**
     * Clear the maps and data
     */
    void clear() final;

    //! Reference to the Host in which this user runs
    details::fragile_ptr<Host> host;

    /*
     * Collection of processes within the host which are involved in the communication.
     * The collection is ordered by the EntityId of the process nodes.
     */
    std::map<EntityId, details::fragile_ptr<Process>> processes;
};

/*
 * Process entities hold data about the computing processes involved in the communication.
 */
struct Process : Entity
{
    Process(
            std::string process_name,
            std::string process_id,
            details::fragile_ptr<User> process_user,
            StatusLevel status = StatusLevel::OK_STATUS) noexcept
        : Entity(EntityKind::PROCESS, process_name, false, false, status)
        , pid(process_id)
        , user(process_user)
    {
    }

    /**
     * Clear the maps and data
     */
    void clear() final;

    //! The PID of the process
    std::string pid;

    //! Reference to the User in which this process runs.
    details::fragile_ptr<User> user;

    /*
     * Collection of DomainParticipant within the process which are involved in the communication.
     * The collection is ordered by the EntityId of the DomainParticipant nodes.
     */
    std::map<EntityId, details::fragile_ptr<DomainParticipant>> participants;
};

/*
 * Domain entities hold data about the DDS domains or Discovery Server networks involved in the
 * communication.
 */
struct Domain : Entity
{
    Domain(
            std::string domain_name,
            StatusLevel status = StatusLevel::OK_STATUS) noexcept
        : Entity(EntityKind::DOMAIN, domain_name, false, true, status)
    {
    }

    /**
     * Clear the maps and data
     */
    void clear() final;

    /*
     * Collection of Topics within the Domain which are either published, subscribed, or both.
     * The collection is ordered by the EntityId of the Topic nodes.
     */
    std::map<EntityId, details::fragile_ptr<Topic>> topics;

    /*
     * Collection of DomainParticipant within the Domain which are involved in the communication.
     * The collection is ordered by the EntityId of the DomainParticipant nodes.
     */
    std::map<EntityId, details::fragile_ptr<DomainParticipant>> participants;
};

/*
 * Base class for the DDS Entities.
 */
struct DDSEntity : Entity
{
    DDSEntity(
            EntityKind entity_kind = EntityKind::INVALID,
            std::string dds_entity_name = "INVALID",
            Qos dds_entity_qos = {},
            std::string dds_entity_guid = "|GUID UNKNOWN|",
            StatusLevel status = StatusLevel::OK_STATUS,
            AppId dds_entity_app_id = AppId::UNKNOWN,
            std::string dds_entity_app_metadata = "",
            DiscoverySource dds_entity_discovery_source = DiscoverySource::UNKNOWN) noexcept
        : Entity(entity_kind, dds_entity_name, false, true, status, dds_entity_discovery_source)
        , qos(dds_entity_qos)
        , guid(dds_entity_guid)
        , app_id(dds_entity_app_id)
        , app_metadata(dds_entity_app_metadata)
        , dds_vendor(dds_vendor_by_guid(dds_entity_guid))
    {
    }

    /**
     * @brief Check whether the vendor is known based on the GUID.
     *
     * @param guid The GUID to check.
     * @return The vendor name if known, "Unknown" otherwise.
     */
    static DdsVendor dds_vendor_by_guid(
            const std::string& guid);

    //! Quality of Service configuration of the entities in a tree structure.
    Qos qos;

    //! The DDS GUID of the entity
    std::string guid;

    //! DDS entity app properties.
    AppId app_id;
    std::string app_metadata;

    //! The vendor of the DomainParticipant
    DdsVendor dds_vendor;
};

/*
 * Base class for the DDS Endpoints.
 */
struct DDSEndpoint : DDSEntity
{
    DDSEndpoint(
            EntityKind entity_kind = EntityKind::INVALID,
            std::string endpoint_name = "INVALID",
            Qos endpoint_qos = {},
            std::string endpoint_guid = "|GUID UNKNOWN|",
            details::fragile_ptr<DomainParticipant> endpoint_participant = nullptr,
            details::fragile_ptr<Topic> endpoint_topic = nullptr,
            StatusLevel status = StatusLevel::OK_STATUS,
            AppId endpoint_app_id = AppId::UNKNOWN,
            std::string endpoint_app_metadata = "",
            DiscoverySource endpoint_discovery_source = DiscoverySource::UNKNOWN,
            DomainId original_domain = UNKNOWN_DOMAIN_ID) noexcept;

    //! Reference to the DomainParticipant in which this Endpoint runs.
    details::fragile_ptr<DomainParticipant> participant;

    //! Reference to the Domain in which this endpoint publishes/subscribes.
    details::fragile_ptr<Topic> topic;

    //! Flag to signal that this is a virtual endpoint used to collect meta traffic data.
    bool is_virtual_metatraffic = false;

    /*
     * Collection of Locators related to this endpoint.
     * The collection is ordered by the EntityId of the Locator nodes.
     */
    std::map<EntityId, details::fragile_ptr<Locator>> locators;

    //! The original domain of the participant, UNKNOWN if not specified
    DomainId original_domain;
};

/*
 * DomainParticipant entities hold data about the DomainParticipant involved in the communication.
 */
struct DomainParticipant : DDSEntity
{
    DomainParticipant(
            std::string participant_name,
            Qos participant_qos,
            std::string participant_guid,
            details::fragile_ptr<Process> participant_process,
            details::fragile_ptr<Domain> participant_domain,
            StatusLevel status = StatusLevel::OK_STATUS,
            AppId participant_app_id = AppId::UNKNOWN,
            std::string participant_app_metadata = "",
            DiscoverySource participant_discovery_source = DiscoverySource::UNKNOWN,
            DomainId original_domain = UNKNOWN_DOMAIN_ID) noexcept
        : DDSEntity(EntityKind::PARTICIPANT, participant_name, participant_qos, participant_guid, status,
                participant_app_id, participant_app_metadata, participant_discovery_source)
        , process(participant_process)
        , domain(participant_domain)
        , original_domain(original_domain)
    {
    }

    /**
     * Clear the maps and data
     */
    void clear() final;

    template<typename T>
    std::map<EntityId, details::fragile_ptr<T>>& ddsendpoints();

    //! Reference to the Process in which this DomainParticipant runs.
    details::fragile_ptr<Process> process;

    //! Reference to the Domain in which this DomainParticipant runs.
    details::fragile_ptr<Domain> domain;

    //! Reference to the meta traffic endpoint of this DomainParticipant.
    details::fragile_ptr<DDSEndpoint> meta_traffic_endpoint;

    /*
     * Collection of DataReaders within the DomainParticipant which are involved in the communication.
     * The collection is ordered by the EntityId of the DataReader nodes.
     */
    std::map<EntityId, details::fragile_ptr<DataReader>> data_readers;

    /*
     * Collection of DataWriters within the DomainParticipant which are involved in the communication.
     * The collection is ordered by the EntityId of the DataWriter nodes.
     */
    std::map<EntityId, details::fragile_ptr<DataWriter>> data_writers;

    //! Actual statistical data reported by Fast DDS Statistics Module regarding this DomainParticipant.
    DomainParticipantStatisticsData data;

    //! Actual monitor service data reported by Fast DDS Statistics Module regarding this DomainParticipant.
    DomainParticipantMonitorServiceData monitor_service_data;

    //! The original domain of the participant, UNKNOWN if not specified
    DomainId original_domain;
};


/*
 * Topic entities hold data about the topics involved in the communication.
 */
struct Topic : Entity
{
    Topic(
            std::string topic_name,
            std::string topic_type,
            details::fragile_ptr<Domain> topic_domain,
            StatusLevel status = StatusLevel::OK_STATUS) noexcept
        : Entity(EntityKind::TOPIC, topic_name, is_metatraffic_topic(topic_name), true, status)
        , data_type(topic_type)
        , domain(topic_domain)
    {
    }

    /**
     * Clear the maps and data
     */
    void clear() final;

    template<typename T>
    std::map<EntityId, details::fragile_ptr<T>>& ddsendpoints();

    //! The data type name of the topic
    std::string data_type;

    //! Reference to the Domain in which this topic is published/subscribed.
    details::fragile_ptr<Domain> domain;

    /*
     * Collection of Datareaders subscribing to this topic.
     * The collection is ordered by the EntityId of the Datareader nodes.
     */
    std::map<EntityId, details::fragile_ptr<DataReader>> data_readers;

    /*
     * Collection of DataWriters publishind in this topic.
     * The collection is ordered by the EntityId of the DataWriter nodes.
     */
    std::map<EntityId, details::fragile_ptr<DataWriter>> data_writers;
};

/*
 * DataReader entities hold data about the DataReaders involved in the communication.
 */
struct DataReader : DDSEndpoint
{
    DataReader(
            std::string datareader_name,
            Qos datareader_qos,
            std::string datareader_guid,
            details::fragile_ptr<DomainParticipant> datareader_participant,
            details::fragile_ptr<Topic> datareader_topic,
            StatusLevel status = StatusLevel::OK_STATUS,
            AppId datareader_app_id = AppId::UNKNOWN,
            std::string datareader_app_metadata = "",
            DiscoverySource datareader_discovery_source = DiscoverySource::UNKNOWN,
            DomainId original_domain = UNKNOWN_DOMAIN_ID) noexcept
        : DDSEndpoint(EntityKind::DATAREADER, datareader_name, datareader_qos, datareader_guid, datareader_participant,
                datareader_topic, status, datareader_app_id, datareader_app_metadata, datareader_discovery_source,
                original_domain)
    {
    }

    /**
     * Clear the maps and data
     * This method does not clear the locators map.
     */
    void clear() final;

    //! Actual statistical data reported by Fast DDS Statistics Module regarding this DataReader.
    DataReaderStatisticsData data;

    //! Actual monitor service data reported by Fast DDS Statistics Module regarding this DataReader.
    DataReaderMonitorServiceData monitor_service_data;
};

/*
 * DataWriter entities hold data about the DataWriters involved in the communication.
 */
struct DataWriter : DDSEndpoint
{
    DataWriter(
            std::string datawriter_name,
            Qos datawriter_qos,
            std::string datawriter_guid,
            details::fragile_ptr<DomainParticipant> datawriter_participant,
            details::fragile_ptr<Topic> datawriter_topic,
            StatusLevel status = StatusLevel::OK_STATUS,
            AppId datawriter_app_id = AppId::UNKNOWN,
            std::string datawriter_app_metadata = "",
            DiscoverySource datawriter_discovery_source = DiscoverySource::UNKNOWN,
            DomainId original_domain = UNKNOWN_DOMAIN_ID) noexcept
        : DDSEndpoint(EntityKind::DATAWRITER, datawriter_name, datawriter_qos, datawriter_guid, datawriter_participant,
                datawriter_topic, status, datawriter_app_id, datawriter_app_metadata, datawriter_discovery_source,
                original_domain)
    {
    }

    /**
     * Clear the maps and data
     * This method does not clear the locators map.
     */
    void clear() final;

    //! Actual statistical data reported by Fast DDS Statistics Module regarding this DataWriter.
    DataWriterStatisticsData data;

    //! Actual monitor service data reported by Fast DDS Statistics Module regarding this DataWriter.
    DataWriterMonitorServiceData monitor_service_data;
};

/*
 * Locator entities hold data about the Locators involved in the communication.
 *
 */
struct Locator : Entity
{
    Locator(
            std::string locator_name) noexcept
        : Entity(EntityKind::LOCATOR, locator_name)
    {
    }

    /**
     * Clear the maps and data
     */
    void clear() final;

    /*
     * Collection of DataReaders using this locator.
     * The collection is ordered by the EntityId of the DataReader nodes.
     */
    std::map<EntityId, details::fragile_ptr<DataReader>> data_readers;

    /*
     * Collection of DataWriters using this locator.
     * The collection is ordered by the EntityId of the DataWriter nodes.
     */
    std::map<EntityId, details::fragile_ptr<DataWriter>> data_writers;
};

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif // FASTDDS_STATISTICS_BACKEND_SRC_CPP_DATABASE__ENTITIES_HPP
