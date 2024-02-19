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

}  // namespace cppad
