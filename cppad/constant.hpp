#pragma once

#include <utility>
#include <type_traits>

#include <cppad/concepts.hpp>
#include <cppad/expression.hpp>
#include <cppad/detail.hpp>

namespace cppad {

template<concepts::Arithmetic V>
class Constant : public ExpressionBase {
 public:
    template<typename T>
    constexpr explicit Constant(T&& t)
    : _storage{std::forward<T>(t)}
    {}

    template<typename Self>
    constexpr decltype(auto) value(this Self&& self) {
        return self._storage.get();
    }

    template<concepts::Expression E>
    constexpr double partial(E&& e) const {
        return 0.0;
    }

    template<typename Self, concepts::Arithmetic T>
    void set(this Self& self, T value) {
        self._storage.get() = value;
    }

    template<typename Self, concepts::Arithmetic T>
    Self& operator*=(this Self& self, T value) {
        self._storage.get() *= value;
        return self;
    }

    template<typename Self, concepts::Arithmetic T>
    Self& operator/=(this Self& self, T&& value) {
        self._storage.get() /= value;
        return self;
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

namespace traits {

template<typename T>
struct IsConstant<Constant<T>> : public std::true_type {};

template<concepts::Arithmetic T>
struct AsExpression<T> {
    template<typename _T> requires(std::same_as<T, std::remove_cvref_t<_T>>)
    static constexpr auto get(_T&& value) {
        return constant(std::forward<_T>(value));
    }
};

}  // namespace traits
}  // namespace cppad
