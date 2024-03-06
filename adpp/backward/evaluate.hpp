#pragma once

#include <utility>
#include <ostream>
#include <type_traits>

#include <adpp/concepts.hpp>
#include <adpp/variadic_accessor.hpp>
#include <adpp/backward/concepts.hpp>
#include <adpp/backward/bindings.hpp>
#include <adpp/backward/expression_tree.hpp>

namespace adpp::backward {

#ifndef DOXYGEN
namespace detail {

    template<typename T, typename... B>
    struct bindings_contain {
        static constexpr bool value = bindings<B...>::template contains_bindings_for<T>;
    };

    template<typename T, typename... B>
    struct bindings_contain_all_leaves;

    template<typename... T, typename... B>
    struct bindings_contain_all_leaves<type_list<T...>, B...> {
        static constexpr bool value = std::conjunction_v<bindings_contain<T, B...>...>;
    };

    template<typename E, typename... B>
    concept bindings_for = traversable_expression<std::remove_cvref_t<E>>
        and bindings_contain_all_leaves<leaf_symbols_t<E>, B...>::value;

    template<typename T, typename... B>
    concept expression = requires(const T& t, B&&... b) {
        { t.evaluate_at(bindings{std::forward<B>(b)...}) };
    };

}  // namespace detail
#endif  // DOXYGEN

template<typename E>
    requires(detail::traversable_expression<E> or traits::is_leaf_expression<E>::value)
struct function {
 public:
    constexpr function(E&& e) noexcept
    : _e{std::move(e)}
    {}

    template<typename... B>
    constexpr decltype(auto) operator()(B&&... values) const {
        return evaluate_at(at(std::forward<B>(values)...));
    }

    template<typename... B>
    constexpr decltype(auto) operator()(const bindings<B...>& values) const {
        return evaluate_at(values);
    }

    template<typename... B>
    constexpr decltype(auto) evaluate_at(const bindings<B...>& values) const {
        return _e.evaluate_at(values);
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
function(E&&) -> function<std::remove_cvref_t<E>>;

namespace traits {

template<typename E>
struct is_leaf_expression<function<E>>
: public is_leaf_expression<std::remove_cvref_t<E>> {};

template<typename E>
    requires(!traits::is_leaf_expression<E>::value)
struct sub_expressions<function<E>> {
    using operands = typename sub_expressions<std::remove_cvref_t<E>>::operands;

    static constexpr decltype(auto) get(const function<E>& e) {
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

}  // namespace adpp
