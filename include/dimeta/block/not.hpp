//
// Created by Mark on 9/15/2019.
//

#ifndef DIMETA_NOT_HPP
#define DIMETA_NOT_HPP

#include <dimeta/primitives.hpp>

namespace dm::block {
    struct not_ {
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

#endif //DIMETA_NOT_HPP
