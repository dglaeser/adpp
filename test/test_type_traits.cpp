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
