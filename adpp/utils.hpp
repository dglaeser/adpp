// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Utilities
 * \brief Utility classes.
 */

#pragma once

#include <type_traits>
#include <algorithm>
#include <ostream>
#include <numeric>
#include <array>

namespace adpp {

//! \addtogroup Utilities
//! \{


#ifndef DOXYGEN
namespace detail {
    template<typename T, std::size_t s = sizeof(T)>
    std::false_type is_incomplete(T*);
    std::true_type is_incomplete(...);
}  // namespace detail
#endif  // DOXYGEN

// type trait to check if a type is complete (e.g. not just forward declared)
template<typename T>
struct is_complete : std::bool_constant<!decltype(detail::is_incomplete(std::declval<T*>()))::value> {};
template<typename T>
inline constexpr bool is_complete_v = is_complete<T>::value;

//! Can be used to signal e.g. automatic type deduction.
struct automatic {};

//! Null type
struct none {};

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

//! type trait that signals if a type is a scalar
template<typename T>
struct is_scalar : std::bool_constant<std::floating_point<T> || std::integral<T>> {};
template<typename T>
inline constexpr bool is_scalar_v = is_scalar<T>::value;

//! type trait that signals if std::remove_cvref_t of A and B are the same
template<typename A, typename B>
struct is_same_cvref : std::is_same<std::remove_cvref_t<A>, std::remove_cvref_t<B>> {};
template<typename A, typename B>
inline constexpr bool is_same_cvref_v = is_same_cvref<A, B>::value;

//! A type to carry a list of types
template<typename... Ts>
struct type_list {
    static constexpr std::size_t size = sizeof...(Ts);
};

//! A type trait to extract the first type of a type_list
template<typename... T>
struct first_type;
template<typename T, typename... Ts>
struct first_type<type_list<T, Ts...>> : std::type_identity<T> {};
template<typename... T>
using first_type_t = typename first_type<T...>::type;

//! A type trait to remove the first type from a type_list
template<typename... T>
struct drop_first_type;
template<typename T, typename... Ts>
struct drop_first_type<type_list<T, Ts...>> : std::type_identity<type_list<Ts...>> {};
template<typename... T>
using drop_first_type_t = typename drop_first_type<T...>::type;

//! A type trait to check if the type T is contained in the given list of types
template<typename T, typename... Ts>
struct is_any_of : std::bool_constant<std::disjunction_v<std::is_same<T, Ts>...>> {};
template<typename T, typename... Ts>
struct is_any_of<T, type_list<Ts...>> : is_any_of<T, Ts...> {};
template<typename T, typename... Ts>
inline constexpr bool is_any_of_v = is_any_of<T, Ts...>::value;

//! A type trait to check if any of the decay_t of the given types is equal to the decay_t of T
template<typename T, typename... Ts>
struct contains_decayed : is_any_of<std::decay_t<T>, std::decay_t<Ts>...> {};
template<typename T, typename... Ts>
struct contains_decayed<T, type_list<Ts...>> : contains_decayed<T, Ts...> {};
template<typename T, typename... Ts>
inline constexpr bool contains_decayed_v = contains_decayed<T, Ts...>::value;

//! A type trait to check if there are no duplicates in the given list of types
template<typename... T>
struct are_unique;
template<typename T1, typename T2, typename... Ts>
struct are_unique<T1, T2, Ts...> {
    static constexpr bool value =
        are_unique<T1, T2>::value &&
        are_unique<T1, Ts...>::value &&
        are_unique<T2, Ts...>::value;
};
template<typename T1, typename T2>
struct are_unique<T1, T2> : std::bool_constant<!std::is_same_v<T1, T2>> {};
template<typename T>
struct are_unique<T> : std::true_type {};
template<>
struct are_unique<> : std::true_type {};
template<typename... T>
struct are_unique<type_list<T...>> : are_unique<T...> {};
template<typename... Ts>
inline constexpr bool are_unique_v = are_unique<Ts...>::value;


#ifndef DOXYGEN
namespace detail {

