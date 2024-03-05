#pragma once

#include <utility>
#include <type_traits>

#include <adpp/backward/concepts.hpp>

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
    concept traversable_expression = is_leaf_expression_v<T> or (
        is_complete_v<traits::sub_expressions<std::remove_cvref_t<T>>> and requires(const T& t) {
            typename traits::sub_expressions<std::remove_cvref_t<T>>::operands;
            { traits::sub_expressions<std::remove_cvref_t<T>>::get(t) } -> tuple_like;
        }
    );

    template<typename filter, typename... current>
    inline constexpr auto collect_leaves(const filter&, std::tuple<current...>&& collected) {
        return std::move(collected);
    }

    template<typename filter, typename... current, traversable_expression E0, typename... E>
    inline constexpr auto collect_leaves(const filter& f, std::tuple<current...>&& collected, const E0& e0, const E&... e) {
        if constexpr (is_leaf_expression_v<std::remove_cvref_t<E0>>) {
            if constexpr (traits::is_symbol<E0>::value && filter::template include<E0> && !contains_decay_v<E0, current...>)
                return collect_leaves(f, std::tuple_cat(std::tuple<const E0&>{e0}, std::move(collected)), e...);
            else
                return collect_leaves(f, std::move(collected), e...);
        } else {
            return collect_leaves(
                f,
                std::apply([&] <typename... S> (const S&... sub_exprs) {
                    return collect_leaves(f, std::move(collected), sub_exprs...);
                }, traits::sub_expressions<E0>::get(e0)),
                e...
            );
        }
    }

    template<typename...>
    struct leaf_symbols_impl;

    template<typename E, typename... Ts> requires(is_leaf_expression_v<E>)
    struct leaf_symbols_impl<E, type_list<Ts...>> {
        using type = std::conditional_t<
            traits::is_symbol<std::remove_cvref_t<E>>::value,
            typename unique_tuple<type_list<Ts...>, std::remove_cvref_t<E>>::type,
            type_list<Ts...>
        >;
    };

    template<typename E, typename... Ts> requires(!is_leaf_expression_v<E>)
    struct leaf_symbols_impl<E, type_list<Ts...>> {
        using type = typename leaf_symbols_impl<
            typename traits::sub_expressions<std::remove_cvref_t<E>>::operands,
            type_list<Ts...>
        >::type;
    };

    template<typename E0, typename... Es, typename... Ts>
    struct leaf_symbols_impl<type_list<E0, Es...>, type_list<Ts...>> {
        using type = typename unique_tuple<
            typename merged_tuple<
                typename leaf_symbols_impl<E0, type_list<Ts...>>::type,
                typename leaf_symbols_impl<Es..., type_list<Ts...>>::type
            >::type
        >::type;
    };

    template<typename... Ts>
    struct leaf_symbols_impl<type_list<Ts...>> : std::type_identity<type_list<Ts...>> {};

    struct var_filter {
        template<typename T>
        static constexpr bool include = traits::is_var<std::remove_cvref_t<T>>::value;
    };

    struct null_filter {
        template<typename T>
        static constexpr bool include = true;
    };

}  // namespace detail
#endif  // DOXYGEN

template<detail::traversable_expression E>
struct leaf_symbols : detail::leaf_symbols_impl<E, type_list<>> {};

template<detail::traversable_expression E>
using leaf_symbols_t = typename leaf_symbols<E>::type;

template<detail::traversable_expression E>
struct leaf_vars : filtered_tuple<typename decayed_arg<traits::is_var>::type, leaf_symbols_t<E>> {};

template<detail::traversable_expression E>
using leaf_vars_t = typename leaf_vars<E>::type;

template<detail::traversable_expression E>
inline constexpr auto leaf_symbols_of(const E& e) {
    return detail::collect_leaves(detail::null_filter{}, std::tuple<>{}, e);
}

template<detail::traversable_expression E>
inline constexpr auto leaf_variables_of(const E& e) {
    return detail::collect_leaves(detail::var_filter{}, std::tuple<>{}, e);
}

}  // namespace adpp
