#pragma once

#include <utility>
#include <ostream>
#include <type_traits>

#include <cppad/concepts.hpp>
#include <cppad/variadic_accessor.hpp>
#include <cppad/backward/concepts.hpp>
#include <cppad/backward/bindings.hpp>
#include <cppad/backward/expression_tree.hpp>

namespace cppad::backward {

#ifndef DOXYGEN
namespace detail {

    template<typename T, typename... B>
    struct bindings_contain {
        static constexpr bool value = bindings<B...>::template contains_bindings_for<T>;
    };

    template<typename T, typename... B>
    struct bindings_contain_all_leaves;

    template<typename... T, typename... B>
    struct bindings_contain_all_leaves<std::tuple<T...>, B...> {
        static constexpr bool value = std::conjunction_v<bindings_contain<T, B...>...>;
    };

    template<typename E, typename... B>
    concept bindings_for = traversable_expression<std::remove_cvref_t<E>> and bindings_contain_all_leaves<
        std::remove_cvref_t<decltype(
            leaf_symbols_of(std::declval<const E&>())
        )>,
        B...
    >::value;

    template<typename T, typename... B>
    concept expression = requires(const T& t, B&&... b) {
        { t.evaluate_at(bindings{std::forward<B>(b)...}) };
    };

}  // namespace detail
#endif  // DOXYGEN

template<typename E>
    requires(detail::traversable_expression<E> or traits::is_leaf_expression<E>::value)
struct expression {
 public:
    constexpr expression(E&& e) noexcept
    : _e{std::move(e)}
    {}

    template<typename... B>
    constexpr decltype(auto) operator()(B&&... bindings) const {
        return evaluate_at(at(std::forward<B>(bindings)...));
    }

    template<typename B>
    constexpr decltype(auto) evaluate_at(const B& bindings) const {
        return _e.evaluate_at(bindings);
    }

    template<typename... Args>
    constexpr decltype(auto) back_propagate(Args&&... args) const {
        return _e.back_propagate(std::forward<Args>(args)...);
    }

    template<typename V>
    constexpr decltype(auto) differentiate_wrt(const V& var) const {
        return _e.differentiate_wrt(var);
    }

    template<typename... Args>
    constexpr std::ostream& stream(std::ostream& s, Args&&... args) const {
        return _e.stream(s, std::forward<Args>(args)...);
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
    requires(detail::bindings_for<E, B...> and detail::expression<E, B...>)
inline constexpr auto evaluate(E&& e, const bindings<B...>& b) {
    return e.evaluate_at(b);
}

}  // namespace cppad
