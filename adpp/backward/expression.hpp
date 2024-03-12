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


// TODO: get rid of forward decls
template<typename op, term... Ts>
struct expression;
template<typename E>
    requires(is_expression_v<E>)
struct function;


#ifndef DOXYGEN
namespace detail {

    template<typename T>
    struct operands;
    template<typename op, typename... T>
    struct operands<expression<op, T...>> : std::type_identity<type_list<T...>> {};
    template<typename E>
    struct operands<function<E>> : operands<E> {};
    template<typename T>
    using operands_t = typename operands<std::remove_cvref_t<T>>::type;

    template<typename T>
    concept traversable_expression = is_symbol_v<std::remove_cvref_t<T>> or is_expression_v<std::remove_cvref_t<T>>;

    template<typename...>
    struct symbols_impl;

    template<typename E, typename... Ts> requires(is_symbol_v<std::remove_cvref_t<E>>)
    struct symbols_impl<E, type_list<Ts...>> {
        using type = std::conditional_t<
            symbolic<std::remove_cvref_t<E>>,
            typename unique_types<type_list<Ts...>, std::remove_cvref_t<E>>::type,
            typename unique_types<type_list<Ts...>>::type
        >;
    };

    template<typename E, typename... Ts> requires(!is_symbol_v<std::remove_cvref_t<E>>)
    struct symbols_impl<E, type_list<Ts...>> {
        using type = typename symbols_impl<operands_t<E>, type_list<Ts...>>::type;
    };

    template<typename E0, typename... Es, typename... Ts>
    struct symbols_impl<type_list<E0, Es...>, type_list<Ts...>> {
        using type = typename unique_types<
            typename merged_types<
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
struct unbound_symbols : filtered_types<decayed_trait<is_unbound_symbol>::type, symbols_t<E>> {};

template<detail::traversable_expression E>
using unbound_symbols_t = typename unbound_symbols<E>::type;

template<detail::traversable_expression E>
inline constexpr auto unbound_symbols_of(const E&) {
    return unbound_symbols_t<E>{};
}


template<detail::traversable_expression E>
struct vars : filtered_types<decayed_trait<detail::is_var>::type, symbols_t<E>> {};

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


template<typename R, typename op, typename... T>
struct back_propagation;

template<typename op, typename... T>
struct format;

template<typename op, typename... T>
struct derivative;

template<typename op, term... Ts>
struct expression {
    constexpr expression() = default;
    constexpr expression(const op&, const Ts&...) noexcept {}

    template<typename... B>
    constexpr decltype(auto) evaluate(const bindings<B...>& operands) const {
        return op{}(Ts{}.evaluate(operands)...);
    }

    template<scalar R, typename Self, typename... B, typename... V>
    constexpr auto back_propagate(this Self&& self, const bindings<B...>& bindings, const type_list<V...>& vars) {
        auto [value, derivs] = back_propagation<R, op, Ts...>{}(bindings, vars);
        if constexpr (contains_decayed_v<Self, V...>)
            derivs[self] = 1.0;
        return std::make_pair(std::move(value), std::move(derivs));
    }

    template<typename V>
    constexpr auto differentiate_wrt(const type_list<V>& var) const {
        return derivative<op, Ts...>{}(var);
    }

