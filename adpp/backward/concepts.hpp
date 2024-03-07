#pragma once

#include <type_traits>

#include <adpp/concepts.hpp>

namespace adpp::backward {
namespace traits {

template<typename T> struct into_operand;
template<typename T> struct sub_expressions;
template<typename T> struct differentiator;
template<typename T> struct formatter;
template<typename T> struct is_leaf_expression : public std::false_type {};
template<typename T> struct is_symbol : public std::false_type {};
template<typename T> struct is_var : public std::false_type {};
template<typename T> struct is_let : public std::false_type {};

}  // namespace traits

template<typename T>
concept into_operand = is_complete_v<traits::into_operand<std::remove_cvref_t<T>>> and requires(const T& t) {
    { traits::into_operand<std::remove_cvref_t<T>>::get(t) };
};

template<typename T>
concept binder = requires(const T& t) {
    typename T::symbol_type;
    { t.unwrap() };
};

template<into_operand T>
inline constexpr decltype(auto) as_operand(T&& t) noexcept {
    return traits::into_operand<std::remove_cvref_t<T>>::get(std::forward<T>(t));
}

template<typename T>
inline constexpr bool is_leaf_expression_v = traits::is_leaf_expression<std::remove_cvref_t<T>>::value;


#ifndef DOXYGEN
namespace detail {

    template<typename T>
    struct sub_expressions_size;
    template<typename T> requires(is_leaf_expression_v<T>)
    struct sub_expressions_size<T> : public std::integral_constant<std::size_t, 0> {};
    template<typename T> requires(!is_leaf_expression_v<T>)
    struct sub_expressions_size<T> {
        static constexpr std::size_t value = std::tuple_size_v<
            std::remove_cvref_t<decltype(traits::sub_expressions<T>::get(std::declval<const T&>()))>
        >;
    };

}  // namespace detail
#endif  // DOXYGEN

template<typename T>
inline constexpr std::size_t sub_expressions_size_v = detail::sub_expressions_size<std::remove_cvref_t<T>>::value;

}  // namespace adpp
