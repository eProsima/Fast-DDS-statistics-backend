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

    // Use parent constructors.
    using Exception::Exception;

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

    // Use parent constructors.
    using Exception::Exception;

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

    // Use parent constructors.
    using Exception::Exception;

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
 * @brief Exception to signal that an operation cannot be performed because the preconditions are not met
 */
class PreconditionNotMet : public Exception
{

public:

    // Use parent constructors.
    using Exception::Exception;

    /**
     * @brief Copies the statistics_backend::PreconditionNotMet exception into a new one
     *
     * @param other The original exception object to copy
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI PreconditionNotMet(
            const PreconditionNotMet& other) = default;

    /**
     * @brief Copies the statistics_backend::PreconditionNotMet exception into the current one
     *
     * @param other The original statistics_backend::PreconditionNotMet exception to copy
     * @return the current statistics_backend::PreconditionNotMet exception after the copy
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI PreconditionNotMet& operator =(
            const PreconditionNotMet& other) = default;
};

/**
 * @brief Exception to signal that a file with an unexpected format has been loaded
 */
class CorruptedFile : public Exception
{

public:

    // Use parent constructors.
    using Exception::Exception;

    /**
     * @brief Copies the statistics_backend::CorruptedFile exception into a new one
     *
     * @param other The original exception object to copy
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI CorruptedFile(
            const CorruptedFile& other) = default;

    /**
     * @brief Copies the statistics_backend::CorruptedFile exception into the current one
     *
     * @param other The original statistics_backend::CorruptedFile exception to copy
     * @return the current statistics_backend::CorruptedFile exception after the copy
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI CorruptedFile& operator =(
            const CorruptedFile& other) = default;
};

/**
 * @brief Exception to signal that an inconsistency inside the database has been found.
 */
class Inconsistency : public Exception
{

public:

    // Use parent constructors.
    using Exception::Exception;

    /**
     * @brief Copies the statistics_backend::Inconsistency exception into a new one
     *
     * @param other The original exception object to copy
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI Inconsistency(
            const Inconsistency& other) = default;

    /**
     * @brief Copies the statistics_backend::Inconsistency exception into the current one
     *
     * @param other The original statistics_backend::Inconsistency exception to copy
     * @return the current statistics_backend::Inconsistency exception after the copy
     */
    FASTDDS_STATISTICS_BACKEND_DllAPI Inconsistency& operator =(
            const Inconsistency& other) = default;
};

} // namespace statistics_backend
} // namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_EXCEPTION_EXCEPTION_HPP_
