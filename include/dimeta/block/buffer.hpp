//
// Created by Mark on 9/15/2019.
//

#ifndef DIMETA_BUFFER_HPP
#define DIMETA_BUFFER_HPP

#include <dimeta/primitives.hpp>

namespace dm::block {
    struct buffer {
        template <class A>
        using f = A;
    };
}

#endif //DIMETA_BUFFER_HPP
