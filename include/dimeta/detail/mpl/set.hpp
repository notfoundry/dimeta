//
// Created by Mark on 9/22/2019.
//

#ifndef DIMETA_SET_HPP
#define DIMETA_SET_HPP

#include <kvasir/mpl/functional/call.hpp>
#include <kvasir/mpl/functional/flow.hpp>

#include <kvasir/mpl/types/list.hpp>
#include <kvasir/mpl/types/traits.hpp>

#include <kvasir/mpl/utility/always.hpp>
#include <kvasir/mpl/sequence/push_back.hpp>

namespace dm::detail::mpl::set {
    namespace mpl = kvasir::mpl;

    template <class... Ts>
    struct inherit : Ts... {};

    template <class E, class C = mpl::listify>
    struct contains {
        template <class... Es>
        using f = mpl::call<mpl::is_base_of<>, mpl::always<E>, inherit<mpl::always<Es>...>>;
    };

    template <class E, class C = mpl::listify>
    struct insert {
        template <class... Es>
        using f = mpl::call<mpl::if_<contains<E>, C, mpl::push_back<E, C>>, Es...>;
    };
}

#endif //DIMETA_SET_HPP
