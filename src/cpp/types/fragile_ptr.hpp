// Copyright 2022 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
 * @file fragile_ptr.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_FRAGILEPTR_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_FRAGILEPTR_HPP_

#include <iostream>  // TODO remove
#include <memory>
#include <type_traits>

#include <fastdds_statistics_backend/exception/Exception.hpp>

namespace eprosima {
namespace statistics_backend {
namespace details {

/**
 * TODO add comments
 */
template <typename T>
class fragile_ptr
{

public:

    /////////////////////
    // CONSTRUCTORS

    fragile_ptr() noexcept = default;

    fragile_ptr(const fragile_ptr<T>& copy_other) = default;
    fragile_ptr(fragile_ptr<T>&& move_other) = default;
    fragile_ptr& operator=(const fragile_ptr<T>& copy_other) = default;
    fragile_ptr& operator=(fragile_ptr<T>&& move_other) = default;

    ~fragile_ptr() = default;

    /////////////////////
    // CONSTRUCTORS FROM SHARED PTR

    fragile_ptr(const std::shared_ptr<T>& shared_reference)
        : reference_(shared_reference)
    {
        // Do nothing
    }

    fragile_ptr(std::nullptr_t)
    {
        // Do nothing
    }

    fragile_ptr& operator=(const std::shared_ptr<T>& copy_other)
    {
        this->reference_ = copy_other;
        return *this;
    }

    /////////////////////
    // OPERATOR METHODS

    // template <typename U>
    // bool operator==(const std::shared_ptr<T>& other) const noexcept
    // {
    //     static_assert(std::is_base_of<U, T>::value, "U must inherit from T");
    //     return std::dynamic_pointer_cast<U>(this->safety_lock_()) == other;
    // }

    // TODO: use if_enabled or somehow limit this cast to only those U that are coherent
    template <typename U>
    bool operator==(const std::shared_ptr<U>& other) const noexcept
    {
        static_assert(std::is_base_of<U, T>::value, "U must inherit from T");
        if (other == nullptr)
        {
            return this->expired();
        }
        else
        {
            return this->safety_lock_() == other;
        }
    }

    /////////////////////
    // CAST METHODS

    // TODO: use if_enabled or somehow limit this cast to only those U that are coherent
    template <typename U>
    operator std::shared_ptr<U> () const
    {
        auto cast_result = std::dynamic_pointer_cast<U>(safety_lock_());
        if (!cast_result)
        {
            // TODO: show in error message the name of the types that are being casted
            throw Inconsistency("Trying to cast to a not valid object.");
        }
        else
        {
            return cast_result;
        }
    }

    // TODO: implement it somehow
    // template <typename U>
    // operator
    // typename std::enable_if<std::true_type::value, std::shared_ptr<U>>::type () const
    // {
    //     auto cast_result = std::dynamic_pointer_cast<U>(safety_lock_());
    //     if (!cast_result)
    //     {
    //         // TODO: show in error message the name of the types that are being casted
    //         throw Inconsistency("Trying to cast to a not valid object.");
    //     }
    //     else
    //     {
    //         return cast_result;
    //     }
    // }

    operator std::shared_ptr<T>() const
    {
        return safety_lock_();
    }

    operator bool() const noexcept
    {
        return !reference_.expired();
    }

    /////////////////////
    // INTERACTION METHODS

    std::shared_ptr<T> get_shared_ptr() const
    {
        return safety_lock_();
    }

    /////////////////////
    // WEAK PTR METHODS

    bool expired() const noexcept
    {
        return reference_.expired();
    }

    std::shared_ptr<T> lock() const noexcept
    {
        return reference_.lock();
    }

    void reset() noexcept
    {
        reference_.reset();
    }

    /////////////////////
    // SHARED PTR METHODS

    T* operator->() const
    {
        return safety_lock_().operator->();
    }

    T* get() const
    {
        return safety_lock_().get();
    }

    T& operator*() const
    {
        return safety_lock_().operator*();
    }

protected:

    std::shared_ptr<T> safety_lock_() const
    {
        auto lock_reference = reference_.lock();
        if (!lock_reference)
        {
            throw Inconsistency("Fragile object trying to use an already destroyed reference.");
        }
        else
        {
            return lock_reference;
        }
    }

    std::weak_ptr<T> reference_;

};

//! Allow to compare an fragile_ptr with nullptr
template<typename T>
bool operator ==(
        const fragile_ptr<T>& lhs,
        std::nullptr_t) noexcept
{
    return lhs.lock() == nullptr;
}

//! Allow to compare an fragile_ptr with nullptr in the other direction (from C++20 this is not needed)
template<typename T>
bool operator ==(
        std::nullptr_t,
        const fragile_ptr<T>& lhs) noexcept
{
    return nullptr == lhs.lock();
}

//! Allow to compare an fragile_ptr with nullptr
template<typename T>
bool operator !=(
        const fragile_ptr<T>& lhs,
        std::nullptr_t) noexcept
{
    return lhs.lock() != nullptr;
}

//! Allow to compare an fragile_ptr with nullptr in the other direction (from C++20 this is not needed)
template<typename T>
bool operator !=(
        std::nullptr_t,
        const fragile_ptr<T>& lhs) noexcept
{
    return nullptr != lhs.lock();
}

template <typename T>
std::ostream& operator <<(
        std::ostream& o,
        const fragile_ptr<T>& f)
{
    if (f.expired())
    {
        o << "{nullptr}";
    }
    else
    {
        o << f.lock();
    }
    return o;
}

} // namespace details
} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_FRAGILEPTR_HPP_
