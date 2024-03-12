#include <cstdlib>
#include <type_traits>

#include <adpp/type_traits.hpp>

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

    return EXIT_SUCCESS;
}
