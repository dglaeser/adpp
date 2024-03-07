#pragma once

#include <utility>
#include <concepts>
#include <type_traits>

#include <adpp/common.hpp>
#include <adpp/concepts.hpp>
#include <adpp/backward/concepts.hpp>

namespace adpp::backward {

#ifndef DOXYGEN
namespace detail {

    template<typename... B>
    inline constexpr bool are_binders = std::conjunction_v<traits::is_value_binder<std::remove_cvref_t<B>>...>;

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
            concepts::same_decay_t_as<T, symbol_type_of<B0>>,
            B0,
            typename binder_type_for<T, Bs...>::type
        >;
    };

    template<typename T>
    struct binder_type_for<T> {
        using type = void;
    };

 public:
    template<typename T>
    static constexpr bool contains_bindings_for = std::disjunction_v<
        std::is_same<std::remove_cvref_t<T>, symbol_type_of<B>>...
    >;

    template<typename T> requires(sizeof...(B) > 0 and contains_bindings_for<T>)
    using binder_type = binder_type_for<T, B...>::type;

    constexpr bindings(B... binders) noexcept
    : base(std::forward<B>(binders)...)
    {}

    template<typename Self, typename T>
        requires(contains_bindings_for<T>)
    constexpr decltype(auto) operator[](this Self&& self, const T&) noexcept {
        return self.get(self.template index_of<binder_type<T>>()).unwrap();
    }
};

template<typename... B>
bindings(B&&...) -> bindings<B...>;

template<typename... B>
    requires(detail::are_binders<B...>)
inline constexpr auto at(B&&... b) {
    return bindings{std::forward<B>(b)...};
}

template<typename... B>
    requires(detail::are_binders<B...>)
inline constexpr auto with(B&&... b) {
    return bindings{std::forward<B>(b)...};
}

template<typename... B>
    requires(detail::are_binders<B...>)
inline constexpr auto bind(B&&... b) {
    return bindings{std::forward<B>(b)...};
}

}  // namespace adpp
