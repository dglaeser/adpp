#pragma once

#include <type_traits>

#include <adpp/backward/concepts.hpp>
#include <adpp/backward/bindings.hpp>

namespace adpp::backward {

template<typename op, term... Ts>
struct expression {
    template<typename... B>
    constexpr auto operator()(const bindings<B...> operands) {
        return op{}(Ts{}(operands)...);
    }
};

template<typename op, term... Ts>
struct is_expression<expression<op, Ts...>> : std::true_type {};

}  // namespace adpp::backward
