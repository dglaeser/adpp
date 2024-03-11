#pragma once

#include <tuple>
#include <type_traits>

#include <adpp/common.hpp>
#include <adpp/backward/bindings.hpp>
#include <adpp/backward/expression.hpp>

namespace adpp::backward {

#ifndef DOXYGEN
namespace detail {

    template<typename T, typename B, typename... V>
    concept derivable_expression = requires(const T& t, const B& b, const type_list<V...>& vars) {
        { t.template back_propagate<double>(b, vars) };
    };

}  // namespace detail
#endif  // DOXYGEN

struct automatic {};

template<typename R = automatic, typename E, typename... B, typename... V>
    requires(detail::derivable_expression<E, bindings<B...>, V...>)
inline constexpr auto derivatives_of(E&& expression, const type_list<V...>& vars, const bindings<B...>& b) {
    using result_t = std::conditional_t<
        std::is_same_v<R, automatic>,
        typename bindings<B...>::common_value_type,
        R
    >;
    return expression.template back_propagate<result_t>(b, vars).second;
}

template<typename R = automatic, typename E, typename... B, typename V>
    requires(detail::derivable_expression<E, bindings<B...>, V>)
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
inline constexpr auto derivative_of_impl(E&& expression, const type_list<V>& var, const bindings<B...>& bindings) {
    static_assert(cur <= requested);
    if constexpr (cur < requested) {
        return derivative_of_impl<cur + 1, requested, R>(expression.differentiate_wrt(var), var, bindings);
    } else {
        return derivative_of<R>(expression, var, bindings);
    }
}

}  // namespace detail
#endif  // DOXYGEN

template<typename R = automatic, typename E, typename V, typename... B, unsigned int i>
inline constexpr auto derivative_of(const E& expression, const type_list<V>& vars, const bindings<B...>& bindings, const order<i>&) {
    return detail::derivative_of_impl<1, i, R>(expression, vars, bindings);
}

template<typename E, typename V>
inline constexpr auto differentiate(const E& expression, const type_list<V>& var) {
    return expression.differentiate_wrt(var);
}

}  // namespace adpp::backward
