#pragma once

#include <type_traits>
#include <initializer_list>

#include <adpp/common.hpp>
#include <adpp/type_traits.hpp>
#include <adpp/backward/concepts.hpp>
#include <adpp/backward/expression.hpp>
#include <adpp/backward/symbols.hpp>

namespace adpp::backward {

template<term... Es>
    requires(are_unique_v<Es...>)
struct vector_expression : indexed<Es...> {
    static constexpr std::size_t size = sizeof...(Es);

    constexpr vector_expression() = default;
    constexpr vector_expression(const Es&...) {}

    template<typename Self, static_vec_n<size> V>
    constexpr auto bind(this Self&& self, V&& value) noexcept {
        return value_binder(std::forward<Self>(self), std::forward<V>(value));
    }

    template<typename Self, static_vec_n<size> V>
    constexpr auto operator=(this Self&& self, V&& values) noexcept {
        return self.bind(std::forward<V>(values));
    }

    template<typename Self, typename V>
    constexpr auto operator=(this Self&& self, std::initializer_list<V> values) noexcept {
        return self.bind(std::array{values});
    }

    template<typename Self, typename... B>
        requires(bindings<B...>::template contains_bindings_for<Self>)
    constexpr decltype(auto) evaluate(this Self&& self, const bindings<B...>& b) noexcept {
        return b[self];
    }
};

template<term... Es>
vector_expression(Es&&...) -> vector_expression<std::remove_cvref_t<Es>...>;


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
