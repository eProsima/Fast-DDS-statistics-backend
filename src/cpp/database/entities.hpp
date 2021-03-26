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

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_ENTITIES_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_ENTITIES_HPP_

#include "data.hpp"
#include <fastdds-statistics-backend/types/types.hpp>
#include <fastdds-statistics-backend/types/EntityId.hpp>

#include <string>

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
            EntityKind entity_kind = EntityKind::INVALID)
        : kind(entity_kind)
    {
    }

    //! The unique identification of the entity
    EntityId id;

    //! The kind of entity
    EntityKind kind;

    //! A user defined human-readable name for the entity
    std::string name;
};

/*
 * Host entities hold data about the real host involved in the communication
 */
struct Host : Entity
{
    Host()
        : Entity(EntityKind::HOST)
    {
    }

    /*
     * Collection of users within the host which are involved in the communication.
     * The collection is order by EntityId of the user nodes.
     */
    std::map<EntityId, User*> users;
};

/*
 * User entities hold data about the host users involved in the communication
 */
struct User : Entity
{
    User()
        : Entity(EntityKind::USER)
    {
    }

    //! Reference to the Host in which this user runs
    Host* host;

    /*
     * Collection of processes within the host which are involved in the communication.
     * The collection is order by EntityId of the process nodes.
     */
    std::map<EntityId, Process*> processes;
};

/*
 * Process entities hold data about the computing processes involved in the communication.
 */
struct Process : Entity
{
    Process()
        : Entity(EntityKind::PROCESS)
    {
    }

    //! The PID of the process
    std::string pid;

    //! Reference to the User in which this process runs.
    User* user;

    /*
     * Collection of DomainParticipant within the process which are involved in the communication.
     * The collection is order by EntityId of the DomainParticipant nodes.
     */
    std::map<EntityId, DomainParticipant*> participants;
};

/*
 * Domain entities hold data about the DDS domains or Discovery Server networks involved in the
 * communication.
 */
struct Domain : Entity
{
    Domain()
        : Entity(EntityKind::DOMAIN)
    {
    }

    /*
     * Collection of Topics within the Domain which are either published, subscribed, or both.
     * The collection is order by EntityId of the Topic nodes.
     */
    std::map<EntityId, Topic*> topics;

    /*
     * Collection of DomainParticipant within the Domain which are involved in the communication.
     * The collection is order by EntityId of the DomainParticipant nodes.
     */
    std::map<EntityId, DomainParticipant*> participants;
};

/*
 * Base class for the DDS Entities.
 */
struct DDSEntity : Entity
{
    DDSEntity(
            EntityKind entity_kind = EntityKind::INVALID)
        : Entity(kind)
    {
    }

    //! Quality of Service configuration of the entities in a tree structure.
    Qos qos;

    //! The DDS GUID of the entity
    std::string guid;
};

/*
 * DomainParticipant entities hold data about the DomainParticipant involved in the communication.
 */
struct DomainParticipant : DDSEntity
{
    DomainParticipant()
        : DDSEntity(EntityKind::PARTICIPANT)
    {
    }

    //! Reference to the Process in which this DomainParticipant runs.
    Process* process;

    //! Reference to the Domain in which this DomainParticipant runs.
    Domain* domain;

    /*
     * Collection of DataReaders within the DomainParticipant which are involved in the communication.
     * The collection is order by EntityId of the DataReader nodes.
     */
    std::map<EntityId, DataReader*> data_readers;

    /*
     * Collection of DataWriters within the DomainParticipant which are involved in the communication.
     * The collection is order by EntityId of the DataWriter nodes.
     */
    std::map<EntityId, DataWriter*> data_writers;

    //! Actual statistical data reported by Fast DDS Statistics Module regarding this DomainParticipant.
    DomainParticipantData data;
};

/*
 * Topic entities hold data about the topics involved in the communication.
 */
struct Topic : Entity
{
    Topic()
        : Entity(EntityKind::TOPIC)
    {
    }

    //! The data type name of the topic
    std::string data_type;

    //! Reference to the Domain in which this topic is published/subscribed.
    Domain* domain;

    /*
     * Collection of Datareaders subscribing to this topic.
     * The collection is order by EntityId of the Datareader nodes.
     */
    std::map<EntityId, DataReader*> data_readers;

    /*
     * Collection of DataWriters publishind in this topic.
     * The collection is order by EntityId of the DataWriter nodes.
     */
    std::map<EntityId, DataWriter*> data_writers;
};

/*
 * Base class for the DDS Endpoints.
 */
struct DDSEndpoint : DDSEntity
{
    DDSEndpoint(
            EntityKind entity_kind = EntityKind::INVALID)
        : DDSEntity(kind)
    {
    }

    //! Reference to the DomainParticipant in which this Endpoint runs.
    DomainParticipant* participant;

    //! Reference to the Domain in which this endpoint publishes/subscribes.
    Topic* topic;

    /*
     * Collection of Locators related to this endpoint.
     * The collection is order by EntityId of the Locator nodes.
     */
    std::map<EntityId, Locator*> locators;
};

/*
 * DataReader entities hold data about the DataReaders involved in the communication.
 */
struct DataReader : DDSEndpoint
{
    DataReader()
        : DDSEndpoint(EntityKind::DATAREADER)
    {
    }

    //! Actual statistical data reported by Fast DDS Statistics Module regarding this DataReader.
    DataReaderData data;
};

/*
 * DataWriter entities hold data about the DataWriters involved in the communication.
 */
struct DataWriter : DDSEndpoint
{
    DataWriter()
        : DDSEndpoint(EntityKind::DATAWRITER)
    {
    }

    //! Actual statistical data reported by Fast DDS Statistics Module regarding this DataWriter.
    DataWriterData data;
};

/*
 * Locator entities hold data about the Locators involved in the communication.
 *
 */
struct Locator : Entity
{
    Locator()
        : Entity(EntityKind::LOCATOR)
    {
    }

    /*
     * Collection of DataReaders using this locator.
     * The collection is order by EntityId of the DataReader nodes.
     */
    std::map<EntityId, DataReader*> data_readers;

    /*
     * Collection of DataWriters using this locator.
     * The collection is order by EntityId of the DataWriter nodes.
     */
    std::map<EntityId, DataWriter*> data_writers;

    //! Actual statistical data reported by Fast DDS Statistics Module regarding this locator.
    LocatorData data;
};

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_ENTITIES_HPP_
