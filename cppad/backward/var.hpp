#pragma once

#include <limits>

#include <cppad/common.hpp>

#include <cppad/backward/val.hpp>
#include <cppad/backward/let.hpp>
#include <cppad/backward/expression.hpp>
#include <cppad/backward/derivative.hpp>


namespace cppad::backward {

template<concepts::arithmetic T, auto = [] () {}>
class var : public val<T> {
    using base = val<T>;

 public:
    using base::base;
    constexpr var(var&&) = default;
    constexpr var(const var&) = delete;
    constexpr var() : base(undefined_value<T>) {}

    // for better error message when attempting to copy
    template<concepts::arithmetic V, auto _>
    constexpr var& operator=(const var<V, _>&) = delete;

    constexpr var& operator=(T value) {
        static_cast<base&>(*this) = value;
        return *this;
    }

    template<typename Self, concepts::expression... E>
    constexpr auto backpropagate(this Self&& self, const E&... e) {
        derivatives derivs{double{}, e...};
        if constexpr (contains_decay_v<Self, E...>)
            derivs[self] = 1.0;
        return std::make_pair(self.value(), std::move(derivs));
    }

    template<typename Self, concepts::expression E>
    constexpr auto partial_expression(this Self&&, E&& e) {
        if constexpr (concepts::same_decay_t_as<Self, E>)
            return let<T>{1};
        return let<T>{0};
    }

    template<typename Self>
    constexpr auto operator|=(this Self&& self, const char* name) noexcept {
        return named_expression<Self>{std::forward<Self>(self), name};
    }
};

template<concepts::arithmetic T, auto _ = [] () {}>
var(T&&) -> var<std::remove_cvref_t<T>, _>;

}  // namespace cppad::backward

namespace cppad::traits {

template<concepts::arithmetic T, auto _>
struct is_variable<cppad::backward::var<T, _>> : public std::true_type {};

}  // namespace cppad::traits
