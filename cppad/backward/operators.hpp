#pragma once

#include <cmath>
#include <utility>
#include <functional>
#include <ostream>

#include <cppad/concepts.hpp>
#include <cppad/type_traits.hpp>
#include <cppad/backward/concepts.hpp>
#include <cppad/backward/bindings.hpp>

namespace cppad::backward {

namespace operators {

struct exp {
    template<concepts::arithmetic T>
    auto operator()(T value) const {
        using std::exp;
        return exp(value);
    }
};

}  // namespace operators

namespace traits {

template<>
struct differentiator<std::plus<void>> {
    static constexpr auto differentiate(
        const auto& a,
        const auto& b,
        const auto& var
    ) {
        return a.differentiate_wrt(var) + b.differentiate_wrt(var);
    }

    template<typename... V>
    static constexpr auto back_propagate(
        const backward::bindings<V...>& bindings,
        const auto& a,
        const auto& b,
        const auto&... vars
    ) {
        auto [value_a, derivs_a] = a.back_propagate(bindings, vars...);
        auto [value_b, derivs_b] = b.back_propagate(bindings, vars...);
        auto result = value_a + value_b;
        return std::make_pair(result, std::move(derivs_a) + std::move(derivs_b));
    }
};

template<>
struct formatter<std::plus<void>> {
    static constexpr void format(
        std::ostream& out,
        const auto& a,
        const auto& b,
        const auto& name_map
    ) {
        a.stream(out, name_map);
        out << " + ";
        b.stream(out, name_map);
    }
};

template<>
struct differentiator<std::minus<void>> {
    static constexpr auto differentiate(
        const auto& a,
        const auto& b,
        const auto& var
    ) {
        return a.differentiate_wrt(var) - b.differentiate_wrt(var);
    }

    template<typename... V>
    static constexpr auto back_propagate(
        const backward::bindings<V...>& bindings,
        const auto& a,
        const auto& b,
        const auto&... vars
    ) {
        auto [value_a, derivs_a] = a.back_propagate(bindings, vars...);
        auto [value_b, derivs_b] = b.back_propagate(bindings, vars...);
        auto result = value_a - value_b;
        return std::make_pair(result, std::move(derivs_a) + std::move(derivs_b).scaled_with(-1));
    }
};

template<>
struct formatter<std::minus<void>> {
    static constexpr void format(
        std::ostream& out,
        const auto& a,
        const auto& b,
        const auto& name_map
    ) {
        a.stream(out, name_map);
        out << " - ";
        b.stream(out, name_map);
    }
};

template<>
struct differentiator<std::multiplies<void>> {
    static constexpr auto differentiate(
        const auto& a,
        const auto& b,
        const auto& var
    ) {
        return a.differentiate_wrt(var)*b + a*b.differentiate_wrt(var);
    }

    template<typename... V>
    static constexpr auto back_propagate(
        const backward::bindings<V...>& bindings,
        const auto& a,
        const auto& b,
        const auto&... vars
    ) {
        auto [value_a, derivs_a] = a.back_propagate(bindings, vars...);
        auto [value_b, derivs_b] = b.back_propagate(bindings, vars...);
        auto result = value_a*value_b;
        return std::make_pair(
            result,
            std::move(derivs_a).scaled_with(value_b) + std::move(derivs_b).scaled_with(value_a)
        );
    }
};

template<>
struct formatter<std::multiplies<void>> {
    template<typename A, typename B>
    static constexpr void format(std::ostream& out, const A& a, const B& b, const auto& name_map) {
        constexpr bool use_braces_around_a = backward::sub_expressions_size_v<A> > 1;
        if constexpr (use_braces_around_a) out << "(";
        a.stream(out, name_map);
        if constexpr (use_braces_around_a) out << ")";

        out << "*";

        constexpr bool use_braces_around_b = backward::sub_expressions_size_v<B> > 1;
        if constexpr (use_braces_around_b) out << "(";
        b.stream(out, name_map);
        if constexpr (use_braces_around_b) out << ")";
    }
};

template<>
struct differentiator<cppad::backward::operators::exp> {
    static constexpr auto exp_op = cppad::backward::operators::exp{};

    static constexpr auto differentiate(
        const auto& e,
        const auto& var
    ) {
        return e.exp()*e.differentiate_wrt(var);
    }

    template<typename... V>
    static constexpr auto back_propagate(
        const backward::bindings<V...>& bindings,
        const auto& e,
        const auto&... vars
    ) {
        auto [value, derivs] = e.back_propagate(bindings, vars...);
        auto result = exp_op(value);
        return std::make_pair(result, std::move(derivs).scaled_with(result));
    }
};

template<>
struct formatter<cppad::backward::operators::exp> {
    static constexpr void format(std::ostream& out, const auto& a, const auto& name_map) {
        out << "exp(";
        a.stream(out, name_map);
        out << ")";
    }
};

}  // namespace traits
}  // namespace cppad::backward
