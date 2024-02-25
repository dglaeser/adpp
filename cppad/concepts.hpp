#pragma once

#include <limits>
#include <utility>
#include <concepts>

#include <cppad/type_traits.hpp>

namespace cppad {

#ifndef DOXYGEN
namespace detail {

    struct test_derivatives {
        constexpr double operator[](const auto&) const { return 0; }
        constexpr double& operator[](const auto&) { return std::declval<double&>(); }
        constexpr void add_to_derivative_wrt(const auto& t, auto value) {}
    };

    struct test_expression {
        constexpr double value() const { return 0.0; }
        constexpr auto backpropagate(const auto&) const {
            return std::make_pair(double{}, test_derivatives{});
        }
    };

    template<typename T>
    constexpr std::remove_cvref_t<T> as_copy(T&&) {
        return std::declval<std::remove_cvref_t<T>>();
    }

}  // end namespace detail
#endif  // DOXYGEN


namespace concepts {

template<typename T>
concept arithmetic = std::floating_point<std::remove_cvref_t<T>> or std::integral<std::remove_cvref_t<T>>;

template<typename T>
concept pair = is_pair_v<T>;

template<typename A, typename B>
concept same_decay_t_as = std::same_as<std::decay_t<A>, std::decay_t<B>>;

template<typename T>
concept ownable = is_ownable<T>::value;

template<typename T, typename V = detail::test_expression>
concept derivative_for = requires(const T& t, const V& variable) {
    { detail::as_copy(t[variable]) } -> arithmetic;
};
static_assert(derivative_for<detail::test_derivatives, detail::test_expression>);

template<typename T>
concept expression = requires(const T& t) {
    { t.value() } -> arithmetic;
    { t.backpropagate(detail::test_expression{}) } -> pair;
    { detail::as_copy(t.backpropagate(detail::test_expression{}).first) } -> concepts::arithmetic;
    { detail::as_copy(t.backpropagate(detail::test_expression{}).second) } -> derivative_for<detail::test_expression>;
};
static_assert(expression<detail::test_expression>);

template<typename T>
concept into_expression = requires(const T& t) {
    requires is_complete_v<as_expression<std::remove_cvref_t<T>>>;
    { as_expression<std::remove_cvref_t<T>>::get(t) } -> expression;
};

template<typename T>
concept unary_operator = std::default_initializable<T> and requires(const T& t) {
    { detail::as_copy(t(double{})) } -> arithmetic;
};

template<typename T>
concept binary_operator = std::default_initializable<T> and requires(const T& t) {
    { detail::as_copy(t(double{}, double{})) } -> arithmetic;
};

template<typename T, typename E, typename V>
concept derivable_unary_operator
    = unary_operator<T>
    and is_complete_v<differentiator<T>>
    and requires(const T& t, const E& e, const V& variable) {
        { differentiator<T>::backpropagate(e, variable) };
        { differentiator<T>::expression(e, variable) } -> expression;
    };

template<typename T, typename A, typename B, typename V>
concept derivable_binary_operator
    = binary_operator<T>
    and expression<A> and expression<B>
    and is_complete_v<differentiator<T>>
    and requires(const T& t, const A& a, const B& b, const V& variable) {
        { differentiator<T>::backpropagate(a, b, variable) };
        { differentiator<T>::expression(a, b, variable) } -> expression;
    };

}  // namespace concepts


template<typename E>
    requires(!concepts::arithmetic<E>)
struct as_expression<E> {
    template<typename _E> requires(concepts::same_decay_t_as<E, _E>)
    static constexpr decltype(auto) get(_E&& e) noexcept {
        return std::forward<_E>(e);
    }
};

template<concepts::arithmetic T>
struct undefined_value<T> {
    static constexpr T value = std::numeric_limits<T>::max();
};

template<concepts::expression E>
struct expression_value<E> : public std::type_identity<std::remove_cvref_t<decltype(std::declval<const E&>().value())>> {};

}  // namespace cppad
