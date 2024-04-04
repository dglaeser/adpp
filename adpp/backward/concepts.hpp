// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Backward
 * \brief Concepts common to the backward subpackage.
 */
#pragma once

#include <type_traits>

#include <adpp/concepts.hpp>

namespace adpp::backward {

//! \addtogroup Backward
//! \{

//! Type trait to define is a type is a symbol.
template<typename T>
struct is_symbol : std::false_type {};
template<typename T>
inline constexpr bool is_symbol_v = is_symbol<T>::value;
template<typename T>
concept symbolic = is_symbol_v<T>;

//! Type trait to define is a type is an unbound symbol.
template<typename T>
struct is_unbound_symbol : std::false_type {};
template<typename T>
inline constexpr bool is_unbound_symbol_v = is_unbound_symbol<T>::value;
template<typename T>
concept unbound_symbol = is_unbound_symbol_v<T>;

//! Type trait to define is a type is an expression.
template<typename T>
struct is_expression : std::false_type {};
template<typename T>
inline constexpr bool is_expression_v = is_expression<T>::value;

//! Type trait to define is a type is an expression that yields a scalar value.
template<typename T>
struct is_scalar_expression : std::true_type {};
template<typename T>
inline constexpr bool is_scalar_expression_v = is_scalar_expression<T>::value;

//! Type trait to extract the operands of an expression
template<typename T>
struct operands;
template<typename T>
using operands_t = typename operands<T>::type;

//! Concept for terms (symbols or expressions)
template<typename T>
concept term = symbolic<std::remove_cvref_t<T>> or is_expression_v<std::remove_cvref_t<T>>;

//! Concept for types that are convertible into terms
template<typename T>
concept into_term = term<std::remove_cvref_t<T>> or scalar<std::remove_cvref_t<T>>;

//! Concept that defines if an expression of type `T` is evaluatable with an argument of type `Arg`
template<typename T, typename Arg>
concept evaluatable_with = term<T> and requires(const T& t, const Arg& b) { { t.evaluate(b) }; };

//! Concept for types that bind values to symbols
template<typename T>
concept binder = requires(const T& t) {
    typename T::symbol_type;
    typename T::value_type;
    { t.unwrap() } -> same_remove_cvref_t_as<typename T::value_type>;
};

//! \} group Backward

}  // namespace adpp::backward
