#pragma once

#include <tuple>
#include <type_traits>

#include <cppad/common.hpp>

namespace cppad::backward {

#ifndef DOXYGEN
namespace detail {

template<std::size_t I, typename T>
struct derivative_element {
    using index = index_constant<I>;
    using value_type = expression_value_t<T>;

    constexpr derivative_element(const T& t) : _t{t} {}

 protected:
    constexpr auto& get(const index&) noexcept { return _value; }
    constexpr const auto& get(const index&) const noexcept { return _value; }
    constexpr auto get_index_of(const T&) const noexcept { return index{}; }

 private:
    const T& _t;
    value_type _value = value_type{0};
};

template<typename... Ts>
struct derivative_accessor;

template<std::size_t... I, typename... Ts>
struct derivative_accessor<std::index_sequence<I...>, Ts...> : derivative_element<I, Ts>... {
    constexpr derivative_accessor(const Ts&... ts)
    : derivative_element<I, Ts>(ts)...
    {}

 protected:
    using derivative_element<I, Ts>::get_index_of...;
    using derivative_element<I, Ts>::get...;
};

}  // namespace detail
#endif  // DOXYGEN

template<typename... Ts>
    requires(std::conjunction_v<traits::is_variable<Ts>...> and are_unique_v<Ts...>)
struct derivatives : detail::derivative_accessor<std::make_index_sequence<sizeof...(Ts)>, Ts...> {
 private:
    using base = detail::derivative_accessor<std::make_index_sequence<sizeof...(Ts)>, Ts...>;
 public:
    using base::base;

    template<typename T> requires(is_variable_v<T>)
    constexpr auto operator[](const T& t) const {
        return this->get(this->get_index_of(t));
    }

    template<typename T> requires(is_variable_v<T>)
    constexpr void add_to_derivative_wrt(const T& t, concepts::arithmetic auto value) {
        this->get(this->get_index_of(t)) += value;
    }
};

template<typename... Ts>
    requires(std::conjunction_v<std::is_lvalue_reference<Ts>...>)
derivatives(Ts&&...) -> derivatives<std::remove_cvref_t<Ts>...>;


template<concepts::expression E, typename... V>
constexpr derivatives<V...> derivatives_of(E&& expression, derivatives<V...>&& derivatives) {
    return std::move(derivatives);
}

template<typename... V>
    requires(
        std::conjunction_v<std::is_lvalue_reference<V>...> and
        std::conjunction_v<traits::is_variable<std::remove_cvref_t<V>>...>
    )
constexpr auto wrt(V&&... vars) {
    return derivatives(std::forward<V>(vars)...);
}

}  // namespace cppad::backward
