#pragma once

#include <utility>
#include <type_traits>
#include <functional>

#include <cppad/type_traits.hpp>
#include <cppad/concepts.hpp>

#include <cppad/backward/concepts.hpp>
#include <cppad/backward/operators.hpp>

namespace cppad::backward {

// forward declarations
template<typename O, typename E>
class unary_operator;
template<typename O, typename A, typename B>
class binary_operator;

struct operand {
    template<typename Self, into_operand O>
    constexpr auto operator+(this Self&& self, O&& other) noexcept {
        return binary_operator{std::plus{}, std::forward<Self>(self), as_operand(std::forward<O>(other))};
    }

    template<typename Self, into_operand O>
    constexpr auto operator-(this Self&& self, O&& other) noexcept {
        return binary_operator{std::minus{}, std::forward<Self>(self), as_operand(std::forward<O>(other))};
    }

    template<typename Self, into_operand O>
    constexpr auto operator*(this Self&& self, O&& other) noexcept {
        return binary_operator{std::multiplies{}, std::forward<Self>(self), as_operand(std::forward<O>(other))};
    }

    template<typename Self>
    constexpr auto exp(this Self&& self) {
        return unary_operator{operators::exp{}, std::forward<Self>(self) };
    }
};

template<concepts::arithmetic T, typename E>
    requires(std::derived_from<std::remove_cvref_t<E>, operand>)
constexpr decltype(auto) operator+(T t, E&& expression) noexcept {
    return as_operand(t).operator+(expression);
}

template<concepts::arithmetic T, typename E>
    requires(std::derived_from<std::remove_cvref_t<E>, operand>)
constexpr decltype(auto) operator-(T t, E&& expression) noexcept {
    return as_operand(t).operator-(expression);
}

template<concepts::arithmetic T, typename E>
    requires(std::derived_from<std::remove_cvref_t<E>, operand>)
constexpr decltype(auto) operator*(T t, E&& expression) noexcept {
    return as_operand(t).operator*(expression);
}

template<typename T>
struct val : operand {
    template<concepts::same_decay_t_as<T> _T>
    constexpr val(_T&& value)
    : _value{std::forward<_T>(value)}
    {}

    template<typename Self>
    constexpr decltype(auto) unwrap(this Self&& self) {
        if constexpr (!std::is_lvalue_reference_v<Self>)
            return std::move(self._value).get();
        else
            return self._value.get();
    }

 private:
    storage<T> _value;
};

template<typename T>
val(T&&) -> val<T>;

template<typename O, typename E>
class unary_operator {
 public:
    constexpr unary_operator(O&& op, E e) noexcept
    : _expression{std::forward<E>(e)}
    {}

    constexpr const auto& operand() const {
        return _expression.get();
    }

 private:
    storage<E> _expression;
};

template<concepts::ownable O, typename E>
unary_operator(O&&, E&&) -> unary_operator<std::remove_cvref_t<O>, E>;


template<typename O, typename A, typename B>
class binary_operator {
 public:
    constexpr binary_operator(O&&, A a, B b) noexcept
    : _a{std::forward<A>(a)}
    , _b{std::forward<B>(b)}
    {}

    constexpr const auto& operand0() const { return _a.get(); }
    constexpr const auto& operand1() const { return _b.get(); }

 private:
    storage<A> _a;
    storage<B> _b;
};

template<concepts::ownable O, typename A, typename B>
binary_operator(O&&, A&&, B&&) -> binary_operator<std::remove_cvref_t<O>, A, B>;

}  // namespace cppad::backward

namespace std {

template<typename T>
    requires(std::derived_from<std::remove_cvref_t<T>, cppad::backward::operand>)
constexpr auto exp(T&& t) {
    return std::forward<T>(t).exp();
}

}  // namespace std
