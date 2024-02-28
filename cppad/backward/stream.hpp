#pragma once

#include <ostream>

#include <cppad/backward/bindings.hpp>

namespace cppad::backward {

template<typename E, typename... V>
inline constexpr std::ostream& stream(std::ostream& out, E&& expression, const bindings<V...>& name_bindings) {
    expression.stream(out, name_bindings);
    return out;
}

}  // namespace cppad::backward
