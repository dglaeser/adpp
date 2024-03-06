#pragma once

#include <tuple>
#include <type_traits>

#include <adpp/common.hpp>
#include <adpp/backward/bindings.hpp>
#include <adpp/backward/expression_tree.hpp>

namespace adpp::backward {

#ifndef DOXYGEN
namespace detail {

    template<typename T, typename B, typename... V>
    concept derivable_expression = requires(const T& t, const B& b, const type_list<V...>& vars) {
        { t.back_propagate(b, vars) };
    };

}  // namespace detail
#endif  // DOXYGEN

template<typename E, typename... V, typename... B>
    requires(detail::derivable_expression<E, bindings<B...>, V...>)
inline constexpr auto derivatives_of(const E& expression, const type_list<V...>& vars, const bindings<B...>& bindings) {
    return expression.back_propagate(bindings, vars).second;
}

template<typename E, typename V, typename... B>
    requires(detail::derivable_expression<E, bindings<B...>, V>)
inline constexpr auto derivative_of(const E& expression, const type_list<V>& vars, const bindings<B...>& bindings) {
    return derivatives_of(expression, vars, bindings).template get<V>();
}

#ifndef DOXYGEN
namespace detail {

template<int cur, int requested, typename E, typename V, typename... B>
inline constexpr auto derivative_of_impl(E&& expression, const type_list<V>& var, const bindings<B...>& bindings) {
    static_assert(cur <= requested);
    if constexpr (cur < requested) {
        return derivative_of_impl<cur + 1, requested>(expression.differentiate_wrt(var), var, bindings);
    } else {
        return expression.back_propagate(bindings, var).second.template get<V>();
    }
}

}  // namespace detail
#endif  // DOXYGEN

template<typename E, typename V, typename... B, unsigned int i>
inline constexpr auto derivative_of(const E& expression, const type_list<V>& vars, const bindings<B...>& bindings, const order<i>&) {
    return detail::derivative_of_impl<1, i>(expression, vars, bindings);
}

template<typename E, typename... B>
inline constexpr auto grad(const E& expression, const bindings<B...>& bindings) {
    return derivatives_of(expression, leaf_variables_of(expression), bindings);
}

template<typename... V>
inline constexpr auto wrt(V&&...) {
    return type_list<std::remove_cvref_t<V>...>{};
}

template<typename E, typename V>
inline constexpr auto differentiate(const E& expression, const type_list<V>& var) {
    return expression.differentiate_wrt(var);
}

}  // namespace adpp::backward
