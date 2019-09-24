//
// Created by Mark on 9/15/2019.
//

#ifndef DIMETA_NOT_FUNCTION_HPP
#define DIMETA_NOT_FUNCTION_HPP

#include <dimeta/primitives.hpp>

namespace dm::function {
    struct not_function {
        constexpr static logic impl(logic a) {
            if (a == logic::H) {
                return logic::L;
            } else if (a == logic::L) {
                return logic::H;
            } else {
                return logic::X;
            }
        }

        template <class A>
        using f = logic_constant<impl(A::value)>;
    };
}

#endif //DIMETA_NOT_FUNCTION_HPP
