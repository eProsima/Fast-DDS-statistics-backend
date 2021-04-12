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

#include <database/samples.hpp>

#include "gtest/gtest.h"

#include <chrono>

using namespace eprosima::statistics_backend;
using namespace eprosima::statistics_backend::database;

TEST(database, statistics_sample_clear)
{
    StatisticsSample sample(DataKind::NACKFRAG_COUNT);
    sample.src_ts = std::chrono::steady_clock::now();
    sample.clear();
    ASSERT_EQ(sample.kind, DataKind::INVALID);
    ASSERT_EQ(sample.src_ts, std::chrono::steady_clock::time_point());
}

TEST(database, entitydata_sample_clear)
{
    EntityDataSample sample;
    sample.src_ts = std::chrono::steady_clock::now();
    sample.data = 12.0;
    sample.clear();
    ASSERT_EQ(sample.kind, DataKind::INVALID);
    ASSERT_EQ(sample.src_ts, std::chrono::steady_clock::time_point());
    ASSERT_EQ(sample.data, 0);
}

TEST(database, entitycount_sample_clear)
{
    EntityCountSample sample;
    sample.src_ts = std::chrono::steady_clock::now();
    sample.count = 12;
    sample.clear();
    ASSERT_EQ(sample.kind, DataKind::INVALID);
    ASSERT_EQ(sample.src_ts, std::chrono::steady_clock::time_point());
    ASSERT_EQ(sample.count, 0);
}

TEST(database, bytecount_sample_clear)
{
    ByteCountSample sample;
    sample.src_ts = std::chrono::steady_clock::now();
    sample.count = 12;
    sample.magnitude_order = 2;
    sample.clear();
    ASSERT_EQ(sample.kind, DataKind::INVALID);
    ASSERT_EQ(sample.src_ts, std::chrono::steady_clock::time_point());
    ASSERT_EQ(sample.count, 0);
    ASSERT_EQ(sample.magnitude_order, 0);
}

int main(
        int argc,
        char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
