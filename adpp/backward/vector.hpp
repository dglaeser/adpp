#pragma once

#include <array>
#include <tuple>
#include <ranges>
#include <type_traits>

#include <adpp/common.hpp>
#include <adpp/type_traits.hpp>
#include <adpp/backward/concepts.hpp>
#include <adpp/backward/expression.hpp>
#include <adpp/backward/symbols.hpp>

namespace adpp::backward {

#ifndef DOXYGEN
namespace detail {

    template<typename T> struct is_std_array : std::false_type {};
    template<typename T, std::size_t N> struct is_std_array<std::array<T, N>> : std::true_type {};

}  // namespace detail
#endif  // DOXYGEN

template<typename S, typename V>
    requires(static_vec_n<std::remove_cvref_t<V>, S::size>)
struct vector_value_binder {
    static constexpr std::size_t number_of_sub_binders = S::size;

    using symbol_type = std::remove_cvref_t<S>;
    using value_type = std::remove_cvref_t<V>;

    template<same_remove_cvref_t_as<V> _V>
    constexpr vector_value_binder(const S&, _V&& v) noexcept
    : _value{std::forward<_V>(v)}
    {}

    template<typename Self>
    constexpr decltype(auto) unwrap(this Self&& self) {
        if constexpr (!std::is_lvalue_reference_v<Self>)
            return std::move(self._value).get();
        else
            return self._value.get();
    }

    template<typename Self>
    constexpr auto sub_binders(this Self&& self) noexcept {
        static constexpr bool owning = !std::is_lvalue_reference_v<Self>;
        return self.template _concat<owning>(typename S::sub_expressions{}, std::tuple{});
    }

 private:
    template<bool owning, typename... E, typename... T>
    constexpr auto _concat(type_list<E...>, std::tuple<T...>&& current) const noexcept {
        if constexpr (sizeof...(T) == number_of_sub_binders) {
            return std::move(current);
        } else {
            static constexpr std::size_t i = sizeof...(T);
            return _concat<owning>(
                drop_first_type_t<type_list<E...>>{},
                std::tuple_cat(
                    std::move(current),
                    std::make_tuple(value_binder{first_type_t<type_list<E...>>{}, _get<i, owning>()})
                )
            );
        }
    }

    template<std::size_t i, bool copy>
    constexpr decltype(auto) _get() const noexcept {
        if constexpr (copy)
            return typename V::value_type{_value.get()[i]};
        else
            return _value.get()[i];
    }

    storage<V> _value;
};

template<typename E, typename S>
vector_value_binder(E&&, S&&) -> vector_value_binder<std::remove_cvref_t<E>, S>;

template<term... Es>
    requires(sizeof...(Es) > 0 and are_unique_v<Es...>)
struct vector_expression : bindable, indexed<Es...> {
    static constexpr std::size_t size = sizeof...(Es);
    using sub_expressions = type_list<Es...>;

    constexpr vector_expression() = default;
    constexpr vector_expression(const Es&...) {}

    template<typename Self, static_vec_n<size> V>
    constexpr auto bind(this Self&& self, V&& value) noexcept {
        return vector_value_binder{std::forward<Self>(self), std::forward<V>(value)};
    }

    template<typename Self, static_vec_n<size> V>
    constexpr auto operator=(this Self&& self, V&& values) noexcept {
        return self.bind(std::forward<V>(values));
    }

    template<typename Self> // TODO: generalize double somewhow?
    constexpr auto operator=(this Self&& self, std::array<double, size>&& values) noexcept {
        return self.bind(std::move(values));
    }

    template<typename Self> // TODO: generalize string/double somewhow?
    constexpr auto operator=(this Self&& self, std::array<std::string, size>&& values) noexcept {
        return self.bind(std::move(values));
    }

    template<typename... B>
    constexpr auto evaluate(const bindings<B...>& b) const noexcept {
        std::array<typename bindings<B...>::common_value_type, size> result;
        _visit<0>([&] <auto i> (index_constant<i> idx) {
            result[i] = (*this)[idx].evaluate(b);
        });
        return result;
    }

