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
 * @file Exception.cpp
 */

#include <fastdds_statistics_backend/exception/Exception.hpp>

namespace eprosima {
namespace statistics_backend {

Exception::Exception(
        const char* message) noexcept
    : message_(message)
{
}

Exception::Exception(
        const std::string& message)
    : message_(message)
{
}

const char* Exception::what() const noexcept
{
    return message_.c_str();
}

Error::Error(
        const char* message) noexcept
    : Exception(message)
{
}

Error::Error(
        const std::string& message)
    : Exception(message)
{
}

Unsupported::Unsupported(
        const char* message) noexcept
    : Exception(message)
{
}

Unsupported::Unsupported(
        const std::string& message)
    : Exception(message)
{
}

BadParameter::BadParameter(
        const char* message) noexcept
    : Exception(message)
{
}

BadParameter::BadParameter(
        const std::string& message)
    : Exception(message)
{
}

NoData::NoData(
        const char* message) noexcept
    : Exception(message)
{
}

NoData::NoData(
        const std::string& message)
    : Exception(message)
{
}

CorruptedFile::CorruptedFile(
        const char* message) noexcept
    : Exception(message)
{
}

CorruptedFile::CorruptedFile(
        const std::string& message)
    : Exception(message)
{
}

} // namespace statistics_backend
} // namespace eprosima
