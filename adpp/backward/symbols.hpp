// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Backward
 * \brief Data structures to represent symbols from which expressions can be constructed.
 */
#pragma once

#include <ostream>
#include <utility>
#include <type_traits>

#include <adpp/utils.hpp>
#include <adpp/dtype.hpp>
#include <adpp/concepts.hpp>

#include <adpp/backward/concepts.hpp>
#include <adpp/backward/bindings.hpp>
#include <adpp/backward/derivatives.hpp>

namespace adpp::backward {

//! \addtogroup Backward
//! \{

//! symbol that represents a constant value
template<auto v>
struct constant : bindable {
    static constexpr auto value = v;

    constexpr auto operator-() { return constant<-v>{}; }
    template<auto k> constexpr auto operator+(const constant<k>&) const noexcept { return constant<v+k>{}; }
    template<auto k> constexpr auto operator-(const constant<k>&) const noexcept { return constant<v-k>{}; }
    template<auto k> constexpr auto operator*(const constant<k>&) const noexcept { return constant<v*k>{}; }
    template<auto k> constexpr auto operator/(const constant<k>&) const noexcept { return constant<v/k>{}; }

    //! Return the constant value (signature compatible with the expression interface)
    template<typename... B>
    constexpr auto evaluate(const bindings<B...>&) const noexcept {
        return value;
    }

    //! Back-propagate derivatives w.r.t. the given variables (signature compatible with the expression interface)
    template<scalar R, typename... B, typename... V>
    constexpr auto back_propagate(const bindings<B...>&, const type_list<V...>&) const noexcept {
        return std::make_pair(value, derivatives<R, V...>{});
    }

    //! Differentiate this expression (symbol) w.r.t. the given symbol
    template<typename Self, typename V>
    constexpr auto differentiate(this Self&&, const type_list<V>&) noexcept {
        if constexpr (std::is_same_v<std::remove_cvref_t<Self>, std::remove_cvref_t<V>>)
            return constant<1>{};
        else
            return constant<0>{};
    }

    //! Export this symbol into the given stream
    template<typename... B>
    constexpr void export_to(std::ostream& out, const bindings<B...>&) const {
        out << value;
    }
};

template<auto v>
struct is_symbol<constant<v>> : std::true_type {};

template<auto value>
inline constexpr constant<value> cval;


#ifndef DOXYGEN
namespace detail {

    template<typename T>
    struct is_constant : std::false_type {};
    template<auto v>
    struct is_constant<constant<v>> : std::true_type {};

    template<typename T>
    struct constant_value;
    template<auto v>
    struct constant_value<constant<v>> { static constexpr auto value = v; };

    template<typename T>
    struct is_zero_constant;
    template<typename T> requires(!is_constant<T>::value)
    struct is_zero_constant<T> : std::false_type {};
    template<typename T> requires(is_constant<T>::value)
    struct is_zero_constant<T> : std::bool_constant<constant_value<T>::value == 0> {};

}  // namespace detail
#endif  // DOXYGEN

template<typename T>
inline constexpr bool is_zero_constant_v = detail::is_zero_constant<std::remove_cvref_t<T>>::value;

//! Base class for negatable symbols
struct negatable {
    template<typename Self>
    constexpr auto operator-(this Self&& self) {
        return constant<-1>{}*std::forward<Self>(self);
    }
};

//! symbol that represents a (runtime) constant
template<typename T, auto _ = [] () {}>
struct val : bindable, negatable {
 private:
    static constexpr bool is_reference = std::is_lvalue_reference_v<T>;
    using stored_t = std::conditional_t<
        is_reference,
        std::add_pointer_t<std::remove_reference_t<T>>,
        std::remove_cvref_t<T>
    >;
    static stored_t _value;

 public:
    using stored_type = stored_t;

    constexpr val() = default;
    constexpr val(val&&) = default;
    constexpr val(const val&) = default;

    template<typename _T>
        requires(contains_decayed_v<_T, T>)
    constexpr val(_T&& v) noexcept {
        if constexpr (is_reference) {
            static_assert(std::is_lvalue_reference_v<T>, "Provided value is not an lvalue reference");
            val<T, _>::_value = &v;
        } else {
            val<T, _>::_value = std::forward<_T>(v);
        }
    }

    template<typename _T, auto __ = [] () {}>
        requires(contains_decayed_v<_T, T>)
    constexpr auto operator=(_T&& v) noexcept {
        return val<T, __>{v};
    }

    //! Return the bound value (signature compatible with the expression interface)
    template<typename... B>
    constexpr decltype(auto) evaluate(const bindings<B...>&) const noexcept {
        return get();
    }

    //! Back-propagate derivatives w.r.t. the given variables (signature compatible with the expression interface)
    template<scalar R, typename... B, typename... V>
    constexpr auto back_propagate(const bindings<B...>&, const type_list<V...>&) const noexcept {
        return std::make_pair(get(), derivatives<R, V...>{});
    }

