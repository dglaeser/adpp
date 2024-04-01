#pragma once

#include <array>
#include <limits>
#include <utility>
#include <concepts>

#include <adpp/utils.hpp>

namespace adpp {

template<typename T>
concept scalar = std::floating_point<std::remove_cvref_t<T>> or std::integral<std::remove_cvref_t<T>>;

template<typename T>
concept indexable = is_complete_v<value_type<T>> and requires(const T& t) {
    typename value_type<T>::type;
    { t[std::size_t{0}] } -> std::convertible_to<typename value_type<T>::type>;
};

template<typename T>
concept static_vec = is_complete_v<size_of<std::remove_cvref_t<T>>> and indexable<T>;

template<typename T, std::size_t N>
concept static_vec_n = static_vec<T> and size_of_v<std::remove_cvref_t<T>> == N;

template<typename A, typename B>
concept same_remove_cvref_t_as = std::same_as<std::remove_cvref_t<A>, std::remove_cvref_t<B>>;

}  // namespace adpp
