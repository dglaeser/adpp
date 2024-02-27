#pragma once

#include <type_traits>
#include <cppad/concepts.hpp>

namespace cppad {
namespace dtype {

struct any {};
struct real {};
struct integral {};

template<typename T, typename Arg>
struct accepts;

template<typename Arg> struct accepts<any, Arg> : public std::true_type {};
template<typename Arg> struct accepts<real, Arg> : public std::bool_constant<std::is_floating_point_v<Arg>> {};
template<typename Arg> struct accepts<integral, Arg> : public std::bool_constant<std::is_integral_v<Arg>> {};

}  // namespace dtype

namespace concepts {

template<typename T, typename Arg>
concept accepts = is_complete_v<dtype::accepts<T, Arg>> and dtype::accepts<T, Arg>::value;

}  // namespace concepts
}  // namespace cppad
