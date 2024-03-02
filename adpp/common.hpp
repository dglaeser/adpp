#pragma once

#include <type_traits>
#include <concepts>
#include <utility>
#include <memory>

#include <adpp/type_traits.hpp>

namespace adpp {

template<unsigned int i>
struct order : public std::integral_constant<unsigned int, i> {};

inline constexpr order<1> first_order;
inline constexpr order<2> second_order;
inline constexpr order<3> third_order;

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


#ifndef DOXYGEN
namespace detail {

     template<std::size_t cur, std::size_t max, typename F>
     inline constexpr void for_each_n_impl(F&& f) {
        if constexpr (cur < max) {
            f(index_constant<cur>{});
            for_each_n_impl<cur+1, max>(std::forward<F>(f));
        }
     }

}  // namespace detail
#endif  // DOXYGEN

template<std::size_t i, typename F>
inline constexpr void for_each_n(F&& function) {
    detail::for_each_n_impl<0, i>(std::forward<F>(function));
}

}  // namespace adpp
