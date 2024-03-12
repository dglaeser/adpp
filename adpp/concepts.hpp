#pragma once

#include <limits>
#include <utility>
#include <concepts>

#include <adpp/type_traits.hpp>

namespace adpp::concepts {

template<typename T>
concept arithmetic = std::floating_point<std::remove_cvref_t<T>> or std::integral<std::remove_cvref_t<T>>;

template<typename A, typename B>
concept same_decay_t_as = std::same_as<std::decay_t<A>, std::decay_t<B>>;

template<typename A, typename B>
concept same_remove_cvref_t_as = std::same_as<std::remove_cvref_t<A>, std::remove_cvref_t<B>>;

template<typename T>
concept ownable = is_ownable<T>::value;

}  // namespace adpp::concepts
