// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Backward
 * \brief Data structures to represent operations on symbols/expressions.
 */
#pragma once

#include <cmath>
#include <type_traits>

#include <adpp/utils.hpp>
#include <adpp/backward/symbols.hpp>
#include <adpp/backward/concepts.hpp>
#include <adpp/backward/bindings.hpp>
#include <adpp/backward/derivatives.hpp>
#include <adpp/backward/expression.hpp>

namespace adpp::backward {

//! \addtogroup Backward
//! \{

namespace op {

struct exp {
    template<typename T>
    constexpr auto operator()(const T& t) const {
        using std::exp;
        return exp(t);
    }
};

struct sqrt {
    template<typename T>
    constexpr auto operator()(const T& t) const {
        using std::sqrt;
        return sqrt(t);
    }
};

struct add : std::plus<void> {};
struct subtract : std::minus<void> {};
struct multiply : std::multiplies<void> {};
struct divide : std::divides<void> {};

}  // namespace op


#ifndef DOXYGEN
namespace detail {

    template<typename T>
    struct is_cval : std::false_type {};
    template<auto v>
    struct is_cval<constant<v>> : std::true_type {};
    template<typename T>
    struct is_scalar : std::bool_constant<scalar<T> or is_scalar_expression_v<T>> {};

    template<typename... Ts>
    inline constexpr bool all_cvals_v = std::conjunction_v<is_cval<std::remove_cvref_t<Ts>>...>;

    template<typename... Ts>
    inline constexpr bool all_scalars_v = std::conjunction_v<is_scalar<std::remove_cvref_t<Ts>>...>;

}  // namespace detail
#endif  // DOXYGEN

template<into_term A, into_term B> requires(detail::all_scalars_v<A, B> and !detail::all_cvals_v<A, B>)
inline constexpr auto operator+(A&& a, B&& b) {
    return expression{op::add{}, as_term(std::forward<A>(a)), as_term(std::forward<B>(b))};
}
template<into_term A, into_term B> requires(detail::all_scalars_v<A, B> and !detail::all_cvals_v<A, B>)
inline constexpr auto operator-(A&& a, B&& b) {
    return expression{op::subtract{}, as_term(std::forward<A>(a)), as_term(std::forward<B>(b))};
}
template<into_term A, into_term B> requires(detail::all_scalars_v<A, B> and !detail::all_cvals_v<A, B>)
inline constexpr auto operator*(A&& a, B&& b) {
    return expression{op::multiply{}, as_term(std::forward<A>(a)), as_term(std::forward<B>(b))};
}
template<into_term A, into_term B> requires(detail::all_scalars_v<A, B> and !detail::all_cvals_v<A, B>)
inline constexpr auto operator/(A&& a, B&& b) {
    return expression{op::divide{}, as_term(std::forward<A>(a)), as_term(std::forward<B>(b))};
}
template<into_term A>
inline constexpr auto exp(A&& a) {
    return expression{op::exp{}, as_term(std::forward<A>(a))};
}
template<into_term A>
inline constexpr auto sqrt(A&& a) {
    return expression{op::sqrt{}, as_term(std::forward<A>(a))};
}

template<typename R, typename A, typename B>
struct back_propagator<R, op::add, A, B> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_a, derivs_a] = A{}.template back_propagate<R>(b, vars);
        auto [value_b, derivs_b] = B{}.template back_propagate<R>(b, vars);
        return std::make_pair(value_a + value_b, std::move(derivs_a) + std::move(derivs_b));
    }
};

template<typename R, typename A, typename B>
struct back_propagator<R, op::subtract, A, B> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_a, derivs_a] = A{}.template back_propagate<R>(b, vars);
        auto [value_b, derivs_b] = B{}.template back_propagate<R>(b, vars);
        return std::make_pair(value_a - value_b, std::move(derivs_a) + std::move(derivs_b).scaled_with(-1));
    }
};

