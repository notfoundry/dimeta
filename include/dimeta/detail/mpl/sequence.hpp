//
// Created by Mark on 9/12/2019.
//

#ifndef DIMETA_SEQUENCE_HPP
#define DIMETA_SEQUENCE_HPP

#include <kvasir/mpl/functional/call.hpp>
#include <kvasir/mpl/functional/fork.hpp>
#include <kvasir/mpl/sequence/take.hpp>
#include <kvasir/mpl/sequence/push_back.hpp>
#include <kvasir/mpl/functions/arithmetic/increment.hpp>
#include <kvasir/mpl/sequence/join.hpp>
#include <kvasir/mpl/types/list.hpp>

namespace dm::detail::mpl::seq {
    namespace mpl = kvasir::mpl;

    template <class I, class T, class C = mpl::listify>
    struct assign {

        template <class... Ts>
        using f = mpl::call<
                mpl::fork<
                    mpl::take<I, mpl::push_back<T>>,
                    mpl::drop<mpl::eager::increment<I>>,
                    mpl::join<C>
                >,
                Ts...
        >;
    };
}

#endif //DIMETA_SEQUENCE_HPP
