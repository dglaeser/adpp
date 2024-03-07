#pragma once

#include <utility>
#include <type_traits>
#include <functional>

#include <adpp/type_traits.hpp>
#include <adpp/concepts.hpp>

#include <adpp/backward/concepts.hpp>
#include <adpp/backward/operators.hpp>
#include <adpp/backward/derivatives.hpp>
#include <adpp/backward/bindings.hpp>

namespace adpp::backward {

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
constexpr decltype(auto) operator+(T&& t, E&& expression) noexcept {
    return as_operand(std::forward<T>(t)).operator+(expression);
}

template<concepts::arithmetic T, typename E>
    requires(std::derived_from<std::remove_cvref_t<E>, operand>)
constexpr decltype(auto) operator-(T&& t, E&& expression) noexcept {
    return as_operand(std::forward<T>(t)).operator-(expression);
}

template<concepts::arithmetic T, typename E>
    requires(std::derived_from<std::remove_cvref_t<E>, operand>)
constexpr decltype(auto) operator*(T&& t, E&& expression) noexcept {
    return as_operand(std::forward<T>(t)).operator*(expression);
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

    template<typename... B>
    constexpr decltype(auto) evaluate_at(const bindings<B...>&) const noexcept {
        return _value.get();
    }

    template<typename Self, typename... B, typename... V>
    constexpr auto back_propagate(this Self&& self, const bindings<B...>& bindings, const type_list<V...>&) {
        return std::make_pair(self.evaluate_at(bindings), derivatives<T, V...>{});
    }

    template<typename Self, typename V>
    constexpr auto differentiate_wrt(this Self&&, const type_list<V>&) {
        return val<T>{T{0}};
    }

    template<typename... V>
    constexpr std::ostream& stream(std::ostream& out, const bindings<V...>&) const {
        out << _value.get();
        return out;
    }

 private:
    storage<T> _value;
};

template<typename T>
val(T&&) -> val<T>;

template<typename T>
struct is_symbol<val<T>> : std::true_type {};

namespace traits {

template<concepts::arithmetic T>
struct into_operand<T> {
    template<concepts::same_decay_t_as<T> _T>
    static constexpr auto get(_T&& t) noexcept {
        return val{std::forward<_T>(t)};
    }
};

}  // namespace traits

template<typename O, typename E>
class unary_operator : public operand  {
 public:
    using operand_type = std::remove_cvref_t<E>;

    constexpr unary_operator(O&&, E e) noexcept
    : _expression{std::forward<E>(e)}
    {}

    template<typename... B>
    constexpr decltype(auto) evaluate_at(const bindings<B...>& values) const noexcept {
        return O{}(_expression.get().evaluate_at(values));
    }

    template<typename... B, typename... V>
    constexpr auto back_propagate(const bindings<B...>& values, const type_list<V...>& vars) const {
        return traits::differentiator<O>::back_propagate(values, _expression.get(), vars);
    }

    template<typename V>
    constexpr auto differentiate_wrt(const type_list<V>& var) const {
        return traits::differentiator<O>::differentiate(_expression.get(), var);
    }

    template<typename... V>
    constexpr std::ostream& stream(std::ostream& out, const bindings<V...>& name_bindings) const {
        traits::formatter<O>::format(out, _expression.get(), name_bindings);
        return out;
    }

    constexpr const operand_type& operand() const {
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
    using first_type = std::remove_cvref_t<A>;
    using second_type = std::remove_cvref_t<B>;

    constexpr binary_operator(O&&, A a, B b) noexcept
    : _a{std::forward<A>(a)}
    , _b{std::forward<B>(b)}
    {}

    template<typename... _B>
    constexpr decltype(auto) evaluate_at(const bindings<_B...>& values) const noexcept {
        return O{}(
            _a.get().evaluate_at(values),
            _b.get().evaluate_at(values)
        );
    }

    template<typename... _B, typename... V>
    constexpr auto back_propagate(const bindings<_B...>& bindings, const type_list<V...>& vars) const {
        return traits::differentiator<O>::back_propagate(bindings, _a.get(), _b.get(), vars);
    }

    template<typename V>
    constexpr auto differentiate_wrt(const type_list<V>& var) const {
        return traits::differentiator<O>::differentiate(_a.get(), _b.get(), var);
    }

    template<typename... V>
    constexpr std::ostream& stream(std::ostream& out, const bindings<V...>& name_bindings) const {
        traits::formatter<O>::format(out, _a.get(), _b.get(), name_bindings);
        return out;
    }

    constexpr const first_type& first() const { return _a.get(); }
    constexpr const second_type& second() const { return _b.get(); }

 private:
    storage<A> _a;
    storage<B> _b;
};

template<concepts::ownable O, typename A, typename B>
binary_operator(O&&, A&&, B&&) -> binary_operator<std::remove_cvref_t<O>, A, B>;

namespace traits {

template<typename O, typename E>
struct sub_expressions<unary_operator<O, E>> {
    using operands = type_list<typename unary_operator<O, E>::operand_type>;

    static constexpr auto get(const unary_operator<O, E>& op) {
        return std::forward_as_tuple(op.operand());
    }
};

template<typename O, typename A, typename B>
struct sub_expressions<binary_operator<O, A, B>> {
    using operands = type_list<
        typename binary_operator<O, A, B>::first_type,
        typename binary_operator<O, A, B>::second_type
    >;

    static constexpr auto get(const binary_operator<O, A, B>& op) {
        return std::forward_as_tuple(op.first(), op.second());
    }
};

template<typename T>
    requires(std::derived_from<std::remove_cvref_t<T>, operand>)
struct into_operand<T> {
    template<typename _T> requires(concepts::same_decay_t_as<T, _T>)
    static constexpr decltype(auto) get(_T&& t) noexcept {
        return std::forward<_T>(t);
    }
};

}  // namespace traits
}  // namespace adpp::backward

namespace std {

template<typename T>
    requires(std::derived_from<std::remove_cvref_t<T>, adpp::backward::operand>)
constexpr auto exp(T&& t) {
    return std::forward<T>(t).exp();
}

}  // namespace std
