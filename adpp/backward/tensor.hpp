// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Backward
 * \brief Data structure for tensorial expressions.
 */
#pragma once

#include <array>
#include <tuple>
#include <ranges>
#include <type_traits>
#include <functional>

#include <adpp/utils.hpp>
#include <adpp/backward/concepts.hpp>
#include <adpp/backward/expression.hpp>
#include <adpp/backward/bindings.hpp>
#include <adpp/backward/symbols.hpp>
#include <adpp/backward/derivatives.hpp>

namespace adpp::backward {

//! \addtogroup Backward
//! \{

//! Binder for tensorial expressions, takes the values for all elements as a flat vector
template<typename S, typename V>
    requires(static_vec_n<std::remove_cvref_t<V>, S::shape.count>)
struct tensor_value_binder : value_binder<S, V>{
    static constexpr std::size_t number_of_sub_binders = S::shape.count;

    using value_binder<S, V>::value_binder;

    //! Return a tuple with the binders of all values in the tensor
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
            return value_type_t<std::remove_cvref_t<V>>{this->unwrap()[i]};
        else
            return this->unwrap()[i];
    }
};

template<typename E, typename S>
tensor_value_binder(E&&, S&&) -> tensor_value_binder<std::remove_cvref_t<E>, S>;

//! Stores values of type T accessible via indices in the given shape
template<typename T, auto shape>
class md_array {
 public:
    constexpr md_array() = default;

    //! Return the value at the given md index
    template<typename Self, std::size_t... i>
    constexpr decltype(auto) operator[](this Self&& self, const md_index_constant<i...>&) {
        static_assert(shape.flat_index_of(i...) < shape.count);
        return self._values[shape.flat_index_of(i...)];
    }

    //! Return the value at the given indices
    template<typename Self, std::integral... I>
        requires(sizeof...(I) == shape.dimension)
    constexpr decltype(auto) operator[](this Self&& self, I&&... indices) {
        return self._values[shape.flat_index_of(std::forward<I>(indices)...)];
    }

    //! Return the value at the given index (only available for vectors)
    template<typename Self, std::integral I>
        requires(shape.dimension == 1 or (shape.dimension == 2 && shape.last() == 1))
    constexpr decltype(auto) operator[](this Self&& self, const I& index) {
        return self._values[index];
    }

    template<typename Self> constexpr auto begin(this Self&& self) { return self._values.begin(); }
    template<typename Self> constexpr auto end(this Self&& self) { return self._values.end(); }

 private:
    std::array<T, shape.count> _values;
};

//! A tensorial expression with a given shape
template<auto md_shape, term... Es>
    requires(sizeof...(Es) > 0 and sizeof...(Es) == md_shape.count and are_unique_v<Es...>)
struct tensor_expression : bindable, indexed<Es...> {
    static constexpr std::size_t size = md_shape.count;
    static constexpr auto shape = md_shape;
    static constexpr bool is_vector = shape.dimension == 1 || (shape.dimension == 2 and shape.last() == 1);
    using sub_expressions = type_list<Es...>;

    constexpr tensor_expression() = default;
    constexpr tensor_expression(const Es&...) {}
    template<std::size_t... n>
    constexpr tensor_expression(adpp::md_shape<n...> d, const Es&...) requires(d == shape) {}

    //! Bind the given vector to this expression (has to be as many elements as this expression)
    template<typename Self, typename V>
        requires(static_vec_n<std::remove_cvref_t<V>, size>)
    constexpr auto bind(this Self&& self, V&& value) noexcept {
        return tensor_value_binder{std::forward<Self>(self), std::forward<V>(value)};
    }

    //! Bind the given vector to this expression (has to be as many elements as this expression)
    template<typename Self, typename V>
    constexpr auto operator=(this Self&& self, V&& values) noexcept {
        return self.bind(std::forward<V>(values));
    }

    //! Bind the given array to this expression (has to be as many elements as this expression)
    template<typename Self, typename T>
    constexpr auto operator=(this Self&& self, T (&&values)[size]) noexcept {
        return self.bind(to_array<T, size>::from(std::move(values)));
    }

    // TODO: Rename? Or put somewhere else? Free function?
    //! Return the jacobian matrix of this expression for the given values (only available for vectors)
    template<typename... B>
    constexpr auto jacobian(const bindings<B...>& bindings) const noexcept requires(is_vector) {
        using vars = vars_t<first_type_t<sub_expressions>>;
        auto results_tuple = _apply_to_all([&] <typename V> (auto, V&& v) {
            return derivatives_of(std::forward<V>(v), vars{}, bindings);
        });
        return std::apply([] <typename... R> (R&&... results) {
            return backward::jacobian{std::forward<R>(results)...};
        }, std::move(results_tuple));
    }

    // TODO: Rename? Or put somewhere else? Free function?
    //! Return the jacobian matrix w.r.t the given variables as a new expression (only available for vectors)
    template<typename... V>
    constexpr auto jacobian_expression(const type_list<V...>&) const noexcept requires(is_vector) {
        auto derivs_tuple = _apply_to_all([&] <typename E> (auto, E&& expr) {
            return std::tuple{differentiate(expr, type_list<V>{})...};
        });
        return std::apply([] <typename... R> (R&&... results) {
            return backward::tensor_expression{
                adpp::shape<shape.first(), sizeof...(V)>,
                std::forward<R>(results)...
            };
        }, std::move(derivs_tuple));
    }

