#pragma once

#include <type_traits>
#include <concepts>
#include <utility>

#include <cppad/concepts.hpp>
#include <cppad/expression.hpp>

#ifndef DOXYGEN
namespace cppad::detail {

template<typename T>
class Storage {
 public:
    template<typename _T>
        requires(std::convertible_to<_T, T>)
    constexpr explicit Storage(_T&& value)
    : _value{std::forward<_T>(value)}
    {}

    template<typename Self> requires(!std::is_lvalue_reference_v<Self>)
    constexpr T&& get(this Self&& self) {
        return std::move(self._value);
    }

    template<typename Self>
    constexpr auto& get(this Self& self) {
        if constexpr (std::is_const_v<Self>)
            return std::as_const(self._value);
        else
            return self._value;
    }

 private:
    T _value;
};

template<typename T>
Storage(T&&) -> Storage<T>;


}  // namespace cppad::detail
#endif  // DOXYGEN
