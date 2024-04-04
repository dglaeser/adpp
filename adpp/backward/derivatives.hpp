#pragma once

#include <algorithm>
#include <type_traits>
#include <array>
#include <tuple>

#include <adpp/utils.hpp>
#include <adpp/concepts.hpp>

namespace adpp::backward {

template<scalar R, typename... Ts>
    requires(are_unique_v<Ts...>)
struct derivatives : indexed<const Ts&...> {
 private:
     using base = indexed<const Ts&...>;

 public:
    using value_type = R;
    static constexpr std::size_t size = sizeof...(Ts);

    constexpr derivatives() noexcept {
        std::ranges::fill(_values, R{0});
    }

    template<typename Self, typename T> requires(contains_decayed_v<T, Ts...>)
    constexpr std::convertible_to<value_type> decltype(auto) operator[](this Self&& self, const T& t) noexcept {
        return self._values[self.index_of(t)];
    }

    template<typename T> requires(contains_decayed_v<T, Ts...>)
    constexpr std::convertible_to<value_type> decltype(auto) get() const noexcept {
        return _values[base::template index_of<T>()];
    }

    template<typename Self, scalar T>
    constexpr decltype(auto) scaled_with(this Self&& self, T factor) noexcept {
        std::ranges::for_each(self._values, [factor=static_cast<R>(factor)] (auto& v) { v *= factor; });
        return std::forward<Self>(self);
    }

    template<typename Self, scalar T> requires(!std::is_lvalue_reference_v<Self>)
    constexpr decltype(auto) operator+(this Self&& self, derivatives<T, Ts...>&& other) noexcept {
        using result_t = std::common_type_t<R, T>;
        static_assert(is_any_of_v<result_t, R, T>);
        if constexpr (std::is_same_v<result_t, R>) {
            auto& out = self._values;
            const auto& in = other.as_array();
            std::transform(in.begin(), in.end(), out.begin(), out.begin(), std::plus<result_t>{});
            return std::forward<Self>(self);
        } else {
            auto& out = other.as_array();
            const auto& in = self._values;
            std::transform(in.begin(), in.end(), out.begin(), out.begin(), std::plus<result_t>{});
            return std::move(other);
        }
    }

    constexpr const auto& as_array() const noexcept { return _values; }
    constexpr auto& as_array() noexcept { return _values; }

 private:
    std::array<value_type, size> _values;
};


#ifndef DOXYGEN
namespace detail {

    template<typename T>
    struct vars_of;
    template<typename T, typename... Ts>
    struct vars_of<derivatives<T, Ts...>> : std::type_identity<type_list<Ts...>> {};

    template<typename T, typename... Ts>
    struct same_vars;
    template<typename A, typename B, typename... Cs>
    struct same_vars<A, B, Cs...> {
        static constexpr bool value = same_vars<A, B>::value && same_vars<B, Cs...>::value;
    };
    template<typename A, typename B>
    struct same_vars<A, B> {
        using vars_a = typename vars_of<A>::type;
        using vars_b = typename vars_of<B>::type;
        using intersection = unique_t<merged_t<vars_a, vars_b>>;
        static constexpr bool value = vars_a::size == vars_b::size && intersection::size == vars_a::size;
    };
    template<typename T>
    struct same_vars<T> : std::bool_constant<true> {};

    template<typename T>
    concept gradient = is_complete_v<vars_of<T>>;

}  // namespace detail
#endif  // DOXYGEN

template<detail::gradient... gradients>
    requires(detail::same_vars<gradients...>::value)
struct jacobian {
 public:

    // TODO: check if all value_types are the same?

    constexpr jacobian(gradients&&... grads) noexcept
    : _gradients{std::move(grads)...}
    {}

    template<std::size_t i, typename variable>
    constexpr auto operator[](index_constant<i>, const variable& v) const noexcept {
        return std::get<i>(_gradients)[v];
    }

 private:
    std::tuple<gradients...> _gradients;
};

template<typename... gradients>
jacobian(gradients&&...) -> jacobian<std::remove_cvref_t<gradients>...>;

}  // namespace adpp::backward
