#pragma once

#include <utility>
#include <concepts>
#include <type_traits>

#include <adpp/utils.hpp>
#include <adpp/concepts.hpp>
#include <adpp/backward/concepts.hpp>

namespace adpp::backward {

#ifndef DOXYGEN
namespace detail {

    template<typename T> struct is_value_binder : std::bool_constant<binder<T>> {};

    template<typename... B>
    inline constexpr bool are_binders = std::conjunction_v<is_value_binder<std::remove_cvref_t<B>>...>;

}  // namespace detail
#endif  // DOXYGEN

template<typename... B>
    requires(are_unique_v<B...> and detail::are_binders<B...>)
struct bindings : variadic_accessor<B...> {
 private:
    using base = variadic_accessor<B...>;

    template<typename T>
    using symbol_type_of = typename std::remove_cvref_t<T>::symbol_type;

    template<typename T, typename... Bs>
    struct binder_type_for;

    template<typename T, typename B0, typename... Bs>
    struct binder_type_for<T, B0, Bs...> {
        using type = std::conditional_t<
            same_remove_cvref_t_as<T, symbol_type_of<B0>>, B0, typename binder_type_for<T, Bs...>::type
        >;
    };

    template<typename T>
    struct binder_type_for<T> {
        using type = void;
    };

    template<typename T>
    struct is_contained : std::disjunction<std::is_same<std::remove_cvref_t<T>, symbol_type_of<B>>...> {};

    template<typename T> requires(sizeof...(B) > 0 and is_contained<T>::value)
    using binder_type = binder_type_for<T, B...>::type;

 public:
    template<typename... T>
    static constexpr bool contains_bindings_for = std::conjunction_v<is_contained<T>...>;

    // TODO: common_value_t?
    using common_value_type = std::common_type_t<typename std::remove_cvref_t<B>::value_type...>;

    constexpr bindings(B... binders) noexcept
    : base(std::forward<B>(binders)...)
    {}

    using base::get;
    template<typename Self, typename T>
        requires(contains_bindings_for<T>)
    constexpr decltype(auto) get(this Self&& self) noexcept {
        return self.get(self.template index_of<binder_type<T>>()).unwrap();
    }

    template<typename Self, typename T>
        requires(contains_bindings_for<T>)
    constexpr decltype(auto) operator[](this Self&& self, const T&) noexcept {
        return self.template get<Self, T>();
    }
};

template<>
struct bindings<> {
    template<typename... T>
    static constexpr bool contains_bindings_for = false;

    using common_value_type = double;
};

template<typename... B>
bindings(B&&...) -> bindings<B...>;


template<typename T>
struct is_binding : std::false_type {};
template<typename... B>
struct is_binding<bindings<B...>> : std::true_type {};
template<typename T>
inline constexpr bool is_binding_v = is_binding<T>::value;

template<term E, typename B>
class bound_expression {
 public:
    constexpr bound_expression(E e, B b)
    : _expression{std::forward<E>(e)}
    , _bindings{std::forward<B>(b)}
    {}

    constexpr decltype(auto) evaluate() const requires(expression_for<E, B>) {
        return _expression.get().evaluate(_bindings.get());
    }

    template<scalar R, typename Self, typename... V>
    constexpr auto back_propagate(this Self&& self, const type_list<V...>& vars) {
        auto [value, derivs] = self._expression.get().template back_propagate<R>(self._bindings.get(), vars);
        if constexpr (contains_decayed_v<Self, V...>)
            derivs[self] = 1.0;
        return std::make_pair(std::move(value), std::move(derivs));
    }

    template<typename Self, typename V>
    constexpr auto differentiate(this Self&& self, const type_list<V>& var) {
        if constexpr (std::is_lvalue_reference_v<Self>)
            return _make(self._expression.get().differentiate(var), self._bindings.get());
        else
            return _make(
                std::move(self._expression).get().differentiate(var),
                std::move(self._bindings).get()
            );
    }

    constexpr void export_to(std::ostream& out) const {
        _expression.get().export_to(out, _bindings.get());
    }

    friend constexpr std::ostream& operator<<(std::ostream& out, const bound_expression& e) {
        e.export_to(out);
        return out;
    }

 private:
    template<typename _E, typename _B>
    static constexpr auto _make(_E&& e, _B&& b) {
        return bound_expression<_E, _B>{std::forward<_E>(e), std::forward<_B>(b)};
    }

    storage<E> _expression;
    storage<B> _bindings;
};

template<typename E, typename B>
bound_expression(E&&, B&&) -> bound_expression<E, B>;


#ifndef DOXYGEN
namespace detail {

