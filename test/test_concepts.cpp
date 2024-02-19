#include <cstdlib>

#include <cppad/cppad.hpp>

struct NoExpression {};

int main() {
    static_assert(cppad::concepts::IntoExpression<double>);
    static_assert(cppad::concepts::IntoExpression<double&>);
    static_assert(cppad::concepts::IntoExpression<const double&>);

    static_assert(cppad::concepts::IntoExpression<int>);
    static_assert(cppad::concepts::IntoExpression<int&>);
    static_assert(cppad::concepts::IntoExpression<const int&>);

    static_assert(!cppad::concepts::IntoExpression<NoExpression>);
    static_assert(!cppad::concepts::IntoExpression<NoExpression&>);
    static_assert(!cppad::concepts::IntoExpression<const NoExpression&>);

    return EXIT_SUCCESS;
}
