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
 * @file samples.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_SAMPLES_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_SAMPLES_HPP_

#include <chrono>

namespace eprosima {
namespace statistics_backend {
namespace database {

/**
 * Enumeration for the type of sample
 */
enum StatisticsSampleKind
{
    INVALID,
    ENTITY_DATA_SAMPLE,
    ENTITY_COUNT_SAMPLE,
    BYTE_COUNT_SAMPLE,
};

/*
 * Base class for all statistics samples. It adds the timepoint for the sample
 */
struct StatisticsSample
{
    StatisticsSample(
            StatisticsSampleKind sample_kind = StatisticsSampleKind::INVALID)
        : kind(sample_kind)
    {
    }

    StatisticsSampleKind kind;
    std::chrono::steady_clock::time_point src_ts;
};

/*
 * Generic data reported by an entity
 */
struct EntityDataSample : StatisticsSample
{
    EntityDataSample()
        : StatisticsSample(StatisticsSampleKind::ENTITY_DATA_SAMPLE)
    {
    }

    double data;
};

/*
 * Count reported by an entity
 */
struct EntityCountSample : StatisticsSample
{
    EntityCountSample()
        : StatisticsSample(StatisticsSampleKind::ENTITY_COUNT_SAMPLE)
    {
    }

    uint64_t count;
};

/*
 * Count reported by an entity. The magnitude is a multiplyer in base 10
 */
struct ByteCountSample : StatisticsSample
{
    ByteCountSample()
        : StatisticsSample(StatisticsSampleKind::BYTE_COUNT_SAMPLE)
    {
    }

    uint64_t count;
    int16_t magnitude_order;
};

} //namespace database
} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_DATABASE_SAMPLES_HPP_
