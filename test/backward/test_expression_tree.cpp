#include <cstdlib>

#include <boost/ut.hpp>

#include <cppad/common.hpp>
#include <cppad/backward/symbols.hpp>
#include <cppad/backward/expression_tree.hpp>

using boost::ut::operator""_test;

using cppad::backward::var;
using cppad::backward::let;

int main(int argc, char** argv) {

    "leaf_expressions_of_var"_test = [] () {
        static constexpr var a;
        constexpr auto leaves = leaf_symbols_of(a);
        static_assert(std::tuple_size_v<decltype(leaves)> == 1);
        static_assert(cppad::is_same_object(a, std::get<0>(leaves)));
        static_assert(std::is_same_v<
            std::remove_cvref_t<decltype(std::get<0>(leaves))>,
            std::remove_cvref_t<decltype(a)>
        >);
    };

    "leaf_expressions_of_let"_test = [] () {
        static constexpr let a;
        constexpr auto leaves = leaf_symbols_of(a);
        static_assert(std::tuple_size_v<decltype(leaves)> == 1);
        static_assert(cppad::is_same_object(a, std::get<0>(leaves)));
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
        static_assert(cppad::is_same_object(std::get<const std::remove_cvref_t<decltype(a)>&>(leaves), a));
        static_assert(cppad::is_same_object(std::get<const std::remove_cvref_t<decltype(b)>&>(leaves), b));
        static_assert(cppad::is_same_object(std::get<const std::remove_cvref_t<decltype(c)>&>(leaves), c));
    };

    "leaf_variables_of"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        static constexpr let c;
        constexpr auto formula = (a + b)*c - 1.0;
        constexpr auto leaves = leaf_variables_of(formula);
        static_assert(std::tuple_size_v<decltype(leaves)> == 2);
        static_assert(cppad::is_same_object(std::get<const std::remove_cvref_t<decltype(a)>&>(leaves), a));
        static_assert(cppad::is_same_object(std::get<const std::remove_cvref_t<decltype(b)>&>(leaves), b));
    };

    return EXIT_SUCCESS;
}
