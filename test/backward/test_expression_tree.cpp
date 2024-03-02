#include <cstdlib>

#include <boost/ut.hpp>

#include <adpp/common.hpp>
#include <adpp/backward/symbols.hpp>
#include <adpp/backward/expression_tree.hpp>

using boost::ut::operator""_test;

using adpp::backward::var;
using adpp::backward::let;

template<typename A, typename B>
inline constexpr bool is_same_object(A&& a, B&& b) {
    if constexpr (std::same_as<std::remove_cvref_t<A>, std::remove_cvref_t<B>>)
        return std::addressof(a) == std::addressof(b);
    return false;
}

int main() {

    "leaf_expressions_of_var"_test = [] () {
        static constexpr var a;
        constexpr auto leaves = leaf_symbols_of(a);
        static_assert(std::tuple_size_v<decltype(leaves)> == 1);
        static_assert(is_same_object(a, std::get<0>(leaves)));
        static_assert(std::is_same_v<
            std::remove_cvref_t<decltype(std::get<0>(leaves))>,
            std::remove_cvref_t<decltype(a)>
        >);
    };

    "leaf_expressions_of_let"_test = [] () {
        static constexpr let a;
        constexpr auto leaves = leaf_symbols_of(a);
        static_assert(std::tuple_size_v<decltype(leaves)> == 1);
        static_assert(is_same_object(a, std::get<0>(leaves)));
        static_assert(std::is_same_v<
            std::remove_cvref_t<decltype(std::get<0>(leaves))>,
            std::remove_cvref_t<decltype(a)>
        >);
    };

    "leaf_expressions_of"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        static constexpr let c;
        constexpr auto formula = (a + b)*c - 1.0;
        constexpr auto leaves = leaf_symbols_of(formula);
        static_assert(std::tuple_size_v<decltype(leaves)> == 3);
        static_assert(is_same_object(std::get<const std::remove_cvref_t<decltype(a)>&>(leaves), a));
        static_assert(is_same_object(std::get<const std::remove_cvref_t<decltype(b)>&>(leaves), b));
        static_assert(is_same_object(std::get<const std::remove_cvref_t<decltype(c)>&>(leaves), c));
    };

    "leaf_variables_of"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        static constexpr let c;
        constexpr auto formula = (a + b)*c - 1.0;
        constexpr auto leaves = leaf_variables_of(formula);
        static_assert(std::tuple_size_v<decltype(leaves)> == 2);
        static_assert(is_same_object(std::get<const std::remove_cvref_t<decltype(a)>&>(leaves), a));
        static_assert(is_same_object(std::get<const std::remove_cvref_t<decltype(b)>&>(leaves), b));
    };

    return EXIT_SUCCESS;
}
