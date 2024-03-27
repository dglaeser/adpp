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

//! type trait that signals if a is less or equal to b.
template<auto a, auto b>
struct is_less_equal : std::bool_constant<(a <= b)> {};
template<auto a, auto b>
inline constexpr bool is_less_equal_v = is_less_equal<a, b>::value;

//! class to represent an index at compile-time.
template<std::size_t i>
struct index_constant : std::integral_constant<std::size_t, i> {
    template<std::size_t o>
    constexpr auto operator<=>(index_constant<o>) const { return i <=> o; }
    constexpr auto operator<=>(std::size_t o) const { return i <=> o; }
    static constexpr auto incremented() { return index_constant<i+1>{}; }
};

template<std::size_t idx>
inline constexpr index_constant<idx> index;


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
        (s << ... << ((v == at(index<0>) ? "" : ", ") + std::to_string(v)));
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

//! class representing multi-dimensional indices
template<std::size_t... i>
struct md_index_constant {
    using as_value_list = value_list<i...>;

    static constexpr std::size_t dimension = sizeof...(i);

    template<std::size_t _i>
    constexpr md_index_constant(index_constant<_i>) requires(sizeof...(i) == 1) {}
    template<std::size_t... _i>
    constexpr md_index_constant(value_list<_i...>) requires(std::conjunction_v<is_equal<i, _i>...>) {}
    constexpr md_index_constant() = default;

    template<std::size_t idx> requires(idx < dimension)
    static constexpr auto at(index_constant<idx> _i) noexcept {
        return as_value_list::at(_i);
    }

    static constexpr std::size_t first() noexcept requires(sizeof...(i) > 0) {
        return as_value_list::at(index<0>);
    }

    static constexpr std::size_t last() noexcept requires(sizeof...(i) > 0) {
        return as_value_list::at(index<sizeof...(i) - 1>);
    }

    template<std::size_t idx>
    static constexpr auto operator[](index_constant<idx> _i = {}) noexcept {
        return value_list<index_constant<i>{}...>::at(_i);
    }

    template<std::size_t idx>
    static constexpr auto with_prepended(index_constant<idx>) { return md_index_constant<idx, i...>{}; }
    template<std::size_t idx>
    static constexpr auto with_appended(index_constant<idx>) { return md_index_constant<i..., idx>{}; }
    template<std::size_t... _i>
    static constexpr auto with_appended(md_index_constant<_i...>) {
        return adpp::md_index_constant{value_list<i..., _i...>{}};
    }

    template<std::size_t pos, std::size_t idx> requires(pos < dimension)
    static constexpr auto with_index_at(index_constant<pos>, index_constant<idx>) {
        using split = split_at<pos, value_list<i...>>;
        using tail = drop_n_t<1, typename split::tail>;
        return adpp::md_index_constant{typename split::head{} + value_list<idx>{} + tail{}};
    }

    template<std::size_t... _n>
    constexpr bool operator==(const md_index_constant<_n...>&) const { return false; }
    constexpr bool operator==(const md_index_constant&) const { return true; }
};

template<std::size_t i>
md_index_constant(index_constant<i>) -> md_index_constant<i>;
template<std::size_t... i>
md_index_constant(value_list<i...>) -> md_index_constant<i...>;

template<std::size_t... i>
inline constexpr md_index_constant<i...> md_index;

//! class representing the shape of a multidimensional array
template<std::size_t... n>
struct md_shape {
    using as_value_list = adpp::value_list<n...>;

    static constexpr std::size_t dimension = sizeof...(n);
    static constexpr std::size_t count = value_list<n...>::reduce_with(std::multiplies<void>{}, dimension > 0 ? 1 : 0);

    static constexpr std::size_t first() noexcept requires(dimension > 0) {
        return value_list<n...>::at(index<0>);
    }

    static constexpr std::size_t last() noexcept requires(dimension > 0) {
        return value_list<n...>::at(index<sizeof...(n)-1>);
    }

    constexpr md_shape() = default;
    constexpr md_shape(value_list<n...>) noexcept {}

