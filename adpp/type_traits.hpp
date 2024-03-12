#pragma once

#include <type_traits>
#include <utility>


namespace adpp {

struct automatic {};

template<typename... Ts>
struct type_list {};


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


template<std::size_t i>
using index_constant = std::integral_constant<std::size_t, i>;


template<template<typename> typename trait>
struct decayed_arg {
    template<typename T>
    struct type : trait<std::decay_t<T>> {};
};


template<typename T>
struct type_size;
template<typename... Ts>
struct type_size<type_list<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};
template<typename T> requires(is_complete_v<type_size<T>>)
inline constexpr std::size_t type_size_v = type_size<T>::value;


template<typename... T>
struct first_type;
template<typename T, typename... Ts>
struct first_type<type_list<T, Ts...>> : std::type_identity<T> {};
template<typename... T>
using first_type_t = typename first_type<T...>::type;


template<typename T>
struct is_ownable : std::bool_constant<!std::is_lvalue_reference_v<T>> {};
template<typename T>
inline constexpr bool is_ownable_v = is_ownable<T>::value;


template<typename T, typename... Ts>
struct is_any_of : std::bool_constant<std::disjunction_v<std::is_same<T, Ts>...>> {};
template<typename T, typename... Ts>
struct is_any_of<T, type_list<Ts...>> : is_any_of<T, Ts...> {};
template<typename T, typename... Ts>
inline constexpr bool is_any_of_v = is_any_of<T, Ts...>::value;


template<typename T, typename... Ts>
struct contains_decay : is_any_of<std::decay_t<T>, std::decay_t<Ts>...> {};
template<typename T, typename... Ts>
struct contains_decay<T, type_list<Ts...>> : contains_decay<T, Ts...> {};
template<typename T, typename... Ts>
inline constexpr bool contains_decay_v = contains_decay<T, Ts...>::value;


template<typename... T>
struct are_unique;
template<typename T1, typename T2, typename... Ts>
struct are_unique<T1, T2, Ts...> {
    static constexpr bool value =
        are_unique<T1, T2>::value &&
        are_unique<T1, Ts...>::value &&
        are_unique<T2, Ts...>::value;
};
template<typename T1, typename T2> struct are_unique<T1, T2> : std::bool_constant<!std::is_same_v<T1, T2>> {};
template<typename T> struct are_unique<T> : std::true_type {};
template<> struct are_unique<> : std::true_type {};
template<typename... T> struct are_unique<type_list<T...>> : are_unique<T...> {};
template<typename... Ts>
inline constexpr bool are_unique_v = are_unique<Ts...>::value;


#ifndef DOXYGEN
namespace detail {

    template<typename T, typename... Ts>
    struct unique_tuple {
        using type = std::conditional_t<
            is_any_of_v<T, Ts...>,
            typename unique_tuple<Ts...>::type,
            typename unique_tuple<type_list<T>, Ts...>::type
        >;
    };

    template<typename T>
    struct unique_tuple<T> : std::type_identity<type_list<T>> {};

    template<typename... Ts, typename T, typename... Rest>
    struct unique_tuple<type_list<Ts...>, T, Rest...> {
        using type = std::conditional_t<
            is_any_of_v<T, Ts...>,
            typename unique_tuple<type_list<Ts...>, Rest...>::type,
            typename unique_tuple<type_list<Ts..., T>, Rest...>::type
        >;
    };

    template<typename... Ts>
    struct unique_tuple<type_list<Ts...>> : std::type_identity<type_list<Ts...>> {};

    template<typename A, typename B>
    struct merged_tuple;

    template<typename... As, typename... Bs>
    struct merged_tuple<type_list<As...>, type_list<Bs...>> {
        using type = type_list<As..., Bs...>;
    };

}  // namespace detail
#endif  // DOXYGEN


// TODO: Rename, these arent tuples...s
template<typename T, typename... Ts>
struct unique_tuple : detail::unique_tuple<T, Ts...> {};
template<typename... Ts> requires(sizeof...(Ts) > 0)
struct unique_tuple<type_list<Ts...>> : detail::unique_tuple<Ts...> {};
template<>
struct unique_tuple<type_list<>> : std::type_identity<type_list<>> {};
template<typename A, typename... Ts>
using unique_tuple_t = typename unique_tuple<A, Ts...>::type;

template<typename A, typename... Ts>
struct merged_tuple : detail::merged_tuple<A, Ts...> {};
template<typename A, typename... Ts>
using merged_tuple_t = typename merged_tuple<A, Ts...>::type;


#ifndef DOXYGEN
namespace detail {

    template<template<typename> typename filter, typename...>
    struct filtered_tuple_impl;
    template<template<typename> typename filter, typename T, typename... rest, typename... current>
    struct filtered_tuple_impl<filter, type_list<T, rest...>, type_list<current...>> {
        using type = std::conditional_t<
            filter<T>::value,
            typename filtered_tuple_impl<filter, type_list<rest...>, merged_tuple_t<type_list<T>, type_list<current...>>>::type,
            typename filtered_tuple_impl<filter, type_list<rest...>, type_list<current...>>::type
        >;
    };
    template<template<typename> typename filter, typename... current>
    struct filtered_tuple_impl<filter, type_list<>, type_list<current...>> {
        using type = type_list<current...>;
    };

}  // namespace detail
#endif  // DOXYGEN

template<template<typename> typename filter, typename... Ts>
struct filtered_tuple : detail::filtered_tuple_impl<filter, type_list<Ts...>, type_list<>> {};
template<template<typename> typename filter, typename... Ts>
struct filtered_tuple<filter, type_list<Ts...>> : detail::filtered_tuple_impl<filter, type_list<Ts...>, type_list<>> {};
template<template<typename> typename filter, typename... Ts>
using filtered_tuple_t = typename filtered_tuple<filter, Ts...>::type;

}  // namespace adpp
