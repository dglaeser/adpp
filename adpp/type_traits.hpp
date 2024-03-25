#pragma once

#include <ostream>
#include <type_traits>
#include <concepts>
#include <utility>
#include <array>

#include "utils.hpp"

namespace adpp {

template<std::size_t idx>
inline constexpr index_constant<idx> ic;


#ifndef DOXYGEN
namespace detail {

    template<std::size_t i, auto v>
    struct value_i {
        static constexpr auto at(index_constant<i>) {
            return v;
        }
    };

    template<typename I, auto...>
    struct values;
    template<std::size_t... i, auto... v> requires(sizeof...(i) == sizeof...(v))
    struct values<std::index_sequence<i...>, v...> : value_i<i, v>... {
        using value_i<i, v>::at...;
    };

}  // namespace detail
#endif  // DOXYGEN

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
        if constexpr (sizeof...(_v) != size)
            return false;
        else
            return std::conjunction_v<is_equal<v, _v>...>;
    }

    friend std::ostream& operator<<(std::ostream& s, const value_list& vl) {
        s << "[";
        if constexpr (sizeof...(v) > 0)
            vl._push_to<v...>(s);
        s << "]";
        return s;
    }

 private:
    template<auto v0, auto... _v>
    void _push_to(std::ostream& s) const {
        s << v0;
        if constexpr (sizeof...(_v) > 0) {
            s << ", ";
            _push_to<_v...>(s);
        }
    }
};

template<typename T>
struct is_value_list : std::false_type {};
template<auto... v>
struct is_value_list<value_list<v...>> : std::true_type {};
template<typename T>
inline constexpr bool is_value_list_v = is_value_list<T>::value;

template<typename T, std::size_t n>
concept value_list_with_size = is_value_list_v<T> and T::size == n;
template<typename T, std::size_t n>
concept value_list_with_min_size = is_value_list_v<T> and T::size >= n;
template<typename T, std::size_t n>
concept value_list_with_max_size = is_value_list_v<T> and T::size <= n;


#ifndef DOXYGEN
namespace detail {

    template<
        std::size_t,
        std::size_t,
        typename head,
        typename tail,
        auto...>
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

template<std::size_t n, value_list_with_min_size<n> values>
struct split_at : detail::split_at_impl<n, values> {};


template<std::size_t n, value_list_with_min_size<n> values>
struct drop_n : std::type_identity<typename split_at<n, values>::tail> {};
template<std::size_t n, value_list_with_min_size<n> values>
using drop_n_t = typename drop_n<n, values>::type;


template<auto... v> requires(sizeof...(v) > 0)
inline constexpr auto last_value_v = split_at<sizeof...(v)-1, value_list<v...>>::tail::at(ic<0>);
template<auto v0, auto... v>
inline constexpr auto first_value_v = v0;


#ifndef DOXYGEN
namespace detail {

    template<typename op, auto...>
    struct accumulate;
    template<typename op, auto current>
    struct accumulate<op, current> {
        static constexpr auto value = current;
    };
    template<typename op, auto current, auto next, auto... values>
    struct accumulate<op, current, next, values...> {
        static constexpr auto value = accumulate<op, op{}(current, next), values...>::value;
    };

    template<auto... v> struct last_value : std::integral_constant<std::size_t, last_value_v<v...>> {};
    template<> struct last_value<> : std::integral_constant<std::size_t, 0> {};

}  // namespace detail
#endif  // DOXYGEN

template<auto initial, typename op, auto... values>
inline constexpr auto accumulate_v = detail::accumulate<op, initial, values...>::value;


template<std::size_t... n>
struct md_shape {
    static constexpr std::size_t size = sizeof...(n);
    static constexpr std::size_t number_of_elements = size > 0 ? accumulate_v<1, std::multiplies<void>, n...> : 0;
    static constexpr std::size_t last_axis_size = detail::last_value<n...>::value;

    // TODO: remove?
    using as_list = adpp::value_list<n...>;

