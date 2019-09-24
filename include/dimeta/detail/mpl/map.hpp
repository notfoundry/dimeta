/*
* Created by Mark Johnson on 6/22/2019.
*/

#ifndef DIMETA_MPL_MAP_HPP
#define DIMETA_MPL_MAP_HPP

#include <kvasir/mpl/algorithm/remove_if.hpp>
#include <kvasir/mpl/algorithm/transform.hpp>

#include <kvasir/mpl/functional/call.hpp>
#include <kvasir/mpl/functional/flow.hpp>
#include <kvasir/mpl/functional/fork.hpp>
#include <kvasir/mpl/functional/identity.hpp>

#include <kvasir/mpl/functions/arithmetic/increment.hpp>

#include <kvasir/mpl/sequence/at.hpp>
#include <kvasir/mpl/sequence/front.hpp>
#include <kvasir/mpl/sequence/push_back.hpp>
#include <kvasir/mpl/sequence/take.hpp>

#include <kvasir/mpl/types/list.hpp>
#include <kvasir/mpl/types/bool.hpp>

#include <kvasir/mpl/utility/always.hpp>
#include <kvasir/mpl/utility/same_as.hpp>

namespace dm::detail::mpl::map {
    namespace mpl = kvasir::mpl;

    template <class... Ts>
    struct inherit : Ts... {};

    template <class K, class C = mpl::identity>
    struct get {
        template <template <class...> class L, class V>
        static V lookup(mpl::always<L<K, V>>*);

        static void lookup(...);

        template <class... Es>
        using f = typename C::template f<decltype(lookup((inherit<mpl::always<Es>...>*)0))>;
    };

    template <class K, class C = mpl::identity>
    struct contains {
        template <class... Es>
        using f = mpl::call<get<K, mpl::same_as<void, C>>, Es...>;
    };

    template <class K, class C = mpl::listify>
    struct erase {
        template <class... Es>
        using f = mpl::call<mpl::remove_if<mpl::front<mpl::same_as<K>>, C>, Es...>;
    };

    template <class K, class V, class C = mpl::listify>
    struct replace {
        template <class... Es>
        using f = mpl::call<erase<K, mpl::push_back<mpl::list<K, V>, C>>, Es...>;
    };

    template <class E, class F, class C = mpl::listify>
    struct update {
        using key = mpl::call<mpl::unpack<mpl::front<>>, E>;

        using update_if_uses_key = mpl::unpack<mpl::if_<
                mpl::front<mpl::same_as<key>>,
                mpl::fork<mpl::front<>, F, mpl::listify>,
                mpl::listify
        >>;

        template <class... Es>
        using f = mpl::call<mpl::if_<
                contains<key>,
                mpl::push_back<E, C>,
                mpl::transform<update_if_uses_key, C>
        >, Es...>;
    };

    template <class K, class V, class C = mpl::listify>
    struct insert {
        template <class... Es>
        using f = mpl::call<mpl::if_<contains<K>, C, mpl::push_back<mpl::list<K, V>, C>>, Es...>;
    };

    template <class C = mpl::listify>
    struct replace_all {
        template <class InK, class InV, class FromM>
        using impl = mpl::call<mpl::unpack<
                get<InK, mpl::if_<
                        mpl::same_as<void>, mpl::always<mpl::list<InK, InV>>, mpl::identity
                >>
        >, FromM>;

        template <class InM, class FromM>
        using f = mpl::call<mpl::unpack<mpl::transform<mpl::unpack<mpl::push_back<FromM, mpl::cfe<impl>>>, C>>, InM>;
    };

    template <class C = mpl::listify>
    struct keys {
        template <class... Es>
        using f = mpl::call<mpl::transform<mpl::unpack<mpl::front<>>, C>, Es...>;
    };

    template <class C = mpl::listify>
    struct values {
        template <class... Es>
        using f = mpl::call<mpl::transform<mpl::unpack<mpl::at1<>>, C>, Es...>;
    };
}

#endif //DIMETA_MPL_MAP_HPP
