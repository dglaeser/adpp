// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Utilities
 * \brief Utility classes and concepts.
 */

#pragma once

#include <type_traits>
#include <ostream>

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


#ifndef DOXYGEN
namespace detail {

    template<std::size_t i, auto v>
    struct value_i {
        static constexpr auto at(index_constant<i>) {
            return v;
        }
    };

    template<typename I, auto...> struct values;
    template<std::size_t... i, auto... v> requires(sizeof...(i) == sizeof...(v))
    struct values<std::index_sequence<i...>, v...> : value_i<i, v>... {
        using value_i<i, v>::at...;
    };

}  // namespace detail
#endif  // DOXYGEN

//! class to represent a list of values.
template<auto... v>
struct value_list : detail::values<std::make_index_sequence<sizeof...(v)>, v...> {
    static constexpr std::size_t size = sizeof...(v);

    template<std::size_t i> requires(i < sizeof...(v))
    static constexpr auto at(index_constant<i> idx) {
        using base = detail::values<std::make_index_sequence<sizeof...(v)>, v...>;
        return base::at(idx);
    }

    template<auto... _v>
    constexpr auto operator+(const value_list<_v...>&) const {
        return value_list<v..., _v...>{};
    }

    template<auto... _v>
    constexpr bool operator==(const value_list<_v...>&) const {
        if constexpr (sizeof...(_v) == size)
            return std::conjunction_v<is_equal<v, _v>...>;
        return false;
    }

    friend std::ostream& operator<<(std::ostream& s, const value_list&) {
        s << "[";
        (s << ... << ((v == at(indices::i<0>) ? "" : ", ") + std::to_string(v)));
        s << "]";
        return s;
    }
};

template<std::size_t... v>
inline constexpr value_list<v...> value_list_v;

//! \} group Utilities

}  // namespace adpp
