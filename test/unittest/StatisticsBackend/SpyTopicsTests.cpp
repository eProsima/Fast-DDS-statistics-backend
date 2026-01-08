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

#include <algorithm>
#include <list>
#include <string>
#include <vector>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>

#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>

#include <StatisticsBackend.hpp>
#include <StatisticsBackendData.hpp>
#include <Monitor.hpp>
#include <types/Alerts.hpp>
#include <types/app_names.h>
#include <types/JSONTags.h>
#include <types/Notifiers.hpp>
#include <types/types.hpp>
#include <database/database.hpp>
#include <DatabaseUtils.hpp>

using namespace eprosima::fastdds::dds;
using namespace eprosima::statistics_backend;

using DomainParticipantFactory = eprosima::fastdds::dds::DomainParticipantFactory;
using DomainParticipant = eprosima::fastdds::dds::DomainParticipant;
using Publisher = eprosima::fastdds::dds::Publisher;
using Topic = eprosima::fastdds::dds::Topic;
using DataWriter = eprosima::fastdds::dds::DataWriter;
using TopicDataType = eprosima::fastdds::dds::TopicDataType;
using TypeSupport = eprosima::fastdds::dds::TypeSupport;

class WriterHelper : public eprosima::fastdds::dds::DataWriterListener
{
public:

    WriterHelper()
    {
        // Creates a publisher in the given domain with a topic with the given name
        // and waits until the monitor discovers it
        participant_ =
                DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
        if (nullptr == participant_)
        {
            // Error
            return;
        }

        publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT);
        if (nullptr == publisher_)
        {
            // Error
            return;
        }

        auto type_builder =
                DynamicTypeBuilderFactory::get_instance()->create_type_w_uri(
            "../../types/UserData/FooType.idl",
            "FooType",
            {});

        dyn_type_ = type_builder->build();
        dyn_ts_ = TypeSupport(new DynamicPubSubType(dyn_type_));
        participant_->register_type(dyn_ts_);

        topic_name_ = "FooTopic";
        topic_ = participant_->create_topic(
            topic_name_,
            "FooType",
            TOPIC_QOS_DEFAULT);
        if (!topic_)
        {
            throw std::runtime_error("Error creating topic");
        }

        writer_ = publisher_->create_datawriter(topic_, eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT,
                        this);
        if (!writer_)
        {
            throw std::runtime_error("Error creating writer");
        }

    }

    // Listener callback
    void on_publication_matched(
            eprosima::fastdds::dds::DataWriter* /*writer*/,
            const PublicationMatchedStatus& info) override
    {
        if (info.current_count_change > 0)
        {
            matched_ = true;
        }
        else
        {
            matched_ = false;
        }
    }

    // Write method for a simple string message
    void write(
            long num,
            const std::string& message)
    {
        DynamicData::_ref_type data = DynamicDataFactory::get_instance()->create_data(dyn_type_);
        data->set_int32_value(0, num);
        data->set_string_value(1, message);
        writer_->write(&data);
    }

    // Cleanup
    ~WriterHelper()
    {
        if (publisher_ && writer_)
        {
            publisher_->delete_datawriter(writer_);
        }

        if (participant_ && topic_)
        {
            participant_->delete_topic(topic_);
        }

        if (participant_ && publisher_)
        {
            participant_->delete_publisher(publisher_);
        }

        if (participant_)
        {
            DomainParticipantFactory::get_instance()->delete_participant(participant_);
        }
    }

    eprosima::fastdds::dds::DomainParticipant* participant_;
    Publisher* publisher_;
    eprosima::fastdds::dds::DataWriter* writer_;
    eprosima::fastdds::dds::Topic* topic_;
    std::string topic_name_;
    TypeSupport dyn_ts_;
    eprosima::fastdds::dds::DynamicType::_ref_type dyn_type_;
    bool matched_ = false;
};


class spy_topics_tests : public ::testing::TestWithParam<std::tuple<EntityKind, size_t,
            std::vector<size_t>>>
{
public:

    using TestId = PopulateDatabase::TestId;

    void SetUp()
    {
        // Setting up the monitor
        monitor_id_ = StatisticsBackend::init_monitor(
            0,
            nullptr,
            CallbackMask::none(),
            DataKindMask::all(),
            "test_monitor",
            "metadata");
    }

    void TearDown()
    {
        StatisticsBackend::stop_monitor(monitor_id_);
    }

    EntityId monitor_id_;
};

