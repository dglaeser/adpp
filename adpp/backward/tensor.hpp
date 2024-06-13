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

//! Binder for tensorial expressions, takes the values for all elements as a flat range or mdrange with matching shape
template<typename S, typename V>
    requires(
        static_vec_n<std::remove_cvref_t<V>, S::shape.count> or
        shape_of_v<std::remove_cvref_t<V>> == S::shape
    )
struct tensor_value_binder : value_binder<S, V>{
    static constexpr std::size_t number_of_sub_binders = S::shape.count;

    using value_binder<S, V>::value_binder;

    //! Return a tuple with the binders of all values in the tensor
    template<typename Self>
    constexpr auto sub_binders(this Self&& self) noexcept {
        static constexpr bool owning = !std::is_lvalue_reference_v<Self>;
        return self.template _concat<owning>(
            typename S::sub_expressions{},
            std::tuple{},
            md_index_constant_iterator{S::shape}
        );
    }

 private:
    template<bool owning, typename... E, typename... T, std::size_t... n, typename I>
    constexpr auto _concat(type_list<E...>,
                           std::tuple<T...>&& current,
                           md_index_constant_iterator<md_shape<n...>, I>&& md_iterator) const noexcept {
        if constexpr (md_index_constant_iterator<md_shape<n...>, I>::is_end()) {
            static_assert(sizeof...(T) == number_of_sub_binders);
            return std::move(current);
        } else {
            return _concat<owning>(
                drop_first_type_t<type_list<E...>>{},
                std::tuple_cat(
                    std::move(current),
                    std::make_tuple(value_binder{first_type_t<type_list<E...>>{}, _get<owning>(md_iterator.current())})
                ),
                md_iterator.next()
            );
        }
    }

    template<bool copy, std::size_t... is>
    constexpr decltype(auto) _get(md_index_constant<is...>&&) const noexcept {
        static constexpr bool is_vec = static_vec_n<std::remove_cvref_t<V>, S::shape.count>;
        if constexpr (is_vec) {
            static constexpr auto i = S::shape.flat_index_of(md_index_constant<is...>{});
            if constexpr (copy)
                return value_type_t<std::remove_cvref_t<V>>{this->unwrap()[i]};
            else
                return this->unwrap()[i];
        } else {
            if constexpr (copy)
                return md_value_type_t<std::remove_cvref_t<V>>{
                    access_with(md_index_constant<is...>{}, this->unwrap())
                };
            else
                return access_with(md_index_constant<is...>{}, this->unwrap());
        }
    }
};

template<typename E, typename S>
tensor_value_binder(E&&, S&&) -> tensor_value_binder<std::remove_cvref_t<E>, S>;

//! A tensorial expression with a given shape
template<auto md_shape, term... Es>
    requires(sizeof...(Es) > 0 and sizeof...(Es) == md_shape.count and are_unique_v<Es...>)
struct tensor_expression : bindable, indexed<Es...> {
    static constexpr std::size_t size = md_shape.count;
    static constexpr auto shape = md_shape;
    using sub_expressions = type_list<Es...>;

    constexpr tensor_expression() = default;
    constexpr tensor_expression(const Es&...) {}
    template<std::size_t... n>
    constexpr tensor_expression(adpp::md_shape<n...> d, const Es&...) requires(d == shape) {}

    //! Bind the given values to this expression (has to be as many elements as this expression)
    template<typename Self, typename V>
    constexpr auto bind(this Self&& self, V&& value) noexcept {
        return tensor_value_binder{std::forward<Self>(self), std::forward<V>(value)};
    }

    //! Bind the given values to this expression (has to be as many elements as this expression)
    template<typename Self, typename V>
    constexpr auto operator=(this Self&& self, V&& values) noexcept {
        return self.bind(std::forward<V>(values));
    }

    //! Bind the given values to this expression (has to be as many elements as this expression)
    template<typename Self, typename T>
    constexpr auto operator=(this Self&& self, T (&&values)[size]) noexcept {
        return self.bind(to_array<T, size>::from(std::move(values)));
    }

