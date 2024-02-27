#pragma once

#include <type_traits>

#include <cppad/concepts.hpp>
#include <cppad/variadic_accessor.hpp>
#include <cppad/backward/concepts.hpp>
#include <cppad/backward/bindings.hpp>

namespace cppad::backward {

template<typename T, typename... B>
concept expression = requires(const T& t, B&&... b) {
    { t.evaluate_at(bindings{std::forward<B>(b)...}) };
};

template<typename E, typename... B>
    requires(expression<E, B...>)
inline constexpr auto evaluate(E&& e, const bindings<B...>& b) {
    return e.evaluate_at(b);
}

}  // namespace cppad
