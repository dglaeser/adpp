#include <cstdlib>
#include <type_traits>

#include <boost/ut.hpp>
#include <adpp/common.hpp>

int main() {
    using boost::ut::operator""_test;
    using boost::ut::expect;
    using boost::ut::eq;

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