    //! evaluate this expression at the given values (signature compatible with the expression interface)
    template<typename... B>
    constexpr auto evaluate(const bindings<B...>& b) const noexcept {
        md_array<typename bindings<B...>::common_value_type, shape> result;
        _visit([&] <auto... i> (md_index_constant<i...> md_index) {
            result[md_index] = (*this)[md_index].evaluate(b);
        }, md_index_constant_iterator{shape});
        return result;
    }

    //! Export this expression to the given stream
    template<typename... V>
    constexpr void export_to(std::ostream& out, const bindings<V...>& name_bindings) const {
        out << "[";
        bool first = true;
        _visit([&] <auto... i> (md_index_constant<i...> idx) {
            if constexpr (is_vector && idx.at(index<0>) > 0) out << ", ";
            if constexpr (!is_vector && idx.last() > 0) out << ", ";
            else if (!is_vector && idx.last() == 0 && !first) out << " // ";
            (*this)[idx].export_to(out, name_bindings);
            first = false;
        }, md_index_constant_iterator{shape});
        out << "]";
    }

    //! Return the expression at the given md index
    template<std::size_t... i>
    constexpr auto operator[](const md_index_constant<i...>&) const {
        static_assert(shape.flat_index_of(i...) < size);
        return this->make(shape.flat_index_of(md_index<i...>));
    }

    //! Return the expression at the given index (only available for vectors)
    template<std::size_t i>
    constexpr auto operator[](const index_constant<i>& idx) const requires(is_vector) {
        return this->make(idx);
    }

    //! Scale the tensor expression with a scalar value/expression
    template<into_term V>
        requires(is_scalar_expression_v<std::remove_cvref_t<decltype(as_term(std::declval<const V&>()))>>)
    constexpr auto operator*(V&& value) const {
        return scaled_with(std::forward<V>(value));
    }

    //! Perform a dot product (on vectors) or matrix multiplication (on tensors)
    template<auto other_shape, typename... T>
    constexpr auto operator*(const tensor_expression<other_shape, T...>& other) const {
        return dot(other);
    }

    //! Scale the tensor expression with a scalar value/expression
    template<into_term V>
        requires(is_scalar_expression_v<std::remove_cvref_t<decltype(as_term(std::declval<const V&>()))>>)
    constexpr auto scaled_with(V&& value) const noexcept {
        auto results_tuple = _apply_to_all([&] (auto, auto&& v) {
            return std::move(v)*as_term(value);
        });
        return std::apply([] <typename... R> (R&&... results) {
            return backward::tensor_expression{shape, std::forward<R>(results)...};
        }, std::move(results_tuple));
    }

    //! Perform a dot product (on vectors) or matrix multiplication (on tensors)
    template<auto other_shape, typename... T>
        requires(is_vector and shape.extent_in(index<0>) == other_shape.extent_in(index<0>))
    constexpr auto dot(const tensor_expression<other_shape, T...>& other) const {
        return _reduce([&] (auto i, auto&& e) {
            if constexpr (is_zero_constant_v<decltype(e)>)
                return (*this)[i]*other[i];
            else
                return std::move(e) + (*this)[i]*other[i];
        }, cval<0>, md_index_constant_iterator{shape});
    }

    //! Perform a dot product (on vectors) or matrix multiplication (on tensors)
    template<auto other_shape, typename... T>
        requires(!is_vector and shape.last() == other_shape.extent_in(index<0>))
    constexpr auto dot(const tensor_expression<other_shape, T...>& other) const {
        static constexpr auto my_dim = shape.dimension;
        using head = typename split_at<my_dim - 1, typename decltype(shape)::as_value_list>::head;
        using tail = typename split_at<1, typename decltype(other_shape)::as_value_list>::tail;

        static constexpr auto new_shape = adpp::md_shape{head{} + tail{}};
        auto result_tuple = _reduce([&] <auto... i> (md_index_constant<i...>, auto&& terms) {
            constexpr auto i_head = typename split_at<my_dim - 1, typename md_index_constant<i...>::as_value_list>::head{};
            constexpr auto i_tail = typename split_at<my_dim - 1, typename md_index_constant<i...>::as_value_list>::tail{};
            return std::tuple_cat(
                std::move(terms),
                std::tuple{
                    _reduce([&] <auto... j> (md_index_constant<j...>, auto&& e) {
                        static constexpr auto my_idx = md_index_constant{i_head + value_list<j...>{}};
                        static constexpr auto other_idx = md_index_constant{value_list<j...>{} + i_tail};
                        if constexpr (is_zero_constant_v<decltype(e)>)
                            return (*this)[my_idx]*other[other_idx];
                        else
                            return std::move(e) + (*this)[my_idx]*other[other_idx];
                    }, cval<0>, md_index_constant_iterator{adpp::md_shape<shape.last()>{}})
                }
            );
        }, std::tuple{}, md_index_constant_iterator{new_shape});
        return std::apply([] <typename... Terms> (Terms&&... ts) {
            return backward::tensor_expression{new_shape, std::forward<Terms>(ts)...};
        }, std::move(result_tuple));
    }

