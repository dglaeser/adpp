#pragma once

#include <algorithm>
#include <type_traits>

#include <cppad/concepts.hpp>
#include <cppad/variadic_accessor.hpp>

namespace cppad::backward {

template<concepts::arithmetic R, typename... Ts>
    requires(are_unique_v<Ts...>)
struct derivatives : variadic_accessor<const Ts&...> {
 private:
    using base = variadic_accessor<const Ts&...>;

 public:
    using value_type = R;

    constexpr derivatives(value_type, const Ts&... ts) noexcept
    : base(ts...) {
        std::ranges::fill(_values, R{0});
    }

    template<typename Self, typename T> requires(contains_decay_v<T, Ts...>)
    constexpr std::convertible_to<value_type> decltype(auto) operator[](this Self&& self, const T& t) noexcept {
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
    std::array<value_type, sizeof...(Ts)> _values;
};

template<typename R, typename... Ts>
    requires(std::conjunction_v<std::is_lvalue_reference<Ts>...>)
derivatives(R&&, Ts&&...) -> derivatives<std::remove_cvref_t<R>, std::remove_cvref_t<Ts>...>;

}  // namespace cppad::backward