    //! Differentiate this expression (symbol) w.r.t. the given symbol
    template<typename Self, typename V>
    constexpr auto differentiate(this Self&&, const type_list<V>&) noexcept {
        if constexpr (std::is_same_v<std::remove_cvref_t<Self>, std::remove_cvref_t<V>>)
            return constant<1>{};
        else
            return constant<0>{};
    }

    //! Export the value to the given output stream
    template<typename... B>
    constexpr void export_to(std::ostream& out, const bindings<B...>&) const {
        out << get();
    }

    //! Return the value of this symbol
    const T& get() const {
        if constexpr (is_reference)
            return *val<T, _>::_value;
        else
            return val<T, _>::_value;
    }
};

template<typename T, auto _>
typename val<T, _>::stored_t val<T, _>::_value = {};

template<typename T, auto _ = [] () {}>
val(T&&) -> val<T, _>;

template<typename T, auto _>
struct is_symbol<val<T, _>> : std::true_type {};

//! Data structure to store a value bound to a symbol
template<typename S, typename V>
struct value_binder {
    using symbol_type = std::remove_cvref_t<S>;
    using value_type = std::remove_cvref_t<V>;

    template<same_remove_cvref_t_as<V> _V>
    constexpr value_binder(const S&, _V&& v) noexcept
    : _value{std::forward<_V>(v)}
    {}

    //! Return the bound value
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
value_binder(S&&, V&&) -> value_binder<std::remove_cvref_t<S>, V>;


//! Data structure to represent a symbol
template<typename T>
struct symbol : bindable, negatable {
    constexpr symbol() = default;
    constexpr symbol(symbol&&) = default;
    constexpr symbol(const symbol&) = delete;

    //! bind the given value to this symbol
    template<typename Self, typename V> requires(accepts<T, V>)
    constexpr auto bind(this Self&& self, V&& value) noexcept {
        return value_binder(std::forward<Self>(self), std::forward<V>(value));
    }

    //! bind the given value to this symbol
    template<typename Self, typename V> requires(accepts<T, V>)
    constexpr auto operator=(this Self&& self, V&& value) noexcept {
        return self.bind(std::forward<V>(value));
    }

    //! evaluate this expression (symbol) at the given values (signature compatible with the expression interface)
    template<typename Self, typename... B>
        requires(bindings<B...>::template contains_bindings_for<Self>)
    constexpr decltype(auto) evaluate(this Self&& self, const bindings<B...>& b) noexcept {
        return b[self];
    }

    //! Back-propagate derivatives w.r.t. the given variables (signature compatible with the expression interface)
    template<scalar R, typename Self, typename B, typename... V>
    constexpr auto back_propagate(this Self&& self, const B& bindings, const type_list<V...>&) {
        derivatives<R, V...> derivs{};
        if constexpr (contains_decayed_v<Self, V...>)
            derivs[self] = 1.0;
        return std::make_pair(self.evaluate(bindings), std::move(derivs));
    }

    //! Differentiate this expression (symbol) w.r.t. the given symbol
    template<typename Self, typename V>
    constexpr auto differentiate(this Self&&, const type_list<V>&) {
        if constexpr (same_remove_cvref_t_as<Self, V>)
            return cval<1>;
        else
            return cval<0>;
    }

    //! Export this symbol to the given stream
    template<typename Self, typename... V>
        requires(bindings<V...>::template contains_bindings_for<Self>)
    constexpr void export_to(this Self&& self, std::ostream& out, const bindings<V...>& name_bindings) {
        out << name_bindings[self];
    }
};

//! Symbol implementation to represent independent variables of expressions
template<typename T = dtype::any, auto = [] () {}>
struct var : symbol<T> {
    using symbol<T>::operator=;

    // for better compiler error messages about symbols being unique (not copyable)
    template<typename _T, auto __>
    constexpr var& operator=(const var<_T, __>&) = delete;
};

//! Symbol implementation to represent parameters of expressions
template<typename T = dtype::any, auto = [] () {}>
struct let : symbol<T> {
    using symbol<T>::operator=;

    // for better compiler error messages about symbols being unique (not copyable)
    template<typename _T, auto __>
    constexpr let& operator=(const let<_T, __>&) = delete;
};

template<typename T, auto _> struct is_symbol<var<T, _>> : public std::true_type {};
template<typename T, auto _> struct is_symbol<let<T, _>> : public std::true_type {};
template<typename T, auto _> struct is_unbound_symbol<var<T, _>> : public std::true_type {};
template<typename T, auto _> struct is_unbound_symbol<let<T, _>> : public std::true_type {};

//! Convert the given value into a term
template<into_term T, auto _ = [] () {}>
inline constexpr decltype(auto) as_term(T&& t) noexcept {
    if constexpr (term<std::remove_cvref_t<T>>)
        return std::forward<T>(t);
    else
        return val<T, _>{std::forward<T>(t)};
}

//! \} group Backward

}  // namespace adpp::backward
