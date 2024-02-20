#pragma once

#include <concepts>
#include <utility>
#include <type_traits>
#include <cmath>

#include <cppad/concepts.hpp>
#include <cppad/traits.hpp>
#include <cppad/detail.hpp>

namespace cppad {

// forward declarations
template<concepts::Expression A, concepts::Expression B> class Plus;
template<concepts::Expression A, concepts::Expression B> class Times;
template<concepts::Expression E> class Exponential;

struct ExpressionBase {
    template<typename Self, concepts::IntoExpression Other>
    constexpr auto operator+(this Self&& self, Other&& other) noexcept {
        using OtherExpression = decltype(as_expression(std::forward<Other>(other)));
        return Plus<Self, OtherExpression>{
            std::forward<Self>(self),
            as_expression(std::forward<Other>(other))
        };
    }

    template<typename Self, concepts::IntoExpression Other>
    constexpr auto operator*(this Self&& self, Other&& other) noexcept {
        using OtherExpression = decltype(as_expression(std::forward<Other>(other)));
        return Times<Self, OtherExpression>{
            std::forward<Self>(self),
            as_expression(std::forward<Other>(other))
        };
    }

    template<typename Self>
    constexpr auto exp(this Self&& self) {
        return Exponential<Self>{std::forward<Self>(self)};
    }

 protected:
    template<typename Self, concepts::Expression E, std::invocable<E> Partial>
        requires(concepts::Arithmetic<std::invoke_result_t<Partial, E>>)
    constexpr double partial_to(this Self&& self, E&& e, Partial&& partial) {
        static_assert(
            !traits::IsConstant<std::remove_cvref_t<E>>::value,
            "Derivative w.r.t. a constant requested"
        );
        return detail::is_same_object(self, e) ? 1.0 : partial(std::forward<E>(e));
    }
};

}  // namespace cppad

namespace std {

template<cppad::concepts::Expression E>
constexpr auto exp(E&& e) {
    return std::forward<E>(e).exp();
}

}  // namespace std
