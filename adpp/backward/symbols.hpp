#pragma once

#include <ostream>
#include <utility>
#include <type_traits>

#include <adpp/dtype.hpp>
#include <adpp/common.hpp>
#include <adpp/concepts.hpp>
#include <adpp/type_traits.hpp>

#include <adpp/backward/concepts.hpp>
#include <adpp/backward/bindings.hpp>
#include <adpp/backward/derivatives.hpp>

namespace adpp::backward {

template<auto v>
struct constant {
    static constexpr auto value = v;

    template<auto k> constexpr auto operator+(const constant<k>&) const noexcept { return constant<v+k>{}; }
    template<auto k> constexpr auto operator-(const constant<k>&) const noexcept { return constant<v-k>{}; }
    template<auto k> constexpr auto operator*(const constant<k>&) const noexcept { return constant<v*k>{}; }
    template<auto k> constexpr auto operator/(const constant<k>&) const noexcept { return constant<v/k>{}; }

    template<typename... B>
    constexpr auto operator()(const bindings<B...>&) const noexcept {
        return value;
    }

    template<typename... B>
    constexpr auto evaluate_at(const bindings<B...>&) const noexcept {
        return value;
    }

    template<concepts::arithmetic R, typename... B, typename... V>
    constexpr auto back_propagate(const bindings<B...>&, const type_list<V...>&) const noexcept {
        return std::make_pair(value, derivatives<R, V...>{});
    }

    template<typename Self, typename V>
    constexpr auto differentiate_wrt(this Self&&, const type_list<V>&) noexcept {
        if constexpr (std::is_same_v<std::remove_cvref_t<Self>, std::remove_cvref_t<V>>)
            return constant<1>{};
        else
            return constant<0>{};
    }

    template<typename... B>
    constexpr std::ostream& stream(std::ostream& out, const bindings<B...>&) const {
        out << value;
        return out;
    }
};


template<auto v>
struct is_symbol<constant<v>> : std::true_type {};

template<auto value>
inline constexpr constant<value> val;


template<typename T, auto _ = [] () {}>
struct value {
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

    constexpr value() = default;
    constexpr value(value&&) = default;
    constexpr value(const value&) = default;

    template<typename _T>
        requires(contains_decay_v<_T, T>)
    constexpr value(_T&& v) noexcept {
        if constexpr (is_reference) {
            static_assert(std::is_lvalue_reference_v<T>, "Provided value is not an lvalue reference");
            value<T, _>::_value = &v;
        } else {
            value<T, _>::_value = std::forward<_T>(v);
        }
    }

    template<typename _T, auto __ = [] () {}>
        requires(contains_decay_v<_T, T>)
    constexpr auto operator=(_T&& v) noexcept {
        return value<T, __>{v};
    }

    template<typename... B>
    constexpr decltype(auto) operator()(const bindings<B...>&) const noexcept {
        return get();
    }

    template<typename... B>
    constexpr decltype(auto) evaluate_at(const bindings<B...>&) const noexcept {
        return get();
    }

    template<concepts::arithmetic R, typename... B, typename... V>
    constexpr auto back_propagate(const bindings<B...>&, const type_list<V...>&) const noexcept {
        return std::make_pair(get(), derivatives<R, V...>{});
    }

    template<typename Self, typename V>
    constexpr auto differentiate_wrt(this Self&&, const type_list<V>&) noexcept {
        if constexpr (std::is_same_v<std::remove_cvref_t<Self>, std::remove_cvref_t<V>>)
            return constant<1>{};
        else
            return constant<0>{};
    }

    template<typename... B>
    constexpr std::ostream& stream(std::ostream& out, const bindings<B...>&) const {
        out << get();
        return out;
    }

    const T& get() const {
        if constexpr (is_reference)
            return *value<T, _>::_value;
        else
            return value<T, _>::_value;
    }
};

template<typename T, auto _>
typename value<T, _>::stored_t value<T, _>::_value = {};

template<typename T, auto _ = [] () {}>
value(T&&) -> value<T, _>;

template<typename T, auto _>
struct is_symbol<value<T, _>> : std::true_type {};


template<typename S, typename V>
struct value_binder {
    using symbol_type = std::remove_cvref_t<S>;
    using value_type = std::remove_cvref_t<V>;

    template<concepts::same_decay_t_as<V> _V>
    constexpr value_binder(const S&, _V&& v) noexcept
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

    template<typename Self, typename... B>
        requires(bindings<B...>::template contains_bindings_for<Self>)
    constexpr decltype(auto) operator()(this Self&& self, const bindings<B...>& bindings) noexcept {
        return bindings[self];
    }

    // currently unused
    template<typename Self, typename... B>
        requires(bindings<B...>::template contains_bindings_for<Self>)
    constexpr decltype(auto) evaluate_at(this Self&& self, const bindings<B...>& b) noexcept {
        return b[self];
    }

    template<concepts::arithmetic R, typename Self, typename B, typename... V>
    constexpr auto back_propagate(this Self&& self, const B& bindings, const type_list<V...>&) {
        derivatives<R, V...> derivs{};
        if constexpr (contains_decay_v<Self, V...>)
            derivs[self] = 1.0;
        return std::make_pair(self.evaluate_at(bindings), std::move(derivs));
    }

    template<typename Self, typename V>
    constexpr auto differentiate_wrt(this Self&&, const type_list<V>&) {
        if constexpr (concepts::same_decay_t_as<Self, V>)
            return val<1>;
        else
            return val<0>;
    }

    template<typename Self, typename... V>
        requires(bindings<V...>::template contains_bindings_for<Self>)
    constexpr std::ostream& stream(this Self&& self, std::ostream& out, const bindings<V...>& name_bindings) {
        out << name_bindings[self];
        return out;
    }
};

template<typename T = dtype::any, auto = [] () {}>
struct var : symbol<T> {
    using symbol<T>::operator=;

    // for better compiler error messages about symbols being unique (not copyable)
    template<typename _T, auto __>
    constexpr var& operator=(const var<_T, __>&) = delete;
};

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

}  // namespace adpp::backward
