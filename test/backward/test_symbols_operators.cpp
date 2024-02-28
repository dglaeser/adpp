#include <cstdlib>

#include <boost/ut.hpp>

#include <cppad/backward/symbols.hpp>

using boost::ut::operator""_test;

using cppad::backward::var;
using cppad::backward::let;


int main(int argc, char** argv) {

    "var_times_scalar"_test = [] () {
        var a;
        {
            auto op_right = a*1;
            auto op_left = 1*a;
        }
        {
            auto op_right = a + 1;
            auto op_left = 1 + a;
        }
        {
            auto op_right = a - 1;
            auto op_left = 1 - a;
        }
    };

    return EXIT_SUCCESS;
}
