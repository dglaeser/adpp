#pragma once

#include <type_traits>
#include <concepts>
#include <utility>
#include <memory>

namespace cppad {

template<typename T>
class storage {
    using stored = std::conditional_t<std::is_lvalue_reference_v<T>, T, std::remove_cvref_t<T>>;

public:
    template<typename _T> requires(std::convertible_to<_T, stored>)
    constexpr explicit storage(_T&& value) noexcept
    : _value{std::forward<_T>(value)}
    {}

    template<typename S> requires(!std::is_lvalue_reference_v<S>)
    constexpr T&& get(this S&& self) noexcept {
        return std::move(self._value);
    }

    template<typename S>
    constexpr auto& get(this S& self) noexcept {
        if constexpr (std::is_const_v<S>)
            return std::as_const(self._value);
        else
            return self._value;
    }

private:
    stored _value;
};

template<typename T>
storage(T&&) -> storage<T>;


template<typename A, typename B>
constexpr bool is_same_object(A&& a, B&& b) {
    if constexpr (std::same_as<std::remove_cvref_t<A>, std::remove_cvref_t<B>>)
        return std::addressof(a) == std::addressof(b);
    return false;
}

}  // namespace cppad
