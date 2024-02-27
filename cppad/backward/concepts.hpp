#pragma once

#include <cppad/concepts.hpp>

namespace cppad::backward {
namespace traits {

template<typename T> struct into_operand;

}  // namespace backward::traits

template<typename T>
concept into_operand = is_complete_v<traits::into_operand<std::remove_cvref_t<T>>> and requires(const T& t) {
    { traits::into_operand<std::remove_cvref_t<T>>::get(t) };
};

template<into_operand T>
inline constexpr decltype(auto) as_operand(T&& t) noexcept {
    return traits::into_operand<std::remove_cvref_t<T>>::get(std::forward<T>(t));
}

}  // namespace cppad