    template<typename T>
    concept binder_collection = requires {
        T::number_of_sub_binders;
    };

    template<typename... Ts, typename T>
    constexpr auto concat_with(std::tuple<Ts...>&& tuple, T&& t) {
        if constexpr (std::is_lvalue_reference_v<T>)
            return std::tuple_cat(std::move(tuple), std::forward_as_tuple(std::forward<T>(t)));
        else
            return std::tuple_cat(std::move(tuple), std::tuple<std::remove_cvref_t<T>>{std::move(t)});
    }

    template<typename... Ts, typename F, typename... R> requires(sizeof...(R) > 0)
    constexpr auto concat_with(std::tuple<Ts...>&& tuple, F&& first, R&&... rest) {
        return concat_with(
            concat_with(std::move(tuple), std::forward<F>(first)),
            std::forward<R>(rest)...
        );
    }

    template<typename C, typename F> requires(binder_collection<std::remove_cvref_t<C>>)
    constexpr auto unpack_binder_collection(C&& collection, const F& callback) {
        return std::apply([&] <typename... SB> (SB&&... sub_binders) {
            return callback(std::forward<SB>(sub_binders)...);
        }, std::forward<C>(collection).sub_binders());
    }

    template<typename F, typename... U, typename T0, typename... Ts>
    constexpr auto unpack_binders(const F& callback, std::tuple<U...>&& unpacked, T0&& t0, Ts&&... ts) {
        static constexpr bool stop_recursion = sizeof...(Ts) == 0;
        if constexpr (binder_collection<std::remove_cvref_t<T0>>) {
            return unpack_binder_collection(std::forward<T0>(t0), [&] <typename... SB> (SB&&... sub_binders) {
                auto concatenated = concat_with(std::move(unpacked), std::forward<SB>(sub_binders)...);
                if constexpr (stop_recursion)
                    return callback(std::move(concatenated));
                else
                    return unpack_binders(callback, std::move(concatenated), std::forward<Ts>(ts)...);
            });
        } else {
            auto concatenated = concat_with(std::move(unpacked), std::forward<T0>(t0));
            if constexpr (stop_recursion)
                return callback(std::move(concatenated));
            else
                return unpack_binders(callback, std::move(concatenated), std::forward<Ts>(ts)...);
        }
    }

    template<typename F>
    constexpr auto unpack_binders(const F& callback, std::tuple<>&& t) {
        return callback(std::move(t));
    }

}  // namespace detail
#endif  // DOXYGEN


template<typename... B> requires(detail::are_binders<B...>)
inline constexpr auto bind(B&&... b) {
    return detail::unpack_binders([] <typename T> (T&& binder_tuple) {
        return std::apply([] <typename... SB> (SB&&... sub_binders) {
            return bindings{std::forward<SB>(sub_binders)...};
        }, std::forward<T>(binder_tuple));
    }, std::tuple{}, std::forward<B>(b)...);
}

template<typename... B> requires(detail::are_binders<B...>)
inline constexpr auto at(B&&... b) {
    return adpp::backward::bind(std::forward<B>(b)...);
}

template<typename... B> requires(detail::are_binders<B...>)
inline constexpr auto with(B&&... b) {
    return adpp::backward::bind(std::forward<B>(b)...);
}

template<typename... B> requires(detail::are_binders<B...>)
inline constexpr auto where(B&&... b) {
    return adpp::backward::bind(std::forward<B>(b)...);
}


struct bindable {
    template<typename Self, typename... Bs>
    constexpr auto with(this Self&& self, Bs&&... binders) noexcept {
        return bound_expression{std::forward<Self>(self), adpp::backward::bind(std::forward<Bs>(binders)...)};
    }
};

}  // namespace adpp::backward

#include <format>
#include <sstream>

template<typename E, typename B>
struct std::formatter<adpp::backward::bound_expression<E, B>> {
    // todo: precision formatter?

    template<typename parse_ctx>
    constexpr parse_ctx::iterator parse(parse_ctx& ctx) {
        auto it = ctx.begin();
        if (it == ctx.end())
            return it;
        if (*it != '}')
            throw std::format_error("adpp::backward::bound_expression does not support format args.");
        return it;
    }

    template<typename fmt_ctx>
    fmt_ctx::iterator format(const adpp::backward::bound_expression<E, B>& e, fmt_ctx& ctx) const {
        std::ostringstream out;
        e.export_to(out);
        return std::ranges::copy(std::move(out).str(), ctx.out()).out;
    }
};
