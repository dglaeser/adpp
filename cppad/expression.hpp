#pragma once

#include <concepts>
#include <utility>
#include <memory>
#include <type_traits>

#include <cppad/concepts.hpp>

namespace cppad {

#ifndef DOXYGEN
namespace detail {

    template<typename A, typename B>
    constexpr bool is_same_object(A&& a, B&& b) {
        if constexpr (std::same_as<std::remove_cvref_t<A>, std::remove_cvref_t<B>>)
            return std::addressof(a) == std::addressof(b);
        return false;
    }

}  // namespace detail
#endif  // DOXYGEN

// forward declarations
template<concepts::Expression A, concepts::Expression B> class Plus;
template<concepts::Expression A, concepts::Expression B> class Times;

struct ExpressionBase {
    template<typename Self, concepts::Expression Other>
    constexpr auto operator+(this Self&& self, Other&& other) {
        return Plus<Self, Other>{
            std::forward<Self>(self),
            std::forward<Other>(other)
        };
    }

    template<typename Self, concepts::Expression Other>
    constexpr auto operator*(this Self&& self, Other&& other) {
        return Times<Self, Other>{
            std::forward<Self>(self),
            std::forward<Other>(other)
        };
    }

 protected:
    template<typename Self, concepts::Expression E, std::invocable<E> Partial>
        requires(concepts::Arithmetic<std::invoke_result_t<Partial, E>>)
    double partial_to(this Self&& self, E&& e, Partial&& partial) {
        return detail::is_same_object(self, e) ? 1.0 : partial(std::forward<E>(e));
    }
};

}  // namespace cppad
