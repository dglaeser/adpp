#pragma once

#include <cmath>
#include <utility>
#include <type_traits>
#include <functional>
#include <concepts>
#include <ostream>


#include <cppad/common.hpp>
#include <cppad/backward/derivative.hpp>

namespace cppad::backward {

template<concepts::unary_operator O, typename E> class unary_operator;
template<concepts::binary_operator O, typename A, typename B> class binary_operator;


namespace operators {

struct exp {
    template<concepts::arithmetic T>
    auto operator()(T value) const {
        using std::exp;
        return exp(value);
    }
};

}  // namespace operators


template<concepts::expression E>
class expression {
 public:
    constexpr expression(E&& e)
    : _e{std::move(e)}
    {}

    template<typename Self, concepts::arithmetic T, typename... V>
    constexpr void accumulate_derivatives(this Self&& self, T multiplier, derivatives<V...>& derivs) {
        if constexpr (contains_decay_v<Self, V...>)
            derivs.add_to_derivative_wrt(self, multiplier);
        self._e.accumulate_derivatives(multiplier, derivs);
    }

    template<concepts::expression... _E>
    constexpr auto backpropagate(const _E&... e) const {
        return _e.backpropagate(e...);
    }

    constexpr decltype(auto) value() const {
        return _e.value();
    }

    template<concepts::expression _E>
    constexpr decltype(auto) partial_expression(_E&& e) const {
        return _e.partial_expression(std::forward<_E>(e));
    }

 private:
    E _e;
};

template<concepts::expression E>
expression(E&&) -> expression<std::remove_cvref_t<E>>;


template<concepts::expression E>
    requires(std::is_lvalue_reference_v<E>)
class named_expression {
 public:
    constexpr named_expression(E e, const char* name) noexcept
    : _storage{std::forward<E>(e)}
    , _name{name}
    {}

    constexpr const char* name() const {
        return _name;
    }

    template<concepts::expression _E>
    constexpr bool operator==(_E&& e) const {
        return is_same_object(e, _storage.get());
    }

 private:
    storage<E> _storage;
    const char* _name;
};


struct expression_base {
    template<typename Self, concepts::into_expression Other>
    constexpr auto operator+(this Self&& self, Other&& other) noexcept {
        return binary_operator{
            std::plus{},
            std::forward<Self>(self),
            as_expression(std::forward<Other>(other))
        };
    }

    template<typename Self, concepts::into_expression Other>
    constexpr auto operator-(this Self&& self, Other&& other) noexcept {
        return binary_operator{
            std::minus{},
            std::forward<Self>(self),
            as_expression(std::forward<Other>(other))
        };
    }

    template<typename Self, concepts::into_expression Other>
    constexpr auto operator*(this Self&& self, Other&& other) noexcept {
        return binary_operator{
            std::multiplies{},
            std::forward<Self>(self),
            as_expression(std::forward<Other>(other))
        };
    }

    template<typename Self>
    constexpr auto exp(this Self&& self) {
        return unary_operator{
            operators::exp{},
            std::forward<Self>(self)
        };
    }

    template<typename Self, concepts::arithmetic T, typename... V>
    constexpr void accumulate_derivatives(this Self&& self, T multiplier, derivatives<V...>& derivs) {
        if constexpr (contains_decay_v<Self, V...>)
            derivs.add_to_derivative_wrt(self, multiplier);
    }

 protected:
    template<typename Self, concepts::expression E, std::invocable<E> Partial>
        requires(std::constructible_from<std::invoke_result_t<Partial, E>, expression_value_t<E>>)
    constexpr std::invoke_result_t<Partial, E> partial_to(this Self&& self, E&& e, Partial&& partial) noexcept {
        using return_type = std::invoke_result_t<Partial, E>;
        using value_type = expression_value_t<E>;
        static_assert(
            !traits::is_constant<std::remove_cvref_t<E>>::value,
            "Derivative w.r.t. a constant requested"
        );
        if constexpr (!std::is_same_v<std::remove_cvref_t<Self>, std::remove_cvref_t<E>>)
            return partial(std::forward<E>(e));
        else
            return is_same_object(self, e) ? return_type{value_type{1}} : partial(std::forward<E>(e));
    }
};


template<concepts::unary_operator O, typename E>
class unary_operator : public expression_base {
 public:
    constexpr unary_operator(O&& op, E e) noexcept
    : _expression{std::forward<E>(e)}
    {}

    template<concepts::arithmetic T, typename... V>
    constexpr void accumulate_derivatives(T multiplier, derivatives<V...>& derivs) const {
        expression_base::accumulate_derivatives(multiplier, derivs);
        traits::derivative<O>::accumulate(_expression.get(), multiplier, derivs);
    }

    template<concepts::expression... _E>
    constexpr auto backpropagate(const _E&... e) const {
        return traits::derivative<O>::backpropagate(_expression.get(), e...);
    }

    constexpr auto value() const {
        return O{}(_expression.get().value());
    }

    template<concepts::expression _E>
        requires(concepts::derivable_unary_operator<O, E, _E>)
    constexpr auto partial_expression(_E&& e) const {
        return traits::derivative<O>::expression(_expression.get(), std::forward<_E>(e));
    }

 private:
    storage<E> _expression;
};

template<concepts::ownable O, typename E>
unary_operator(O&&, E&&) -> unary_operator<std::remove_cvref_t<O>, E>;


template<concepts::binary_operator O, typename A, typename B>
class binary_operator : public expression_base {
 public:
    constexpr binary_operator(O&&, A a, B b) noexcept
    : _a{std::forward<A>(a)}
    , _b{std::forward<B>(b)}
    {}

