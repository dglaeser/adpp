#include <cstdlib>
#include <sstream>
#include <string>

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

    "value_list_access"_test = [] () {
        using adpp::indices::i;
        using vl = adpp::value_list<0, 1, 2, 3>;
        static_assert(vl::size == 4);
        static_assert(vl::at(i<0>) == 0);
        static_assert(vl::at(i<1>) == 1);
        static_assert(vl::at(i<2>) == 2);
        static_assert(vl::at(i<3>) == 3);
    };

    "value_list_merge"_test = [] () {
        using adpp::indices::i;
        using vl = adpp::value_list<0, 1>;
        constexpr auto merged = vl{} + adpp::value_list_v<4, 5>;
        static_assert(merged.size == 4);
        static_assert(merged.at(i<0>) == 0);
        static_assert(merged.at(i<1>) == 1);
        static_assert(merged.at(i<2>) == 4);
        static_assert(merged.at(i<3>) == 5);
    };

    "value_list_comparators"_test = [] () {
        using adpp::value_list_v;
        static_assert(value_list_v<> == value_list_v<>);
        static_assert(value_list_v<1> == value_list_v<1>);
        static_assert(value_list_v<1> != value_list_v<2>);
        static_assert(value_list_v<1> != value_list_v<>);
        static_assert(value_list_v<1, 2> == value_list_v<1, 2>);
        static_assert(value_list_v<1, 2> != value_list_v<1, 3>);
        static_assert(value_list_v<1, 2> != value_list_v<1>);
        static_assert(value_list_v<1, 2> != value_list_v<>);
    };

    "value_list_streaming"_test = [] () {
        std::ostringstream s;
        s << adpp::value_list_v<0, 42, 43, 44>;
        expect(eq(s.str(), std::string{"[0, 42, 43, 44]"}));
    };

    "split_value_list"_test = [] {
        using adpp::split_at;
        using adpp::value_list;

        using values = value_list<0, 1, 2, 3, 4>;
        static_assert(std::is_same_v<typename split_at<0, values>::head, value_list<>>);
        static_assert(std::is_same_v<typename split_at<0, values>::tail, value_list<0, 1, 2, 3, 4>>);

        static_assert(std::is_same_v<typename split_at<1, values>::head, value_list<0>>);
        static_assert(std::is_same_v<typename split_at<1, values>::tail, value_list<1, 2, 3, 4>>);

        static_assert(std::is_same_v<typename split_at<2, values>::head, value_list<0, 1>>);
        static_assert(std::is_same_v<typename split_at<2, values>::tail, value_list<2, 3, 4>>);

        static_assert(std::is_same_v<typename split_at<3, values>::head, value_list<0, 1, 2>>);
        static_assert(std::is_same_v<typename split_at<3, values>::tail, value_list<3, 4>>);

        static_assert(std::is_same_v<typename split_at<4, values>::head, value_list<0, 1, 2, 3>>);
        static_assert(std::is_same_v<typename split_at<4, values>::tail, value_list<4>>);

        static_assert(std::is_same_v<typename split_at<5, values>::head, value_list<0, 1, 2, 3, 4>>);
        static_assert(std::is_same_v<typename split_at<5, values>::tail, value_list<>>);
    };

    return EXIT_SUCCESS;
}
