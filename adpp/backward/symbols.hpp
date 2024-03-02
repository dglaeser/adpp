#pragma once

#include <ostream>
#include <utility>
#include <type_traits>

#include <adpp/dtype.hpp>
#include <adpp/common.hpp>
#include <adpp/concepts.hpp>

#include <adpp/backward/concepts.hpp>
#include <adpp/backward/operand.hpp>
#include <adpp/backward/derivative.hpp>

namespace adpp::backward {

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
    constexpr auto differentiate_wrt(this Self&&, V&&) {
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

template<std::size_t N, typename T = dtype::any>
class vec_var : public symbol<dtype::array<T, N>> {
    template<std::size_t I, typename partial, auto _ = [] () {}>
    struct tuple_creator;

    template<std::size_t I, typename partial, auto _>
        requires(I == N - 1)
    struct tuple_creator<I, partial, _> {
        using type = partial;
    };

    template<std::size_t I, typename partial, auto _>
        requires(I < N - 1)
    struct tuple_creator<I, partial, _> {
        using type = std::remove_cvref_t<decltype(
            std::tuple_cat(std::declval<partial>(), std::declval<std::tuple<var<T, _>>>())
        )>;
    };


    using tuple = typename tuple_creator<0, std::tuple<>>::type;
 public:
    using symbol<dtype::array<T, N>>::operator=;

    constexpr const auto& vars() const {
        return _values;
    }

 private:
    tuple _values;
};

namespace traits {

template<typename T, auto _> struct is_leaf_expression<var<T, _>> : public std::true_type {};
template<typename T, auto _> struct is_symbol<var<T, _>> : public std::true_type {};
template<typename T, auto _> struct is_var<var<T, _>> : public std::true_type {};

template<typename T, auto _> struct is_leaf_expression<let<T, _>> : public std::true_type {};
template<typename T, auto _> struct is_symbol<let<T, _>> : public std::true_type {};
template<typename T, auto _> struct is_let<let<T, _>> : public std::true_type {};

template<std::size_t N, typename T> struct sub_expressions<vec_var<N, T>> {
    static constexpr const auto& get(const vec_var<N, T>& v) {
        return v.vars();
    }
};

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
}  // namespace adpp::backward
