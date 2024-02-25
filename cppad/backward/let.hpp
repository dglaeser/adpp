#pragma once

#include <limits>
#include <type_traits>

#include <cppad/common.hpp>
#include <cppad/backward/val.hpp>
#include <cppad/backward/expression.hpp>
#include <cppad/backward/derivatives.hpp>


namespace cppad::backward {

template<concepts::arithmetic T>
class let : public val<T> {
    using Parent = val<T>;

 public:
    using Parent::Parent;

    template<concepts::expression... E>
    constexpr auto back_propagate(const E&... e) const {
        return std::make_pair(this->value(), derivatives{double{}, e...});
    }

    constexpr auto partial_expression(concepts::expression auto&&) const {
        return let<T>{0};
    }
};

template<concepts::arithmetic T>
let(T&&) -> let<std::remove_cvref_t<T>>;

}  // namespace cppad::backward


namespace cppad {

template<concepts::arithmetic T>
struct is_constant<cppad::backward::let<T>> : public std::true_type {};

template<concepts::arithmetic T>
struct as_expression<T> {
    static constexpr auto get(T value) {
        return cppad::backward::let<T>{value};
    }
};

}  // namespace cppad
