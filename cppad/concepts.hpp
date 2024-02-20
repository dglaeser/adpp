#pragma once

#include <utility>
#include <concepts>
#include <type_traits>

#include <cppad/type_traits.hpp>
#include <cppad/traits.hpp>

namespace cppad {
namespace concepts {

template<typename T>
concept Arithmetic = std::floating_point<std::remove_cvref_t<T>> or std::integral<std::remove_cvref_t<T>>;

template<typename T>
concept Expression = requires(const T& t) {
    { t.value() } -> Arithmetic;
    { t.partial(EmptyExpression{}) } -> Arithmetic;
};

template<typename T>
concept IntoExpression = Expression<T> or requires(const T& t) {
    requires is_complete<traits::AsExpression<std::remove_cvref_t<T>>>;
    { traits::AsExpression<std::remove_cvref_t<T>>::get(t) } -> Expression;
};

}  // namespace concepts

template<concepts::IntoExpression E>
constexpr decltype(auto) as_expression(E&& e) noexcept {
    return traits::AsExpression<std::remove_cvref_t<E>>::get(std::forward<E>(e));
}

namespace traits {

template<concepts::Expression E>
struct AsExpression<E> {
    template<typename _E> requires(std::same_as<E, std::remove_cvref_t<_E>>)
    static constexpr decltype(auto) get(_E&& e) noexcept {
        return std::forward<_E>(e);
    }
};

}  // namespace traits
}  // namespace cppad
