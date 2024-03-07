#pragma once

#include <algorithm>
#include <type_traits>

#include <adpp/common.hpp>
#include <adpp/concepts.hpp>

namespace adpp::backward {

template<concepts::arithmetic R, typename... Ts>
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

    template<typename Self, typename T> requires(contains_decay_v<T, Ts...>)
    constexpr std::convertible_to<value_type> decltype(auto) operator[](this Self&& self, const T& t) noexcept {
        return self._values[self.index_of(t)];
    }

    template<typename T> requires(contains_decay_v<T, Ts...>)
    constexpr std::convertible_to<value_type> decltype(auto) get() const noexcept {
        return _values[base::template index_of<T>()];
    }

    template<typename Self, concepts::arithmetic T>
    constexpr decltype(auto) scaled_with(this Self&& self, T factor) noexcept {
        std::ranges::for_each(self._values, [factor] (auto& v) { v *= factor; });
        return std::forward<Self>(self);
    }

    template<typename Self, concepts::arithmetic T>
    constexpr decltype(auto) operator+(this Self&& self, derivatives<T, Ts...>&& other) noexcept {
        using result_type = std::common_type_t<R, T>;
        if constexpr (std::is_same_v<result_type, R>) {
            const auto& other_values = other.as_array();
            std::transform(
                other_values.begin(), other_values.end(),
                self._values.begin(), self._values.begin(),
                std::plus{}
            );
            return std::forward<Self>(self);
        } else {
            auto& other_values = other.as_array();
            std::transform(
                self._values.begin(), self._values.end(),
                other_values.begin(), other_values.begin(),
                std::plus{}
            );
            return std::move(other);
        }
    }

    constexpr const auto& as_array() const {
        return _values;
    }

    constexpr auto& as_array() {
        return _values;
    }

 private:
    std::array<value_type, size> _values;
};

}  // namespace adpp::backward