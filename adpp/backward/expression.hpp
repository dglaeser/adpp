#pragma once

#include <cmath>
#include <utility>
#include <type_traits>

#include <adpp/type_traits.hpp>
#include <adpp/backward/concepts.hpp>
#include <adpp/backward/bindings.hpp>
#include <adpp/backward/derivatives.hpp>
#include <adpp/backward/symbols.hpp>

namespace adpp::backward {

#ifndef DOXYGEN
namespace detail {

    template<typename...>
    struct symbols_impl;

    template<typename E, typename... Ts> requires(is_symbol_v<std::remove_cvref_t<E>>)
    struct symbols_impl<E, type_list<Ts...>> {
        using type = std::conditional_t<
            symbolic<std::remove_cvref_t<E>>,
            unique_t<type_list<Ts...>, std::remove_cvref_t<E>>,
            unique_t<type_list<Ts...>>
        >;
    };

    template<typename E, typename... Ts> requires(!is_symbol_v<std::remove_cvref_t<E>>)
    struct symbols_impl<E, type_list<Ts...>> {
        using type = typename symbols_impl<operands_t<E>, type_list<Ts...>>::type;
    };

    template<typename E0, typename... Es, typename... Ts>
    struct symbols_impl<type_list<E0, Es...>, type_list<Ts...>> {
        using type = unique_t<merged_t<
            typename symbols_impl<E0, type_list<Ts...>>::type,
            typename symbols_impl<type_list<Es...>, type_list<Ts...>>::type
        >>;
    };

    // closures to stop recursion
    template<typename... Ts> struct symbols_impl<type_list<Ts...>> : std::type_identity<type_list<Ts...>> {};
    template<typename... Ts> struct symbols_impl<type_list<>, type_list<Ts...>> : std::type_identity<type_list<Ts...>> {};

    template<typename T> struct is_var : std::false_type {};
    template<typename T, auto _> struct is_var<var<T, _>> : std::true_type {};

}  // namespace detail
#endif  // DOXYGEN

template<typename E> requires(is_expression_v<std::remove_cvref_t<E>>)
struct symbols : detail::symbols_impl<E, type_list<>> {};

template<typename E> requires(is_expression_v<std::remove_cvref_t<E>>)
using symbols_t = typename symbols<E>::type;

template<typename E> requires(is_expression_v<std::remove_cvref_t<E>>)
inline constexpr auto symbols_of(const E&) {
    return symbols_t<E>{};
}


template<typename E> requires(is_expression_v<std::remove_cvref_t<E>>)
struct unbound_symbols : filtered_types<decayed_arg_trait<is_unbound_symbol>::type, symbols_t<E>> {};

template<typename E> requires(is_expression_v<std::remove_cvref_t<E>>)
using unbound_symbols_t = typename unbound_symbols<E>::type;

template<typename E> requires(is_expression_v<std::remove_cvref_t<E>>)
inline constexpr auto unbound_symbols_of(const E&) {
    return unbound_symbols_t<E>{};
}


template<typename E> requires(is_expression_v<std::remove_cvref_t<E>>)
struct vars : filtered_types<decayed_arg_trait<detail::is_var>::type, symbols_t<E>> {};

template<typename E> requires(is_expression_v<std::remove_cvref_t<E>>)
using vars_t = typename vars<E>::type;

template<typename E> requires(is_expression_v<std::remove_cvref_t<E>>)
inline constexpr auto variables_of(const E&) {
    return vars_t<E>{};
}


// traits forward declarations
template<typename R, typename op, typename... T> struct back_propagator;
template<typename op, typename... T> struct formatter;
template<typename op, typename... T> struct differentiator;

template<typename op, term... Ts>
struct expression : bindable, negatable {
    constexpr expression() = default;
    constexpr expression(const op&, const Ts&...) noexcept {}

    template<typename... B>
    constexpr decltype(auto) evaluate(const bindings<B...>& operands) const {
        return op{}(Ts{}.evaluate(operands)...);
    }

    template<scalar R, typename Self, typename... B, typename... V>
    constexpr auto back_propagate(this Self&& self, const bindings<B...>& bindings, const type_list<V...>& vars) {
        auto [value, derivs] = back_propagator<R, op, Ts...>{}(bindings, vars);
        if constexpr (contains_decayed_v<Self, V...>)
            derivs[self] = 1.0;
        return std::make_pair(std::move(value), std::move(derivs));
    }

    template<typename V>
    constexpr auto differentiate(const type_list<V>& var) const {
        return differentiator<op, Ts...>{}(var);
    }

    template<typename... B>
    constexpr void export_to(std::ostream& out, const bindings<B...>& name_bindings) const {
        formatter<op, Ts...>{}(out, name_bindings);
    }
};

template<typename op, typename... Ts>
expression(op&&, Ts&&...) -> expression<std::remove_cvref_t<op>, std::remove_cvref_t<Ts>...>;

template<typename op, term... Ts>
struct is_expression<expression<op, Ts...>> : std::true_type {};

template<typename op, typename... T>
struct operands<expression<op, T...>> : std::type_identity<type_list<T...>> {};

}  // namespace adpp::backward

// TODO: register (e.g.) std::exp for terms?
