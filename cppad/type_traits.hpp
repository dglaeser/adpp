#pragma once

#include <type_traits>
#include <utility>


namespace cppad {

template<std::size_t i>
using index_constant = std::integral_constant<std::size_t, i>;


template<typename T>
struct is_ownable : public std::bool_constant<!std::is_lvalue_reference_v<T>> {};
template<typename T>
inline constexpr bool is_ownable_v = is_ownable<T>::value;


template<typename T, typename... Ts>
struct contains_decay : public std::bool_constant<std::disjunction_v<std::is_same<std::decay_t<T>, Ts>...>> {};
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
template<typename T1, typename T2> struct are_unique<T1, T2> : public std::bool_constant<!std::is_same_v<T1, T2>> {};
template<typename T> struct are_unique<T> : public std::true_type {};
template<> struct are_unique<> : public std::true_type {};
template<typename... Ts>
inline constexpr bool are_unique_v = are_unique<Ts...>::value;


#ifndef DOXYGEN
namespace detail {

    template<typename T, std::size_t s = sizeof(T)>
    std::false_type is_incomplete(T*);
    std::true_type is_incomplete(...);

}  // end namespace detail
#endif  // DOXYGEN

template<typename T>
struct is_complete : public std::bool_constant<!decltype(detail::is_incomplete(std::declval<T*>()))::value> {};
template<typename T>
inline constexpr bool is_complete_v = is_complete<T>::value;

}  // namespace cppad
