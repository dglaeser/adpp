#pragma once

#include <type_traits>
#include <concepts>
#include <utility>
#include <memory>

#include <cppad/concepts.hpp>

#ifndef DOXYGEN
namespace cppad::detail {

    template<typename T>
    class Storage {
        using Stored = std::conditional_t<std::is_lvalue_reference_v<T>, T, std::remove_cvref_t<T>>;

    public:
        template<typename _T>
            requires(std::convertible_to<_T, Stored>)
        constexpr explicit Storage(_T&& value) noexcept
        : _value{std::forward<_T>(value)}
        {}

        template<typename Self> requires(!std::is_lvalue_reference_v<Self>)
        constexpr T&& get(this Self&& self) noexcept {
            return std::move(self._value);
        }

        template<typename Self>
        constexpr auto& get(this Self& self) noexcept {
            if constexpr (std::is_const_v<Self>)
                return std::as_const(self._value);
            else
                return self._value;
        }

    private:
        Stored _value;
    };

    template<typename T>
    Storage(T&&) -> Storage<T>;


    template<typename A, typename B>
    constexpr bool is_same_object(A&& a, B&& b) {
        if constexpr (std::same_as<std::remove_cvref_t<A>, std::remove_cvref_t<B>>)
            return std::addressof(a) == std::addressof(b);
        return false;
    }

}  // namespace cppad::detail
#endif  // DOXYGEN
