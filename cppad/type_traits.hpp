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


template<typename T>
struct is_constant : public std::false_type {};
template<typename T>
inline constexpr bool is_constant_v = is_constant<std::remove_cvref_t<T>>::value;


template<typename T>
struct is_named_expression : public std::false_type {};
template<typename T>
inline constexpr bool is_named_expression_v = is_named_expression<std::remove_cvref_t<T>>::value;


template<typename T>
struct is_variable : public std::false_type {};
template<typename T>
inline constexpr bool is_variable_v = is_variable<std::remove_cvref_t<T>>::value;

template<typename T>
struct expression_value;
template<typename T>
using expression_value_t = expression_value<std::remove_cvref_t<T>>::type;


template<typename T>
struct as_expression;
template<typename T>
inline constexpr decltype(auto) to_expression(T&& t) noexcept {
    return as_expression<std::remove_cvref_t<T>>::get(std::forward<T>(t));
}


template<typename T>
struct undefined_value;
template<typename T>
inline constexpr auto undefined_value_v = undefined_value<T>::value;


template<typename T>
struct formatter;
template<typename T>
inline constexpr decltype(auto) format(T&& t) noexcept {
    return formatter<std::remove_cvref_t<T>>(std::forward<T>(t));
}


template<typename T>
struct differentiator;


template<typename T>
struct sub_expressions;


template<typename T>
struct is_pair : public std::false_type {};
template<typename A, typename B>
struct is_pair<std::pair<A, B>> : public std::true_type {};
template<typename T>
inline constexpr bool is_pair_v = is_pair<T>::value;

}  // namespace cppad
