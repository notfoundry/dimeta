//
// Created by Mark on 9/14/2019.
//

#ifndef DIMETA_NETLIST_MERGE_HPP
#define DIMETA_NETLIST_MERGE_HPP

#include <type_traits>

struct identity {
    template <typename T>
    using f = T;
};

template <typename T, typename C = identity>
struct always {
    template <typename...>
    using f = typename C::template f<T>;
};

template <class... Ts>
struct inherit : Ts... {};

template <class M, class K>
struct map_get;

template <template <class...> class M, class... Es, class K>
struct map_get<M<Es...>, K> {
    template <template <class...> class L, class V>
    static V lookup(always<L<K, V>>*);

    static void lookup(...);

    using type = decltype(lookup((inherit<always<Es>...>*)0));
};

template <bool>
struct if_ {
    template <class T, class F>
    using f = F;
};

template <>
struct if_<true> {
    template <class T, class F>
    using f = T;
};



template <class V, class R>
using foo2 = typename if_<std::is_same<V, void>::value>::template f<R, V>;

template <class M, unsigned I, class R>
using try_assign = foo2<typename map_get<M, std::integral_constant<std::size_t, I>>::type, R>;

static constexpr int select_assign(std::size_t const in) {
    return in >= 16 ? 16 : in >= 4 ? 4 : in >= 2 ? 2 : in == 1 ? 1 : 0;
}

template <unsigned, class... Acc>
struct assign_impl;

template <class... Acc>
struct assign_impl<0, Acc...> {
    template <template <class...> class C, class M, std::size_t I>
    using f = C<Acc...>;
};

template <class... Acc>
struct assign_impl<1, Acc...> {
    template <template <class...> class C, class M, std::size_t I,
            class T0>
    using f = C<Acc..., try_assign<M, I, T0>>;
};

template <class... Acc>
struct assign_impl<2, Acc...> {
    template <template <class...> class C, class M, std::size_t I,
            class T0, class T1, class... Ts>
    using f = typename assign_impl<select_assign(sizeof...(Ts)), Acc...,
            try_assign<M, I, T0>,
            try_assign<M, I + 1, T1>
    >::template f<C, M, I + 2, Ts...>;
};

template <class... Acc>
struct assign_impl<4, Acc...> {
    template <template <class...> class C, class M, std::size_t I,
            class T0, class T1, class T2, class T3, class... Ts>
    using f = typename assign_impl<select_assign(sizeof...(Ts)), Acc...,
            try_assign<M, I, T0>,
            try_assign<M, I + 1, T1>,
            try_assign<M, I + 2, T2>,
            try_assign<M, I + 3, T3>
    >::template f<C, M, I + 4, Ts...>;
};

template <class... Acc>
struct assign_impl<16, Acc...> {
    template <template <class...> class C, class M, std::size_t I,
            class T0, class T1, class T2, class T3, class T4,
            class T5, class T6, class T7, class T8, class T9,
            class T10, class T11, class T12, class T13, class T14,
            class T15, class... Ts>
    using f = typename assign_impl<select_assign(sizeof...(Ts)), Acc...,
            try_assign<M, I, T0>,
            try_assign<M, I + 1, T1>,
            try_assign<M, I + 2, T2>,
            try_assign<M, I + 3, T3>,
            try_assign<M, I + 4, T4>,
            try_assign<M, I + 5, T5>,
            try_assign<M, I + 6, T6>,
            try_assign<M, I + 7, T7>,
            try_assign<M, I + 8, T8>,
            try_assign<M, I + 9, T9>,
            try_assign<M, I + 10, T10>,
            try_assign<M, I + 11, T11>,
            try_assign<M, I + 12, T12>,
            try_assign<M, I + 13, T13>,
            try_assign<M, I + 14, T14>,
            try_assign<M, I + 15, T15>
    >::template f<C, M, I + 16, Ts...>;
};

template <class M, template <class...> class C = std::tuple>
struct assign {
    template <class... Ts>
    using f = typename assign_impl<select_assign(sizeof...(Ts))>::template f<C, M, 0, Ts...>;
};

#endif //DIMETA_NETLIST_MERGE_HPP
