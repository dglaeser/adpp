#include <cstdlib>
#include <type_traits>
#include <functional>
#include <sstream>

#include <boost/ut.hpp>

#include <adpp/type_traits.hpp>

template<std::size_t idx>
inline constexpr auto i = adpp::index<idx>;

using boost::ut::operator""_test;
using boost::ut::expect;
using boost::ut::eq;

int main() {

    {
        using merged = adpp::merged_types_t<adpp::type_list<int>, adpp::type_list<char, double>>;
        static_assert(merged::size == 3);
        static_assert(adpp::contains_decayed_v<int, merged>);
        static_assert(adpp::contains_decayed_v<char, merged>);
        static_assert(adpp::contains_decayed_v<double, merged>);
    }

    {
        using unique_merged = adpp::unique_t<
            adpp::merged_types_t<adpp::type_list<int, char, double>, adpp::type_list<char, double>>
        >;
        static_assert(unique_merged::size == 3);
        static_assert(adpp::contains_decayed_v<int, unique_merged>);
        static_assert(adpp::contains_decayed_v<char, unique_merged>);
        static_assert(adpp::contains_decayed_v<double, unique_merged>);
    }

    {
        using adpp::md_index;
        using adpp::value_list;
        static_assert(md_index<0, 0, 0>.with_index_at(i<0>, i<42>) == md_index<42, 0, 0>);
    }

    {
        using adpp::md_index;
        static_assert(md_index<0, 0, 0>.with_appended(i<42>) == md_index<0, 0, 0, 42>);
        static_assert(md_index<0, 0, 0>.with_appended(md_index<42, 43>) == md_index<0, 0, 0, 42, 43>);
    }

    return EXIT_SUCCESS;
}
