#pragma once

#include <cppad/expression.hpp>

namespace cppad {

template<concepts::Arithmetic V>
class Variable : public ExpressionBase {
 public:
    explicit Variable(V&& v)
    : _value{std::move(v)}
    {}

    V value() const {
        return _value;
    }

    template<concepts::Expression E>
    double partial(E&& e) const {
        return this->partial_to(std::forward<E>(e), [] (auto&& e) {
            return 0.0;
        });
    }

 private:
    V _value;
};

template<typename E>
constexpr auto var(E&& e) {
    return Variable{std::forward<E>(e)};
}

}  // namespace cppad
