#pragma once

#include <concepts>

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
concept Arithmetic = std::floating_point<T> or std::integral<T>;

template<typename T>
concept Expression = requires(const T& t) {
    { t.value() } -> Arithmetic;
    { t.partial(detail::MockExpression{}) } -> Arithmetic;
};

}  // namespace cppad::concepts
