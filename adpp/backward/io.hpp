#pragma once

#include <string>
#include <sstream>
#include <ostream>

#include <adpp/common.hpp>
#include <adpp/backward/concepts.hpp>

namespace adpp::backward {

template<typename E, typename S>
struct formatted {
    constexpr formatted(E e, S substitutions)
    : _expr{std::forward<E>(e)}
    , _substitutions{std::forward<S>(substitutions)}
    {}

    friend constexpr std::ostream& operator<<(std::ostream& out, const formatted& f) {
        f._expr.get().export_to(out, f._substitutions.get());
        return out;
    }

    constexpr std::string to_string() const {
        std::stringstream s;
        s << *this;
        return s.str();
    }

 private:
    storage<E> _expr;
    storage<S> _substitutions;
};

template<typename E, typename Arg>
formatted(E&&, Arg&&) -> formatted<E, Arg>;

}  // namespace adpp::backward
