//
// Created by Mark on 9/15/2019.
//

#ifndef DIMETA_BUFFER_FUNCTION_HPP
#define DIMETA_BUFFER_FUNCTION_HPP

#include <dimeta/primitives.hpp>

namespace dm::function {
    struct buffer_function {
        template <class A>
        using f = A;
    };
}

#endif //DIMETA_BUFFER_FUNCTION_HPP
