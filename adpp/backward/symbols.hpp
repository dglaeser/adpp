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
struct constant : bindable {
    static constexpr auto value = v;

    constexpr auto operator-() { return constant<-v>{}; }
    template<auto k> constexpr auto operator+(const constant<k>&) const noexcept { return constant<v+k>{}; }
    template<auto k> constexpr auto operator-(const constant<k>&) const noexcept { return constant<v-k>{}; }
    template<auto k> constexpr auto operator*(const constant<k>&) const noexcept { return constant<v*k>{}; }
    template<auto k> constexpr auto operator/(const constant<k>&) const noexcept { return constant<v/k>{}; }

    template<typename... B>
    constexpr auto evaluate(const bindings<B...>&) const noexcept {
        return value;
    }

    template<scalar R, typename... B, typename... V>
    constexpr auto back_propagate(const bindings<B...>&, const type_list<V...>&) const noexcept {
        return std::make_pair(value, derivatives<R, V...>{});
    }

    template<typename Self, typename V>
    constexpr auto differentiate(this Self&&, const type_list<V>&) noexcept {
        if constexpr (std::is_same_v<std::remove_cvref_t<Self>, std::remove_cvref_t<V>>)
            return constant<1>{};
        else
            return constant<0>{};
    }

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


struct negatable {
    template<typename Self>
    constexpr auto operator-(this Self&& self) {
        return constant<-1>{}*std::forward<Self>(self);
    }
};


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

    template<typename... B>
    constexpr decltype(auto) evaluate(const bindings<B...>&) const noexcept {
        return get();
    }

    template<scalar R, typename... B, typename... V>
    constexpr auto back_propagate(const bindings<B...>&, const type_list<V...>&) const noexcept {
        return std::make_pair(get(), derivatives<R, V...>{});
    }

    template<typename Self, typename V>
    constexpr auto differentiate(this Self&&, const type_list<V>&) noexcept {
        if constexpr (std::is_same_v<std::remove_cvref_t<Self>, std::remove_cvref_t<V>>)
            return constant<1>{};
        else
            return constant<0>{};
    }

    template<typename... B>
    constexpr void export_to(std::ostream& out, const bindings<B...>&) const {
        out << get();
    }

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


template<typename S, typename V>
struct value_binder {
    using symbol_type = std::remove_cvref_t<S>;
    using value_type = std::remove_cvref_t<V>;

    template<same_remove_cvref_t_as<V> _V>
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
value_binder(S&&, V&&) -> value_binder<std::remove_cvref_t<S>, V>;


template<typename T>
struct symbol : bindable, negatable {
    constexpr symbol() = default;
    constexpr symbol(symbol&&) = default;
    constexpr symbol(const symbol&) = delete;

    template<typename Self, typename V> requires(accepts<T, V>)
    constexpr auto bind(this Self&& self, V&& value) noexcept {
        return value_binder(std::forward<Self>(self), std::forward<V>(value));
    }

    template<typename Self, typename V> requires(accepts<T, V>)
    constexpr auto operator=(this Self&& self, V&& value) noexcept {
        return self.bind(std::forward<V>(value));
    }

    template<typename Self, typename... B>
        requires(bindings<B...>::template contains_bindings_for<Self>)
    constexpr decltype(auto) evaluate(this Self&& self, const bindings<B...>& b) noexcept {
        return b[self];
    }

    template<scalar R, typename Self, typename B, typename... V>
    constexpr auto back_propagate(this Self&& self, const B& bindings, const type_list<V...>&) {
        derivatives<R, V...> derivs{};
        if constexpr (contains_decayed_v<Self, V...>)
            derivs[self] = 1.0;
        return std::make_pair(self.evaluate(bindings), std::move(derivs));
    }

    template<typename Self, typename V>
    constexpr auto differentiate(this Self&&, const type_list<V>&) {
        if constexpr (same_remove_cvref_t_as<Self, V>)
            return cval<1>;
        else
            return cval<0>;
    }

    template<typename Self, typename... V>
        requires(bindings<V...>::template contains_bindings_for<Self>)
    constexpr void export_to(this Self&& self, std::ostream& out, const bindings<V...>& name_bindings) {
        out << name_bindings[self];
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

template<into_term T, auto _ = [] () {}>
inline constexpr decltype(auto) as_term(T&& t) noexcept {
    if constexpr (term<std::remove_cvref_t<T>>)
        return std::forward<T>(t);
    else
        return val<T, _>{std::forward<T>(t)};
}

}  // namespace adpp::backward
