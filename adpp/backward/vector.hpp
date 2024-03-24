#pragma once

#include <array>
#include <tuple>
#include <ranges>
#include <type_traits>
#include <functional>

#include <adpp/common.hpp>
#include <adpp/type_traits.hpp>
#include <adpp/backward/concepts.hpp>
#include <adpp/backward/expression.hpp>
#include <adpp/backward/bindings.hpp>
#include <adpp/backward/symbols.hpp>

namespace adpp::backward {

template<typename S, typename V>
    requires(static_vec_n<std::remove_cvref_t<V>, S::shape.number_of_elements>)
struct tensor_value_binder : value_binder<S, V>{
    static constexpr std::size_t number_of_sub_binders = S::shape.number_of_elements;

    using value_binder<S, V>::value_binder;

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
            return typename std::remove_cvref_t<V>::value_type{this->unwrap()[i]};
        else
            return this->unwrap()[i];
    }
};

template<typename E, typename S>
tensor_value_binder(E&&, S&&) -> tensor_value_binder<std::remove_cvref_t<E>, S>;

template<typename T, auto shape>
class md_array {
 public:
    constexpr md_array() = default;

    template<typename Self, std::size_t... i>
    constexpr decltype(auto) operator[](this Self&& self, const md_index_constant<i...>& idx) {
        static_assert(md_index_constant<i...>::as_flat_index(shape).value < shape.number_of_elements);
        return self._values[idx.as_flat_index(shape)];
    }

    template<typename Self, std::integral... I>
        requires(sizeof...(I) == shape.size)
    constexpr decltype(auto) operator[](this Self&& self, I&&... indices) {
        return self._values[flat_index(shape, std::forward<I>(indices)...)];
    }

    template<typename Self, std::integral I>
        requires(shape.size == 1 or (shape.size == 2 && shape.last_axis_size == 1))
    constexpr decltype(auto) operator[](this Self&& self, const I& index) {
        return self._values[index];
    }

    template<typename Self> constexpr auto begin(this Self&& self) { return self._values.begin(); }
    template<typename Self> constexpr auto end(this Self&& self) { return self._values.end(); }

 private:
    std::array<T, shape.number_of_elements> _values;
};

template<auto md_shape, term... Es>
    requires(sizeof...(Es) > 0 and sizeof...(Es) == md_shape.number_of_elements and are_unique_v<Es...>)
struct tensor_expression : bindable, indexed<Es...> {
    static constexpr std::size_t size = md_shape.number_of_elements;
    static constexpr auto shape = md_shape;
    using sub_expressions = type_list<Es...>;

    constexpr tensor_expression() = default;
    constexpr tensor_expression(const Es&...) {}
    template<std::size_t... n>
    constexpr tensor_expression(adpp::md_shape<n...> d, const Es&...) requires(d == shape) {}

    template<typename Self, typename V>
        requires(static_vec_n<std::remove_cvref_t<V>, size>)
    constexpr auto bind(this Self&& self, V&& value) noexcept {
        return tensor_value_binder{std::forward<Self>(self), std::forward<V>(value)};
    }

    template<typename Self, typename V>
        requires(static_vec_n<std::remove_cvref_t<V>, size>)
    constexpr auto operator=(this Self&& self, V&& values) noexcept {
        return self.bind(std::forward<V>(values));
    }

    template<typename Self, typename T>
    constexpr auto operator=(this Self&& self, T (&&values)[size]) noexcept {
        return self.bind(to_array<T, size>::from(std::move(values)));
    }

    template<typename... B>
    constexpr auto evaluate(const bindings<B...>& b) const noexcept {
        md_array<typename bindings<B...>::common_value_type, shape> result;
        _visit([&] <auto... i> (md_index_constant<i...> md_index) {
            result[md_index] = (*this)[md_index].evaluate(b);
        }, md_index_constant_iterator{shape});
        return result;
    }

    template<typename... V>
    constexpr void export_to(std::ostream& out, const bindings<V...>& name_bindings) const {
        out << "[";
        bool first = true;
        _visit([&] <auto... i> (md_index_constant<i...> idx) {
            if constexpr (is_vector() && idx.at(ic<0>) > 0) out << ", ";
            if constexpr (!is_vector() && idx.last() > 0) out << ", ";
            else if (!is_vector() && idx.last() == 0 && !first) out << " // ";
            (*this)[idx].export_to(out, name_bindings);
            first = false;
        }, md_index_constant_iterator{shape});
        out << "]";
    }

    template<std::size_t... i>
    constexpr auto operator[](const md_index_constant<i...>& idx) const {
        static_assert(md_index_constant<i...>::as_flat_index(shape).value < size);
        return this->make(idx.as_flat_index(shape));
    }

    template<std::size_t i>
        requires(shape.size == 1 or (shape.size == 2 && shape.last_axis_size == 1))
    constexpr auto operator[](const index_constant<i>& idx) const {
        return this->make(idx);
    }

