#pragma once

#include <type_traits>

#include <cppad/concepts.hpp>

namespace cppad::backward {
namespace traits {

template<typename T> struct into_operand;
template<typename T> struct is_value_binder : public std::false_type {};

}  // namespace traits

template<typename T>
concept into_operand = is_complete_v<traits::into_operand<std::remove_cvref_t<T>>> and requires(const T& t) {
    { traits::into_operand<std::remove_cvref_t<T>>::get(t) };
};

template<typename T>
concept binder = requires(const T& t) {
    typename T::symbol_type;
    typename T::value_type;
    { t.unwrap() } -> std::convertible_to<typename T::value_type>;
};

template<into_operand T>
inline constexpr decltype(auto) as_operand(T&& t) noexcept {
    return traits::into_operand<std::remove_cvref_t<T>>::get(std::forward<T>(t));
}

namespace traits {

template<binder T> struct is_value_binder<T> : public std::true_type {};

}  // namespace traits
}  // namespace cppad