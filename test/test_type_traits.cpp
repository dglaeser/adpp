#include <cstdlib>
#include <type_traits>

#include <adpp/type_traits.hpp>

int main() {

    {
        using unique = adpp::unique_tuple_t<int, char, int, double, int, double>;
        static_assert(adpp::type_size_v<unique> == 3);
        static_assert(adpp::contains_decay_v<int, unique>);
        static_assert(adpp::contains_decay_v<char, unique>);
        static_assert(adpp::contains_decay_v<double, unique>);
    }
    {
        using unique = adpp::unique_tuple_t<
            adpp::type_list<int, char, int, double, int, double>
        >;
        static_assert(adpp::type_size_v<unique> == 3);
        static_assert(adpp::contains_decay_v<int, unique>);
        static_assert(adpp::contains_decay_v<char, unique>);
        static_assert(adpp::contains_decay_v<double, unique>);
    }

    return EXIT_SUCCESS;
}
