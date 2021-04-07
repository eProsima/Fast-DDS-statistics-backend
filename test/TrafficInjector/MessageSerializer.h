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
 * @file MessageSerializer.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TEST_MESSAGE_SERIALIZER_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TEST_MESSAGE_SERIALIZER_HPP_

#include <topic_types/types.h>

#include <nlohmann-json/json.hpp>

#include <string>
#include <fstream>

namespace eprosima {
namespace statistics_backend {


class MessageSerializer
{

public:

    using Message = nlohmann::ordered_json;

protected:

    using StatisticsData = eprosima::fastdds::statistics::Data;

    using StatisticsWriterReaderData = eprosima::fastdds::statistics::WriterReaderData;
    using StatisticsLocator2LocatorData = eprosima::fastdds::statistics::Locator2LocatorData;
    using StatisticsEntityData = eprosima::fastdds::statistics::EntityData;
    using StatisticsEntity2LocatorTraffic = eprosima::fastdds::statistics::Entity2LocatorTraffic;
    using StatisticsEntityCount = eprosima::fastdds::statistics::EntityCount;
    using StatisticsDiscoveryTime = eprosima::fastdds::statistics::DiscoveryTime;
    using StatisticsSampleIdentityCount = eprosima::fastdds::statistics::SampleIdentityCount;
    using StatisticsPhysicalData = eprosima::fastdds::statistics::PhysicalData;
    using StatisticsEntityId = eprosima::fastdds::statistics::detail::EntityId_s;
    using StatisticsGuidPrefix = eprosima::fastdds::statistics::detail::GuidPrefix_s;
    using StatisticsGuid = eprosima::fastdds::statistics::detail::GUID_s;
    using StatisticsSequenceNumber = eprosima::fastdds::statistics::detail::SequenceNumber_s;
    using StatisticsSampleIdentity = eprosima::fastdds::statistics::detail::SampleIdentity_s;
    using StatisticsLocator = eprosima::fastdds::statistics::detail::Locator_s;

public:

    virtual void serialize(
            void* data,
            Message& message) = 0;

    virtual void deserialize(
            void* data,
            const Message& message) = 0;
};

class GuidMessageSerializer : public MessageSerializer
{
public:

    virtual void serialize(
            void* data,
            Message& message) override
    {
        message["guid_prefix"] = static_cast<StatisticsGuid*>(data)->guidPrefix().value();
        message["entityId"] = static_cast<StatisticsGuid*>(data)->guidPrefix().value();
    }

    virtual void deserialize(
            void* data,
            const Message& message) override
    {
        static_cast<StatisticsGuid*>(data)->guidPrefix().value(message.at("guid_prefix").get<std::array<uint8_t,
                12>>());
        static_cast<StatisticsGuid*>(data)->entityId().value(message.at("entityId").get<std::array<uint8_t, 4>>());
    }

};

class LocatorMessageSerializer : public MessageSerializer
{
public:

    virtual void serialize(
            void* data,
            Message& message) override
    {
        message["kind"] = static_cast<StatisticsLocator*>(data)->kind();
        message["port"] = static_cast<StatisticsLocator*>(data)->port();
        message["address"] = static_cast<StatisticsLocator*>(data)->address();
    }

    virtual void deserialize(
            void* data,
            const Message& message) override
    {
        static_cast<StatisticsLocator*>(data)->kind(message.at("kind").get<int32_t>());
        static_cast<StatisticsLocator*>(data)->port(message.at("port").get<uint32_t>());
        static_cast<StatisticsLocator*>(data)->address(message.at("address").get<std::array<uint8_t, 16>>());
    }

};

class SequenceNumberMessageSerializer : public MessageSerializer
{
public:

    virtual void serialize(
            void* data,
            Message& message) override
    {
        message["high"] = static_cast<StatisticsSequenceNumber*>(data)->high();
        message["low"] = static_cast<StatisticsSequenceNumber*>(data)->low();
    }

    virtual void deserialize(
            void* data,
            const Message& message) override
    {
        static_cast<StatisticsSequenceNumber*>(data)->high(message.at("high").get<int32_t>());
        static_cast<StatisticsSequenceNumber*>(data)->low(message.at("low").get<uint32_t>());
    }

};

class SampleIdentityMessageSerializer : public MessageSerializer
{
public:

