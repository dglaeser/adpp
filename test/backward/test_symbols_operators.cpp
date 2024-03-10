#include <cstdlib>

#include <boost/ut.hpp>

#include <adpp/backward/symbols.hpp>

// TODO: This include should not be necessary
#include <adpp/backward/expression.hpp>

using boost::ut::operator""_test;

using adpp::backward::var;
using adpp::backward::let;
using adpp::backward::val;


int main() {

    "var_times_scalar"_test = [] () {
        var a;
        {
            [[maybe_unused]] auto op_right = a*val<1>;
            [[maybe_unused]] auto op_left = val<1>*a;
        }
        {
            [[maybe_unused]] auto op_right = a + val<1>;
            [[maybe_unused]] auto op_left = val<1> + a;
        }
        {
            [[maybe_unused]] auto op_right = a - val<1>;
            [[maybe_unused]] auto op_left = val<1> - a;
        }
    };

    return EXIT_SUCCESS;
}
