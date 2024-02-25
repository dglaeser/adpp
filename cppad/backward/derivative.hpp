#pragma once

#include <tuple>
#include <type_traits>

#include <cppad/common.hpp>

namespace cppad::backward {

#ifndef DOXYGEN
namespace detail {

template<concepts::arithmetic R, std::size_t I, typename T>
struct derivative_element {
    using index = index_constant<I>;

    constexpr derivative_element(const T& t) : _t{t} {}

 protected:
    constexpr auto& get(const index&) noexcept { return _value; }
    constexpr const auto& get(const index&) const noexcept { return _value; }
    constexpr auto get_index_of(const T&) const noexcept { return index{}; }
    constexpr const T& get_ref(const index& = {}) const noexcept { return _t; }

 private:
    const T& _t;
    R _value = R{0};
};

template<typename... Ts>
struct derivative_accessor;

template<concepts::arithmetic R, std::size_t... I, typename... Ts>
struct derivative_accessor<R, std::index_sequence<I...>, Ts...> : derivative_element<R, I, Ts>... {
    constexpr derivative_accessor(const Ts&... ts)
    : derivative_element<R, I, Ts>(ts)...
    {}

 protected:
    using derivative_element<R, I, Ts>::get_index_of...;
    using derivative_element<R, I, Ts>::get...;
};

}  // namespace detail
#endif  // DOXYGEN

template<concepts::arithmetic R, typename... Ts>
    requires(are_unique_v<Ts...>)
struct derivatives : detail::derivative_accessor<R, std::make_index_sequence<sizeof...(Ts)>, Ts...> {
 private:
    using base = detail::derivative_accessor<R, std::make_index_sequence<sizeof...(Ts)>, Ts...>;

 public:
    constexpr derivatives(R, const Ts&... ts) noexcept
    : base(ts...)
    {}

    template<typename T> requires(contains_decay_v<T, Ts...>)
    constexpr R operator[](const T& t) const noexcept {
        return this->get(this->get_index_of(t));
    }

    template<typename T> requires(contains_decay_v<T, Ts...>)
    constexpr void add_to_derivative_wrt(const T& t, concepts::arithmetic auto value) noexcept {
        if constexpr (contains_decay_v<T, Ts...>)
            this->get(this->get_index_of(t)) += value;
    }
};

template<typename R, typename... Ts>
    requires(std::conjunction_v<std::is_lvalue_reference<Ts>...>)
derivatives(R&&, Ts&&...) -> derivatives<R, std::remove_cvref_t<Ts>...>;

template<concepts::expression E, typename... V>
constexpr auto derivatives_of(E&& expression, const std::tuple<V...>& vars) {
    using R = expression_value_t<E>;
    return std::apply([&] <typename... Vs> (Vs&&... vs) {
        derivatives derivs{R{}, std::forward<Vs>(vs)...};
        expression.accumulate_derivatives(R{1}, derivs);
        return derivs;
    }, vars);
}

template<concepts::expression E, typename... V> requires(sizeof...(V) == 1)
constexpr auto derivative_of(E&& expression, const std::tuple<V...>& vars) {
    return derivatives_of(std::forward<E>(expression), vars)[std::get<0>(vars)];
}

template<typename... V>
    requires(std::conjunction_v<std::is_lvalue_reference<V>...>)
constexpr auto wrt(V&&... vars) {
    return std::forward_as_tuple(vars...);
}

}  // namespace cppad::backward
