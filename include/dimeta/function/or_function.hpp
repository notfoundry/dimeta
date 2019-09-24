//
// Created by Mark on 9/15/2019.
//

#ifndef DIMETA_OR_FUNCTION_HPP
#define DIMETA_OR_FUNCTION_HPP

#include <dimeta/primitives.hpp>

namespace dm::function {
    struct or_function {
        constexpr static logic impl(logic a, logic b) {
            if (a == logic::H || b == logic::H) {
                return logic::H;
            } else if (a == logic::L && b == logic::L) {
                return logic::L;
            } else {
                return logic::X;
            }
        }

        template <class A, class B>
        using f = logic_constant<impl(A::value, B::value)>;
    };
}

#endif //DIMETA_OR_FUNCTION_HPP