    template<typename... B>
    constexpr void export_to(std::ostream& out, const bindings<B...>& name_bindings) const {
        format<op, Ts...>{}(out, name_bindings);
    }
};


template<typename op, typename... Ts>
expression(op&&, Ts&&...) -> expression<std::remove_cvref_t<op>, std::remove_cvref_t<Ts>...>;


template<typename T> struct _is_val : std::false_type {};
template<auto v> struct _is_val<constant<v>> : std::true_type {};

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

// for arithmetic operations between vals, we use the operators defined in-class
template<typename op, term... Ts>
    requires(!std::conjunction_v<_is_val<std::remove_cvref_t<Ts>>...>)
using arithmetic_op_result_t = expression<op, std::remove_cvref_t<Ts>...>;

template<typename op, term... Ts>
using op_result_t = expression<op, std::remove_cvref_t<Ts>...>;

template<typename T>
concept to_term = term<std::remove_cvref_t<T>> or scalar<std::remove_cvref_t<T>>;

template<to_term T>
inline constexpr decltype(auto) as_term(T&& t) noexcept {
    if constexpr (term<std::remove_cvref_t<T>>)
        return std::forward<T>(t);
    else
        return val{std::forward<T>(t)};
}

template<to_term T, auto _ = [] () {}>
using term_t = std::conditional_t<
    scalar<std::remove_cvref_t<T>>,
    val<std::remove_cvref_t<T>, _>,
    std::remove_cvref_t<T>
>;

template<typename... Ts>
concept all_vals = std::conjunction_v<_is_val<std::remove_cvref_t<Ts>>...>;

template<to_term A, to_term B> requires(!all_vals<A, B>)
inline constexpr auto operator+(A&& a, B&& b) {
    return expression{op::add{}, as_term(std::forward<A>(a)), as_term(std::forward<B>(b))};
}
template<to_term A, to_term B> requires(!all_vals<A, B>)
inline constexpr auto operator-(A&& a, B&& b) {
    return expression{op::subtract{}, as_term(std::forward<A>(a)), as_term(std::forward<B>(b))};
}
template<to_term A, to_term B> requires(!all_vals<A, B>)
inline constexpr auto operator*(A&& a, B&& b) {
    return expression{op::multiply{}, as_term(std::forward<A>(a)), as_term(std::forward<B>(b))};
}
template<to_term A, to_term B> requires(!all_vals<A, B>)
inline constexpr auto operator/(A&& a, B&& b) {
    return expression{op::divide{}, as_term(std::forward<A>(a)), as_term(std::forward<B>(b))};
}
template<to_term A>
inline constexpr op_result_t<op::exp, A> exp(A&& a) {
    return expression{op::exp{}, as_term(std::forward<A>(a))};
}


template<typename R, typename A, typename B>
struct back_propagation<R, op::add, A, B> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_a, derivs_a] = A{}.template back_propagate<R>(b, vars);
        auto [value_b, derivs_b] = B{}.template back_propagate<R>(b, vars);
        return std::make_pair(value_a + value_b, std::move(derivs_a) + std::move(derivs_b));
    }
};

template<typename R, typename A, typename B>
struct back_propagation<R, op::subtract, A, B> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_a, derivs_a] = A{}.template back_propagate<R>(b, vars);
        auto [value_b, derivs_b] = B{}.template back_propagate<R>(b, vars);
        return std::make_pair(value_a - value_b, std::move(derivs_a) + std::move(derivs_b).scaled_with(-1));
    }
};

template<typename R, typename A, typename B>
struct back_propagation<R, op::multiply, A, B> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_a, derivs_a] = A{}.template back_propagate<R>(b, vars);
        auto [value_b, derivs_b] = B{}.template back_propagate<R>(b, vars);
        auto derivs = std::move(derivs_a).scaled_with(value_b) + std::move(derivs_b).scaled_with(value_a);
        return std::make_pair(value_a*value_b, std::move(derivs));
    }
};

template<typename R, typename A, typename B>
struct back_propagation<R, op::divide, A, B> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_a, derivs_a] = A{}.template back_propagate<R>(b, vars);
        auto [value_b, derivs_b] = B{}.template back_propagate<R>(b, vars);

        using TA = std::remove_cvref_t<decltype(value_a)>;
        using TB = std::remove_cvref_t<decltype(value_b)>;
        return std::make_pair(
            value_a/value_b,
            std::move(derivs_a).scaled_with(TB{1}/value_b)
            + std::move(derivs_b).scaled_with(TA{-1}*value_a/(value_b*value_b))
        );
    }
};

template<typename R, typename A>
struct back_propagation<R, op::exp, A> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_inner, derivs_inner] = A{}.template back_propagate<R>(b, vars);
        auto result = op::exp{}(value_inner);
        return std::make_pair(std::move(result), std::move(derivs_inner).scaled_with(result));
    }
};

