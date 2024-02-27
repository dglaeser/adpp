#pragma once

#include <type_traits>

namespace dtype {

struct any {};

template<typename T, typename Arg>
struct accepts;

template<typename Arg>
struct accepts<any, Arg> : public std::true_type {};

}  // namespace dtype
