#include <cstdlib>
#include <functional>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>
#include <array>
#include <list>

#include <boost/ut.hpp>

#include <adpp/utils.hpp>

template<std::size_t idx>
inline constexpr auto i = adpp::index<idx>;

struct incomplete;
struct complete {};

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

    "type_list"_test = [] () {
        static_assert(adpp::type_list<>::size == 0);
        static_assert(adpp::type_list<int>::size == 1);
        static_assert(adpp::type_list<int, double>::size == 2);
        static_assert(adpp::type_list<int, double, char>::size == 3);
    };

    "decayed_arg_trait"_test = [] () {
        using checked = const int&;
        static_assert(std::is_lvalue_reference_v<checked>);
        static_assert(!adpp::decayed_arg_trait<std::is_lvalue_reference>::template type<checked>::value);
    };

    "value_type_trait"_test = [] () {
        static_assert(std::is_same_v<adpp::value_type_t<std::array<double, 3>>, double>);
        static_assert(std::is_same_v<adpp::value_type_t<std::array<double*, 3>>, double*>);
        static_assert(std::is_same_v<adpp::value_type_t<double[3]>, double>);
        static_assert(std::is_same_v<adpp::value_type_t<std::array<std::array<int, 2>, 1>>, std::array<int, 2>>);
    };

    "md_value_type_trait"_test = [] () {
        static_assert(std::is_same_v<adpp::md_value_type_t<std::array<int, 2>>, int>);
        static_assert(std::is_same_v<adpp::md_value_type_t<std::array<std::array<int, 2>, 1>>, int>);
    };

    "size_of_type_trait"_test = [] () {
        static_assert(adpp::size_of_v<std::array<int, 4>> == 4);
        static_assert(adpp::size_of_v<int[4]> == 4);
    };

    "shape_of_type_trait"_test = [] () {
        static_assert(adpp::shape_of_v<std::array<int, 4>> == adpp::shape<4>);
        static_assert(adpp::shape_of_v<std::array<std::array<int, 2>, 4>> == adpp::shape<4, 2>);
        static_assert(adpp::shape_of_v<std::array<std::array<std::array<int, 1>, 2>, 4>> == adpp::shape<4, 2, 1>);
    };

    "is_complete_trait"_test = [] () {
        static_assert(adpp::is_complete_v<complete>);
        static_assert(!adpp::is_complete_v<incomplete>);
    };

    "first_type_trait"_test = [] () {
        static_assert(std::is_same_v<adpp::first_type_t<adpp::type_list<int>>, int>);
        static_assert(std::is_same_v<adpp::first_type_t<adpp::type_list<int, double>>, int>);
        static_assert(std::is_same_v<adpp::first_type_t<adpp::type_list<double, char>>, double>);
        static_assert(std::is_same_v<adpp::first_type_t<adpp::type_list<double, char, int, bool>>, double>);
    };

    "drop_first_type_trait"_test = [] () {
        static_assert(std::is_same_v<adpp::drop_first_type_t<adpp::type_list<int>>, adpp::type_list<>>);
        static_assert(std::is_same_v<adpp::drop_first_type_t<adpp::type_list<int, double>>, adpp::type_list<double>>);
    };

    "is_any_of_type_trait"_test = [] () {
        static_assert(adpp::is_any_of_v<bool, int, char> == false);
        static_assert(adpp::is_any_of_v<bool, int, char, bool> == true);
        static_assert(adpp::is_any_of_v<bool, adpp::type_list<int, char>> == false);
        static_assert(adpp::is_any_of_v<bool, adpp::type_list<int, char, bool>> == true);
    };

    "is_indexable_trait"_test = [] () {
        static_assert(adpp::is_indexable_v<std::vector<int>>);
        static_assert(adpp::is_indexable_v<std::array<int, 2>>);
        static_assert(!adpp::is_indexable_v<std::list<int>>);
    };

    "is_indexable_nd_trait"_test = [] () {
        static_assert(adpp::is_indexable_nd_v<adpp::md_array<int, adpp::shape<1>>, 1>);
        static_assert(!adpp::is_indexable_nd_v<adpp::md_array<int, adpp::shape<1>>, 2>);

        static_assert(!adpp::is_indexable_nd_v<adpp::md_array<int, adpp::shape<2, 2>>, 1>);
        static_assert(adpp::is_indexable_nd_v<adpp::md_array<int, adpp::shape<2, 2>>, 2>);
        static_assert(adpp::is_indexable_nd_v<adpp::md_array<int, adpp::shape<2, 2, 2>>, 3>);
    };

    "is_indexable_with_trait"_test = [] () {
        static_assert(adpp::is_indexable_with_v<adpp::md_array<int, adpp::shape<1>>, std::size_t>);
        static_assert(!adpp::is_indexable_with_v<adpp::md_array<int, adpp::shape<1>>, std::size_t, std::size_t>);

        static_assert(!adpp::is_indexable_with_v<adpp::md_array<int, adpp::shape<2, 2>>, std::size_t>);
        static_assert(adpp::is_indexable_with_v<adpp::md_array<int, adpp::shape<2, 2>>, std::size_t, std::size_t>);
        static_assert(adpp::is_indexable_with_v<adpp::md_array<int, adpp::shape<2, 2, 2>>, std::size_t, std::size_t, std::size_t>);
    };

    "contains_decayed_type_trait"_test = [] () {
        static_assert(adpp::contains_decayed_v<int&, int, char> == true);
        static_assert(adpp::contains_decayed_v<int&, const int&, char> == true);
        static_assert(adpp::contains_decayed_v<int&, const double&, char> == false);
        static_assert(adpp::contains_decayed_v<int&, adpp::type_list<const int&, char>> == true);
        static_assert(adpp::contains_decayed_v<int&, adpp::type_list<const double&, char>> == false);
    };

    "are_unique_type_trait"_test = [] () {
        static_assert(adpp::are_unique_v<int, char, bool> == true);
        static_assert(adpp::are_unique_v<int, char, int&> == true);
        static_assert(adpp::are_unique_v<int, char, int> == false);
        static_assert(adpp::are_unique_v<adpp::type_list<int, char, int>> == false);
        static_assert(adpp::are_unique_v<adpp::type_list<int, char, int&>> == true);
        static_assert(adpp::are_unique_v<adpp::type_list<int, char, int&>> == true);
    };

    "unique_type_trait"_test = [] () {
        using unique = adpp::unique_t<int, char, int, double, int, double>;
        static_assert(unique::size == 3);
        static_assert(adpp::contains_decayed_v<int, unique>);
        static_assert(adpp::contains_decayed_v<char, unique>);
        static_assert(adpp::contains_decayed_v<double, unique>);

        using unique_from_list = adpp::unique_t<adpp::type_list<int, char, int, double, int, double>>;
        static_assert(unique_from_list::size == 3);
        static_assert(adpp::contains_decayed_v<int, unique_from_list>);
        static_assert(adpp::contains_decayed_v<char, unique_from_list>);
        static_assert(adpp::contains_decayed_v<double, unique_from_list>);
    };

    "merged_type_trait"_test = [] () {
        using merged_lists = adpp::merged_t<adpp::type_list<int, char>, adpp::type_list<double>>;
        static_assert(merged_lists::size == 3);
        static_assert(adpp::contains_decayed_v<int, merged_lists>);
        static_assert(adpp::contains_decayed_v<char, merged_lists>);
        static_assert(adpp::contains_decayed_v<double, merged_lists>);

        using merged_type_with_list = adpp::merged_t<int, adpp::type_list<char, double>>;
        static_assert(merged_type_with_list::size == 3);
        static_assert(adpp::contains_decayed_v<int, merged_type_with_list>);
        static_assert(adpp::contains_decayed_v<char, merged_type_with_list>);
        static_assert(adpp::contains_decayed_v<double, merged_type_with_list>);

        using merged_list_with_type = adpp::merged_t<adpp::type_list<int, char>, double>;
        static_assert(merged_list_with_type::size == 3);
        static_assert(adpp::contains_decayed_v<int, merged_list_with_type>);
        static_assert(adpp::contains_decayed_v<char, merged_list_with_type>);
        static_assert(adpp::contains_decayed_v<double, merged_list_with_type>);
    };

    "filtered_type_trait"_test = [] () {
        using lvalues = adpp::filtered_t<std::is_lvalue_reference, int, char, bool, const std::string&>;
        using pointers = adpp::filtered_t<std::is_pointer, int*, char*, bool, const std::string&>;

        static_assert(lvalues::size == 1);
        static_assert(adpp::is_any_of_v<const std::string&, lvalues>);

        static_assert(pointers::size == 2);
        static_assert(adpp::is_any_of_v<int*, pointers>);
        static_assert(adpp::is_any_of_v<char*, pointers>);
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

        static_assert(adpp::md_index<0, 0, 0>.with_index_at(i<0>, i<42>) == adpp::md_index<42, 0, 0>);
        static_assert(adpp::md_index<0, 0, 0>.with_appended(i<42>) == adpp::md_index<0, 0, 0, 42>);
        static_assert(adpp::md_index<0, 0, 0>.with_appended(adpp::md_index<42, 43>) == adpp::md_index<0, 0, 0, 42, 43>);
    };

    "md_index_constant_iterator"_test = [] () {
        static_assert(adpp::md_index_constant_iterator{adpp::shape<2, 2>}.current() == adpp::md_index<0, 0>);
        static_assert(adpp::md_index_constant_iterator{adpp::shape<2, 2>}.next().current() == adpp::md_index<0, 1>);
        static_assert(adpp::md_index_constant_iterator{adpp::shape<2, 2>}.next().next().current() == adpp::md_index<1, 0>);
        static_assert(adpp::md_index_constant_iterator{adpp::shape<2, 2>}.next().next().next().current() == adpp::md_index<1, 1>);
        static_assert(adpp::md_index_constant_iterator{adpp::shape<2, 2>}.next().next().next().next().is_end());
    };

    "md_access"_test = [] () {
        constexpr std::array<std::array<int, 2>, 2> tensor{{{0, 1}, {2, 3}}};
        static_assert(adpp::access_with(adpp::md_index<0, 0>, tensor) == 0);
        static_assert(adpp::access_with(adpp::md_index<0, 1>, tensor) == 1);
        static_assert(adpp::access_with(adpp::md_index<1, 0>, tensor) == 2);
        static_assert(adpp::access_with(adpp::md_index<1, 1>, tensor) == 3);

        std::vector<std::vector<int>> dyn_tensor{{{0, 1}, {2, 3}}};
        expect(eq(adpp::access_with(adpp::md_index<0, 0>, tensor), 0));
        expect(eq(adpp::access_with(adpp::md_index<0, 1>, tensor), 1));
        expect(eq(adpp::access_with(adpp::md_index<1, 0>, tensor), 2));
        expect(eq(adpp::access_with(adpp::md_index<1, 1>, tensor), 3));
    };

    "for_each_index"_test = [] () {
        adpp::for_each_index_in(adpp::shape<1, 1>, [] <std::size_t... i> (const adpp::md_index_constant<i...>) {
            static_assert(((i == 0) && ...));
        });

        std::size_t v = 0;
        adpp::for_each_index_in(adpp::shape<3>, [&] <std::size_t i> (const adpp::md_index_constant<i>) {
            v += i;
        });
        expect(eq(v, std::size_t{3}));
    };

    "reduce_for_each_index"_test = [] () {
        constexpr auto reduced = adpp::reduce_for_each_index_in(
            adpp::shape<4>,
            double{1},
            [&] <std::size_t i, typename V> (const adpp::md_index_constant<i>, V&& value) {
                return std::forward<V>(value)*static_cast<double>(i+1);
        });
        static_assert(reduced == 1*1*2*3*4);
    };

    "storage_owned"_test = [] () {
        adpp::storage stored{42.0};
        expect(eq(stored.get(), 42.0));
        static_assert(std::is_same_v<
            decltype(stored.get()),
            double&
        >);
    };

    "storage_owned_const"_test = [] () {
        const adpp::storage stored{42.0};
        expect(eq(stored.get(), 42.0));
        static_assert(std::is_same_v<
            decltype(stored.get()),
            const double&
        >);
    };

    "storage_reference"_test = [] () {
        double value = 42.0;
        adpp::storage stored{value};
        expect(eq(stored.get(), 42.0));
        static_assert(std::is_same_v<
            decltype(stored.get()),
            double&
        >);

        value *= 2;
        expect(eq(stored.get(), 84.0));
    };

    "storage_reference_const"_test = [] () {
        double value = 42.0;
        const adpp::storage stored{value};
        expect(eq(stored.get(), 42.0));
        static_assert(std::is_same_v<
            decltype(stored.get()),
            const double&
        >);

        value *= 2;
        expect(eq(stored.get(), 84.0));
    };

    return EXIT_SUCCESS;
}
