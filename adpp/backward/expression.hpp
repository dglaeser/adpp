#pragma once

#include <cmath>
#include <type_traits>

#include <adpp/backward/concepts.hpp>
#include <adpp/backward/bindings.hpp>

namespace adpp::backward {

struct exp {
    template<typename T>
    constexpr auto operator()(const T& t) const {
        using std::exp;
        return exp(t);
    }
};

template<typename op, term... Ts>
struct expression {
    template<typename... B>
    constexpr decltype(auto) operator()(const bindings<B...> operands) const {
        return op{}(Ts{}(operands)...);
    }
};

template<typename op, term... Ts>
struct is_expression<expression<op, Ts...>> : std::true_type {};


template<typename op, term... Ts>
using op_result_t = expression<op, std::remove_cvref_t<Ts>...>;

template<term A, term B>
inline constexpr op_result_t<std::plus<void>, A, B> operator+(A&&, B&&) { return {}; }
template<term A, term B>
inline constexpr op_result_t<std::minus<void>, A, B> operator-(A&&, B&&) { return {}; }
template<term A, term B>
inline constexpr op_result_t<std::multiplies<void>, A, B> operator*(A&&, B&&) { return {}; }
template<term A, term B>
inline constexpr op_result_t<std::divides<void>, A, B> operator/(A&&, B&&) { return {}; }
template<term A>
inline constexpr op_result_t<exp, A> exp(A&&) { return {}; }

}  // namespace adpp::backward
