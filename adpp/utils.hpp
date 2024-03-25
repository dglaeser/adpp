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

    template<typename op, typename T>
    static constexpr auto reduce_with(op&& action, T&& value) {
        return _reduce_with(std::forward<op>(action), std::forward<T>(value), v...);
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

 private:
    template<typename op, typename T>
    static constexpr auto _reduce_with(op&&, T&& initial) noexcept {
        return std::forward<T>(initial);
    }

    template<typename op, typename T, typename V0, typename... V>
    static constexpr auto _reduce_with(op&& action, T&& initial, V0&& v0, V&&... values) noexcept {
        auto next = action(std::forward<T>(initial), std::forward<V0>(v0));
        if constexpr (sizeof...(V) == 0) {
            return next;
        } else {
            return _reduce_with(std::forward<op>(action), std::move(next), std::forward<V>(values)...);
        }
    }
};

template<auto... v>
inline constexpr value_list<v...> value_list_v;


#ifndef DOXYGEN
namespace detail {

    template<typename T>
    struct is_value_list : std::false_type {};

    template<auto... v>
    struct is_value_list<value_list<v...>> : std::true_type {};

    template<std::size_t, std::size_t, typename head, typename tail, auto...>
    struct split_at;

    template<std::size_t n, std::size_t i, auto... h, auto... t, auto v0, auto... v>
        requires(i < n)
    struct split_at<n, i, value_list<h...>, value_list<t...>, v0, v...>
    : split_at<n, i+1, value_list<h..., v0>, value_list<t...>, v...> {};

    template<std::size_t n, std::size_t i, auto... h, auto... t, auto v0, auto... v>
        requires(i >= n)
    struct split_at<n, i, value_list<h...>, value_list<t...>, v0, v...>
    : split_at<n, i+1, value_list<h...>, value_list<t..., v0>, v...> {};

    template<std::size_t n, std::size_t i, auto... h, auto... t>
    struct split_at<n, i, value_list<h...>, value_list<t...>> {
        using head = value_list<h...>;
        using tail = value_list<t...>;
    };

    template<std::size_t, typename>
    struct split_at_impl;
    template<std::size_t n, auto... v>
    struct split_at_impl<n, value_list<v...>> : detail::split_at<n, 0, value_list<>, value_list<>, v...> {};

}  // namespace detail
#endif  // DOXYGEN

//! Metafunction to split a value_list at the given index into head & tail lists
template<std::size_t n, typename values>
    requires(detail::is_value_list<values>::value and values::size >= n)
struct split_at : detail::split_at_impl<n, values> {};

//! Metafunction to drop the first n values in a list
template<std::size_t n, typename values>
struct drop_n : std::type_identity<typename split_at<n, values>::tail> {};
template<std::size_t n, typename values>
using drop_n_t = typename drop_n<n, values>::type;

//! Metafunction to crop the last n values in a list
template<std::size_t n, typename values>
    requires(detail::is_value_list<values>::value and values::size >= n)
struct crop_n : std::type_identity<typename split_at<values::size - n, values>::head> {};
template<std::size_t n, typename values>
using crop_n_t = typename crop_n<n, values>::type;

//! \} group Utilities

}  // namespace adpp