    template<std::size_t idx>
    static constexpr auto extent_in(index_constant<idx> i) noexcept {
        return as_value_list::at(i);
    }

    template<std::integral... I>
        requires(sizeof...(I) == dimension)
    constexpr std::size_t flat_index_of(I&&... indices) const noexcept {
        if constexpr (dimension != 0)
            return _to_flat_index<n...>(0, std::forward<I>(indices)...);
        return 0;
    }

    template<std::size_t... i>
        requires(sizeof...(i) == dimension)
    constexpr auto flat_index_of(md_index_constant<i...>) const noexcept {
        return index<_to_flat_index<n...>(0, i...)>;
    }

    template<std::size_t... _n>
    constexpr bool operator==(const md_shape<_n...>&) const noexcept { return false; }
    constexpr bool operator==(const md_shape&) const noexcept { return true; }

 private:
    template<std::size_t _n0, std::size_t... _n, std::integral I0, std::integral... I>
    static constexpr auto _to_flat_index(std::size_t current, const I0& i0, I&&... indices) noexcept {
        if constexpr (sizeof...(I) == 0)
            return current + i0;
        else
            return _to_flat_index<_n...>(
                current + i0*value_list<_n...>::reduce_with(std::multiplies<void>{}, 1),
                std::forward<I>(indices)...
            );
    }
};

template<std::size_t... n>
inline constexpr md_shape<n...> shape;

//! Allows iteration over the indices in an md_shape at compile time, starting from a given index
template<typename shape, typename md_index_current>
struct md_index_constant_iterator;

template<std::size_t... n, std::size_t... i>
    requires(sizeof...(n) == sizeof...(i) and std::conjunction_v<is_less_equal<i, n>...>)
struct md_index_constant_iterator<md_shape<n...>, md_index_constant<i...>> {
    constexpr md_index_constant_iterator(md_shape<n...>) {};
    constexpr md_index_constant_iterator(md_shape<n...>, md_index_constant<i...>) {};

    static constexpr auto current() noexcept {
        return md_index_constant<i...>{};
    }

    static constexpr bool is_end() noexcept {
        if constexpr (sizeof...(n) != 0)
            return value_list<i...>::at(index_constant<0>{}) >= value_list<n...>::at(index_constant<0>{});
        return true;
    }

    static constexpr auto next() noexcept {
        static_assert(!is_end());
        return adpp::md_index_constant_iterator{
            md_shape<n...>{},
            _increment<sizeof...(n)-1, true>(md_index_constant<>{})
        };
    }

 private:
    template<std::size_t dimension_to_increment, bool increment, std::size_t... collected>
    static constexpr auto _increment(md_index_constant<collected...>&& tmp) noexcept {
        const index_constant<dimension_to_increment> inc_pos;
        const auto _recursion = [] <bool keep_incrementing> (std::bool_constant<keep_incrementing>, auto&& r) {
            if constexpr (dimension_to_increment == 0)
                return std::move(r);
            else
                return _increment<dimension_to_increment-1, keep_incrementing>(std::move(r));
        };
        if constexpr (increment) {
            auto incremented = current()[inc_pos].incremented();
            if constexpr (incremented.value >= md_shape<n...>::extent_in(inc_pos) && dimension_to_increment > 0)
                return _recursion(std::bool_constant<true>(), tmp.with_prepended(index<0>));
            else
                return _recursion(std::bool_constant<false>{}, tmp.with_prepended(incremented));
        } else {
            return _recursion(std::bool_constant<false>{}, tmp.with_prepended(current()[inc_pos]));
        }
    }
};

template<std::size_t... n, std::size_t... i>
md_index_constant_iterator(md_shape<n...>, md_index_constant<i...>)
    -> md_index_constant_iterator<md_shape<n...>, md_index_constant<i...>>;
template<std::size_t... n>
md_index_constant_iterator(md_shape<n...>)
    -> md_index_constant_iterator<md_shape<n...>, md_index_constant<(n*0)...>>;

//! \} group Utilities

}  // namespace adpp