    virtual void serialize(
            void* data,
            Message& message) override
    {
        GuidMessageSerializer guid_serializer;
        guid_serializer.serialize(&static_cast<StatisticsSampleIdentity*>(data)->writer_guid(), message["writer_guid"]);
        SequenceNumberMessageSerializer sn_serializer;
        sn_serializer.serialize(&static_cast<StatisticsSampleIdentity*>(data)->sequence_number(),
                message["sequence_number"]);
    }

    virtual void deserialize(
            void* data,
            const Message& message) override
    {
        GuidMessageSerializer guid_serializer;
        guid_serializer.deserialize(&static_cast<StatisticsSampleIdentity*>(data)->writer_guid(),
                message.at("writer_guid"));
        SequenceNumberMessageSerializer sn_serializer;
        sn_serializer.deserialize(&static_cast<StatisticsSampleIdentity*>(data)->sequence_number(),
                message.at("sequence_number"));
    }

};

class WriterReaderDataMessageSerializer : public MessageSerializer
{
public:

    virtual void serialize(
            void* data,
            Message& message) override
    {
        StatisticsWriterReaderData inner_data = static_cast<StatisticsData*>(data)->writer_reader_data();

        GuidMessageSerializer guid_serializer;
        guid_serializer.serialize(&inner_data.writer_guid(), message["writer_guid"]);
        guid_serializer.serialize(&inner_data.reader_guid(), message["reader_guid"]);
        message["data"] = inner_data.data();
    }

    virtual void deserialize(
            void* data,
            const Message& message) override
    {
        StatisticsWriterReaderData inner_data;

        GuidMessageSerializer guid_serializer;
        guid_serializer.deserialize(&inner_data.writer_guid(), message.at("writer_guid"));
        guid_serializer.deserialize(&inner_data.reader_guid(), message.at("reader_guid"));
        inner_data.data(message.at("data").get<float>());

        static_cast<StatisticsData*>(data)->writer_reader_data(inner_data);
    }

};

class Locator2LocatorDataMessageSerializer : public MessageSerializer
{
public:

    virtual void serialize(
            void* data,
            Message& message) override
    {
        StatisticsLocator2LocatorData inner_data = static_cast<StatisticsData*>(data)->locator2locator_data();

        LocatorMessageSerializer locator_serializer;
        locator_serializer.serialize(&inner_data.src_locator(), message["src_locator"]);
        locator_serializer.serialize(&inner_data.dst_locator(), message["dst_locator"]);
        message["data"] = inner_data.data();
    }

    virtual void deserialize(
            void* data,
            const Message& message) override
    {
        StatisticsLocator2LocatorData inner_data;

        LocatorMessageSerializer locator_serializer;
        locator_serializer.deserialize(&inner_data.src_locator(), message.at("src_locator"));
        locator_serializer.deserialize(&inner_data.dst_locator(), message.at("dst_locator"));
        inner_data.data(message.at("data").get<float>());

        static_cast<StatisticsData*>(data)->locator2locator_data(inner_data);
    }

};

class EntityDataMessageSerializer : public MessageSerializer
{
public:

    virtual void serialize(
            void* data,
            Message& message) override
    {
        StatisticsEntityData inner_data = static_cast<StatisticsData*>(data)->entity_data();

        GuidMessageSerializer guid_serializer;
        guid_serializer.serialize(&inner_data.guid(), message["guid"]);
        message["data"] = inner_data.data();
    }

    virtual void deserialize(
            void* data,
            const Message& message) override
    {
        StatisticsEntityData inner_data;

        GuidMessageSerializer guid_serializer;
        guid_serializer.deserialize(&inner_data.guid(), message.at("guid"));
        inner_data.data(message.at("data").get<float>());

        static_cast<StatisticsData*>(data)->entity_data(inner_data);
    }

};

class EntityCountMessageSerializer : public MessageSerializer
{
public:

    virtual void serialize(
            void* data,
            Message& message) override
    {
        StatisticsEntityCount inner_data = static_cast<StatisticsData*>(data)->entity_count();

        GuidMessageSerializer guid_serializer;
        guid_serializer.serialize(&inner_data.guid(), message["guid"]);
        message["count"] = inner_data.count();
    }

    virtual void deserialize(
            void* data,
            const Message& message) override
    {
        StatisticsEntityCount inner_data;

        GuidMessageSerializer guid_serializer;
        guid_serializer.deserialize(&inner_data.guid(), message.at("guid"));
        inner_data.count(message.at("count").get<uint64_t>());

        static_cast<StatisticsData*>(data)->entity_count(inner_data);
    }

};

class DiscoveryTimeMessageSerializer : public MessageSerializer
{
public:

