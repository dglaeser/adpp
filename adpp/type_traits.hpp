#pragma once

#include <ostream>
#include <type_traits>
#include <concepts>
#include <utility>
#include <array>

#include "utils.hpp"

namespace adpp {


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
