#pragma once

#include <ostream>
#include <utility>
#include <type_traits>

#include <cppad/dtype.hpp>
#include <cppad/common.hpp>
#include <cppad/concepts.hpp>

#include <cppad/backward/concepts.hpp>
#include <cppad/backward/operand.hpp>
#include <cppad/backward/derivative.hpp>

namespace cppad::backward {

template<typename S, typename V>
struct value_binder {
    using symbol_type = std::remove_cvref_t<S>;

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
struct symbol : operand {
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

    template<typename Self, typename B>
    constexpr decltype(auto) evaluate_at(this Self&& self, const B& bindings) noexcept {
        return bindings[self];
    }

    template<typename Self, typename B, typename... V>
    constexpr auto back_propagate(this Self&& self, const B& bindings, const V&... vars) {
        using value_type = std::remove_cvref_t<decltype(bindings[self])>;
        derivatives derivs{value_type{}, vars...};
        if constexpr (contains_decay_v<Self, V...>)
            derivs[self] = 1.0;
        return std::make_pair(self.evaluate_at(bindings), std::move(derivs));
    }

    template<typename Self, typename V>
    constexpr auto differentiate_wrt(this Self&& self, V&& var) {
        if constexpr (concepts::same_decay_t_as<Self, V>)
            return val<int>{1};
        else
            return val<int>{0};
    }

    template<typename Self, typename... V>
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

namespace traits {

template<typename T, auto _> struct is_leaf_expression<var<T, _>> : public std::true_type {};
template<typename T, auto _> struct is_symbol<var<T, _>> : public std::true_type {};
template<typename T, auto _> struct is_var<var<T, _>> : public std::true_type {};

template<typename T, auto _> struct is_leaf_expression<let<T, _>> : public std::true_type {};
template<typename T, auto _> struct is_symbol<let<T, _>> : public std::true_type {};
template<typename T, auto _> struct is_let<let<T, _>> : public std::true_type {};

template<concepts::arithmetic T>
struct into_operand<T> {
    template<concepts::same_decay_t_as<T> _T>
    static constexpr auto get(_T&& t) noexcept {
        return val{std::forward<_T>(t)};
    }
};

template<typename T, auto _>
struct into_operand<var<T, _>> {
    template<concepts::same_decay_t_as<var<T, _>> V>
    static constexpr decltype(auto) get(V&& var) noexcept {
        return std::forward<V>(var);
    }
};

template<typename T, auto _>
struct into_operand<let<T, _>> {
    template<concepts::same_decay_t_as<let<T, _>> L>
    static constexpr decltype(auto) get(L&& let) noexcept {
        return std::forward<L>(let);
    }
};

}  // namespace traits
}  // namespace cppad::backward
