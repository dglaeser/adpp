#pragma once

#include <limits>
#include <type_traits>

#include <cppad/common.hpp>
#include <cppad/backward/expression.hpp>


namespace cppad::backward {

template<concepts::arithmetic T>
class val : public expression_base {
 public:
    constexpr val(T value)  : _value{value} {}

    constexpr T value() const { return _value; }
    constexpr operator T() const { return _value; }

    constexpr auto& operator=(this auto& self, std::same_as<T> auto value) {
        self._value = value;
        return self;
    }

    constexpr val& operator-=(std::same_as<T> auto value) {
        _value -= value;
        return *this;
    }

    constexpr val& operator+=(std::same_as<T> auto value) {
        _value += value;
        return *this;
    }

    constexpr val& operator*=(std::same_as<T> auto value) {
        _value *= value;
        return *this;
    }

    constexpr val& operator/=(std::same_as<T> auto value) {
        _value /= value;
        return *this;
    }

 private:
    T _value;
};

template<concepts::arithmetic T>
val(T&&) -> val<std::remove_cvref_t<T>>;

template<concepts::arithmetic T>
class let : public val<T> {
    using Parent = val<T>;

 public:
    using Parent::Parent;

    constexpr T partial(concepts::expression auto&&) const {
        return T{0};
    }
};

template<concepts::arithmetic T>
let(T&&) -> let<std::remove_cvref_t<T>>;

}  // namespace cppad::backward


namespace cppad::traits {

template<concepts::arithmetic T>
struct is_constant<cppad::backward::let<T>> : public std::true_type {};

template<concepts::arithmetic T>
struct is_constant<cppad::backward::val<T>> : public std::true_type {};

template<concepts::arithmetic T>
struct as_expression<T> {
    static constexpr auto get(T value) {
        return cppad::backward::let<T>{value};
    }
};

}  // namespace cppad::traits
