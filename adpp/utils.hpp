// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Utilities
 * \brief Utility classes and concepts.
 */

#pragma once

#include <type_traits>

namespace adpp {

//! \addtogroup Utilities
//! \{

//! Can be used to signal e.g. automatic type deduction.
struct automatic {};

//! A type trait that is always false, independent of the type T.
template<typename T>
struct always_false : std::false_type {};

//! type trait that signals if a is smaller than b.
template<auto a, auto b>
struct is_less : std::bool_constant<(a < b)> {};
template<auto a, auto b>
inline constexpr bool is_less_v = is_less<a, b>::value;

//! type trait that signals if a is equal to b.
template<auto a, auto b>
struct is_equal : std::bool_constant<(a == b)> {};
template<auto a, auto b>
inline constexpr bool is_equal_v = is_equal<a, b>::value;

//! class to represent an index at compile-time.
template<std::size_t i>
struct index_constant : std::integral_constant<std::size_t, i> {
    template<std::size_t o>
    constexpr auto operator<=>(index_constant<o>) const { return i <=> o; }
    constexpr auto operator<=>(std::size_t o) const { return i <=> o; }
    static constexpr auto incremented() { return index_constant<i+1>{}; }
};

namespace indices {

template<std::size_t idx>
inline constexpr index_constant<idx> i;

}  // namespace indices

//! \} group Utilities

}  // namespace adpp