TEST_F(spy_topics_tests, no_callback_called_if_no_write)
{
    // Create a publisher with a topic
    WriterHelper writer;
    std::mutex reception_mutex;
    bool data_received = false;


    StatisticsBackend::start_topic_spy(monitor_id_, writer.topic_name_,
            [&](const std::string& /*data*/)
            {
                try
                {
                    {
                        std::lock_guard<std::mutex> lock(reception_mutex);
                        data_received = true;
                    }

                }
                catch (const std::exception& /*e*/)
                {
                    // Exceptions are ignored to avoid breaking the topic spy

                }
            });


    // Give some time for the monitor to set up everything
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Validate no callback was called
    {
        std::lock_guard<std::mutex> lock(reception_mutex);
        EXPECT_EQ(data_received, false);
    }

    // Cleanup
    StatisticsBackend::stop_topic_spy(monitor_id_, writer.topic_name_);
}

TEST_F(spy_topics_tests, spy_simple_message)
{
    // Create a publisher with a topic
    WriterHelper writer;
    std::mutex reception_mutex;
    bool data_received = false;
    std::condition_variable reception_cv;
    std::string incoming_data;


    StatisticsBackend::start_topic_spy(monitor_id_, writer.topic_name_,
            [&](const std::string& data)
            {
                try
                {
                    {
                        std::lock_guard<std::mutex> lock(reception_mutex);
                        incoming_data = data;
                        data_received = true;
                    }

                    reception_cv.notify_one();
                }
                catch (const std::exception& /*e*/)
                {
                    // Exceptions are ignored to avoid breaking the topic spy

                }
            });


    // Give some time for the monitor to set up everything
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Write data
    std::string expected_json = R"({"msg":"Hello World","num":5})";
    writer.write(5, "Hello World");

    // Wait for the callback to process some data
    {
        std::unique_lock<std::mutex> lock(reception_mutex);
        reception_cv.wait_for(lock, std::chrono::seconds(2), [&]
                {
                    return data_received;
                });
    }

    auto clean_string = [](std::string s)
            {
                s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
                return s;
            };

    // Validate received JSON
    ASSERT_EQ(clean_string(incoming_data), clean_string(expected_json));

    // Cleanup
    StatisticsBackend::stop_topic_spy(monitor_id_, writer.topic_name_);
}


TEST_F(spy_topics_tests, second_spy_on_topic_ignored)
{
    // Create a publisher with a topic
    WriterHelper writer;
    std::mutex reception_mutex;
    bool data_received = false;
    bool data_received_2 = false;

    StatisticsBackend::start_topic_spy(monitor_id_, writer.topic_name_,
            [&](const std::string& /*data*/)
            {
                try
                {
                    {
                        std::lock_guard<std::mutex> lock(reception_mutex);
                        data_received = true;
                    }

                }
                catch (const std::exception& /*e*/)
                {
                    // Exceptions are ignored to avoid breaking the topic spy

                }
            });


    // Second call, should be ignored
    StatisticsBackend::start_topic_spy(monitor_id_, writer.topic_name_,
            [&](const std::string& /*data*/)
            {
                try
                {
                    {
                        std::lock_guard<std::mutex> lock(reception_mutex);
                        data_received_2 = true;
                    }

                }
                catch (const std::exception& /*e*/)
                {
                    // Exceptions are ignored to avoid breaking the topic spy

                }
            });

    // Give some time for the monitor to set up everything
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    writer.write(5, "Hello World");

    // Validate no callback was called
    {
        std::lock_guard<std::mutex> lock(reception_mutex);
        EXPECT_EQ(data_received, true);
        EXPECT_EQ(data_received_2, false);
    }

    // Cleanup
    StatisticsBackend::stop_topic_spy(monitor_id_, writer.topic_name_);
}


TEST_F(spy_topics_tests, exception_with_unknown_topic)
{
    // Create a publisher with a topic
    EXPECT_THROW(
        StatisticsBackend::start_topic_spy(monitor_id_, "unknown_topic_name",
        [&](const std::string& /*data*/)
        {
            // No need to have content here
        });
        , Error);
}

TEST_F(spy_topics_tests, can_spy_on_statistics_topics)
{
    // Create a participant with statistics enabled to trigger discovery
    DomainParticipantQos pqos = PARTICIPANT_QOS_DEFAULT;
    pqos.properties().properties().emplace_back(
        "fastdds.statistics",
        "NETWORK_LATENCY_TOPIC",  // Enable the specific topic we want to spy on
        "true");

    auto stats_participant = DomainParticipantFactory::get_instance()->create_participant(
        0, pqos);

    // Give time for discovery
    std::this_thread::sleep_for(std::chrono:: milliseconds(200));

    std::string statistics_topic_name = "_fastdds_statistics_network_latency";

    // Now the type should be discovered
    EXPECT_NO_THROW(
        StatisticsBackend::start_topic_spy(monitor_id_, statistics_topic_name,
        [&](const std::string& /*data*/)
        {
            // Regular callback
        })
        );

    // Cleanup
    StatisticsBackend:: stop_topic_spy(monitor_id_, statistics_topic_name);
    DomainParticipantFactory:: get_instance()->delete_participant(stats_participant);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
