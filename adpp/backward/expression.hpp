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

template<typename op, typename... T>
struct format;

template<typename op, typename... T>
struct derivative;

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

    template<typename V>
    constexpr auto differentiate_wrt(const type_list<V>& var) const {
        return derivative<op, Ts...>{}(var);
    }

    template<typename... B>
    constexpr std::ostream& stream(std::ostream& out, const bindings<B...>& name_bindings) const {
        format<op, Ts...>{}(out, name_bindings);
        return out;
    }
};




template<typename T> struct _is_val : std::false_type {};
template<auto v> struct _is_val<_val<v>> : std::true_type {};

template<typename T> requires(!_is_val<std::remove_cvref_t<T>>::value)
constexpr bool is_zero() {
    return false;
}
template<typename T> requires(_is_val<std::remove_cvref_t<T>>::value)
constexpr bool is_zero() {
    return T::value == 0;
}
template<typename T>
constexpr bool is_zero(const T&) {
    return is_zero<T>();
}

template<typename T>
constexpr bool is_val() {
    return _is_val<std::remove_cvref_t<T>>::value;
}

template<typename T>
constexpr bool is_val(const T&) {
    return is_val<T>();
}




template<typename op, term... Ts>
struct is_expression<expression<op, Ts...>> : std::true_type {};

// for operations between vals, we use the operators defined in-class
template<typename op, term... Ts>
    requires(!std::conjunction_v<_is_val<std::remove_cvref_t<Ts>>...>)
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

template<typename A, typename B>
struct derivative<op::add, A, B> {
    template<typename V>
    constexpr auto operator()(const type_list<V>& v) {
        return A{}.differentiate_wrt(v) + B{}.differentiate_wrt(v);
    }
};

template<typename A, typename B>
struct derivative<op::subtract, A, B> {
    template<typename V>
    constexpr auto operator()(const type_list<V>& v) {
        return A{}.differentiate_wrt(v) - B{}.differentiate_wrt(v);
    }
};

template<typename A, typename B>
struct derivative<op::multiply, A, B> {
    template<typename V>
    constexpr auto operator()(const type_list<V>& v) {
        return A{}.differentiate_wrt(v)*B{} + A{}*B{}.differentiate_wrt(v);
    }
};

template<typename A, typename B>
struct derivative<op::divide, A, B> {
    template<typename V>
    constexpr auto operator()(const type_list<V>& v) {
        return A{}.differentiate_wrt(v)*_val<1>{}/B{}
            - A{}*B{}.differentiate_wrt(v)/(B{}*B{});
    }
};

template<typename A>
struct derivative<op::exp, A> {
    template<typename V>
    constexpr auto operator()(const type_list<V>& v) {
        return exp(A{})*A{}.differentiate_wrt(v);
    }
};

template<typename A, typename B>
struct format<op::add, A, B> {
    template<typename... N>
    constexpr void operator()(std::ostream& out, const bindings<N...>& name_map) {
        A{}.stream(out, name_map);
        out << " + ";
        B{}.stream(out, name_map);
    }
};

template<typename A, typename B>
struct format<op::subtract, A, B> {
    template<typename... N>
    constexpr void operator()(std::ostream& out, const bindings<N...>& name_map) {
        A{}.stream(out, name_map);
        out << " - ";
        B{}.stream(out, name_map);
    }
};

#ifndef DOXYGEN
namespace detail {

    template<typename A, typename... N>
    inline constexpr void in_braces(std::ostream& out, const A& a, const bindings<N...>& name_map) {
        constexpr bool use_braces = !is_symbol_v<A>;
        if constexpr (use_braces) out << "(";
        a.stream(out, name_map);
        if constexpr (use_braces) out << ")";
    }

}  // namespace detail
#endif  // DOXYGEN

template<typename A, typename B>
struct format<op::divide, A, B> {
    template<typename... N>
    constexpr void operator()(std::ostream& out, const bindings<N...>& name_map) {
        detail::in_braces(out, A{}, name_map);
        out << "/";
        detail::in_braces(out, B{}, name_map);
    }
};

template<typename A, typename B>
struct format<op::multiply, A, B> {
    template<typename... N>
    constexpr void operator()(std::ostream& out, const bindings<N...>& name_map) {
        detail::in_braces(out, A{}, name_map);
        out << "*";
        detail::in_braces(out, B{}, name_map);
    }
};

template<typename A>
struct format<op::exp, A> {
    template<typename... N>
    constexpr void operator()(std::ostream& out, const bindings<N...>& name_map) {
        out << "exp(";
        A{}.stream(out, name_map);
        out << ")";
    }
};

template<typename E, typename... N> requires(is_expression_v<E>)
inline constexpr void print_to(std::ostream& s, const E& expression, const bindings<N...>& name_map) {
    expression.stream(s, name_map);
}

}  // namespace adpp::backward