    template<typename T, typename... Ts>
    struct unique_types {
        using type = std::conditional_t<
            is_any_of_v<T, Ts...>,
            typename unique_types<Ts...>::type,
            typename unique_types<type_list<T>, Ts...>::type
        >;
    };

    template<typename T>
    struct unique_types<T> : std::type_identity<type_list<T>> {};

    template<typename... Ts, typename T, typename... Rest>
    struct unique_types<type_list<Ts...>, T, Rest...> {
        using type = std::conditional_t<
            is_any_of_v<T, Ts...>,
            typename unique_types<type_list<Ts...>, Rest...>::type,
            typename unique_types<type_list<Ts..., T>, Rest...>::type
        >;
    };

    template<typename... Ts>
    struct unique_types<type_list<Ts...>> : std::type_identity<type_list<Ts...>> {};

    template<typename A, typename... B>
    struct merged_types;

    template<typename... As, typename... Bs>
    struct merged_types<type_list<As...>, type_list<Bs...>> {
        using type = type_list<As..., Bs...>;
    };

    template<typename A, typename... Bs>
    struct merged_types<A, type_list<Bs...>>
    : merged_types<type_list<A>, type_list<Bs...>> {};

    template<typename... As, typename B>
    struct merged_types<type_list<As...>, B>
    : merged_types<type_list<As...>, type_list<B>> {};

}  // namespace detail
#endif  // DOXYGEN

//! A type trait to extract a list of unique types from the given types
template<typename T, typename... Ts>
struct unique : detail::unique_types<T, Ts...> {};
template<typename... Ts> requires(sizeof...(Ts) > 0)
struct unique<type_list<Ts...>> : detail::unique_types<Ts...> {};
template<>
struct unique<type_list<>> : std::type_identity<type_list<>> {};
template<typename A, typename... Ts>
using unique_t = typename unique<A, Ts...>::type;

//! A type trait to merge a type into a type_list, or merge two type_list
template<typename A, typename B>
struct merged : detail::merged_types<A, B> {};
template<typename A, typename... Ts>
using merged_t = typename merged<A, Ts...>::type;


#ifndef DOXYGEN
namespace detail {

    template<template<typename> typename filter, typename...>
    struct filtered_types_impl;
    template<template<typename> typename filter, typename T, typename... rest, typename... current>
    struct filtered_types_impl<filter, type_list<T, rest...>, type_list<current...>> {
        using type = std::conditional_t<
            filter<T>::value,
            typename filtered_types_impl<filter, type_list<rest...>, merged_t<type_list<T>, type_list<current...>>>::type,
            typename filtered_types_impl<filter, type_list<rest...>, type_list<current...>>::type
        >;
    };
    template<template<typename> typename filter, typename... current>
    struct filtered_types_impl<filter, type_list<>, type_list<current...>> {
        using type = type_list<current...>;
    };

}  // namespace detail
#endif  // DOXYGEN

//! A type trait to filter a list of types accordint to a predicate
template<template<typename> typename predicate, typename... Ts>
struct filtered : detail::filtered_types_impl<predicate, type_list<Ts...>, type_list<>> {};
template<template<typename> typename predicate, typename... Ts>
struct filtered<predicate, type_list<Ts...>> : detail::filtered_types_impl<predicate, type_list<Ts...>, type_list<>> {};
template<template<typename> typename predicate, typename... Ts>
using filtered_t = typename filtered<predicate, Ts...>::type;


#ifndef DOXYGEN
namespace detail {

    template<typename T, typename... I>
    concept indexable = requires(const T& t, I&&... indices) {
        { t[std::forward<I>(indices)...] };
    };

    template<typename T, std::size_t... i>
        requires(indexable<T, decltype(i)...>)
    inline constexpr auto indexable_nd_check(T&& t, std::integer_sequence<std::size_t, i...>) {
        return t[(i*0)...];
    }

