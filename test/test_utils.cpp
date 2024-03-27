#include <cstdlib>
#include <functional>
#include <sstream>
#include <string>

#include <boost/ut.hpp>

#include <adpp/utils.hpp>

template<std::size_t idx>
inline constexpr auto i = adpp::index<idx>;

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
        using vl = adpp::value_list<0, 1, 2, 3>;
        static_assert(vl::size == 4);
        static_assert(vl::at(i<0>) == 0);
        static_assert(vl::at(i<1>) == 1);
        static_assert(vl::at(i<2>) == 2);
        static_assert(vl::at(i<3>) == 3);
    };

    "value_list_merge"_test = [] () {
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

    "value_list_reduce"_test = [] () {
        static_assert(adpp::value_list_v<0, 1, 2, 3>.reduce_with(std::plus<void>{}, 0) == 6);
        static_assert(adpp::value_list_v<0, 1, 2, 3>.reduce_with(std::plus<void>{}, 1) == 7);
        static_assert(adpp::value_list_v<1, 2, 3, 4>.reduce_with(std::multiplies<void>{}, 1) == 24);
        static_assert(adpp::value_list_v<1, 2, 3.0, 4>.reduce_with(std::multiplies<void>{}, 1) == 24.0);
        static_assert(adpp::value_list_v<1, 2, 3, 4>.reduce_with(std::multiplies<void>{}, 0) == 0);
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

    "drop_n"_test = [] ()  {
        using adpp::drop_n_t;
        using adpp::value_list;
        static_assert(std::is_same_v<drop_n_t<2, value_list<0, 1, 2, 3, 4>>, value_list<2, 3, 4>>);
        static_assert(std::is_same_v<drop_n_t<2, value_list<0, 1>>, value_list<>>);
    };

    "crop_n"_test = [] ()  {
        using adpp::crop_n_t;
        using adpp::value_list;
        static_assert(std::is_same_v<crop_n_t<2, value_list<0, 1, 2, 3, 4>>, value_list<0, 1, 2>>);
        static_assert(std::is_same_v<crop_n_t<2, value_list<0, 1>>, value_list<>>);
    };

    "md_shape"_test = [] () {
        static_assert(adpp::md_shape<>{} == adpp::shape<>);
        static_assert(adpp::md_shape<0, 1>{} == adpp::shape<0, 1>);
        static_assert(adpp::md_shape<0, 2>{} != adpp::shape<0, 1>);
        static_assert(adpp::md_shape<0, 1, 2>{} != adpp::shape<0, 1>);
        static_assert(adpp::md_shape<0, 1>{} != adpp::shape<0, 1, 2>);

        static_assert(adpp::md_shape<>::dimension == 0);
        static_assert(adpp::md_shape<1>::dimension == 1);
        static_assert(adpp::md_shape<1, 2>::dimension == 2);
        static_assert(adpp::md_shape<1, 2, 3>::dimension == 3);
        static_assert(adpp::md_shape<1, 2, 3, 4>::dimension == 4);

        static_assert(adpp::md_shape<>::count == 0);
        static_assert(adpp::md_shape<1>::count == 1);
        static_assert(adpp::md_shape<1, 2>::count == 2);
        static_assert(adpp::md_shape<2, 2>::count == 4);
        static_assert(adpp::md_shape<1, 2, 3>::count == 6);

        static_assert(adpp::md_shape<1>::last() == 1);
        static_assert(adpp::md_shape<1, 2>::last() == 2);
        static_assert(adpp::md_shape<2, 3>::last() == 3);
        static_assert(adpp::md_shape<1, 2, 3>::last() == 3);

        static_assert(adpp::md_shape<1>::first() == 1);
        static_assert(adpp::md_shape<1, 2>::first() == 1);
        static_assert(adpp::md_shape<2, 3>::first() == 2);
        static_assert(adpp::md_shape<1, 2, 3>::first() == 1);

        static_assert(adpp::md_shape<1, 2, 3>::extent_in(i<0>) == 1);
        static_assert(adpp::md_shape<1, 2, 3>::extent_in(i<1>) == 2);
        static_assert(adpp::md_shape<1, 2, 3>::extent_in(i<2>) == 3);
    };

    "shape_flat_index"_test = [] () {
        static_assert(adpp::shape<1, 2, 3>.flat_index_of(0, 0, 1) == 1);
        static_assert(adpp::shape<1, 2, 3>.flat_index_of(0, 0, 2) == 2);
        static_assert(adpp::shape<1, 2, 3>.flat_index_of(0, 1, 1) == 4);
        static_assert(adpp::shape<1, 2, 3>.flat_index_of(0, 1, 2) == 5);
        static_assert(adpp::shape<2, 2, 3>.flat_index_of(1, 0, 0) == 6);

        using adpp::md_index;
        static_assert(adpp::shape<1, 2, 3>.flat_index_of(md_index<0, 0, 1>) == 1);
        static_assert(adpp::shape<1, 2, 3>.flat_index_of(md_index<0, 0, 2>) == 2);
        static_assert(adpp::shape<1, 2, 3>.flat_index_of(md_index<0, 1, 1>) == 4);
        static_assert(adpp::shape<1, 2, 3>.flat_index_of(md_index<0, 1, 2>) == 5);
        static_assert(adpp::shape<2, 2, 3>.flat_index_of(md_index<1, 0, 0>) == 6);
    };

    "md_index_constant"_test = [] () {
        static_assert(adpp::md_index<1, 2, 3>.dimension == 3);
        static_assert(adpp::md_index<1, 2, 3>.at(i<0>) == 1);
        static_assert(adpp::md_index<1, 2, 3>.at(i<1>) == 2);
        static_assert(adpp::md_index<1, 2, 3>.at(i<2>) == 3);
        static_assert(adpp::md_index<1, 2, 3>[i<0>] == 1);
        static_assert(adpp::md_index<1, 2, 3>[i<1>] == 2);
        static_assert(adpp::md_index<1, 2, 3>[i<2>] == 3);
        static_assert(adpp::md_index<1, 2, 3>.first() == 1);
        static_assert(adpp::md_index<1, 2, 3>.last() == 3);
    };

    "md_index_constant_iterator"_test = [] () {
        static_assert(adpp::md_index_constant_iterator{adpp::shape<2, 2>}.current() == adpp::md_index<0, 0>);
        static_assert(adpp::md_index_constant_iterator{adpp::shape<2, 2>}.next().current() == adpp::md_index<0, 1>);
        static_assert(adpp::md_index_constant_iterator{adpp::shape<2, 2>}.next().next().current() == adpp::md_index<1, 0>);
        static_assert(adpp::md_index_constant_iterator{adpp::shape<2, 2>}.next().next().next().current() == adpp::md_index<1, 1>);
        static_assert(adpp::md_index_constant_iterator{adpp::shape<2, 2>}.next().next().next().next().is_end());
    };

    return EXIT_SUCCESS;
}
