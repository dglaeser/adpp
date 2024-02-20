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

}  // namespace cppad
