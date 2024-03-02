#pragma once

#include <ostream>

#include <adpp/backward/bindings.hpp>

namespace adpp::backward {

template<typename E, typename... V>
inline constexpr std::ostream& stream(std::ostream& out, E&& expression, const bindings<V...>& name_bindings) {
    expression.stream(out, name_bindings);
    return out;
}

}  // namespace adpp::backward
