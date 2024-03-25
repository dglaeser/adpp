#include <cstdlib>

#include <boost/ut.hpp>

#include <adpp/utils.hpp>

int main() {
    using boost::ut::operator""_test;
    using boost::ut::expect;
    using boost::ut::eq;

    "is_less"_test = [] () {
        static_assert(adpp::is_less_v<1, 2> == true);
        static_assert(adpp::is_less_v<1, 1> == false);
        static_assert(adpp::is_less_v<2, 1> == false);

        static_assert(adpp::is_less_v<1.0, 2> == true);
        static_assert(adpp::is_less_v<1, 1.0> == false);
        static_assert(adpp::is_less_v<2.0, 1> == false);
    };

    "is_equal"_test = [] () {
        static_assert(adpp::is_equal_v<1, 2> == false);
        static_assert(adpp::is_equal_v<1, 1> == true);
        static_assert(adpp::is_equal_v<2, 1> == false);

        static_assert(adpp::is_equal_v<1.0, 2> == false);
        static_assert(adpp::is_equal_v<1, 1.0> == true);
        static_assert(adpp::is_equal_v<2.0, 1> == false);
    };

    "index_constant"_test = [] () {
        using adpp::index_constant;
        using adpp::indices::i;

        static_assert(i<0> == i<0>);
        static_assert(i<0> == 0);

        static_assert(i<1> != i<0>);
        static_assert(i<1> != 0);

        static_assert(i<1> > i<0>);
        static_assert(i<0> < i<1>);

        static_assert(i<1>.incremented() == i<2>);
        static_assert(i<2>.incremented() == i<3>);
    };

    return EXIT_SUCCESS;
}