    // TODO: Rename? Or put somewhere else? Free function?
    //! Return the jacobian matrix of this expression for the given values (only available for vectors)
    template<typename... B>
    constexpr auto jacobian(const bindings<B...>& bindings) const noexcept requires(shape.is_vector()) {
        return jacobian(vars_t<tensor_expression>{}, bindings);
    }

    // TODO: Rename? Or put somewhere else? Free function?
    //! Return the jacobian matrix of this expression w.r.t the given variables at the given values (only available for vectors)
    template<typename... Vs, typename... B>
    constexpr auto jacobian(const type_list<Vs...>& vars, const bindings<B...>& bindings) const noexcept requires(shape.is_vector()) {
        auto results_tuple = _apply_to_all([&] <typename V> (auto, V&& v) {
            return derivatives_of(std::forward<V>(v), vars, bindings);
        });
        return std::apply([] <typename... R> (R&&... results) {
            return backward::jacobian{std::forward<R>(results)...};
        }, std::move(results_tuple));
    }

    // TODO: Rename? Or put somewhere else? Free function?
    //! Return the jacobian matrix expression w.r.t the given variables (only available for vectors)
    template<typename... V>
    constexpr auto jacobian_expression(const type_list<V...>&) const noexcept requires(shape.is_vector()) {
        auto derivs_tuple = _apply_to_all([&] <typename E> (auto, E&& expr) {
            return std::tuple{differentiate(expr, type_list<V>{})...};
        });
        return std::apply([] <typename... R> (R&&... results) {
            return backward::tensor_expression{
                adpp::shape<shape.first(), sizeof...(V)>,
                std::move(results)...
            };
        }, std::move(derivs_tuple));
    }

    //! evaluate this expression at the given values (signature compatible with the expression interface)
    template<typename... B>
    constexpr auto evaluate(const bindings<B...>& b) const noexcept {
        md_array<typename bindings<B...>::common_value_type, shape> result;
        for_each_index_in(shape, [&] <auto... i> (md_index_constant<i...> md_index) {
            result[md_index] = (*this)[md_index].evaluate(b);
        });
        return result;
    }

