#pragma once

#include <utility>
#include <type_traits>

#include <adpp/backward/concepts.hpp>
#include <adpp/backward/symbols.hpp>

namespace adpp::backward {

#ifndef DOXYGEN
namespace detail {

    template<typename T>
    struct is_tuple : public std::false_type {};
    template<typename... Args>
    struct is_tuple<std::tuple<Args...>> : public std::true_type {};
    template<typename T>
    concept tuple_like = is_tuple<std::remove_cvref_t<T>>::value;

    template<typename T>
    concept traversable_expression = is_symbol_v<std::remove_cvref_t<T>> or (
        is_complete_v<traits::sub_expressions<std::remove_cvref_t<T>>> and requires(const T& t) {
            typename traits::sub_expressions<std::remove_cvref_t<T>>::operands;
        }
    );

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
        using type = typename symbols_impl<
            typename traits::sub_expressions<std::remove_cvref_t<E>>::operands,
            type_list<Ts...>
        >::type;
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

}  // namespace adpp
