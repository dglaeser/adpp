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
struct derivatives : variadic_accessor<Ts...> {
 private:
    using base = variadic_accessor<Ts...>;

 public:
    constexpr derivatives(R, const Ts&... ts) noexcept
    : base(ts...) {
        std::ranges::fill(_values, R{0});
    }

    template<typename T> requires(contains_decay_v<T, Ts...>)
    constexpr R operator[](const T& t) const noexcept {
        return _values[this->index_of(t)];
    }

    template<typename T> requires(contains_decay_v<T, Ts...>)
    constexpr void add_to_derivative_wrt(const T& t, concepts::arithmetic auto value) noexcept {
        if constexpr (contains_decay_v<T, Ts...>)
            _values[this->index_of(t)] += value;
    }

 private:
    std::array<R, sizeof...(Ts)> _values;
};

template<typename R, typename... Ts>
    requires(std::conjunction_v<std::is_lvalue_reference<Ts>...>)
derivatives(R&&, Ts&&...) -> derivatives<R, std::remove_cvref_t<Ts>...>;

template<concepts::expression E, typename... V>
constexpr auto derivatives_of(E&& expression, const std::tuple<V...>& vars) {
    using R = expression_value_t<E>;
    return std::apply([&] <typename... Vs> (Vs&&... vs) {
        derivatives derivs{R{}, std::forward<Vs>(vs)...};
        expression.accumulate_derivatives(R{1}, derivs);
        return derivs;
    }, vars);
}

template<concepts::expression E, typename... V> requires(sizeof...(V) == 1)
constexpr auto derivative_of(E&& expression, const std::tuple<V...>& vars) {
    return derivatives_of(std::forward<E>(expression), vars)[std::get<0>(vars)];
}

template<typename... V>
    requires(std::conjunction_v<std::is_lvalue_reference<V>...>)
constexpr auto wrt(V&&... vars) {
    return std::forward_as_tuple(vars...);
}

}  // namespace cppad::backward