    template<typename T, std::size_t n>
    concept indexable_nd = requires(const T& t) {
        { indexable_nd_check(t, std::make_index_sequence<n>{}) };
    };

    template<typename T>
    concept exposes_value_type = requires { typename T::value_type; };

    template<typename T>
    concept exposes_static_size = requires {
        { T::size() } -> std::convertible_to<std::size_t>;
    };

}  // namespace detail
#endif  // DOXYGEN

//! Type trait that exposes if a type is indexable
template<typename T>
struct is_indexable : std::bool_constant<detail::indexable<T, std::size_t>> {};
template<typename T>
inline constexpr bool is_indexable_v = is_indexable<T>::value;

//! Type trait that exposes if a type is indexable with the given arguuments
template<typename T, typename... I>
struct is_indexable_with : std::bool_constant<detail::indexable<T, I...>> {};
template<typename T, typename... I>
inline constexpr bool is_indexable_with_v = is_indexable_with<T, I...>::value;

//! Type trait that exposes if a type is indexable with the given arguuments
template<typename T, std::size_t n>
struct is_indexable_nd : std::bool_constant<detail::indexable_nd<T, n>> {};
template<typename T, std::size_t n>
inline constexpr bool is_indexable_nd_v = is_indexable_nd<T, n>::value;

//! Type trait to extract a containers value_type
template<typename T>
struct value_type;
template<detail::exposes_value_type T> requires(!detail::indexable<T>)
struct value_type<T> : std::type_identity<typename T::value_type> {};
template<detail::indexable T>
struct value_type<T> : std::remove_cvref<decltype(std::declval<T>()[std::size_t{}])> {};
template<typename T, std::size_t N>
struct value_type<T[N]> : std::type_identity<T> {};
template<typename T>
using value_type_t = typename value_type<T>::type;

//! Type trait to deduce the value_type of a multi-dimensional array
template<typename T>
struct md_value_type;
template<typename T> requires(is_complete_v<value_type<T>> and !is_complete_v<value_type<value_type_t<T>>>)
struct md_value_type<T> : std::type_identity<value_type_t<T>> {};
template<typename T> requires(is_complete_v<value_type<T>> and is_complete_v<value_type<value_type_t<T>>>)
struct md_value_type<T> : md_value_type<value_type_t<T>> {};
template<typename T>
using md_value_type_t = typename md_value_type<T>::type;

//! Type trait to define the (static) size of a container
template<typename T>
struct size_of;
template<typename T, std::size_t N>
struct size_of<std::array<T, N>> : std::integral_constant<std::size_t, N> {};
template<detail::exposes_static_size T>
struct size_of<T> : std::integral_constant<std::size_t, T::size()> {};
template<typename T, std::size_t N>
struct size_of<T[N]> : std::integral_constant<std::size_t, N> {};
template<typename T>
inline constexpr std::size_t size_of_v = size_of<T>::value;

//! Adapter around a type trait to take std::decay_t of the argument type
template<template<typename> typename trait>
struct decayed_arg_trait {
    template<typename T>
    struct type : trait<std::decay_t<T>> {};
};

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

namespace indices {

inline constexpr index_constant<0> _0{};
inline constexpr index_constant<1> _1{};
inline constexpr index_constant<2> _2{};

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

    static constexpr std::size_t first() noexcept requires(dimension > 0) {
        return as_value_list::at(index<0>);
    }

    static constexpr std::size_t last() noexcept requires(dimension > 0) {
        return as_value_list::at(index<sizeof...(i) - 1>);
    }

    static constexpr auto crop_1() noexcept requires(dimension > 0) {
        return adpp::md_index_constant{crop_n_t<1, as_value_list>{}};
    }

    static constexpr auto drop_1() noexcept requires(dimension > 0) {
        return adpp::md_index_constant{drop_n_t<1, as_value_list>{}};
    }

