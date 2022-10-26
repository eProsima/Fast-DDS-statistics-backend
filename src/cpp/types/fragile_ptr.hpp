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

#include <memory>
#include <type_traits>

#include <fastdds_statistics_backend/exception/Exception.hpp>

namespace eprosima {
namespace statistics_backend {
namespace details {

/**
 * @brief This class represents a Smart Pointer that works as a \c weak_ptr but has the API of a \c shared_ptr .
 *
 * A \c fragile_ptr is a Smart Pointer that holds a weak reference to an object of kind \c T .
 * A weak reference implies that there is no assurance that this object will exist (will not be destroyed somewhere
 * else) along the lifetime of this object.
 * This class replicates the API of a shared_ptr and those times that the internal reference is used
 * (operator-> and operator*) it throws an exception if the internal reference has expired.
 *
 * @note This class may seen as dangerous one. But it is actually better than a shared_ptr, as shared does not
 * prevent you to do a operator-> of a nullptr, and this does.
 * @note This class may also seen as a useless one. I agree, but its main use is not to change the whole project
 * all around where shared_ptr was used before.
 *
 * @note This class has been used because internal entities of the database hold references to each other.
 * This implies loops between shared ptrs.
 * These references are supposed to not be destroyed as long as the entity (that holds them) lives, so
 * changing shared for fragile should not change the behaviour.
 * These shared_ptr has been replaced with fragile_ptr so code does not have to change everywhere.
 */
template <typename T>
class fragile_ptr
{

public:

    /////////////////////
    // CONSTRUCTORS

    //! Default constructor to nullptr
    fragile_ptr() noexcept = default;

    //! Default constructors and operator= from other \c fragile_ptr ( \c weak_ptr are copiable and moveable).
    fragile_ptr(
            const fragile_ptr<T>& copy_other) noexcept = default;
    fragile_ptr(
            fragile_ptr<T>&& move_other) noexcept = default;
    fragile_ptr& operator =(
            const fragile_ptr<T>& copy_other) noexcept = default;
    fragile_ptr& operator =(
            fragile_ptr<T>&& move_other) noexcept = default;

    //! Default destructor
    ~fragile_ptr() = default;

    /////////////////////
    // CONSTRUCTORS FROM SHARED PTR

    /**
     * @brief Construct a new fragile ptr object from a shared one
     *
     * It initializes the internal \c weak_ptr with the reference of the shared one.
     *
     * @param shared_reference \c shared_ptr to the reference to point from this.
     */
    fragile_ptr(
            const std::shared_ptr<T>& shared_reference) noexcept
        : reference_(shared_reference)
    {
        // Do nothing
    }

    //! Same as calling basic constructor.
    fragile_ptr(
            std::nullptr_t) noexcept
    {
        // Do nothing
    }

    /**
     * @brief Assign the fragile ptr object from a shared one
     *
     * It initializes the internal \c weak_ptr with the reference of the shared one.
     *
     * @param shared_reference \c shared_ptr to the reference to point from this.
     */
    fragile_ptr& operator =(
            const std::shared_ptr<T>& shared_reference) noexcept
    {
        this->reference_ = shared_reference;
        return *this;
    }

    /////////////////////
    // OPERATOR METHODS

    /**
     * @brief Compare this ptr reference with a shared one.
     *
     * @param other \c shared_ptr to compare with.
     *
     * @todo use if_enabled or somehow limit this cast to only those U that are coherent.
     */
    template <typename U>
    bool operator ==(
            const std::shared_ptr<U>& other) const noexcept
    {
        static_assert(std::is_base_of<U, T>::value, "U must inherit from T");
        return this->lock() == other;
    }

    /////////////////////
    // CAST METHODS

    /**
     * @brief Cast this object to a \c shared_ptr object of type \c U .
     *
     * @todo use if_enabled or somehow limit this cast to only those U that are coherent.
     */
    template <typename U>
    operator std::shared_ptr<U> () const
    {
        static_assert(std::is_base_of<U, T>::value, "U must inherit from T");
        return this->lock();
    }

    //! Cast this object to a \c shared_ptr of the same type.
    operator std::shared_ptr<T>() const noexcept
    {
        return this->lock();
    }

    //! Whether the internal ptr is valid (it has been initialized and has not expired).
    operator bool() const noexcept
    {
        return !reference_.expired();
    }

    /////////////////////
    // WEAK PTR METHODS

    //! \c weak_ptr::expired call.
    bool expired() const noexcept
    {
        return reference_.expired();
    }

    //! \c weak_ptr::lock call.
    std::shared_ptr<T> lock() const noexcept
    {
        return reference_.lock();
    }

    //! \c weak_ptr::reset call.
    void reset() noexcept
    {
        reference_.reset();
    }

    /////////////////////
    // SHARED PTR METHODS

    /**
     * @brief Get the internal reference.
     *
     * @warning The internal reference is not protected. Thus, it could be removed from other thread while using
     * it even if this object still exist. Use \c lock instead for a protected use of the internal value.
     *
     * @return T* internal reference (could be null)
     */
    T* get() const noexcept
    {
        return lock().get();
    }

    /**
     * @brief Get the internal reference by -> operator.
     *
     * @warning The internal reference is not protected. Thus, it could be removed from other thread while using
     * it even if this object still exist. Use \c lock instead for a protected use of the internal value.
     *
     * @return T* internal reference if not expired.
     * @throw \c Inconsistency if the internal reference is not valid.
     */
    T* operator ->() const
    {
        return safety_lock_().operator ->();
    }

    /**
     * @brief Get the internal reference by * operator.
     *
     * @warning The internal reference is not protected. Thus, it could be removed from other thread while using
     * it even if this object still exist. Use \c lock instead for a protected use of the internal value.
     *
     * @return T& internal reference if not expired.
     * @throw \c Inconsistency if the internal reference is not valid.
     */
    T& operator *() const
    {
        return safety_lock_().operator *();
    }

protected:

    /**
     * @brief This method calls \c weak_ptr::lock . In case the reference is not valid anymore
     * it throws an exception to warn about its usage.
     *
     * It is used for those cases where using the internal reference must be protected by an exception call
     * in case of error.
     *
     * @return return of shared_ptr result from lock
     * @throw \c Inconsistency if the internal reference is not valid.
     */
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

    //! Internal weak reference to a ptr.
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

//! Serialize operator giving the address of the internal reference, or nullptr if not valid.
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
        o << f.get();
    }
    return o;
}

} // namespace details
} // namespace statistics_backend
} // namespace eprosima

#endif //_EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_FRAGILEPTR_HPP_
