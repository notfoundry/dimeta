//
// Created by Mark on 9/15/2019.
//

#ifndef DIMETA_XOR_FUNCTION_HPP
#define DIMETA_XOR_FUNCTION_HPP

#include <dimeta/primitives.hpp>

namespace dm::function {
    struct xor_function {
        constexpr static logic impl(logic a, logic b) {
            if ((a == logic::H && b == logic::L) || (a == logic::L && b == logic::H)) {
                return logic::H;
            } else if ((a == logic::L && b == logic::L) || (a == logic::H && b == logic::H)) {
                return logic::L;
            } else {
                return logic::X;
            }
        }

        template <class A, class B>
        using f = logic_constant<impl(A::value, B::value)>;
    };
}

#endif //DIMETA_XOR_FUNCTION_HPP