    template<into_term V>
        requires(is_scalar_expression_v<std::remove_cvref_t<V>>)
    constexpr auto operator*(V&& value) const {
        return scaled_with(std::forward<V>(value));
    }

    template<auto other_shape, typename... T>
    constexpr auto operator*(const tensor_expression<other_shape, T...>& other) const {
        return dot(other);
    }

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

    template<auto other_shape, typename... T>
        requires(tensor_expression::is_vector() and shape.at(ic<0>) == other_shape.at(ic<0>))
    constexpr auto dot(const tensor_expression<other_shape, T...>& other) const {
        return _reduce([&] (auto i, auto&& e) {
            if constexpr (is_zero_constant_v<decltype(e)>)
                return (*this)[i]*other[i];
            else
                return std::move(e) + (*this)[i]*other[i];
        }, cval<0>, md_index_constant_iterator{shape});
    }

    template<auto other_shape, typename... T>
        requires(!tensor_expression::is_vector() and shape.last_axis_size == other_shape.at(ic<0>))
    constexpr auto dot(const tensor_expression<other_shape, T...>& other) const {
        static constexpr auto my_dim = shape.size;
        using head = typename split_at<my_dim - 1, typename decltype(shape)::as_list>::head;
        using tail = typename split_at<1, typename decltype(other_shape)::as_list>::tail;

        static constexpr auto new_shape = adpp::md_shape{head{} + tail{}};
        auto result_tuple = _reduce([&] <auto... i> (md_index_constant<i...> md_i, auto&& terms) {
            constexpr auto i_head = typename split_at<my_dim - 1, typename md_index_constant<i...>::as_list>::head{};
            constexpr auto i_tail = typename split_at<my_dim - 1, typename md_index_constant<i...>::as_list>::tail{};
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
                    }, cval<0>, md_index_constant_iterator{adpp::md_shape<shape.last_axis_size>{}})
                }
            );
        }, std::tuple{}, md_index_constant_iterator{new_shape});
        return std::apply([] <typename... Terms> (Terms&&... ts) {
            return backward::tensor_expression{new_shape, std::forward<Terms>(ts)...};
        }, std::move(result_tuple));
    }

    constexpr auto l2_norm_squared() const {
        return dot(*this);
    }

    constexpr auto l2_norm() const {
        return sqrt(l2_norm_squared());
    }

    static constexpr bool is_vector() {
        return shape.size == 2 and shape.last_axis_size == 1;
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
            return _reduce(action, action(index_iterator.index(), std::move(value)), index_iterator.next());
        else
            return std::move(value);
    }

    template<typename V, typename I>
    constexpr void _visit(const V& visitor, const I& index_iterator) const {
        if constexpr (!I::is_end()) {
            visitor(index_iterator.index());
            _visit(visitor, index_iterator.next());
        }
    }
};

template<std::size_t... n, term... Es>
    requires(md_shape<n...>::number_of_elements == sizeof...(Es))
tensor_expression(md_shape<n...>, Es&&...) -> tensor_expression<md_shape<n...>{}, std::remove_cvref_t<Es>...>;


template<auto shape, typename... T>
struct is_expression<tensor_expression<shape, T...>> : std::true_type {};
template<auto shape, typename... T>
struct is_scalar_expression<tensor_expression<shape, T...>> : std::false_type {};
template<auto shape, typename... T>
struct operands<tensor_expression<shape, T...>> : std::type_identity<type_list<T...>> {};


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

    template<std::size_t, auto _ = [] () {}>
    constexpr auto new_lambda() { return _; }

    template<std::size_t N, typename... T>
    struct vars_n;
    template<std::size_t N, typename... T> requires(sizeof...(T) < N)
    struct vars_n<N, type_list<T...>> : vars_n<N, type_list<var<dtype::any, [] () {}>, T...>> {};
    template<std::size_t N, typename... T> requires(sizeof...(T) == N)
    struct vars_n<N, type_list<T...>> : std::type_identity<type_list<T...>> {};

    template<typename T>
    struct vec_with;
    template<typename... T>
    struct vec_with<type_list<T...>> : std::type_identity<tensor_expression<md_shape<sizeof...(T), 1>{}, T...>> {};

    template<auto shape, typename T>
    struct tensor_with;
    template<auto shape, typename... T>
    struct tensor_with<shape, type_list<T...>> : std::type_identity<tensor_expression<shape, T...>> {};

}  // namespace detail
#endif  // DOXYGEN

template<std::size_t N>
using vec = typename detail::vec_with<typename detail::vars_n<N, type_list<>>::type>::type;

template<std::size_t... N>
using tensor = typename detail::tensor_with<
    md_shape<N...>{},
    typename detail::vars_n<md_shape<N...>{}.number_of_elements, type_list<>
>::type>::type;

}  // namespace adpp::backward
