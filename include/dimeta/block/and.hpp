//
// Created by Mark on 9/15/2019.
//

#ifndef DIMETA_AND_HPP
#define DIMETA_AND_HPP

#include <dimeta/primitives.hpp>

namespace dm::block {
    struct and_ {
        constexpr static logic impl(logic a, logic b) {
            if (a == logic::L || b == logic::L) {
                return logic::L;
            } else if (a == logic::H && b == logic::H) {
                return logic::H;
            } else {
                return logic::X;
            }
        }

        template <class A, class B>
        using f = logic_constant<impl(A::value, B::value)>;
    };
}

#endif //DIMETA_AND_HPP
