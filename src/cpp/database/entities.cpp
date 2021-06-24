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

#include "entities.hpp"

namespace eprosima {
namespace statistics_backend {
namespace database {

void Host::clear()
{
    users.clear();
}

void User::clear()
{
    processes.clear();
}

void Process::clear()
{
    participants.clear();
}

void Domain::clear()
{
    topics.clear();
    participants.clear();
}

void DomainParticipant::clear()
{
    data_readers.clear();
    data_writers.clear();
    data.clear();
}

void Topic::clear()
{
    data_readers.clear();
    data_writers.clear();
}

void DataReader::clear()
{
    data.clear();
}

void DataWriter::clear()
{
    data.clear();
}

void Locator::clear()
{
    data_readers.clear();
    data_writers.clear();
    data.clear();
}

void DDSEndpoint::clear()
{
    locators.clear();
}

template<>
std::map<EntityId, std::shared_ptr<DataReader>>& DomainParticipant::ddsendpoints<DataReader>()
{
    return data_readers;
}

template<>
std::map<EntityId, std::shared_ptr<DataWriter>>& DomainParticipant::ddsendpoints<DataWriter>()
{
    return data_writers;
}

template<>
std::map<EntityId, std::shared_ptr<DataReader>>& Topic::ddsendpoints<DataReader>()
{
    return data_readers;
}

template<>
std::map<EntityId, std::shared_ptr<DataWriter>>& Topic::ddsendpoints<DataWriter>()
{
    return data_writers;
}

} //namespace database
} //namespace statistics_backend
} //namespace eprosima