    constexpr md_shape() = default;
    constexpr md_shape(value_list<n...>) noexcept {}

    template<std::size_t idx>
    static constexpr auto at(index_constant<idx> i) {
        return as_list::at(i);
    }

    template<std::integral... I>
        requires(sizeof...(I) == size)
    friend constexpr std::size_t flat_index(const md_shape& dims, I&&... indices) {
        if constexpr (size == 0)
            return 0;
        else
            return dims._to_flat_index<n...>(0, std::forward<I>(indices)...);
    }

    template<std::size_t... _n>
    constexpr bool operator==(const md_shape<_n...>&) const { return false; }
    constexpr bool operator==(const md_shape&) const { return true; }

 private:
    template<std::size_t _n0, std::size_t... _n, std::integral I0, std::integral... I>
    static constexpr auto _to_flat_index(std::size_t current, const I0& i0, I&&... indices) {
        if constexpr (sizeof...(I) == 0)
            return current + i0;
        else
            return _to_flat_index<_n...>(
                current + i0*accumulate_v<1, std::multiplies<void>, _n...>,
                std::forward<I>(indices)...
            );
    }
};

template<std::size_t... n>
inline constexpr md_shape<n...> shape;


#ifndef DOXYGEN
namespace detail {

    template<typename dims, std::size_t... indices>
    struct flat_index_closure;
    template<std::size_t dim1, std::size_t... dims, std::size_t i0, std::size_t... indices>
    struct flat_index_closure<md_shape<dim1, dims...>, i0, indices...> {
        static constexpr std::size_t value
            = i0*md_shape<dim1, dims...>::number_of_elements
            + flat_index_closure<md_shape<dims...>, indices...>::value;
    };
    template<std::size_t... dims, std::size_t iN>
    struct flat_index_closure<md_shape<dims...>, iN>
    : std::integral_constant<std::size_t, iN> {};

    template<typename dims, std::size_t... indices>
    struct flat_index;
    template<std::size_t dim0, std::size_t... dims, std::size_t... indices>
    struct flat_index<md_shape<dim0, dims...>, indices...> {
        static constexpr std::size_t value = flat_index_closure<md_shape<dims...>, indices...>::value;
    };

}  // namespace detail
#endif  // DOXYGEN


template<std::size_t... i>
struct md_index_constant {
    static constexpr std::size_t size = sizeof...(i);

    // TODO: remove?
    using as_list = value_list<i...>;

    template<std::size_t _i>
    constexpr md_index_constant(index_constant<_i>) requires(sizeof...(i) == 1) {}
    template<std::size_t... _i>
    constexpr md_index_constant(value_list<_i...>) requires(std::conjunction_v<is_equal<i, _i>...>) {}
    constexpr md_index_constant() = default;

    template<std::size_t idx>
    static constexpr auto at(index_constant<idx> _i) {
        return as_list::at(_i);
    }

    static constexpr std::size_t last() {
        return as_list::at(index_constant<sizeof...(i) - 1>{});
    }

    template<std::size_t idx>
    static constexpr auto operator[](index_constant<idx> _i = {}) {
        return value_list<index_constant<i>{}...>::at(_i);
    }

    template<std::size_t... n> requires(sizeof...(n) == size)
    static constexpr auto as_flat_index(const md_shape<n...>&) {
        static_assert(std::conjunction_v<is_less<i, n>...>);
        if constexpr (size == 0)
            return index_constant<0>{};
        else
            return index_constant<detail::flat_index<md_shape<n...>, i...>::value>{};
    }

    template<std::size_t idx>
    static constexpr auto with_prepended(index_constant<idx>) { return md_index_constant<idx, i...>{}; }
    template<std::size_t idx>
    static constexpr auto with_appended(index_constant<idx>) { return md_index_constant<i..., idx>{}; }
    template<std::size_t... _i>
    static constexpr auto with_appended(md_index_constant<_i...>) {
        return adpp::md_index_constant{value_list<i..., _i...>{}};
    }

