//
// Created by Mark on 9/15/2019.
//

#ifndef DIMETA_NOR_HPP
#define DIMETA_NOR_HPP

#include <dimeta/primitives.hpp>

namespace dm::block {
    struct or_ {
        constexpr static logic impl(logic a, logic b) {
            if (a == logic::H || b == logic::H) {
                return logic::L;
            } else if (a == logic::L && b == logic::L) {
                return logic::H;
            } else {
                return logic::X;
            }
        }

        template <class A, class B>
        using f = logic_constant<impl(A::value, B::value)>;
    };
}

#endif //DIMETA_NOR_HPP
