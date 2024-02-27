#pragma once

#include <utility>
#include <type_traits>

#include <cppad/dtype.hpp>
#include <cppad/common.hpp>
#include <cppad/concepts.hpp>

namespace cppad::backward {

template<typename S, typename V>
struct value_binder {
    template<concepts::same_decay_t_as<V> _V>
    constexpr value_binder(const S& symbol, _V&& v) noexcept
    : _value{std::forward<_V>(v)}
    {}

    template<typename Self>
    constexpr decltype(auto) unwrap(this Self&& self) {
        if constexpr (!std::is_lvalue_reference_v<Self>)
            return std::move(self._value).get();
        else
            return self._value.get();
    }

 private:
    storage<V> _value;
};

template<typename S, typename V>
    requires(std::is_lvalue_reference_v<S>)
value_binder(S&&, V&&) -> value_binder<std::remove_cvref_t<S>, V>;

template<typename T>
struct symbol {
    constexpr symbol() = default;
    constexpr symbol(symbol&&) = default;
    constexpr symbol(const symbol&) = delete;

    template<typename Self, typename V>
        requires(concepts::accepts<T, V>)
    constexpr auto bind(this Self&& self, V&& value) noexcept {
        return value_binder(std::forward<Self>(self), std::forward<V>(value));
    }

    template<typename Self, typename V>
        requires(concepts::accepts<T, V>)
    constexpr auto operator=(this Self&& self, V&& value) noexcept {
        return self.bind(std::forward<V>(value));
    }
};

template<typename T = dtype::any, auto = [] () {}>
struct var : symbol<T> {
    using symbol<T>::operator=;

    // for better compiler error messages
    template<typename _T, auto __>
    constexpr var& operator=(const var<_T, __>&) = delete;
};

template<typename T = dtype::any, auto = [] () {}>
struct let : symbol<T> {
    using symbol<T>::operator=;

    // for better compiler error messages
    template<typename _T, auto __>
    constexpr let& operator=(const let<_T, __>&) = delete;
};

}  // namespace cppad::backward
