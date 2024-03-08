#pragma once

#include <cmath>
#include <type_traits>

#include <adpp/type_traits.hpp>
#include <adpp/backward/concepts.hpp>
#include <adpp/backward/bindings.hpp>
#include <adpp/backward/derivatives.hpp>

namespace adpp::backward {

template<typename... V>
inline constexpr auto wrt(V&&...) {
    return type_list<std::remove_cvref_t<V>...>{};
}

template<typename op, term... Ts>
struct expression;


#ifndef DOXYGEN
namespace detail {

    template<typename T>
    struct operands;
    template<typename op, typename... T>
    struct operands<expression<op, T...>> : std::type_identity<type_list<T...>> {};
    template<typename T>
    using operands_t = typename operands<T>::type;

    template<typename T>
    concept traversable_expression = is_symbol_v<std::remove_cvref_t<T>> or is_expression_v<std::remove_cvref_t<T>>;

    template<typename...>
    struct symbols_impl;

    template<typename E, typename... Ts> requires(is_symbol_v<std::remove_cvref_t<E>>)
    struct symbols_impl<E, type_list<Ts...>> {
        using type = std::conditional_t<
            symbolic<std::remove_cvref_t<E>>,
            typename unique_tuple<type_list<Ts...>, std::remove_cvref_t<E>>::type,
            typename unique_tuple<type_list<Ts...>>::type
        >;
    };

    template<typename E, typename... Ts> requires(!is_symbol_v<std::remove_cvref_t<E>>)
    struct symbols_impl<E, type_list<Ts...>> {
        using type = typename symbols_impl<operands_t<E>, type_list<Ts...>>::type;
    };

    template<typename E0, typename... Es, typename... Ts>
    struct symbols_impl<type_list<E0, Es...>, type_list<Ts...>> {
        using type = typename unique_tuple<
            typename merged_tuple<
                typename symbols_impl<E0, type_list<Ts...>>::type,
                typename symbols_impl<type_list<Es...>, type_list<Ts...>>::type
            >::type
        >::type;
    };

    // closures to stop recursion
    template<typename... Ts> struct symbols_impl<type_list<Ts...>> : std::type_identity<type_list<Ts...>> {};
    template<typename... Ts> struct symbols_impl<type_list<>, type_list<Ts...>> : std::type_identity<type_list<Ts...>> {};

    template<typename T> struct is_var : std::false_type {};
    template<typename T, auto _> struct is_var<var<T, _>> : std::true_type {};

}  // namespace detail
#endif  // DOXYGEN

template<detail::traversable_expression E>
struct symbols : detail::symbols_impl<E, type_list<>> {};

template<detail::traversable_expression E>
using symbols_t = typename symbols<E>::type;

template<detail::traversable_expression E>
inline constexpr auto symbols_of(const E&) {
    return symbols_t<E>{};
}


template<detail::traversable_expression E>
struct unbound_symbols : filtered_tuple<decayed_arg<is_unbound_symbol>::type, symbols_t<E>> {};

template<detail::traversable_expression E>
using unbound_symbols_t = typename unbound_symbols<E>::type;

template<detail::traversable_expression E>
inline constexpr auto unbound_symbols_of(const E&) {
    return unbound_symbols_t<E>{};
}


template<detail::traversable_expression E>
struct vars : filtered_tuple<decayed_arg<detail::is_var>::type, symbols_t<E>> {};

template<detail::traversable_expression E>
using vars_t = typename vars<E>::type;

template<detail::traversable_expression E>
inline constexpr auto variables_of(const E&) {
    return vars_t<E>{};
}



namespace op {

struct exp {
    template<typename T>
    constexpr auto operator()(const T& t) const {
        using std::exp;
        return exp(t);
    }
};

struct add : std::plus<void> {};
struct subtract : std::minus<void> {};
struct multiply : std::multiplies<void> {};
struct divide : std::divides<void> {};

}  // namespace op


template<typename op, typename... T>
struct back_propagation;

template<typename op, term... Ts>
struct expression {
    template<typename... B>
    constexpr decltype(auto) operator()(const bindings<B...>& operands) const {
        return op{}(Ts{}(operands)...);
    }

    template<typename Self, typename... B>
    constexpr auto gradient(this Self&& self, const bindings<B...>& bindings) {
        return self.back_propagate(bindings, variables_of(self)).second;
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
inline constexpr op_result_t<op::add, A, B> operator+(A&&, B&&) { return {}; }
template<term A, term B>
inline constexpr op_result_t<op::subtract, A, B> operator-(A&&, B&&) { return {}; }
template<term A, term B>
inline constexpr op_result_t<op::multiply, A, B> operator*(A&&, B&&) { return {}; }
template<term A, term B>
inline constexpr op_result_t<op::divide, A, B> operator/(A&&, B&&) { return {}; }
template<term A>
inline constexpr op_result_t<op::exp, A> exp(A&&) { return {}; }


template<typename A, typename B>
struct back_propagation<op::add, A, B> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_a, derivs_a] = A{}.back_propagate(b, vars);
        auto [value_b, derivs_b] = B{}.back_propagate(b, vars);
        return std::make_pair(value_a + value_b, std::move(derivs_a) + std::move(derivs_b));
    }
};

template<typename A, typename B>
struct back_propagation<op::subtract, A, B> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_a, derivs_a] = A{}.back_propagate(b, vars);
        auto [value_b, derivs_b] = B{}.back_propagate(b, vars);
        return std::make_pair(value_a - value_b, std::move(derivs_a) + std::move(derivs_b).scaled_with(-1));
    }
};

template<typename A, typename B>
struct back_propagation<op::multiply, A, B> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_a, derivs_a] = A{}.back_propagate(b, vars);
        auto [value_b, derivs_b] = B{}.back_propagate(b, vars);
        auto derivs = std::move(derivs_a).scaled_with(value_b) + std::move(derivs_b).scaled_with(value_a);
        return std::make_pair(value_a*value_b, std::move(derivs));
    }
};

template<typename A, typename B>
struct back_propagation<op::divide, A, B> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_a, derivs_a] = A{}.back_propagate(b, vars);
        auto [value_b, derivs_b] = B{}.back_propagate(b, vars);

        using TA = std::remove_cvref_t<decltype(value_a)>;
        using TB = std::remove_cvref_t<decltype(value_b)>;
        return std::make_pair(
            value_a/value_b,
            std::move(derivs_a).scaled_with(TB{1}/value_b)
            + std::move(derivs_b).scaled_with(TA{-1}*value_a/(value_b*value_b))
        );
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
