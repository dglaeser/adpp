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

template<template<typename> typename filter, typename... Ts>
struct filtered_types : detail::filtered_types_impl<filter, type_list<Ts...>, type_list<>> {};
template<template<typename> typename filter, typename... Ts>
struct filtered_types<filter, type_list<Ts...>> : detail::filtered_types_impl<filter, type_list<Ts...>, type_list<>> {};
template<template<typename> typename filter, typename... Ts>
using filtered_types_t = typename filtered_types<filter, Ts...>::type;

}  // namespace adpp
