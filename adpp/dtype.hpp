#pragma once

#include <array>
#include <type_traits>
#include <initializer_list>

#include <adpp/concepts.hpp>

namespace adpp {
namespace dtype {

struct any {};
struct real {};
struct integral {};

template<typename dtype, std::size_t N>
struct array {};

template<typename T, typename Arg>
struct accepts;

template<typename Arg> struct accepts<any, Arg> : public std::true_type {};
template<typename Arg> struct accepts<real, Arg> : public std::bool_constant<std::is_floating_point_v<std::remove_cvref_t<Arg>>> {};
template<typename Arg> struct accepts<integral, Arg> : public std::bool_constant<std::is_integral_v<std::remove_cvref_t<Arg>>> {};
template<concepts::arithmetic T, typename Arg> struct accepts<T, Arg> : public std::is_same<T, std::remove_cvref_t<Arg>> {};

template<std::size_t N, typename T>
struct accepts<array<any, N>, std::array<T, N>> : public std::true_type {};
template<std::size_t N, typename T>
struct accepts<array<any, N>, std::initializer_list<T>> : public std::true_type {};

}  // namespace dtype

namespace concepts {

template<typename T, typename Arg>
concept accepts = is_complete_v<dtype::accepts<T, Arg>> and dtype::accepts<T, Arg>::value;

}  // namespace concepts
}  // namespace adpp
