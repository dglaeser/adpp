#pragma once

#include <type_traits>

#include <cppad/common.hpp>


namespace cppad {

#ifndef DOXYGEN
namespace detail {

template<std::size_t I, typename T>
struct variadic_element {
    using index = index_constant<I>;

    constexpr variadic_element(const T& t) noexcept : _t{t} {}
    constexpr const T& get(const index&) const noexcept { return _t; }
    constexpr index index_of(const T&) const noexcept { return {}; }

 private:
    const T& _t;
};

template<typename... Ts>
struct variadic_accessor;

template<std::size_t... I, typename... Ts>
struct variadic_accessor<std::index_sequence<I...>, Ts...> : variadic_element<I, Ts>... {
    constexpr variadic_accessor(Ts... ts)
    : variadic_element<I, Ts>(std::forward<Ts>(ts))...
    {}

    using variadic_element<I, Ts>::index_of...;
    using variadic_element<I, Ts>::get...;
};

}  // namespace detail
#endif  // DOXYGEN

template<typename... Ts>
    requires(are_unique_v<Ts...>)
struct variadic_accessor : detail::variadic_accessor<std::make_index_sequence<sizeof...(Ts)>, Ts...> {
 private:
    using base = detail::variadic_accessor<std::make_index_sequence<sizeof...(Ts)>, Ts...>;

 public:
    constexpr variadic_accessor(Ts... ts) noexcept
    : base(std::forward<Ts>(ts)...)
    {}

    template<typename T> requires(contains_decay_v<T, Ts...>)
    constexpr auto index(const T& t) const noexcept {
        return this->get(this->index_of(t));
    }
};

template<typename... Ts>
variadic_accessor(Ts&&...) -> variadic_accessor<Ts...>;

}  // namespace cppad
