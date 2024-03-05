#include <stdexcept>
#include <iostream>
#include <utility>

#define ADD_2(x) x + x
#define ADD_4(x) ADD_2(x) + ADD_2(x)
#define ADD_8(x) ADD_4(x) + ADD_4(x)
#define ADD_16(x) ADD_8(x) + ADD_8(x)
#define ADD_32(x) ADD_16(x) + ADD_16(x)
#define ADD_64(x) ADD_32(x) + ADD_32(x)

#define UNIT_EXPRESSION(a, b) a*((a + b)*b + (a*b) + b)
#define GENERATE_EXPRESSION(a, b) ADD_64(UNIT_EXPRESSION(a, b))

#include <adpp/backward/symbols.hpp>
#include <adpp/backward/evaluate.hpp>
#include <adpp/backward/differentiate.hpp>

int main(int argc, char** argv) {
    if (argc < 3)
        throw std::runtime_error("Expected two input arguments (x, y)");

    double xv = std::atof(argv[1]);
    double yv = std::atof(argv[1]);
    adpp::backward::var x;
    adpp::backward::var y;

    constexpr std::size_t N = 10000;
    std::array<std::array<double, 2>, N> grads;
    for (unsigned int i = 0; i < N; ++i) {
        auto expression = GENERATE_EXPRESSION(x, y);
        auto gradient = grad(expression, at(x = xv, y = yv));
        grads[i][0] = gradient[x];
        grads[i][1] = gradient[y];
    }

    if (argc > 1) {
        double avg_x = 0, avg_y = 0;
        for (unsigned int i = 0; i < N; ++i) {
            avg_x += grads[i][0];
            avg_y += grads[i][1];
        }
        avg_x /= N;
        avg_y /= N;
        std::cout << "∂r/∂x = " << avg_x << std::endl;
        std::cout << "∂r/∂y = " << avg_y << std::endl;
    }

    return 0;
}
