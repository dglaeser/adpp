#pragma once

#include <tuple>
#include <type_traits>

#include <cppad/common.hpp>
#include <cppad/backward/bindings.hpp>
#include <cppad/backward/expression_tree.hpp>

namespace cppad::backward {

#ifndef DOXYGEN
namespace detail {

    template<typename T, typename B, typename... V>
    concept derivable_expression = requires(const T& t, const B& b, const V&... vars) {
        { t.back_propagate(b, vars...) };
    };

}  // namespace detail
#endif  // DOXYGEN

template<typename E, typename... V, typename... B>
    requires(detail::derivable_expression<E, bindings<B...>, V...>)
inline constexpr auto derivatives_of(const E& expression, const std::tuple<V...>& vars, const bindings<B...>& bindings) {
    using R = std::remove_cvref_t<decltype(expression.evaluate_at(bindings))>;
    return std::apply([&] <typename... Vs> (Vs&&... vs) {
        return expression.back_propagate(bindings, vs...).second;
    }, vars);
}

template<typename E, typename... V, typename... B>
    requires(sizeof...(V) == 1 && detail::derivable_expression<E, bindings<B...>, V...>)
inline constexpr auto derivative_of(const E& expression, const std::tuple<V...>& vars, const bindings<B...>& bindings) {
    return derivatives_of(expression, vars, bindings)[std::get<0>(vars)];
}

#ifndef DOXYGEN
namespace detail {

template<int cur, int requested, typename E, typename V, typename... B>
inline constexpr auto derivative_of_impl(E&& expression, const V& var, const bindings<B...>& bindings) {
    static_assert(cur <= requested);
    if constexpr (cur < requested) {
        return derivative_of_impl<cur + 1, requested>(expression.differentiate_wrt(var), var, bindings);
    } else {
        return expression.back_propagate(bindings, var).second[var];
    }
}

}  // namespace detail
#endif  // DOXYGEN

template<typename E, typename V, typename... B, unsigned int i>
inline constexpr auto derivative_of(const E& expression, const std::tuple<V>& vars, const bindings<B...>& bindings, const order<i>& order) {
    return detail::derivative_of_impl<1, i>(expression, std::get<0>(vars), bindings);
}

template<typename E, typename... B>
inline constexpr auto grad(const E& expression, const bindings<B...>& bindings) {
    return derivatives_of(expression, leaf_variables_of(expression), bindings);
}

template<typename... V>
    requires(std::conjunction_v<std::is_lvalue_reference<V>...>)
inline constexpr auto wrt(V&&... vars) {
    return std::forward_as_tuple(vars...);
}

template<typename E, typename V>
inline constexpr auto differentiate(const E& expression, const std::tuple<V>& var) {
    return expression.differentiate_wrt(std::get<0>(var));
}

}  // namespace cppad::backward
