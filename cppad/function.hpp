#pragma once

#include <tuple>
#include <type_traits>
#include <stdexcept>

#include <cppad/type_traits.hpp>
#include <cppad/variable.hpp>
#include <cppad/detail.hpp>

namespace cppad {

template<typename... V>
class IndependentVariables {
    static_assert(
        std::conjunction_v<traits::IsVariable<std::remove_cvref_t<V>>...>,
        "All independent variables must be of type cppad::Variable<...>"
    );
    static_assert(
        std::conjunction_v<std::is_lvalue_reference<V>...>,
        "All independent variables must be passed in by reference and managed externally"
    );
    static_assert(
        UniqueTypes<std::remove_cvref_t<V>...>::value,
        "Each independent variable must be unique"
    );

 public:
    IndependentVariables(V&&... vars)
    : _vars{std::forward<V>(vars)...}
    {}

    constexpr std::size_t size() const {
        return sizeof...(V);
    }

 private:
    std::tuple<V...> _vars;
};

template<typename... V>
IndependentVariables(V&&...) -> IndependentVariables<V...>;

}  // namespace cppad
