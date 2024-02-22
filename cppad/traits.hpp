#pragma once

#include <concepts>
#include <type_traits>

namespace cppad::traits {

template<typename T> struct IsConstant : public std::false_type {};
template<typename T> struct IsVariable : public std::false_type {};
template<typename T> struct IsNamedVariable : public std::false_type {};
template<typename T> struct AsExpression;

}  // namespace cppad::traits
