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
 * @file Bitmask.hpp
 */

#ifndef _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_BITMASK_HPP_
#define _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_BITMASK_HPP_


namespace eprosima {
namespace statistics_backend {

/**
 * @brief Generic bitmask for an enumerated type
 * 
 * This class can be used as a companion bitmask of any enumerated type whose values
 * have been constructed so that a single bit is set for each enum value.
 * The enumerated values can be seen as the names of the bits in the bitmask.
 * 
 * Bitwise operations are defined between masks of the same type, between
 * a mask and its companion ennumeration, and between enumerated values.
 * 
 * \code{.cpp}
 *     enum my_enum
 *     {
 *         RED    = 1 << 0,
 *         GREEN  = 1 << 1,
 *         BLUE   = 1 << 2
 *     };
 * 
 *     // Combine enumerated labels to create a mask
 *     Bitmask<my_enum> yellow_mask = RED | GREEN;
 * 
 *     // Combine a mask with a value to create a new mask
 *     Bitmask<my_enum> white_mask = yellow_mask | BLUE;
 * 
 *     // Flip all the bits in the mask
 *     Bitmask<my_enum> black_mask = ~white_mask;
 * 
 *     // Set a bit in the mask
 *     black_mask.set(RED);
 * 
 *     // Test if a bit is set in the mask
 *     bool is_red = white_mask.is_set(RED);
 * \endcode
 * 
 * @tparam E The enumerated type for which the bitmask is constructed
 */
template <typename E>
class Bitmask
{

private:

    using underlying_type = std::underlying_type_t<E>;

public:

    Bitmask()
        : mask_(0)
    {
    }

    Bitmask(
            const E& value)
        : mask_(static_cast<underlying_type>(value))
    {
    }

    Bitmask(
            const Bitmask<E>& other) = default;

    Bitmask<E>& operator=(
            const Bitmask<E>& other) = default;

    void set (const E& value)
    {
        mask_ |= static_cast<underlying_type>(value);
    }

    void clear (const E& value)
    {
        mask_ &= ~static_cast<underlying_type>(value);
    }

    bool is_set(const E& value) const
    {
        underlying_type v = static_cast<underlying_type>(value);
        return (mask_ & v) == v;
    }

    static constexpr Bitmask none()
    {
        return Bitmask(0);
    }

    static constexpr Bitmask all()
    {
        return Bitmask(static_cast<underlying_type>(-1));
    }

    Bitmask<E>& operator&= (const Bitmask& other)
    {
        mask_ &= other.mask_;
        return *this;
    }

    Bitmask<E>& operator&= (const E& value)
    {
        mask_ &= static_cast<underlying_type>(value);
        return *this;
    }

    Bitmask<E> operator& (const Bitmask& other) const
    {
        Bitmask result = *this;
        return result &= other;
    }

    Bitmask<E> operator& (const E& value) const
    {
        Bitmask result = *this;
        return result &= value;
    }

    Bitmask<E>& operator|= (const Bitmask& other)
    {
        mask_ |= other.mask_;
        return *this;
    }

    Bitmask<E>& operator|= (const E& value)
    {
        mask_ |= static_cast<underlying_type>(value);
        return *this;
    }

    Bitmask<E> operator| (const Bitmask& other) const
    {
        Bitmask result = *this;
        return result |= other;
    }

    Bitmask<E> operator| (const E& value) const
    {
        Bitmask result = *this;
        return result |= value;
    }

    Bitmask<E>& operator^= (const Bitmask& other)
    {
        mask_ ^= other.mask_;
        return *this;
    }

    Bitmask<E>& operator^= (const E& value)
    {
        mask_ ^= static_cast<underlying_type>(value);
        return *this;
    }

    Bitmask<E> operator^ (const Bitmask& other) const
    {
        Bitmask result = *this;
        return result ^= other;
    }

    Bitmask<E> operator| (const Bitmask& other) const
    {
        Bitmask result = *this;
        return result |= other;
    }

    Bitmask<E> operator~ () const
    {
        Bitmask result(~mask_);
        return result;
    }

private:

    Bitmask(
            underlying_type mask)
        : mask_(mask)
    {
    }

    underlying_type mask_;
};

template<typename E>
Bitmask<E> operator& (const E& lhs, const E& rhs)
{
    Bitmask<E> result (lhs);
    return result &= rhs;
}

template<typename E>
Bitmask<E> operator| (const E& lhs, const E& rhs)
{
    Bitmask<E> result (lhs);
    return result |= rhs;
}

template<typename E>
Bitmask<E> operator^ (const E& lhs, const E& rhs)
{
    Bitmask<E> result (lhs);
    return result ^= rhs;
}


} //namespace statistics_backend
} //namespace eprosima

#endif // _EPROSIMA_FASTDDS_STATISTICS_BACKEND_TYPES_BITMASK_HPP_