template<typename R, typename A, typename B>
struct back_propagator<R, op::multiply, A, B> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_a, derivs_a] = A{}.template back_propagate<R>(b, vars);
        auto [value_b, derivs_b] = B{}.template back_propagate<R>(b, vars);
        auto derivs = std::move(derivs_a).scaled_with(value_b) + std::move(derivs_b).scaled_with(value_a);
        return std::make_pair(value_a*value_b, std::move(derivs));
    }
};

template<typename R, typename A, typename B>
struct back_propagator<R, op::divide, A, B> {
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
struct back_propagator<R, op::exp, A> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_inner, derivs_inner] = A{}.template back_propagate<R>(b, vars);
        auto result = op::exp{}(value_inner);
        return std::make_pair(std::move(result), std::move(derivs_inner).scaled_with(result));
    }
};

template<typename R, typename A>
struct back_propagator<R, op::sqrt, A> {
    template<typename... _B, typename... V>
    constexpr auto operator()(const bindings<_B...>& b, const type_list<V...>& vars) {
        auto [value_inner, derivs_inner] = A{}.template back_propagate<R>(b, vars);
        auto result = op::sqrt{}(value_inner);
        return std::make_pair(std::move(result), std::move(derivs_inner).scaled_with(-1.0/result));
    }
};



#ifndef DOXYGEN
namespace detail {

    template<typename T0, typename T1, typename F0, typename F1, typename F>
    constexpr auto simplified(T0&& t0, T1&& t1, const F0& f0, const F1& f1, const F& f) {
        if constexpr (is_zero_constant_v<std::remove_cvref_t<T0>>)
            return f1(std::forward<T1>(t1));
        else if constexpr (is_zero_constant_v<std::remove_cvref_t<T1>>)
            return f0(std::forward<T0>(t0));
        else
            return f(std::forward<T0>(t0), std::forward<T1>(t1));
    }

    template<typename T0, typename T1>
    constexpr auto simplify_mul(T0 t0, T1 t1) {
        return simplified(
            std::forward<T0>(t0), std::forward<T1>(t1),
            [] (auto&&) { return cval<0>; },
            [] (auto&&) { return cval<0>; },
            [] (auto&& a, auto&& b) { return a*b; }
        );
    }

    template<typename T0, typename T1>
    constexpr auto simplify_plus(T0 t0, T1 t1) {
        return simplified(
            std::forward<T0>(t0), std::forward<T1>(t1),
            [] (auto&& a) { return a; },
            [] (auto&& b) { return b; },
            [] (auto&& a, auto&& b) { return a + b; }
        );
    }

    template<typename T0, typename T1>
    constexpr auto simplify_division(T0 t0, T1 t1) {
        return simplified(
            std::forward<T0>(t0), std::forward<T1>(t1),
            [] (auto&& a) { static_assert(always_false<T0>::value, "division by zero!"); return a; },
            [] (auto&&) { return cval<0>; },
            [] (auto&& a, auto&& b) { return a/b; }
        );
    }

}  // namespace detail
#endif  // DOXYGEN

template<typename A, typename B>
struct differentiator<op::add, A, B> {
    template<typename V>
    constexpr auto operator()(const type_list<V>& v) {
        return detail::simplified(
            A{}.differentiate(v),
            B{}.differentiate(v),
            [] (auto&& da_dv) { return da_dv; },
            [] (auto&& db_dv) { return db_dv; },
            [] (auto&& da_dv, auto&& db_dv) {
                return detail::simplify_plus(da_dv, db_dv);
            }
        );
    }
};

template<typename A, typename B>
struct differentiator<op::subtract, A, B> {
    template<typename V>
    constexpr auto operator()(const type_list<V>& v) {
        return detail::simplified(
            A{}.differentiate(v),
            B{}.differentiate(v),
            [] (auto&& da_dv) { return da_dv; },
            [] (auto&& db_dv) { return detail::simplify_mul(cval<-1>, db_dv); },
            [] (auto&& da_dv, auto&& db_dv) {
                return detail::simplify_plus(da_dv, detail::simplify_mul(cval<-1>, db_dv));
            }
        );
    }
};

