#pragma once

#include <utility>
#include <type_traits>

#include <cppad/concepts.hpp>
#include <cppad/variadic_accessor.hpp>
#include <cppad/backward/concepts.hpp>
#include <cppad/backward/bindings.hpp>

namespace cppad::backward {

#ifndef DOXYGEN
namespace detail {

    template<typename T, typename... B>
    concept expression = requires(const T& t, B&&... b) {
        { t.evaluate_at(bindings{std::forward<B>(b)...}) };
    };

}  // namespace detail
#endif  // DOXYGEN

template<typename E>
struct expression {
 public:
    constexpr expression(E&& e) noexcept
    : _e{std::move(e)}
    {}

    template<typename B>
    constexpr decltype(auto) evaluate_at(const B& bindings) const {
        return _e.evaluate_at(bindings);
    }

    operator const E&() const {
        return _e;
    }

 private:
    E _e;
};

template<typename E>
expression(E&&) -> expression<std::remove_cvref_t<E>>;

namespace traits {

template<typename E>
struct is_leaf_expression<expression<E>>
: public is_leaf_expression<std::remove_cvref_t<E>> {};

template<typename E>
    requires(!traits::is_leaf_expression<E>::value)
struct sub_expressions<expression<E>> {
    static constexpr decltype(auto) get(const expression<E>& e) {
        return sub_expressions<std::remove_cvref_t<E>>::get(
            static_cast<const std::remove_cvref_t<E>&>(e)
        );
    }
};

}  // namespace traits

template<typename E, typename... B>
    requires(detail::expression<E, B...>)
inline constexpr auto evaluate(E&& e, const bindings<B...>& b) {
    return e.evaluate_at(b);
}

}  // namespace cppad