template<typename T0, typename T1, typename F0, typename F1, typename F>
constexpr auto _decide(T0 t0, T1 t1, const F0& f0, const F1& f1, const F& f) {
    if constexpr (is_zero<T0>())
        return f1(t1);
    else if constexpr (is_zero<T1>())
        return f0(t0);
    else
        return f(t0, t1);
}

template<typename A, typename B>
struct derivative<op::add, A, B> {
    template<typename V>
    constexpr auto operator()(const type_list<V>& v) {
        return _decide(
            A{}.differentiate_wrt(v),
            B{}.differentiate_wrt(v),
            [] (auto&& da_dv) { return da_dv; },
            [] (auto&& db_dv) { return db_dv; },
            [] (auto&& da_dv, auto&& db_dv) { return da_dv + db_dv; }
        );
    }
};

template<typename A, typename B>
struct derivative<op::subtract, A, B> {
    template<typename V>
    constexpr auto operator()(const type_list<V>& v) {
        return _decide(
            A{}.differentiate_wrt(v),
            B{}.differentiate_wrt(v),
            [] (auto&& da_dv) { return da_dv; },
            [] (auto&& db_dv) { return cval<-1>*db_dv; },
            [] (auto&& da_dv, auto&& db_dv) { return da_dv - db_dv; }
        );
    }
};

template<typename A, typename B>
struct derivative<op::multiply, A, B> {
    template<typename V>
    constexpr auto operator()(const type_list<V>& v) {
        return _decide(
            A{}.differentiate_wrt(v),
            B{}.differentiate_wrt(v),
            [] (auto&& da_dv) { return da_dv*B{}; },
            [] (auto&& db_dv) { return A{}*db_dv; },
            [] (auto&& da_dv, auto&& db_dv) { return da_dv*B{} + A{}*db_dv; }
        );
    }
};

template<typename A, typename B>
struct derivative<op::divide, A, B> {
    template<typename V>
    constexpr auto operator()(const type_list<V>& v) {
        return _decide(
            A{}.differentiate_wrt(v),
            B{}.differentiate_wrt(v),
            [] (auto&& da_dv) { return da_dv/B{}; },
            [] (auto&& db_dv) { return cval<-1>*A{}*db_dv/(B{}*B{}); },
            [] (auto&& da_dv, auto&& db_dv) { return da_dv/B{} - A{}*db_dv/(B{}*B{}); }
        );
    }
};

template<typename A>
struct derivative<op::exp, A> {
    template<typename V>
    constexpr auto operator()(const type_list<V>& v) {
        const auto da_dv = A{}.differentiate_wrt(v);
        if constexpr (is_zero(da_dv))
            return cval<0>;
        else
            return exp(A{})*A{}.differentiate_wrt(v);
    }
};

template<typename A, typename B>
struct format<op::add, A, B> {
    template<typename... N>
    constexpr void operator()(std::ostream& out, const bindings<N...>& name_map) {
        A{}.export_to(out, name_map);
        out << " + ";
        B{}.export_to(out, name_map);
    }
};

template<typename A, typename B>
struct format<op::subtract, A, B> {
    template<typename... N>
    constexpr void operator()(std::ostream& out, const bindings<N...>& name_map) {
        A{}.export_to(out, name_map);
        out << " - ";
        B{}.export_to(out, name_map);
    }
};

#ifndef DOXYGEN
namespace detail {

    template<typename A, typename... N>
    inline constexpr void in_braces(std::ostream& out, const A& a, const bindings<N...>& name_map) {
        constexpr bool use_braces = !is_symbol_v<A>;
        if constexpr (use_braces) out << "(";
        a.export_to(out, name_map);
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
        A{}.export_to(out, name_map);
        out << ")";
    }
};

template<typename E, typename... N> // requires(is_expression_v<E>)
inline constexpr void print_to(std::ostream& s, const E& expression, const bindings<N...>& name_map) {
    expression.export_to(s, name_map);
}

}  // namespace adpp::backward


// TODO: register (e.g.) std::exp for terms?