    template<std::size_t idx>
    static constexpr auto operator[](index_constant<idx> _i = {}) noexcept {
        return value_list<index_constant<i>{}...>::at(_i);
    }

    static constexpr auto with_zeroes() { return md_index_constant<(i*0)...>{}; }
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

    static constexpr bool is_vector() noexcept {
        return dimension == 1 || (dimension == 2 && last() == 1);
    }

    static constexpr std::size_t first() noexcept requires(dimension > 0) {
        return value_list<n...>::at(index<0>);
    }

    static constexpr std::size_t last() noexcept requires(dimension > 0) {
        return value_list<n...>::at(index<sizeof...(n)-1>);
    }

    static constexpr auto crop_1() noexcept requires(dimension > 0) {
        return adpp::md_shape{crop_n_t<1, as_value_list>{}};
    }

    static constexpr auto drop_1() noexcept requires(dimension > 0)  {
        return adpp::md_shape{drop_n_t<1, as_value_list>{}};
    }

    constexpr md_shape() = default;
    explicit constexpr md_shape(value_list<n...>) noexcept {}

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

    template<std::size_t... _n>
    constexpr auto operator+(const md_shape<_n...>&) const noexcept {
        return adpp::md_shape<n..., _n...>{};
    }

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

template<std::size_t n>
inline constexpr md_shape<n> length;


#ifndef DOXYGEN
namespace detail {

    template<typename T>
    concept statically_sized_indexable = is_complete_v<size_of<T>> and is_indexable_v<T>;

    template<typename shape, typename T>
    struct shape_of : std::type_identity<shape> {};
    template<std::size_t... n, statically_sized_indexable T>
    struct shape_of<md_shape<n...>, T> : shape_of<md_shape<n..., size_of_v<T>>, value_type_t<T>> {};

}  // namespace detail
#endif  // DOXYGEN

//! Type trait to obtain the shape of a multidimensional array
template<typename T>
struct shape_of;
template<detail::statically_sized_indexable T>
struct shape_of<T> : detail::shape_of<md_shape<>, T> {};
template<typename T> requires(is_complete_v<shape_of<T>>)
using shape_of_t = typename shape_of<T>::type;
template<typename T>
inline constexpr auto shape_of_v = shape_of_t<T>{};

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


//! Metafunction to access multidimensional arrays with md_index_constant
template<typename T>
struct md_access;
template<detail::statically_sized_indexable T>
struct md_access<T> {
    template<std::size_t... i, typename _T>
        requires(shape_of_v<T>.dimension == sizeof...(i) and is_same_cvref_v<T, _T>)
    static constexpr decltype(auto) at(const md_index_constant<i...>& idx, _T& t) {
        return _at<0>(idx, std::forward<_T>(t));
    }

 private:
    template<std::size_t dim, std::size_t... i, typename _T>
    static constexpr decltype(auto) _at(const md_index_constant<i...>& idx, _T&& sub_values) {
        if constexpr (dim >= sizeof...(i)) {
            return std::forward<_T>(sub_values);
        } else {
            static_assert(is_indexable_v<std::remove_cvref_t<_T>>);
            return _at<dim+1>(idx, std::forward<_T>(sub_values)[idx.at(index<dim>)]);
        }
    }
};

template<std::size_t...i, typename T> requires(is_complete_v<md_access<std::remove_cvref_t<T>>>)
inline constexpr decltype(auto) access_with(const md_index_constant<i...>& idx, T&& t) {
    return md_access<std::remove_cvref_t<T>>::at(idx, std::forward<T>(t));
}


//! Perform the given action invoked with each index within the given shape (TODO: constraints)
template<std::size_t... n, typename A>
constexpr void for_each_index_in(const md_shape<n...>& shape, const A& action) {
    const auto _visit = [&] <typename I> (this auto&& self, const I& index_iterator) {
        if constexpr (!I::is_end()) {
            action(index_iterator.current());
            self(index_iterator.next());
        }
    };
    _visit(md_index_constant_iterator{shape});
}

//! Reduce the given value by recursively calling the given action over all indices in the given shape (TODO: constraints)
template<std::size_t... n, typename V, typename A>
constexpr auto reduce_for_each_index_in(const md_shape<n...>& shape, V&& value, const A& action) {
    const auto _visit = [&] <typename _V, typename I> (this auto&& self, _V&& v, const I& index_iterator) {
        if constexpr (I::is_end())
            return std::forward<V>(v);
        else
            return self(action(index_iterator.current(), std::forward<_V>(v)), index_iterator.next());
    };
    return _visit(std::forward<V>(value), md_index_constant_iterator{shape});
}


// TODO: move to concepts
template<typename T>
concept indexable_2d = requires(const T& t) {
    typename std::remove_cvref_t<T>::value_type;
    { t[md_index<0, 0>] } -> std::convertible_to<const typename std::remove_cvref_t<T>::value_type&>;
};


#ifndef DOXYGEN
namespace detail {

