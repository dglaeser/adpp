// SPDX-FileCopyrightText: 2024 Dennis Gläser <dennis.glaeser@iws.uni-stuttgart.de>
// SPDX-License-Identifier: MIT
/*!
 * \file
 * \ingroup Backward
 * \brief Classes and functions for evaluating expressions.
 */
#pragma once

#include <utility>
#include <ostream>
#include <type_traits>

#include <adpp/concepts.hpp>
#include <adpp/backward/concepts.hpp>
#include <adpp/backward/bindings.hpp>

namespace adpp::backward {

//! \addtogroup Backward
//! \{

//! Stores an expression and exposes a function-like interface
template<term E>
struct function {
 public:
    constexpr function(E&& e) noexcept
    : _e{std::move(e)}
    {}

    //! Evaluate the expression at the given values
    template<typename... B>
    constexpr decltype(auto) operator()(const bindings<B...>& values) const {
        return evaluate(values);
    }

    //! Evaluate the expression at the given values
    template<typename... B>
        requires(sizeof...(B) != 1 or !is_binding_v<B...>)
    constexpr decltype(auto) operator()(B&&... values) const {
        return evaluate(at(std::forward<B>(values)...));
    }

    //! Evaluate the expression at the given values
    template<typename... B>
        requires(evaluatable_with<E, bindings<B...>>)
    constexpr decltype(auto) evaluate(const bindings<B...>& values) const {
        return _e.evaluate(values);
    }

    //! Back-propagate derivatives
    template<scalar R, typename... Args>
    constexpr decltype(auto) back_propagate(Args&&... args) const {
        return _e.template back_propagate<R>(std::forward<Args>(args)...);
    }

    //! Differentiate the underlying expression w.r.t. the given variable
    template<typename V>
    constexpr decltype(auto) differentiate(const V& var) const {
        return _e.differentiate(var);
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

//! Evaluate the given expression at the given values
template<typename E, typename... B>
    requires(evaluatable_with<E, bindings<B...>>)
inline constexpr auto evaluate(E&& e, const bindings<B...>& b) {
    return e.evaluate(b);
}

//! \} group Backward

}  // namespace adpp
