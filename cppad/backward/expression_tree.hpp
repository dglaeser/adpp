#pragma once

#include <utility>
#include <type_traits>

#include <cppad/backward/concepts.hpp>

namespace cppad::backward {

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
        is_complete_v<traits::sub_expressions<T>> and requires(const T& t) {
            { traits::sub_expressions<T>::get(t) } -> detail::tuple_like;
        }
    );

    template<typename Filter, typename F, traversable_expression E, typename... L>
        requires(std::conjunction_v<traits::is_leaf_expression<L>...>)
    inline constexpr auto visit_expression(const F& sub_expr_callback, std::tuple<const L&...>&& leaves, const E& e) {
        if constexpr (is_leaf_expression_v<E>) {
            if constexpr (!traits::is_symbol<E>::value || contains_decay_v<E, L...> || !Filter::template include<E>)
                return std::move(leaves);
            else
                return std::tuple_cat(std::tuple<const E&>{e}, std::move(leaves));
        } else {
            return std::apply([&] <typename... SE> (SE&&... sub_expr) {
                return sub_expr_callback(std::move(leaves), std::forward<SE>(sub_expr)...);
            }, traits::sub_expressions<E>::get(e));
        }
    }

    template<typename Filter, typename F, traversable_expression E0, traversable_expression... E, typename... L>
        requires(std::conjunction_v<traits::is_leaf_expression<L>...>)
    inline constexpr auto concatenate_leaf_expressions_impl(const F& sub_expr_callback, std::tuple<const L&...>&& leaves, const E0& e0, const E&... e) {
        auto first_processed = visit_expression<Filter>(sub_expr_callback, std::move(leaves), e0);
        if constexpr (sizeof...(E) == 0)
            return std::move(first_processed);
        else
            return concatenate_leaf_expressions_impl<Filter>(sub_expr_callback, std::move(first_processed), e...);
    }

    template<typename Filter, typename... L, traversable_expression... E>
        requires(std::conjunction_v<traits::is_leaf_expression<L>...>)
    inline constexpr auto concatenate_leaf_expressions(std::tuple<const L&...>&& leaves, const E&... e) {
        if constexpr (sizeof...(E) == 0) {
            return std::move(leaves);
        } else if constexpr (sizeof...(E) == 1) {
            return visit_expression<Filter>([] <typename... SE> (auto&& result, SE&&... sub_expr) {
                return concatenate_leaf_expressions<Filter>(std::move(result), std::forward<SE>(sub_expr)...);
            }, std::move(leaves), e...);
        } else {
            return concatenate_leaf_expressions_impl<Filter>([] <typename... SE> (auto&& result, SE&&... sub_expr) {
                return concatenate_leaf_expressions<Filter>(std::move(result), std::forward<SE>(sub_expr)...);
            }, std::move(leaves), e...);
        }
    }

    struct null_filter {
        template<typename T>
        static constexpr bool include = true;
    };

    struct var_filter {
        template<typename T>
        static constexpr bool include = traits::is_var<std::remove_cvref_t<T>>::value;
    };

}  // namespace detail
#endif  // DOXYGEN

template<detail::traversable_expression E>
inline constexpr auto leaf_symbols_of(const E& e) {
    return detail::concatenate_leaf_expressions<detail::null_filter>(std::tuple{}, e);
}

template<detail::traversable_expression E>
inline constexpr auto leaf_variables_of(const E& e) {
    return detail::concatenate_leaf_expressions<detail::var_filter>(std::tuple{}, e);
}

}  // namespace cppad
