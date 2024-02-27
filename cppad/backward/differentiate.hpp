#pragma once

#include <tuple>
#include <type_traits>

#include <cppad/backward/bindings.hpp>

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

template<typename... V>
    requires(std::conjunction_v<std::is_lvalue_reference<V>...>)
inline constexpr auto wrt(V&&... vars) {
    return std::forward_as_tuple(vars...);
}

}  // namespace cppad