    template<std::size_t pos, std::size_t idx> requires(pos < size)
    static constexpr auto with_index_at(index_constant<pos>, index_constant<idx>) {
        using split = split_at<pos, value_list<i...>>;
        using tail = drop_n_t<1, typename split::tail>;
        return adpp::md_index_constant{typename split::head{} + value_list<idx>{} + tail{}};
    }

    // TODO: md_index_constant and md_shape have much overlap!
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


template<typename...>
struct md_index_constant_iterator;

template<std::size_t... n, std::size_t... i>
    requires(sizeof...(n) == sizeof...(i))
struct md_index_constant_iterator<md_shape<n...>, md_index_constant<i...>> {
    constexpr md_index_constant_iterator(md_shape<n...>) {};
    constexpr md_index_constant_iterator(md_shape<n...>, md_index_constant<i...>) {};

    static constexpr auto index() {
        return md_index_constant<i...>{};
    }

    static constexpr bool is_end() {
        if constexpr (sizeof...(n) == 0)
            return true;
        else
            return md_index_constant<i...>::at(index_constant<0>{}) >= first_value_v<n...>;
    }

    static constexpr auto next() {
        static_assert(!is_end());
        return adpp::md_index_constant_iterator{
            md_shape<n...>{},
            _increment<sizeof...(n)-1, true>(md_index_constant<>{})
        };
    }

 private:
    template<std::size_t dimension_to_increment, bool increment, std::size_t... collected>
    static constexpr auto _increment(md_index_constant<collected...>&& tmp) {
        const auto _recursion = [] <bool keep_incrementing> (std::bool_constant<keep_incrementing>, auto&& r) {
            if constexpr (dimension_to_increment == 0)
                return std::move(r);
            else
                return _increment<dimension_to_increment-1, keep_incrementing>(std::move(r));
        };
        if constexpr (increment) {
            auto incremented = index()[index_constant<dimension_to_increment>()].incremented();
            if constexpr (incremented.value >= md_shape<n...>::at(index_constant<dimension_to_increment>{})
                            && dimension_to_increment > 0)
                return _recursion(std::bool_constant<true>(), tmp.with_prepended(index_constant<0>{}));
            else
                return _recursion(std::bool_constant<false>{}, tmp.with_prepended(incremented));
        } else {
            return _recursion(std::bool_constant<false>{}, tmp.with_prepended(
                index()[index_constant<dimension_to_increment>()])
            );
        }
    }
};

template<std::size_t... n, std::size_t... i>
md_index_constant_iterator(md_shape<n...>, md_index_constant<i...>)
    -> md_index_constant_iterator<md_shape<n...>, md_index_constant<i...>>;
template<std::size_t... n>
md_index_constant_iterator(md_shape<n...>)
    -> md_index_constant_iterator<md_shape<n...>, md_index_constant<(n*0)...>>;


template<template<typename> typename trait>
struct decayed_trait {
    template<typename T>
    struct type : trait<std::decay_t<T>> {};
};

template<typename... Ts>
struct type_list {};

template<typename T>
struct type_list_size;
template<typename... T>
struct type_list_size<type_list<T...>> : std::integral_constant<std::size_t, sizeof...(T)> {};
template<typename T>
inline constexpr std::size_t type_list_size_v = type_list_size<T>::value;


template<typename T>
struct value_type;
template<typename T, std::size_t N>
struct value_type<std::array<T, N>> : std::type_identity<T> {};
template<typename T, std::size_t N>
struct value_type<T[N]> : std::type_identity<T> {};
template<typename T>
using value_type_t = typename value_type<T>::type;


template<typename T>
struct static_size;
template<typename T, std::size_t N>
struct static_size<std::array<T, N>> : std::integral_constant<std::size_t, N> {};
template<typename T>
inline constexpr std::size_t static_size_v = static_size<T>::value;


#ifndef DOXYGEN
namespace detail {

