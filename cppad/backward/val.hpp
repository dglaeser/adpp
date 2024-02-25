#pragma once

#include <limits>
#include <type_traits>

#include <cppad/common.hpp>
#include <cppad/backward/expression.hpp>
#include <cppad/backward/derivative.hpp>


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

}  // namespace cppad::backward


namespace cppad::traits {

template<concepts::arithmetic T>
struct is_constant<cppad::backward::val<T>> : public std::true_type {};

}  // namespace cppad::traits