    template<std::size_t... n>
    inline constexpr auto md_index_in(md_shape<n...>) {
        return md_index_constant<(n*0)...>{};
    }

    template<auto shape>
    using md_index_in_t = decltype(md_index_in(shape));

}  // namespace detail
#endif  // DOXYGEN

//! Base class for tensors (exposing arithmetic operators)
template<auto my_shape>
struct tensor_base {
    static constexpr auto shape = my_shape;

    //! Multiply this tensor with the given tensor and store the result in the given output tensor
    template<typename Self, detail::statically_sized_indexable In, detail::statically_sized_indexable Out>
        // requires(is_indexable_with_v<Self, detail::md_index_in_t<shape>>)
    constexpr void apply_to(this Self&& self, const In& in, Out& out) noexcept {
        self.template _apply<shape_of_v<Out>>(in, value_type_t<Out>{0}, [&] (auto i, auto&& value) {
            access_with(i, out) = std::move(value);
        });
    }

    //! Multiply this tensor with the given tensor and store the result in the given output tensor
    template<typename Self, detail::statically_sized_indexable In, detail::statically_sized_indexable Out>
        // requires(is_indexable_with_v<Self, detail::md_index_in_t<shape>>)
    constexpr void add_apply_to(this Self&& self, const In& in, Out& out) noexcept {
        self.template _apply<shape_of_v<Out>>(in, value_type_t<Out>{0}, [&] (auto i, auto&& value) {
            access_with(i, out) += std::move(value);
        });
    }

    //! Multiply this tensor with the given tensor and store the result in the given output tensor
    template<typename Self, detail::statically_sized_indexable In>
        // requires(is_indexable_with_v<Self, detail::md_index_in_t<shape>>)
    constexpr auto apply_to(this Self&& self, const In& in) noexcept {
        static_assert(
            shape.dimension == 2 && shape_of_v<In>.dimension == 1,
            "Currently only implemented for matrix-vectors"
        );
        std::array<typename std::remove_cvref_t<Self>::value_type, shape.extent_in(indices::_0)> out;
        self.apply_to(in, out);
        return out;
    }

    //! Scale all values of this tensor with the given factor
    template<typename Self, typename S>
        requires(!std::is_const_v<Self> and is_scalar_v<std::remove_cvref_t<S>>)
    constexpr void scale_with(this Self&& self, S&& s) {
        for_each_index_in(shape, [&] (const auto& i) {
            self[i] *= s;
        });
    }

    //! Return an md_array that contains the values of this array, scaled with the given scalar
    template<typename Self, typename S> requires(is_scalar_v<std::remove_cvref_t<S>>)
    [[nodiscard]] constexpr auto scaled_with(this Self&& self, S&& s) {
        if constexpr (!std::is_lvalue_reference_v<Self>) {
            self.scale_with(s);
            return self;
        } else {
            static_assert(std::is_copy_constructible_v<Self>);
            auto copy = self;
            copy.scale_with(s);
            return copy;
        }
    }

