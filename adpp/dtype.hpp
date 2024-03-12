#pragma once

#include <type_traits>
#include <adpp/concepts.hpp>

namespace adpp {
namespace dtype {

struct any {};
struct real {};
struct integral {};

template<typename T, typename Arg>
struct accepts;

template<typename Arg>
struct accepts<any, Arg> : public std::true_type {};
template<typename Arg>
struct accepts<real, Arg> : public std::bool_constant<std::is_floating_point_v<std::remove_cvref_t<Arg>>> {};
template<typename Arg>
struct accepts<integral, Arg> : public std::bool_constant<std::is_integral_v<std::remove_cvref_t<Arg>>> {};
template<arithmetic T, typename Arg>
struct accepts<T, Arg> : public std::is_same<T, std::remove_cvref_t<Arg>> {};

}  // namespace dtype

template<typename T, typename Arg>
concept accepts = is_complete_v<dtype::accepts<T, Arg>> and dtype::accepts<T, Arg>::value;

}  // namespace adpp
