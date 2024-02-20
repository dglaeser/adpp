#pragma once

#include <tuple>
#include <type_traits>
#include <stdexcept>

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
 public:
    IndependentVariables(V&&... vars) : _vars{std::forward<V>(vars)...} {
        if constexpr (sizeof...(V) > 0) {
            const bool has_duplicate = std::apply([] <typename... VI> (const auto& v0, const VI&... vi) {
                return std::apply([&] <typename... VS> (const VS&... vs) {
                    return (detail::is_same_object(v0, vs) || ...);
                }, std::forward_as_tuple(vi...));
            }, _vars);
            if (has_duplicate)
                throw std::runtime_error("Duplicate independent variables provided!");
        }
    }

    constexpr std::size_t size() const {
        return sizeof...(V);
    }

 private:
    std::tuple<V...> _vars;
};

template<typename... V>
IndependentVariables(V&&...) -> IndependentVariables<V...>;

}  // namespace cppad
