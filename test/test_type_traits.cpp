#include <cstdlib>
#include <type_traits>
#include <functional>
#include <sstream>

#include <boost/ut.hpp>

#include <adpp/type_traits.hpp>

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

int main() {

    {
        using adpp::drop_n_t;
        using adpp::value_list;
        static_assert(std::is_same_v<drop_n_t<2, value_list<0, 1, 2, 3, 4>>, value_list<2, 3, 4>>);
        static_assert(std::is_same_v<drop_n_t<2, value_list<0, 1>>, value_list<>>);
    }

    {
        using adpp::ic;
        using adpp::value_list;
        static_assert(value_list<0, 1, 2, 3>::at(ic<0>) == 0);
        static_assert(value_list<0, 1, 2, 3>::at(ic<1>) == 1);
        static_assert(value_list<0, 1, 2, 3>::at(ic<2>) == 2);
        static_assert(value_list<0, 1, 2, 42>::at(ic<3>) == 42);
    }

    {
        using adpp::value_list;
        static_assert(value_list<>{} == value_list<>{});
        static_assert(value_list<1>{} == value_list<1>{});
        static_assert(value_list<1>{} != value_list<2>{});
        static_assert(value_list<1>{} != value_list<>{});
        static_assert(value_list<1, 2>{} == value_list<1, 2>{});
        static_assert(value_list<1, 2>{} != value_list<1, 3>{});
        static_assert(value_list<1, 2>{} != value_list<1>{});
        static_assert(value_list<1, 2>{} != value_list<>{});
    }

    {
        using adpp::value_list;
        static_assert(value_list<>{} + value_list<>{} == value_list<>{});
        static_assert(value_list<>{} + value_list<1>{} == value_list<1>{});
        static_assert(value_list<1>{} + value_list<1>{} == value_list<1, 1>{});
        static_assert(value_list<1>{} + value_list<>{} == value_list<1>{});
        static_assert(value_list<1, 2>{} + value_list<3, 4>{} == value_list<1, 2, 3, 4>{});
    }

    "value_list_stream_operator"_test = [] () {
        std::stringstream s;
        s << adpp::value_list<42, 43, 44>{};
        expect(eq(s.str(), std::string{"[42, 43, 44]"}));
    };

    {
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
    }

    {
        using unique = adpp::unique_types_t<int, char, int, double, int, double>;
        static_assert(adpp::type_list_size_v<unique> == 3);
        static_assert(adpp::contains_decayed_v<int, unique>);
        static_assert(adpp::contains_decayed_v<char, unique>);
        static_assert(adpp::contains_decayed_v<double, unique>);
    }
    {
        using unique = adpp::unique_types_t<
            adpp::type_list<int, char, int, double, int, double>
        >;
        static_assert(adpp::type_list_size_v<unique> == 3);
        static_assert(adpp::contains_decayed_v<int, unique>);
        static_assert(adpp::contains_decayed_v<char, unique>);
        static_assert(adpp::contains_decayed_v<double, unique>);
    }
    {
        using merged = adpp::merged_types_t<adpp::type_list<int>, adpp::type_list<char, double>>;
        static_assert(adpp::type_list_size_v<merged> == 3);
        static_assert(adpp::contains_decayed_v<int, merged>);
        static_assert(adpp::contains_decayed_v<char, merged>);
        static_assert(adpp::contains_decayed_v<double, merged>);
    }

    {
        using unique_merged = adpp::unique_types_t<
            adpp::merged_types_t<adpp::type_list<int, char, double>, adpp::type_list<char, double>>
        >;
        static_assert(adpp::type_list_size_v<unique_merged> == 3);
        static_assert(adpp::contains_decayed_v<int, unique_merged>);
        static_assert(adpp::contains_decayed_v<char, unique_merged>);
        static_assert(adpp::contains_decayed_v<double, unique_merged>);
    }

    {
        static_assert(adpp::accumulate_v<0, std::plus<void>, 1, 2, 3> == 6);
        static_assert(adpp::accumulate_v<1, std::plus<void>, 1, 2, 3> == 7);
        static_assert(adpp::accumulate_v<1, std::multiplies<void>, 1, 2, 3> == 6);
        static_assert(adpp::accumulate_v<0, std::multiplies<void>, 1, 2.0, 3> == 0);
    }

    {
        static_assert(adpp::dimensions<>::size == 0);
        static_assert(adpp::dimensions<1>::size == 1);
        static_assert(adpp::dimensions<1, 2>::size == 2);
        static_assert(adpp::dimensions<1, 2, 3>::size == 3);
        static_assert(adpp::dimensions<1, 2, 3, 4>::size == 4);

        static_assert(adpp::dimensions<>::number_of_elements == 0);
        static_assert(adpp::dimensions<1>::number_of_elements == 1);
        static_assert(adpp::dimensions<1, 2>::number_of_elements == 2);
        static_assert(adpp::dimensions<2, 2>::number_of_elements == 4);
        static_assert(adpp::dimensions<1, 2, 3>::number_of_elements == 6);

        static_assert(adpp::dimensions<>::last_axis_size == 0);
        static_assert(adpp::dimensions<1>::last_axis_size == 1);
        static_assert(adpp::dimensions<1, 2>::last_axis_size == 2);
        static_assert(adpp::dimensions<2, 3>::last_axis_size == 3);
        static_assert(adpp::dimensions<1, 2, 3>::last_axis_size == 3);
    }

    {
        using md_index = adpp::md_index_constant<1, 2, 3>;
        static_assert(md_index::size == 3);
        static_assert(md_index::get(adpp::index_constant<0>{}) == 1);
        static_assert(md_index::get(adpp::index_constant<1>{}) == 2);
        static_assert(md_index::get(adpp::index_constant<2>{}) == 3);
        static_assert(adpp::md_index<1, 2, 3>.get(adpp::index_constant<0>{}) == 1);
        static_assert(adpp::md_index<1, 2, 3>.get(adpp::index_constant<1>{}) == 2);
        static_assert(adpp::md_index<1, 2, 3>.get(adpp::index_constant<2>{}) == 3);
    }

    {
        using adpp::md_index_constant;
        using adpp::dimensions;
        static_assert(md_index_constant<0, 0, 0>::as_flat_index(dimensions<2, 2, 3>{}) == 0);
        static_assert(md_index_constant<0, 0, 1>::as_flat_index(dimensions<2, 2, 3>{}) == 1);
        static_assert(md_index_constant<0, 0, 2>::as_flat_index(dimensions<2, 2, 3>{}) == 2);
        static_assert(md_index_constant<0, 0, 0>{}.last() == 0);
        static_assert(md_index_constant<0, 0, 1>{}.last() == 1);
        static_assert(md_index_constant<0, 0, 2>{}.last() == 2);

        static_assert(md_index_constant<0, 1, 0>::as_flat_index(dimensions<2, 2, 3>{}) == 3);
        static_assert(md_index_constant<0, 1, 1>::as_flat_index(dimensions<2, 2, 3>{}) == 4);
        static_assert(md_index_constant<0, 1, 2>::as_flat_index(dimensions<2, 2, 3>{}) == 5);

        static_assert(md_index_constant<1, 0, 0>::as_flat_index(dimensions<2, 2, 3>{}) == 6);
        static_assert(md_index_constant<1, 0, 1>::as_flat_index(dimensions<2, 2, 3>{}) == 7);
        static_assert(md_index_constant<1, 0, 2>::as_flat_index(dimensions<2, 2, 3>{}) == 8);

        static_assert(md_index_constant<1, 1, 0>::as_flat_index(dimensions<2, 2, 3>{}) == 9);
        static_assert(md_index_constant<1, 1, 1>::as_flat_index(dimensions<2, 2, 3>{}) == 10);
        static_assert(md_index_constant<1, 1, 2>::as_flat_index(dimensions<2, 2, 3>{}) == 11);
    }

    {
        using adpp::ic;
        using adpp::md_index;
        using adpp::value_list;
        static_assert(md_index<0, 0, 0>.with_index_at(ic<0>, ic<42>) == md_index<42, 0, 0>);
    }

    {
        using adpp::ic;
        using adpp::md_index;
        static_assert(md_index<0, 0, 0>.with_appended(ic<42>) == md_index<0, 0, 0, 42>);
        static_assert(md_index<0, 0, 0>.with_appended(md_index<42, 43>) == md_index<0, 0, 0, 42, 43>);
    }

    {
        using adpp::dimensions;
        using adpp::md_index_constant;
        using adpp::md_index_constant_iterator;

        static_assert(md_index_constant_iterator{dimensions<2, 2>{}}.index() == md_index_constant<0, 0>{});
        static_assert(md_index_constant_iterator{dimensions<2, 2>{}}.next().index() == md_index_constant<0, 1>{});
        static_assert(md_index_constant_iterator{dimensions<2, 2>{}}.next().next().index() == md_index_constant<1, 0>{});
        static_assert(md_index_constant_iterator{dimensions<2, 2>{}}.next().next().next().index() == md_index_constant<1, 1>{});
        static_assert(md_index_constant_iterator{dimensions<2, 2>{}}.next().next().next().next().is_end());
    }

    return EXIT_SUCCESS;
}
