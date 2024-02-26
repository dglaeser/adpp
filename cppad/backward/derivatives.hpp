#pragma once

#include <tuple>
#include <array>
#include <algorithm>
#include <type_traits>

#include <cppad/common.hpp>
#include <cppad/variadic_accessor.hpp>

namespace cppad::backward {

template<concepts::arithmetic R, typename... Ts>
    requires(are_unique_v<Ts...>)
struct derivatives : variadic_accessor<const Ts&...> {
 private:
    using base = variadic_accessor<const Ts&...>;

 public:
    constexpr derivatives(R, const Ts&... ts) noexcept
    : base(ts...) {
        std::ranges::fill(_values, R{0});
    }

    template<typename Self, typename T> requires(contains_decay_v<T, Ts...>)
    constexpr decltype(auto) operator[](this Self&& self, const T& t) noexcept {
        return self._values[self.index_of(t)];
    }

    template<typename Self, concepts::arithmetic T>
    constexpr decltype(auto) scaled_with(this Self&& self, T factor) noexcept {
        std::ranges::for_each(self._values, [factor] (auto& v) { v *= factor; });
        return std::forward<Self>(self);
    }

    template<typename Self>
    constexpr decltype(auto) operator+(this Self&& self, const derivatives& other) noexcept {
        std::transform(
            other._values.begin(), other._values.end(),
            self._values.begin(), self._values.begin(),
            std::plus{}
        );
        return std::forward<Self>(self);
    }

 private:
    std::array<R, sizeof...(Ts)> _values;
};

template<typename R, typename... Ts>
    requires(std::conjunction_v<std::is_lvalue_reference<Ts>...>)
derivatives(R&&, Ts&&...) -> derivatives<R, std::remove_cvref_t<Ts>...>;

template<concepts::expression E, typename... V>
inline constexpr auto derivatives_of(E&& expression, const std::tuple<V...>& vars) {
    using R = expression_value_t<E>;
    return std::apply([&] <typename... Vs> (Vs&&... vs) {
        return expression.back_propagate(vs...).second;
    }, vars);
}

// TODO: gradient

template<concepts::expression E, typename... V> requires(sizeof...(V) == 1)
inline constexpr auto derivative_of(E&& expression, const std::tuple<V...>& vars) {
    return derivatives_of(std::forward<E>(expression), vars)[std::get<0>(vars)];
}

#ifndef DOXYGEN
namespace detail {

template<int cur, int requested, concepts::expression E, typename V>
inline constexpr auto derivative_of_impl(E&& expression, const V& var) {
    static_assert(cur <= requested);
    if constexpr (cur < requested) {
        return derivative_of_impl<cur + 1, requested>(expression.differentiate_wrt(var), var);
    } else {
        return expression.back_propagate(var).second[var];
    }
}

}  // namespace detail
#endif  // DOXYGEN

template<concepts::expression E, typename... V, unsigned int i> requires(sizeof...(V) == 1)
inline constexpr auto derivative_of(E&& expression, const std::tuple<V...>& vars, const order<i>& order) {
    return detail::derivative_of_impl<1, i>(std::forward<E>(expression), std::get<0>(vars));
}


#ifndef DOXYGEN
namespace detail {

    template<typename T>
    concept leaf_expression = is_variable_v<T> or is_constant_v<T>;

    template<typename F, concepts::expression E, typename... V>
        requires(std::conjunction_v<is_variable<V>...>)
    inline constexpr auto visit_expression(const F& sub_expr_callback, std::tuple<const V&...>&& vars, const E& e) {
        if constexpr (leaf_expression<E> && is_variable_v<E>) {
            if constexpr (contains_decay_v<E, V...>)
                return std::move(vars);
            else
                return std::tuple_cat(std::tuple<const E&>{e}, std::move(vars));
        } else if constexpr (leaf_expression<E>) {
            return std::move(vars);
        } else {
            return std::apply([&] <typename... SE> (SE&&... sub_expr) {
                return sub_expr_callback(std::move(vars), std::forward<SE>(sub_expr)...);
            }, sub_expressions<E>::get(e));
        }
    }

    template<typename F, concepts::expression E0, concepts::expression... E, typename... V>
        requires(std::conjunction_v<is_variable<V>...>)
    inline constexpr auto concatenate_variables_impl(const F& sub_expr_callback, std::tuple<const V&...>&& vars, const E0& e0, const E&... e) {
        auto first_processed = visit_expression(sub_expr_callback, std::move(vars), e0);
        if constexpr (sizeof...(E) == 0)
            return std::move(first_processed);
        else
            return concatenate_variables_impl(sub_expr_callback, std::move(first_processed), e...);
    }

    template<typename... V, concepts::expression... E>
        requires(std::conjunction_v<is_variable<V>...>)
    inline constexpr auto concatenate_variables(std::tuple<const V&...>&& vars, const E&... e) {
        if constexpr (sizeof...(E) == 0) {
            return std::move(vars);
        } else if constexpr (sizeof...(E) == 1) {
            return visit_expression([] <typename... SE> (auto&& result, SE&&... sub_expr) {
                return concatenate_variables(std::move(result), std::forward<SE>(sub_expr)...);
            }, std::move(vars), e...);
        } else {
            return concatenate_variables_impl([] <typename... SE> (auto&& result, SE&&... sub_expr) {
                return concatenate_variables(std::move(result), std::forward<SE>(sub_expr)...);
            }, std::move(vars), e...);
        }
    }

}  // namespace detail
#endif  // DOXYGEN

template<concepts::expression E>
inline constexpr auto variables_of(const E& e) {
    return detail::concatenate_variables(std::tuple{}, e);
}

template<typename... V>
    requires(std::conjunction_v<std::is_lvalue_reference<V>...>)
inline constexpr auto wrt(V&&... vars) {
    return std::forward_as_tuple(vars...);
}

}  // namespace cppad::backward