    template<concepts::arithmetic T, typename... V>
    constexpr void accumulate_derivatives(T multiplier, derivatives<V...>& derivs) const {
        expression_base::accumulate_derivatives(multiplier, derivs);
        traits::derivative<O>::accumulate(_a.get(), _b.get(), multiplier, derivs);
    }

    template<concepts::expression... E>
    constexpr auto backpropagate(const E&... e) const {
        return traits::derivative<O>::backpropagate(_a.get(), _b.get(), e...);
    }

    constexpr auto value() const {
        return O{}(_a.get().value(), _b.get().value());
    }

    template<concepts::expression E>
        requires(concepts::derivable_binary_operator<O, A, B, E>)
    constexpr auto partial_expression(E&& e) const {
        return traits::derivative<O>::expression(_a.get(), _b.get(), std::forward<E>(e));
    }

 private:
    storage<A> _a;
    storage<B> _b;
};

template<concepts::ownable O, typename A, typename B>
binary_operator(O&&, A&&, B&&) -> binary_operator<std::remove_cvref_t<O>, A, B>;

}  // namespace cppad::backward

namespace cppad::traits {

template<concepts::expression V> requires(is_variable_v<V>)
struct is_variable<cppad::backward::named_expression<V>> : public std::true_type {};

template<concepts::expression V> requires(is_variable_v<V>)
struct is_named_variable<cppad::backward::named_expression<V>> : public std::true_type {};

template<>
struct derivative<std::plus<void>> {
    static constexpr auto expression( // todo: inline
        const concepts::expression auto& a,
        const concepts::expression auto& b,
        const concepts::expression auto& var) {
        return a.partial_expression(var) + b.partial_expression(var);
    }

    static constexpr void accumulate(
        const concepts::expression auto& a,
        const concepts::expression auto& b,
        concepts::arithmetic auto multiplier,
        auto& derivatives) {
        a.accumulate_derivatives(multiplier, derivatives);
        b.accumulate_derivatives(multiplier, derivatives);
    }

    static constexpr auto backpropagate(
        const concepts::expression auto& a,
        const concepts::expression auto& b,
        const concepts::expression auto&... vars
    ) {
        auto [value_a, derivs_a] = a.backpropagate(vars...);
        auto [value_b, derivs_b] = b.backpropagate(vars...);
        auto result = value_a + value_b;
        return std::make_pair(result, std::move(derivs_a) + std::move(derivs_b));
    }
};

template<>
struct derivative<std::minus<void>> {
    static constexpr auto expression(
        const concepts::expression auto& a,
        const concepts::expression auto& b,
        const concepts::expression auto& var) {
        return a.partial_expression(var) - b.partial_expression(var);
    }

    static constexpr void accumulate(
        const concepts::expression auto& a,
        const concepts::expression auto& b,
        concepts::arithmetic auto multiplier,
        auto& derivatives) {
        a.accumulate_derivatives(multiplier, derivatives);
        b.accumulate_derivatives(std::negate{}(multiplier), derivatives);
    }

    static constexpr auto backpropagate(
        const concepts::expression auto& a,
        const concepts::expression auto& b,
        const concepts::expression auto&... vars
    ) {
        auto [value_a, derivs_a] = a.backpropagate(vars...);
        auto [value_b, derivs_b] = b.backpropagate(vars...);
        auto result = value_a - value_b;
        return std::make_pair(result, std::move(derivs_a) + std::move(derivs_b).scaled_with(-1));
    }
};
template<>
struct derivative<std::multiplies<void>> {
    static constexpr auto expression(
        const concepts::expression auto& a,
        const concepts::expression auto& b,
        const concepts::expression auto& var) {
        return a.partial_expression(var)*b + a*b.partial_expression(var);
    }

    static constexpr void accumulate(
        const concepts::expression auto& a,
        const concepts::expression auto& b,
        concepts::arithmetic auto multiplier,
        auto& derivatives) {
        a.accumulate_derivatives(multiplier*b.value(), derivatives);
        b.accumulate_derivatives(multiplier*a.value(), derivatives);
    }

    static constexpr auto backpropagate(
        const concepts::expression auto& a,
        const concepts::expression auto& b,
        const concepts::expression auto&... vars
    ) {
        auto [value_a, derivs_a] = a.backpropagate(vars...);
        auto [value_b, derivs_b] = b.backpropagate(vars...);
        auto result = value_a*value_b;
        return std::make_pair(
            result,
            std::move(derivs_a).scaled_with(value_b) + std::move(derivs_b).scaled_with(value_a)
        );
    }
};

template<>
struct derivative<cppad::backward::operators::exp> {
    static constexpr auto exp_op = cppad::backward::operators::exp{};

    static constexpr auto expression(
        const concepts::expression auto& e,
        const concepts::expression auto& var) {
        return e.exp()*e.partial_expression(var);
    }

    static constexpr void accumulate(
        const concepts::expression auto& e,
        concepts::arithmetic auto multiplier,
        auto& derivatives) {
        e.accumulate_derivatives(multiplier*exp_op(e.value()), derivatives);
    }

    static constexpr auto backpropagate(
        const concepts::expression auto& e,
        const concepts::expression auto&... vars
    ) {
        auto [value, derivs] = e.backpropagate(vars...);
        auto result = exp_op(value);
        return std::make_pair(result, std::move(derivs).scaled_with(result));
    }
};

}  // namespace cppad::traits

namespace std {

template<cppad::concepts::expression E>
constexpr auto exp(E&& e) {
    return std::forward<E>(e).exp();
}

}  // namespace std
