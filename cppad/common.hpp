#pragma once

#include <type_traits>
#include <concepts>
#include <utility>
#include <memory>


namespace cppad {

#ifndef DOXYGEN
namespace detail {

    struct null_expression {
        constexpr double value() const { return 0.0; }
        template<typename T>
        constexpr double partial(T&&) const { return 0.0; }
    };

    template<typename T, std::size_t s = sizeof(T)>
    std::false_type is_incomplete(T*);
    std::true_type is_incomplete(...);

}  // end namespace detail
#endif  // DOXYGEN

template<typename T>
struct is_complete {
    static constexpr bool value = !decltype(detail::is_incomplete(std::declval<T*>()))::value;
};

template<typename T>
inline constexpr bool is_complete_v = is_complete<T>::value;

template<typename T>
struct is_ownable : public std::bool_constant<!std::is_lvalue_reference_v<T>> {};

template<typename T>
inline constexpr bool is_ownable_v = is_ownable<T>::value;


namespace traits {

template<typename T> struct is_constant : public std::false_type {};
template<typename T> struct is_variable : public std::false_type {};
template<typename T> struct is_named_variable : public std::false_type {};
template<typename T> struct as_expression;
template<typename T> struct undefined_value;

}  // namespace traits


namespace concepts {

template<typename T>
concept arithmetic = std::floating_point<std::remove_cvref_t<T>> or std::integral<std::remove_cvref_t<T>>;

template<typename T>
concept expression = requires(const T& t) {
    { t.value() } -> arithmetic;
    { t.partial(detail::null_expression{}) } -> arithmetic;
};

template<typename T>
concept into_expression = expression<T> or requires(const T& t) {
    requires is_complete_v<traits::as_expression<std::remove_cvref_t<T>>>;
    { traits::as_expression<std::remove_cvref_t<T>>::get(t) } -> expression;
};

template<typename A, typename B>
concept same_decay_t_as = std::same_as<std::decay_t<A>, std::decay_t<B>>;

template<typename T>
concept ownable = is_ownable<T>::value;

}  // namespace concepts


namespace traits {

template<concepts::expression E>
struct as_expression<E> {
    template<typename _E> requires(concepts::same_decay_t_as<E, _E>)
    static constexpr decltype(auto) get(_E&& e) noexcept {
        return std::forward<_E>(e);
    }
};

template<concepts::arithmetic T>
struct undefined_value<T> {
    static constexpr T value = std::numeric_limits<T>::max();
};

}  // namespace traits


template<concepts::into_expression E>
constexpr decltype(auto) as_expression(E&& e) noexcept {
    return traits::as_expression<std::remove_cvref_t<E>>::get(std::forward<E>(e));
}

template<concepts::arithmetic T>
inline constexpr T undefined_value = traits::undefined_value<T>::value;

template<typename T>
inline constexpr bool is_variable_v = traits::is_variable<std::remove_cvref_t<T>>::value;

template<typename T>
inline constexpr bool is_named_variable_v = traits::is_named_variable<std::remove_cvref_t<T>>::value;

template<typename T>
inline constexpr bool is_constant_v = traits::is_constant<std::remove_cvref_t<T>>::value;

template<typename T>
class storage {
    using stored = std::conditional_t<std::is_lvalue_reference_v<T>, T, std::remove_cvref_t<T>>;

public:
    template<typename _T> requires(std::convertible_to<_T, stored>)
    constexpr explicit storage(_T&& value) noexcept
    : _value{std::forward<_T>(value)}
    {}

    template<typename S> requires(!std::is_lvalue_reference_v<S>)
    constexpr T&& get(this S&& self) noexcept {
        return std::move(self._value);
    }

    template<typename S>
    constexpr auto& get(this S& self) noexcept {
        if constexpr (std::is_const_v<S>)
            return std::as_const(self._value);
        else
            return self._value;
    }

private:
    stored _value;
};

template<typename T>
storage(T&&) -> storage<T>;


template<typename A, typename B>
constexpr bool is_same_object(A&& a, B&& b) {
    if constexpr (std::same_as<std::remove_cvref_t<A>, std::remove_cvref_t<B>>)
        return std::addressof(a) == std::addressof(b);
    return false;
}

}  // namespace cppad
