#pragma once

#include <utility>
#include <ostream>
#include <type_traits>

#include <adpp/concepts.hpp>
#include <adpp/backward/concepts.hpp>
#include <adpp/backward/bindings.hpp>

namespace adpp::backward {

template<term E>
struct function {
 public:
    constexpr function(E&& e) noexcept
    : _e{std::move(e)}
    {}

    template<typename... B>
    constexpr decltype(auto) operator()(const bindings<B...>& values) const {
        return evaluate(values);
    }

    template<typename... B>
        requires(sizeof...(B) != 1 or !is_binding_v<B...>)
    constexpr decltype(auto) operator()(B&&... values) const {
        return evaluate(at(std::forward<B>(values)...));
    }

    template<typename... B>
        requires(expression_for<E, bindings<B...>>)
    constexpr decltype(auto) evaluate(const bindings<B...>& values) const {
        return _e.evaluate(values);
    }

    template<scalar R, typename... Args>
    constexpr decltype(auto) back_propagate(Args&&... args) const {
        return _e.template back_propagate<R>(std::forward<Args>(args)...);
    }

    template<typename V>
    constexpr decltype(auto) differentiate(const V& var) const {
        return _e.differentiate(var);
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

template<typename E>
struct operands<function<E>> : operands<E> {};

template<typename E, typename... B>
    requires(expression_for<E, bindings<B...>>)
inline constexpr auto evaluate(E&& e, const bindings<B...>& b) {
    return e.evaluate(b);
}

}  // namespace adpp
