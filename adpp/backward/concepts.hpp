#pragma once

#include <type_traits>

#include <adpp/concepts.hpp>

namespace adpp::backward {

template<typename T>
struct is_symbol : std::false_type {};
template<typename T>
inline constexpr bool is_symbol_v = is_symbol<T>::value;
template<typename T>
concept symbolic = is_symbol_v<T>;


template<typename T>
struct is_unbound_symbol : std::false_type {};
template<typename T>
inline constexpr bool is_unbound_symbol_v = is_unbound_symbol<T>::value;
template<typename T>
concept unbound_symbol = is_unbound_symbol_v<T>;


template<typename T>
struct is_expression : std::false_type {};
template<typename T>
inline constexpr bool is_expression_v = is_expression<T>::value;
template<typename T>
concept term = symbolic<std::remove_cvref_t<T>> or is_expression_v<std::remove_cvref_t<T>>;


template<typename T>
concept binder = requires(const T& t) {
    typename T::symbol_type;
    typename T::value_type;
    { t.unwrap() };
};

}  // namespace adpp::backward