    template<typename T, std::size_t s = sizeof(T)>
    std::false_type is_incomplete(T*);
    std::true_type is_incomplete(...);

}  // end namespace detail
#endif  // DOXYGEN

template<typename T>
struct is_complete : std::bool_constant<!decltype(detail::is_incomplete(std::declval<T*>()))::value> {};
template<typename T>
inline constexpr bool is_complete_v = is_complete<T>::value;


template<typename... T>
struct first_type;
template<typename T, typename... Ts>
struct first_type<type_list<T, Ts...>> : std::type_identity<T> {};
template<typename... T>
using first_type_t = typename first_type<T...>::type;


template<typename... T>
struct drop_first_type;
template<typename T, typename... Ts>
struct drop_first_type<type_list<T, Ts...>> : std::type_identity<type_list<Ts...>> {};
template<typename... T>
using drop_first_type_t = typename drop_first_type<T...>::type;


template<typename T, typename... Ts>
struct is_any_of : std::bool_constant<std::disjunction_v<std::is_same<T, Ts>...>> {};
template<typename T, typename... Ts>
struct is_any_of<T, type_list<Ts...>> : is_any_of<T, Ts...> {};
template<typename T, typename... Ts>
inline constexpr bool is_any_of_v = is_any_of<T, Ts...>::value;


template<typename T, typename... Ts>
struct contains_decayed : is_any_of<std::decay_t<T>, std::decay_t<Ts>...> {};
template<typename T, typename... Ts>
struct contains_decayed<T, type_list<Ts...>> : contains_decayed<T, Ts...> {};
template<typename T, typename... Ts>
inline constexpr bool contains_decayed_v = contains_decayed<T, Ts...>::value;


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

    template<typename A, typename B>
    struct merged_types;

    template<typename... As, typename... Bs>
    struct merged_types<type_list<As...>, type_list<Bs...>> {
        using type = type_list<As..., Bs...>;
    };

}  // namespace detail
#endif  // DOXYGEN


template<typename T, typename... Ts>
struct unique_types : detail::unique_types<T, Ts...> {};
template<typename... Ts> requires(sizeof...(Ts) > 0)
struct unique_types<type_list<Ts...>> : detail::unique_types<Ts...> {};
template<>
struct unique_types<type_list<>> : std::type_identity<type_list<>> {};
template<typename A, typename... Ts>
using unique_types_t = typename unique_types<A, Ts...>::type;

template<typename A, typename... Ts>
struct merged_types : detail::merged_types<A, Ts...> {};
template<typename A, typename... Ts>
using merged_types_t = typename merged_types<A, Ts...>::type;


#ifndef DOXYGEN
namespace detail {

    template<template<typename> typename filter, typename...>
    struct filtered_types_impl;
    template<template<typename> typename filter, typename T, typename... rest, typename... current>
    struct filtered_types_impl<filter, type_list<T, rest...>, type_list<current...>> {
        using type = std::conditional_t<
            filter<T>::value,
            typename filtered_types_impl<filter, type_list<rest...>, merged_types_t<type_list<T>, type_list<current...>>>::type,
            typename filtered_types_impl<filter, type_list<rest...>, type_list<current...>>::type
        >;
    };
    template<template<typename> typename filter, typename... current>
    struct filtered_types_impl<filter, type_list<>, type_list<current...>> {
        using type = type_list<current...>;
    };

}  // namespace detail
#endif  // DOXYGEN

template<template<typename> typename filter, typename... Ts>
struct filtered_types : detail::filtered_types_impl<filter, type_list<Ts...>, type_list<>> {};
template<template<typename> typename filter, typename... Ts>
struct filtered_types<filter, type_list<Ts...>> : detail::filtered_types_impl<filter, type_list<Ts...>, type_list<>> {};
template<template<typename> typename filter, typename... Ts>
using filtered_types_t = typename filtered_types<filter, Ts...>::type;

}  // namespace adpp
