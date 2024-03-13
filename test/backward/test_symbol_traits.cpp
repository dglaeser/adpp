#include <cstdlib>
#include <type_traits>

#include <boost/ut.hpp>

#include <adpp/backward/symbols.hpp>
#include <adpp/backward/expression.hpp>
#include <adpp/backward/operators.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

using adpp::backward::var;
using adpp::backward::let;
using adpp::backward::val;
using adpp::backward::constant;


int main() {

    "symbols_type_trait"_test = [] () {
        var a;
        let b;
        constant<3> c;
        val d{3};
        auto expr = (a + b)*c + d;
        using symbols = adpp::backward::symbols_t<std::remove_cvref_t<decltype(expr)>>;
        static_assert(adpp::type_list_size_v<symbols> == 4);
        static_assert(adpp::contains_decayed_v<std::remove_cvref_t<decltype(a)>, symbols>);
        static_assert(adpp::contains_decayed_v<std::remove_cvref_t<decltype(b)>, symbols>);
        static_assert(adpp::contains_decayed_v<std::remove_cvref_t<decltype(c)>, symbols>);
        static_assert(adpp::contains_decayed_v<std::remove_cvref_t<decltype(d)>, symbols>);
    };

    "unbound_symbols_type_trait"_test = [] () {
        var a;
        let b;
        constant<3> c;
        val d{3};
        auto expr = (a + b)*c + d;
        using unbound = adpp::backward::unbound_symbols_t<std::remove_cvref_t<decltype(expr)>>;
        static_assert(adpp::type_list_size_v<unbound> == 2);
        static_assert(adpp::contains_decayed_v<std::remove_cvref_t<decltype(a)>, unbound>);
        static_assert(adpp::contains_decayed_v<std::remove_cvref_t<decltype(b)>, unbound>);
    };

    "vars_type_trait"_test = [] () {
        var a;
        let b;
        constant<3> c;
        val d{3};
        auto expr = (a + b)*c + d;
        using unbound = adpp::backward::vars_t<std::remove_cvref_t<decltype(expr)>>;
        static_assert(adpp::type_list_size_v<unbound> == 1);
        static_assert(adpp::contains_decayed_v<std::remove_cvref_t<decltype(a)>, unbound>);
    };

    return EXIT_SUCCESS;
}
