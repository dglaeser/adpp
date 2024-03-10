#include <stdexcept>
#include <iostream>
#include <utility>
#include <array>

#define ADD_2(x) x + x
#define ADD_4(x) ADD_2(x) + ADD_2(x)
#define ADD_8(x) ADD_4(x) + ADD_4(x)
#define ADD_16(x) ADD_8(x) + ADD_8(x)
#define ADD_32(x) ADD_16(x) + ADD_16(x)
#define ADD_64(x) ADD_32(x) + ADD_32(x)

#define UNIT_EXPRESSION(a, b) 2*a*((a + b)*b*4 + (a*b) + 4*b)
#define GENERATE_EXPRESSION(a, b) ADD_64(UNIT_EXPRESSION(a, b))

#include <adpp/backward/symbols.hpp>
#include <adpp/backward/evaluate.hpp>
#include <adpp/backward/differentiate.hpp>

int main(int argc, char** argv) {
    if (argc < 3)
        throw std::runtime_error("Expected two input arguments (x, y)");

    double xv = std::atof(argv[1]);
    double yv = std::atof(argv[2]);
    adpp::backward::var x;
    adpp::backward::var y;

    constexpr std::size_t N = 10000;
    double value = 0.0;
    std::array derivs{0.0, 0.0};
    for (unsigned int i = 0; i < N; ++i) {
        auto expression = GENERATE_EXPRESSION(x, y);
        auto eval = expression(at(x = xv, y = yv));
        auto gradient = expression.gradient(at(x = xv, y = yv));

        value += eval;
        derivs[0] += gradient[x];
        derivs[1] += gradient[y];
    }

    value /= N;
    derivs[0] /= N;
    derivs[1] /= N;

    std::cout << "f(x, y) = " << value << std::endl;
    std::cout << "∂r/∂x = " << derivs[0] << std::endl;
    std::cout << "∂r/∂y = " << derivs[1] << std::endl;

    return 0;
}
