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

#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicData.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicDataFactory.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilder.hpp>
#include <fastdds/dds/xtypes/dynamic_types/DynamicTypeBuilderFactory.hpp>

#include <subscriber/UserDataReaderListener.hpp>
#include <subscriber/UserDataContext.hpp>
#include <fastdds_statistics_backend/nlohmann-json/json.hpp>

#include <functional>
#include <vector>

#include <gtest_aux.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace eprosima::fastdds::dds;
using namespace eprosima::statistics_backend::subscriber;
using json = nlohmann::json;

class TestUserDataContext : public UserDataContext
{

public:

    void clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        discovered_user_data_types_.clear();
        discovered_topics_.clear();
    }
};

class user_data_reader_listener_tests : public ::testing::Test
{

public:

    TestUserDataContext user_data_context_;
    std::vector<std::string> processed_data_messages_;
    std::function<void(const std::string& data)> on_data_received_;
    UserDataReaderListener reader_listener_;
    eprosima::fastdds::dds::DataReader datareader_;

    user_data_reader_listener_tests()
        : user_data_context_()
        , processed_data_messages_()
        , on_data_received_([this](const std::string& data)
                {
                    processed_data_messages_.push_back(data);
                })
        , reader_listener_(on_data_received_, &user_data_context_)
    {
    }

    void add_sample_to_reader_history(
            DynamicData::_ref_type data,
            std::shared_ptr<SampleInfo> info)
    {
        datareader_.add_user_data_sample(data, info);
    }

    std::shared_ptr<SampleInfo> get_default_info()
    {
        std::shared_ptr<SampleInfo> info = std::make_shared<SampleInfo>();

        info->sample_state = NOT_READ_SAMPLE_STATE;
        info->view_state = NOT_NEW_VIEW_STATE;
        info->disposed_generation_count = 0;
        info->no_writers_generation_count = 1;
        info->sample_rank = 0;
        info->generation_rank = 0;
        info->absolute_generation_rank = 0;
        info->valid_data = true;
        info->instance_state = ALIVE_INSTANCE_STATE;

        return info;
    }

    void clear()
    {
        processed_data_messages_.clear();
        user_data_context_.clear();
    }

};

TEST_F(user_data_reader_listener_tests, process_data_from_known_type)
{
    // Precondition: The data type is already known
    DynamicTypeBuilder::_ref_type type_builder = DynamicTypeBuilderFactory::get_instance()->create_type_w_uri("../../types/UserData/FooType.idl", "FooType", std::vector<std::string>());
    ASSERT_NE(type_builder, nullptr);
    DynamicType::_ref_type type = type_builder->build();
    user_data_context_.register_user_data_topic("FooTopic", type);

    // Simulate data reception
    std::shared_ptr<SampleInfo> info = get_default_info();
    DynamicData::_ref_type data = DynamicDataFactory::get_instance()->create_data(type);
    data->set_int32_value(0, 2);
    data->set_string_value(1, "FooMessage");
    json expected_message = {
            {"num", 2},
            {"msg", "FooMessage"}
    };

    add_sample_to_reader_history(data, info);
    datareader_.set_topic_name("FooTopic");
    reader_listener_.on_data_available(&datareader_);

    ASSERT_EQ(processed_data_messages_.size(), 1u);
    json received_message = json::parse(processed_data_messages_.front());
    ASSERT_EQ(received_message, expected_message);

    ASSERT_EQ(RETCODE_OK, DynamicDataFactory::get_instance()->delete_data(data));

    // Clear state
    clear();
}

TEST_F(user_data_reader_listener_tests, process_data_from_unknown_type)
{
    // Simulate data reception from a topic which type is unknown (i.e: not registered in the context).
    std::shared_ptr<SampleInfo> info = get_default_info();
    DynamicTypeBuilder::_ref_type type_builder = DynamicTypeBuilderFactory::get_instance()->create_type_w_uri("../../types/UserData/FooType.idl", "FooType", std::vector<std::string>());
    ASSERT_NE(type_builder, nullptr);
    DynamicType::_ref_type type = type_builder->build();
    DynamicData::_ref_type data = DynamicDataFactory::get_instance()->create_data(type);
    data->set_int32_value(0, 2);
    data->set_string_value(1, "FooMessage");

    add_sample_to_reader_history(data, info);
    datareader_.set_topic_name("FooTopic");
    reader_listener_.on_data_available(&datareader_);

    // Expected: No data processed (type is unknown)
    ASSERT_EQ(processed_data_messages_.size(), 0u);

    ASSERT_EQ(RETCODE_OK, DynamicDataFactory::get_instance()->delete_data(data));

    // Clear state
    clear();
}

TEST_F(user_data_reader_listener_tests, process_invalid_sample)
{
    DynamicTypeBuilder::_ref_type type_builder = DynamicTypeBuilderFactory::get_instance()->create_type_w_uri("../../types/UserData/FooType.idl", "FooType", std::vector<std::string>());
    ASSERT_NE(type_builder, nullptr);
    DynamicType::_ref_type type = type_builder->build();
    user_data_context_.register_user_data_topic("FooTopic", type);

    // Simulate invalid data reception
    std::shared_ptr<SampleInfo> info = get_default_info();
    info->valid_data = false;
    DynamicData::_ref_type data = DynamicDataFactory::get_instance()->create_data(type);
    data->set_int32_value(0, 2);
    data->set_string_value(1, "FooMessage");

    add_sample_to_reader_history(data, info);
    datareader_.set_topic_name("FooTopic");
    reader_listener_.on_data_available(&datareader_);

    // Expected: No data processed (invalid data received)
    ASSERT_EQ(processed_data_messages_.size(), 0u);

    ASSERT_EQ(RETCODE_OK, DynamicDataFactory::get_instance()->delete_data(data));

    // Clear state
    clear();
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