template<typename A, typename B>
struct differentiator<op::multiply, A, B> {
    template<typename V>
    constexpr auto operator()(const type_list<V>& v) {
        return detail::simplified(
            A{}.differentiate(v),
            B{}.differentiate(v),
            [] (auto&& da_dv) { return detail::simplify_mul(std::move(da_dv), B{}); },
            [] (auto&& db_dv) { return detail::simplify_mul(A{}, std::move(db_dv)); },
            [] (auto&& da_dv, auto&& db_dv) {
                return detail::simplify_mul(std::move(da_dv), B{}) + detail::simplify_mul(A{}, std::move(db_dv));
            }
        );
    }
};

template<typename A, typename B>
struct differentiator<op::divide, A, B> {
    template<typename V>
    constexpr auto operator()(const type_list<V>& v) {
        return detail::simplified(
            A{}.differentiate(v),
            B{}.differentiate(v),
            [] (auto&& da_dv) { return detail::simplify_division(da_dv, B{}); },
            [] (auto&& db_dv) {
                return detail::simplify_division(
                    detail::simplify_mul(
                        detail::simplify_mul(cval<-1>, A{}),
                        db_dv
                    ),
                    detail::simplify_mul(B{}, B{})
                );
            },
            [] (auto&& da_dv, auto&& db_dv) {
                return detail::simplify_plus(
                    detail::simplify_division(da_dv, B{}),
                    detail::simplify_division(
                        detail::simplify_mul(
                            detail::simplify_mul(cval<-1>, A{}),
                            db_dv
                        ),
                        detail::simplify_mul(B{}, B{})
                    )
                );
            }
        );
    }
};

template<typename A>
struct differentiator<op::exp, A> {
    template<typename V>
    constexpr auto operator()(const type_list<V>& v) {
        return detail::simplify_mul(exp(A{}), A{}.differentiate(v));
    }
};

template<typename A>
struct differentiator<op::sqrt, A> {
    template<typename V>
    constexpr auto operator()(const type_list<V>& v) {
        return detail::simplify_mul(cval<-1>/sqrt(A{}), A{}.differentiate(v));
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
struct formatter<op::add, A, B> {
    template<typename... N>
    constexpr void operator()(std::ostream& out, const bindings<N...>& name_map) {
        A{}.export_to(out, name_map);
        out << " + ";
        B{}.export_to(out, name_map);
    }
};

template<typename A, typename B>
struct formatter<op::subtract, A, B> {
    template<typename... N>
    constexpr void operator()(std::ostream& out, const bindings<N...>& name_map) {
        A{}.export_to(out, name_map);
        out << " - ";
        B{}.export_to(out, name_map);
    }
};

template<typename A, typename B>
struct formatter<op::divide, A, B> {
    template<typename... N>
    constexpr void operator()(std::ostream& out, const bindings<N...>& name_map) {
        detail::in_braces(out, A{}, name_map);
        out << "/";
        detail::in_braces(out, B{}, name_map);
    }
};

template<typename A, typename B>
struct formatter<op::multiply, A, B> {
    template<typename... N>
    constexpr void operator()(std::ostream& out, const bindings<N...>& name_map) {
        detail::in_braces(out, A{}, name_map);
        out << "*";
        detail::in_braces(out, B{}, name_map);
    }
};

template<typename A>
struct formatter<op::exp, A> {
    template<typename... N>
    constexpr void operator()(std::ostream& out, const bindings<N...>& name_map) {
        out << "exp(";
        A{}.export_to(out, name_map);
        out << ")";
    }
};

template<typename A>
struct formatter<op::sqrt, A> {
    template<typename... N>
    constexpr void operator()(std::ostream& out, const bindings<N...>& name_map) {
        out << "√(";
        A{}.export_to(out, name_map);
        out << ")";
    }
};

//! \} group Backward

}  // namespace adpp::backward


// TODO: register (e.g.) std::exp for terms?
