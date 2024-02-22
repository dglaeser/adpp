#pragma once

#include <type_traits>

#include <cppad/concepts.hpp>
#include <cppad/constant.hpp>
#include <cppad/detail.hpp>
#include <cppad/traits.hpp>


namespace cppad {

template<concepts::Arithmetic V, auto _ = [] () {}>
class Variable : public Constant<V> {
    using Parent = Constant<V>;

 public:
    using Parent::Parent;

    template<concepts::Expression E>
    constexpr double partial(E&& e) const {
        return this->partial_to(std::forward<E>(e), [] (auto&& e) {
            return 0.0;
        });
    }
};

template<typename T>
Variable(T&&) -> Variable<std::remove_cvref_t<T>>;

template<typename T, auto _ = [] () {}>
inline constexpr auto var(T&& value) {
    return Variable<std::remove_cvref_t<T>, _>{std::forward<T>(value)};
}

namespace traits {

template<typename T, auto _>
struct IsVariable<Variable<T, _>> : public std::true_type {};

}  // namespace traits
}  // namespace cppad
