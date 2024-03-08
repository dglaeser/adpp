#pragma once

#include <cmath>
#include <type_traits>

#include <adpp/type_traits.hpp>
#include <adpp/backward/concepts.hpp>
#include <adpp/backward/bindings.hpp>
#include <adpp/backward/derivatives.hpp>
#include <adpp/backward/differentiate.hpp>

namespace adpp::backward {

namespace op {

struct exp {
    template<typename T>
    constexpr auto operator()(const T& t) const {
        using std::exp;
        return exp(t);
    }
};

}  // namespace op


template<typename op, typename... T>
struct back_propagation;

template<typename op, term... Ts>
struct expression {
    template<typename... B>
    constexpr decltype(auto) operator()(const bindings<B...>& operands) const {
        return op{}(Ts{}(operands)...);
    }

    template<typename Self, typename... B, typename... V>
    constexpr auto back_propagate(this Self&& self, const bindings<B...>& bindings, const type_list<V...>& vars) {
        auto [value, derivs] = back_propagation<op, Ts...>{}(bindings, vars);
        if constexpr (contains_decay_v<Self, V...>)
            derivs[self] = 1.0;
        return std::make_pair(std::move(value), std::move(derivs));
    }
};

template<typename op, term... Ts>
struct is_expression<expression<op, Ts...>> : std::true_type {};


template<auto value>
struct _val {
    template<typename... B>
    constexpr auto operator()(const bindings<B...>&) const noexcept {
        return value;
    }

    template<typename Self, typename... B, typename... V>
    constexpr auto back_propagate(this Self&& self, const bindings<B...>& bindings, const type_list<V...>&) {
        using T = std::remove_cvref_t<decltype(value)>;
        return std::make_pair(value, derivatives<T, V...>{});
    }
};

template<auto v>
struct is_symbol<_val<v>> : std::true_type {};

template<auto value>
inline constexpr _val<value> aval;

template<typename op, term... Ts>
using op_result_t = expression<op, std::remove_cvref_t<Ts>...>;

template<term A, term B>
inline constexpr op_result_t<std::plus<void>, A, B> operator+(A&&, B&&) { return {}; }
template<term A, term B>
inline constexpr op_result_t<std::minus<void>, A, B> operator-(A&&, B&&) { return {}; }
template<term A, term B>
inline constexpr op_result_t<std::multiplies<void>, A, B> operator*(A&&, B&&) { return {}; }
template<term A, term B>
inline constexpr op_result_t<std::divides<void>, A, B> operator/(A&&, B&&) { return {}; }
template<term A>
inline constexpr op_result_t<op::exp, A> exp(A&&) { return {}; }


template<typename A, typename B>
struct back_propagation<std::plus<void>, A, B> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_a, derivs_a] = A{}.back_propagate(b, vars);
        auto [value_b, derivs_b] = B{}.back_propagate(b, vars);
        return std::make_pair(value_a + value_b, std::move(derivs_a) + std::move(derivs_b));
    }
};

template<typename A, typename B>
struct back_propagation<std::minus<void>, A, B> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_a, derivs_a] = A{}.back_propagate(b, vars);
        auto [value_b, derivs_b] = B{}.back_propagate(b, vars);
        return std::make_pair(value_a - value_b, std::move(derivs_a) + std::move(derivs_b).scaled_with(-1));
    }
};

template<typename A, typename B>
struct back_propagation<std::multiplies<void>, A, B> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_a, derivs_a] = A{}.back_propagate(b, vars);
        auto [value_b, derivs_b] = B{}.back_propagate(b, vars);
        auto derivs = std::move(derivs_a).scaled_with(value_b) + std::move(derivs_b).scaled_with(value_a);
        return std::make_pair(value_a*value_b, std::move(derivs));
    }
};

template<typename A, typename B>
struct back_propagation<std::divides<void>, A, B> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_a, derivs_a] = A{}.back_propagate(b, vars);
        auto [value_b, derivs_b] = B{}.back_propagate(b, vars);
        // TODO: correct
        auto derivs = std::move(derivs_a).scaled_with(value_b) + std::move(derivs_b).scaled_with(value_a);
        return std::make_pair(value_a/value_b, std::move(derivs));
    }
};

template<typename A>
struct back_propagation<op::exp, A> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_inner, derivs_inner] = A{}.back_propagate(b, vars);
        auto result = op::exp{}(value_inner);
        return std::make_pair(std::move(result), std::move(derivs_inner).scaled_with(result));
    }
};

}  // namespace adpp::backward
