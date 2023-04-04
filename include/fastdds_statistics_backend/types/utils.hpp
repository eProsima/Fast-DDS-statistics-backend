/* Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file utils.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_UTILS_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_UTILS_HPP_

#include <chrono>

#include <fastdds_statistics_backend/types/types.hpp>

namespace eprosima {
namespace statistics_backend {

//! Alias of \c now function. Returns a timestamp referring to the time the function is called.
constexpr const auto now = std::chrono::system_clock::now;

/**
 * @brief Alias of \c time_point::min function. Returns the minimum timestamp possible.
 *
 * @warning This is a negative number. It is not the same as initial time. Use \c the_initial_time for time 0.
 */
constexpr const auto the_beginning_of_time = std::chrono::time_point<std::chrono::system_clock>::min;

//! Alias of \c time_point::max function. Returns a timestamp referring to the maximum time.
constexpr const auto the_end_of_time = std::chrono::time_point<std::chrono::system_clock>::max;

/**
 * @brief 0 time.
 *
 * @return Time 0
 * @note: it is not possible to alias a ctor, thus it requires its own function.
 */
inline Timestamp the_initial_time() noexcept
{
    return Timestamp();
}

} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_UTILS_HPP_
