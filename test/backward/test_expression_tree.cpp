#include <cstdlib>

#include <boost/ut.hpp>

#include <adpp/common.hpp>
#include <adpp/backward/symbols.hpp>
#include <adpp/backward/expression_tree.hpp>

using boost::ut::operator""_test;

using adpp::backward::var;
using adpp::backward::let;

int main() {

    "symbols_of_var"_test = [] () {
        static constexpr var a;

        using leaves_t = adpp::backward::symbols_t<decltype(a)>;
        static_assert(adpp::is_any_of_v<std::remove_cvref_t<decltype(a)>, leaves_t>);
        static_assert(adpp::type_size_v<leaves_t> == 1);

        constexpr auto leaves = symbols_of(a);
        static_assert(adpp::type_size_v<std::remove_cvref_t<decltype(leaves)>> == 1);
        static_assert(std::is_same_v<
            std::remove_cvref_t<adpp::first_type_t<std::remove_cvref_t<decltype(leaves)>>>,
            std::remove_cvref_t<decltype(a)>
        >);
    };

    "symbols_of_let"_test = [] () {
        static constexpr let a;

        using leaves_t = adpp::backward::symbols_t<decltype(a)>;
        static_assert(adpp::is_any_of_v<std::remove_cvref_t<decltype(a)>, leaves_t>);
        static_assert(adpp::type_size_v<leaves_t> == 1);

        constexpr auto leaves = symbols_of(a);
        static_assert(adpp::type_size_v<std::remove_cvref_t<decltype(leaves)>> == 1);
        static_assert(std::is_same_v<
            std::remove_cvref_t<adpp::first_type_t<std::remove_cvref_t<decltype(leaves)>>>,
            std::remove_cvref_t<decltype(a)>
        >);
    };

    "symbols_of"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        static constexpr let c;
        constexpr auto formula = (a + b)*c - 1.0;

        using leaves_t = adpp::backward::symbols_t<decltype(formula)>;
        static_assert(adpp::is_any_of_v<std::remove_cvref_t<decltype(a)>, leaves_t>);
        static_assert(adpp::is_any_of_v<std::remove_cvref_t<decltype(b)>, leaves_t>);
        static_assert(adpp::is_any_of_v<std::remove_cvref_t<decltype(c)>, leaves_t>);
        static_assert(adpp::type_size_v<leaves_t> == 4);
        static_assert(adpp::are_unique_v<leaves_t>);

        using leaves_unbound_t = adpp::backward::unbound_symbols_t<decltype(formula)>;
        static_assert(adpp::is_any_of_v<std::remove_cvref_t<decltype(a)>, leaves_unbound_t>);
        static_assert(adpp::is_any_of_v<std::remove_cvref_t<decltype(b)>, leaves_unbound_t>);
        static_assert(adpp::is_any_of_v<std::remove_cvref_t<decltype(c)>, leaves_unbound_t>);
        static_assert(adpp::type_size_v<leaves_unbound_t> == 3);
        static_assert(adpp::are_unique_v<leaves_unbound_t>);
    };

    "vars_of"_test = [] () {
        static constexpr var a;
        static constexpr var b;
        static constexpr let c;
        constexpr auto formula = (a + b)*b*c - 1.0;

        using leaves_t = adpp::backward::vars_t<decltype(formula)>;
        static_assert(adpp::is_any_of_v<std::remove_cvref_t<decltype(a)>, leaves_t>);
        static_assert(adpp::is_any_of_v<std::remove_cvref_t<decltype(b)>, leaves_t>);
        static_assert(!adpp::is_any_of_v<std::remove_cvref_t<decltype(c)>, leaves_t>);
        static_assert(adpp::type_size_v<leaves_t> == 2);
        static_assert(adpp::are_unique_v<leaves_t>);
    };

    return EXIT_SUCCESS;
}
