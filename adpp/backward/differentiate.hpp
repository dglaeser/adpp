// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Backward
 * \brief Classes and functions for differentiating expressions.
 */
#pragma once

#include <type_traits>

#include <adpp/utils.hpp>
#include <adpp/backward/bindings.hpp>

namespace adpp::backward {

//! \addtogroup Backward
//! \{

//! Create a type_list containing the given terms
template<typename... V>
inline constexpr auto wrt(V&&...) {
    return type_list<std::remove_cvref_t<V>...>{};
}

//! Evaluate the derivatives of the given expression w.r.t. to the given variables at the given values
template<typename R = automatic, typename E, typename... B, typename... V>
    requires(evaluatable_with<E, bindings<B...>>)
inline constexpr auto derivatives_of(E&& expression, const type_list<V...>& vars, const bindings<B...>& b) {
    using result_t = std::conditional_t<
        std::is_same_v<R, automatic>, typename bindings<B...>::common_value_type, R
    >;
    return expression.template back_propagate<result_t>(b, vars).second;
}

//! Evaluate the derivative of the given expression w.r.t. to the given variable at the given values
template<typename R = automatic, typename E, typename... B, typename V>
inline constexpr auto derivative_of(E&& expression, const type_list<V>& var, const bindings<B...>& bindings) {
    return derivatives_of<R>(std::forward<E>(expression), var, bindings).template get<V>();
}

//! Evaluate the gradient of the given expression at the given values
template<typename R = automatic, typename E, typename... B>
inline constexpr auto grad(E&& expression, const bindings<B...>& bindings) {
    return derivatives_of<R>(std::forward<E>(expression), variables_of(expression), bindings);
}


#ifndef DOXYGEN
namespace detail {

template<int cur, int requested, typename R, typename E, typename V, typename... B>
inline constexpr auto higher_order_derivative_of_impl(E&& expression, const type_list<V>& var, const bindings<B...>& bindings) {
    static_assert(cur <= requested);
    if constexpr (cur < requested) {
        return higher_order_derivative_of_impl<cur + 1, requested, R>(expression.differentiate(var), var, bindings);
    } else {
        return derivative_of<R>(expression, var, bindings);
    }
}

}  // namespace detail
#endif  // DOXYGEN

//! Evaluate the derivative of given order of the given expression w.r.t. to the given variable at the given values
template<typename R = automatic, typename E, typename V, typename... B, unsigned int i>
inline constexpr auto derivative_of(const E& expression, const type_list<V>& vars, const bindings<B...>& bindings, const order<i>&) {
    return detail::higher_order_derivative_of_impl<1, i, R>(expression, vars, bindings);
}

//! Differentitate the given expression w.r.t. the given variable and return the resulting expression.
template<typename E, typename V>
inline constexpr auto differentiate(const E& expression, const type_list<V>& var) {
    return expression.differentiate(var);
}

//! \} group Backward

}  // namespace adpp::backward
