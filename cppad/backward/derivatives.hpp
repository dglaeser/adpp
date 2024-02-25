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
constexpr auto derivatives_of(E&& expression, const std::tuple<V...>& vars) {
    using R = expression_value_t<E>;
    return std::apply([&] <typename... Vs> (Vs&&... vs) {
        return expression.back_propagate(vs...).second;
    }, vars);
}

// TODO: gradient

template<concepts::expression E, typename... V> requires(sizeof...(V) == 1)
constexpr auto derivative_of(E&& expression, const std::tuple<V...>& vars) {
    return derivatives_of(std::forward<E>(expression), vars)[std::get<0>(vars)];
}

#ifndef DOXYGEN
namespace detail {

template<int cur, int requested, concepts::expression E, typename V>
constexpr auto derivative_of_impl(E&& expression, const V& var) {
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
constexpr auto derivative_of(E&& expression, const std::tuple<V...>& vars, const order::order<i>& order) {
    return detail::derivative_of_impl<1, i>(std::forward<E>(expression), std::get<0>(vars));
}

template<typename... V>
    requires(std::conjunction_v<std::is_lvalue_reference<V>...>)
constexpr auto wrt(V&&... vars) {
    return std::forward_as_tuple(vars...);
}

}  // namespace cppad::backward