    //! Export this expression to the given stream
    template<typename... V>
    constexpr void export_to(std::ostream& out, const bindings<V...>& name_bindings) const {
        out << "[";
        bool first = true;
        for_each_index_in(shape, [&] <auto... i> (md_index_constant<i...> idx) {
            if constexpr (shape.is_vector() && idx.first() > 0) out << ", ";
            if constexpr (!shape.is_vector() && idx.last() > 0) out << ", ";
            else if (!shape.is_vector() && idx.last() == 0 && !first) out << " // ";
            (*this)[idx].export_to(out, name_bindings);
            first = false;
        });
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
    constexpr auto operator[](const index_constant<i>& idx) const requires(shape.is_vector()) {
        return this->make(idx);
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

    //! Perform a dot product with another vector
    template<auto other_shape, typename... T>
        requires(shape == other_shape)
    constexpr auto dot(const tensor_expression<other_shape, T...>& other) const {
        return _reduce([&] <typename E> (auto i, E&& e) {
            if constexpr (is_zero_constant_v<std::remove_cvref_t<E>>)
                return (*this)[i]*other[i];
            else
                return std::move(e) + (*this)[i]*other[i];
        }, cval<0>, md_index_constant_iterator{shape});
    }

    //! Perform a matrix multiplication with another tensor
    template<auto other_shape, typename... T>
        requires(!shape.is_vector() and shape.last() == other_shape.extent_in(index<0>))
    constexpr auto apply_to(const tensor_expression<other_shape, T...>& other) const {
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

    //! Return the trace of this tensor (only available for quadratic 2d tensors)
    template<typename Self>
    constexpr auto trace(this Self&& self) requires(shape.dimension == 2 and shape.first() == shape.last()) {
        return self._reduce(
            [&] <std::size_t i> (md_index_constant<i>, auto&& result) {
                return std::move(result) + self[md_index<i, i>];
            },
            self[md_index<0, 0>],
            md_index_constant_iterator{
                adpp::md_shape<shape.first()>{},
                md_index_constant<1>{}
            }
        );
    }

    //! Return the determinant of this tensor (only available for 2x2 or 3x3 tensors)
    template<typename Self>
    constexpr auto det(this Self&& self) requires(shape.dimension == 2) {
        using namespace indices;
        if constexpr (shape.extent_in(_0) == 2 && shape.extent_in(_1) == 2) {
            return self[md_index<0, 0>]*self[md_index<1, 1>]
                - self[md_index<1, 0>]*self[md_index<0, 1>];
        } else if constexpr (shape.extent_in(_0) == 3 && shape.extent_in(_1) == 3) {
            return self[md_index<0, 0>]*self[md_index<1, 1>]*self[md_index<2, 2>]
                + self[md_index<0, 1>]*self[md_index<1, 2>]*self[md_index<2, 0>]
                + self[md_index<0, 2>]*self[md_index<1, 0>]*self[md_index<2, 1>]
                - (
                    self[md_index<2, 0>]*self[md_index<1, 1>]*self[md_index<0, 2>]
                    + self[md_index<2, 1>]*self[md_index<1, 2>]*self[md_index<0, 0>]
                    + self[md_index<2, 2>]*self[md_index<1, 0>]*self[md_index<0, 1>]
                );
        } else {
            static_assert(always_false<>::value, "Determinant only implemented for 2x2 and 3x3 tensors");
        }
    }

    //! Return the inverse of this matrix (only available for 2x2 or 3x3 tensors)
    template<typename Self>
    constexpr auto inverted(this Self&& self) requires(shape.dimension == 2) {
        using namespace indices;
        if constexpr (shape.extent_in(_0) == 2 && shape.extent_in(_1) == 2) {
            return backward::tensor_expression{
                shape,
                self[md_index<1, 1>], -self[md_index<0, 1>],
                -self[md_index<1, 0>], self[md_index<0, 0>]
            }.scaled_with(self.det());
        } else if constexpr (shape.extent_in(_0) == 3 && shape.extent_in(_1) == 3) {
            // see https://mo.mathematik.uni-stuttgart.de/inhalt/beispiel/beispiel1113/
            return backward::tensor_expression{
                shape,
                self[md_index<1, 1>]*self[md_index<2, 2>] - self[md_index<1, 2>]*self[md_index<2, 1>],
                -self[md_index<0, 1>]*self[md_index<2, 2>] + self[md_index<0, 2>]*self[md_index<2, 1>],
                self[md_index<0, 1>]*self[md_index<1, 2>] - self[md_index<0, 2>]*self[md_index<1, 1>],

                -self[md_index<1, 0>]*self[md_index<2, 2>] + self[md_index<1, 2>]*self[md_index<2, 0>],
                self[md_index<0, 0>]*self[md_index<2, 2>] - self[md_index<0, 2>]*self[md_index<2, 0>],
                -self[md_index<0, 0>]*self[md_index<1, 2>] + self[md_index<0, 2>]*self[md_index<1, 0>],

                self[md_index<1, 0>]*self[md_index<2, 1>] - self[md_index<1, 1>]*self[md_index<2, 0>],
                -self[md_index<0, 0>]*self[md_index<2, 1>] + self[md_index<0, 1>]*self[md_index<2, 0>],
                self[md_index<0, 0>]*self[md_index<1, 1>] - self[md_index<0, 1>]*self[md_index<1, 0>]
            }.scaled_with(self.det());
        } else {
            static_assert(always_false<>::value, "Inverse only implemented for 2x2 and 3x3 tensors");
        }
    }

    //! Return this tensor transposed (only available for 2d tensors)
    template<typename Self>
    constexpr auto transposed(this Self&& self) requires(shape.dimension == 2) {
        auto transposed_terms = self._reduce([&] <std::size_t i, std::size_t j> (md_index_constant<i, j>, auto&& terms) {
            return std::tuple_cat(std::move(terms), std::tuple{self[md_index<j, i>]});
        }, std::tuple{}, md_index_constant_iterator{shape});
        return std::apply([] <typename... Terms> (Terms&&... ts) {
            return backward::tensor_expression{shape, std::forward<Terms>(ts)...};
        }, std::move(transposed_terms));
    }

    //! Cast this into a scalar expression (only available if shape.count == 1)
    constexpr auto as_scalar() const requires(shape.count == 1) {
        return first_type_t<type_list<Es...>>{};
    }

    //! Return the expression representing the squared l2-norm of this expression
    constexpr auto l2_norm_squared() const {
        return dot(*this);
    }

    //! Return the expression representing the l2-norm of this expression
    constexpr auto l2_norm() const {
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


template<typename I, typename A, typename V>
    requires(std::invocable<A, decltype(I::current().with_zeroes()), V>)
inline constexpr auto reduce_over(I index_iterator, const A& action, V&& initial) {
    if constexpr (!I::is_end())
        return reduce_over(index_iterator.next(), action, action(index_iterator.current(), std::forward<V>(initial)));
    else
        return std::forward<V>(initial);
}

template<typename I, typename A>
    requires(std::invocable<A, decltype(I::current())>)
inline constexpr auto concat_tuple_for_each(I index_iterator, const A& action) {
    return reduce_over(index_iterator, [&] (auto i, auto&& tup) {
        return std::tuple_cat(std::move(tup), std::tuple{action(i)});
    }, std::tuple{});
}

//! Evaluate the Jacobian of the given vector expression at the given values
template<typename R = automatic, auto shape, typename... Es, typename... Vs, typename... B>
    requires(shape.is_vector())
inline constexpr auto jacobian_of(
    const tensor_expression<shape, Es...>& tensor,
    const type_list<Vs...>& vars,
    const bindings<B...>& bindings
) {
    auto results_tuple = concat_tuple_for_each(
        md_index_constant_iterator{tensor_expression<shape, Es...>::shape},
        [&] (auto index) { return derivatives_of(tensor[index], vars, bindings); }
    );
    return std::apply([] (auto&&... results) {
        return backward::jacobian{std::move(results)...};
    }, std::move(results_tuple));
}

//! Evaluate the Jacobian of the given vector expression at the given values
template<typename R = automatic, auto shape, typename... Es, typename... B> requires(shape.is_vector())
inline constexpr auto jacobian_of(const tensor_expression<shape, Es...>& tensor, const bindings<B...>& bindings) {
    return jacobian_of(tensor, vars_t<tensor_expression<shape, Es...>>{}, bindings);
}

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

//! A vector expression composed of `var` instances
template<std::size_t N, auto _ = [] () {}>
struct vector
: detail::vec_with<typename detail::vars_n<_, N, type_list<>>::type>::type {
    using base = detail::vec_with<typename detail::vars_n<_, N, type_list<>>::type>::type;
    using base::operator=;
    using variables = typename detail::vars_n<_, N, type_list<>>::type;

    constexpr vector() noexcept = default;
    constexpr vector(md_shape<N>) noexcept {}
    constexpr variables vars() const noexcept { return {}; }
};

template<std::size_t n, auto _ = [] () {}>
vector(md_shape<n>) -> vector<n, _>;

//! Convenience alias for a vector with length N
template<std::size_t N, auto _ = [] () {}>
using vec = vector<N, _>;

template<std::size_t N, auto _>
struct is_expression<vector<N, _>> : is_expression<typename vector<N, _>::base> {};
template<std::size_t N, auto _>
struct is_scalar_expression<vector<N, _>> : is_scalar_expression<typename vector<N, _>::base> {};
template<std::size_t N, auto _>
struct operands<vector<N, _>> : operands<typename vector<N, _>::base> {};

//! A tensor expression composed of `var` instances
template<auto shape, auto _ = [] () {}>
struct tensor
: detail::tensor_with<shape, typename detail::vars_n<_, shape.count, type_list<>>::type>::type {
    using base = detail::tensor_with<shape, typename detail::vars_n<_, shape.count, type_list<>>::type>::type;
    using base::operator=;
    using variables = typename detail::vars_n<_, shape.count, type_list<>>::type;

    template<std::size_t... n>
    constexpr tensor(md_shape<n...>) noexcept {}
    constexpr variables vars() const noexcept { return {}; }
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
