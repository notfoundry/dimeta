/*
* Created by Mark Johnson on 6/22/2019.
*/

#ifndef DIMETA_CONSISTENCY_HPP
#define DIMETA_CONSISTENCY_HPP

#ifdef DIMETA_CONSISTENCY_CHECKS

#include <cstddef>
#include <utility>

#define DIMETA_ASSERT(...) static_assert(__VA_ARGS__)

namespace dm {
    namespace detail {
        template <std::size_t I, bool B>
        struct template_assertion_case {
            static_assert(B, "Contract not satisfied");
        };

        template <class Indices, bool... Bs>
        struct template_assert_impl;

        template <std::size_t... Is, bool... Bs>
        struct template_assert_impl<std::index_sequence<Is...>, Bs...>
                : template_assertion_case<Is, Bs>... {};

        template <bool... Bs>
        struct template_assert : detail::template_assert_impl<std::make_index_sequence<sizeof...(Bs)>, Bs...> {
            template <class T>
            using f = T;
        };
    }
}

#define DIMETA_TMPL_ASSERT_IMPL(...) ::template f<__VA_ARGS__>
#define DIMETA_TMPL_ASSERT(...) typename ::dm::detail::template_assert<__VA_ARGS__> DIMETA_TMPL_ASSERT_IMPL
#else
#define DIMETA_ASSERT(...)

#define DIMETA_TMPL_ASSERT_IMPL(...) __VA_ARGS__
#define DIMETA_TMPL_ASSERT(...) DIMETA_TMPL_ASSERT_IMPL
#endif

#endif //DIMETA_CONSISTENCY_HPP
