#pragma once

#include <type_traits>

#include <cppad/concepts.hpp>
#include <cppad/constant.hpp>
#include <cppad/detail.hpp>
#include <cppad/traits.hpp>


namespace cppad {

template<typename V> requires(traits::IsVariable<std::remove_cvref_t<V>>::value)
class NamedVariable {
    static_assert(std::is_lvalue_reference_v<V>);

 public:
    constexpr NamedVariable(V v, const char* name) noexcept
    : _storage{std::forward<V>(v)}
    , _name{name}
    {}

    constexpr const char* name() const {
        return _name;
    }

    template<typename _V> requires(traits::IsVariable<std::remove_cvref_t<_V>>::value)
    constexpr bool operator==(_V&& var) const {
        return detail::is_same_object(var, _storage.get());
    }

 private:
    detail::Storage<V> _storage;
    const char* _name;
};

template<concepts::Arithmetic V, auto _ = [] () {}>
class Variable : public Constant<V> {
    using Parent = Constant<V>;

 public:
    using Parent::Parent;

    template<typename Self>
    constexpr auto operator|=(this Self&& self, const char* name) noexcept {
        return NamedVariable<Self>{std::forward<Self>(self), name};
    }

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

template<typename T>
struct IsNamedVariable<NamedVariable<T>> : public std::true_type {};

}  // namespace traits
}  // namespace cppad
