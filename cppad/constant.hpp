#pragma once

#include <utility>

#include <cppad/concepts.hpp>
#include <cppad/expression.hpp>
#include <cppad/detail.hpp>

namespace cppad {

template<concepts::Arithmetic V>
class Constant : public ExpressionBase {
 public:
    template<typename T>
    explicit Constant(T&& t)
    : _storage{std::forward<T>(t)}
    {}

    template<typename Self>
    decltype(auto) value(this Self&& self) {
        return self._storage.get();
    }

    template<concepts::Expression E>
    double partial(E&& e) const {
        return 0.0;
    }

 private:
    detail::Storage<V> _storage;
};

template<typename T>
Constant(T&&) -> Constant<std::remove_cvref_t<T>>;

template<typename T>
    requires(concepts::Arithmetic<std::remove_cvref_t<T>>)
constexpr auto constant(T&& value) {
    return Constant<std::remove_cvref_t<T>>{std::forward<T>(value)};
}

}  // namespace cppad