    virtual void serialize(
            void* data,
            Message& message) override
    {
        StatisticsDiscoveryTime inner_data = static_cast<StatisticsData*>(data)->discovery_time();

        GuidMessageSerializer guid_serializer;
        guid_serializer.serialize(&inner_data.local_participant_guid(), message["local_participant_guid"]);
        guid_serializer.serialize(&inner_data.remote_entity_guid(), message["remote_entity_guid"]);
        message["time"] = inner_data.time();
    }

    virtual void deserialize(
            void* data,
            const Message& message) override
    {
        StatisticsDiscoveryTime inner_data;

        GuidMessageSerializer guid_serializer;
        guid_serializer.deserialize(&inner_data.local_participant_guid(), message.at("local_participant_guid"));
        guid_serializer.deserialize(&inner_data.remote_entity_guid(), message.at("remote_entity_guid"));
        inner_data.time(message.at("time").get<uint64_t>());

        static_cast<StatisticsData*>(data)->discovery_time(inner_data);
    }

};

class Entity2LocatorTrafficMessageSerializer : public MessageSerializer
{
public:

    virtual void serialize(
            void* data,
            Message& message) override
    {
        StatisticsEntity2LocatorTraffic inner_data = static_cast<StatisticsData*>(data)->entity2locator_traffic();

        GuidMessageSerializer guid_serializer;
        guid_serializer.serialize(&inner_data.src_guid(), message["src_guid"]);
        LocatorMessageSerializer locator_serializer;
        locator_serializer.serialize(&inner_data.dst_locator(), message["dst_locator"]);
        message["packet_count"] = inner_data.packet_count();
        message["byte_count"] = inner_data.byte_count();
        message["order"] = inner_data.byte_magnitude_order();
    }

    virtual void deserialize(
            void* data,
            const Message& message) override
    {
        StatisticsEntity2LocatorTraffic inner_data;

        GuidMessageSerializer guid_serializer;
        guid_serializer.deserialize(&inner_data.src_guid(), message.at("src_guid"));
        LocatorMessageSerializer locator_serializer;
        locator_serializer.deserialize(&inner_data.dst_locator(), message.at("dst_locator"));
        inner_data.packet_count(message.at("packet_count").get<uint64_t>());
        inner_data.byte_count(message.at("byte_count").get<uint64_t>());
        inner_data.byte_magnitude_order(message.at("order").get<int16_t>());

        static_cast<StatisticsData*>(data)->entity2locator_traffic(inner_data);
    }

};

class SampleIdentityCountMessageSerializer : public MessageSerializer
{
public:

    virtual void serialize(
            void* data,
            Message& message) override
    {
        StatisticsSampleIdentityCount inner_data = static_cast<StatisticsData*>(data)->sample_identity_count();

        SampleIdentityMessageSerializer sid_serializer;
        sid_serializer.serialize(&inner_data.sample_id(), message["sample_id"]);
        message["count"] = inner_data.count();
    }

    virtual void deserialize(
            void* data,
            const Message& message) override
    {
        StatisticsSampleIdentityCount inner_data;

        SampleIdentityMessageSerializer sid_serializer;
        sid_serializer.deserialize(&inner_data.sample_id(), message.at("sample_id"));
        inner_data.count(message.at("count").get<uint64_t>());

        static_cast<StatisticsData*>(data)->sample_identity_count(inner_data);
    }

};

class PhysicalDataMessageSerializer : public MessageSerializer
{
public:

    virtual void serialize(
            void* data,
            Message& message) override
    {
        StatisticsPhysicalData inner_data = static_cast<StatisticsData*>(data)->physical_data();

        GuidMessageSerializer guid_serializer;
        guid_serializer.serialize(&inner_data.participant_guid(), message["participant_guid"]);
        message["host"] = inner_data.host();
        message["user"] = inner_data.user();
        message["process"] = inner_data.process();
    }

    virtual void deserialize(
            void* data,
            const Message& message) override
    {
        StatisticsPhysicalData inner_data;

        GuidMessageSerializer guid_serializer;
        guid_serializer.deserialize(&inner_data.participant_guid(), message.at("participant_guid"));
        inner_data.host(message.at("host").get<std::string>());
        inner_data.user(message.at("user").get<std::string>());
        inner_data.process(message.at("process").get<std::string>());

        static_cast<StatisticsData*>(data)->physical_data(inner_data);
    }

};


} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TEST_MESSAGE_SERIALIZER_HPP_
