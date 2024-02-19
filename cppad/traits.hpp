#pragma once

#include <type_traits>

namespace cppad::traits {

template<typename T> struct IsConstant : public std::false_type {};

}  // namespace cppad::traits
