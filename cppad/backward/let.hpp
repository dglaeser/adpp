#pragma once

#include <limits>
#include <type_traits>

#include <cppad/concepts.hpp>
#include <cppad/backward/val.hpp>
#include <cppad/backward/expression.hpp>
#include <cppad/backward/derivatives.hpp>


namespace cppad::backward {

template<concepts::arithmetic T, auto = [] () {}>
class let : public val<T> {
    using base = val<T>;

 public:
    using base::base;
    constexpr let(let&&) = default;
    constexpr let(const let&) = delete;

    // for better error message when attempting to copy
    template<concepts::arithmetic V, auto _>
    constexpr let& operator=(const let<V, _>&) = delete;

    constexpr let& operator=(T value) {
        static_cast<base&>(*this) = value;
        return *this;
    }

    template<concepts::expression... E>
    constexpr auto back_propagate(const E&... e) const {
        return std::make_pair(this->value(), derivatives{double{}, e...});
    }

    template<auto _ = [] () {}>
    constexpr auto differentiate_wrt(auto&&) const {
        return let<T, _>{0};
    }
};

template<concepts::arithmetic T, auto _ = [] () {}>
let(T&&) -> let<std::remove_cvref_t<T>, _>;

}  // namespace cppad::backward


namespace cppad {

template<concepts::arithmetic T, auto _>
struct is_constant<cppad::backward::let<T, _>> : public std::true_type {};

template<concepts::arithmetic T>
struct as_expression<T> {
    template<auto _ = [] () {}>
    static constexpr auto get(T value) {
        return cppad::backward::let<T, _>{value};
    }
};

}  // namespace cppad