    //! Return the squared l2 norm of this array (only available for vectors)
    template<typename Self>
    constexpr auto l2_norm_squared(this Self&& self) requires(shape.is_vector()) {
        using value_type = typename std::remove_cvref_t<Self>::value_type;
        return reduce_for_each_index_in(shape, value_type{0}, [&] (const auto& i, auto&& v) {
            const auto& v_i = self[i];
            return std::move(v) + v_i*v_i;
        });
    }

 protected:
    template<auto out_shape, typename Self, typename In, typename Z, typename U>
    constexpr void _apply(this Self&& self, const In& in, Z&& zero, const U& update) noexcept {
        static constexpr auto in_shape = shape_of_v<In>;
        static_assert(shape.last() == in_shape.first(), "Tensor dimensions do not match");
        static_assert(out_shape == shape.crop_1() + in_shape.drop_1(), "Output tensor dimensions do not match");

        for_each_index_in(out_shape, [&] <std::size_t... out> (const md_index_constant<out...>& out_index) {
            using split = split_at<shape.dimension - 1, value_list<out...>>;
            static constexpr auto my_head = md_index_constant{typename split::head{}};
            static constexpr auto in_tail = md_index_constant{typename split::tail{}};
            static constexpr auto in_index_for = [] <std::size_t i> (const md_index_constant<i>& reduction_index) {
                if constexpr (in_tail.dimension == 0)
                    return reduction_index;
                else
                    return in_tail.drop_1().with_prepended(reduction_index[indices::_0]);
            };
            update(out_index, reduce_for_each_index_in(
                md_shape<shape.last()>{},
                Z{zero},
                [&] <std::size_t i, typename V> (const md_index_constant<i>& idx, V&& value) {
                    return std::forward<V>(value) +
                        self[my_head.with_appended(index<i>)]
                        *access_with(in_index_for(idx), in);
            }));
        });
    }
};

//! Stores values of type T accessible via indices in the given shape
template<typename T, auto shape>
class md_array : public tensor_base<shape> {
 public:
    using value_type = T;
    constexpr md_array() = default;
    constexpr md_array(std::array<T, shape.count>&& values) noexcept
    : _values{std::move(values)}
    {}

    //! Return the size of this array (only available for vectors)
    static constexpr std::size_t size() noexcept requires(shape.is_vector()) {
        return shape.count;
    }

    //! Return the value at the given md index
    template<typename Self, std::size_t... i>
        requires(sizeof...(i) == shape.dimension)
    constexpr decltype(auto) operator[](this Self&& self, const md_index_constant<i...>&) {
        static_assert(shape.flat_index_of(i...) < shape.count);
        return self._values[shape.flat_index_of(i...)];
    }

    //! Return the value at the given indices
    template<typename Self, std::integral... I>
        requires(sizeof...(I) == shape.dimension)
    constexpr decltype(auto) operator[](this Self&& self, I&&... indices) {
        return self._values[shape.flat_index_of(std::forward<I>(indices)...)];
    }

    //! Return the value at the given index (only available for vectors)
    template<typename Self, std::integral I>
        requires(shape.is_vector())
    constexpr decltype(auto) operator[](this Self&& self, const I& index) {
        return self._values[index];
    }

    template<typename Self> constexpr auto begin(this Self&& self) { return self._values.begin(); }
    template<typename Self> constexpr auto end(this Self&& self) { return self._values.end(); }

 private:
    std::array<T, shape.count> _values;
};

template<typename T, auto shape> requires(shape.is_vector())
struct size_of<md_array<T, shape>> : std::integral_constant<std::size_t, shape.count> {};

//! Class that defines an order (e.g. of differentiation)
template<unsigned int i>
struct order : public std::integral_constant<unsigned int, i> {};

inline constexpr order<1> first_order;
inline constexpr order<2> second_order;
inline constexpr order<3> third_order;

//! Helper class for storing values of type `T` either by reference or by value.
template<typename T>
class storage {
    using stored = std::conditional_t<std::is_lvalue_reference_v<T>, T, std::remove_cvref_t<T>>;

public:
    template<typename _T> requires(std::convertible_to<_T, stored>)
    constexpr explicit storage(_T&& value) noexcept
    : _value{std::forward<_T>(value)}
    {}

