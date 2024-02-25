#pragma once

#include <ostream>
#include <utility>

#include <cppad/variadic_accessor.hpp>
#include <cppad/backward/expression.hpp>

namespace cppad::backward {

template<concepts::expression E, typename... V>
inline constexpr std::ostream& stream(std::ostream& out, E&& expression, const expression_name_map<V...>& map) {
    expression.stream(out, map);
    return out;
}

template<typename... V>
inline constexpr auto with_names(V&&... v) noexcept {
    return expression_name_map{std::forward<V>(v)...};
}

}  // namespace cppad::backward
