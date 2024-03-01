#include <cstdlib>

#include <boost/ut.hpp>

#include <cppad/backward/symbols.hpp>

using boost::ut::operator""_test;

using cppad::backward::var;
using cppad::backward::let;


int main() {

    "var_times_scalar"_test = [] () {
        var a;
        {
            [[maybe_unused]] auto op_right = a*1;
            [[maybe_unused]] auto op_left = 1*a;
        }
        {
            [[maybe_unused]] auto op_right = a + 1;
            [[maybe_unused]] auto op_left = 1 + a;
        }
        {
            [[maybe_unused]] auto op_right = a - 1;
            [[maybe_unused]] auto op_left = 1 - a;
        }
    };

    return EXIT_SUCCESS;
}
