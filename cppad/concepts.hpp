#pragma once

#include <concepts>

#include <cppad/type_traits.hpp>
#include <cppad/traits.hpp>

namespace cppad::concepts {

#ifndef DOXYGEN
namespace detail {

    struct MockExpression {
        double value() const { return 1.0; }

        template<typename T>
        double partial(T&&) const { return 1.0; }
    };

}  // namespace detail
#endif  // DOXYGEN

template<typename T>
concept Arithmetic = std::floating_point<std::remove_cvref_t<T>> or std::integral<std::remove_cvref_t<T>>;

template<typename T>
concept Expression = requires(const T& t) {
    { t.value() } -> Arithmetic;
    { t.partial(detail::MockExpression{}) } -> Arithmetic;
};

template<typename T>
concept IntoExpression = Expression<T> or requires(const T& t) {
    requires is_complete<traits::AsExpression<std::remove_cvref_t<T>>>;
    { traits::AsExpression<std::remove_cvref_t<T>>::get(t) } -> Expression;
};

}  // namespace cppad::concepts
