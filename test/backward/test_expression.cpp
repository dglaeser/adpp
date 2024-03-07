#include <cstdlib>
#include <functional>
#include <type_traits>

#include <boost/ut.hpp>

#include <adpp/backward/symbols.hpp>
#include <adpp/backward/expression.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

int main() {

    "operator"_test = [] () {
        adpp::backward::var x;
        adpp::backward::var y;
        adpp::backward::bindings bindings{x = 3, y = 2};

        using X = std::remove_cvref_t<decltype(x)>;
        using Y = std::remove_cvref_t<decltype(y)>;
        using E1 = adpp::backward::expression<std::multiplies<void>, X, Y>;
        using E2 = adpp::backward::expression<std::plus<void>, E1, X>;

        expect(eq(E2{}(bindings), 9));
    };

    return EXIT_SUCCESS;
}
