#pragma once

#include <type_traits>

#include <adpp/common.hpp>
#include <adpp/type_traits.hpp>
#include <adpp/backward/bindings.hpp>

namespace adpp::backward {

template<typename... V>
inline constexpr auto wrt(V&&...) {
    return type_list<std::remove_cvref_t<V>...>{};
}

template<typename R = automatic, typename E, typename... B, typename... V>
    requires(expression_for<E, bindings<B...>>)
inline constexpr auto derivatives_of(E&& expression, const type_list<V...>& vars, const bindings<B...>& b) {
    using result_t = std::conditional_t<
        std::is_same_v<R, automatic>, typename bindings<B...>::common_value_type, R
    >;
    return expression.template back_propagate<result_t>(b, vars).second;
}

template<typename R = automatic, typename E, typename... B, typename V>
inline constexpr auto derivative_of(E&& expression, const type_list<V>& var, const bindings<B...>& bindings) {
    return derivatives_of<R>(std::forward<E>(expression), var, bindings).template get<V>();
}

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

template<typename R = automatic, typename E, typename V, typename... B, unsigned int i>
inline constexpr auto derivative_of(const E& expression, const type_list<V>& vars, const bindings<B...>& bindings, const order<i>&) {
    return detail::higher_order_derivative_of_impl<1, i, R>(expression, vars, bindings);
}

template<typename E, typename V>
inline constexpr auto differentiate(const E& expression, const type_list<V>& var) {
    return expression.differentiate(var);
}

}  // namespace adpp::backward
