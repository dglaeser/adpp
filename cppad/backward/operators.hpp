#pragma once

#include <cmath>
#include <utility>
#include <functional>

#include <cppad/type_traits.hpp>
#include <cppad/concepts.hpp>
#include <cppad/backward/derivatives.hpp>

namespace cppad {

namespace backward::operators {

struct exp {
    template<concepts::arithmetic T>
    auto operator()(T value) const {
        using std::exp;
        return exp(value);
    }
};

}  // namespace operators


template<>
struct differentiator<std::plus<void>> {
    static constexpr auto differentiate(
        const concepts::expression auto& a,
        const concepts::expression auto& b,
        const concepts::expression auto& var) {
        return a.differentiate_wrt(var) + b.differentiate_wrt(var);
    }

    static constexpr auto back_propagate(
        const concepts::expression auto& a,
        const concepts::expression auto& b,
        const concepts::expression auto&... vars
    ) {
        auto [value_a, derivs_a] = a.back_propagate(vars...);
        auto [value_b, derivs_b] = b.back_propagate(vars...);
        auto result = value_a + value_b;
        return std::make_pair(result, std::move(derivs_a) + std::move(derivs_b));
    }
};

template<>
struct differentiator<std::minus<void>> {
    static constexpr auto differentiate(
        const concepts::expression auto& a,
        const concepts::expression auto& b,
        const concepts::expression auto& var
    ) {
        return a.differentiate_wrt(var) - b.differentiate_wrt(var);
    }

    static constexpr auto back_propagate(
        const concepts::expression auto& a,
        const concepts::expression auto& b,
        const concepts::expression auto&... vars
    ) {
        auto [value_a, derivs_a] = a.back_propagate(vars...);
        auto [value_b, derivs_b] = b.back_propagate(vars...);
        auto result = value_a - value_b;
        return std::make_pair(result, std::move(derivs_a) + std::move(derivs_b).scaled_with(-1));
    }
};
template<>
struct differentiator<std::multiplies<void>> {
    static constexpr auto differentiate(
        const concepts::expression auto& a,
        const concepts::expression auto& b,
        const concepts::expression auto& var
    ) {
        return a.differentiate_wrt(var)*b + a*b.differentiate_wrt(var);
    }

    static constexpr auto back_propagate(
        const concepts::expression auto& a,
        const concepts::expression auto& b,
        const concepts::expression auto&... vars
    ) {
        auto [value_a, derivs_a] = a.back_propagate(vars...);
        auto [value_b, derivs_b] = b.back_propagate(vars...);
        auto result = value_a*value_b;
        return std::make_pair(
            result,
            std::move(derivs_a).scaled_with(value_b) + std::move(derivs_b).scaled_with(value_a)
        );
    }
};

template<>
struct differentiator<cppad::backward::operators::exp> {
    static constexpr auto exp_op = cppad::backward::operators::exp{};

    static constexpr auto differentiate(
        const concepts::expression auto& e,
        const concepts::expression auto& var
    ) {
        return e.exp()*e.differentiate_wrt(var);
    }

    static constexpr auto back_propagate(
        const concepts::expression auto& e,
        const concepts::expression auto&... vars
    ) {
        auto [value, derivs] = e.back_propagate(vars...);
        auto result = exp_op(value);
        return std::make_pair(result, std::move(derivs).scaled_with(result));
    }
};

}  // namespace cppad
