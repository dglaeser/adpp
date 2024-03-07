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


#ifndef DOXYGEN
namespace detail {

    template<std::size_t I, typename T>
    struct variadic_element {
        using index = index_constant<I>;

        constexpr variadic_element(T t) noexcept : _storage{std::forward<T>(t)} {}

        template<concepts::same_decay_t_as<T> _T>
        constexpr index index_of() const noexcept { return {}; }
        constexpr index index_of(const T&) const noexcept { return {}; }

        constexpr const std::remove_cvref_t<T>& get(const index&) const noexcept {
            return _storage.get();
        }

    private:
        storage<T> _storage;
    };

    template<typename... Ts>
    struct variadic_accessor;

    template<std::size_t... I, typename... Ts>
    struct variadic_accessor<std::index_sequence<I...>, Ts...> : variadic_element<I, Ts>... {
        constexpr variadic_accessor(Ts... ts) noexcept : variadic_element<I, Ts>(std::forward<Ts>(ts))... {}
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

}  // namespace adpp
