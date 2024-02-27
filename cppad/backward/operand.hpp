#pragma once

#include <utility>
#include <type_traits>
#include <functional>

#include <cppad/type_traits.hpp>
#include <cppad/concepts.hpp>

#include <cppad/backward/concepts.hpp>
#include <cppad/backward/operators.hpp>
#include <cppad/backward/derivative.hpp>

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

    template<typename B>
    constexpr decltype(auto) evaluate_at(B&&) const noexcept {
        return _value.get();
    }

    template<typename Self, typename B, typename... V>
    constexpr auto back_propagate(this Self&& self, const B& bindings, const V&... vars) {
        return std::make_pair(self.evaluate_at(bindings), derivatives{T{}, vars...});
    }

 private:
    storage<T> _value;
};

template<typename T>
val(T&&) -> val<T>;

namespace traits {

template<typename T>
struct is_leaf_expression<val<T>> : public std::true_type {};

}  // namespace traits

template<typename O, typename E>
class unary_operator : public operand  {
 public:
    constexpr unary_operator(O&& op, E e) noexcept
    : _expression{std::forward<E>(e)}
    {}

    template<typename B>
    constexpr decltype(auto) evaluate_at(const B& bindings) const noexcept {
        return O{}(_expression.get().evaluate_at(bindings));
    }

    template<typename B, typename... V>
    constexpr auto back_propagate(const B& bindings, const V&... vars) const {
        return differentiator<O>::back_propagate(bindings, _expression.get(), vars...);
    }

    constexpr const auto& operand() const {
        return _expression.get();
    }

 private:
    storage<E> _expression;
};

template<concepts::ownable O, typename E>
unary_operator(O&&, E&&) -> unary_operator<std::remove_cvref_t<O>, E>;


template<typename O, typename A, typename B>
class binary_operator : public operand {
 public:
    constexpr binary_operator(O&&, A a, B b) noexcept
    : _a{std::forward<A>(a)}
    , _b{std::forward<B>(b)}
    {}

    template<typename _B>
    constexpr decltype(auto) evaluate_at(const _B& bindings) const noexcept {
        return O{}(
            _a.get().evaluate_at(bindings),
            _b.get().evaluate_at(bindings)
        );
    }

    template<typename _B, typename... V>
    constexpr auto back_propagate(const _B& bindings, const V&... vars) const {
        return differentiator<O>::back_propagate(bindings, _a.get(), _b.get(), vars...);
    }

    constexpr const auto& operand0() const { return _a.get(); }
    constexpr const auto& operand1() const { return _b.get(); }

 private:
    storage<A> _a;
    storage<B> _b;
};

template<concepts::ownable O, typename A, typename B>
binary_operator(O&&, A&&, B&&) -> binary_operator<std::remove_cvref_t<O>, A, B>;

namespace traits {

template<typename O, typename E>
struct sub_expressions<unary_operator<O, E>> {
    static constexpr auto get(const unary_operator<O, E>& op) {
        return std::forward_as_tuple(op.operand());
    }
};

template<typename O, typename A, typename B>
struct sub_expressions<binary_operator<O, A, B>> {
    static constexpr auto get(const binary_operator<O, A, B>& op) {
        return std::forward_as_tuple(op.operand0(), op.operand1());
    }
};

}  // namespace traits
}  // namespace cppad::backward

namespace std {

template<typename T>
    requires(std::derived_from<std::remove_cvref_t<T>, cppad::backward::operand>)
constexpr auto exp(T&& t) {
    return std::forward<T>(t).exp();
}

}  // namespace std