    //! Extract the underlying object
    template<typename S> requires(!std::is_lvalue_reference_v<S>)
    constexpr T&& get(this S&& self) noexcept {
        return std::move(self._value);
    }

    //! Get a reference to the underlying object
    template<typename S>
    constexpr auto& get(this S& self) noexcept {
        if constexpr (std::is_const_v<S>)
            return std::as_const(self._value);
        else
            return self._value;
    }

private:
    stored _value;
};

template<typename T>
storage(T&&) -> storage<T>;


#ifndef DOXYGEN
namespace detail {

    template<std::size_t I, typename T>
    struct indexed_element {
        using index = index_constant<I>;

        template<typename _T> requires(is_same_cvref_v<T, _T>)
        constexpr index index_of() const noexcept { return {}; }
        constexpr index index_of(const T&) const noexcept { return {}; }

        constexpr auto make(index) const noexcept requires(std::default_initializable<std::remove_cvref_t<T>>) {
            return std::remove_cvref_t<T>{};
        }
    };

    template<typename... Ts>
    struct indexed;

    template<std::size_t... I, typename... Ts>
    struct indexed<std::index_sequence<I...>, Ts...> : indexed_element<I, Ts>... {
        using indexed_element<I, Ts>::index_of...;
        using indexed_element<I, Ts>::make...;
    };

}  // namespace detail
#endif  // DOXYGEN

//! Tuple-like type that assigns an index to each type in the pack
template<typename... Ts> requires(are_unique_v<Ts...>)
struct indexed : detail::indexed<std::make_index_sequence<sizeof...(Ts)>, Ts...> {};


#ifndef DOXYGEN
namespace detail {

    template<std::size_t I, typename T>
    struct variadic_element {
        using index = index_constant<I>;

        constexpr variadic_element(T t) noexcept : _storage{std::forward<T>(t)} {}

        template<typename _T> requires(is_same_cvref_v<T, _T>)
        constexpr index index_of() const noexcept { return {}; }
        constexpr index index_of(const T&) const noexcept { return {}; }

        constexpr const std::remove_cvref_t<T>& at(const index&) const noexcept {
            return _storage.get();
        }

    private:
        storage<T> _storage;
    };

    template<typename... Ts>
    struct variadic_accessor;

    template<std::size_t... I, typename... Ts>
    struct variadic_accessor<std::index_sequence<I...>, Ts...> : variadic_element<I, Ts>... {
        constexpr variadic_accessor(Ts... ts) noexcept : variadic_element<I, Ts>(std::forward<Ts>(ts))... {}
        using variadic_element<I, Ts>::index_of...;
        using variadic_element<I, Ts>::at...;
    };

}  // namespace detail
#endif  // DOXYGEN

//! Tuple-like class that assigns an index to each stored type.
template<typename... Ts>
    requires(are_unique_v<Ts...>)
struct variadic_accessor : detail::variadic_accessor<std::make_index_sequence<sizeof...(Ts)>, Ts...> {
 private:
    using base = detail::variadic_accessor<std::make_index_sequence<sizeof...(Ts)>, Ts...>;

 public:
    constexpr variadic_accessor(Ts... ts) noexcept
    : base(std::forward<Ts>(ts)...)
    {}
};

template<typename... Ts>
variadic_accessor(Ts&&...) -> variadic_accessor<Ts...>;

//! Factory for std::array from different inputs
template<typename T, std::size_t N>
struct to_array {
    static constexpr auto from(T (&&values)[N]) noexcept {
        std::array<T, N> result;
        std::ranges::move(values, result.begin());
        return result;
    }
};

//! \} group Utilities

}  // namespace adpp
