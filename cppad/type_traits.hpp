#pragma once

namespace cppad {

#ifndef DOXYGEN
namespace detail {

    template<typename T, std::size_t s = sizeof(T)>
    std::false_type is_incomplete(T*);
    std::true_type is_incomplete(...);

}  // end namespace detail
#endif  // DOXYGEN

template<typename T>
inline constexpr bool is_complete = !decltype(detail::is_incomplete(std::declval<T*>()))::value;

struct EmptyExpression {
    constexpr double value() const {
        return 0.0;
    }

    template<typename T>
    constexpr double partial(T&&) const {
        return 0.0;
    }
};

template<typename... T>
struct UniqueTypes;

template<typename T1, typename T2, typename... Ts>
struct UniqueTypes<T1, T2, Ts...> {
    static constexpr bool value =
        UniqueTypes<T1, T2>::value &&
        UniqueTypes<T1, Ts...>::value &&
        UniqueTypes<T2, Ts...>::value;
};

template<typename T1, typename T2>
struct UniqueTypes<T1, T2> : public std::bool_constant<!std::is_same_v<T1, T2>> {};
template<typename T>
struct UniqueTypes<T> : public std::true_type {};
template<>
struct UniqueTypes<> : public std::true_type {};

}  // namespace cppad
