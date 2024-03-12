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


// traits forward declarations
template<typename R, typename op, typename... T> struct back_propagator;
template<typename op, typename... T> struct formatter;
template<typename op, typename... T> struct differentiator;

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
        auto [value, derivs] = back_propagator<R, op, Ts...>{}(bindings, vars);
        if constexpr (contains_decayed_v<Self, V...>)
            derivs[self] = 1.0;
        return std::make_pair(std::move(value), std::move(derivs));
    }

    template<typename V>
    constexpr auto differentiate_wrt(const type_list<V>& var) const {
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

}  // namespace adpp::backward

// TODO: register (e.g.) std::exp for terms?
