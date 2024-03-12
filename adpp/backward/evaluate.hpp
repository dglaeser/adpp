#pragma once

#include <utility>
#include <ostream>
#include <type_traits>

#include <adpp/concepts.hpp>
#include <adpp/backward/concepts.hpp>
#include <adpp/backward/bindings.hpp>
#include <adpp/backward/expression.hpp>

namespace adpp::backward {

#ifndef DOXYGEN
namespace detail {

    template<typename T, typename... B>
    struct bindings_contain {
        static constexpr bool value = bindings<B...>::template contains_bindings_for<T>;
    };

    template<typename T, typename... B>
    struct bindings_contain_all_symbols;

    template<typename... T, typename... B>
    struct bindings_contain_all_symbols<type_list<T...>, B...> {
        static constexpr bool value = std::conjunction_v<bindings_contain<T, B...>...>;
    };

    template<typename E, typename... B>
    concept bindings_for = traversable_expression<std::remove_cvref_t<E>>
        and bindings_contain_all_symbols<unbound_symbols_t<E>, B...>::value;

    template<typename... B> struct is_binding : std::false_type {};
    template<typename... B> struct is_binding<bindings<B...>> : std::true_type {};

}  // namespace detail
#endif  // DOXYGEN

template<typename E>
    requires(is_expression_v<E>)
struct function {
 public:
    constexpr function(E&& e) noexcept
    : _e{std::move(e)}
    {}

    template<typename... B>
        requires(detail::bindings_for<E, B...>)
    constexpr decltype(auto) operator()(const bindings<B...>& values) const {
        return _e(values);
    }

    template<typename... B>
        requires(!detail::is_binding<B...>::value)
    constexpr decltype(auto) operator()(B&&... values) const {
        return (*this)(at(std::forward<B>(values)...));
    }

    template<concepts::arithmetic R, typename... Args>
    constexpr decltype(auto) back_propagate(Args&&... args) const {
        return _e.template back_propagate<R>(std::forward<Args>(args)...);
    }

    template<typename V>
    constexpr decltype(auto) differentiate_wrt(const V& var) const {
        return _e.differentiate_wrt(var);
    }

    template<typename... Args>
    constexpr std::ostream& stream(std::ostream& s, Args&&... args) const {
        return _e.stream(s, std::forward<Args>(args)...);
    }

 private:
    E _e;
};

template<typename E>
function(E&&) -> function<std::remove_cvref_t<E>>;

template<typename E>
struct is_expression<function<E>> : std::true_type {};

template<typename E, typename... B>
    requires(detail::bindings_for<E, B...>)
inline constexpr auto evaluate(E&& e, const bindings<B...>& b) {
    return e(b);
}

}  // namespace adpp
