#pragma once

#include <type_traits>
#include <concepts>
#include <utility>
#include <memory>

#include <adpp/type_traits.hpp>

namespace adpp {

template<unsigned int i>
struct order : public std::integral_constant<unsigned int, i> {};

inline constexpr order<1> first_order;
inline constexpr order<2> second_order;
inline constexpr order<3> third_order;

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


#ifndef DOXYGEN
namespace detail {

    template<std::size_t cur, std::size_t max, typename Op, typename T>
    inline constexpr decltype(auto) index_reduce(Op&& op, T&& current) {
        if constexpr (cur < max) {
            const auto process = [&] <typename N> (N&& next) {
                return index_reduce<cur+1, max>(std::forward<Op>(op), std::forward<N>(next));
            };
            return process(op(index_constant<cur>{}, std::forward<T>(current)));
        } else {
            return std::forward<T>(current);
        }
    }

}  // namespace detail
#endif  // DOXYGEN

// TODO: Should be in detail? provided lambda requires forwarding operands, otherwise
//       operators take args be reference which eventually will dangle...
template<std::size_t begin, std::size_t end, typename Operator, typename T>
inline constexpr decltype(auto) recursive_reduce(index_range<begin, end>, Operator&& op, T&& initial) {
    if constexpr (begin < end)
        return detail::index_reduce<begin, end>(std::forward<Operator>(op), std::forward<T>(initial));
    else
        return std::forward<T>(initial);
}

}  // namespace adpp