    template<typename... V>
    constexpr void export_to(std::ostream& out, const bindings<V...>& name_bindings) const {
        out << "[";
        _visit<0>([&] <auto i> (index_constant<i> idx) {
            if constexpr (i > 0)
                out << ", ";
            (*this)[idx].export_to(out, name_bindings);
        });
        out << "]";
    }

    template<std::size_t i>
    constexpr auto operator[](const index_constant<i>& idx) const {
        return this->make(idx);
    }

    template<into_term V>
        requires(is_scalar_expression_v<std::remove_cvref_t<decltype(as_term(std::declval<const V&>()))>>)
    constexpr auto scaled_with(V&& value) const noexcept {
        auto results_tuple = _apply_to_all([&] (auto, auto&& v) {
            return std::move(v)*as_term(value);
        });
        return std::apply([] <typename... R> (R&&... results) {
            return backward::vector_expression{std::forward<R>(results)...};
        }, std::move(results_tuple));
    }

    template<typename... T> requires(sizeof...(T) == size)
    constexpr auto dot(const vector_expression<T...>& other) const {
        return _reduce<0>([&] (auto i, auto&& e) {
            return std::move(e) + (*this)[i]*other[i];
        }, cval<0>);
    }

 private:
    template<typename A>
    constexpr auto _apply_to_all(const A& action) const {
        return _reduce<0>([&] (auto i, auto&& tup) {
            return std::tuple_cat(std::move(tup), std::tuple{action(i, (*this)[i])});
        }, std::tuple{});
    }

    template<std::size_t i, typename A, typename V>
    constexpr auto _reduce(const A& action, V&& value) const {
        if constexpr (i < size)
            return _reduce<i+1>(action, action(index_constant<i>{}, std::move(value)));
        else
            return std::move(value);
    }

    template<std::size_t i, typename V>
    constexpr void _visit(const V& visitor) const {
        visitor(index_constant<i>{});
        if constexpr (i < size - 1)
            _visit<i+1>(visitor);
    }

    // template<scalar R, typename Self, typename B, typename... V>
    // constexpr auto back_propagate(this Self&& self, const B& bindings, const type_list<V...>&) {
    //     derivatives<R, V...> derivs{};
    //     if constexpr (contains_decayed_v<Self, V...>)
    //         derivs[self] = 1.0;
    //     return std::make_pair(self.evaluate(bindings), std::move(derivs));
    // }

    // template<typename Self, typename V>
    // constexpr auto differentiate(this Self&&, const type_list<V>&) {
    //     if constexpr (same_remove_cvref_t_as<Self, V>)
    //         return cval<1>;
    //     else
    //         return cval<0>;
    // }


};

template<term... Es>
vector_expression(Es&&...) -> vector_expression<std::remove_cvref_t<Es>...>;


template<typename... T>
struct is_expression<vector_expression<T...>> : std::true_type {};
template<typename... T>
struct is_scalar_expression<vector_expression<T...>> : std::false_type {};

#ifndef DOXYGEN
namespace detail {

    template<std::size_t, auto _ = [] () {}>
    constexpr auto new_lambda() { return _; }

    template<std::size_t N, typename... T>
    struct vec_type;

    template<std::size_t N, typename... T> requires(sizeof...(T) < N)
    struct vec_type<N, type_list<T...>> : vec_type<N, type_list<var<dtype::any, [] () {}>, T...>> {};

    template<std::size_t N, typename... T> requires(sizeof...(T) == N)
    struct vec_type<N, type_list<T...>> : std::type_identity<vector_expression<T...>> {};

}  // namespace detail
#endif  // DOXYGEN

template<std::size_t N>
using vec = typename detail::vec_type<N, type_list<>>::type;

template<typename... T>
    requires(std::conjunction_v<is_symbol<T>...>)
struct is_symbol<vector_expression<T...>> : std::true_type {};

}  // namespace adpp::backward