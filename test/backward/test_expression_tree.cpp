#include <cstdlib>

#include <boost/ut.hpp>

#include <adpp/common.hpp>
#include <adpp/backward/symbols.hpp>
#include <adpp/backward/expression_tree.hpp>

using boost::ut::operator""_test;

using adpp::backward::var;
using adpp::backward::let;

int main() {

    "leaf_symbols_of_var"_test = [] () {
        static constexpr var a;

        using leaves_t = adpp::backward::leaf_symbols_t<decltype(a)>;
        static_assert(adpp::is_any_of_v<std::remove_cvref_t<decltype(a)>, leaves_t>);

        constexpr auto leaves = leaf_symbols_of(a);
        static_assert(std::tuple_size_v<decltype(leaves)> == 1);
        static_assert(adpp::is_same_object(a, std::get<0>(leaves)));
        static_assert(std::is_same_v<
            std::remove_cvref_t<decltype(std::get<0>(leaves))>,
            std::remove_cvref_t<decltype(a)>
        >);
    };

    "leaf_symbols_of_let"_test = [] () {
        static constexpr let a;

        using leaves_t = adpp::backward::leaf_symbols_t<decltype(a)>;
        static_assert(adpp::is_any_of_v<std::remove_cvref_t<decltype(a)>, leaves_t>);

        constexpr auto leaves = leaf_symbols_of(a);
        static_assert(std::tuple_size_v<decltype(leaves)> == 1);
        static_assert(adpp::is_same_object(a, std::get<0>(leaves)));
        static_assert(std::is_same_v<
            std::remove_cvref_t<decltype(std::get<0>(leaves))>,
            std::remove_cvref_t<decltype(a)>
        >);
    };

    "leaf_symbols_of"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        static constexpr let c;
        constexpr auto formula = (a + b)*c - 1.0;

        using leaves_t = adpp::backward::leaf_symbols_t<decltype(formula)>;
        static_assert(adpp::is_any_of_v<std::remove_cvref_t<decltype(a)>, leaves_t>);
        static_assert(adpp::is_any_of_v<std::remove_cvref_t<decltype(b)>, leaves_t>);
        static_assert(adpp::is_any_of_v<std::remove_cvref_t<decltype(c)>, leaves_t>);

        constexpr auto leaves = leaf_symbols_of(formula);
        static_assert(std::tuple_size_v<decltype(leaves)> == 3);
        static_assert(adpp::is_same_object(std::get<const std::remove_cvref_t<decltype(a)>&>(leaves), a));
        static_assert(adpp::is_same_object(std::get<const std::remove_cvref_t<decltype(b)>&>(leaves), b));
        static_assert(adpp::is_same_object(std::get<const std::remove_cvref_t<decltype(c)>&>(leaves), c));
    };

    "leaf_vars_of"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        static constexpr let c;
        constexpr auto formula = (a + b)*c - 1.0;

        using leaves_t = adpp::backward::leaf_vars_t<decltype(formula)>;
        static_assert(adpp::is_any_of_v<std::remove_cvref_t<decltype(a)>, leaves_t>);
        static_assert(adpp::is_any_of_v<std::remove_cvref_t<decltype(b)>, leaves_t>);
        static_assert(!adpp::is_any_of_v<std::remove_cvref_t<decltype(c)>, leaves_t>);

        constexpr auto leaves = leaf_variables_of(formula);
        static_assert(std::tuple_size_v<decltype(leaves)> == 2);
        static_assert(adpp::is_same_object(std::get<const std::remove_cvref_t<decltype(a)>&>(leaves), a));
        static_assert(adpp::is_same_object(std::get<const std::remove_cvref_t<decltype(b)>&>(leaves), b));
    };

    return EXIT_SUCCESS;
}
