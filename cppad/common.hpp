#pragma once

#include <type_traits>
#include <concepts>
#include <utility>
#include <memory>


namespace cppad {

#ifndef DOXYGEN
namespace detail {

    struct test_expression {
        constexpr double value() const { return 0.0; }
        template<typename T>
        constexpr double partial(T&&) const { return 0.0; }
    };

    template<typename T, std::size_t s = sizeof(T)>
    std::false_type is_incomplete(T*);
    std::true_type is_incomplete(...);

    template<typename T>
    constexpr std::remove_cvref_t<T> as_copy(T&&) {
        return std::declval<std::remove_cvref_t<T>>();
    }

}  // end namespace detail
#endif  // DOXYGEN

template<typename T>
struct is_complete : public std::bool_constant<!decltype(detail::is_incomplete(std::declval<T*>()))::value> {};

template<typename T>
inline constexpr bool is_complete_v = is_complete<T>::value;

template<typename T>
struct is_ownable : public std::bool_constant<!std::is_lvalue_reference_v<T>> {};

template<typename T>
inline constexpr bool is_ownable_v = is_ownable<T>::value;

template<std::size_t i>
using index_constant = std::integral_constant<std::size_t, i>;

template<typename... T>
struct are_unique;

template<typename T1, typename T2, typename... Ts>
struct are_unique<T1, T2, Ts...> {
    static constexpr bool value =
        are_unique<T1, T2>::value &&
        are_unique<T1, Ts...>::value &&
        are_unique<T2, Ts...>::value;
};

template<typename T1, typename T2>
struct are_unique<T1, T2> : public std::bool_constant<!std::is_same_v<T1, T2>> {};
template<typename T>
struct are_unique<T> : public std::true_type {};
template<>
struct are_unique<> : public std::true_type {};

template<typename... Ts>
inline constexpr bool are_unique_v = are_unique<Ts...>::value;

namespace traits {

template<typename T> struct is_constant : public std::false_type {};
template<typename T> struct is_variable : public std::false_type {};
template<typename T> struct is_named_variable : public std::false_type {};
template<typename T> struct expression_value;
template<typename T> struct as_expression;
template<typename T> struct undefined_value;
template<typename T> struct format;
template<typename T> struct derivative;

}  // namespace traits


namespace concepts {

template<typename T>
concept arithmetic = std::floating_point<std::remove_cvref_t<T>> or std::integral<std::remove_cvref_t<T>>;

template<typename T>
concept expression = requires(const T& t) {
    { t.value() } -> arithmetic;
    { t.partial(detail::test_expression{}) } -> arithmetic;
};

template<typename T>
concept into_expression = requires(const T& t) {
    requires is_complete_v<traits::as_expression<std::remove_cvref_t<T>>>;
    {
        traits::as_expression<std::remove_cvref_t<T>>::get(t)
    };  // -> expression does not work because of self-reference issues in places where we use this
};

template<typename T>
concept unary_operator = std::default_initializable<T> and requires(const T& t) {
    { detail::as_copy(T{}(double{})) } -> arithmetic;
};

template<typename T>
concept binary_operator = std::default_initializable<T> and requires(const T& t) {
    { detail::as_copy(T{}(double{}, double{})) } -> arithmetic;
};


template<typename T, typename E, typename V>
concept derivable_unary_operator
    = unary_operator<T>
    and is_complete_v<traits::derivative<T>>
    and requires(const T& t, const E& e, const V& variable) {
        { detail::as_copy(traits::derivative<T>::value(e, variable)) } -> arithmetic;
        {
            traits::derivative<T>::expression(e, variable)
        };  // -> expression does not work because of self-reference issues in places where we use this
    };

template<typename T, typename A, typename B, typename V>
concept derivable_binary_operator
    = binary_operator<T>
    and expression<A> and expression<B>
    and is_complete_v<traits::derivative<T>>
    and requires(const T& t, const A& a, const B& b, const V& variable) {
        { detail::as_copy(traits::derivative<T>::value(a, b, variable)) } -> arithmetic;
        {
            traits::derivative<T>::expression(a, b, variable)
        };  // -> expression does not work because of self-reference issues in places where we use this
    };

template<typename T, typename A>
concept unary_operator_formatter = requires(const A& a) {
    { T::format(a) } -> std::convertible_to<const char*>;
};

template<typename T, typename A, typename B>
concept binary_operator_formatter = requires(const A& a, const B& b) {
    { T::format(a, b) } -> std::convertible_to<const char*>;
};

template<typename A, typename B>
concept same_decay_t_as = std::same_as<std::decay_t<A>, std::decay_t<B>>;

template<typename T>
concept ownable = is_ownable<T>::value;

}  // namespace concepts


namespace traits {

template<typename E>
    requires(!concepts::arithmetic<E>)
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

template<concepts::expression E>
struct expression_value<E> : public std::type_identity<std::remove_cvref_t<decltype(std::declval<const E&>().value())>> {};

}  // namespace traits


template<concepts::into_expression E>
constexpr decltype(auto) as_expression(E&& e) noexcept {
    return traits::as_expression<std::remove_cvref_t<E>>::get(std::forward<E>(e));
}

template<concepts::expression E>
using expression_value_t = typename traits::expression_value<E>::type;

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
