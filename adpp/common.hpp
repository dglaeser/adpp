#pragma once

#include <type_traits>
#include <concepts>
#include <utility>
#include <memory>

#include <adpp/type_traits.hpp>
#include <adpp/concepts.hpp>

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


template<typename A, typename B>
constexpr bool is_same_object(A&& a, B&& b) {
    if constexpr (std::same_as<std::remove_cvref_t<A>, std::remove_cvref_t<B>>)
        return std::addressof(a) == std::addressof(b);
    return false;
}


#ifndef DOXYGEN
namespace detail {

template<std::size_t I, typename T>
struct indexed_element {
    using index = index_constant<I>;

    template<concepts::same_decay_t_as<T> _T>
    constexpr index index_of() const noexcept { return {}; }
    constexpr index index_of(const T&) const noexcept { return {}; }
};

template<typename... Ts>
struct indexed;

template<std::size_t... I, typename... Ts>
struct indexed<std::index_sequence<I...>, Ts...> : indexed_element<I, Ts>... {
    using indexed_element<I, Ts>::index_of...;
};

}  // namespace detail
#endif  // DOXYGEN

template<typename... Ts> requires(are_unique_v<Ts...>)
struct indexed : detail::indexed<std::make_index_sequence<sizeof...(Ts)>, Ts...> {};

}  // namespace adpp
