#include <cstdlib>
#include <type_traits>

#include <boost/ut.hpp>
#include <cppad/detail.hpp>

int main() {
    using boost::ut::operator""_test;
    using boost::ut::expect;
    using boost::ut::eq;

    "storage_owned"_test = [] () {
        cppad::detail::Storage stored{42.0};
        expect(eq(stored.get(), 42.0));
        static_assert(std::is_same_v<
            decltype(stored.get()),
            double&
        >);
    };

    "storage_owned_const"_test = [] () {
        const cppad::detail::Storage stored{42.0};
        expect(eq(stored.get(), 42.0));
        static_assert(std::is_same_v<
            decltype(stored.get()),
            const double&
        >);
    };

    "storage_reference"_test = [] () {
        double value = 42.0;
        cppad::detail::Storage stored{value};
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
        const cppad::detail::Storage stored{value};
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
