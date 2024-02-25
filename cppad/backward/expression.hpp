#pragma once

#include <cmath>
#include <utility>
#include <type_traits>
#include <functional>
#include <concepts>
#include <ostream>

#include <cppad/common.hpp>
#include <cppad/backward/operators.hpp>
#include <cppad/backward/derivatives.hpp>

namespace cppad::backward {

// forward declarations
template<concepts::unary_operator O, typename E>
class unary_operator;
template<concepts::binary_operator O, typename A, typename B>
class binary_operator;


struct expression_base {
    template<typename Self, concepts::into_expression Other>
    constexpr auto operator+(this Self&& self, Other&& other) noexcept {
        return binary_operator{
            std::plus{},
            std::forward<Self>(self),
            to_expression(std::forward<Other>(other))
        };
    }

    template<typename Self, concepts::into_expression Other>
    constexpr auto operator-(this Self&& self, Other&& other) noexcept {
        return binary_operator{
            std::minus{},
            std::forward<Self>(self),
            to_expression(std::forward<Other>(other))
        };
    }

    template<typename Self, concepts::into_expression Other>
    constexpr auto operator*(this Self&& self, Other&& other) noexcept {
        return binary_operator{
            std::multiplies{},
            std::forward<Self>(self),
            to_expression(std::forward<Other>(other))
        };
    }

    template<typename Self>
    constexpr auto exp(this Self&& self) {
        return unary_operator{
            operators::exp{},
            std::forward<Self>(self)
        };
    }

 protected:
    template<typename Self, concepts::expression E, std::invocable<E> Partial>
        requires(std::constructible_from<std::invoke_result_t<Partial, E>, expression_value_t<E>>)
    constexpr std::invoke_result_t<Partial, E> partial_to(this Self&& self, E&& e, Partial&& partial) noexcept {
        using return_type = std::invoke_result_t<Partial, E>;
        using value_type = expression_value_t<E>;
        static_assert(
            !is_constant_v<std::remove_cvref_t<E>>,
            "Derivative w.r.t. a constant requested"
        );
        if constexpr (!std::is_same_v<std::remove_cvref_t<Self>, std::remove_cvref_t<E>>)
            return partial(std::forward<E>(e));
        else
            return is_same_object(self, e) ? return_type{value_type{1}} : partial(std::forward<E>(e));
    }
};


template<concepts::expression E>
class expression {
 public:
    constexpr expression(E&& e)
    : _e{std::move(e)}
    {}

    template<concepts::expression... _E>
    constexpr auto back_propagate(const _E&... e) const {
        return _e.back_propagate(e...);
    }

    constexpr decltype(auto) value() const {
        return _e.value();
    }

    template<typename _E>
    constexpr decltype(auto) differentiate_wrt(_E&& e) const {
        return _e.differentiate_wrt(std::forward<_E>(e));
    }

 private:
    E _e;
};

template<concepts::expression E>
expression(E&&) -> expression<std::remove_cvref_t<E>>;


template<concepts::expression E>
class named_expression {
    static_assert(!std::is_lvalue_reference_v<E>);

 public:
    using expression = E;

    constexpr named_expression(const E& e, const char* name) noexcept
    : _e{e}
    , _name{name}
    {}

    constexpr const char* name() const noexcept {
        return _name;
    }

    template<concepts::expression _E>
    constexpr bool operator==(_E&& e) const noexcept {
        return is_same_object(e, _e);
    }

 private:
    const E& _e;
    const char* _name;
};

template<concepts::expression E>
    requires(std::is_lvalue_reference_v<E>)
named_expression(E&&, const char*) -> named_expression<std::remove_cvref_t<E>>;


template<typename... V>
    requires(std::conjunction_v<is_named_expression<V>...>)
class expression_name_map : variadic_accessor<V...> {
    using base = variadic_accessor<V...>;

    template<typename A, typename B>
    struct same_decay : public std::bool_constant<concepts::same_decay_t_as<A, B>> {};

    template<typename T>
    using expr_t = typename T::expression;

 public:
    using base::base;

    template<typename T>
    static constexpr bool is_contained = std::disjunction_v<same_decay<T, expr_t<V>>...>;

    template<typename T>
    static constexpr bool contains(T&& t) noexcept {
        return is_contained<T>;
    }

    template<typename T> requires(is_contained<T>)
    constexpr decltype(auto) name_of(const T& t) const noexcept {
        return this->get(this->template index_of<named_expression<T>>()).name();
    }
};

template<typename... V>
    requires(std::conjunction_v<std::negation<std::is_lvalue_reference<V>>...>)
expression_name_map(V&&...) -> expression_name_map<std::remove_cvref_t<V>...>;


template<concepts::unary_operator O, typename E>
class unary_operator : public expression_base {
 public:
    constexpr unary_operator(O&& op, E e) noexcept
    : _expression{std::forward<E>(e)}
    {}

    template<concepts::expression... _E>
    constexpr auto back_propagate(const _E&... e) const {
        return differentiator<O>::back_propagate(_expression.get(), e...);
    }

    constexpr auto value() const {
        return O{}(_expression.get().value());
    }

    template<typename _E>
        requires(concepts::derivable_unary_operator<O, E, _E>)
    constexpr auto differentiate_wrt(_E&& e) const {
        return differentiator<O>::differentiate(_expression.get(), std::forward<_E>(e));
    }

    template<typename... V>  // TODO: binary_op_formatter_concept
    constexpr std::ostream& stream(std::ostream& out, const expression_name_map<V...>& name_map) const {
        formatter<O>::format(out, _expression.get(), name_map);
        return out;
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

    template<concepts::expression... E>
    constexpr auto back_propagate(const E&... e) const {
        return differentiator<O>::back_propagate(_a.get(), _b.get(), e...);
    }

    constexpr auto value() const {
        return O{}(_a.get().value(), _b.get().value());
    }

    template<typename E>
        requires(concepts::derivable_binary_operator<O, A, B, E>)
    constexpr auto differentiate_wrt(E&& e) const {
        return differentiator<O>::differentiate(_a.get(), _b.get(), std::forward<E>(e));
    }

    template<typename... V>  // TODO: binary_op_formatter_concept
    constexpr std::ostream& stream(std::ostream& out, const expression_name_map<V...>& name_map) const {
        formatter<O>::format(out, _a.get(), _b.get(), name_map);
        return out;
    }

 private:
    storage<A> _a;
    storage<B> _b;
};

template<concepts::ownable O, typename A, typename B>
binary_operator(O&&, A&&, B&&) -> binary_operator<std::remove_cvref_t<O>, A, B>;


template<concepts::expression E, typename V>
inline constexpr auto differentiate(E&& expression, const std::tuple<V>& var) {
    return expression.differentiate_wrt(std::get<0>(var));
}

}  // namespace cppad::backward

namespace cppad {

template<concepts::expression V> requires(is_variable_v<V>)
struct is_variable<cppad::backward::named_expression<V>> : public std::true_type {};

template<concepts::expression V>
struct is_named_expression<cppad::backward::named_expression<V>> : public std::true_type {};

}  // namespace cppad

namespace std {

template<cppad::concepts::expression E>
constexpr auto exp(E&& e) {
    return std::forward<E>(e).exp();
}

}  // namespace std
