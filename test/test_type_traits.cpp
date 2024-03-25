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
        using md_index = adpp::md_index_constant<1, 2, 3>;
        static_assert(md_index::size == 3);
        static_assert(md_index::at(adpp::index_constant<0>{}) == 1);
        static_assert(md_index::at(adpp::index_constant<1>{}) == 2);
        static_assert(md_index::at(adpp::index_constant<2>{}) == 3);
        static_assert(adpp::md_index<1, 2, 3>.at(adpp::index_constant<0>{}) == 1);
        static_assert(adpp::md_index<1, 2, 3>.at(adpp::index_constant<1>{}) == 2);
        static_assert(adpp::md_index<1, 2, 3>.at(adpp::index_constant<2>{}) == 3);
    }

    {
        using adpp::md_index_constant;
        using adpp::md_shape;
        static_assert(md_index_constant<0, 0, 0>::as_flat_index(md_shape<2, 2, 3>{}) == 0);
        static_assert(md_index_constant<0, 0, 1>::as_flat_index(md_shape<2, 2, 3>{}) == 1);
        static_assert(md_index_constant<0, 0, 2>::as_flat_index(md_shape<2, 2, 3>{}) == 2);
        static_assert(md_index_constant<0, 0, 0>{}.last() == 0);
        static_assert(md_index_constant<0, 0, 1>{}.last() == 1);
        static_assert(md_index_constant<0, 0, 2>{}.last() == 2);

        static_assert(md_index_constant<0, 1, 0>::as_flat_index(md_shape<2, 2, 3>{}) == 3);
        static_assert(md_index_constant<0, 1, 1>::as_flat_index(md_shape<2, 2, 3>{}) == 4);
        static_assert(md_index_constant<0, 1, 2>::as_flat_index(md_shape<2, 2, 3>{}) == 5);

        static_assert(md_index_constant<1, 0, 0>::as_flat_index(md_shape<2, 2, 3>{}) == 6);
        static_assert(md_index_constant<1, 0, 1>::as_flat_index(md_shape<2, 2, 3>{}) == 7);
        static_assert(md_index_constant<1, 0, 2>::as_flat_index(md_shape<2, 2, 3>{}) == 8);

        static_assert(md_index_constant<1, 1, 0>::as_flat_index(md_shape<2, 2, 3>{}) == 9);
        static_assert(md_index_constant<1, 1, 1>::as_flat_index(md_shape<2, 2, 3>{}) == 10);
        static_assert(md_index_constant<1, 1, 2>::as_flat_index(md_shape<2, 2, 3>{}) == 11);
    }

    {
        using adpp::md_index;
        using adpp::value_list;
        using adpp::indices::i;
        static_assert(md_index<0, 0, 0>.with_index_at(i<0>, i<42>) == md_index<42, 0, 0>);
    }

    {
        using adpp::md_index;
        using adpp::indices::i;
        static_assert(md_index<0, 0, 0>.with_appended(i<42>) == md_index<0, 0, 0, 42>);
        static_assert(md_index<0, 0, 0>.with_appended(md_index<42, 43>) == md_index<0, 0, 0, 42, 43>);
    }

    {
        using adpp::md_shape;
        using adpp::md_index_constant;
        using adpp::md_index_constant_iterator;

        static_assert(md_index_constant_iterator{md_shape<2, 2>{}}.index() == md_index_constant<0, 0>{});
        static_assert(md_index_constant_iterator{md_shape<2, 2>{}}.next().index() == md_index_constant<0, 1>{});
        static_assert(md_index_constant_iterator{md_shape<2, 2>{}}.next().next().index() == md_index_constant<1, 0>{});
        static_assert(md_index_constant_iterator{md_shape<2, 2>{}}.next().next().next().index() == md_index_constant<1, 1>{});
        static_assert(md_index_constant_iterator{md_shape<2, 2>{}}.next().next().next().next().is_end());
    }

    return EXIT_SUCCESS;
}