    //! Return the expression representing the squared l2-norm of this expression (only available for vectors)
    constexpr auto l2_norm_squared() const requires(is_vector) {
        return dot(*this);
    }

    //! Return the expression representing the l2-norm of this expression (only available for vectors)
    constexpr auto l2_norm() const requires(is_vector) {
        return sqrt(l2_norm_squared());
    }

 private:
    template<typename A>
    constexpr auto _apply_to_all(const A& action) const {
        return _reduce([&] (auto i, auto&& tup) {
            return std::tuple_cat(std::move(tup), std::tuple{action(i, (*this)[i])});
        }, std::tuple{}, md_index_constant_iterator{shape});
    }

    template<typename A, typename V, typename I>
    constexpr auto _reduce(const A& action, V&& value, I&& index_iterator) const {
        if constexpr (!I::is_end())
            return _reduce(action, action(index_iterator.current(), std::move(value)), index_iterator.next());
        else
            return std::move(value);
    }

    template<typename V, typename I>
    constexpr void _visit(const V& visitor, const I& index_iterator) const {
        if constexpr (!I::is_end()) {
            visitor(index_iterator.current());
            _visit(visitor, index_iterator.next());
        }
    }
};

template<std::size_t... n, term... Es>
    requires(md_shape<n...>::count == sizeof...(Es))
tensor_expression(md_shape<n...>, Es&&...) -> tensor_expression<md_shape<n...>{}, std::remove_cvref_t<Es>...>;


template<auto shape, typename... T>
struct is_expression<tensor_expression<shape, T...>> : std::true_type {};
template<auto shape, typename... T>
struct is_scalar_expression<tensor_expression<shape, T...>> : std::false_type {};
template<auto shape, typename... T>
struct operands<tensor_expression<shape, T...>> : std::type_identity<type_list<T...>> {};

//! Convenience class to create a vector expression
template<term... Es>
struct vector_expression : tensor_expression<md_shape<sizeof...(Es), 1>{}, Es...> {
 private:
    using base = tensor_expression<md_shape<sizeof...(Es), 1>{}, Es...>;
 public:
    using base::base;
    using base::operator=;
};

template<term... Es>
vector_expression(Es&&...) -> vector_expression<std::remove_cvref_t<Es>...>;

template<typename... T>
struct is_expression<vector_expression<T...>> : std::true_type {};
template<typename... T>
struct is_scalar_expression<vector_expression<T...>> : std::false_type {};
template<typename... T>
struct operands<vector_expression<T...>> : std::type_identity<type_list<T...>> {};


#ifndef DOXYGEN
namespace detail {

    template<auto lambda, std::size_t N, typename... T>
    struct vars_n;
    template<auto lambda, std::size_t N, typename... T> requires(sizeof...(T) < N)
    struct vars_n<lambda, N, type_list<T...>> : vars_n<lambda, N, type_list<var<dtype::any, [] () {}>, T...>> {};
    template<auto lambda, std::size_t N, typename... T> requires(sizeof...(T) == N)
    struct vars_n<lambda, N, type_list<T...>> : std::type_identity<type_list<T...>> {};

    template<typename T>
    struct vec_with;
    template<typename... T>
    struct vec_with<type_list<T...>> : std::type_identity<vector_expression<T...>> {};

    template<auto shape, typename T>
    struct tensor_with;
    template<auto shape, typename... T>
    struct tensor_with<shape, type_list<T...>> : std::type_identity<tensor_expression<shape, T...>> {};

}  // namespace detail
#endif  // DOXYGEN

// TODO: streamline vec with tensor? (i.e. make class and construct with ctad)
//! A vector expression composed of `var` instances
template<std::size_t N, auto _ = [] () {}>
using vec = typename detail::vec_with<typename detail::vars_n<_, N, type_list<>>::type>::type;

//! A tensor expression composed of `var` instances
template<auto shape, auto _ = [] () {}>
struct tensor
: detail::tensor_with<shape, typename detail::vars_n<_, shape.count, type_list<>>::type>::type {
    using base = detail::tensor_with<shape, typename detail::vars_n<_, shape.count, type_list<>>::type>::type;
    using base::operator=;

    template<std::size_t... n>
    constexpr tensor(md_shape<n...>) noexcept {}
};

template<std::size_t... n, auto _ = [] () {}>
tensor(md_shape<n...>) -> tensor<md_shape<n...>{}, _>;

template<auto shape, auto _>
struct is_expression<tensor<shape, _>> : is_expression<typename tensor<shape, _>::base> {};
template<auto shape, auto _>
struct is_scalar_expression<tensor<shape, _>> : is_scalar_expression<typename tensor<shape, _>::base> {};
template<auto shape, auto _>
struct operands<tensor<shape, _>> : operands<typename tensor<shape, _>::base> {};

//! \} group Backward

}  // namespace adpp::backward
