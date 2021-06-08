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
 * @file Exception.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_EXCEPTION_EXCEPTION_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_EXCEPTION_EXCEPTION_HPP_

#include <exception>
#include <string>

#include <fastdds_statistics_backend/fastdds_statistics_backend_dll.h>

namespace eprosima {
namespace statistics_backend {

/**
 * @brief Base class for all exceptions thrown by the eProsima statistics backend library.
 *
 */
class Exception : public std::exception
{

public:

    /**
     * @brief Construct a new statistics_backend::Exception object
     *
     * @param message The message to be returned by what()
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI Exception(
            const char* message) noexcept;

    /**
     * @brief Construct a new statistics_backend::Exception object
     *
     * @param message The message to be returned by what()
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI Exception(
            const std::string& message);

    /**
     * @brief Copies the statistics_backend::Exception object into a new one
     *
     * @param other The original exception object to copy
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI Exception(
            const Exception& other) = default;

    /**
     * @brief Copies the statistics_backend::Exception object into the current one
     *
     * @param other The original exception object to copy
     * @return the current statistics_backend::Exception object after the copy
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI Exception& operator =(
            const Exception& other) = default;

    /**
     * @brief Returns the explanatory string of the exception
     *
     * @return Null-terminated string with the explanatory information
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI virtual const char* what() const noexcept override;

protected:

    std::string message_;
};


/**
 * @brief Exception to signal a generic error that falls in no other specific category
 */
class Error : public Exception
{

public:

    /**
     * @brief Construct a new statistics_backend::Error exception
     *
     * @param message The message to be returned by what()
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI Error(
            const char* message) noexcept;

    /**
     * @brief Construct a new statistics_backend::Error exception
     *
     * @param message The message to be returned by what()
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI Error(
            const std::string& message);

    /**
     * @brief Copies the statistics_backend::Error exception into a new one
     *
     * @param other The original exception object to copy
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI Error(
            const Error& other) = default;

    /**
     * @brief Copies the statistics_backend::Error exception into the current one
     *
     * @param other The original statistics_backend::Error exception to copy
     * @return the current statistics_backend::Error exception after the copy
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI Error& operator =(
            const Error& other) = default;
};

/**
 * @brief Exception to signal that an operation is not supported
 */
class Unsupported : public Exception
{

public:

    /**
     * @brief Construct a new statistics_backend::Unsupported exception
     *
     * @param message The message to be returned by what()
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI Unsupported(
            const char* message) noexcept;

    /**
     * @brief Construct a new statistics_backend::Unsupported exception
     *
     * @param message The message to be returned by what()
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI Unsupported(
            const std::string& message);

    /**
     * @brief Copies the statistics_backend::Unsupported exception into a new one
     *
     * @param other The original exception object to copy
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI Unsupported(
            const Unsupported& other) = default;

    /**
     * @brief Copies the statistics_backend::Unsupported exception into the current one
     *
     * @param other The original statistics_backend::Unsupported exception to copy
     * @return the current statistics_backend::Unsupported exception after the copy
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI Unsupported& operator =(
            const Unsupported& other) = default;
};

/**
 * @brief Exception to signal that an operation has been called with an invalid parameter
 */
class BadParameter : public Exception
{

public:

    /**
     * @brief Construct a new statistics_backend::BadParameter exception
     *
     * @param message The message to be returned by what()
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI BadParameter(
            const char* message) noexcept;

    /**
     * @brief Construct a new statistics_backend::BadParameter exception
     *
     * @param message The message to be returned by what()
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI BadParameter(
            const std::string& message);

    /**
     * @brief Copies the statistics_backend::BadParameter exception into a new one
     *
     * @param other The original exception object to copy
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI BadParameter(
            const BadParameter& other) = default;

    /**
     * @brief Copies the statistics_backend::BadParameter exception into the current one
     *
     * @param other The original statistics_backend::BadParameter exception to copy
     * @return the current statistics_backend::BadParameter exception after the copy
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI BadParameter& operator =(
            const BadParameter& other) = default;
};

/**
 * @brief Exception to signal that an operation found no data to return
 */
class NoData : public Exception
{

public:

    /**
     * @brief Construct a new statistics_backend::NoData exception
     *
     * @param message The message to be returned by what()
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI NoData(
            const char* message) noexcept;

    /**
     * @brief Construct a new statistics_backend::NoData exception
     *
     * @param message The message to be returned by what()
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI NoData(
            const std::string& message);

    /**
     * @brief Copies the statistics_backend::NoData exception into a new one
     *
     * @param other The original exception object to copy
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI NoData(
            const NoData& other) = default;

    /**
     * @brief Copies the statistics_backend::NoData exception into the current one
     *
     * @param other The original statistics_backend::NoData exception to copy
     * @return the current statistics_backend::NoData exception after the copy
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI NoData& operator =(
            const NoData& other) = default;
};

/**
 * @brief Exception to signal that a file with an unexpected format has been loaded
 */
class FASTDDS_STATISTICS_BACKEND_DllAPI CorruptedFile : public Exception
{

public:

    /**
     * @brief Construct a new statistics_backend::CorruptedFile exception
     *
     * @param message The message to be returned by what()
     */
    CorruptedFile(
            const char* message) noexcept;

    /**
     * @brief Construct a new statistics_backend::CorruptedFile exception
     *
     * @param message The message to be returned by what()
     */
    CorruptedFile(
            const std::string& message);

    /**
     * @brief Copies the statistics_backend::CorruptedFile exception into a new one
     *
     * @param other The original exception object to copy
     */
    CorruptedFile(
            const CorruptedFile& other) = default;

    /**
     * @brief Copies the statistics_backend::CorruptedFile exception into the current one
     *
     * @param other The original statistics_backend::CorruptedFile exception to copy
     * @return the current statistics_backend::CorruptedFile exception after the copy
     */
    CorruptedFile& operator =(
            const CorruptedFile& other) = default;
};

} // namespace statistics_backend
} // namespace eprosima


#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_EXCEPTION_EXCEPTION_HPP_

